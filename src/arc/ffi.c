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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdint.h>

#অন্তর্ভাৱ <sys/cachectl.h>

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * পরিমাণ(দ্ৱিগুণ))

প্রকারনির্দেশ সংরচনা call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  ৱর্ণ frame[16];
} call_context;

প্রকারনির্দেশ সংরচনা call_builder
{
  call_context *aregs;
  পূর্ণাঙ্ক used_integer;
  //int used_float;
  size_t *used_স্তূপ;
  শূন্য *struct_স্তূপ;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#যদি defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#অন্যথা
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#যদি __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#অন্যথা
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  ৱিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      ভঙ্গ;

    পক্ষ FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
      ভঙ্গ;

    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ভঙ্গ;

    পক্ষ FFI_TYPE_FLOAT:
    স্ৱভাৱ:
      cif->flags = FFI_TYPE_INT;
      ভঙ্গ;
    }

  ঘূৰাই FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
স্থির শূন্য marshal_atom(call_builder *cb, পূর্ণাঙ্ক type, শূন্য *data) {
  size_t মান = 0;
  ৱিকল্প (type) {
    পক্ষ FFI_TYPE_UINT8: মান = *(uint8_t *)data; ভঙ্গ;
    পক্ষ FFI_TYPE_SINT8: মান = *(int8_t *)data; ভঙ্গ;
    পক্ষ FFI_TYPE_UINT16: মান = *(uint16_t *)data; ভঙ্গ;
    পক্ষ FFI_TYPE_SINT16: মান = *(int16_t *)data; ভঙ্গ;
    /* 32-bit quantities are always sign-extended in the ABI */
    পক্ষ FFI_TYPE_UINT32: মান = *(int32_t *)data; ভঙ্গ;
    পক্ষ FFI_TYPE_SINT32: মান = *(int32_t *)data; ভঙ্গ;
#যদি defined(__ARC64_ARCH64__)
    পক্ষ FFI_TYPE_UINT64: মান = *(uint64_t *)data; ভঙ্গ;
    পক্ষ FFI_TYPE_SINT64: মান = *(int64_t *)data; ভঙ্গ;
#endif
    পক্ষ FFI_TYPE_POINTER: মান = *(size_t *)data; ভঙ্গ;
    স্ৱভাৱ: FFI_ASSERT(0); ভঙ্গ;
  }

  যদি (cb->used_integer == NARGREG) {
    *cb->used_স্তূপ++ = মান;
  } অন্যথা {
    cb->aregs->r[cb->used_integer++] = মান;
  }
}

