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
#परिभाषय ABI_FLEN 64
प्रकारनिर्देश सङ्घ {
    uint64_t i;
    द्विगुण d;
} float_reg;
#नोचेद्यदि __riscv_float_abi_single
#परिभाषय ABI_FLEN 32
प्रकारनिर्देश सङ्घ {
    भिन्न f;
} float_reg;
#यदि_अन्तः

#परिभाषय NARGREG 8
#परिभाषय STKALIGN 16
#परिभाषय MAXCOPYARG (2 * परिमाण(द्विगुण))

प्रकारनिर्देश संरचना call_context
{
#यदि ABI_FLEN
    float_reg fa[8];
#यदि_अन्तः
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    वर्ण frame[16];
} call_context;

प्रकारनिर्देश संरचना call_builder
{
    call_context *aregs;
    पूर्णाङ्क प्रयुक्तम्_integer;
    पूर्णाङ्क प्रयुक्तम्_float;
    size_t *प्रयुक्तम्_राशिः;
    शून्य *struct_राशिः;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#यदि __SIZEOF_POINTER__ == 8
#परिभाषय IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अन्यथा
#परिभाषय IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#यदि_अन्तः

#यदि ABI_FLEN
प्रकारनिर्देश संरचना {
    वर्ण as_elements, type1, offset2, type2;
} float_struct_info;

#यदि ABI_FLEN >= 64
#परिभाषय IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#अन्यथा
#परिभाषय IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#यदि_अन्तः

स्थिर ffi_type **flatten_struct(ffi_type *अन्तर्, ffi_type **बहिर्गमनम्, ffi_type **बहिर्गमनम्_अन्तः) {
    पूर्णाङ्क i;
    यदि (बहिर्गमनम् == बहिर्गमनम्_अन्तः) प्रत्यागम बहिर्गमनम्;
    यदि (अन्तर्->type != FFI_TYPE_STRUCT) {
        *(बहिर्गमनम्++) = अन्तर्;
    } अन्यथा {
        प्रति (i = 0; अन्तर्->elements[i]; i++)
            बहिर्गमनम् = flatten_struct(अन्तर्->elements[i], बहिर्गमनम्, बहिर्गमनम्_अन्तः);
    }
    प्रत्यागम बहिर्गमनम्;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
स्थिर float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *शिखरम्) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    पूर्णाङ्क अङ्कः_floats, अङ्कः_ints;
    पूर्णाङ्क अङ्कः_fields = flatten_struct(शिखरम्, fields, fields + 3) - fields;

    यदि (अङ्कः_fields == 1) {
        यदि (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } अन्यथा यदि (अङ्कः_fields == 2) {
        अङ्कः_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        अङ्कः_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        यदि (अङ्कः_floats == 0 || अङ्कः_floats + अङ्कः_ints != 2)
            प्रत्यागम ret;
        यदि (cb->प्रयुक्तम्_float + अङ्कः_floats > NARGREG || cb->प्रयुक्तम्_integer + (2 - अङ्कः_floats) > NARGREG)
            प्रत्यागम ret;
        यदि (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            प्रत्यागम ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->आकार, fields[1]->alignment);
        ret.as_elements = 1;
    }

    प्रत्यागम ret;
}

#यदि ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
स्थिर शून्य marshal_float(call_builder *cb, शून्य *दत्त) {
    सङ्घ {
        uint32_t i;
        भिन्न f;
    } मानम्;

    मानम्.f = *(भिन्न *)दत्त;
    cb->aregs->fa[cb->प्रयुक्तम्_float++].i =
        UINT64_C(0xffffffff00000000) | मानम्.i;
}

स्थिर शून्य unmarshal_float(call_builder *cb, शून्य *दत्त) {
    सङ्घ {
        uint32_t i;
        भिन्न f;
    } मानम्;

    मानम्.i = (uint32_t)cb->aregs->fa[cb->प्रयुक्तम्_float++].i;
    *(भिन्न *)दत्त = मानम्.f;
}
#यदि_अन्तः
#यदि_अन्तः

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
स्थिर शून्य marshal_atom(call_builder *cb, पूर्णाङ्क type, शून्य *दत्त) {
    size_t मानम् = 0;
    विकल्प (type) {
        पक्ष FFI_TYPE_UINT8: मानम् = *(uint8_t *)दत्त; भङ्ग;
        पक्ष FFI_TYPE_SINT8: मानम् = *(int8_t *)दत्त; भङ्ग;
        पक्ष FFI_TYPE_UINT16: मानम् = *(uint16_t *)दत्त; भङ्ग;
        पक्ष FFI_TYPE_SINT16: मानम् = *(int16_t *)दत्त; भङ्ग;
        /* 32-bit quantities are always sign-extended in the ABI */
        पक्ष FFI_TYPE_UINT32: मानम् = *(int32_t *)दत्त; भङ्ग;
        पक्ष FFI_TYPE_SINT32: मानम् = *(int32_t *)दत्त; भङ्ग;
#यदि __SIZEOF_POINTER__ == 8
        पक्ष FFI_TYPE_UINT64: मानम् = *(uint64_t *)दत्त; भङ्ग;
        पक्ष FFI_TYPE_SINT64: मानम् = *(int64_t *)दत्त; भङ्ग;
#यदि_अन्तः
        पक्ष FFI_TYPE_POINTER: मानम् = *(size_t *)दत्त; भङ्ग;

#यदि ABI_FLEN >= 64
        पक्ष FFI_TYPE_FLOAT:
            marshal_float(cb, दत्त);
            प्रत्यागम;
#नोचेद्यदि ABI_FLEN >= 32
        पक्ष FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->प्रयुक्तम्_float++].f) : "0"(*(भिन्न *)दत्त));
            प्रत्यागम;
