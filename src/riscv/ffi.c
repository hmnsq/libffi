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

#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل "internal.h"
#شامل <tramp.h>

#شامل <stdlib.h>
#شامل <stdint.h>

#اگر __riscv_float_abi_double
#تعریف_کرو ABI_FLEN 64
قسمتعریف اتحاد {
    uint64_t i;
    دوہرا d;
} float_reg;
#وگرنہ_اگر __riscv_float_abi_single
#تعریف_کرو ABI_FLEN 32
قسمتعریف اتحاد {
    اعشاری f;
} float_reg;
#اگر_ختم

#تعریف_کرو NARGREG 8
#تعریف_کرو STKALIGN 16
#تعریف_کرو MAXCOPYARG (2 * حجم(دوہرا))

قسمتعریف ساخت call_context
{
#اگر ABI_FLEN
    float_reg fa[8];
#اگر_ختم
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    حرف چوکھٹا[16];
} call_context;

قسمتعریف ساخت call_builder
{
    call_context *aregs;
    صحیح مستعمل_integer;
    صحیح مستعمل_float;
    size_t *مستعمل_ڈھیر;
    خالی *struct_ڈھیر;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#اگر __SIZEOF_POINTER__ == 8
#تعریف_کرو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ورنہ
#تعریف_کرو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#اگر_ختم

#اگر ABI_FLEN
قسمتعریف ساخت {
    حرف as_elements, type1, offset2, type2;
} float_struct_info;

#اگر ABI_FLEN >= 64
#تعریف_کرو IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#ورنہ
#تعریف_کرو IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#اگر_ختم

ساکن ffi_type **flatten_struct(ffi_type *اندر, ffi_type **برآمد, ffi_type **برآمد_انتہا) {
    صحیح i;
    اگر (برآمد == برآمد_انتہا) واپس برآمد;
    اگر (اندر->type != FFI_TYPE_STRUCT) {
        *(برآمد++) = اندر;
    } ورنہ {
        برائے (i = 0; اندر->elements[i]; i++)
            برآمد = flatten_struct(اندر->elements[i], برآمد, برآمد_انتہا);
    }
    واپس برآمد;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
ساکن float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *چوٹی) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    صحیح عدد_floats, عدد_ints;
    صحیح عدد_fields = flatten_struct(چوٹی, fields, fields + 3) - fields;

    اگر (عدد_fields == 1) {
        اگر (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } ورنہ اگر (عدد_fields == 2) {
        عدد_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        عدد_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        اگر (عدد_floats == 0 || عدد_floats + عدد_ints != 2)
            واپس ret;
        اگر (cb->مستعمل_float + عدد_floats > NARGREG || cb->مستعمل_integer + (2 - عدد_floats) > NARGREG)
            واپس ret;
        اگر (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            واپس ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->سائز, fields[1]->alignment);
        ret.as_elements = 1;
    }

    واپس ret;
}

#اگر ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
ساکن خالی marshal_float(call_builder *cb, خالی *ڈیٹا) {
    اتحاد {
        uint32_t i;
        اعشاری f;
    } قدر;

    قدر.f = *(اعشاری *)ڈیٹا;
    cb->aregs->fa[cb->مستعمل_float++].i =
        UINT64_C(0xffffffff00000000) | قدر.i;
}

ساکن خالی unmarshal_float(call_builder *cb, خالی *ڈیٹا) {
    اتحاد {
        uint32_t i;
        اعشاری f;
    } قدر;

    قدر.i = (uint32_t)cb->aregs->fa[cb->مستعمل_float++].i;
    *(اعشاری *)ڈیٹا = قدر.f;
}
#اگر_ختم
#اگر_ختم

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ساکن خالی marshal_atom(call_builder *cb, صحیح type, خالی *ڈیٹا) {
    size_t قدر = 0;
    انتخاب (type) {
        حالت FFI_TYPE_UINT8: قدر = *(uint8_t *)ڈیٹا; توڑو;
        حالت FFI_TYPE_SINT8: قدر = *(int8_t *)ڈیٹا; توڑو;
        حالت FFI_TYPE_UINT16: قدر = *(uint16_t *)ڈیٹا; توڑو;
        حالت FFI_TYPE_SINT16: قدر = *(int16_t *)ڈیٹا; توڑو;
        /* 32-bit quantities are always sign-extended in the ABI */
        حالت FFI_TYPE_UINT32: قدر = *(int32_t *)ڈیٹا; توڑو;
        حالت FFI_TYPE_SINT32: قدر = *(int32_t *)ڈیٹا; توڑو;
#اگر __SIZEOF_POINTER__ == 8
        حالت FFI_TYPE_UINT64: قدر = *(uint64_t *)ڈیٹا; توڑو;
        حالت FFI_TYPE_SINT64: قدر = *(int64_t *)ڈیٹا; توڑو;
#اگر_ختم
        حالت FFI_TYPE_اشارہ_گر: قدر = *(size_t *)ڈیٹا; توڑو;

#اگر ABI_FLEN >= 64
        حالت FFI_TYPE_FLOAT:
            marshal_float(cb, ڈیٹا);
            واپس;
#وگرنہ_اگر ABI_FLEN >= 32
        حالت FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->مستعمل_float++].f) : "0"(*(اعشاری *)ڈیٹا));
            واپس;
