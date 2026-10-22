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
#define MAXCOPYARG (2 * आकार(द्विगुण))

प्रकारव्याख्या संरचना call_context
{
  माप_प्रकार r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  अक्षर frame[16];
} call_context;

प्रकारव्याख्या संरचना call_builder
{
  call_context *aregs;
  पूर्णांक used_integer;
  //int used_float;
  माप_प्रकार *used_चळत;
  शून्य *struct_चळत;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#जर defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अन्यथा
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#जर __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#अन्यथा
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
      cif->flags = (अचिन्हित) cif->rtype->type;
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
      cif->flags = (अचिन्हित) cif->rtype->type;
      थांबा;

    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      थांबा;

    स्थिती FFI_TYPE_FLOAT:
    पूर्वनिर्धारित:
      cif->flags = FFI_TYPE_INT;
      थांबा;
    }

  परत FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
स्थिर शून्य marshal_atom(call_builder *cb, पूर्णांक type, शून्य *data) {
  माप_प्रकार मूल्य = 0;
  निवड (type) {
    स्थिती FFI_TYPE_UINT8: मूल्य = *(uint8_t *)data; थांबा;
    स्थिती FFI_TYPE_SINT8: मूल्य = *(int8_t *)data; थांबा;
    स्थिती FFI_TYPE_UINT16: मूल्य = *(uint16_t *)data; थांबा;
    स्थिती FFI_TYPE_SINT16: मूल्य = *(int16_t *)data; थांबा;
    /* 32-bit quantities are always sign-extended in the ABI */
    स्थिती FFI_TYPE_UINT32: मूल्य = *(int32_t *)data; थांबा;
    स्थिती FFI_TYPE_SINT32: मूल्य = *(int32_t *)data; थांबा;
#जर defined(__ARC64_ARCH64__)
    स्थिती FFI_TYPE_UINT64: मूल्य = *(uint64_t *)data; थांबा;
    स्थिती FFI_TYPE_SINT64: मूल्य = *(int64_t *)data; थांबा;
#endif
    स्थिती FFI_TYPE_POINTER: मूल्य = *(माप_प्रकार *)data; थांबा;
    पूर्वनिर्धारित: FFI_ASSERT(0); थांबा;
  }

  जर (cb->used_integer == NARGREG) {
    *cb->used_चळत++ = मूल्य;
  } अन्यथा {
    cb->aregs->r[cb->used_integer++] = मूल्य;
  }
}

