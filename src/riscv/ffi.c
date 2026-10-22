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
#تعریف_کرو ABI_FLEN 64
تعريفنوع اتحاد {
    uint64_t i;
    مزدوج d;
} float_reg;
#وإلاإذا __riscv_float_abi_single
#تعریف_کرو ABI_FLEN 32
تعريفنوع اتحاد {
    عائم f;
} float_reg;
#جے_انت

#تعریف_کرو NARGREG 8
#تعریف_کرو STKALIGN 16
#تعریف_کرو MAXCOPYARG (2 * حجم(مزدوج))

تعريفنوع بنية call_context
{
#إذا ABI_FLEN
    float_reg fa[8];
#جے_انت
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    حرف چوکھٹا[16];
} call_context;

تعريفنوع بنية call_builder
{
    call_context *aregs;
    صحيح ورتیا_integer;
    صحيح ورتیا_float;
    size_t *ورتیا_ڈھیر;
    فراغ *struct_ڈھیر;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#إذا __SIZEOF_POINTER__ == 8
#تعریف_کرو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
#تعریف_کرو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#جے_انت

#إذا ABI_FLEN
تعريفنوع بنية {
    حرف as_elements, type1, offset2, type2;
} float_struct_info;

#إذا ABI_FLEN >= 64
#تعریف_کرو IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#وإلا
#تعریف_کرو IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#جے_انت

ساكن ffi_type **flatten_struct(ffi_type *وچ, ffi_type **باہرلا, ffi_type **باہرلا_سرا) {
    صحيح i;
    إذا (باہرلا == باہرلا_سرا) أرجع باہرلا;
    إذا (وچ->type != FFI_TYPE_STRUCT) {
        *(باہرلا++) = وچ;
    } وإلا {
        لكل (i = 0; وچ->elements[i]; i++)
            باہرلا = flatten_struct(وچ->elements[i], باہرلا, باہرلا_سرا);
    }
    أرجع باہرلا;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
ساكن float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *چوٹی) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    صحيح عدد_floats, عدد_ints;
    صحيح عدد_fields = flatten_struct(چوٹی, fields, fields + 3) - fields;

    إذا (عدد_fields == 1) {
        إذا (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } وإلا إذا (عدد_fields == 2) {
        عدد_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        عدد_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        إذا (عدد_floats == 0 || عدد_floats + عدد_ints != 2)
            أرجع ret;
        إذا (cb->ورتیا_float + عدد_floats > NARGREG || cb->ورتیا_integer + (2 - عدد_floats) > NARGREG)
            أرجع ret;
        إذا (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            أرجع ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->ناپ, fields[1]->alignment);
        ret.as_elements = 1;
    }

    أرجع ret;
}

#إذا ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
ساكن فراغ marshal_float(call_builder *cb, فراغ *انکڑے) {
    اتحاد {
        uint32_t i;
        عائم f;
    } قدر;

    قدر.f = *(عائم *)انکڑے;
    cb->aregs->fa[cb->ورتیا_float++].i =
        UINT64_C(0xffffffff00000000) | قدر.i;
}

ساكن فراغ unmarshal_float(call_builder *cb, فراغ *انکڑے) {
    اتحاد {
        uint32_t i;
        عائم f;
    } قدر;

    قدر.i = (uint32_t)cb->aregs->fa[cb->ورتیا_float++].i;
    *(عائم *)انکڑے = قدر.f;
}
#جے_انت
#جے_انت

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ساكن فراغ marshal_atom(call_builder *cb, صحيح type, فراغ *انکڑے) {
    size_t قدر = 0;
    بدل (type) {
        حالة FFI_TYPE_UINT8: قدر = *(uint8_t *)انکڑے; اكسر;
        حالة FFI_TYPE_SINT8: قدر = *(int8_t *)انکڑے; اكسر;
        حالة FFI_TYPE_UINT16: قدر = *(uint16_t *)انکڑے; اكسر;
        حالة FFI_TYPE_SINT16: قدر = *(int16_t *)انکڑے; اكسر;
        /* 32-bit quantities are always sign-extended in the ABI */
        حالة FFI_TYPE_UINT32: قدر = *(int32_t *)انکڑے; اكسر;
        حالة FFI_TYPE_SINT32: قدر = *(int32_t *)انکڑے; اكسر;
#إذا __SIZEOF_POINTER__ == 8
        حالة FFI_TYPE_UINT64: قدر = *(uint64_t *)انکڑے; اكسر;
        حالة FFI_TYPE_SINT64: قدر = *(int64_t *)انکڑے; اكسر;
#جے_انت
        حالة FFI_TYPE_اشارہ_گر: قدر = *(size_t *)انکڑے; اكسر;

#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_FLOAT:
            marshal_float(cb, انکڑے);
            أرجع;
#وإلاإذا ABI_FLEN >= 32
        حالة FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->ورتیا_float++].f) : "0"(*(عائم *)انکڑے));
            أرجع;
#جے_انت
#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->ورتیا_float++].d) : "0"(*(مزدوج *)انکڑے));
            أرجع;