/* adds an argument to a call, or a not by reference return value */
স্থির শূন্য marshal(call_builder *cb, ffi_type *type, পূর্ণাঙ্ক var, শূন্য *data) {
  size_t realign[2];

#যদি (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  যদি (type->size > 2 * __SIZEOF_POINTER__) {
    যদি (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } অন্যথা {
      /* copy to stack and pass by reference */
      data = memcpy (cb->struct_স্তূপ, data, type->size);
      cb->struct_স্তূপ = (size_t *) FFI_ALIGN ((ৱর্ণ *) cb->struct_স্তূপ + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#অন্যথা
  যদি (type->type == FFI_TYPE_STRUCT) {
    যদি (var) {
      যদি (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } অন্যথা {
      পূর্ণাঙ্ক i;
      
      বাবে (i = 0; i < type->size; i += পরিমাণ(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += পরিমাণ(size_t);
      }
    }
  }
#endif
  অন্যথা যদি (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } অন্যথা {
      memcpy(realign, data, type->size);
      যদি (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      যদি (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

স্থির শূন্য unmarshal_atom(call_builder *cb, পূর্ণাঙ্ক type, শূন্য *data) {
  size_t মান;

  যদি (cb->used_integer == NARGREG) {
    মান = *cb->used_স্তূপ++;
  } অন্যথা {
    মান = cb->aregs->r[cb->used_integer++];
  }

  ৱিকল্প (type) {
    পক্ষ FFI_TYPE_UINT8: *(uint8_t *)data = মান; ভঙ্গ;
    পক্ষ FFI_TYPE_SINT8: *(uint8_t *)data = মান; ভঙ্গ;
    পক্ষ FFI_TYPE_UINT16: *(uint16_t *)data = মান; ভঙ্গ;
    পক্ষ FFI_TYPE_SINT16: *(uint16_t *)data = মান; ভঙ্গ;
    পক্ষ FFI_TYPE_UINT32: *(uint32_t *)data = মান; ভঙ্গ;
    পক্ষ FFI_TYPE_SINT32: *(uint32_t *)data = মান; ভঙ্গ;
#যদি defined(__ARC64_ARCH64__)
    পক্ষ FFI_TYPE_UINT64: *(uint64_t *)data = মান; ভঙ্গ;
    পক্ষ FFI_TYPE_SINT64: *(uint64_t *)data = মান; ভঙ্গ;
#endif
    পক্ষ FFI_TYPE_POINTER: *(size_t *)data = মান; ভঙ্গ;
    স্ৱভাৱ: FFI_ASSERT(0); ভঙ্গ;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
স্থির শূন্য *unmarshal(call_builder *cb, ffi_type *type, পূর্ণাঙ্ক var, শূন্য *data) {
  size_t realign[2];

#যদি defined(__ARC64_ARCH64__)
  শূন্য *pointer;

  যদি (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (ৱর্ণ*)&pointer);
        ঘূৰাই pointer;
    }
#নোচেদ্যদি defined(__ARC64_ARCH32__)
  যদি (type->type == FFI_TYPE_STRUCT) {
    যদি (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(data, (ধ্রুৱ শূন্য*)realign[0], type->size);
      ঘূৰাই data;
    } অন্যথা {
      পূর্ণাঙ্ক i;
      শূন্য *pdata = data;

      বাবে (i = 0; i < type->size; i += পরিমাণ(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += পরিমাণ(size_t);
      }
      ঘূৰাই data;
    }
  }
#অন্যথা
  যদি (type->type == FFI_TYPE_STRUCT) {

      যদি (var) {
        পূর্ণাঙ্ক i;
        শূন্য *pdata = data;

        বাবে (i = 0; i < type->size; i += পরিমাণ(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += পরিমাণ(size_t);
        }
        ঘূৰাই data;
      } অন্যথা {
        যদি (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(data, (ধ্রুৱ শূন্য*)realign[0], type->size);
        ঘূৰাই data;
      }
  }
#endif
  অন্যথা যদি (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    ঘূৰাই data;
  } অন্যথা {
      যদি (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      যদি (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(data, realign, type->size);
      ঘূৰাই data;
  }
}

স্থির পূর্ণাঙ্ক passed_by_ref(ffi_type *type, পূর্ণাঙ্ক var) {
  যদি (type->type == FFI_TYPE_STRUCT)
	  ঘূৰাই 1;
  
  ঘূৰাই type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
বাহ্য শূন্য ffi_call_asm (শূন্য *স্তূপ, সংরচনা call_context *regs,
			  শূন্য (*fn) (শূন্য), শূন্য *closure) FFI_HIDDEN;

স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue,
	      শূন্য *closure)
{
  পূর্ণাঙ্ক return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * পরিমাণ(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + পরিমাণ(call_context);
  size_t alloc_base = (size_t)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_স্তূপ = (শূন্য*)alloc_base;
  cb.struct_স্তূপ = (শূন্য *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  যদি (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  পূর্ণাঙ্ক i;
  বাবে (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((শূন্য *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  যদি (!return_by_ref && rvalue)
    {
	    যদি (IS_INT(cif->rtype->type)
	        && cif->rtype->size < পরিমাণ (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    ৱিকল্প (cif->rtype->type) {
	      পক্ষ FFI_TYPE_SINT8:
	      পক্ষ FFI_TYPE_SINT16:
	      পক্ষ FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (পরিমাণ (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      ভঙ্গ;
	      পক্ষ FFI_TYPE_UINT8:
	      পক্ষ FFI_TYPE_UINT16:
	      পক্ষ FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (পরিমাণ (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      ভঙ্গ;
	      }
	    }
	    অন্যথা
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, রিক্ত);
}

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

বাহ্য শূন্য ffi_closure_asm(শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		      শূন্য *user_data, শূন্য *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#যদি defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  ৱিকল্প (cif->abi)
    {
#যদি defined(__ARC64_ARCH64__)
    পক্ষ FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      ভঙ্গ;
#অন্যথা
    পক্ষ FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      ভঙ্গ;
#endif

    স্ৱভাৱ:
      ঘূৰাই FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  ঘূৰাই FFI_OK;
}

বাহ্য শূন্য ffi_go_closure_asm (শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *))
{
  যদি (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ঘূৰাই FFI_BAD_ABI;

  closure->tramp = (শূন্য *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ঘূৰাই FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
শূন্য FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		   শূন্য *user_data,
		   size_t *স্তূপ, call_context *aregs)
{
    শূন্য **avalue = alloca(cif->nargs * পরিমাণ(শূন্য*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    ৱর্ণ *astorage = alloca(cif->bytes);
    ৱর্ণ *ptr = astorage;
    শূন্য *rvalue;
    call_builder cb;
    পূর্ণাঙ্ক i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_স্তূপ = স্তূপ;
    
    /* handle hidden argument */
    যদি (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    অন্যথা
      rvalue = alloca(cif->rtype->size);

    বাবে (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    যদি (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
