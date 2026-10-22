/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2013  Synopsys, Inc. (www.synopsys.com)
   
   ARC Foreign Function Interface 

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL RENESAS TECHNOLOGY BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>

#अन्तर्भाव <sys/cachectl.h>

#परिभाषय NARGREG 8
#परिभाषय STKALIGN 4
#परिभाषय MAXCOPYARG (2 * परिमाण(द्विगुण))

प्रकारनिर्देश संरचना call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  वर्ण frame[16];
} call_context;

प्रकारनिर्देश संरचना call_builder
{
  call_context *aregs;
  पूर्णाङ्क प्रयुक्तम्_integer;
  //int used_float;
  size_t *प्रयुक्तम्_राशिः;
  शून्य *struct_राशिः;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#यदि defined(__ARC64_ARCH64__)
#परिभाषय IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अन्यथा
#परिभाषय IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#यदि_अन्तः

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#यदि __BIG_ENDIAN__
#परिभाषय संहिता_ENDIAN(x) (x)
#अन्यथा
#परिभाषय संहिता_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#यदि_अन्तः

/* Perform machine dependent cif processing.  */
ffi_स्थितिः
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      cif->flags = (अचिह्नित) cif->rtype->type;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      cif->flags = (अचिह्नित) cif->rtype->type;
      भङ्ग;

    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      भङ्ग;

    पक्ष FFI_TYPE_FLOAT:
    स्वभाव:
      cif->flags = FFI_TYPE_INT;
      भङ्ग;
    }

  प्रत्यागम FFI_OK;
}

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
#यदि defined(__ARC64_ARCH64__)
    पक्ष FFI_TYPE_UINT64: मानम् = *(uint64_t *)दत्त; भङ्ग;
    पक्ष FFI_TYPE_SINT64: मानम् = *(int64_t *)दत्त; भङ्ग;
#यदि_अन्तः
    पक्ष FFI_TYPE_POINTER: मानम् = *(size_t *)दत्त; भङ्ग;
    स्वभाव: FFI_ASSERT(0); भङ्ग;
  }

  यदि (cb->प्रयुक्तम्_integer == NARGREG) {
    *cb->प्रयुक्तम्_राशिः++ = मानम्;
  } अन्यथा {
    cb->aregs->r[cb->प्रयुक्तम्_integer++] = मानम्;
  }
}

/* adds an argument to a call, or a not by reference return value */
स्थिर शून्य marshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *दत्त) {
  size_t realign[2];

#यदि (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  यदि (type->आकार > 2 * __SIZEOF_POINTER__) {
    यदि (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &दत्त);
    } अन्यथा {
      /* copy to stack and pass by reference */
      दत्त = स्मृतिं_प्रतिलिख (cb->struct_राशिः, दत्त, type->आकार);
      cb->struct_राशिः = (size_t *) FFI_ALIGN ((वर्ण *) cb->struct_राशिः + type->आकार, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &दत्त);
    }
  }
#अन्यथा
  यदि (type->type == FFI_TYPE_STRUCT) {
    यदि (var) {
      यदि (type->आकार > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, दत्त);
    } अन्यथा {
      पूर्णाङ्क i;
      
      प्रति (i = 0; i < type->आकार; i += परिमाण(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, दत्त);
        दत्त += परिमाण(size_t);
      }
    }
  }
