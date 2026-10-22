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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>
#समावेश <stdint.h>

#समावेश <sys/cachectl.h>

#घोषणा NARGREG 8
#घोषणा STKALIGN 4
#घोषणा MAXCOPYARG (2 * माप(द्विगुन))

प्रकार काष्ठा call_context
{
  माप_प्रकार r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  अक्षर frame[16];
} call_context;

प्रकार काष्ठा call_builder
{
  call_context *aregs;
  पूर्णांक used_integer;
  //int used_float;
  माप_प्रकार *used_ढेर;
  व्योम *struct_ढेर;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#अगर defined(__ARC64_ARCH64__)
#घोषणा IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अन्यथा
#घोषणा IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#पूर्ण_अगर

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#अगर __BIG_ENDIAN__
#घोषणा CODE_ENDIAN(x) (x)
#अन्यथा
#घोषणा CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#पूर्ण_अगर

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_VOID:
      cif->flags = (अचिन्हित) cif->rtype->type;
      अवरोध;

    हाल FFI_TYPE_STRUCT:
      cif->flags = (अचिन्हित) cif->rtype->type;
      अवरोध;

    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      अवरोध;

    हाल FFI_TYPE_FLOAT:
    शेष:
      cif->flags = FFI_TYPE_INT;
      अवरोध;
    }

  वापस FFI_OK;
}

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
#अगर defined(__ARC64_ARCH64__)
    हाल FFI_TYPE_UINT64: मूल्य = *(uint64_t *)तथ्य; अवरोध;
    हाल FFI_TYPE_SINT64: मूल्य = *(int64_t *)तथ्य; अवरोध;
#पूर्ण_अगर
    हाल FFI_TYPE_POINTER: मूल्य = *(माप_प्रकार *)तथ्य; अवरोध;
    शेष: FFI_ASSERT(0); अवरोध;
  }

  अगर (cb->used_integer == NARGREG) {
    *cb->used_ढेर++ = मूल्य;
  } अन्यथा {
    cb->aregs->r[cb->used_integer++] = मूल्य;
  }
}

/* adds an argument to a call, or a not by reference return value */
अटल व्योम marshal(call_builder *cb, ffi_type *type, पूर्णांक var, व्योम *तथ्य) {
  माप_प्रकार realign[2];

#अगर (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  अगर (type->आकार > 2 * __SIZEOF_POINTER__) {
    अगर (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &तथ्य);
    } अन्यथा {
      /* copy to stack and pass by reference */
      तथ्य = स_नकल (cb->struct_ढेर, तथ्य, type->आकार);
      cb->struct_ढेर = (माप_प्रकार *) FFI_ALIGN ((अक्षर *) cb->struct_ढेर + type->आकार, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &तथ्य);
    }
  }
#अन्यथा
  अगर (type->type == FFI_TYPE_STRUCT) {
    अगर (var) {
      अगर (type->आकार > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, तथ्य);
    } अन्यथा {
      पूर्णांक i;
      
      क्रम (i = 0; i < type->आकार; i += माप(माप_प्रकार)) {
        marshal_atom(cb, FFI_TYPE_POINTER, तथ्य);
        तथ्य += माप(माप_प्रकार);
      }
    }
  }
