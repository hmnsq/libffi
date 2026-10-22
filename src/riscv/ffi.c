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

#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश "internal.h"
#समावेश <tramp.h>

#समावेश <stdlib.h>
#समावेश <stdint.h>

#अगर __riscv_float_abi_double
#घोषणा ABI_FLEN 64
प्रकार जोड़ {
    uint64_t i;
    द्विगुन d;
} float_reg;
#या_अगर __riscv_float_abi_single
#घोषणा ABI_FLEN 32
प्रकार जोड़ {
    भग्न f;
} float_reg;
#पूर्ण_अगर

#घोषणा NARGREG 8
#घोषणा STKALIGN 16
#घोषणा MAXCOPYARG (2 * माप(द्विगुन))

प्रकार काष्ठा call_context
{
#अगर ABI_FLEN
    float_reg fa[8];
#पूर्ण_अगर
    माप_प्रकार a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    अक्षर frame[16];
} call_context;

प्रकार काष्ठा call_builder
{
    call_context *aregs;
    पूर्णांक used_integer;
    पूर्णांक used_float;
    माप_प्रकार *used_ढेर;
    व्योम *struct_ढेर;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#अगर __SIZEOF_POINTER__ == 8
#घोषणा IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अन्यथा
#घोषणा IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#पूर्ण_अगर

#अगर ABI_FLEN
प्रकार काष्ठा {
    अक्षर as_elements, type1, offset2, type2;
} float_struct_info;

#अगर ABI_FLEN >= 64
#घोषणा IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#अन्यथा
#घोषणा IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#पूर्ण_अगर

अटल ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    पूर्णांक i;
    अगर (out == out_end) वापस out;
    अगर (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } अन्यथा {
        क्रम (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    वापस out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
अटल float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *शीर्ष_2) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    पूर्णांक अंक_floats, अंक_ints;
    पूर्णांक अंक_fields = flatten_struct(शीर्ष_2, fields, fields + 3) - fields;

    अगर (अंक_fields == 1) {
        अगर (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } अन्यथा अगर (अंक_fields == 2) {
        अंक_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        अंक_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        अगर (अंक_floats == 0 || अंक_floats + अंक_ints != 2)
            वापस ret;
        अगर (cb->used_float + अंक_floats > NARGREG || cb->used_integer + (2 - अंक_floats) > NARGREG)
            वापस ret;
        अगर (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            वापस ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->आकार, fields[1]->alignment);
        ret.as_elements = 1;
    }

    वापस ret;
}

#अगर ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
अटल व्योम marshal_float(call_builder *cb, व्योम *तथ्य) {
    जोड़ {
        uint32_t i;
        भग्न f;
    } मूल्य;

    मूल्य.f = *(भग्न *)तथ्य;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | मूल्य.i;
}

अटल व्योम unmarshal_float(call_builder *cb, व्योम *तथ्य) {
    जोड़ {
        uint32_t i;
        भग्न f;
    } मूल्य;

    मूल्य.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(भग्न *)तथ्य = मूल्य.f;
}
#पूर्ण_अगर
#पूर्ण_अगर

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
अटल व्योम marshal_atom(call_builder *cb, पूर्णांक type, व्योम *तथ्य) {
    माप_प्रकार मूल्य = 0;
    चयन (type) {
        हाल FFI_TYPE_UINT8: मूल्य = *(uint8_t *)तथ्य; अवरोध;
        हाल FFI_TYPE_SINT8: मूल्य = *(int8_t *)तथ्य; अवरोध;
        हाल FFI_TYPE_UINT16: मूल्य = *(uint16_t *)तथ्य; अवरोध;
        हाल FFI_TYPE_SINT16: मूल्य = *(int16_t *)तथ्य; अवरोध;
        /* 32-bit quantities are always sign-extended in the ABI */
        हाल FFI_TYPE_UINT32: मूल्य = *(int32_t *)तथ्य; अवरोध;
        हाल FFI_TYPE_SINT32: मूल्य = *(int32_t *)तथ्य; अवरोध;
#अगर __SIZEOF_POINTER__ == 8
        हाल FFI_TYPE_UINT64: मूल्य = *(uint64_t *)तथ्य; अवरोध;
        हाल FFI_TYPE_SINT64: मूल्य = *(int64_t *)तथ्य; अवरोध;
#पूर्ण_अगर
        हाल FFI_TYPE_POINTER: मूल्य = *(माप_प्रकार *)तथ्य; अवरोध;

#अगर ABI_FLEN >= 64
        हाल FFI_TYPE_FLOAT:
            marshal_float(cb, तथ्य);
            वापस;
#या_अगर ABI_FLEN >= 32
        हाल FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(भग्न *)तथ्य));
            वापस;
#पूर्ण_अगर
#अगर ABI_FLEN >= 64
        हाल FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(द्विगुन *)तथ्य));
            वापस;
