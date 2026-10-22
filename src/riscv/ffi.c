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
#تعریف_کن ABI_FLEN 64
تعريفنوع اتحاد {
    uint64_t i;
    مزدوج d;
} float_reg;
#وإلاإذا __riscv_float_abi_single
#تعریف_کن ABI_FLEN 32
تعريفنوع اتحاد {
    عائم f;
} float_reg;
#پایان_اگر

#تعریف_کن NARGREG 8
#تعریف_کن STKALIGN 16
#تعریف_کن MAXCOPYARG (2 * حجم(مزدوج))

تعريفنوع بنية call_context
{
#إذا ABI_FLEN
    float_reg fa[8];
#پایان_اگر
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    حرف قاب[16];
} call_context;

تعريفنوع بنية call_builder
{
    call_context *aregs;
    صحيح استفاده_شده_integer;
    صحيح استفاده_شده_float;
    size_t *استفاده_شده_پشته;
    فراغ *struct_پشته;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#إذا __SIZEOF_POINTER__ == 8
#تعریف_کن IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
#تعریف_کن IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#پایان_اگر

#إذا ABI_FLEN
تعريفنوع بنية {
    حرف as_elements, type1, offset2, type2;
} float_struct_info;

#إذا ABI_FLEN >= 64
#تعریف_کن IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#وإلا
#تعریف_کن IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#پایان_اگر

ساكن ffi_type **flatten_struct(ffi_type *درون, ffi_type **بیرون, ffi_type **بیرون_انتها) {
    صحيح i;
    إذا (بیرون == بیرون_انتها) أرجع بیرون;
    إذا (درون->type != FFI_TYPE_STRUCT) {
        *(بیرون++) = درون;
    } وإلا {
        لكل (i = 0; درون->elements[i]; i++)
            بیرون = flatten_struct(درون->elements[i], بیرون, بیرون_انتها);
    }
    أرجع بیرون;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
ساكن float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *بالاترین) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    صحيح رقم_floats, رقم_ints;
    صحيح رقم_fields = flatten_struct(بالاترین, fields, fields + 3) - fields;

    إذا (رقم_fields == 1) {
        إذا (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } وإلا إذا (رقم_fields == 2) {
        رقم_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        رقم_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        إذا (رقم_floats == 0 || رقم_floats + رقم_ints != 2)
            أرجع ret;
        إذا (cb->استفاده_شده_float + رقم_floats > NARGREG || cb->استفاده_شده_integer + (2 - رقم_floats) > NARGREG)
            أرجع ret;
        إذا (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            أرجع ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->اندازه, fields[1]->alignment);
        ret.as_elements = 1;
    }

    أرجع ret;
}

#إذا ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
ساكن فراغ marshal_float(call_builder *cb, فراغ *داده) {
    اتحاد {
        uint32_t i;
        عائم f;
    } مقدار;

    مقدار.f = *(عائم *)داده;
    cb->aregs->fa[cb->استفاده_شده_float++].i =
        UINT64_C(0xffffffff00000000) | مقدار.i;
}

ساكن فراغ unmarshal_float(call_builder *cb, فراغ *داده) {
    اتحاد {
        uint32_t i;
        عائم f;
    } مقدار;

    مقدار.i = (uint32_t)cb->aregs->fa[cb->استفاده_شده_float++].i;
    *(عائم *)داده = مقدار.f;
}
#پایان_اگر
#پایان_اگر

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ساكن فراغ marshal_atom(call_builder *cb, صحيح type, فراغ *داده) {
    size_t مقدار = 0;
    بدل (type) {
        حالة FFI_TYPE_UINT8: مقدار = *(uint8_t *)داده; اكسر;
        حالة FFI_TYPE_SINT8: مقدار = *(int8_t *)داده; اكسر;
        حالة FFI_TYPE_UINT16: مقدار = *(uint16_t *)داده; اكسر;
        حالة FFI_TYPE_SINT16: مقدار = *(int16_t *)داده; اكسر;
        /* 32-bit quantities are always sign-extended in the ABI */
        حالة FFI_TYPE_UINT32: مقدار = *(int32_t *)داده; اكسر;
        حالة FFI_TYPE_SINT32: مقدار = *(int32_t *)داده; اكسر;
#إذا __SIZEOF_POINTER__ == 8
        حالة FFI_TYPE_UINT64: مقدار = *(uint64_t *)داده; اكسر;
        حالة FFI_TYPE_SINT64: مقدار = *(int64_t *)داده; اكسر;
#پایان_اگر
        حالة FFI_TYPE_اشاره_گر: مقدار = *(size_t *)داده; اكسر;

#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_FLOAT:
            marshal_float(cb, داده);
            أرجع;
#وإلاإذا ABI_FLEN >= 32
        حالة FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->استفاده_شده_float++].f) : "0"(*(عائم *)داده));
            أرجع;
#پایان_اگر
#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->استفاده_شده_float++].d) : "0"(*(مزدوج *)داده));
            أرجع;