#पूर्ण_अगर
  अन्यथा अगर (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, तथ्य);
  } अन्यथा {
      स_नकल(realign, तथ्य, type->आकार);
      अगर (type->आकार > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      अगर (type->आकार > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

अटल व्योम unmarshal_atom(call_builder *cb, पूर्णांक type, व्योम *तथ्य) {
  माप_प्रकार मूल्य;

  अगर (cb->used_integer == NARGREG) {
    मूल्य = *cb->used_ढेर++;
  } अन्यथा {
    मूल्य = cb->aregs->r[cb->used_integer++];
  }

  चयन (type) {
    हाल FFI_TYPE_UINT8: *(uint8_t *)तथ्य = मूल्य; अवरोध;
    हाल FFI_TYPE_SINT8: *(uint8_t *)तथ्य = मूल्य; अवरोध;
    हाल FFI_TYPE_UINT16: *(uint16_t *)तथ्य = मूल्य; अवरोध;
    हाल FFI_TYPE_SINT16: *(uint16_t *)तथ्य = मूल्य; अवरोध;
    हाल FFI_TYPE_UINT32: *(uint32_t *)तथ्य = मूल्य; अवरोध;
    हाल FFI_TYPE_SINT32: *(uint32_t *)तथ्य = मूल्य; अवरोध;
#अगर defined(__ARC64_ARCH64__)
    हाल FFI_TYPE_UINT64: *(uint64_t *)तथ्य = मूल्य; अवरोध;
    हाल FFI_TYPE_SINT64: *(uint64_t *)तथ्य = मूल्य; अवरोध;
#पूर्ण_अगर
    हाल FFI_TYPE_POINTER: *(माप_प्रकार *)तथ्य = मूल्य; अवरोध;
    शेष: FFI_ASSERT(0); अवरोध;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
अटल व्योम *unmarshal(call_builder *cb, ffi_type *type, पूर्णांक var, व्योम *तथ्य) {
  माप_प्रकार realign[2];

#अगर defined(__ARC64_ARCH64__)
  व्योम *pointer;

  अगर (type->आकार > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (अक्षर*)&pointer);
        वापस pointer;
    }
#या_अगर defined(__ARC64_ARCH32__)
  अगर (type->type == FFI_TYPE_STRUCT) {
    अगर (type->आकार > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      स_नकल(तथ्य, (स्थिर व्योम*)realign[0], type->आकार);
      वापस तथ्य;
    } अन्यथा {
      पूर्णांक i;
      व्योम *pdata = तथ्य;

      क्रम (i = 0; i < type->आकार; i += माप(माप_प्रकार)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += माप(माप_प्रकार);
      }
      वापस तथ्य;
    }
  }
#अन्यथा
  अगर (type->type == FFI_TYPE_STRUCT) {

      अगर (var) {
        पूर्णांक i;
        व्योम *pdata = तथ्य;

        क्रम (i = 0; i < type->आकार; i += माप(माप_प्रकार)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += माप(माप_प्रकार);
        }
        वापस तथ्य;
      } अन्यथा {
        अगर (type->आकार > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        स_नकल(तथ्य, (स्थिर व्योम*)realign[0], type->आकार);
        वापस तथ्य;
      }
  }
#पूर्ण_अगर
  अन्यथा अगर (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, तथ्य);
    वापस तथ्य;
  } अन्यथा {
      अगर (type->आकार > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      अगर (type->आकार > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      स_नकल(तथ्य, realign, type->आकार);
      वापस तथ्य;
  }
}

अटल पूर्णांक passed_by_ref(ffi_type *type, पूर्णांक var) {
  अगर (type->type == FFI_TYPE_STRUCT)
	  वापस 1;
  
  वापस type->आकार > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
बाह्य व्योम ffi_call_asm (व्योम *ढेर, काष्ठा call_context *regs,
			  व्योम (*fn) (व्योम), व्योम *closure) FFI_HIDDEN;

अटल व्योम
ffi_call_int (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue, व्योम **avalue,
	      व्योम *closure)
{
  पूर्णांक return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  माप_प्रकार arg_bytes = FFI_ALIGN(2 * माप(माप_प्रकार) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  माप_प्रकार struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  माप_प्रकार alloc_आकार = arg_bytes + /*rval_bytes +*/ struct_bytes + माप(call_context);
  माप_प्रकार alloc_base = (माप_प्रकार)alloca(alloc_आकार);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_ढेर = (व्योम*)alloc_base;
  cb.struct_ढेर = (व्योम *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  अगर (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  पूर्णांक i;
  क्रम (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((व्योम *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  अगर (!return_by_ref && rvalue)
    {
	    अगर (IS_INT(cif->rtype->type)
	        && cif->rtype->आकार < माप (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    चयन (cif->rtype->type) {
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

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
		      व्योम *user_तथ्य, व्योम *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#अगर defined(__ARC64_ARCH64__)
  माप_प्रकार address_ffi_closure = (माप_प्रकार) ffi_closure_asm;
#पूर्ण_अगर

  चयन (cif->abi)
    {
#अगर defined(__ARC64_ARCH64__)
    हाल FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      अवरोध;
#अन्यथा
    हाल FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      अवरोध;
#पूर्ण_अगर

    शेष:
      वापस FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;
  cacheflush (codeloc, FFI_TRAMPOLINE_आकार, BCACHE);

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
    अक्षर *astorage = alloca(cif->bytes);
    अक्षर *ptr = astorage;
    व्योम *rvalue;
    call_builder cb;
    पूर्णांक i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_ढेर = ढेर;
    
    /* handle hidden argument */
    अगर (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    अन्यथा
      rvalue = alloca(cif->rtype->आकार);

    क्रम (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->आकार;
    }

    fun (cif, rvalue, avalue, user_तथ्य);

    अगर (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
