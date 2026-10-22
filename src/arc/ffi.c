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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdint.h>

#અન્તર્ભાવ <sys/cachectl.h>

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * કદ(દ્વિગુણ))

પ્રકારવ્યાખ્યા સંરચના call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  અક્ષર frame[16];
} call_context;

પ્રકારવ્યાખ્યા સંરચના call_builder
{
  call_context *aregs;
  પૂર્ણાંક used_integer;
  //int used_float;
  size_t *used_ઢગલો;
  શૂન્ય *struct_ઢગલો;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#જો defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#નહીંતર
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#જો __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#નહીંતર
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
      તોડો;

    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      તોડો;

    સ્થિતિ FFI_TYPE_FLOAT:
    મૂળભૂત:
      cif->flags = FFI_TYPE_INT;
      તોડો;
    }

  પાછા FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
સ્થિર શૂન્ય marshal_atom(call_builder *cb, પૂર્ણાંક type, શૂન્ય *data) {
  size_t કિંમત = 0;
  પસંદગી (type) {
    સ્થિતિ FFI_TYPE_UINT8: કિંમત = *(uint8_t *)data; તોડો;
    સ્થિતિ FFI_TYPE_SINT8: કિંમત = *(int8_t *)data; તોડો;
    સ્થિતિ FFI_TYPE_UINT16: કિંમત = *(uint16_t *)data; તોડો;
    સ્થિતિ FFI_TYPE_SINT16: કિંમત = *(int16_t *)data; તોડો;
    /* 32-bit quantities are always sign-extended in the ABI */
    સ્થિતિ FFI_TYPE_UINT32: કિંમત = *(int32_t *)data; તોડો;
    સ્થિતિ FFI_TYPE_SINT32: કિંમત = *(int32_t *)data; તોડો;
#જો defined(__ARC64_ARCH64__)
    સ્થિતિ FFI_TYPE_UINT64: કિંમત = *(uint64_t *)data; તોડો;
    સ્થિતિ FFI_TYPE_SINT64: કિંમત = *(int64_t *)data; તોડો;
#endif
    સ્થિતિ FFI_TYPE_POINTER: કિંમત = *(size_t *)data; તોડો;
    મૂળભૂત: FFI_ASSERT(0); તોડો;
  }

  જો (cb->used_integer == NARGREG) {
    *cb->used_ઢગલો++ = કિંમત;
  } નહીંતર {
    cb->aregs->r[cb->used_integer++] = કિંમત;
  }
}

