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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব "internal.h"
#অন্তর্ভাব <tramp.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdint.h>

#যদি __riscv_float_abi_double
#define ABI_FLEN 64
প্রকারলেখ সংঘ {
    uint64_t i;
    দ্বিগুণ d;
} float_reg;
#নতুবা_যদি __riscv_float_abi_single
#define ABI_FLEN 32
প্রকারলেখ সংঘ {
    দশমিক f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * আকার(দ্বিগুণ))

প্রকারলেখ কাঠামো call_context
{
#যদি ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    অক্ষর frame[16];
} call_context;

প্রকারলেখ কাঠামো call_builder
{
    call_context *aregs;
    পূর্ণসংখ্যা used_integer;
    পূর্ণসংখ্যা used_float;
    size_t *used_স্তূপ;
    শূন্য *struct_স্তূপ;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#যদি __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#অন্যথা
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#যদি ABI_FLEN
প্রকারলেখ কাঠামো {
    অক্ষর as_elements, type1, offset2, type2;
} float_struct_info;

#যদি ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#অন্যথা
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

স্থির ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    পূর্ণসংখ্যা i;
    যদি (out == out_end) ফেরত out;
    যদি (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } অন্যথা {
        জন্য (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    ফেরত out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
স্থির float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *শীর্ষ_2) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    পূর্ণসংখ্যা অঙ্ক_floats, অঙ্ক_ints;
    পূর্ণসংখ্যা অঙ্ক_fields = flatten_struct(শীর্ষ_2, fields, fields + 3) - fields;

    যদি (অঙ্ক_fields == 1) {
        যদি (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } অন্যথা যদি (অঙ্ক_fields == 2) {
        অঙ্ক_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        অঙ্ক_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        যদি (অঙ্ক_floats == 0 || অঙ্ক_floats + অঙ্ক_ints != 2)
            ফেরত ret;
        যদি (cb->used_float + অঙ্ক_floats > NARGREG || cb->used_integer + (2 - অঙ্ক_floats) > NARGREG)
            ফেরত ret;
        যদি (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            ফেরত ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->আকার_2, fields[1]->alignment);
        ret.as_elements = 1;
    }

    ফেরত ret;
}

#যদি ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
স্থির শূন্য marshal_float(call_builder *cb, শূন্য *তথ্য) {
    সংঘ {
        uint32_t i;
        দশমিক f;
    } মান;

    মান.f = *(দশমিক *)তথ্য;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | মান.i;
}

স্থির শূন্য unmarshal_float(call_builder *cb, শূন্য *তথ্য) {
    সংঘ {
        uint32_t i;
        দশমিক f;
    } মান;

    মান.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(দশমিক *)তথ্য = মান.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
স্থির শূন্য marshal_atom(call_builder *cb, পূর্ণসংখ্যা type, শূন্য *তথ্য) {
    size_t মান = 0;
    নির্বাচন (type) {
        ক্ষেত্র FFI_TYPE_UINT8: মান = *(uint8_t *)তথ্য; বিরতি;
        ক্ষেত্র FFI_TYPE_SINT8: মান = *(int8_t *)তথ্য; বিরতি;
        ক্ষেত্র FFI_TYPE_UINT16: মান = *(uint16_t *)তথ্য; বিরতি;
        ক্ষেত্র FFI_TYPE_SINT16: মান = *(int16_t *)তথ্য; বিরতি;
        /* 32-bit quantities are always sign-extended in the ABI */
        ক্ষেত্র FFI_TYPE_UINT32: মান = *(int32_t *)তথ্য; বিরতি;
        ক্ষেত্র FFI_TYPE_SINT32: মান = *(int32_t *)তথ্য; বিরতি;
#যদি __SIZEOF_POINTER__ == 8
        ক্ষেত্র FFI_TYPE_UINT64: মান = *(uint64_t *)তথ্য; বিরতি;
        ক্ষেত্র FFI_TYPE_SINT64: মান = *(int64_t *)তথ্য; বিরতি;
#endif
        ক্ষেত্র FFI_TYPE_POINTER: মান = *(size_t *)তথ্য; বিরতি;

#যদি ABI_FLEN >= 64
        ক্ষেত্র FFI_TYPE_FLOAT:
            marshal_float(cb, তথ্য);
            ফেরত;
#নতুবা_যদি ABI_FLEN >= 32
        ক্ষেত্র FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(দশমিক *)তথ্য));
            ফেরত;
#endif
#যদি ABI_FLEN >= 64
        ক্ষেত্র FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(দ্বিগুণ *)তথ্য));
            ফেরত;
#endif
        পূর্বনির্ধারিত: FFI_ASSERT(0); বিরতি;
    }

    যদি (cb->used_integer == NARGREG) {
        *cb->used_স্তূপ++ = মান;
    } অন্যথা {
        cb->aregs->a[cb->used_integer++] = মান;
    }
}