#यदि_अन्तः
#यदि ABI_FLEN >= 64
        पक्ष FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->प्रयुक्तम्_float++].d) : "0"(*(द्विगुण *)दत्त));
            प्रत्यागम;
#यदि_अन्तः
        स्वभाव: FFI_ASSERT(0); भङ्ग;
    }

    यदि (cb->प्रयुक्तम्_integer == NARGREG) {
        *cb->प्रयुक्तम्_राशिः++ = मानम्;
    } अन्यथा {
        cb->aregs->a[cb->प्रयुक्तम्_integer++] = मानम्;
    }
}

स्थिर शून्य unmarshal_atom(call_builder *cb, पूर्णाङ्क type, शून्य *दत्त) {
    size_t मानम्;
    विकल्प (type) {
#यदि ABI_FLEN >= 64
        पक्ष FFI_TYPE_FLOAT:
            unmarshal_float(cb, दत्त);
            प्रत्यागम;
#नोचेद्यदि ABI_FLEN >= 32
        पक्ष FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(भिन्न *)दत्त) : "0"(cb->aregs->fa[cb->प्रयुक्तम्_float++].f));
            प्रत्यागम;
#यदि_अन्तः
#यदि ABI_FLEN >= 64
        पक्ष FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(द्विगुण *)दत्त) : "0"(cb->aregs->fa[cb->प्रयुक्तम्_float++].d));
            प्रत्यागम;
#यदि_अन्तः
    }

    यदि (cb->प्रयुक्तम्_integer == NARGREG) {
        मानम् = *cb->प्रयुक्तम्_राशिः++;
    } अन्यथा {
        मानम् = cb->aregs->a[cb->प्रयुक्तम्_integer++];
    }

    विकल्प (type) {
        पक्ष FFI_TYPE_UINT8: *(uint8_t *)दत्त = मानम्; भङ्ग;
        पक्ष FFI_TYPE_SINT8: *(uint8_t *)दत्त = मानम्; भङ्ग;
        पक्ष FFI_TYPE_UINT16: *(uint16_t *)दत्त = मानम्; भङ्ग;
        पक्ष FFI_TYPE_SINT16: *(uint16_t *)दत्त = मानम्; भङ्ग;
        पक्ष FFI_TYPE_UINT32: *(uint32_t *)दत्त = मानम्; भङ्ग;
        पक्ष FFI_TYPE_SINT32: *(uint32_t *)दत्त = मानम्; भङ्ग;
#यदि __SIZEOF_POINTER__ == 8
        पक्ष FFI_TYPE_UINT64: *(uint64_t *)दत्त = मानम्; भङ्ग;
        पक्ष FFI_TYPE_SINT64: *(uint64_t *)दत्त = मानम्; भङ्ग;
#यदि_अन्तः
        पक्ष FFI_TYPE_POINTER: *(size_t *)दत्त = मानम्; भङ्ग;
        स्वभाव: FFI_ASSERT(0); भङ्ग;
    }
}