#पूर्ण_अगर
        शेष: FFI_ASSERT(0); अवरोध;
    }

    अगर (cb->used_integer == NARGREG) {
        *cb->used_ढेर++ = मूल्य;
    } अन्यथा {
        cb->aregs->a[cb->used_integer++] = मूल्य;
    }
}

अटल व्योम unmarshal_atom(call_builder *cb, पूर्णांक type, व्योम *तथ्य) {
    माप_प्रकार मूल्य;
    चयन (type) {
#अगर ABI_FLEN >= 64
        हाल FFI_TYPE_FLOAT:
            unmarshal_float(cb, तथ्य);
            वापस;
#या_अगर ABI_FLEN >= 32
        हाल FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(भग्न *)तथ्य) : "0"(cb->aregs->fa[cb->used_float++].f));
            वापस;
#पूर्ण_अगर
#अगर ABI_FLEN >= 64
        हाल FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(द्विगुन *)तथ्य) : "0"(cb->aregs->fa[cb->used_float++].d));
            वापस;
#पूर्ण_अगर
    }

    अगर (cb->used_integer == NARGREG) {
        मूल्य = *cb->used_ढेर++;
    } अन्यथा {
        मूल्य = cb->aregs->a[cb->used_integer++];
    }

    चयन (type) {
        हाल FFI_TYPE_UINT8: *(uint8_t *)तथ्य = मूल्य; अवरोध;
        हाल FFI_TYPE_SINT8: *(uint8_t *)तथ्य = मूल्य; अवरोध;
        हाल FFI_TYPE_UINT16: *(uint16_t *)तथ्य = मूल्य; अवरोध;
        हाल FFI_TYPE_SINT16: *(uint16_t *)तथ्य = मूल्य; अवरोध;
        हाल FFI_TYPE_UINT32: *(uint32_t *)तथ्य = मूल्य; अवरोध;
        हाल FFI_TYPE_SINT32: *(uint32_t *)तथ्य = मूल्य; अवरोध;
#अगर __SIZEOF_POINTER__ == 8
        हाल FFI_TYPE_UINT64: *(uint64_t *)तथ्य = मूल्य; अवरोध;
        हाल FFI_TYPE_SINT64: *(uint64_t *)तथ्य = मूल्य; अवरोध;
#पूर्ण_अगर
        हाल FFI_TYPE_POINTER: *(माप_प्रकार *)तथ्य = मूल्य; अवरोध;
        शेष: FFI_ASSERT(0); अवरोध;
    }
}

/* adds an argument to a call, or a not by reference return value */
अटल व्योम marshal(call_builder *cb, ffi_type *type, पूर्णांक var, व्योम *तथ्य) {
    माप_प्रकार realign[2];

#अगर ABI_FLEN
    अगर (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        अगर (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, तथ्य);
            अगर (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((अक्षर*)तथ्य) + fsi.offset2);
            वापस;
        }
    }

    अगर (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, तथ्य);
        वापस;
    }