স্থির শূন্য unmarshal_atom(call_builder *cb, পূর্ণসংখ্যা type, শূন্য *তথ্য) {
    size_t মান;
    নির্বাচন (type) {
#যদি ABI_FLEN >= 64
        ক্ষেত্র FFI_TYPE_FLOAT:
            unmarshal_float(cb, তথ্য);
            ফেরত;
#নতুবা_যদি ABI_FLEN >= 32
        ক্ষেত্র FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(দশমিক *)তথ্য) : "0"(cb->aregs->fa[cb->used_float++].f));
            ফেরত;
#endif
#যদি ABI_FLEN >= 64
        ক্ষেত্র FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(দ্বিগুণ *)তথ্য) : "0"(cb->aregs->fa[cb->used_float++].d));
            ফেরত;
#endif
    }

    যদি (cb->used_integer == NARGREG) {
        মান = *cb->used_স্তূপ++;
    } অন্যথা {
        মান = cb->aregs->a[cb->used_integer++];
    }

    নির্বাচন (type) {
        ক্ষেত্র FFI_TYPE_UINT8: *(uint8_t *)তথ্য = মান; বিরতি;
        ক্ষেত্র FFI_TYPE_SINT8: *(uint8_t *)তথ্য = মান; বিরতি;
        ক্ষেত্র FFI_TYPE_UINT16: *(uint16_t *)তথ্য = মান; বিরতি;
        ক্ষেত্র FFI_TYPE_SINT16: *(uint16_t *)তথ্য = মান; বিরতি;
        ক্ষেত্র FFI_TYPE_UINT32: *(uint32_t *)তথ্য = মান; বিরতি;
        ক্ষেত্র FFI_TYPE_SINT32: *(uint32_t *)তথ্য = মান; বিরতি;
#যদি __SIZEOF_POINTER__ == 8
        ক্ষেত্র FFI_TYPE_UINT64: *(uint64_t *)তথ্য = মান; বিরতি;
        ক্ষেত্র FFI_TYPE_SINT64: *(uint64_t *)তথ্য = মান; বিরতি;
#endif
        ক্ষেত্র FFI_TYPE_POINTER: *(size_t *)তথ্য = মান; বিরতি;
        পূর্বনির্ধারিত: FFI_ASSERT(0); বিরতি;
    }
}

/* adds an argument to a call, or a not by reference return value */
স্থির শূন্য marshal(call_builder *cb, ffi_type *type, পূর্ণসংখ্যা var, শূন্য *তথ্য) {
    size_t realign[2];

#যদি ABI_FLEN
    যদি (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        যদি (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, তথ্য);
            যদি (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((অক্ষর*)তথ্য) + fsi.offset2);
            ফেরত;
        }
    }

    যদি (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, তথ্য);
        ফেরত;
    }