/* adds an argument to a call, or a not by reference return value */
સ્થિર શૂન્ય marshal(call_builder *cb, ffi_type *type, પૂર્ણાંક var, શૂન્ય *data) {
  size_t realign[2];

#જો (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  જો (type->size > 2 * __SIZEOF_POINTER__) {
    જો (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } નહીંતર {
      /* copy to stack and pass by reference */
      data = memcpy (cb->struct_ઢગલો, data, type->size);
      cb->struct_ઢગલો = (size_t *) FFI_ALIGN ((અક્ષર *) cb->struct_ઢગલો + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#નહીંતર
  જો (type->type == FFI_TYPE_STRUCT) {
    જો (var) {
      જો (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } નહીંતર {
      પૂર્ણાંક i;
      
      માટે (i = 0; i < type->size; i += કદ(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += કદ(size_t);
      }
    }
  }
#endif
  નહીંતર જો (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } નહીંતર {
      memcpy(realign, data, type->size);
      જો (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      જો (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

સ્થિર શૂન્ય unmarshal_atom(call_builder *cb, પૂર્ણાંક type, શૂન્ય *data) {
  size_t કિંમત;

  જો (cb->used_integer == NARGREG) {
    કિંમત = *cb->used_ઢગલો++;
  } નહીંતર {
    કિંમત = cb->aregs->r[cb->used_integer++];
  }

  પસંદગી (type) {
    સ્થિતિ FFI_TYPE_UINT8: *(uint8_t *)data = કિંમત; તોડો;
    સ્થિતિ FFI_TYPE_SINT8: *(uint8_t *)data = કિંમત; તોડો;
    સ્થિતિ FFI_TYPE_UINT16: *(uint16_t *)data = કિંમત; તોડો;
    સ્થિતિ FFI_TYPE_SINT16: *(uint16_t *)data = કિંમત; તોડો;
    સ્થિતિ FFI_TYPE_UINT32: *(uint32_t *)data = કિંમત; તોડો;
    સ્થિતિ FFI_TYPE_SINT32: *(uint32_t *)data = કિંમત; તોડો;
#જો defined(__ARC64_ARCH64__)
    સ્થિતિ FFI_TYPE_UINT64: *(uint64_t *)data = કિંમત; તોડો;
    સ્થિતિ FFI_TYPE_SINT64: *(uint64_t *)data = કિંમત; તોડો;
#endif
    સ્થિતિ FFI_TYPE_POINTER: *(size_t *)data = કિંમત; તોડો;
    મૂળભૂત: FFI_ASSERT(0); તોડો;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
સ્થિર શૂન્ય *unmarshal(call_builder *cb, ffi_type *type, પૂર્ણાંક var, શૂન્ય *data) {
  size_t realign[2];

#જો defined(__ARC64_ARCH64__)
  શૂન્ય *pointer;

  જો (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (અક્ષર*)&pointer);
        પાછા pointer;
    }
#અથવાજો defined(__ARC64_ARCH32__)
  જો (type->type == FFI_TYPE_STRUCT) {
    જો (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(data, (અચલ શૂન્ય*)realign[0], type->size);
      પાછા data;
    } નહીંતર {
      પૂર્ણાંક i;
      શૂન્ય *pdata = data;

      માટે (i = 0; i < type->size; i += કદ(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += કદ(size_t);
      }
      પાછા data;
    }
  }
#નહીંતર
  જો (type->type == FFI_TYPE_STRUCT) {

      જો (var) {
        પૂર્ણાંક i;
        શૂન્ય *pdata = data;

        માટે (i = 0; i < type->size; i += કદ(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += કદ(size_t);
        }
        પાછા data;
      } નહીંતર {
        જો (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(data, (અચલ શૂન્ય*)realign[0], type->size);
        પાછા data;
      }
  }
#endif
  નહીંતર જો (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    પાછા data;
  } નહીંતર {
      જો (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      જો (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(data, realign, type->size);
      પાછા data;
  }
}

સ્થિર પૂર્ણાંક passed_by_ref(ffi_type *type, પૂર્ણાંક var) {
  જો (type->type == FFI_TYPE_STRUCT)
	  પાછા 1;
  
  પાછા type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
બાહ્ય શૂન્ય ffi_call_asm (શૂન્ય *ઢગલો, સંરચના call_context *regs,
			  શૂન્ય (*fn) (શૂન્ય), શૂન્ય *closure) FFI_HIDDEN;

સ્થિર શૂન્ય
ffi_call_int (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue,
	      શૂન્ય *closure)
{
  પૂર્ણાંક return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * કદ(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + કદ(call_context);
  size_t alloc_base = (size_t)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_ઢગલો = (શૂન્ય*)alloc_base;
  cb.struct_ઢગલો = (શૂન્ય *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  જો (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  પૂર્ણાંક i;
  માટે (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((શૂન્ય *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  જો (!return_by_ref && rvalue)
    {
	    જો (IS_INT(cif->rtype->type)
	        && cif->rtype->size < કદ (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    પસંદગી (cif->rtype->type) {
	      સ્થિતિ FFI_TYPE_SINT8:
	      સ્થિતિ FFI_TYPE_SINT16:
	      સ્થિતિ FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (કદ (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      તોડો;
	      સ્થિતિ FFI_TYPE_UINT8:
	      સ્થિતિ FFI_TYPE_UINT16:
	      સ્થિતિ FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (કદ (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      તોડો;
	      }
	    }
	    નહીંતર
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ખાલી);
}

શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

બાહ્ય શૂન્ય ffi_closure_asm(શૂન્ય) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
		      શૂન્ય *user_data, શૂન્ય *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#જો defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  પસંદગી (cif->abi)
    {
#જો defined(__ARC64_ARCH64__)
    સ્થિતિ FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      તોડો;
#નહીંતર
    સ્થિતિ FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      તોડો;
#endif

    મૂળભૂત:
      પાછા FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  પાછા FFI_OK;
}

બાહ્ય શૂન્ય ffi_go_closure_asm (શૂન્ય) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *))
{
  જો (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    પાછા FFI_BAD_ABI;

  closure->tramp = (શૂન્ય *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
શૂન્ય FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
		   શૂન્ય *user_data,
		   size_t *ઢગલો, call_context *aregs)
{
    શૂન્ય **avalue = alloca(cif->nargs * કદ(શૂન્ય*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    અક્ષર *astorage = alloca(cif->bytes);
    અક્ષર *ptr = astorage;
    શૂન્ય *rvalue;
    call_builder cb;
    પૂર્ણાંક i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_ઢગલો = ઢગલો;
    
    /* handle hidden argument */
    જો (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    નહીંતર
      rvalue = alloca(cif->rtype->size);

    માટે (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    જો (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