#यदि_अन्तः
  अन्यथा यदि (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, दत्त);
  } अन्यथा {
      स्मृतिं_प्रतिलिख(realign, दत्त, type->आकार);
      यदि (type->आकार > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      यदि (type->आकार > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

स्थिर शून्य unmarshal_atom(call_builder *cb, पूर्णाङ्क type, शून्य *दत्त) {
  size_t मानम्;

  यदि (cb->प्रयुक्तम्_integer == NARGREG) {
    मानम् = *cb->प्रयुक्तम्_राशिः++;
  } अन्यथा {
    मानम् = cb->aregs->r[cb->प्रयुक्तम्_integer++];
  }

  विकल्प (type) {
    पक्ष FFI_TYPE_UINT8: *(uint8_t *)दत्त = मानम्; भङ्ग;
    पक्ष FFI_TYPE_SINT8: *(uint8_t *)दत्त = मानम्; भङ्ग;
    पक्ष FFI_TYPE_UINT16: *(uint16_t *)दत्त = मानम्; भङ्ग;
    पक्ष FFI_TYPE_SINT16: *(uint16_t *)दत्त = मानम्; भङ्ग;
    पक्ष FFI_TYPE_UINT32: *(uint32_t *)दत्त = मानम्; भङ्ग;
    पक्ष FFI_TYPE_SINT32: *(uint32_t *)दत्त = मानम्; भङ्ग;
#यदि defined(__ARC64_ARCH64__)
    पक्ष FFI_TYPE_UINT64: *(uint64_t *)दत्त = मानम्; भङ्ग;
    पक्ष FFI_TYPE_SINT64: *(uint64_t *)दत्त = मानम्; भङ्ग;
#यदि_अन्तः
    पक्ष FFI_TYPE_POINTER: *(size_t *)दत्त = मानम्; भङ्ग;
    स्वभाव: FFI_ASSERT(0); भङ्ग;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
स्थिर शून्य *unmarshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *दत्त) {
  size_t realign[2];

#यदि defined(__ARC64_ARCH64__)
  शून्य *pointer;

  यदि (type->आकार > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (वर्ण*)&pointer);
        प्रत्यागम pointer;
    }
#नोचेद्यदि defined(__ARC64_ARCH32__)
  यदि (type->type == FFI_TYPE_STRUCT) {
    यदि (type->आकार > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      स्मृतिं_प्रतिलिख(दत्त, (ध्रुव शून्य*)realign[0], type->आकार);
      प्रत्यागम दत्त;
    } अन्यथा {
      पूर्णाङ्क i;
      शून्य *pdata = दत्त;

      प्रति (i = 0; i < type->आकार; i += परिमाण(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += परिमाण(size_t);
      }
      प्रत्यागम दत्त;
    }
  }
#अन्यथा
  यदि (type->type == FFI_TYPE_STRUCT) {

      यदि (var) {
        पूर्णाङ्क i;
        शून्य *pdata = दत्त;

        प्रति (i = 0; i < type->आकार; i += परिमाण(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += परिमाण(size_t);
        }
        प्रत्यागम दत्त;
      } अन्यथा {
        यदि (type->आकार > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        स्मृतिं_प्रतिलिख(दत्त, (ध्रुव शून्य*)realign[0], type->आकार);
        प्रत्यागम दत्त;
      }
  }
#यदि_अन्तः
  अन्यथा यदि (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, दत्त);
    प्रत्यागम दत्त;
  } अन्यथा {
      यदि (type->आकार > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      यदि (type->आकार > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      स्मृतिं_प्रतिलिख(दत्त, realign, type->आकार);
      प्रत्यागम दत्त;
  }
}

स्थिर पूर्णाङ्क passed_by_ref(ffi_type *type, पूर्णाङ्क var) {
  यदि (type->type == FFI_TYPE_STRUCT)
	  प्रत्यागम 1;
  
  प्रत्यागम type->आकार > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
बाह्य शून्य ffi_call_asm (शून्य *राशिः, संरचना call_context *regs,
			  शून्य (*fn) (शून्य), शून्य *संवृतिः) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	      शून्य *संवृतिः)
{
  पूर्णाङ्क return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_अष्टकानि = FFI_ALIGN(2 * परिमाण(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_अष्टकानि = FFI_ALIGN(cif->अष्टकानि, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_आकार = arg_अष्टकानि + /*rval_bytes +*/ struct_अष्टकानि + परिमाण(call_context);
  size_t alloc_आधारः = (size_t)alloca(alloc_आकार);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.प्रयुक्तम्_integer = 0;
  cb.aregs = (call_context*)(alloc_आधारः + arg_अष्टकानि /*+ rval_bytes*/ + struct_अष्टकानि);
  cb.प्रयुक्तम्_राशिः = (शून्य*)alloc_आधारः;
  cb.struct_राशिः = (शून्य *)(alloc_आधारः + arg_अष्टकानि /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  यदि (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  पूर्णाङ्क i;
  प्रति (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((शून्य *) alloc_आधारः, cb.aregs, fn, संवृतिः);

  cb.प्रयुक्तम्_integer = 0;
  यदि (!return_by_ref && rvalue)
    {
	    यदि (IS_INT(cif->rtype->type)
	        && cif->rtype->आकार < परिमाण (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    विकल्प (cif->rtype->type) {
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

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः * संवृतिः, ffi_cif * cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_दत्त, शून्य *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (संवृतिः->tramp[0]);

#यदि defined(__ARC64_ARCH64__)
  size_t स्थानाङ्कः_ffi_संवृतिः = (size_t) ffi_संवृतिः_asm;
#यदि_अन्तः

  विकल्प (cif->abi)
    {
#यदि defined(__ARC64_ARCH64__)
    पक्ष FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = संहिता_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = संहिता_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = संहिता_ENDIAN ((uint32_t)(स्थानाङ्कः_ffi_संवृतिः >> 32));
      tramp[3] = संहिता_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = संहिता_ENDIAN ((uint32_t)(स्थानाङ्कः_ffi_संवृतिः & 0xffffffff));
      tramp[5] = संहिता_ENDIAN (0x20200300);	/* j [r12] */
      भङ्ग;
#अन्यथा
    पक्ष FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = संहिता_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = संहिता_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = संहिता_ENDIAN ((uint32_t) ffi_संवृतिः_asm);
      भङ्ग;
#यदि_अन्तः

    स्वभाव:
      प्रत्यागम FFI_BAD_ABI;
    }

  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;
  cacheflush (codeloc, FFI_TRAMPOLINE_आकार, BCACHE);

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
    वर्ण *astorage = alloca(cif->अष्टकानि);
    वर्ण *ptr = astorage;
    शून्य *rvalue;
    call_builder cb;
    पूर्णाङ्क i;

    cb.aregs = aregs;
    cb.प्रयुक्तम्_integer = 0;
    cb.प्रयुक्तम्_राशिः = राशिः;
    
    /* handle hidden argument */
    यदि (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    अन्यथा
      rvalue = alloca(cif->rtype->आकार);

    प्रति (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->आकार;
    }

    fun (cif, rvalue, avalue, user_दत्त);

    यदि (cif->rtype->type != FFI_TYPE_VOID) {
        cb.प्रयुक्तम्_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