#اگر_ختم
#اگر ABI_FLEN >= 64
        حالت FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->مستعمل_float++].d) : "0"(*(دوہرا *)ڈیٹا));
            واپس;
#اگر_ختم
        طےشدہ: FFI_ASSERT(0); توڑو;
    }

    اگر (cb->مستعمل_integer == NARGREG) {
        *cb->مستعمل_ڈھیر++ = قدر;
    } ورنہ {
        cb->aregs->a[cb->مستعمل_integer++] = قدر;
    }
}

ساکن خالی unmarshal_atom(call_builder *cb, صحیح type, خالی *ڈیٹا) {
    size_t قدر;
    انتخاب (type) {
#اگر ABI_FLEN >= 64
        حالت FFI_TYPE_FLOAT:
            unmarshal_float(cb, ڈیٹا);
            واپس;
#وگرنہ_اگر ABI_FLEN >= 32
        حالت FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(اعشاری *)ڈیٹا) : "0"(cb->aregs->fa[cb->مستعمل_float++].f));
            واپس;
#اگر_ختم
#اگر ABI_FLEN >= 64
        حالت FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(دوہرا *)ڈیٹا) : "0"(cb->aregs->fa[cb->مستعمل_float++].d));
            واپس;
#اگر_ختم
    }

    اگر (cb->مستعمل_integer == NARGREG) {
        قدر = *cb->مستعمل_ڈھیر++;
    } ورنہ {
        قدر = cb->aregs->a[cb->مستعمل_integer++];
    }

    انتخاب (type) {
        حالت FFI_TYPE_UINT8: *(uint8_t *)ڈیٹا = قدر; توڑو;
        حالت FFI_TYPE_SINT8: *(uint8_t *)ڈیٹا = قدر; توڑو;
        حالت FFI_TYPE_UINT16: *(uint16_t *)ڈیٹا = قدر; توڑو;
        حالت FFI_TYPE_SINT16: *(uint16_t *)ڈیٹا = قدر; توڑو;
        حالت FFI_TYPE_UINT32: *(uint32_t *)ڈیٹا = قدر; توڑو;
        حالت FFI_TYPE_SINT32: *(uint32_t *)ڈیٹا = قدر; توڑو;
#اگر __SIZEOF_POINTER__ == 8
        حالت FFI_TYPE_UINT64: *(uint64_t *)ڈیٹا = قدر; توڑو;
        حالت FFI_TYPE_SINT64: *(uint64_t *)ڈیٹا = قدر; توڑو;
#اگر_ختم
        حالت FFI_TYPE_اشارہ_گر: *(size_t *)ڈیٹا = قدر; توڑو;
        طےشدہ: FFI_ASSERT(0); توڑو;
    }
}

/* adds an argument to a call, or a not by reference return value */
ساکن خالی marshal(call_builder *cb, ffi_type *type, صحیح var, خالی *ڈیٹا) {
    size_t realign[2];

#اگر ABI_FLEN
    اگر (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        اگر (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, ڈیٹا);
            اگر (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((حرف*)ڈیٹا) + fsi.offset2);
            واپس;
        }
    }

    اگر (!var && cb->مستعمل_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, ڈیٹا);
        واپس;
    }
