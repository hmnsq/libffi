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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>

#ଅନ୍ତର୍ଭାଵ <sys/cachectl.h>

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * ଆକାର(ଦ୍ୱିଗୁଣ))

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  ଅକ୍ଷର frame[16];
} call_context;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା call_builder
{
  call_context *aregs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା used_integer;
  //int used_float;
  size_t *used_ଗଦା;
  ଶୂନ୍ୟ *struct_ଗଦା;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ଯଦି defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ଅନ୍ୟଥା
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#ଯଦି __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#ଅନ୍ୟଥା
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_FLOAT:
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      cif->flags = FFI_TYPE_INT;
      ଭାଙ୍ଗ;
    }

  ଫେରନ୍ତୁ FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ସ୍ଥିର ଶୂନ୍ୟ marshal_atom(call_builder *cb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ଶୂନ୍ୟ *data) {
  size_t ମୂଲ୍ୟ = 0;
  ବାଛ (type) {
    ସ୍ଥିତି FFI_TYPE_UINT8: ମୂଲ୍ୟ = *(uint8_t *)data; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT8: ମୂଲ୍ୟ = *(int8_t *)data; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT16: ମୂଲ୍ୟ = *(uint16_t *)data; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT16: ମୂଲ୍ୟ = *(int16_t *)data; ଭାଙ୍ଗ;
    /* 32-bit quantities are always sign-extended in the ABI */
    ସ୍ଥିତି FFI_TYPE_UINT32: ମୂଲ୍ୟ = *(int32_t *)data; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT32: ମୂଲ୍ୟ = *(int32_t *)data; ଭାଙ୍ଗ;
#ଯଦି defined(__ARC64_ARCH64__)
    ସ୍ଥିତି FFI_TYPE_UINT64: ମୂଲ୍ୟ = *(uint64_t *)data; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT64: ମୂଲ୍ୟ = *(int64_t *)data; ଭାଙ୍ଗ;
#endif
    ସ୍ଥିତି FFI_TYPE_POINTER: ମୂଲ୍ୟ = *(size_t *)data; ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: FFI_ASSERT(0); ଭାଙ୍ଗ;
  }

  ଯଦି (cb->used_integer == NARGREG) {
    *cb->used_ଗଦା++ = ମୂଲ୍ୟ;
  } ଅନ୍ୟଥା {
    cb->aregs->r[cb->used_integer++] = ମୂଲ୍ୟ;
  }
}