/* adds an argument to a call, or a not by reference return value */
स्थिर शून्य marshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *दत्त) {
    size_t realign[2];

#यदि ABI_FLEN
    यदि (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        यदि (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, दत्त);
            यदि (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((वर्ण*)दत्त) + fsi.offset2);
            प्रत्यागम;
        }
    }

    यदि (!var && cb->प्रयुक्तम्_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, दत्त);
        प्रत्यागम;
    }
#यदि_अन्तः

    यदि (type->आकार > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        दत्त = स्मृतिं_प्रतिलिख (cb->struct_राशिः, दत्त, type->आकार);
        cb->struct_राशिः = (size_t *) FFI_ALIGN ((वर्ण *) cb->struct_राशिः + type->आकार, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &दत्त);
    } अन्यथा यदि (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, दत्त);
    } अन्यथा {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        यदि (type->alignment > __SIZEOF_POINTER__) {
            यदि (var)
                cb->प्रयुक्तम्_integer = FFI_ALIGN(cb->प्रयुक्तम्_integer, 2);
            cb->प्रयुक्तम्_राशिः = (size_t *)FFI_ALIGN(cb->प्रयुक्तम्_राशिः, 2*__SIZEOF_POINTER__);
        }

        स्मृतिं_प्रतिलिख(realign, दत्त, type->आकार);
        यदि (type->आकार > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        यदि (type->आकार > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
स्थिर शून्य *unmarshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *दत्त) {
    size_t realign[2];
    शून्य *pointer;

#यदि ABI_FLEN
    यदि (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        यदि (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, दत्त);
            यदि (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((वर्ण*)दत्त) + fsi.offset2);
            प्रत्यागम दत्त;
        }
    }

    यदि (!var && cb->प्रयुक्तम्_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, दत्त);
        प्रत्यागम दत्त;
    }
#यदि_अन्तः

    यदि (type->आकार > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (वर्ण*)&pointer);
        प्रत्यागम pointer;
    } अन्यथा यदि (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, दत्त);
        प्रत्यागम दत्त;
    } अन्यथा {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        यदि (type->alignment > __SIZEOF_POINTER__) {
            यदि (var)
                cb->प्रयुक्तम्_integer = FFI_ALIGN(cb->प्रयुक्तम्_integer, 2);
            cb->प्रयुक्तम्_राशिः = (size_t *)FFI_ALIGN(cb->प्रयुक्तम्_राशिः, 2*__SIZEOF_POINTER__);
        }

        यदि (type->आकार > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        यदि (type->आकार > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        स्मृतिं_प्रतिलिख(दत्त, realign, type->आकार);
        प्रत्यागम दत्त;
    }
}

स्थिर पूर्णाङ्क passed_by_ref(call_builder *cb, ffi_type *type, पूर्णाङ्क var) {
#यदि ABI_FLEN
    यदि (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        यदि (fsi.as_elements) प्रत्यागम 0;
    }
#यदि_अन्तः

    प्रत्यागम type->आकार > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_स्थितिः ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    प्रत्यागम FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_स्थितिः ffi_prep_cif_machdep_var(ffi_cif *cif, अचिह्नित पूर्णाङ्क nfixedargs, अचिह्नित पूर्णाङ्क ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    प्रत्यागम FFI_OK;
}

/* Low level routine for calling functions */
बाह्य शून्य ffi_call_asm (शून्य *राशिः, संरचना call_context *regs,
			  शून्य (*fn) (शून्य), शून्य *संवृतिः) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	      शून्य *संवृतिः)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_अष्टकानि = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * परिमाण(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_अष्टकानि = FFI_ALIGN (cif->अष्टकानि, STKALIGN);
    size_t rval_अष्टकानि = 0;
    यदि (rvalue == रिक्त && cif->rtype->आकार > 2*__SIZEOF_POINTER__)
        rval_अष्टकानि = FFI_ALIGN(cif->rtype->आकार, STKALIGN);
    size_t alloc_आकार = arg_अष्टकानि + rval_अष्टकानि + struct_अष्टकानि + परिमाण(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_आधारः;
    /* Argument region must be 16-byte aligned */
    यदि (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_आधारः = (size_t)alloca(alloc_आकार);
    } अन्यथा {
        alloc_आधारः = FFI_ALIGN(alloca(alloc_आकार + STKALIGN - 1), STKALIGN);
    }

    यदि (rval_अष्टकानि)
        rvalue = (शून्य*)(alloc_आधारः + arg_अष्टकानि);

    call_builder cb;
    cb.प्रयुक्तम्_float = cb.प्रयुक्तम्_integer = 0;
    cb.aregs = (call_context*)(alloc_आधारः + arg_अष्टकानि + rval_अष्टकानि + struct_अष्टकानि);
    cb.प्रयुक्तम्_राशिः = (शून्य*)alloc_आधारः;
    cb.struct_राशिः = (शून्य *) (alloc_आधारः + arg_अष्टकानि + rval_अष्टकानि);

    पूर्णाङ्क return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    यदि (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    पूर्णाङ्क i;
    प्रति (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((शून्य *) alloc_आधारः, cb.aregs, fn, संवृतिः);

    cb.प्रयुक्तम्_float = cb.प्रयुक्तम्_integer = 0;
    यदि (!return_by_ref && rvalue)
      {
	यदि (IS_INT(cif->rtype->type)
	    && cif->rtype->आकार < परिमाण (ffi_arg))
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
	     शून्य **avalue, शून्य *संवृतिः)
{
  ffi_call_int(cif, fn, rvalue, avalue, संवृतिः);
}

बाह्य शून्य ffi_संवृतिः_asm(शून्य) FFI_HIDDEN;

ffi_स्थितिः ffi_prep_संवृतिः_loc(ffi_संवृतिः *संवृतिः, ffi_cif *cif, शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*), शून्य *user_दत्त, शून्य *codeloc)
{
    यदि (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_चरमः_ABI)
        प्रत्यागम FFI_BAD_ABI;

#यदि_परिभाषितम् FFI_EXEC_STATIC_TRAMP
  यदि (ffi_tramp_is_present (संवृतिः))
    {
      /* Initialize the static trampoline's parameters. */
      शून्य (*dest)(शून्य) = ffi_संवृतिः_asm;
      ffi_tramp_समुच्चयः_parms (संवृतिः->ftramp, dest, संवृतिः);
    }
  अन्यथा
#यदि_अन्तः
    {
      uint32_t *tramp = (uint32_t *) &संवृतिः->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_संवृतिः_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#यदि __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#अन्यथा
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#यदि_अन्तः
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#यदि !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_आकार);
#यदि_अन्तः
    }

    संवृतिः->cif = cif;
    संवृतिः->fun = fun;
    संवृतिः->user_दत्त = user_दत्त;

    प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_go_संवृतिः_asm (शून्य) FFI_HIDDEN;

ffi_स्थितिः
ffi_prep_go_संवृतिः (ffi_go_संवृतिः *संवृतिः, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  यदि (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_चरमः_ABI)
    प्रत्यागम FFI_BAD_ABI;

  संवृतिः->tramp = (शून्य *) ffi_go_संवृतिः_asm;
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;

  प्रत्यागम FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
शून्य FFI_HIDDEN
ffi_संवृतिः_inner (ffi_cif *cif,
		   शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		   शून्य *user_दत्त,
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
    cb.प्रयुक्तम्_integer = cb.प्रयुक्तम्_float = 0;
    cb.प्रयुक्तम्_राशिः = राशिः;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    यदि (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    अन्यथा
        rvalue = alloca(cif->rtype->आकार);

    प्रति (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_दत्त);

    यदि (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.प्रयुक्तम्_integer = cb.प्रयुक्तम्_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#यदि_परिभाषितम् FFI_EXEC_STATIC_TRAMP
शून्य *
ffi_tramp_arch (size_t *tramp_आकार, size_t *map_आकार)
{
  बाह्य शून्य *trampoline_संहिता_सारणी;
  *tramp_आकार = RISCV_TRAMP_आकार;
  *map_आकार = RISCV_TRAMP_MAP_आकार;
  प्रत्यागम &trampoline_संहिता_सारणी;
}
#यदि_अन्तः