#اگر_ختم

    اگر (type->سائز > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        ڈیٹا = حافظہ_نقل (cb->struct_ڈھیر, ڈیٹا, type->سائز);
        cb->struct_ڈھیر = (size_t *) FFI_ALIGN ((حرف *) cb->struct_ڈھیر + type->سائز, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_اشارہ_گر, &ڈیٹا);
    } ورنہ اگر (IS_INT(type->type) || type->type == FFI_TYPE_اشارہ_گر) {
        marshal_atom(cb, type->type, ڈیٹا);
    } ورنہ {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        اگر (type->alignment > __SIZEOF_POINTER__) {
            اگر (var)
                cb->مستعمل_integer = FFI_ALIGN(cb->مستعمل_integer, 2);
            cb->مستعمل_ڈھیر = (size_t *)FFI_ALIGN(cb->مستعمل_ڈھیر, 2*__SIZEOF_POINTER__);
        }

        حافظہ_نقل(realign, ڈیٹا, type->سائز);
        اگر (type->سائز > 0)
            marshal_atom(cb, FFI_TYPE_اشارہ_گر, realign);
        اگر (type->سائز > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_اشارہ_گر, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساکن خالی *unmarshal(call_builder *cb, ffi_type *type, صحیح var, خالی *ڈیٹا) {
    size_t realign[2];
    خالی *اشارہ_گر;

#اگر ABI_FLEN
    اگر (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        اگر (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, ڈیٹا);
            اگر (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((حرف*)ڈیٹا) + fsi.offset2);
            واپس ڈیٹا;
        }
    }

    اگر (!var && cb->مستعمل_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, ڈیٹا);
        واپس ڈیٹا;
    }
#اگر_ختم

    اگر (type->سائز > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, (حرف*)&اشارہ_گر);
        واپس اشارہ_گر;
    } ورنہ اگر (IS_INT(type->type) || type->type == FFI_TYPE_اشارہ_گر) {
        unmarshal_atom(cb, type->type, ڈیٹا);
        واپس ڈیٹا;
    } ورنہ {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        اگر (type->alignment > __SIZEOF_POINTER__) {
            اگر (var)
                cb->مستعمل_integer = FFI_ALIGN(cb->مستعمل_integer, 2);
            cb->مستعمل_ڈھیر = (size_t *)FFI_ALIGN(cb->مستعمل_ڈھیر, 2*__SIZEOF_POINTER__);
        }

        اگر (type->سائز > 0)
            unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, realign);
        اگر (type->سائز > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, realign + 1);
        حافظہ_نقل(ڈیٹا, realign, type->سائز);
        واپس ڈیٹا;
    }
}

ساکن صحیح passed_by_ref(call_builder *cb, ffi_type *type, صحیح var) {
#اگر ABI_FLEN
    اگر (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        اگر (fsi.as_elements) واپس 0;
    }
#اگر_ختم

    واپس type->سائز > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    واپس FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif, غیرعلامتی صحیح nfixedargs, غیرعلامتی صحیح ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    واپس FFI_OK;
}

/* Low level routine for calling functions */
خارجی خالی ffi_call_asm (خالی *ڈھیر, ساخت call_context *regs,
			  خالی (*fn) (خالی), خالی *احاطہ) FFI_HIDDEN;

