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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव "internal.h"
#अन्तर्भाव <tramp.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>

#यदि __riscv_float_abi_double
#define ABI_FLEN 64
प्रकारनिर्देश सङ्घ {
    uint64_t i;
    द्विगुण d;
} float_reg;
#नोचेद्यदि __riscv_float_abi_single
#define ABI_FLEN 32
प्रकारनिर्देश सङ्घ {
    भिन्न f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * परिमाण(द्विगुण))

प्रकारनिर्देश संरचना call_context
{
#यदि ABI_FLEN
    float_reg fa[8];
#endif
    माप_प्रकार a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    वर्ण frame[16];
} call_context;

प्रकारनिर्देश संरचना call_builder
{
    call_context *aregs;
    पूर्णाङ्क used_integer;
    पूर्णाङ्क used_float;
    माप_प्रकार *used_ढेर;
    शून्य *struct_ढेर;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#यदि __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अन्यथा
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#यदि ABI_FLEN
प्रकारनिर्देश संरचना {
    वर्ण as_elements, type1, offset2, type2;
} float_struct_info;

#यदि ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#अन्यथा
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

स्थिर ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    पूर्णाङ्क i;
    यदि (out == out_end) प्रत्यागम out;
    यदि (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } अन्यथा {
        प्रति (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    प्रत्यागम out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
स्थिर float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *शीर्ष) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    पूर्णाङ्क अंक_floats, अंक_ints;
    पूर्णाङ्क अंक_fields = flatten_struct(शीर्ष, fields, fields + 3) - fields;

    यदि (अंक_fields == 1) {
        यदि (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } अन्यथा यदि (अंक_fields == 2) {
        अंक_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        अंक_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        यदि (अंक_floats == 0 || अंक_floats + अंक_ints != 2)
            प्रत्यागम ret;
        यदि (cb->used_float + अंक_floats > NARGREG || cb->used_integer + (2 - अंक_floats) > NARGREG)
            प्रत्यागम ret;
        यदि (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            प्रत्यागम ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    प्रत्यागम ret;
}

#यदि ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
स्थिर शून्य marshal_float(call_builder *cb, शून्य *data) {
    सङ्घ {
        uint32_t i;
        भिन्न f;
    } मूल्य;

    मूल्य.f = *(भिन्न *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | मूल्य.i;
}

स्थिर शून्य unmarshal_float(call_builder *cb, शून्य *data) {
    सङ्घ {
        uint32_t i;
        भिन्न f;
    } मूल्य;

    मूल्य.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(भिन्न *)data = मूल्य.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
स्थिर शून्य marshal_atom(call_builder *cb, पूर्णाङ्क type, शून्य *data) {
    माप_प्रकार मूल्य = 0;
    विकल्प (type) {
        पक्ष FFI_TYPE_UINT8: मूल्य = *(uint8_t *)data; भङ्ग;
        पक्ष FFI_TYPE_SINT8: मूल्य = *(int8_t *)data; भङ्ग;
        पक्ष FFI_TYPE_UINT16: मूल्य = *(uint16_t *)data; भङ्ग;
        पक्ष FFI_TYPE_SINT16: मूल्य = *(int16_t *)data; भङ्ग;
        /* 32-bit quantities are always sign-extended in the ABI */
        पक्ष FFI_TYPE_UINT32: मूल्य = *(int32_t *)data; भङ्ग;
        पक्ष FFI_TYPE_SINT32: मूल्य = *(int32_t *)data; भङ्ग;
#यदि __SIZEOF_POINTER__ == 8
        पक्ष FFI_TYPE_UINT64: मूल्य = *(uint64_t *)data; भङ्ग;
        पक्ष FFI_TYPE_SINT64: मूल्य = *(int64_t *)data; भङ्ग;
#endif
        पक्ष FFI_TYPE_POINTER: मूल्य = *(माप_प्रकार *)data; भङ्ग;

#यदि ABI_FLEN >= 64
        पक्ष FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            प्रत्यागम;
#नोचेद्यदि ABI_FLEN >= 32
        पक्ष FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(भिन्न *)data));
            प्रत्यागम;
#endif
#यदि ABI_FLEN >= 64
        पक्ष FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(द्विगुण *)data));
            प्रत्यागम;
#endif
        स्वभाव: FFI_ASSERT(0); भङ्ग;
    }

    यदि (cb->used_integer == NARGREG) {
        *cb->used_ढेर++ = मूल्य;
    } अन्यथा {
        cb->aregs->a[cb->used_integer++] = मूल्य;
    }
}

स्थिर शून्य unmarshal_atom(call_builder *cb, पूर्णाङ्क type, शून्य *data) {
    माप_प्रकार मूल्य;
    विकल्प (type) {
#यदि ABI_FLEN >= 64
        पक्ष FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            प्रत्यागम;
#नोचेद्यदि ABI_FLEN >= 32
        पक्ष FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(भिन्न *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            प्रत्यागम;
#endif
#यदि ABI_FLEN >= 64
        पक्ष FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(द्विगुण *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            प्रत्यागम;
#endif
    }

    यदि (cb->used_integer == NARGREG) {
        मूल्य = *cb->used_ढेर++;
    } अन्यथा {
        मूल्य = cb->aregs->a[cb->used_integer++];
    }

    विकल्प (type) {
        पक्ष FFI_TYPE_UINT8: *(uint8_t *)data = मूल्य; भङ्ग;
        पक्ष FFI_TYPE_SINT8: *(uint8_t *)data = मूल्य; भङ्ग;
        पक्ष FFI_TYPE_UINT16: *(uint16_t *)data = मूल्य; भङ्ग;
        पक्ष FFI_TYPE_SINT16: *(uint16_t *)data = मूल्य; भङ्ग;
        पक्ष FFI_TYPE_UINT32: *(uint32_t *)data = मूल्य; भङ्ग;
        पक्ष FFI_TYPE_SINT32: *(uint32_t *)data = मूल्य; भङ्ग;
#यदि __SIZEOF_POINTER__ == 8
        पक्ष FFI_TYPE_UINT64: *(uint64_t *)data = मूल्य; भङ्ग;
        पक्ष FFI_TYPE_SINT64: *(uint64_t *)data = मूल्य; भङ्ग;
#endif
        पक्ष FFI_TYPE_POINTER: *(माप_प्रकार *)data = मूल्य; भङ्ग;
        स्वभाव: FFI_ASSERT(0); भङ्ग;
    }
}

/* adds an argument to a call, or a not by reference return value */
स्थिर शून्य marshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *data) {
    माप_प्रकार realign[2];

#यदि ABI_FLEN
    यदि (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        यदि (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            यदि (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((वर्ण*)data) + fsi.offset2);
            प्रत्यागम;
        }
    }

    यदि (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        प्रत्यागम;
    }
#endif

    यदि (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = स_नकल (cb->struct_ढेर, data, type->size);
        cb->struct_ढेर = (माप_प्रकार *) FFI_ALIGN ((वर्ण *) cb->struct_ढेर + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } अन्यथा यदि (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } अन्यथा {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        यदि (type->alignment > __SIZEOF_POINTER__) {
            यदि (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_ढेर = (माप_प्रकार *)FFI_ALIGN(cb->used_ढेर, 2*__SIZEOF_POINTER__);
        }

        स_नकल(realign, data, type->size);
        यदि (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        यदि (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
स्थिर शून्य *unmarshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *data) {
    माप_प्रकार realign[2];
    शून्य *pointer;

#यदि ABI_FLEN
    यदि (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        यदि (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            यदि (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((वर्ण*)data) + fsi.offset2);
            प्रत्यागम data;
        }
    }

    यदि (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        प्रत्यागम data;
    }
#endif

    यदि (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (वर्ण*)&pointer);
        प्रत्यागम pointer;
    } अन्यथा यदि (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        प्रत्यागम data;
    } अन्यथा {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        यदि (type->alignment > __SIZEOF_POINTER__) {
            यदि (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_ढेर = (माप_प्रकार *)FFI_ALIGN(cb->used_ढेर, 2*__SIZEOF_POINTER__);
        }

        यदि (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        यदि (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        स_नकल(data, realign, type->size);
        प्रत्यागम data;
    }
}

स्थिर पूर्णाङ्क passed_by_ref(call_builder *cb, ffi_type *type, पूर्णाङ्क var) {
#यदि ABI_FLEN
    यदि (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        यदि (fsi.as_elements) प्रत्यागम 0;
    }
#endif

    प्रत्यागम type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    प्रत्यागम FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, अचिन्हित पूर्णाङ्क nfixedargs, अचिन्हित पूर्णाङ्क ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    प्रत्यागम FFI_OK;
}

/* Low level routine for calling functions */
बाह्य शून्य ffi_call_asm (शून्य *ढेर, संरचना call_context *regs,
			  शून्य (*fn) (शून्य), शून्य *closure) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	      शून्य *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    माप_प्रकार arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * परिमाण(माप_प्रकार) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    माप_प्रकार struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    माप_प्रकार rval_bytes = 0;
    यदि (rvalue == रिक्त && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    माप_प्रकार alloc_size = arg_bytes + rval_bytes + struct_bytes + परिमाण(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    माप_प्रकार alloc_base;
    /* Argument region must be 16-byte aligned */
    यदि (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (माप_प्रकार)alloca(alloc_size);
    } अन्यथा {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    यदि (rval_bytes)
        rvalue = (शून्य*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_ढेर = (शून्य*)alloc_base;
    cb.struct_ढेर = (शून्य *) (alloc_base + arg_bytes + rval_bytes);

    पूर्णाङ्क return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    यदि (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    पूर्णाङ्क i;
    प्रति (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((शून्य *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    यदि (!return_by_ref && rvalue)
      {
	यदि (IS_INT(cif->rtype->type)
	    && cif->rtype->size < परिमाण (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    विकल्प (cif->rtype->type)
	      {
	      पक्ष FFI_TYPE_SINT8:
	      पक्ष FFI_TYPE_SINT16:
	      पक्ष FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (परिमाण (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		भङ्ग;
	      पक्ष FFI_TYPE_UINT8:
	      पक्ष FFI_TYPE_UINT16:
	      पक्ष FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (परिमाण (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		भङ्ग;
	      }
	  }
	अन्यथा
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

बाह्य शून्य ffi_closure_asm(शून्य) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*), शून्य *user_data, शून्य *codeloc)
{
    यदि (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        प्रत्यागम FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  यदि (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      शून्य (*dest)(शून्य) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  अन्यथा
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#यदि __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#अन्यथा
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#यदि !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_go_closure_asm (शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  यदि (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    प्रत्यागम FFI_BAD_ABI;

  closure->tramp = (शून्य *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  प्रत्यागम FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
शून्य FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		   शून्य *user_data,
		   माप_प्रकार *ढेर, call_context *aregs)
{
    शून्य **avalue = alloca(cif->nargs * परिमाण(शून्य*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    वर्ण *astorage = alloca(cif->nargs * MAXCOPYARG);
    शून्य *rvalue;
    call_builder cb;
    पूर्णाङ्क return_by_ref;
    पूर्णाङ्क i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_ढेर = ढेर;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    यदि (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    अन्यथा
        rvalue = alloca(cif->rtype->size);

    प्रति (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    यदि (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
शून्य *
ffi_tramp_arch (माप_प्रकार *tramp_size, माप_प्रकार *map_size)
{
  बाह्य शून्य *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  प्रत्यागम &trampoline_code_table;
}
#endif
