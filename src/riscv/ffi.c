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

#सचे __riscv_float_abi_double
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
#सचे ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    वर्ण frame[16];
} call_context;

प्रकारनिर्देश संरचना call_builder
{
    call_context *aregs;
    पूर्णाङ्क used_integer;
    पूर्णाङ्क used_float;
    size_t *used_राशिः;
    शून्य *struct_राशिः;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#सचे __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अञ्ञथा
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#सचे ABI_FLEN
प्रकारनिर्देश संरचना {
    वर्ण as_elements, type1, offset2, type2;
} float_struct_info;

#सचे ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#अञ्ञथा
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

स्थिर ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    पूर्णाङ्क i;
    सचे (out == out_end) पटिनिवत्त out;
    सचे (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } अञ्ञथा {
        अत्थाय (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    पटिनिवत्त out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
स्थिर float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *शिखरम्) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    पूर्णाङ्क अङ्कः_floats, अङ्कः_ints;
    पूर्णाङ्क अङ्कः_fields = flatten_struct(शिखरम्, fields, fields + 3) - fields;

    सचे (अङ्कः_fields == 1) {
        सचे (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } अञ्ञथा सचे (अङ्कः_fields == 2) {
        अङ्कः_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        अङ्कः_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        सचे (अङ्कः_floats == 0 || अङ्कः_floats + अङ्कः_ints != 2)
            पटिनिवत्त ret;
        सचे (cb->used_float + अङ्कः_floats > NARGREG || cb->used_integer + (2 - अङ्कः_floats) > NARGREG)
            पटिनिवत्त ret;
        सचे (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            पटिनिवत्त ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    पटिनिवत्त ret;
}

#सचे ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
स्थिर शून्य marshal_float(call_builder *cb, शून्य *data) {
    सङ्घ {
        uint32_t i;
        भिन्न f;
    } मानम्;

    मानम्.f = *(भिन्न *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | मानम्.i;
}

स्थिर शून्य unmarshal_float(call_builder *cb, शून्य *data) {
    सङ्घ {
        uint32_t i;
        भिन्न f;
    } मानम्;

    मानम्.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(भिन्न *)data = मानम्.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
स्थिर शून्य marshal_atom(call_builder *cb, पूर्णाङ्क type, शून्य *data) {
    size_t मानम् = 0;
    विकल्प (type) {
        पक्ष FFI_TYPE_UINT8: मानम् = *(uint8_t *)data; भङ्ग;
        पक्ष FFI_TYPE_SINT8: मानम् = *(int8_t *)data; भङ्ग;
        पक्ष FFI_TYPE_UINT16: मानम् = *(uint16_t *)data; भङ्ग;
        पक्ष FFI_TYPE_SINT16: मानम् = *(int16_t *)data; भङ्ग;
        /* 32-bit quantities are always sign-extended in the ABI */
        पक्ष FFI_TYPE_UINT32: मानम् = *(int32_t *)data; भङ्ग;
        पक्ष FFI_TYPE_SINT32: मानम् = *(int32_t *)data; भङ्ग;
#सचे __SIZEOF_POINTER__ == 8
        पक्ष FFI_TYPE_UINT64: मानम् = *(uint64_t *)data; भङ्ग;
        पक्ष FFI_TYPE_SINT64: मानम् = *(int64_t *)data; भङ्ग;
#endif
        पक्ष FFI_TYPE_POINTER: मानम् = *(size_t *)data; भङ्ग;

#सचे ABI_FLEN >= 64
        पक्ष FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            पटिनिवत्त;
#नोचेद्यदि ABI_FLEN >= 32
        पक्ष FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(भिन्न *)data));
            पटिनिवत्त;
#endif
#सचे ABI_FLEN >= 64
        पक्ष FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(द्विगुण *)data));
            पटिनिवत्त;
#endif
        स्वभाव: FFI_ASSERT(0); भङ्ग;
    }

    सचे (cb->used_integer == NARGREG) {
        *cb->used_राशिः++ = मानम्;
    } अञ्ञथा {
        cb->aregs->a[cb->used_integer++] = मानम्;
    }
}

स्थिर शून्य unmarshal_atom(call_builder *cb, पूर्णाङ्क type, शून्य *data) {
    size_t मानम्;
    विकल्प (type) {
#सचे ABI_FLEN >= 64
        पक्ष FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            पटिनिवत्त;
#नोचेद्यदि ABI_FLEN >= 32
        पक्ष FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(भिन्न *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            पटिनिवत्त;
#endif
#सचे ABI_FLEN >= 64
        पक्ष FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(द्विगुण *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            पटिनिवत्त;
#endif
    }

    सचे (cb->used_integer == NARGREG) {
        मानम् = *cb->used_राशिः++;
    } अञ्ञथा {
        मानम् = cb->aregs->a[cb->used_integer++];
    }

    विकल्प (type) {
        पक्ष FFI_TYPE_UINT8: *(uint8_t *)data = मानम्; भङ्ग;
        पक्ष FFI_TYPE_SINT8: *(uint8_t *)data = मानम्; भङ्ग;
        पक्ष FFI_TYPE_UINT16: *(uint16_t *)data = मानम्; भङ्ग;
        पक्ष FFI_TYPE_SINT16: *(uint16_t *)data = मानम्; भङ्ग;
        पक्ष FFI_TYPE_UINT32: *(uint32_t *)data = मानम्; भङ्ग;
        पक्ष FFI_TYPE_SINT32: *(uint32_t *)data = मानम्; भङ्ग;
#सचे __SIZEOF_POINTER__ == 8
        पक्ष FFI_TYPE_UINT64: *(uint64_t *)data = मानम्; भङ्ग;
        पक्ष FFI_TYPE_SINT64: *(uint64_t *)data = मानम्; भङ्ग;
#endif
        पक्ष FFI_TYPE_POINTER: *(size_t *)data = मानम्; भङ्ग;
        स्वभाव: FFI_ASSERT(0); भङ्ग;
    }
}

/* adds an argument to a call, or a not by reference return value */
स्थिर शून्य marshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *data) {
    size_t realign[2];

#सचे ABI_FLEN
    सचे (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        सचे (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            सचे (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((वर्ण*)data) + fsi.offset2);
            पटिनिवत्त;
        }
    }

    सचे (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        पटिनिवत्त;
    }
#endif

    सचे (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = memcpy (cb->struct_राशिः, data, type->size);
        cb->struct_राशिः = (size_t *) FFI_ALIGN ((वर्ण *) cb->struct_राशिः + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } अञ्ञथा सचे (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } अञ्ञथा {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        सचे (type->alignment > __SIZEOF_POINTER__) {
            सचे (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_राशिः = (size_t *)FFI_ALIGN(cb->used_राशिः, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, data, type->size);
        सचे (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        सचे (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
स्थिर शून्य *unmarshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *data) {
    size_t realign[2];
    शून्य *pointer;

#सचे ABI_FLEN
    सचे (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        सचे (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            सचे (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((वर्ण*)data) + fsi.offset2);
            पटिनिवत्त data;
        }
    }

    सचे (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        पटिनिवत्त data;
    }
#endif

    सचे (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (वर्ण*)&pointer);
        पटिनिवत्त pointer;
    } अञ्ञथा सचे (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        पटिनिवत्त data;
    } अञ्ञथा {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        सचे (type->alignment > __SIZEOF_POINTER__) {
            सचे (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_राशिः = (size_t *)FFI_ALIGN(cb->used_राशिः, 2*__SIZEOF_POINTER__);
        }

        सचे (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        सचे (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(data, realign, type->size);
        पटिनिवत्त data;
    }
}

स्थिर पूर्णाङ्क passed_by_ref(call_builder *cb, ffi_type *type, पूर्णाङ्क var) {
#सचे ABI_FLEN
    सचे (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        सचे (fsi.as_elements) पटिनिवत्त 0;
    }
#endif

    पटिनिवत्त type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    पटिनिवत्त FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned पूर्णाङ्क nfixedargs, unsigned पूर्णाङ्क ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    पटिनिवत्त FFI_OK;
}

/* Low level routine for calling functions */
बाह्य शून्य ffi_call_asm (शून्य *राशिः, संरचना call_context *regs,
			  शून्य (*fn) (शून्य), शून्य *closure) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	      शून्य *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * परिमाण(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    सचे (rvalue == रिक्त && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    size_t alloc_size = arg_bytes + rval_bytes + struct_bytes + परिमाण(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    सचे (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_size);
    } अञ्ञथा {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    सचे (rval_bytes)
        rvalue = (शून्य*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_राशिः = (शून्य*)alloc_base;
    cb.struct_राशिः = (शून्य *) (alloc_base + arg_bytes + rval_bytes);

    पूर्णाङ्क return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    सचे (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    पूर्णाङ्क i;
    अत्थाय (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((शून्य *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    सचे (!return_by_ref && rvalue)
      {
	सचे (IS_INT(cif->rtype->type)
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
	अञ्ञथा
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
    सचे (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        पटिनिवत्त FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  सचे (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      शून्य (*dest)(शून्य) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  अञ्ञथा
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#सचे __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#अञ्ञथा
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#सचे !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    पटिनिवत्त FFI_OK;
}

बाह्य शून्य ffi_go_closure_asm (शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  सचे (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    पटिनिवत्त FFI_BAD_ABI;

  closure->tramp = (शून्य *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  पटिनिवत्त FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
शून्य FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		   शून्य *user_data,
		   size_t *राशिः, call_context *aregs)
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
    cb.used_राशिः = राशिः;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    सचे (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    अञ्ञथा
        rvalue = alloca(cif->rtype->size);

    अत्थाय (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    सचे (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
शून्य *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  बाह्य शून्य *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  पटिनिवत्त &trampoline_code_table;
}
#endif