#جے_انت
        افتراضي: FFI_ASSERT(0); اكسر;
    }

    إذا (cb->ورتیا_integer == NARGREG) {
        *cb->ورتیا_ڈھیر++ = قدر;
    } وإلا {
        cb->aregs->a[cb->ورتیا_integer++] = قدر;
    }
}

ساكن فراغ unmarshal_atom(call_builder *cb, صحيح type, فراغ *انکڑے) {
    size_t قدر;
    بدل (type) {
#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_FLOAT:
            unmarshal_float(cb, انکڑے);
            أرجع;
#وإلاإذا ABI_FLEN >= 32
        حالة FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(عائم *)انکڑے) : "0"(cb->aregs->fa[cb->ورتیا_float++].f));
            أرجع;
#جے_انت
#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(مزدوج *)انکڑے) : "0"(cb->aregs->fa[cb->ورتیا_float++].d));
            أرجع;
#جے_انت
    }

    إذا (cb->ورتیا_integer == NARGREG) {
        قدر = *cb->ورتیا_ڈھیر++;
    } وإلا {
        قدر = cb->aregs->a[cb->ورتیا_integer++];
    }

    بدل (type) {
        حالة FFI_TYPE_UINT8: *(uint8_t *)انکڑے = قدر; اكسر;
        حالة FFI_TYPE_SINT8: *(uint8_t *)انکڑے = قدر; اكسر;
        حالة FFI_TYPE_UINT16: *(uint16_t *)انکڑے = قدر; اكسر;
        حالة FFI_TYPE_SINT16: *(uint16_t *)انکڑے = قدر; اكسر;
        حالة FFI_TYPE_UINT32: *(uint32_t *)انکڑے = قدر; اكسر;
        حالة FFI_TYPE_SINT32: *(uint32_t *)انکڑے = قدر; اكسر;
#إذا __SIZEOF_POINTER__ == 8
        حالة FFI_TYPE_UINT64: *(uint64_t *)انکڑے = قدر; اكسر;
        حالة FFI_TYPE_SINT64: *(uint64_t *)انکڑے = قدر; اكسر;
#جے_انت
        حالة FFI_TYPE_اشارہ_گر: *(size_t *)انکڑے = قدر; اكسر;
        افتراضي: FFI_ASSERT(0); اكسر;
    }
}

/* adds an argument to a call, or a not by reference return value */
ساكن فراغ marshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *انکڑے) {
    size_t realign[2];

#إذا ABI_FLEN
    إذا (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        إذا (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, انکڑے);
            إذا (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((حرف*)انکڑے) + fsi.offset2);
            أرجع;
        }
    }

    إذا (!var && cb->ورتیا_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, انکڑے);
        أرجع;
    }
#جے_انت

    إذا (type->ناپ > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        انکڑے = حافظہ_نقل (cb->struct_ڈھیر, انکڑے, type->ناپ);
        cb->struct_ڈھیر = (size_t *) FFI_ALIGN ((حرف *) cb->struct_ڈھیر + type->ناپ, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_اشارہ_گر, &انکڑے);
    } وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشارہ_گر) {
        marshal_atom(cb, type->type, انکڑے);
    } وإلا {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        إذا (type->alignment > __SIZEOF_POINTER__) {
            إذا (var)
                cb->ورتیا_integer = FFI_ALIGN(cb->ورتیا_integer, 2);
            cb->ورتیا_ڈھیر = (size_t *)FFI_ALIGN(cb->ورتیا_ڈھیر, 2*__SIZEOF_POINTER__);
        }

        حافظہ_نقل(realign, انکڑے, type->ناپ);
        إذا (type->ناپ > 0)
            marshal_atom(cb, FFI_TYPE_اشارہ_گر, realign);
        إذا (type->ناپ > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_اشارہ_گر, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساكن فراغ *unmarshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *انکڑے) {
    size_t realign[2];
    فراغ *اشارہ_گر;

#إذا ABI_FLEN
    إذا (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        إذا (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, انکڑے);
            إذا (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((حرف*)انکڑے) + fsi.offset2);
            أرجع انکڑے;
        }
    }

    إذا (!var && cb->ورتیا_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, انکڑے);
        أرجع انکڑے;
    }
#جے_انت

    إذا (type->ناپ > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, (حرف*)&اشارہ_گر);
        أرجع اشارہ_گر;
    } وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشارہ_گر) {
        unmarshal_atom(cb, type->type, انکڑے);
        أرجع انکڑے;
    } وإلا {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        إذا (type->alignment > __SIZEOF_POINTER__) {
            إذا (var)
                cb->ورتیا_integer = FFI_ALIGN(cb->ورتیا_integer, 2);
            cb->ورتیا_ڈھیر = (size_t *)FFI_ALIGN(cb->ورتیا_ڈھیر, 2*__SIZEOF_POINTER__);
        }

        إذا (type->ناپ > 0)
            unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, realign);
        إذا (type->ناپ > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, realign + 1);
        حافظہ_نقل(انکڑے, realign, type->ناپ);
        أرجع انکڑے;
    }
}