/* adds an argument to a call, or a not by reference return value */
स्थिर शून्य marshal(call_builder *cb, ffi_type *type, पूर्णांक var, शून्य *data) {
  माप_प्रकार realign[2];

#जर (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  जर (type->size > 2 * __SIZEOF_POINTER__) {
    जर (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } अन्यथा {
      /* copy to stack and pass by reference */
      data = स_नकल (cb->struct_चळत, data, type->size);
      cb->struct_चळत = (माप_प्रकार *) FFI_ALIGN ((अक्षर *) cb->struct_चळत + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#अन्यथा
  जर (type->type == FFI_TYPE_STRUCT) {
    जर (var) {
      जर (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } अन्यथा {
      पूर्णांक i;
      
      साठी (i = 0; i < type->size; i += आकार(माप_प्रकार)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += आकार(माप_प्रकार);
      }
    }
  }
#endif
  अन्यथा जर (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } अन्यथा {
      स_नकल(realign, data, type->size);
      जर (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      जर (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

स्थिर शून्य unmarshal_atom(call_builder *cb, पूर्णांक type, शून्य *data) {
  माप_प्रकार मूल्य;

  जर (cb->used_integer == NARGREG) {
    मूल्य = *cb->used_चळत++;
  } अन्यथा {
    मूल्य = cb->aregs->r[cb->used_integer++];
  }

  निवड (type) {
    स्थिती FFI_TYPE_UINT8: *(uint8_t *)data = मूल्य; थांबा;
    स्थिती FFI_TYPE_SINT8: *(uint8_t *)data = मूल्य; थांबा;
    स्थिती FFI_TYPE_UINT16: *(uint16_t *)data = मूल्य; थांबा;
    स्थिती FFI_TYPE_SINT16: *(uint16_t *)data = मूल्य; थांबा;
    स्थिती FFI_TYPE_UINT32: *(uint32_t *)data = मूल्य; थांबा;
    स्थिती FFI_TYPE_SINT32: *(uint32_t *)data = मूल्य; थांबा;
#जर defined(__ARC64_ARCH64__)
    स्थिती FFI_TYPE_UINT64: *(uint64_t *)data = मूल्य; थांबा;
    स्थिती FFI_TYPE_SINT64: *(uint64_t *)data = मूल्य; थांबा;
#endif
    स्थिती FFI_TYPE_POINTER: *(माप_प्रकार *)data = मूल्य; थांबा;
    पूर्वनिर्धारित: FFI_ASSERT(0); थांबा;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
स्थिर शून्य *unmarshal(call_builder *cb, ffi_type *type, पूर्णांक var, शून्य *data) {
  माप_प्रकार realign[2];

#जर defined(__ARC64_ARCH64__)
  शून्य *pointer;

  जर (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (अक्षर*)&pointer);
        परत pointer;
    }
#किंवाजर defined(__ARC64_ARCH32__)
  जर (type->type == FFI_TYPE_STRUCT) {
    जर (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      स_नकल(data, (स्थिरांक शून्य*)realign[0], type->size);
      परत data;
    } अन्यथा {
      पूर्णांक i;
      शून्य *pdata = data;

      साठी (i = 0; i < type->size; i += आकार(माप_प्रकार)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += आकार(माप_प्रकार);
      }
      परत data;
    }
  }
#अन्यथा
  जर (type->type == FFI_TYPE_STRUCT) {

      जर (var) {
        पूर्णांक i;
        शून्य *pdata = data;

        साठी (i = 0; i < type->size; i += आकार(माप_प्रकार)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += आकार(माप_प्रकार);
        }
        परत data;
      } अन्यथा {
        जर (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        स_नकल(data, (स्थिरांक शून्य*)realign[0], type->size);
        परत data;
      }
  }
#endif
  अन्यथा जर (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    परत data;
  } अन्यथा {
      जर (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      जर (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      स_नकल(data, realign, type->size);
      परत data;
  }
}

स्थिर पूर्णांक passed_by_ref(ffi_type *type, पूर्णांक var) {
  जर (type->type == FFI_TYPE_STRUCT)
	  परत 1;
  
  परत type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
बाह्य शून्य ffi_call_asm (शून्य *चळत, संरचना call_context *regs,
			  शून्य (*fn) (शून्य), शून्य *closure) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	      शून्य *closure)
{
  पूर्णांक return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  माप_प्रकार arg_bytes = FFI_ALIGN(2 * आकार(माप_प्रकार) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  माप_प्रकार struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  माप_प्रकार alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + आकार(call_context);
  माप_प्रकार alloc_base = (माप_प्रकार)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_चळत = (शून्य*)alloc_base;
  cb.struct_चळत = (शून्य *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  जर (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  पूर्णांक i;
  साठी (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((शून्य *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  जर (!return_by_ref && rvalue)
    {
	    जर (IS_INT(cif->rtype->type)
	        && cif->rtype->size < आकार (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    निवड (cif->rtype->type) {
	      स्थिती FFI_TYPE_SINT8:
	      स्थिती FFI_TYPE_SINT16:
	      स्थिती FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (आकार (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      थांबा;
	      स्थिती FFI_TYPE_UINT8:
	      स्थिती FFI_TYPE_UINT16:
	      स्थिती FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (आकार (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      थांबा;
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

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_data, शून्य *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#जर defined(__ARC64_ARCH64__)
  माप_प्रकार address_ffi_closure = (माप_प्रकार) ffi_closure_asm;
#endif

  निवड (cif->abi)
    {
#जर defined(__ARC64_ARCH64__)
    स्थिती FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      थांबा;
#अन्यथा
    स्थिती FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      थांबा;
#endif

    पूर्वनिर्धारित:
      परत FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  परत FFI_OK;
}

बाह्य शून्य ffi_go_closure_asm (शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  जर (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    परत FFI_BAD_ABI;

  closure->tramp = (शून्य *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  परत FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
शून्य FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		   शून्य *user_data,
		   माप_प्रकार *चळत, call_context *aregs)
{
    शून्य **avalue = alloca(cif->nargs * आकार(शून्य*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    अक्षर *astorage = alloca(cif->bytes);
    अक्षर *ptr = astorage;
    शून्य *rvalue;
    call_builder cb;
    पूर्णांक i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_चळत = चळत;
    
    /* handle hidden argument */
    जर (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    अन्यथा
      rvalue = alloca(cif->rtype->size);

    साठी (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    जर (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