#पूर्ण_अगर

    अगर (type->आकार > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        तथ्य = स_नकल (cb->struct_ढेर, तथ्य, type->आकार);
        cb->struct_ढेर = (माप_प्रकार *) FFI_ALIGN ((अक्षर *) cb->struct_ढेर + type->आकार, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &तथ्य);
    } अन्यथा अगर (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, तथ्य);
    } अन्यथा {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        अगर (type->alignment > __SIZEOF_POINTER__) {
            अगर (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_ढेर = (माप_प्रकार *)FFI_ALIGN(cb->used_ढेर, 2*__SIZEOF_POINTER__);
        }

        स_नकल(realign, तथ्य, type->आकार);
        अगर (type->आकार > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        अगर (type->आकार > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
अटल व्योम *unmarshal(call_builder *cb, ffi_type *type, पूर्णांक var, व्योम *तथ्य) {
    माप_प्रकार realign[2];
    व्योम *pointer;

#अगर ABI_FLEN
    अगर (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        अगर (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, तथ्य);
            अगर (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((अक्षर*)तथ्य) + fsi.offset2);
            वापस तथ्य;
        }
    }

    अगर (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, तथ्य);
        वापस तथ्य;
    }
#पूर्ण_अगर

    अगर (type->आकार > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (अक्षर*)&pointer);
        वापस pointer;
    } अन्यथा अगर (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, तथ्य);
        वापस तथ्य;
    } अन्यथा {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        अगर (type->alignment > __SIZEOF_POINTER__) {
            अगर (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_ढेर = (माप_प्रकार *)FFI_ALIGN(cb->used_ढेर, 2*__SIZEOF_POINTER__);
        }

        अगर (type->आकार > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        अगर (type->आकार > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        स_नकल(तथ्य, realign, type->आकार);
        वापस तथ्य;
    }
}

अटल पूर्णांक passed_by_ref(call_builder *cb, ffi_type *type, पूर्णांक var) {
#अगर ABI_FLEN
    अगर (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        अगर (fsi.as_elements) वापस 0;
    }
#पूर्ण_अगर

    वापस type->आकार > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    वापस FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, अचिन्हित पूर्णांक nfixedargs, अचिन्हित पूर्णांक ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    वापस FFI_OK;
}

/* Low level routine for calling functions */
बाह्य व्योम ffi_call_asm (व्योम *ढेर, काष्ठा call_context *regs,
			  व्योम (*fn) (व्योम), व्योम *closure) FFI_HIDDEN;

अटल व्योम
ffi_call_int (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue, व्योम **avalue,
	      व्योम *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    माप_प्रकार arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * माप(माप_प्रकार) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    माप_प्रकार struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    माप_प्रकार rval_bytes = 0;
    अगर (rvalue == शून्य && cif->rtype->आकार > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->आकार, STKALIGN);
    माप_प्रकार alloc_आकार = arg_bytes + rval_bytes + struct_bytes + माप(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    माप_प्रकार alloc_base;
    /* Argument region must be 16-byte aligned */
    अगर (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (माप_प्रकार)alloca(alloc_आकार);
    } अन्यथा {
        alloc_base = FFI_ALIGN(alloca(alloc_आकार + STKALIGN - 1), STKALIGN);
    }

    अगर (rval_bytes)
        rvalue = (व्योम*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_ढेर = (व्योम*)alloc_base;
    cb.struct_ढेर = (व्योम *) (alloc_base + arg_bytes + rval_bytes);

    पूर्णांक return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    अगर (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    पूर्णांक i;
    क्रम (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((व्योम *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    अगर (!return_by_ref && rvalue)
      {
	अगर (IS_INT(cif->rtype->type)
	    && cif->rtype->आकार < माप (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    चयन (cif->rtype->type)
	      {
	      हाल FFI_TYPE_SINT8:
	      हाल FFI_TYPE_SINT16:
	      हाल FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (माप (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		अवरोध;
	      हाल FFI_TYPE_UINT8:
	      हाल FFI_TYPE_UINT16:
	      हाल FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (माप (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		अवरोध;
	      }
	  }
	अन्यथा
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

व्योम
ffi_call (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue, व्योम **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, शून्य);
}

व्योम
ffi_call_go (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue,
	     व्योम **avalue, व्योम *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

बाह्य व्योम ffi_closure_asm(व्योम) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*), व्योम *user_तथ्य, व्योम *codeloc)
{
    अगर (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        वापस FFI_BAD_ABI;

#अगर_घोषित FFI_EXEC_STATIC_TRAMP
  अगर (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      व्योम (*dest)(व्योम) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  अन्यथा
#पूर्ण_अगर
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#अगर __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#अन्यथा
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#पूर्ण_अगर
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#अगर !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_आकार);
#पूर्ण_अगर
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_तथ्य = user_तथ्य;

    वापस FFI_OK;
}

बाह्य व्योम ffi_go_closure_asm (व्योम) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *))
{
  अगर (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    वापस FFI_BAD_ABI;

  closure->tramp = (व्योम *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  वापस FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
व्योम FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
		   व्योम *user_तथ्य,
		   माप_प्रकार *ढेर, call_context *aregs)
{
    व्योम **avalue = alloca(cif->nargs * माप(व्योम*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    अक्षर *astorage = alloca(cif->nargs * MAXCOPYARG);
    व्योम *rvalue;
    call_builder cb;
    पूर्णांक return_by_ref;
    पूर्णांक i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_ढेर = ढेर;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    अगर (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    अन्यथा
        rvalue = alloca(cif->rtype->आकार);

    क्रम (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_तथ्य);

    अगर (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#अगर_घोषित FFI_EXEC_STATIC_TRAMP
व्योम *
ffi_tramp_arch (माप_प्रकार *tramp_आकार, माप_प्रकार *map_आकार)
{
  बाह्य व्योम *trampoline_code_table;
  *tramp_आकार = RISCV_TRAMP_आकार;
  *map_आकार = RISCV_TRAMP_MAP_आकार;
  वापस &trampoline_code_table;
}
#पूर्ण_अगर
