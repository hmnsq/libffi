/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2015 Michael Knyszek <mknyszek@berkeley.edu>
                         2015 Andrew Waterman <waterman@cs.berkeley.edu>
                         2018 Stef O'Rear <sorear2@gmail.com>
   Based on MIPS N32/64 port

   RISC-V Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن "internal.h"
#ضمن <tramp.h>

#ضمن <stdlib.h>
#ضمن <stdint.h>

#إذا __riscv_float_abi_double
#وصف_ڪريو ABI_FLEN 64
تعريفنوع اتحاد {
    uint64_t i;
    مزدوج d;
} float_reg;
#وإلاإذا __riscv_float_abi_single
#وصف_ڪريو ABI_FLEN 32
تعريفنوع اتحاد {
    عائم f;
} float_reg;
#جيڪڏهن_پورو

#وصف_ڪريو NARGREG 8
#وصف_ڪريو STKALIGN 16
#وصف_ڪريو MAXCOPYARG (2 * حجم(مزدوج))

تعريفنوع بنية call_context
{
#إذا ABI_FLEN
    float_reg fa[8];
#جيڪڏهن_پورو
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    حرف چوکٺو[16];
} call_context;

تعريفنوع بنية call_builder
{
    call_context *aregs;
    صحيح استعمال_ٿيل_integer;
    صحيح استعمال_ٿيل_float;
    size_t *استعمال_ٿيل_ڍير;
    فراغ *struct_ڍير;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#إذا __SIZEOF_POINTER__ == 8
#وصف_ڪريو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
#وصف_ڪريو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#جيڪڏهن_پورو

#إذا ABI_FLEN
تعريفنوع بنية {
    حرف as_elements, type1, offset2, type2;
} float_struct_info;

#إذا ABI_FLEN >= 64
#وصف_ڪريو IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#وإلا
#وصف_ڪريو IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#جيڪڏهن_پورو

ساكن ffi_type **flatten_struct(ffi_type *اندريون, ffi_type **ٻاهريون, ffi_type **ٻاهريون_پڇاڙي) {
    صحيح i;
    إذا (ٻاهريون == ٻاهريون_پڇاڙي) أرجع ٻاهريون;
    إذا (اندريون->type != FFI_TYPE_STRUCT) {
        *(ٻاهريون++) = اندريون;
    } وإلا {
        لكل (i = 0; اندريون->elements[i]; i++)
            ٻاهريون = flatten_struct(اندريون->elements[i], ٻاهريون, ٻاهريون_پڇاڙي);
    }
    أرجع ٻاهريون;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
ساكن float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *چوٽي) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    صحيح انگ_floats, انگ_ints;
    صحيح انگ_fields = flatten_struct(چوٽي, fields, fields + 3) - fields;

    إذا (انگ_fields == 1) {
        إذا (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } وإلا إذا (انگ_fields == 2) {
        انگ_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        انگ_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        إذا (انگ_floats == 0 || انگ_floats + انگ_ints != 2)
            أرجع ret;
        إذا (cb->استعمال_ٿيل_float + انگ_floats > NARGREG || cb->استعمال_ٿيل_integer + (2 - انگ_floats) > NARGREG)
            أرجع ret;
        إذا (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            أرجع ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->سائيز, fields[1]->alignment);
        ret.as_elements = 1;
    }

    أرجع ret;
}

#إذا ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
ساكن فراغ marshal_float(call_builder *cb, فراغ *معلومات) {
    اتحاد {
        uint32_t i;
        عائم f;
    } قدر;

    قدر.f = *(عائم *)معلومات;
    cb->aregs->fa[cb->استعمال_ٿيل_float++].i =
        UINT64_C(0xffffffff00000000) | قدر.i;
}

ساكن فراغ unmarshal_float(call_builder *cb, فراغ *معلومات) {
    اتحاد {
        uint32_t i;
        عائم f;
    } قدر;

    قدر.i = (uint32_t)cb->aregs->fa[cb->استعمال_ٿيل_float++].i;
    *(عائم *)معلومات = قدر.f;
}
#جيڪڏهن_پورو
#جيڪڏهن_پورو

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ساكن فراغ marshal_atom(call_builder *cb, صحيح type, فراغ *معلومات) {
    size_t قدر = 0;
    بدل (type) {
        حالة FFI_TYPE_UINT8: قدر = *(uint8_t *)معلومات; اكسر;
        حالة FFI_TYPE_SINT8: قدر = *(int8_t *)معلومات; اكسر;
        حالة FFI_TYPE_UINT16: قدر = *(uint16_t *)معلومات; اكسر;
        حالة FFI_TYPE_SINT16: قدر = *(int16_t *)معلومات; اكسر;
        /* 32-bit quantities are always sign-extended in the ABI */
        حالة FFI_TYPE_UINT32: قدر = *(int32_t *)معلومات; اكسر;
        حالة FFI_TYPE_SINT32: قدر = *(int32_t *)معلومات; اكسر;
#إذا __SIZEOF_POINTER__ == 8
        حالة FFI_TYPE_UINT64: قدر = *(uint64_t *)معلومات; اكسر;
        حالة FFI_TYPE_SINT64: قدر = *(int64_t *)معلومات; اكسر;
#جيڪڏهن_پورو
        حالة FFI_TYPE_اشارو: قدر = *(size_t *)معلومات; اكسر;

#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_FLOAT:
            marshal_float(cb, معلومات);
            أرجع;
#وإلاإذا ABI_FLEN >= 32
        حالة FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->استعمال_ٿيل_float++].f) : "0"(*(عائم *)معلومات));
            أرجع;