ساکن خالی
ffi_call_int (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue, خالی **avalue,
	      خالی *احاطہ)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_بائٹیں = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * حجم(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_بائٹیں = FFI_ALIGN (cif->بائٹیں, STKALIGN);
    size_t rval_بائٹیں = 0;
    اگر (rvalue == تہی && cif->rtype->سائز > 2*__SIZEOF_POINTER__)
        rval_بائٹیں = FFI_ALIGN(cif->rtype->سائز, STKALIGN);
    size_t alloc_سائز = arg_بائٹیں + rval_بائٹیں + struct_بائٹیں + حجم(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_بنیاد;
    /* Argument region must be 16-byte aligned */
    اگر (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_بنیاد = (size_t)alloca(alloc_سائز);
    } ورنہ {
        alloc_بنیاد = FFI_ALIGN(alloca(alloc_سائز + STKALIGN - 1), STKALIGN);
    }

    اگر (rval_بائٹیں)
        rvalue = (خالی*)(alloc_بنیاد + arg_بائٹیں);

    call_builder cb;
    cb.مستعمل_float = cb.مستعمل_integer = 0;
    cb.aregs = (call_context*)(alloc_بنیاد + arg_بائٹیں + rval_بائٹیں + struct_بائٹیں);
    cb.مستعمل_ڈھیر = (خالی*)alloc_بنیاد;
    cb.struct_ڈھیر = (خالی *) (alloc_بنیاد + arg_بائٹیں + rval_بائٹیں);

    صحیح return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    اگر (return_by_ref)
        marshal(&cb, &ffi_type_اشارہ_گر, 0, &rvalue);

    صحیح i;
    برائے (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((خالی *) alloc_بنیاد, cb.aregs, fn, احاطہ);

    cb.مستعمل_float = cb.مستعمل_integer = 0;
    اگر (!return_by_ref && rvalue)
      {
	اگر (IS_INT(cif->rtype->type)
	    && cif->rtype->سائز < حجم (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    انتخاب (cif->rtype->type)
	      {
	      حالت FFI_TYPE_SINT8:
	      حالت FFI_TYPE_SINT16:
	      حالت FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (حجم (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		توڑو;
	      حالت FFI_TYPE_UINT8:
	      حالت FFI_TYPE_UINT16:
	      حالت FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (حجم (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		توڑو;
	      }
	  }
	ورنہ
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, تہی);
}

خالی
ffi_call_go (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  ffi_call_int(cif, fn, rvalue, avalue, احاطہ);
}

خارجی خالی ffi_احاطہ_asm(خالی) FFI_HIDDEN;

ffi_حالت ffi_prep_احاطہ_loc(ffi_احاطہ *احاطہ, ffi_cif *cif, خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*), خالی *user_ڈیٹا, خالی *codeloc)
{
    اگر (cif->abi <= FFI_پہلا_ABI || cif->abi >= FFI_آخری_ABI)
        واپس FFI_BAD_ABI;

#اگر_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
  اگر (ffi_tramp_is_present (احاطہ))
    {
      /* Initialize the static trampoline's parameters. */
      خالی (*dest)(خالی) = ffi_احاطہ_asm;
      ffi_tramp_سیٹ_parms (احاطہ->ftramp, dest, احاطہ);
    }
  ورنہ
#اگر_ختم
    {
      uint32_t *tramp = (uint32_t *) &احاطہ->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_احاطہ_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#اگر __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#ورنہ
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#اگر_ختم
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#اگر !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_سائز);
#اگر_ختم
    }

    احاطہ->cif = cif;
    احاطہ->fun = fun;
    احاطہ->user_ڈیٹا = user_ڈیٹا;

    واپس FFI_OK;
}

خارجی خالی ffi_go_احاطہ_asm (خالی) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ *احاطہ, ffi_cif *cif,
		     خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *))
{
  اگر (cif->abi <= FFI_پہلا_ABI || cif->abi >= FFI_آخری_ABI)
    واپس FFI_BAD_ABI;

  احاطہ->tramp = (خالی *) ffi_go_احاطہ_asm;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
خالی FFI_HIDDEN
ffi_احاطہ_inner (ffi_cif *cif,
		   خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
		   خالی *user_ڈیٹا,
		   size_t *ڈھیر, call_context *aregs)
{
    خالی **avalue = alloca(cif->nargs * حجم(خالی*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    حرف *astorage = alloca(cif->nargs * MAXCOPYARG);
    خالی *rvalue;
    call_builder cb;
    صحیح return_by_ref;
    صحیح i;

    cb.aregs = aregs;
    cb.مستعمل_integer = cb.مستعمل_float = 0;
    cb.مستعمل_ڈھیر = ڈھیر;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    اگر (return_by_ref)
        unmarshal(&cb, &ffi_type_اشارہ_گر, 0, &rvalue);
    ورنہ
        rvalue = alloca(cif->rtype->سائز);

    برائے (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_ڈیٹا);

    اگر (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.مستعمل_integer = cb.مستعمل_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#اگر_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
خالی *
ffi_tramp_arch (size_t *tramp_سائز, size_t *map_سائز)
{
  خارجی خالی *trampoline_رمز_جدول;
  *tramp_سائز = RISCV_TRAMP_سائز;
  *map_سائز = RISCV_TRAMP_MAP_سائز;
  واپس &trampoline_رمز_جدول;
}
#اگر_ختم