ساكن صحيح passed_by_ref(call_builder *cb, ffi_type *type, صحيح var) {
#إذا ABI_FLEN
    إذا (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        إذا (fsi.as_elements) أرجع 0;
    }
#جے_انت

    أرجع type->ناپ > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    أرجع FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif, بنانشانی صحيح nfixedargs, بنانشانی صحيح ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    أرجع FFI_OK;
}

/* Low level routine for calling functions */
خارجي فراغ ffi_call_asm (فراغ *ڈھیر, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *گھیرا) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *گھیرا)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_بائٹاں = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * حجم(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_بائٹاں = FFI_ALIGN (cif->بائٹاں, STKALIGN);
    size_t rval_بائٹاں = 0;
    إذا (rvalue == عدم && cif->rtype->ناپ > 2*__SIZEOF_POINTER__)
        rval_بائٹاں = FFI_ALIGN(cif->rtype->ناپ, STKALIGN);
    size_t alloc_ناپ = arg_بائٹاں + rval_بائٹاں + struct_بائٹاں + حجم(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_ادھار;
    /* Argument region must be 16-byte aligned */
    إذا (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_ادھار = (size_t)alloca(alloc_ناپ);
    } وإلا {
        alloc_ادھار = FFI_ALIGN(alloca(alloc_ناپ + STKALIGN - 1), STKALIGN);
    }

    إذا (rval_بائٹاں)
        rvalue = (فراغ*)(alloc_ادھار + arg_بائٹاں);

    call_builder cb;
    cb.ورتیا_float = cb.ورتیا_integer = 0;
    cb.aregs = (call_context*)(alloc_ادھار + arg_بائٹاں + rval_بائٹاں + struct_بائٹاں);
    cb.ورتیا_ڈھیر = (فراغ*)alloc_ادھار;
    cb.struct_ڈھیر = (فراغ *) (alloc_ادھار + arg_بائٹاں + rval_بائٹاں);

    صحيح return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    إذا (return_by_ref)
        marshal(&cb, &ffi_type_اشارہ_گر, 0, &rvalue);

    صحيح i;
    لكل (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((فراغ *) alloc_ادھار, cb.aregs, fn, گھیرا);

    cb.ورتیا_float = cb.ورتیا_integer = 0;
    إذا (!return_by_ref && rvalue)
      {
	إذا (IS_INT(cif->rtype->type)
	    && cif->rtype->ناپ < حجم (ffi_arg))
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
	     فراغ **avalue, فراغ *گھیرا)
{
  ffi_call_int(cif, fn, rvalue, avalue, گھیرا);
}

خارجي فراغ ffi_گھیرا_asm(فراغ) FFI_HIDDEN;

ffi_حالت ffi_prep_گھیرا_loc(ffi_گھیرا *گھیرا, ffi_cif *cif, فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*), فراغ *user_انکڑے, فراغ *codeloc)
{
    إذا (cif->abi <= FFI_پہلا_ABI || cif->abi >= FFI_آخری_ABI)
        أرجع FFI_BAD_ABI;

#جے_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
  إذا (ffi_tramp_is_present (گھیرا))
    {
      /* Initialize the static trampoline's parameters. */
      فراغ (*dest)(فراغ) = ffi_گھیرا_asm;
      ffi_tramp_سیٹ_parms (گھیرا->ftramp, dest, گھیرا);
    }
  وإلا
#جے_انت
    {
      uint32_t *tramp = (uint32_t *) &گھیرا->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_گھیرا_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#إذا __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#وإلا
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#جے_انت
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#إذا !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_ناپ);
#جے_انت
    }

    گھیرا->cif = cif;
    گھیرا->fun = fun;
    گھیرا->user_انکڑے = user_انکڑے;

    أرجع FFI_OK;
}

خارجي فراغ ffi_go_گھیرا_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_گھیرا (ffi_go_گھیرا *گھیرا, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_پہلا_ABI || cif->abi >= FFI_آخری_ABI)
    أرجع FFI_BAD_ABI;

  گھیرا->tramp = (فراغ *) ffi_go_گھیرا_asm;
  گھیرا->cif = cif;
  گھیرا->fun = fun;

  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
فراغ FFI_HIDDEN
ffi_گھیرا_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_انکڑے,
		   size_t *ڈھیر, call_context *aregs)
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
    cb.ورتیا_integer = cb.ورتیا_float = 0;
    cb.ورتیا_ڈھیر = ڈھیر;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    إذا (return_by_ref)
        unmarshal(&cb, &ffi_type_اشارہ_گر, 0, &rvalue);
    وإلا
        rvalue = alloca(cif->rtype->ناپ);

    لكل (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_انکڑے);

    إذا (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.ورتیا_integer = cb.ورتیا_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#جے_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
فراغ *
ffi_tramp_arch (size_t *tramp_ناپ, size_t *map_ناپ)
{
  خارجي فراغ *trampoline_کوڈ_جدول;
  *tramp_ناپ = RISCV_TRAMP_ناپ;
  *map_ناپ = RISCV_TRAMP_MAP_ناپ;
  أرجع &trampoline_کوڈ_جدول;
}
#جے_انت