/* adds an argument to a call, or a not by reference return value */
ସ୍ଥିର ଶୂନ୍ୟ marshal(call_builder *cb, ffi_type *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା var, ଶୂନ୍ୟ *data) {
  size_t realign[2];

#ଯଦି (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  ଯଦି (type->size > 2 * __SIZEOF_POINTER__) {
    ଯଦି (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } ଅନ୍ୟଥା {
      /* copy to stack and pass by reference */
      data = memcpy (cb->struct_ଗଦା, data, type->size);
      cb->struct_ଗଦା = (size_t *) FFI_ALIGN ((ଅକ୍ଷର *) cb->struct_ଗଦା + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#ଅନ୍ୟଥା
  ଯଦି (type->type == FFI_TYPE_STRUCT) {
    ଯଦି (var) {
      ଯଦି (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } ଅନ୍ୟଥା {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      
      ପାଇଁ (i = 0; i < type->size; i += ଆକାର(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += ଆକାର(size_t);
      }
    }
  }
#endif
  ଅନ୍ୟଥା ଯଦି (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } ଅନ୍ୟଥା {
      memcpy(realign, data, type->size);
      ଯଦି (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      ଯଦି (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

ସ୍ଥିର ଶୂନ୍ୟ unmarshal_atom(call_builder *cb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ଶୂନ୍ୟ *data) {
  size_t ମୂଲ୍ୟ;

  ଯଦି (cb->used_integer == NARGREG) {
    ମୂଲ୍ୟ = *cb->used_ଗଦା++;
  } ଅନ୍ୟଥା {
    ମୂଲ୍ୟ = cb->aregs->r[cb->used_integer++];
  }

  ବାଛ (type) {
    ସ୍ଥିତି FFI_TYPE_UINT8: *(uint8_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT8: *(uint8_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT16: *(uint16_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT16: *(uint16_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT32: *(uint32_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT32: *(uint32_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
#ଯଦି defined(__ARC64_ARCH64__)
    ସ୍ଥିତି FFI_TYPE_UINT64: *(uint64_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT64: *(uint64_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
#endif
    ସ୍ଥିତି FFI_TYPE_POINTER: *(size_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: FFI_ASSERT(0); ଭାଙ୍ଗ;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ସ୍ଥିର ଶୂନ୍ୟ *unmarshal(call_builder *cb, ffi_type *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା var, ଶୂନ୍ୟ *data) {
  size_t realign[2];

#ଯଦି defined(__ARC64_ARCH64__)
  ଶୂନ୍ୟ *pointer;

  ଯଦି (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (ଅକ୍ଷର*)&pointer);
        ଫେରନ୍ତୁ pointer;
    }
#ନଚେତଯଦି defined(__ARC64_ARCH32__)
  ଯଦି (type->type == FFI_TYPE_STRUCT) {
    ଯଦି (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(data, (ଅଚଳ ଶୂନ୍ୟ*)realign[0], type->size);
      ଫେରନ୍ତୁ data;
    } ଅନ୍ୟଥା {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ଶୂନ୍ୟ *pdata = data;

      ପାଇଁ (i = 0; i < type->size; i += ଆକାର(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += ଆକାର(size_t);
      }
      ଫେରନ୍ତୁ data;
    }
  }
#ଅନ୍ୟଥା
  ଯଦି (type->type == FFI_TYPE_STRUCT) {

      ଯଦି (var) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ଶୂନ୍ୟ *pdata = data;

        ପାଇଁ (i = 0; i < type->size; i += ଆକାର(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += ଆକାର(size_t);
        }
        ଫେରନ୍ତୁ data;
      } ଅନ୍ୟଥା {
        ଯଦି (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(data, (ଅଚଳ ଶୂନ୍ୟ*)realign[0], type->size);
        ଫେରନ୍ତୁ data;
      }
  }
#endif
  ଅନ୍ୟଥା ଯଦି (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    ଫେରନ୍ତୁ data;
  } ଅନ୍ୟଥା {
      ଯଦି (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      ଯଦି (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(data, realign, type->size);
      ଫେରନ୍ତୁ data;
  }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା passed_by_ref(ffi_type *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା var) {
  ଯଦି (type->type == FFI_TYPE_STRUCT)
	  ଫେରନ୍ତୁ 1;
  
  ଫେରନ୍ତୁ type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_asm (ଶୂନ୍ୟ *ଗଦା, ସଂରଚନା call_context *regs,
			  ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *closure) FFI_HIDDEN;

ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue,
	      ଶୂନ୍ୟ *closure)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * ଆକାର(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + ଆକାର(call_context);
  size_t alloc_base = (size_t)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_ଗଦା = (ଶୂନ୍ୟ*)alloc_base;
  cb.struct_ଗଦା = (ଶୂନ୍ୟ *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  ଯଦି (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((ଶୂନ୍ୟ *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  ଯଦି (!return_by_ref && rvalue)
    {
	    ଯଦି (IS_INT(cif->rtype->type)
	        && cif->rtype->size < ଆକାର (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    ବାଛ (cif->rtype->type) {
	      ସ୍ଥିତି FFI_TYPE_SINT8:
	      ସ୍ଥିତି FFI_TYPE_SINT16:
	      ସ୍ଥିତି FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (ଆକାର (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      ଭାଙ୍ଗ;
	      ସ୍ଥିତି FFI_TYPE_UINT8:
	      ସ୍ଥିତି FFI_TYPE_UINT16:
	      ସ୍ଥିତି FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (ଆକାର (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      ଭାଙ୍ଗ;
	      }
	    }
	    ଅନ୍ୟଥା
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ଖାଲି);
}

ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_asm(ଶୂନ୍ୟ) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
		      ଶୂନ୍ୟ *user_data, ଶୂନ୍ୟ *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#ଯଦି defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  ବାଛ (cif->abi)
    {
#ଯଦି defined(__ARC64_ARCH64__)
    ସ୍ଥିତି FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      ଭାଙ୍ଗ;
#ଅନ୍ୟଥା
    ସ୍ଥିତି FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      ଭାଙ୍ଗ;
#endif

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_asm (ଶୂନ୍ୟ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *))
{
  ଯଦି (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  closure->tramp = (ଶୂନ୍ୟ *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
ଶୂନ୍ୟ FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
		   ଶୂନ୍ୟ *user_data,
		   size_t *ଗଦା, call_context *aregs)
{
    ଶୂନ୍ୟ **avalue = alloca(cif->nargs * ଆକାର(ଶୂନ୍ୟ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    ଅକ୍ଷର *astorage = alloca(cif->bytes);
    ଅକ୍ଷର *ptr = astorage;
    ଶୂନ୍ୟ *rvalue;
    call_builder cb;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_ଗଦା = ଗଦା;
    
    /* handle hidden argument */
    ଯଦି (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    ଅନ୍ୟଥା
      rvalue = alloca(cif->rtype->size);

    ପାଇଁ (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    ଯଦି (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
