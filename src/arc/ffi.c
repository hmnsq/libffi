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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdint.h>

#അന്തര്ഭാവ <sys/cachectl.h>

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * വലുപ്പം(ഇരട്ട))

തരനിർവചനം ഘടന call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  അക്ഷരം frame[16];
} call_context;

തരനിർവചനം ഘടന call_builder
{
  call_context *aregs;
  പൂർണ്ണസംഖ്യ used_integer;
  //int used_float;
  size_t *used_അട്ടി;
  ശൂന്യം *struct_അട്ടി;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#എങ്കിൽ defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#അല്ലെങ്കിൽ
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#എങ്കിൽ __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#അല്ലെങ്കിൽ
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_FLOAT:
    സ്ഥിരസ്ഥിതി:
      cif->flags = FFI_TYPE_INT;
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
സ്ഥിതം ശൂന്യം marshal_atom(call_builder *cb, പൂർണ്ണസംഖ്യ type, ശൂന്യം *data) {
  size_t മൂല്യം = 0;
  തിരഞ്ഞെടുപ്പ് (type) {
    അവസ്ഥ FFI_TYPE_UINT8: മൂല്യം = *(uint8_t *)data; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT8: മൂല്യം = *(int8_t *)data; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT16: മൂല്യം = *(uint16_t *)data; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16: മൂല്യം = *(int16_t *)data; നിർത്തുക;
    /* 32-bit quantities are always sign-extended in the ABI */
    അവസ്ഥ FFI_TYPE_UINT32: മൂല്യം = *(int32_t *)data; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT32: മൂല്യം = *(int32_t *)data; നിർത്തുക;
#എങ്കിൽ defined(__ARC64_ARCH64__)
    അവസ്ഥ FFI_TYPE_UINT64: മൂല്യം = *(uint64_t *)data; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT64: മൂല്യം = *(int64_t *)data; നിർത്തുക;
#endif
    അവസ്ഥ FFI_TYPE_POINTER: മൂല്യം = *(size_t *)data; നിർത്തുക;
    സ്ഥിരസ്ഥിതി: FFI_ASSERT(0); നിർത്തുക;
  }

  എങ്കിൽ (cb->used_integer == NARGREG) {
    *cb->used_അട്ടി++ = മൂല്യം;
  } അല്ലെങ്കിൽ {
    cb->aregs->r[cb->used_integer++] = മൂല്യം;
  }
}

/* adds an argument to a call, or a not by reference return value */
സ്ഥിതം ശൂന്യം marshal(call_builder *cb, ffi_type *type, പൂർണ്ണസംഖ്യ var, ശൂന്യം *data) {
  size_t realign[2];

#എങ്കിൽ (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  എങ്കിൽ (type->size > 2 * __SIZEOF_POINTER__) {
    എങ്കിൽ (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } അല്ലെങ്കിൽ {
      /* copy to stack and pass by reference */
      data = memcpy (cb->struct_അട്ടി, data, type->size);
      cb->struct_അട്ടി = (size_t *) FFI_ALIGN ((അക്ഷരം *) cb->struct_അട്ടി + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#അല്ലെങ്കിൽ
  എങ്കിൽ (type->type == FFI_TYPE_STRUCT) {
    എങ്കിൽ (var) {
      എങ്കിൽ (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } അല്ലെങ്കിൽ {
      പൂർണ്ണസംഖ്യ i;
      
      വേണ്ടി (i = 0; i < type->size; i += വലുപ്പം(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += വലുപ്പം(size_t);
      }
    }
  }
#endif
  അല്ലെങ്കിൽ എങ്കിൽ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } അല്ലെങ്കിൽ {
      memcpy(realign, data, type->size);
      എങ്കിൽ (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      എങ്കിൽ (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

സ്ഥിതം ശൂന്യം unmarshal_atom(call_builder *cb, പൂർണ്ണസംഖ്യ type, ശൂന്യം *data) {
  size_t മൂല്യം;

  എങ്കിൽ (cb->used_integer == NARGREG) {
    മൂല്യം = *cb->used_അട്ടി++;
  } അല്ലെങ്കിൽ {
    മൂല്യം = cb->aregs->r[cb->used_integer++];
  }

  തിരഞ്ഞെടുപ്പ് (type) {
    അവസ്ഥ FFI_TYPE_UINT8: *(uint8_t *)data = മൂല്യം; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT8: *(uint8_t *)data = മൂല്യം; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT16: *(uint16_t *)data = മൂല്യം; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16: *(uint16_t *)data = മൂല്യം; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT32: *(uint32_t *)data = മൂല്യം; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT32: *(uint32_t *)data = മൂല്യം; നിർത്തുക;
#എങ്കിൽ defined(__ARC64_ARCH64__)
    അവസ്ഥ FFI_TYPE_UINT64: *(uint64_t *)data = മൂല്യം; നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT64: *(uint64_t *)data = മൂല്യം; നിർത്തുക;
#endif
    അവസ്ഥ FFI_TYPE_POINTER: *(size_t *)data = മൂല്യം; നിർത്തുക;
    സ്ഥിരസ്ഥിതി: FFI_ASSERT(0); നിർത്തുക;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
സ്ഥിതം ശൂന്യം *unmarshal(call_builder *cb, ffi_type *type, പൂർണ്ണസംഖ്യ var, ശൂന്യം *data) {
  size_t realign[2];

#എങ്കിൽ defined(__ARC64_ARCH64__)
  ശൂന്യം *pointer;

  എങ്കിൽ (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (അക്ഷരം*)&pointer);
        തിരികെ pointer;
    }
#അല്ലെങ്കിലെങ്കിൽ defined(__ARC64_ARCH32__)
  എങ്കിൽ (type->type == FFI_TYPE_STRUCT) {
    എങ്കിൽ (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(data, (സ്ഥിരം ശൂന്യം*)realign[0], type->size);
      തിരികെ data;
    } അല്ലെങ്കിൽ {
      പൂർണ്ണസംഖ്യ i;
      ശൂന്യം *pdata = data;

      വേണ്ടി (i = 0; i < type->size; i += വലുപ്പം(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += വലുപ്പം(size_t);
      }
      തിരികെ data;
    }
  }
#അല്ലെങ്കിൽ
  എങ്കിൽ (type->type == FFI_TYPE_STRUCT) {

      എങ്കിൽ (var) {
        പൂർണ്ണസംഖ്യ i;
        ശൂന്യം *pdata = data;

        വേണ്ടി (i = 0; i < type->size; i += വലുപ്പം(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += വലുപ്പം(size_t);
        }
        തിരികെ data;
      } അല്ലെങ്കിൽ {
        എങ്കിൽ (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(data, (സ്ഥിരം ശൂന്യം*)realign[0], type->size);
        തിരികെ data;
      }
  }
#endif
  അല്ലെങ്കിൽ എങ്കിൽ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    തിരികെ data;
  } അല്ലെങ്കിൽ {
      എങ്കിൽ (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      എങ്കിൽ (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(data, realign, type->size);
      തിരികെ data;
  }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ passed_by_ref(ffi_type *type, പൂർണ്ണസംഖ്യ var) {
  എങ്കിൽ (type->type == FFI_TYPE_STRUCT)
	  തിരികെ 1;
  
  തിരികെ type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
ബാഹ്യ ശൂന്യം ffi_call_asm (ശൂന്യം *അട്ടി, ഘടന call_context *regs,
			  ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *closure) FFI_HIDDEN;

സ്ഥിതം ശൂന്യം
ffi_call_int (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue,
	      ശൂന്യം *closure)
{
  പൂർണ്ണസംഖ്യ return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * വലുപ്പം(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + വലുപ്പം(call_context);
  size_t alloc_base = (size_t)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_അട്ടി = (ശൂന്യം*)alloc_base;
  cb.struct_അട്ടി = (ശൂന്യം *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  എങ്കിൽ (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((ശൂന്യം *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  എങ്കിൽ (!return_by_ref && rvalue)
    {
	    എങ്കിൽ (IS_INT(cif->rtype->type)
	        && cif->rtype->size < വലുപ്പം (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    തിരഞ്ഞെടുപ്പ് (cif->rtype->type) {
	      അവസ്ഥ FFI_TYPE_SINT8:
	      അവസ്ഥ FFI_TYPE_SINT16:
	      അവസ്ഥ FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (വലുപ്പം (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      നിർത്തുക;
	      അവസ്ഥ FFI_TYPE_UINT8:
	      അവസ്ഥ FFI_TYPE_UINT16:
	      അവസ്ഥ FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (വലുപ്പം (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      നിർത്തുക;
	      }
	    }
	    അല്ലെങ്കിൽ
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ശൂന്യ);
}

ശൂന്യം
ffi_call_go (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue,
	     ശൂന്യം **avalue, ശൂന്യം *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ബാഹ്യ ശൂന്യം ffi_closure_asm(ശൂന്യം) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
		      ശൂന്യം *user_data, ശൂന്യം *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#എങ്കിൽ defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
#എങ്കിൽ defined(__ARC64_ARCH64__)
    അവസ്ഥ FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      നിർത്തുക;
#അല്ലെങ്കിൽ
    അവസ്ഥ FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      നിർത്തുക;
#endif

    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  തിരികെ FFI_OK;
}

ബാഹ്യ ശൂന്യം ffi_go_closure_asm (ശൂന്യം) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *))
{
  എങ്കിൽ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    തിരികെ FFI_BAD_ABI;

  closure->tramp = (ശൂന്യം *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  തിരികെ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
ശൂന്യം FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
		   ശൂന്യം *user_data,
		   size_t *അട്ടി, call_context *aregs)
{
    ശൂന്യം **avalue = alloca(cif->nargs * വലുപ്പം(ശൂന്യം*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    അക്ഷരം *astorage = alloca(cif->bytes);
    അക്ഷരം *ptr = astorage;
    ശൂന്യം *rvalue;
    call_builder cb;
    പൂർണ്ണസംഖ്യ i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_അട്ടി = അട്ടി;
    
    /* handle hidden argument */
    എങ്കിൽ (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    അല്ലെങ്കിൽ
      rvalue = alloca(cif->rtype->size);

    വേണ്ടി (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    എങ്കിൽ (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