#جيڪڏهن_پورو
#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->استعمال_ٿيل_float++].d) : "0"(*(مزدوج *)معلومات));
            أرجع;
#جيڪڏهن_پورو
        افتراضي: FFI_ASSERT(0); اكسر;
    }

    إذا (cb->استعمال_ٿيل_integer == NARGREG) {
        *cb->استعمال_ٿيل_ڍير++ = قدر;
    } وإلا {
        cb->aregs->a[cb->استعمال_ٿيل_integer++] = قدر;
    }
}

ساكن فراغ unmarshal_atom(call_builder *cb, صحيح type, فراغ *معلومات) {
    size_t قدر;
    بدل (type) {
#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_FLOAT:
            unmarshal_float(cb, معلومات);
            أرجع;
#وإلاإذا ABI_FLEN >= 32
        حالة FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(عائم *)معلومات) : "0"(cb->aregs->fa[cb->استعمال_ٿيل_float++].f));
            أرجع;
#جيڪڏهن_پورو
#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(مزدوج *)معلومات) : "0"(cb->aregs->fa[cb->استعمال_ٿيل_float++].d));
            أرجع;
#جيڪڏهن_پورو
    }

    إذا (cb->استعمال_ٿيل_integer == NARGREG) {
        قدر = *cb->استعمال_ٿيل_ڍير++;
    } وإلا {
        قدر = cb->aregs->a[cb->استعمال_ٿيل_integer++];
    }

    بدل (type) {
        حالة FFI_TYPE_UINT8: *(uint8_t *)معلومات = قدر; اكسر;
        حالة FFI_TYPE_SINT8: *(uint8_t *)معلومات = قدر; اكسر;
        حالة FFI_TYPE_UINT16: *(uint16_t *)معلومات = قدر; اكسر;
        حالة FFI_TYPE_SINT16: *(uint16_t *)معلومات = قدر; اكسر;
        حالة FFI_TYPE_UINT32: *(uint32_t *)معلومات = قدر; اكسر;
        حالة FFI_TYPE_SINT32: *(uint32_t *)معلومات = قدر; اكسر;
#إذا __SIZEOF_POINTER__ == 8
        حالة FFI_TYPE_UINT64: *(uint64_t *)معلومات = قدر; اكسر;
        حالة FFI_TYPE_SINT64: *(uint64_t *)معلومات = قدر; اكسر;
#جيڪڏهن_پورو
        حالة FFI_TYPE_اشارو: *(size_t *)معلومات = قدر; اكسر;
        افتراضي: FFI_ASSERT(0); اكسر;
    }
}

/* adds an argument to a call, or a not by reference return value */
ساكن فراغ marshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *معلومات) {
    size_t realign[2];

#إذا ABI_FLEN
    إذا (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        إذا (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, معلومات);
            إذا (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((حرف*)معلومات) + fsi.offset2);
            أرجع;
        }
    }

    إذا (!var && cb->استعمال_ٿيل_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, معلومات);
        أرجع;
    }
