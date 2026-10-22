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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>

#ಅನ್ತರ್ಭಾವ <sys/cachectl.h>

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * ಪರಿಮಾಣ(ದ್ವಿಗುಣ))

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  ವರ್ಣ frame[16];
} call_context;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ call_builder
{
  call_context *aregs;
  ಪೂರ್ಣಾಙ್ಕ used_integer;
  //int used_float;
  size_t *used_ರಾಶಿ;
  ಶೂನ್ಯ *struct_ರಾಶಿ;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ಯದಿ defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ಅನ್ಯಥಾ
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#ಯದಿ __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#ಅನ್ಯಥಾ
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಸ್ವಭಾವ:
      cif->flags = FFI_TYPE_INT;
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ಸ್ಥಿರ ಶೂನ್ಯ marshal_atom(call_builder *cb, ಪೂರ್ಣಾಙ್ಕ type, ಶೂನ್ಯ *data) {
  size_t ಮೌಲ್ಯ = 0;
  ವಿಕಲ್ಪ (type) {
    ಪಕ್ಷ FFI_TYPE_UINT8: ಮೌಲ್ಯ = *(uint8_t *)data; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT8: ಮೌಲ್ಯ = *(int8_t *)data; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT16: ಮೌಲ್ಯ = *(uint16_t *)data; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT16: ಮೌಲ್ಯ = *(int16_t *)data; ಭಙ್ಗ;
    /* 32-bit quantities are always sign-extended in the ABI */
    ಪಕ್ಷ FFI_TYPE_UINT32: ಮೌಲ್ಯ = *(int32_t *)data; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT32: ಮೌಲ್ಯ = *(int32_t *)data; ಭಙ್ಗ;
#ಯದಿ defined(__ARC64_ARCH64__)
    ಪಕ್ಷ FFI_TYPE_UINT64: ಮೌಲ್ಯ = *(uint64_t *)data; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT64: ಮೌಲ್ಯ = *(int64_t *)data; ಭಙ್ಗ;
#endif
    ಪಕ್ಷ FFI_TYPE_POINTER: ಮೌಲ್ಯ = *(size_t *)data; ಭಙ್ಗ;
    ಸ್ವಭಾವ: FFI_ASSERT(0); ಭಙ್ಗ;
  }

  ಯದಿ (cb->used_integer == NARGREG) {
    *cb->used_ರಾಶಿ++ = ಮೌಲ್ಯ;
  } ಅನ್ಯಥಾ {
    cb->aregs->r[cb->used_integer++] = ಮೌಲ್ಯ;
  }
}

/* adds an argument to a call, or a not by reference return value */
ಸ್ಥಿರ ಶೂನ್ಯ marshal(call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಙ್ಕ var, ಶೂನ್ಯ *data) {
  size_t realign[2];

#ಯದಿ (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  ಯದಿ (type->size > 2 * __SIZEOF_POINTER__) {
    ಯದಿ (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } ಅನ್ಯಥಾ {
      /* copy to stack and pass by reference */
      data = memcpy (cb->struct_ರಾಶಿ, data, type->size);
      cb->struct_ರಾಶಿ = (size_t *) FFI_ALIGN ((ವರ್ಣ *) cb->struct_ರಾಶಿ + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#ಅನ್ಯಥಾ
  ಯದಿ (type->type == FFI_TYPE_STRUCT) {
    ಯದಿ (var) {
      ಯದಿ (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } ಅನ್ಯಥಾ {
      ಪೂರ್ಣಾಙ್ಕ i;
      
      ಪ್ರತಿ (i = 0; i < type->size; i += ಪರಿಮಾಣ(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += ಪರಿಮಾಣ(size_t);
      }
    }
  }
#endif
  ಅನ್ಯಥಾ ಯದಿ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } ಅನ್ಯಥಾ {
      memcpy(realign, data, type->size);
      ಯದಿ (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      ಯದಿ (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ unmarshal_atom(call_builder *cb, ಪೂರ್ಣಾಙ್ಕ type, ಶೂನ್ಯ *data) {
  size_t ಮೌಲ್ಯ;

  ಯದಿ (cb->used_integer == NARGREG) {
    ಮೌಲ್ಯ = *cb->used_ರಾಶಿ++;
  } ಅನ್ಯಥಾ {
    ಮೌಲ್ಯ = cb->aregs->r[cb->used_integer++];
  }

  ವಿಕಲ್ಪ (type) {
    ಪಕ್ಷ FFI_TYPE_UINT8: *(uint8_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT8: *(uint8_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT16: *(uint16_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT16: *(uint16_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT32: *(uint32_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT32: *(uint32_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
#ಯದಿ defined(__ARC64_ARCH64__)
    ಪಕ್ಷ FFI_TYPE_UINT64: *(uint64_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT64: *(uint64_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
#endif
    ಪಕ್ಷ FFI_TYPE_POINTER: *(size_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
    ಸ್ವಭಾವ: FFI_ASSERT(0); ಭಙ್ಗ;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ಸ್ಥಿರ ಶೂನ್ಯ *unmarshal(call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಙ್ಕ var, ಶೂನ್ಯ *data) {
  size_t realign[2];

#ಯದಿ defined(__ARC64_ARCH64__)
  ಶೂನ್ಯ *pointer;

  ಯದಿ (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (ವರ್ಣ*)&pointer);
        ಪ್ರತ್ಯಾಗಮ pointer;
    }
#ನೋಚೇದ್ಯದಿ defined(__ARC64_ARCH32__)
  ಯದಿ (type->type == FFI_TYPE_STRUCT) {
    ಯದಿ (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(data, (ಧ್ರುವ ಶೂನ್ಯ*)realign[0], type->size);
      ಪ್ರತ್ಯಾಗಮ data;
    } ಅನ್ಯಥಾ {
      ಪೂರ್ಣಾಙ್ಕ i;
      ಶೂನ್ಯ *pdata = data;

      ಪ್ರತಿ (i = 0; i < type->size; i += ಪರಿಮಾಣ(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += ಪರಿಮಾಣ(size_t);
      }
      ಪ್ರತ್ಯಾಗಮ data;
    }
  }
#ಅನ್ಯಥಾ
  ಯದಿ (type->type == FFI_TYPE_STRUCT) {

      ಯದಿ (var) {
        ಪೂರ್ಣಾಙ್ಕ i;
        ಶೂನ್ಯ *pdata = data;

        ಪ್ರತಿ (i = 0; i < type->size; i += ಪರಿಮಾಣ(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += ಪರಿಮಾಣ(size_t);
        }
        ಪ್ರತ್ಯಾಗಮ data;
      } ಅನ್ಯಥಾ {
        ಯದಿ (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(data, (ಧ್ರುವ ಶೂನ್ಯ*)realign[0], type->size);
        ಪ್ರತ್ಯಾಗಮ data;
      }
  }
#endif
  ಅನ್ಯಥಾ ಯದಿ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    ಪ್ರತ್ಯಾಗಮ data;
  } ಅನ್ಯಥಾ {
      ಯದಿ (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      ಯದಿ (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(data, realign, type->size);
      ಪ್ರತ್ಯಾಗಮ data;
  }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ passed_by_ref(ffi_type *type, ಪೂರ್ಣಾಙ್ಕ var) {
  ಯದಿ (type->type == FFI_TYPE_STRUCT)
	  ಪ್ರತ್ಯಾಗಮ 1;
  
  ಪ್ರತ್ಯಾಗಮ type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_asm (ಶೂನ್ಯ *ರಾಶಿ, ಸಂರಚನಾ call_context *regs,
			  ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *closure) FFI_HIDDEN;

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue,
	      ಶೂನ್ಯ *closure)
{
  ಪೂರ್ಣಾಙ್ಕ return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * ಪರಿಮಾಣ(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + ಪರಿಮಾಣ(call_context);
  size_t alloc_base = (size_t)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_ರಾಶಿ = (ಶೂನ್ಯ*)alloc_base;
  cb.struct_ರಾಶಿ = (ಶೂನ್ಯ *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  ಯದಿ (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((ಶೂನ್ಯ *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  ಯದಿ (!return_by_ref && rvalue)
    {
	    ಯದಿ (IS_INT(cif->rtype->type)
	        && cif->rtype->size < ಪರಿಮಾಣ (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    ವಿಕಲ್ಪ (cif->rtype->type) {
	      ಪಕ್ಷ FFI_TYPE_SINT8:
	      ಪಕ್ಷ FFI_TYPE_SINT16:
	      ಪಕ್ಷ FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (ಪರಿಮಾಣ (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      ಭಙ್ಗ;
	      ಪಕ್ಷ FFI_TYPE_UINT8:
	      ಪಕ್ಷ FFI_TYPE_UINT16:
	      ಪಕ್ಷ FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (ಪರಿಮಾಣ (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      ಭಙ್ಗ;
	      }
	    }
	    ಅನ್ಯಥಾ
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ರಿಕ್ತ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_asm(ಶೂನ್ಯ) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_data, ಶೂನ್ಯ *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#ಯದಿ defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  ವಿಕಲ್ಪ (cif->abi)
    {
#ಯದಿ defined(__ARC64_ARCH64__)
    ಪಕ್ಷ FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      ಭಙ್ಗ;
#ಅನ್ಯಥಾ
    ಪಕ್ಷ FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      ಭಙ್ಗ;
#endif

    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_asm (ಶೂನ್ಯ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *))
{
  ಯದಿ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  closure->tramp = (ಶೂನ್ಯ *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
ಶೂನ್ಯ FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		   ಶೂನ್ಯ *user_data,
		   size_t *ರಾಶಿ, call_context *aregs)
{
    ಶೂನ್ಯ **avalue = alloca(cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    ವರ್ಣ *astorage = alloca(cif->bytes);
    ವರ್ಣ *ptr = astorage;
    ಶೂನ್ಯ *rvalue;
    call_builder cb;
    ಪೂರ್ಣಾಙ್ಕ i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_ರಾಶಿ = ರಾಶಿ;
    
    /* handle hidden argument */
    ಯದಿ (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    ಅನ್ಯಥಾ
      rvalue = alloca(cif->rtype->size);

    ಪ್ರತಿ (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    ಯದಿ (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