#پایان_اگر
        افتراضي: FFI_ASSERT(0); اكسر;
    }

    إذا (cb->استفاده_شده_integer == NARGREG) {
        *cb->استفاده_شده_پشته++ = مقدار;
    } وإلا {
        cb->aregs->a[cb->استفاده_شده_integer++] = مقدار;
    }
}

ساكن فراغ unmarshal_atom(call_builder *cb, صحيح type, فراغ *داده) {
    size_t مقدار;
    بدل (type) {
#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_FLOAT:
            unmarshal_float(cb, داده);
            أرجع;
#وإلاإذا ABI_FLEN >= 32
        حالة FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(عائم *)داده) : "0"(cb->aregs->fa[cb->استفاده_شده_float++].f));
            أرجع;
#پایان_اگر
#إذا ABI_FLEN >= 64
        حالة FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(مزدوج *)داده) : "0"(cb->aregs->fa[cb->استفاده_شده_float++].d));
            أرجع;
#پایان_اگر
    }

    إذا (cb->استفاده_شده_integer == NARGREG) {
        مقدار = *cb->استفاده_شده_پشته++;
    } وإلا {
        مقدار = cb->aregs->a[cb->استفاده_شده_integer++];
    }

    بدل (type) {
        حالة FFI_TYPE_UINT8: *(uint8_t *)داده = مقدار; اكسر;
        حالة FFI_TYPE_SINT8: *(uint8_t *)داده = مقدار; اكسر;
        حالة FFI_TYPE_UINT16: *(uint16_t *)داده = مقدار; اكسر;
        حالة FFI_TYPE_SINT16: *(uint16_t *)داده = مقدار; اكسر;
        حالة FFI_TYPE_UINT32: *(uint32_t *)داده = مقدار; اكسر;
        حالة FFI_TYPE_SINT32: *(uint32_t *)داده = مقدار; اكسر;
#إذا __SIZEOF_POINTER__ == 8
        حالة FFI_TYPE_UINT64: *(uint64_t *)داده = مقدار; اكسر;
        حالة FFI_TYPE_SINT64: *(uint64_t *)داده = مقدار; اكسر;
#پایان_اگر
        حالة FFI_TYPE_اشاره_گر: *(size_t *)داده = مقدار; اكسر;
        افتراضي: FFI_ASSERT(0); اكسر;
    }
}

/* adds an argument to a call, or a not by reference return value */
ساكن فراغ marshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *داده) {
    size_t realign[2];

#إذا ABI_FLEN
    إذا (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        إذا (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, داده);
            إذا (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((حرف*)داده) + fsi.offset2);
            أرجع;
        }
    }

    إذا (!var && cb->استفاده_شده_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, داده);
        أرجع;
    }
#پایان_اگر

    إذا (type->اندازه > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        داده = کپی_حافظه (cb->struct_پشته, داده, type->اندازه);
        cb->struct_پشته = (size_t *) FFI_ALIGN ((حرف *) cb->struct_پشته + type->اندازه, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_اشاره_گر, &داده);
    } وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشاره_گر) {
        marshal_atom(cb, type->type, داده);
    } وإلا {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        إذا (type->alignment > __SIZEOF_POINTER__) {
            إذا (var)
                cb->استفاده_شده_integer = FFI_ALIGN(cb->استفاده_شده_integer, 2);
            cb->استفاده_شده_پشته = (size_t *)FFI_ALIGN(cb->استفاده_شده_پشته, 2*__SIZEOF_POINTER__);
        }

        کپی_حافظه(realign, داده, type->اندازه);
        إذا (type->اندازه > 0)
            marshal_atom(cb, FFI_TYPE_اشاره_گر, realign);
        إذا (type->اندازه > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_اشاره_گر, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساكن فراغ *unmarshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *داده) {
    size_t realign[2];
    فراغ *اشاره_گر;

#إذا ABI_FLEN
    إذا (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        إذا (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, داده);
            إذا (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((حرف*)داده) + fsi.offset2);
            أرجع داده;
        }
    }

    إذا (!var && cb->استفاده_شده_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, داده);
        أرجع داده;
    }
#پایان_اگر

    إذا (type->اندازه > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_اشاره_گر, (حرف*)&اشاره_گر);
        أرجع اشاره_گر;
    } وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشاره_گر) {
        unmarshal_atom(cb, type->type, داده);
        أرجع داده;
    } وإلا {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        إذا (type->alignment > __SIZEOF_POINTER__) {
            إذا (var)
                cb->استفاده_شده_integer = FFI_ALIGN(cb->استفاده_شده_integer, 2);
            cb->استفاده_شده_پشته = (size_t *)FFI_ALIGN(cb->استفاده_شده_پشته, 2*__SIZEOF_POINTER__);
        }

        إذا (type->اندازه > 0)
            unmarshal_atom(cb, FFI_TYPE_اشاره_گر, realign);
        إذا (type->اندازه > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_اشاره_گر, realign + 1);
        کپی_حافظه(داده, realign, type->اندازه);
        أرجع داده;
    }
}