#جيڪڏهن_پورو

    إذا (type->سائيز > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        معلومات = يادگيري_نقل (cb->struct_ڍير, معلومات, type->سائيز);
        cb->struct_ڍير = (size_t *) FFI_ALIGN ((حرف *) cb->struct_ڍير + type->سائيز, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_اشارو, &معلومات);
    } وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشارو) {
        marshal_atom(cb, type->type, معلومات);
    } وإلا {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        إذا (type->alignment > __SIZEOF_POINTER__) {
            إذا (var)
                cb->استعمال_ٿيل_integer = FFI_ALIGN(cb->استعمال_ٿيل_integer, 2);
            cb->استعمال_ٿيل_ڍير = (size_t *)FFI_ALIGN(cb->استعمال_ٿيل_ڍير, 2*__SIZEOF_POINTER__);
        }

        يادگيري_نقل(realign, معلومات, type->سائيز);
        إذا (type->سائيز > 0)
            marshal_atom(cb, FFI_TYPE_اشارو, realign);
        إذا (type->سائيز > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_اشارو, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساكن فراغ *unmarshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *معلومات) {
    size_t realign[2];
    فراغ *اشارو;

#إذا ABI_FLEN
    إذا (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        إذا (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, معلومات);
            إذا (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((حرف*)معلومات) + fsi.offset2);
            أرجع معلومات;
        }
    }

    إذا (!var && cb->استعمال_ٿيل_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, معلومات);
        أرجع معلومات;
    }
#جيڪڏهن_پورو

    إذا (type->سائيز > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_اشارو, (حرف*)&اشارو);
        أرجع اشارو;
    } وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشارو) {
        unmarshal_atom(cb, type->type, معلومات);
        أرجع معلومات;
    } وإلا {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        إذا (type->alignment > __SIZEOF_POINTER__) {
            إذا (var)
                cb->استعمال_ٿيل_integer = FFI_ALIGN(cb->استعمال_ٿيل_integer, 2);
            cb->استعمال_ٿيل_ڍير = (size_t *)FFI_ALIGN(cb->استعمال_ٿيل_ڍير, 2*__SIZEOF_POINTER__);
        }

        إذا (type->سائيز > 0)
            unmarshal_atom(cb, FFI_TYPE_اشارو, realign);
        إذا (type->سائيز > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_اشارو, realign + 1);
        يادگيري_نقل(معلومات, realign, type->سائيز);
        أرجع معلومات;
    }
}

ساكن صحيح passed_by_ref(call_builder *cb, ffi_type *type, صحيح var) {
#إذا ABI_FLEN
    إذا (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        إذا (fsi.as_elements) أرجع 0;
    }
#جيڪڏهن_پورو

    أرجع type->سائيز > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    أرجع FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif, بنانشان صحيح nfixedargs, بنانشان صحيح ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    أرجع FFI_OK;
}

