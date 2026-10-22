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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdint.h>

#அந்தர்பாவ <sys/cachectl.h>

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * அளவு(இரட்டை))

வகைவரை கட்டமைப்பு call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  எழுத்து frame[16];
} call_context;

வகைவரை கட்டமைப்பு call_builder
{
  call_context *aregs;
  முழு used_integer;
  //int used_float;
  size_t *used_அடுக்கு;
  வெறுமை *struct_அடுக்கு;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#எனில் defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#இல்லையெனில்
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#எனில் __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#இல்லையெனில்
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
      நிறுத்து;

    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      நிறுத்து;

    நிலை FFI_TYPE_FLOAT:
    இயல்பு:
      cif->flags = FFI_TYPE_INT;
      நிறுத்து;
    }

  திரும்பு FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
நிலையான வெறுமை marshal_atom(call_builder *cb, முழு type, வெறுமை *data) {
  size_t மதிப்பு = 0;
  தேர்வு (type) {
    நிலை FFI_TYPE_UINT8: மதிப்பு = *(uint8_t *)data; நிறுத்து;
    நிலை FFI_TYPE_SINT8: மதிப்பு = *(int8_t *)data; நிறுத்து;
    நிலை FFI_TYPE_UINT16: மதிப்பு = *(uint16_t *)data; நிறுத்து;
    நிலை FFI_TYPE_SINT16: மதிப்பு = *(int16_t *)data; நிறுத்து;
    /* 32-bit quantities are always sign-extended in the ABI */
    நிலை FFI_TYPE_UINT32: மதிப்பு = *(int32_t *)data; நிறுத்து;
    நிலை FFI_TYPE_SINT32: மதிப்பு = *(int32_t *)data; நிறுத்து;
#எனில் defined(__ARC64_ARCH64__)
    நிலை FFI_TYPE_UINT64: மதிப்பு = *(uint64_t *)data; நிறுத்து;
    நிலை FFI_TYPE_SINT64: மதிப்பு = *(int64_t *)data; நிறுத்து;
#endif
    நிலை FFI_TYPE_POINTER: மதிப்பு = *(size_t *)data; நிறுத்து;
    இயல்பு: FFI_ASSERT(0); நிறுத்து;
  }

  எனில் (cb->used_integer == NARGREG) {
    *cb->used_அடுக்கு++ = மதிப்பு;
  } இல்லையெனில் {
    cb->aregs->r[cb->used_integer++] = மதிப்பு;
  }
}