ساكن صحيح passed_by_ref(call_builder *cb, ffi_type *type, صحيح var) {
#إذا ABI_FLEN
    إذا (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        إذا (fsi.as_elements) أرجع 0;
    }
#پایان_اگر

    أرجع type->اندازه > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_وضعیت ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    أرجع FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_وضعیت ffi_prep_cif_machdep_var(ffi_cif *cif, بیعلامت صحيح nfixedargs, بیعلامت صحيح ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    أرجع FFI_OK;
}

/* Low level routine for calling functions */
خارجي فراغ ffi_call_asm (فراغ *پشته, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *بستگی) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *بستگی)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_بایتها = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * حجم(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_بایتها = FFI_ALIGN (cif->بایتها, STKALIGN);
    size_t rval_بایتها = 0;
    إذا (rvalue == عدم && cif->rtype->اندازه > 2*__SIZEOF_POINTER__)
        rval_بایتها = FFI_ALIGN(cif->rtype->اندازه, STKALIGN);
    size_t alloc_اندازه = arg_بایتها + rval_بایتها + struct_بایتها + حجم(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_پایه;
    /* Argument region must be 16-byte aligned */
    إذا (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_پایه = (size_t)alloca(alloc_اندازه);
    } وإلا {
        alloc_پایه = FFI_ALIGN(alloca(alloc_اندازه + STKALIGN - 1), STKALIGN);
    }

    إذا (rval_بایتها)
        rvalue = (فراغ*)(alloc_پایه + arg_بایتها);

    call_builder cb;
    cb.استفاده_شده_float = cb.استفاده_شده_integer = 0;
    cb.aregs = (call_context*)(alloc_پایه + arg_بایتها + rval_بایتها + struct_بایتها);
    cb.استفاده_شده_پشته = (فراغ*)alloc_پایه;
    cb.struct_پشته = (فراغ *) (alloc_پایه + arg_بایتها + rval_بایتها);

    صحيح return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    إذا (return_by_ref)
        marshal(&cb, &ffi_type_اشاره_گر, 0, &rvalue);

    صحيح i;
    لكل (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((فراغ *) alloc_پایه, cb.aregs, fn, بستگی);

    cb.استفاده_شده_float = cb.استفاده_شده_integer = 0;
    إذا (!return_by_ref && rvalue)
      {
	إذا (IS_INT(cif->rtype->type)
	    && cif->rtype->اندازه < حجم (ffi_arg))
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
	     فراغ **avalue, فراغ *بستگی)
{
  ffi_call_int(cif, fn, rvalue, avalue, بستگی);
}

خارجي فراغ ffi_بستگی_asm(فراغ) FFI_HIDDEN;

ffi_وضعیت ffi_prep_بستگی_loc(ffi_بستگی *بستگی, ffi_cif *cif, فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*), فراغ *user_داده, فراغ *codeloc)
{
    إذا (cif->abi <= FFI_اولی_ABI || cif->abi >= FFI_آخرین_ABI)
        أرجع FFI_BAD_ABI;

#اگر_تعریف_شده FFI_EXEC_STATIC_TRAMP
  إذا (ffi_tramp_is_present (بستگی))
    {
      /* Initialize the static trampoline's parameters. */
      فراغ (*dest)(فراغ) = ffi_بستگی_asm;
      ffi_tramp_مجموعه_parms (بستگی->ftramp, dest, بستگی);
    }
  وإلا
#پایان_اگر
    {
      uint32_t *tramp = (uint32_t *) &بستگی->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_بستگی_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#إذا __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#وإلا
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#پایان_اگر
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#إذا !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_اندازه);
#پایان_اگر
    }

    بستگی->cif = cif;
    بستگی->fun = fun;
    بستگی->user_داده = user_داده;

    أرجع FFI_OK;
}

خارجي فراغ ffi_go_بستگی_asm (فراغ) FFI_HIDDEN;

ffi_وضعیت
ffi_prep_go_بستگی (ffi_go_بستگی *بستگی, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_اولی_ABI || cif->abi >= FFI_آخرین_ABI)
    أرجع FFI_BAD_ABI;

  بستگی->tramp = (فراغ *) ffi_go_بستگی_asm;
  بستگی->cif = cif;
  بستگی->fun = fun;

  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
فراغ FFI_HIDDEN
ffi_بستگی_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_داده,
		   size_t *پشته, call_context *aregs)
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
    cb.استفاده_شده_integer = cb.استفاده_شده_float = 0;
    cb.استفاده_شده_پشته = پشته;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    إذا (return_by_ref)
        unmarshal(&cb, &ffi_type_اشاره_گر, 0, &rvalue);
    وإلا
        rvalue = alloca(cif->rtype->اندازه);

    لكل (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_داده);

    إذا (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.استفاده_شده_integer = cb.استفاده_شده_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#اگر_تعریف_شده FFI_EXEC_STATIC_TRAMP
فراغ *
ffi_tramp_arch (size_t *tramp_اندازه, size_t *map_اندازه)
{
  خارجي فراغ *trampoline_کد_جدول;
  *tramp_اندازه = RISCV_TRAMP_اندازه;
  *map_اندازه = RISCV_TRAMP_MAP_اندازه;
  أرجع &trampoline_کد_جدول;
}
#پایان_اگر