/* Low level routine for calling functions */
خارجي فراغ ffi_call_asm (فراغ *ڍير, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *گهيرو) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *گهيرو)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_بائيٽون = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * حجم(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_بائيٽون = FFI_ALIGN (cif->بائيٽون, STKALIGN);
    size_t rval_بائيٽون = 0;
    إذا (rvalue == عدم && cif->rtype->سائيز > 2*__SIZEOF_POINTER__)
        rval_بائيٽون = FFI_ALIGN(cif->rtype->سائيز, STKALIGN);
    size_t alloc_سائيز = arg_بائيٽون + rval_بائيٽون + struct_بائيٽون + حجم(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_بنياد;
    /* Argument region must be 16-byte aligned */
    إذا (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_بنياد = (size_t)alloca(alloc_سائيز);
    } وإلا {
        alloc_بنياد = FFI_ALIGN(alloca(alloc_سائيز + STKALIGN - 1), STKALIGN);
    }

    إذا (rval_بائيٽون)
        rvalue = (فراغ*)(alloc_بنياد + arg_بائيٽون);

    call_builder cb;
    cb.استعمال_ٿيل_float = cb.استعمال_ٿيل_integer = 0;
    cb.aregs = (call_context*)(alloc_بنياد + arg_بائيٽون + rval_بائيٽون + struct_بائيٽون);
    cb.استعمال_ٿيل_ڍير = (فراغ*)alloc_بنياد;
    cb.struct_ڍير = (فراغ *) (alloc_بنياد + arg_بائيٽون + rval_بائيٽون);

    صحيح return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    إذا (return_by_ref)
        marshal(&cb, &ffi_type_اشارو, 0, &rvalue);

    صحيح i;
    لكل (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((فراغ *) alloc_بنياد, cb.aregs, fn, گهيرو);

    cb.استعمال_ٿيل_float = cb.استعمال_ٿيل_integer = 0;
    إذا (!return_by_ref && rvalue)
      {
	إذا (IS_INT(cif->rtype->type)
	    && cif->rtype->سائيز < حجم (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    بدل (cif->rtype->type)
	      {
	      حالة FFI_TYPE_SINT8:
	      حالة FFI_TYPE_SINT16:
	      حالة FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (حجم (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		اكسر;
	      حالة FFI_TYPE_UINT8:
	      حالة FFI_TYPE_UINT16:
	      حالة FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (حجم (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		اكسر;
	      }
	  }
	وإلا
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گهيرو)
{
  ffi_call_int(cif, fn, rvalue, avalue, گهيرو);
}

خارجي فراغ ffi_گهيرو_asm(فراغ) FFI_HIDDEN;

ffi_حالت ffi_prep_گهيرو_loc(ffi_گهيرو *گهيرو, ffi_cif *cif, فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*), فراغ *user_معلومات, فراغ *codeloc)
{
    إذا (cif->abi <= FFI_پهريون_ABI || cif->abi >= FFI_آخري_ABI)
        أرجع FFI_BAD_ABI;

#جيڪڏهن_وصف_ٿيل FFI_EXEC_STATIC_TRAMP
  إذا (ffi_tramp_is_present (گهيرو))
    {
      /* Initialize the static trampoline's parameters. */
      فراغ (*dest)(فراغ) = ffi_گهيرو_asm;
      ffi_tramp_سيٽ_parms (گهيرو->ftramp, dest, گهيرو);
    }
  وإلا
#جيڪڏهن_پورو
    {
      uint32_t *tramp = (uint32_t *) &گهيرو->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_گهيرو_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#إذا __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#وإلا
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#جيڪڏهن_پورو
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#إذا !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_سائيز);
#جيڪڏهن_پورو
    }

    گهيرو->cif = cif;
    گهيرو->fun = fun;
    گهيرو->user_معلومات = user_معلومات;

    أرجع FFI_OK;
}

خارجي فراغ ffi_go_گهيرو_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_گهيرو (ffi_go_گهيرو *گهيرو, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_پهريون_ABI || cif->abi >= FFI_آخري_ABI)
    أرجع FFI_BAD_ABI;

  گهيرو->tramp = (فراغ *) ffi_go_گهيرو_asm;
  گهيرو->cif = cif;
  گهيرو->fun = fun;

  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
فراغ FFI_HIDDEN
ffi_گهيرو_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_معلومات,
		   size_t *ڍير, call_context *aregs)
{
    فراغ **avalue = alloca(cif->nargs * حجم(فراغ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    حرف *astorage = alloca(cif->nargs * MAXCOPYARG);
    فراغ *rvalue;
    call_builder cb;
    صحيح return_by_ref;
    صحيح i;

    cb.aregs = aregs;
    cb.استعمال_ٿيل_integer = cb.استعمال_ٿيل_float = 0;
    cb.استعمال_ٿيل_ڍير = ڍير;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    إذا (return_by_ref)
        unmarshal(&cb, &ffi_type_اشارو, 0, &rvalue);
    وإلا
        rvalue = alloca(cif->rtype->سائيز);

    لكل (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_معلومات);

    إذا (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.استعمال_ٿيل_integer = cb.استعمال_ٿيل_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#جيڪڏهن_وصف_ٿيل FFI_EXEC_STATIC_TRAMP
فراغ *
ffi_tramp_arch (size_t *tramp_سائيز, size_t *map_سائيز)
{
  خارجي فراغ *trampoline_ڪوڊ_جدول;
  *tramp_سائيز = RISCV_TRAMP_سائيز;
  *map_سائيز = RISCV_TRAMP_MAP_سائيز;
  أرجع &trampoline_ڪوڊ_جدول;
}
#جيڪڏهن_پورو