#endif

    যদি (type->আকার_2 > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        তথ্য = memcpy (cb->struct_স্তূপ, তথ্য, type->আকার_2);
        cb->struct_স্তূপ = (size_t *) FFI_ALIGN ((অক্ষর *) cb->struct_স্তূপ + type->আকার_2, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &তথ্য);
    } অন্যথা যদি (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, তথ্য);
    } অন্যথা {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        যদি (type->alignment > __SIZEOF_POINTER__) {
            যদি (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_স্তূপ = (size_t *)FFI_ALIGN(cb->used_স্তূপ, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, তথ্য, type->আকার_2);
        যদি (type->আকার_2 > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        যদি (type->আকার_2 > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
স্থির শূন্য *unmarshal(call_builder *cb, ffi_type *type, পূর্ণসংখ্যা var, শূন্য *তথ্য) {
    size_t realign[2];
    শূন্য *pointer;

#যদি ABI_FLEN
    যদি (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        যদি (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, তথ্য);
            যদি (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((অক্ষর*)তথ্য) + fsi.offset2);
            ফেরত তথ্য;
        }
    }

    যদি (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, তথ্য);
        ফেরত তথ্য;
    }
#endif

    যদি (type->আকার_2 > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (অক্ষর*)&pointer);
        ফেরত pointer;
    } অন্যথা যদি (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, তথ্য);
        ফেরত তথ্য;
    } অন্যথা {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        যদি (type->alignment > __SIZEOF_POINTER__) {
            যদি (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_স্তূপ = (size_t *)FFI_ALIGN(cb->used_স্তূপ, 2*__SIZEOF_POINTER__);
        }

        যদি (type->আকার_2 > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        যদি (type->আকার_2 > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(তথ্য, realign, type->আকার_2);
        ফেরত তথ্য;
    }
}

স্থির পূর্ণসংখ্যা passed_by_ref(call_builder *cb, ffi_type *type, পূর্ণসংখ্যা var) {
#যদি ABI_FLEN
    যদি (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        যদি (fsi.as_elements) ফেরত 0;
    }
#endif

    ফেরত type->আকার_2 > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    ফেরত FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned পূর্ণসংখ্যা nfixedargs, unsigned পূর্ণসংখ্যা ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    ফেরত FFI_OK;
}

/* Low level routine for calling functions */
বাহ্যিক শূন্য ffi_call_asm (শূন্য *স্তূপ, কাঠামো call_context *regs,
			  শূন্য (*fn) (শূন্য), শূন্য *closure) FFI_HIDDEN;

স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue,
	      শূন্য *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * আকার(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    যদি (rvalue == ফাঁকা && cif->rtype->আকার_2 > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->আকার_2, STKALIGN);
    size_t alloc_আকার = arg_bytes + rval_bytes + struct_bytes + আকার(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    যদি (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_আকার);
    } অন্যথা {
        alloc_base = FFI_ALIGN(alloca(alloc_আকার + STKALIGN - 1), STKALIGN);
    }

    যদি (rval_bytes)
        rvalue = (শূন্য*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_স্তূপ = (শূন্য*)alloc_base;
    cb.struct_স্তূপ = (শূন্য *) (alloc_base + arg_bytes + rval_bytes);

    পূর্ণসংখ্যা return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    যদি (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((শূন্য *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    যদি (!return_by_ref && rvalue)
      {
	যদি (IS_INT(cif->rtype->type)
	    && cif->rtype->আকার_2 < আকার (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    নির্বাচন (cif->rtype->type)
	      {
	      ক্ষেত্র FFI_TYPE_SINT8:
	      ক্ষেত্র FFI_TYPE_SINT16:
	      ক্ষেত্র FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (আকার (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		বিরতি;
	      ক্ষেত্র FFI_TYPE_UINT8:
	      ক্ষেত্র FFI_TYPE_UINT16:
	      ক্ষেত্র FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (আকার (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		বিরতি;
	      }
	  }
	অন্যথা
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ফাঁকা);
}

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

বাহ্যিক শূন্য ffi_closure_asm(শূন্য) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*), শূন্য *user_তথ্য, শূন্য *codeloc)
{
    যদি (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        ফেরত FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  যদি (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      শূন্য (*dest)(শূন্য) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  অন্যথা
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#যদি __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#অন্যথা
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#যদি !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_আকার);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_তথ্য = user_তথ্য;

    ফেরত FFI_OK;
}

বাহ্যিক শূন্য ffi_go_closure_asm (শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *))
{
  যদি (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ফেরত FFI_BAD_ABI;

  closure->tramp = (শূন্য *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ফেরত FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
শূন্য FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		   শূন্য *user_তথ্য,
		   size_t *স্তূপ, call_context *aregs)
{
    শূন্য **avalue = alloca(cif->nargs * আকার(শূন্য*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    অক্ষর *astorage = alloca(cif->nargs * MAXCOPYARG);
    শূন্য *rvalue;
    call_builder cb;
    পূর্ণসংখ্যা return_by_ref;
    পূর্ণসংখ্যা i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_স্তূপ = স্তূপ;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    যদি (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    অন্যথা
        rvalue = alloca(cif->rtype->আকার_2);

    জন্য (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_তথ্য);

    যদি (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
শূন্য *
ffi_tramp_arch (size_t *tramp_আকার, size_t *map_আকার)
{
  বাহ্যিক শূন্য *trampoline_code_table;
  *tramp_আকার = RISCV_TRAMP_আকার;
  *map_আকার = RISCV_TRAMP_MAP_আকার;
  ফেরত &trampoline_code_table;
}
#endif
