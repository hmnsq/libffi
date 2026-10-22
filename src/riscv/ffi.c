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
#define ABI_FLEN 64
تعریفنوع اجتماع {
    uint64_t i;
    دوگانه d;
} float_reg;
#وگرنهاگر __riscv_float_abi_single
#define ABI_FLEN 32
تعریفنوع اجتماع {
    اعشاری f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * اندازه(دوگانه))

تعریفنوع ساختار call_context
{
#اگر ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    نویسه frame[16];
} call_context;

تعریفنوع ساختار call_builder
{
    call_context *aregs;
    صحیح used_integer;
    صحیح used_float;
    size_t *used_پشته;
    تهی *struct_پشته;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#اگر __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وگرنه
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#اگر ABI_FLEN
تعریفنوع ساختار {
    نویسه as_elements, type1, offset2, type2;
} float_struct_info;

#اگر ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#وگرنه
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

ایستا ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    صحیح i;
    اگر (out == out_end) بازگشت out;
    اگر (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } وگرنه {
        برای (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    بازگشت out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
ایستا float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *بالاترین) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    صحیح رقم_floats, رقم_ints;
    صحیح رقم_fields = flatten_struct(بالاترین, fields, fields + 3) - fields;

    اگر (رقم_fields == 1) {
        اگر (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } وگرنه اگر (رقم_fields == 2) {
        رقم_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        رقم_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        اگر (رقم_floats == 0 || رقم_floats + رقم_ints != 2)
            بازگشت ret;
        اگر (cb->used_float + رقم_floats > NARGREG || cb->used_integer + (2 - رقم_floats) > NARGREG)
            بازگشت ret;
        اگر (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            بازگشت ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    بازگشت ret;
}

#اگر ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
ایستا تهی marshal_float(call_builder *cb, تهی *data) {
    اجتماع {
        uint32_t i;
        اعشاری f;
    } مقدار;

    مقدار.f = *(اعشاری *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | مقدار.i;
}

ایستا تهی unmarshal_float(call_builder *cb, تهی *data) {
    اجتماع {
        uint32_t i;
        اعشاری f;
    } مقدار;

    مقدار.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(اعشاری *)data = مقدار.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ایستا تهی marshal_atom(call_builder *cb, صحیح type, تهی *data) {
    size_t مقدار = 0;
    انتخاب (type) {
        حالت FFI_TYPE_UINT8: مقدار = *(uint8_t *)data; بشکن;
        حالت FFI_TYPE_SINT8: مقدار = *(int8_t *)data; بشکن;
        حالت FFI_TYPE_UINT16: مقدار = *(uint16_t *)data; بشکن;
        حالت FFI_TYPE_SINT16: مقدار = *(int16_t *)data; بشکن;
        /* 32-bit quantities are always sign-extended in the ABI */
        حالت FFI_TYPE_UINT32: مقدار = *(int32_t *)data; بشکن;
        حالت FFI_TYPE_SINT32: مقدار = *(int32_t *)data; بشکن;
#اگر __SIZEOF_POINTER__ == 8
        حالت FFI_TYPE_UINT64: مقدار = *(uint64_t *)data; بشکن;
        حالت FFI_TYPE_SINT64: مقدار = *(int64_t *)data; بشکن;
#endif
        حالت FFI_TYPE_POINTER: مقدار = *(size_t *)data; بشکن;

#اگر ABI_FLEN >= 64
        حالت FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            بازگشت;
#وگرنهاگر ABI_FLEN >= 32
        حالت FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(اعشاری *)data));
            بازگشت;
#endif
#اگر ABI_FLEN >= 64
        حالت FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(دوگانه *)data));
            بازگشت;
#endif
        پیشفرض: FFI_ASSERT(0); بشکن;
    }

    اگر (cb->used_integer == NARGREG) {
        *cb->used_پشته++ = مقدار;
    } وگرنه {
        cb->aregs->a[cb->used_integer++] = مقدار;
    }
}

ایستا تهی unmarshal_atom(call_builder *cb, صحیح type, تهی *data) {
    size_t مقدار;
    انتخاب (type) {
#اگر ABI_FLEN >= 64
        حالت FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            بازگشت;
#وگرنهاگر ABI_FLEN >= 32
        حالت FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(اعشاری *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            بازگشت;
#endif
#اگر ABI_FLEN >= 64
        حالت FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(دوگانه *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            بازگشت;
#endif
    }

    اگر (cb->used_integer == NARGREG) {
        مقدار = *cb->used_پشته++;
    } وگرنه {
        مقدار = cb->aregs->a[cb->used_integer++];
    }

    انتخاب (type) {
        حالت FFI_TYPE_UINT8: *(uint8_t *)data = مقدار; بشکن;
        حالت FFI_TYPE_SINT8: *(uint8_t *)data = مقدار; بشکن;
        حالت FFI_TYPE_UINT16: *(uint16_t *)data = مقدار; بشکن;
        حالت FFI_TYPE_SINT16: *(uint16_t *)data = مقدار; بشکن;
        حالت FFI_TYPE_UINT32: *(uint32_t *)data = مقدار; بشکن;
        حالت FFI_TYPE_SINT32: *(uint32_t *)data = مقدار; بشکن;
#اگر __SIZEOF_POINTER__ == 8
        حالت FFI_TYPE_UINT64: *(uint64_t *)data = مقدار; بشکن;
        حالت FFI_TYPE_SINT64: *(uint64_t *)data = مقدار; بشکن;
#endif
        حالت FFI_TYPE_POINTER: *(size_t *)data = مقدار; بشکن;
        پیشفرض: FFI_ASSERT(0); بشکن;
    }
}

/* adds an argument to a call, or a not by reference return value */
ایستا تهی marshal(call_builder *cb, ffi_type *type, صحیح var, تهی *data) {
    size_t realign[2];

#اگر ABI_FLEN
    اگر (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        اگر (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            اگر (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((نویسه*)data) + fsi.offset2);
            بازگشت;
        }
    }

    اگر (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        بازگشت;
    }
#endif

    اگر (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = memcpy (cb->struct_پشته, data, type->size);
        cb->struct_پشته = (size_t *) FFI_ALIGN ((نویسه *) cb->struct_پشته + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } وگرنه اگر (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } وگرنه {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        اگر (type->alignment > __SIZEOF_POINTER__) {
            اگر (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_پشته = (size_t *)FFI_ALIGN(cb->used_پشته, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, data, type->size);
        اگر (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        اگر (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ایستا تهی *unmarshal(call_builder *cb, ffi_type *type, صحیح var, تهی *data) {
    size_t realign[2];
    تهی *pointer;

#اگر ABI_FLEN
    اگر (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        اگر (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            اگر (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((نویسه*)data) + fsi.offset2);
            بازگشت data;
        }
    }

    اگر (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        بازگشت data;
    }
#endif

    اگر (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (نویسه*)&pointer);
        بازگشت pointer;
    } وگرنه اگر (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        بازگشت data;
    } وگرنه {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        اگر (type->alignment > __SIZEOF_POINTER__) {
            اگر (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_پشته = (size_t *)FFI_ALIGN(cb->used_پشته, 2*__SIZEOF_POINTER__);
        }

        اگر (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        اگر (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(data, realign, type->size);
        بازگشت data;
    }
}

ایستا صحیح passed_by_ref(call_builder *cb, ffi_type *type, صحیح var) {
#اگر ABI_FLEN
    اگر (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        اگر (fsi.as_elements) بازگشت 0;
    }
#endif

    بازگشت type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    بازگشت FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned صحیح nfixedargs, unsigned صحیح ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    بازگشت FFI_OK;
}

/* Low level routine for calling functions */
بیرونی تهی ffi_call_asm (تهی *پشته, ساختار call_context *regs,
			  تهی (*fn) (تهی), تهی *closure) FFI_HIDDEN;

ایستا تهی
ffi_call_int (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue, تهی **avalue,
	      تهی *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * اندازه(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    اگر (rvalue == پوچ && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    size_t alloc_size = arg_bytes + rval_bytes + struct_bytes + اندازه(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    اگر (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_size);
    } وگرنه {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    اگر (rval_bytes)
        rvalue = (تهی*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_پشته = (تهی*)alloc_base;
    cb.struct_پشته = (تهی *) (alloc_base + arg_bytes + rval_bytes);

    صحیح return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    اگر (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    صحیح i;
    برای (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((تهی *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    اگر (!return_by_ref && rvalue)
      {
	اگر (IS_INT(cif->rtype->type)
	    && cif->rtype->size < اندازه (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    انتخاب (cif->rtype->type)
	      {
	      حالت FFI_TYPE_SINT8:
	      حالت FFI_TYPE_SINT16:
	      حالت FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (اندازه (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		بشکن;
	      حالت FFI_TYPE_UINT8:
	      حالت FFI_TYPE_UINT16:
	      حالت FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (اندازه (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		بشکن;
	      }
	  }
	وگرنه
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

تهی
ffi_call (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue, تهی **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, پوچ);
}

تهی
ffi_call_go (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue,
	     تهی **avalue, تهی *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

بیرونی تهی ffi_closure_asm(تهی) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*), تهی *user_data, تهی *codeloc)
{
    اگر (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        بازگشت FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  اگر (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      تهی (*dest)(تهی) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  وگرنه
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#اگر __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#وگرنه
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#اگر !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    بازگشت FFI_OK;
}

بیرونی تهی ffi_go_closure_asm (تهی) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *))
{
  اگر (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    بازگشت FFI_BAD_ABI;

  closure->tramp = (تهی *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  بازگشت FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
تهی FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
		   تهی *user_data,
		   size_t *پشته, call_context *aregs)
{
    تهی **avalue = alloca(cif->nargs * اندازه(تهی*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    نویسه *astorage = alloca(cif->nargs * MAXCOPYARG);
    تهی *rvalue;
    call_builder cb;
    صحیح return_by_ref;
    صحیح i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_پشته = پشته;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    اگر (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    وگرنه
        rvalue = alloca(cif->rtype->size);

    برای (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    اگر (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
تهی *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  بیرونی تهی *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  بازگشت &trampoline_code_table;
}
#endif
