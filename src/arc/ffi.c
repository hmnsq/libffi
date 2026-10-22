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
  माप_प्रकार r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  वर्ण frame[16];
} call_context;

प्रकारनिर्देश संरचना call_builder
{
  call_context *aregs;
  पूर्णाङ्क used_integer;
  //int used_float;
  माप_प्रकार *used_थाक;
  शून्य *struct_थाक;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#यदि defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अन्यथा
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#यदि __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#अन्यथा
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
      cif->flags = (अचिन्हित) cif->rtype->type;
      रोक;

    पक्ष FFI_TYPE_STRUCT:
      cif->flags = (अचिन्हित) cif->rtype->type;
      रोक;

    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      रोक;

    पक्ष FFI_TYPE_FLOAT:
    स्वभाव:
      cif->flags = FFI_TYPE_INT;
      रोक;
    }

  फर्काऊ FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
स्थिर शून्य marshal_atom(call_builder *cb, पूर्णाङ्क type, शून्य *data) {
  माप_प्रकार मान = 0;
  विकल्प (type) {
    पक्ष FFI_TYPE_UINT8: मान = *(uint8_t *)data; रोक;
    पक्ष FFI_TYPE_SINT8: मान = *(int8_t *)data; रोक;
    पक्ष FFI_TYPE_UINT16: मान = *(uint16_t *)data; रोक;
    पक्ष FFI_TYPE_SINT16: मान = *(int16_t *)data; रोक;
    /* 32-bit quantities are always sign-extended in the ABI */
    पक्ष FFI_TYPE_UINT32: मान = *(int32_t *)data; रोक;
    पक्ष FFI_TYPE_SINT32: मान = *(int32_t *)data; रोक;
#यदि defined(__ARC64_ARCH64__)
    पक्ष FFI_TYPE_UINT64: मान = *(uint64_t *)data; रोक;
    पक्ष FFI_TYPE_SINT64: मान = *(int64_t *)data; रोक;
#endif
    पक्ष FFI_TYPE_POINTER: मान = *(माप_प्रकार *)data; रोक;
    स्वभाव: FFI_ASSERT(0); रोक;
  }

  यदि (cb->used_integer == NARGREG) {
    *cb->used_थाक++ = मान;
  } अन्यथा {
    cb->aregs->r[cb->used_integer++] = मान;
  }
}

