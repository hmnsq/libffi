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

#𐤀𐤌 __riscv_float_abi_double
#define ABI_FLEN 64
تعريفنوع اتحاد {
    uint64_t i;
    مزدوج d;
} float_reg;
#وإلاإذا __riscv_float_abi_single
#define ABI_FLEN 32
تعريفنوع اتحاد {
    عائم f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * حجم(مزدوج))

تعريفنوع بنية call_context
{
#𐤀𐤌 ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    حرف frame[16];
} call_context;

تعريفنوع بنية call_builder
{
    call_context *aregs;
    صحيح used_integer;
    صحيح used_float;
    size_t *used_stack;
    فراغ *struct_stack;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#𐤀𐤌 __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#𐤀𐤌 ABI_FLEN
تعريفنوع بنية {
    حرف as_elements, type1, offset2, type2;
} float_struct_info;

#𐤀𐤌 ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#وإلا
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

ساكن ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    صحيح i;
    𐤀𐤌 (out == out_end) أرجع out;
    𐤀𐤌 (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } وإلا {
        لكل (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    أرجع out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
ساكن float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *top) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    صحيح num_floats, num_ints;
    صحيح num_fields = flatten_struct(top, fields, fields + 3) - fields;

    𐤀𐤌 (num_fields == 1) {
        𐤀𐤌 (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } وإلا 𐤀𐤌 (num_fields == 2) {
        num_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        num_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        𐤀𐤌 (num_floats == 0 || num_floats + num_ints != 2)
            أرجع ret;
        𐤀𐤌 (cb->used_float + num_floats > NARGREG || cb->used_integer + (2 - num_floats) > NARGREG)
            أرجع ret;
        𐤀𐤌 (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            أرجع ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    أرجع ret;
}

#𐤀𐤌 ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
ساكن فراغ marshal_float(call_builder *cb, فراغ *data) {
    اتحاد {
        uint32_t i;
        عائم f;
    } value;

    value.f = *(عائم *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | value.i;
}

ساكن فراغ unmarshal_float(call_builder *cb, فراغ *data) {
    اتحاد {
        uint32_t i;
        عائم f;
    } value;

    value.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(عائم *)data = value.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ساكن فراغ marshal_atom(call_builder *cb, صحيح type, فراغ *data) {
    size_t value = 0;
    بدل (type) {
        حالة FFI_TYPE_UINT8: value = *(uint8_t *)data; اكسر;
        حالة FFI_TYPE_SINT8: value = *(int8_t *)data; اكسر;
        حالة FFI_TYPE_UINT16: value = *(uint16_t *)data; اكسر;
        حالة FFI_TYPE_SINT16: value = *(int16_t *)data; اكسر;
        /* 32-bit quantities are always sign-extended in the ABI */
        حالة FFI_TYPE_UINT32: value = *(int32_t *)data; اكسر;
        حالة FFI_TYPE_SINT32: value = *(int32_t *)data; اكسر;
#𐤀𐤌 __SIZEOF_POINTER__ == 8
        حالة FFI_TYPE_UINT64: value = *(uint64_t *)data; اكسر;
        حالة FFI_TYPE_SINT64: value = *(int64_t *)data; اكسر;
#endif
        حالة FFI_TYPE_POINTER: value = *(size_t *)data; اكسر;

#𐤀𐤌 ABI_FLEN >= 64
        حالة FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            أرجع;
#وإلاإذا ABI_FLEN >= 32
        حالة FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(عائم *)data));
            أرجع;
#endif
#𐤀𐤌 ABI_FLEN >= 64
        حالة FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(مزدوج *)data));
            أرجع;
#endif
        افتراضي: FFI_ASSERT(0); اكسر;
    }

    𐤀𐤌 (cb->used_integer == NARGREG) {
        *cb->used_stack++ = value;
    } وإلا {
        cb->aregs->a[cb->used_integer++] = value;
    }
}

ساكن فراغ unmarshal_atom(call_builder *cb, صحيح type, فراغ *data) {
    size_t value;
    بدل (type) {
#𐤀𐤌 ABI_FLEN >= 64
        حالة FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            أرجع;
#وإلاإذا ABI_FLEN >= 32
        حالة FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(عائم *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            أرجع;
#endif
#𐤀𐤌 ABI_FLEN >= 64
        حالة FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(مزدوج *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            أرجع;
#endif
    }

    𐤀𐤌 (cb->used_integer == NARGREG) {
        value = *cb->used_stack++;
    } وإلا {
        value = cb->aregs->a[cb->used_integer++];
    }

    بدل (type) {
        حالة FFI_TYPE_UINT8: *(uint8_t *)data = value; اكسر;
        حالة FFI_TYPE_SINT8: *(uint8_t *)data = value; اكسر;
        حالة FFI_TYPE_UINT16: *(uint16_t *)data = value; اكسر;
        حالة FFI_TYPE_SINT16: *(uint16_t *)data = value; اكسر;
        حالة FFI_TYPE_UINT32: *(uint32_t *)data = value; اكسر;
        حالة FFI_TYPE_SINT32: *(uint32_t *)data = value; اكسر;
#𐤀𐤌 __SIZEOF_POINTER__ == 8
        حالة FFI_TYPE_UINT64: *(uint64_t *)data = value; اكسر;
        حالة FFI_TYPE_SINT64: *(uint64_t *)data = value; اكسر;
#endif
        حالة FFI_TYPE_POINTER: *(size_t *)data = value; اكسر;
        افتراضي: FFI_ASSERT(0); اكسر;
    }
}

/* adds an argument to a call, or a not by reference return value */
ساكن فراغ marshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *data) {
    size_t realign[2];

#𐤀𐤌 ABI_FLEN
    𐤀𐤌 (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        𐤀𐤌 (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            𐤀𐤌 (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((حرف*)data) + fsi.offset2);
            أرجع;
        }
    }

    𐤀𐤌 (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        أرجع;
    }
#endif

    𐤀𐤌 (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = memcpy (cb->struct_stack, data, type->size);
        cb->struct_stack = (size_t *) FFI_ALIGN ((حرف *) cb->struct_stack + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } وإلا 𐤀𐤌 (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } وإلا {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        𐤀𐤌 (type->alignment > __SIZEOF_POINTER__) {
            𐤀𐤌 (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_stack = (size_t *)FFI_ALIGN(cb->used_stack, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, data, type->size);
        𐤀𐤌 (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        𐤀𐤌 (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساكن فراغ *unmarshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *data) {
    size_t realign[2];
    فراغ *pointer;

#𐤀𐤌 ABI_FLEN
    𐤀𐤌 (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        𐤀𐤌 (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            𐤀𐤌 (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((حرف*)data) + fsi.offset2);
            أرجع data;
        }
    }

    𐤀𐤌 (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        أرجع data;
    }
#endif

    𐤀𐤌 (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (حرف*)&pointer);
        أرجع pointer;
    } وإلا 𐤀𐤌 (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        أرجع data;
    } وإلا {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        𐤀𐤌 (type->alignment > __SIZEOF_POINTER__) {
            𐤀𐤌 (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_stack = (size_t *)FFI_ALIGN(cb->used_stack, 2*__SIZEOF_POINTER__);
        }

        𐤀𐤌 (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        𐤀𐤌 (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(data, realign, type->size);
        أرجع data;
    }
}

ساكن صحيح passed_by_ref(call_builder *cb, ffi_type *type, صحيح var) {
#𐤀𐤌 ABI_FLEN
    𐤀𐤌 (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        𐤀𐤌 (fsi.as_elements) أرجع 0;
    }
#endif

    أرجع type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    أرجع FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned صحيح nfixedargs, unsigned صحيح ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    أرجع FFI_OK;
}

/* Low level routine for calling functions */
خارجي فراغ ffi_call_asm (فراغ *stack, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *closure) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * حجم(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    𐤀𐤌 (rvalue == عدم && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    size_t alloc_size = arg_bytes + rval_bytes + struct_bytes + حجم(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    𐤀𐤌 (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_size);
    } وإلا {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    𐤀𐤌 (rval_bytes)
        rvalue = (فراغ*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_stack = (فراغ*)alloc_base;
    cb.struct_stack = (فراغ *) (alloc_base + arg_bytes + rval_bytes);

    صحيح return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    𐤀𐤌 (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    صحيح i;
    لكل (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((فراغ *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    𐤀𐤌 (!return_by_ref && rvalue)
      {
	𐤀𐤌 (IS_INT(cif->rtype->type)
	    && cif->rtype->size < حجم (ffi_arg))
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
	     فراغ **avalue, فراغ *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

خارجي فراغ ffi_closure_asm(فراغ) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*), فراغ *user_data, فراغ *codeloc)
{
    𐤀𐤌 (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        أرجع FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  𐤀𐤌 (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      فراغ (*dest)(فراغ) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  وإلا
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#𐤀𐤌 __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#وإلا
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#𐤀𐤌 !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    أرجع FFI_OK;
}

خارجي فراغ ffi_go_closure_asm (فراغ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  𐤀𐤌 (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    أرجع FFI_BAD_ABI;

  closure->tramp = (فراغ *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
فراغ FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_data,
		   size_t *stack, call_context *aregs)
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
    cb.used_integer = cb.used_float = 0;
    cb.used_stack = stack;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    𐤀𐤌 (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    وإلا
        rvalue = alloca(cif->rtype->size);

    لكل (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    𐤀𐤌 (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
فراغ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  خارجي فراغ *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  أرجع &trampoline_code_table;
}
#endif
