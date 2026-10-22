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

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * परिमाण(द्विगुण))

प्रकारनिर्देश संरचना call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  वर्ण frame[16];
} call_context;

प्रकारनिर्देश संरचना call_builder
{
  call_context *aregs;
  पूर्णाङ्क used_integer;
  //int used_float;
  size_t *used_राशिः;
  शून्य *struct_राशिः;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#जइ defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अण्णहा
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#जइ __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#अण्णहा
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
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
#जइ defined(__ARC64_ARCH64__)
    पक्ष FFI_TYPE_UINT64: मानम् = *(uint64_t *)data; भङ्ग;
    पक्ष FFI_TYPE_SINT64: मानम् = *(int64_t *)data; भङ्ग;
#endif
    पक्ष FFI_TYPE_POINTER: मानम् = *(size_t *)data; भङ्ग;
    स्वभाव: FFI_ASSERT(0); भङ्ग;
  }

  जइ (cb->used_integer == NARGREG) {
    *cb->used_राशिः++ = मानम्;
  } अण्णहा {
    cb->aregs->r[cb->used_integer++] = मानम्;
  }
}

/* adds an argument to a call, or a not by reference return value */
स्थिर शून्य marshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *data) {
  size_t realign[2];

#जइ (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  जइ (type->size > 2 * __SIZEOF_POINTER__) {
    जइ (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } अण्णहा {
      /* copy to stack and pass by reference */
      data = memcpy (cb->struct_राशिः, data, type->size);
      cb->struct_राशिः = (size_t *) FFI_ALIGN ((वर्ण *) cb->struct_राशिः + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#अण्णहा
  जइ (type->type == FFI_TYPE_STRUCT) {
    जइ (var) {
      जइ (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } अण्णहा {
      पूर्णाङ्क i;
      
      प्रति (i = 0; i < type->size; i += परिमाण(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += परिमाण(size_t);
      }
    }
  }
#endif
  अण्णहा जइ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } अण्णहा {
      memcpy(realign, data, type->size);
      जइ (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      जइ (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

स्थिर शून्य unmarshal_atom(call_builder *cb, पूर्णाङ्क type, शून्य *data) {
  size_t मानम्;

  जइ (cb->used_integer == NARGREG) {
    मानम् = *cb->used_राशिः++;
  } अण्णहा {
    मानम् = cb->aregs->r[cb->used_integer++];
  }

  विकल्प (type) {
    पक्ष FFI_TYPE_UINT8: *(uint8_t *)data = मानम्; भङ्ग;
    पक्ष FFI_TYPE_SINT8: *(uint8_t *)data = मानम्; भङ्ग;
    पक्ष FFI_TYPE_UINT16: *(uint16_t *)data = मानम्; भङ्ग;
    पक्ष FFI_TYPE_SINT16: *(uint16_t *)data = मानम्; भङ्ग;
    पक्ष FFI_TYPE_UINT32: *(uint32_t *)data = मानम्; भङ्ग;
    पक्ष FFI_TYPE_SINT32: *(uint32_t *)data = मानम्; भङ्ग;
#जइ defined(__ARC64_ARCH64__)
    पक्ष FFI_TYPE_UINT64: *(uint64_t *)data = मानम्; भङ्ग;
    पक्ष FFI_TYPE_SINT64: *(uint64_t *)data = मानम्; भङ्ग;
#endif
    पक्ष FFI_TYPE_POINTER: *(size_t *)data = मानम्; भङ्ग;
    स्वभाव: FFI_ASSERT(0); भङ्ग;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
स्थिर शून्य *unmarshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *data) {
  size_t realign[2];

#जइ defined(__ARC64_ARCH64__)
  शून्य *pointer;

  जइ (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (वर्ण*)&pointer);
        प्रत्यागम pointer;
    }
#नोचेद्यदि defined(__ARC64_ARCH32__)
  जइ (type->type == FFI_TYPE_STRUCT) {
    जइ (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(data, (ध्रुव शून्य*)realign[0], type->size);
      प्रत्यागम data;
    } अण्णहा {
      पूर्णाङ्क i;
      शून्य *pdata = data;

      प्रति (i = 0; i < type->size; i += परिमाण(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += परिमाण(size_t);
      }
      प्रत्यागम data;
    }
  }
#अण्णहा
  जइ (type->type == FFI_TYPE_STRUCT) {

      जइ (var) {
        पूर्णाङ्क i;
        शून्य *pdata = data;

        प्रति (i = 0; i < type->size; i += परिमाण(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += परिमाण(size_t);
        }
        प्रत्यागम data;
      } अण्णहा {
        जइ (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(data, (ध्रुव शून्य*)realign[0], type->size);
        प्रत्यागम data;
      }
  }
#endif
  अण्णहा जइ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    प्रत्यागम data;
  } अण्णहा {
      जइ (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      जइ (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(data, realign, type->size);
      प्रत्यागम data;
  }
}

स्थिर पूर्णाङ्क passed_by_ref(ffi_type *type, पूर्णाङ्क var) {
  जइ (type->type == FFI_TYPE_STRUCT)
	  प्रत्यागम 1;
  
  प्रत्यागम type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
बाह्य शून्य ffi_call_asm (शून्य *राशिः, संरचना call_context *regs,
			  शून्य (*fn) (शून्य), शून्य *closure) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	      शून्य *closure)
{
  पूर्णाङ्क return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * परिमाण(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + परिमाण(call_context);
  size_t alloc_base = (size_t)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_राशिः = (शून्य*)alloc_base;
  cb.struct_राशिः = (शून्य *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  जइ (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  पूर्णाङ्क i;
  प्रति (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((शून्य *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  जइ (!return_by_ref && rvalue)
    {
	    जइ (IS_INT(cif->rtype->type)
	        && cif->rtype->size < परिमाण (ffi_arg))
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
	    अण्णहा
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

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_data, शून्य *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#जइ defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  विकल्प (cif->abi)
    {
#जइ defined(__ARC64_ARCH64__)
    पक्ष FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      भङ्ग;
#अण्णहा
    पक्ष FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      भङ्ग;
#endif

    स्वभाव:
      प्रत्यागम FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_go_closure_asm (शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  जइ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
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
		   size_t *राशिः, call_context *aregs)
{
    शून्य **avalue = alloca(cif->nargs * परिमाण(शून्य*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    वर्ण *astorage = alloca(cif->bytes);
    वर्ण *ptr = astorage;
    शून्य *rvalue;
    call_builder cb;
    पूर्णाङ्क i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_राशिः = राशिः;
    
    /* handle hidden argument */
    जइ (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    अण्णहा
      rvalue = alloca(cif->rtype->size);

    प्रति (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    जइ (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