/* adds an argument to a call, or a not by reference return value */
स्थिर शून्य marshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *data) {
  माप_प्रकार realign[2];

#यदि (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  यदि (type->size > 2 * __SIZEOF_POINTER__) {
    यदि (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } अन्यथा {
      /* copy to stack and pass by reference */
      data = स_नकल (cb->struct_थाक, data, type->size);
      cb->struct_थाक = (माप_प्रकार *) FFI_ALIGN ((वर्ण *) cb->struct_थाक + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#अन्यथा
  यदि (type->type == FFI_TYPE_STRUCT) {
    यदि (var) {
      यदि (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } अन्यथा {
      पूर्णाङ्क i;
      
      लागि (i = 0; i < type->size; i += परिमाण(माप_प्रकार)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += परिमाण(माप_प्रकार);
      }
    }
  }
#endif
  अन्यथा यदि (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } अन्यथा {
      स_नकल(realign, data, type->size);
      यदि (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      यदि (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

स्थिर शून्य unmarshal_atom(call_builder *cb, पूर्णाङ्क type, शून्य *data) {
  माप_प्रकार मान;

  यदि (cb->used_integer == NARGREG) {
    मान = *cb->used_थाक++;
  } अन्यथा {
    मान = cb->aregs->r[cb->used_integer++];
  }

  विकल्प (type) {
    पक्ष FFI_TYPE_UINT8: *(uint8_t *)data = मान; रोक;
    पक्ष FFI_TYPE_SINT8: *(uint8_t *)data = मान; रोक;
    पक्ष FFI_TYPE_UINT16: *(uint16_t *)data = मान; रोक;
    पक्ष FFI_TYPE_SINT16: *(uint16_t *)data = मान; रोक;
    पक्ष FFI_TYPE_UINT32: *(uint32_t *)data = मान; रोक;
    पक्ष FFI_TYPE_SINT32: *(uint32_t *)data = मान; रोक;
#यदि defined(__ARC64_ARCH64__)
    पक्ष FFI_TYPE_UINT64: *(uint64_t *)data = मान; रोक;
    पक्ष FFI_TYPE_SINT64: *(uint64_t *)data = मान; रोक;
#endif
    पक्ष FFI_TYPE_POINTER: *(माप_प्रकार *)data = मान; रोक;
    स्वभाव: FFI_ASSERT(0); रोक;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
स्थिर शून्य *unmarshal(call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *data) {
  माप_प्रकार realign[2];

#यदि defined(__ARC64_ARCH64__)
  शून्य *pointer;

  यदि (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (वर्ण*)&pointer);
        फर्काऊ pointer;
    }
#अथवायदि defined(__ARC64_ARCH32__)
  यदि (type->type == FFI_TYPE_STRUCT) {
    यदि (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      स_नकल(data, (अचर शून्य*)realign[0], type->size);
      फर्काऊ data;
    } अन्यथा {
      पूर्णाङ्क i;
      शून्य *pdata = data;

      लागि (i = 0; i < type->size; i += परिमाण(माप_प्रकार)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += परिमाण(माप_प्रकार);
      }
      फर्काऊ data;
    }
  }
#अन्यथा
  यदि (type->type == FFI_TYPE_STRUCT) {

      यदि (var) {
        पूर्णाङ्क i;
        शून्य *pdata = data;

        लागि (i = 0; i < type->size; i += परिमाण(माप_प्रकार)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += परिमाण(माप_प्रकार);
        }
        फर्काऊ data;
      } अन्यथा {
        यदि (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        स_नकल(data, (अचर शून्य*)realign[0], type->size);
        फर्काऊ data;
      }
  }
#endif
  अन्यथा यदि (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    फर्काऊ data;
  } अन्यथा {
      यदि (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      यदि (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      स_नकल(data, realign, type->size);
      फर्काऊ data;
  }
}

स्थिर पूर्णाङ्क passed_by_ref(ffi_type *type, पूर्णाङ्क var) {
  यदि (type->type == FFI_TYPE_STRUCT)
	  फर्काऊ 1;
  
  फर्काऊ type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
बाह्य शून्य ffi_call_asm (शून्य *थाक, संरचना call_context *regs,
			  शून्य (*fn) (शून्य), शून्य *closure) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	      शून्य *closure)
{
  पूर्णाङ्क return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  माप_प्रकार arg_bytes = FFI_ALIGN(2 * परिमाण(माप_प्रकार) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  माप_प्रकार struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  माप_प्रकार alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + परिमाण(call_context);
  माप_प्रकार alloc_base = (माप_प्रकार)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_थाक = (शून्य*)alloc_base;
  cb.struct_थाक = (शून्य *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  यदि (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  पूर्णाङ्क i;
  लागि (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((शून्य *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  यदि (!return_by_ref && rvalue)
    {
	    यदि (IS_INT(cif->rtype->type)
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
		      रोक;
	      पक्ष FFI_TYPE_UINT8:
	      पक्ष FFI_TYPE_UINT16:
	      पक्ष FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (परिमाण (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      रोक;
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

#यदि defined(__ARC64_ARCH64__)
  माप_प्रकार address_ffi_closure = (माप_प्रकार) ffi_closure_asm;
#endif

  विकल्प (cif->abi)
    {
#यदि defined(__ARC64_ARCH64__)
    पक्ष FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      रोक;
#अन्यथा
    पक्ष FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      रोक;
#endif

    स्वभाव:
      फर्काऊ FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  फर्काऊ FFI_OK;
}

बाह्य शून्य ffi_go_closure_asm (शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  यदि (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    फर्काऊ FFI_BAD_ABI;

  closure->tramp = (शून्य *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  फर्काऊ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
शून्य FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		   शून्य *user_data,
		   माप_प्रकार *थाक, call_context *aregs)
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
    cb.used_थाक = थाक;
    
    /* handle hidden argument */
    यदि (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    अन्यथा
      rvalue = alloca(cif->rtype->size);

    लागि (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    यदि (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