/* adds an argument to a call, or a not by reference return value */
நிலையான வெறுமை marshal(call_builder *cb, ffi_type *type, முழு var, வெறுமை *data) {
  size_t realign[2];

#எனில் (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  எனில் (type->size > 2 * __SIZEOF_POINTER__) {
    எனில் (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } இல்லையெனில் {
      /* copy to stack and pass by reference */
      data = memcpy (cb->struct_அடுக்கு, data, type->size);
      cb->struct_அடுக்கு = (size_t *) FFI_ALIGN ((எழுத்து *) cb->struct_அடுக்கு + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#இல்லையெனில்
  எனில் (type->type == FFI_TYPE_STRUCT) {
    எனில் (var) {
      எனில் (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } இல்லையெனில் {
      முழு i;
      
      க்கு (i = 0; i < type->size; i += அளவு(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += அளவு(size_t);
      }
    }
  }
#endif
  இல்லையெனில் எனில் (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } இல்லையெனில் {
      memcpy(realign, data, type->size);
      எனில் (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      எனில் (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

நிலையான வெறுமை unmarshal_atom(call_builder *cb, முழு type, வெறுமை *data) {
  size_t மதிப்பு;

  எனில் (cb->used_integer == NARGREG) {
    மதிப்பு = *cb->used_அடுக்கு++;
  } இல்லையெனில் {
    மதிப்பு = cb->aregs->r[cb->used_integer++];
  }

  தேர்வு (type) {
    நிலை FFI_TYPE_UINT8: *(uint8_t *)data = மதிப்பு; நிறுத்து;
    நிலை FFI_TYPE_SINT8: *(uint8_t *)data = மதிப்பு; நிறுத்து;
    நிலை FFI_TYPE_UINT16: *(uint16_t *)data = மதிப்பு; நிறுத்து;
    நிலை FFI_TYPE_SINT16: *(uint16_t *)data = மதிப்பு; நிறுத்து;
    நிலை FFI_TYPE_UINT32: *(uint32_t *)data = மதிப்பு; நிறுத்து;
    நிலை FFI_TYPE_SINT32: *(uint32_t *)data = மதிப்பு; நிறுத்து;
#எனில் defined(__ARC64_ARCH64__)
    நிலை FFI_TYPE_UINT64: *(uint64_t *)data = மதிப்பு; நிறுத்து;
    நிலை FFI_TYPE_SINT64: *(uint64_t *)data = மதிப்பு; நிறுத்து;
#endif
    நிலை FFI_TYPE_POINTER: *(size_t *)data = மதிப்பு; நிறுத்து;
    இயல்பு: FFI_ASSERT(0); நிறுத்து;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
நிலையான வெறுமை *unmarshal(call_builder *cb, ffi_type *type, முழு var, வெறுமை *data) {
  size_t realign[2];

#எனில் defined(__ARC64_ARCH64__)
  வெறுமை *pointer;

  எனில் (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (எழுத்து*)&pointer);
        திரும்பு pointer;
    }
#அல்லதெனில் defined(__ARC64_ARCH32__)
  எனில் (type->type == FFI_TYPE_STRUCT) {
    எனில் (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(data, (மாறிலி வெறுமை*)realign[0], type->size);
      திரும்பு data;
    } இல்லையெனில் {
      முழு i;
      வெறுமை *pdata = data;

      க்கு (i = 0; i < type->size; i += அளவு(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += அளவு(size_t);
      }
      திரும்பு data;
    }
  }
#இல்லையெனில்
  எனில் (type->type == FFI_TYPE_STRUCT) {

      எனில் (var) {
        முழு i;
        வெறுமை *pdata = data;

        க்கு (i = 0; i < type->size; i += அளவு(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += அளவு(size_t);
        }
        திரும்பு data;
      } இல்லையெனில் {
        எனில் (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(data, (மாறிலி வெறுமை*)realign[0], type->size);
        திரும்பு data;
      }
  }
#endif
  இல்லையெனில் எனில் (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    திரும்பு data;
  } இல்லையெனில் {
      எனில் (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      எனில் (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(data, realign, type->size);
      திரும்பு data;
  }
}

நிலையான முழு passed_by_ref(ffi_type *type, முழு var) {
  எனில் (type->type == FFI_TYPE_STRUCT)
	  திரும்பு 1;
  
  திரும்பு type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
வெளி வெறுமை ffi_call_asm (வெறுமை *அடுக்கு, கட்டமைப்பு call_context *regs,
			  வெறுமை (*fn) (வெறுமை), வெறுமை *closure) FFI_HIDDEN;

நிலையான வெறுமை
ffi_call_int (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue,
	      வெறுமை *closure)
{
  முழு return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * அளவு(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + அளவு(call_context);
  size_t alloc_base = (size_t)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_அடுக்கு = (வெறுமை*)alloc_base;
  cb.struct_அடுக்கு = (வெறுமை *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  எனில் (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  முழு i;
  க்கு (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((வெறுமை *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  எனில் (!return_by_ref && rvalue)
    {
	    எனில் (IS_INT(cif->rtype->type)
	        && cif->rtype->size < அளவு (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    தேர்வு (cif->rtype->type) {
	      நிலை FFI_TYPE_SINT8:
	      நிலை FFI_TYPE_SINT16:
	      நிலை FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (அளவு (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      நிறுத்து;
	      நிலை FFI_TYPE_UINT8:
	      நிலை FFI_TYPE_UINT16:
	      நிலை FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (அளவு (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      நிறுத்து;
	      }
	    }
	    இல்லையெனில்
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, வெற்று);
}

வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

வெளி வெறுமை ffi_closure_asm(வெறுமை) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		      வெறுமை *user_data, வெறுமை *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#எனில் defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  தேர்வு (cif->abi)
    {
#எனில் defined(__ARC64_ARCH64__)
    நிலை FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      நிறுத்து;
#இல்லையெனில்
    நிலை FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      நிறுத்து;
#endif

    இயல்பு:
      திரும்பு FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  திரும்பு FFI_OK;
}

வெளி வெறுமை ffi_go_closure_asm (வெறுமை) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *))
{
  எனில் (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    திரும்பு FFI_BAD_ABI;

  closure->tramp = (வெறுமை *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
வெறுமை FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		   வெறுமை *user_data,
		   size_t *அடுக்கு, call_context *aregs)
{
    வெறுமை **avalue = alloca(cif->nargs * அளவு(வெறுமை*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    எழுத்து *astorage = alloca(cif->bytes);
    எழுத்து *ptr = astorage;
    வெறுமை *rvalue;
    call_builder cb;
    முழு i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_அடுக்கு = அடுக்கு;
    
    /* handle hidden argument */
    எனில் (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    இல்லையெனில்
      rvalue = alloca(cif->rtype->size);

    க்கு (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    எனில் (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
