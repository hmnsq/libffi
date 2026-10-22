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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>
#شامل <stdint.h>

#شامل <sys/cachectl.h>

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * اندازه(دوگانه))

تعریفنوع ساختار call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  نویسه frame[16];
} call_context;

تعریفنوع ساختار call_builder
{
  call_context *aregs;
  صحیح used_integer;
  //int used_float;
  size_t *used_پشته;
  تهی *struct_پشته;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#اگر defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وگرنه
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#اگر __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#وگرنه
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      بشکن;

    حالت FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
      بشکن;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      بشکن;

    حالت FFI_TYPE_FLOAT:
    پیشفرض:
      cif->flags = FFI_TYPE_INT;
      بشکن;
    }

  بازگشت FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ایستا تهی marshal_atom(call_builder *cb, صحیح type, تهی *data) {
  size_t مقدار = 0;
  انتخاب (type) {
    حالت FFI_TYPE_UINT8: مقدار = *(uint8_t *)data; بشکن;
    حالت FFI_TYPE_SINT8: مقدار = *(int8_t *)data; بشکن;
    حالت FFI_TYPE_UINT16: مقدار = *(uint16_t *)data; بشکن;
    حالت FFI_TYPE_SINT16: مقدار = *(int16_t *)data; بشکن;
    /* 32-bit quantities are always sign-extended in the ABI */
    حالت FFI_TYPE_UINT32: مقدار = *(int32_t *)data; بشکن;
    حالت FFI_TYPE_SINT32: مقدار = *(int32_t *)data; بشکن;
#اگر defined(__ARC64_ARCH64__)
    حالت FFI_TYPE_UINT64: مقدار = *(uint64_t *)data; بشکن;
    حالت FFI_TYPE_SINT64: مقدار = *(int64_t *)data; بشکن;
#endif
    حالت FFI_TYPE_POINTER: مقدار = *(size_t *)data; بشکن;
    پیشفرض: FFI_ASSERT(0); بشکن;
  }

  اگر (cb->used_integer == NARGREG) {
    *cb->used_پشته++ = مقدار;
  } وگرنه {
    cb->aregs->r[cb->used_integer++] = مقدار;
  }
}

/* adds an argument to a call, or a not by reference return value */
ایستا تهی marshal(call_builder *cb, ffi_type *type, صحیح var, تهی *data) {
  size_t realign[2];

#اگر (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  اگر (type->size > 2 * __SIZEOF_POINTER__) {
    اگر (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } وگرنه {
      /* copy to stack and pass by reference */
      data = memcpy (cb->struct_پشته, data, type->size);
      cb->struct_پشته = (size_t *) FFI_ALIGN ((نویسه *) cb->struct_پشته + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#وگرنه
  اگر (type->type == FFI_TYPE_STRUCT) {
    اگر (var) {
      اگر (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } وگرنه {
      صحیح i;
      
      برای (i = 0; i < type->size; i += اندازه(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += اندازه(size_t);
      }
    }
  }
#endif
  وگرنه اگر (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } وگرنه {
      memcpy(realign, data, type->size);
      اگر (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      اگر (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

ایستا تهی unmarshal_atom(call_builder *cb, صحیح type, تهی *data) {
  size_t مقدار;

  اگر (cb->used_integer == NARGREG) {
    مقدار = *cb->used_پشته++;
  } وگرنه {
    مقدار = cb->aregs->r[cb->used_integer++];
  }

  انتخاب (type) {
    حالت FFI_TYPE_UINT8: *(uint8_t *)data = مقدار; بشکن;
    حالت FFI_TYPE_SINT8: *(uint8_t *)data = مقدار; بشکن;
    حالت FFI_TYPE_UINT16: *(uint16_t *)data = مقدار; بشکن;
    حالت FFI_TYPE_SINT16: *(uint16_t *)data = مقدار; بشکن;
    حالت FFI_TYPE_UINT32: *(uint32_t *)data = مقدار; بشکن;
    حالت FFI_TYPE_SINT32: *(uint32_t *)data = مقدار; بشکن;
#اگر defined(__ARC64_ARCH64__)
    حالت FFI_TYPE_UINT64: *(uint64_t *)data = مقدار; بشکن;
    حالت FFI_TYPE_SINT64: *(uint64_t *)data = مقدار; بشکن;
#endif
    حالت FFI_TYPE_POINTER: *(size_t *)data = مقدار; بشکن;
    پیشفرض: FFI_ASSERT(0); بشکن;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ایستا تهی *unmarshal(call_builder *cb, ffi_type *type, صحیح var, تهی *data) {
  size_t realign[2];

#اگر defined(__ARC64_ARCH64__)
  تهی *pointer;

  اگر (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (نویسه*)&pointer);
        بازگشت pointer;
    }
#وگرنهاگر defined(__ARC64_ARCH32__)
  اگر (type->type == FFI_TYPE_STRUCT) {
    اگر (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(data, (ثابت تهی*)realign[0], type->size);
      بازگشت data;
    } وگرنه {
      صحیح i;
      تهی *pdata = data;

      برای (i = 0; i < type->size; i += اندازه(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += اندازه(size_t);
      }
      بازگشت data;
    }
  }
#وگرنه
  اگر (type->type == FFI_TYPE_STRUCT) {

      اگر (var) {
        صحیح i;
        تهی *pdata = data;

        برای (i = 0; i < type->size; i += اندازه(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += اندازه(size_t);
        }
        بازگشت data;
      } وگرنه {
        اگر (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(data, (ثابت تهی*)realign[0], type->size);
        بازگشت data;
      }
  }
#endif
  وگرنه اگر (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    بازگشت data;
  } وگرنه {
      اگر (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      اگر (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(data, realign, type->size);
      بازگشت data;
  }
}

ایستا صحیح passed_by_ref(ffi_type *type, صحیح var) {
  اگر (type->type == FFI_TYPE_STRUCT)
	  بازگشت 1;
  
  بازگشت type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
بیرونی تهی ffi_call_asm (تهی *پشته, ساختار call_context *regs,
			  تهی (*fn) (تهی), تهی *closure) FFI_HIDDEN;

ایستا تهی
ffi_call_int (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue, تهی **avalue,
	      تهی *closure)
{
  صحیح return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * اندازه(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + اندازه(call_context);
  size_t alloc_base = (size_t)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_پشته = (تهی*)alloc_base;
  cb.struct_پشته = (تهی *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  اگر (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  صحیح i;
  برای (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((تهی *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  اگر (!return_by_ref && rvalue)
    {
	    اگر (IS_INT(cif->rtype->type)
	        && cif->rtype->size < اندازه (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    انتخاب (cif->rtype->type) {
	      حالت FFI_TYPE_SINT8:
	      حالت FFI_TYPE_SINT16:
	      حالت FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (اندازه (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      بشکن;
	      حالت FFI_TYPE_UINT8:
	      حالت FFI_TYPE_UINT16:
	      حالت FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (اندازه (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      بشکن;
	      }
	    }
	    وگرنه
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

تهی
ffi_call (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue, تهی **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, پوچ);
}

تهی
ffi_call_go (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue,
	     تهی **avalue, تهی *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

بیرونی تهی ffi_closure_asm(تهی) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
		      تهی *user_data, تهی *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#اگر defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  انتخاب (cif->abi)
    {
#اگر defined(__ARC64_ARCH64__)
    حالت FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      بشکن;
#وگرنه
    حالت FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      بشکن;
#endif

    پیشفرض:
      بازگشت FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  بازگشت FFI_OK;
}

بیرونی تهی ffi_go_closure_asm (تهی) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *))
{
  اگر (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    بازگشت FFI_BAD_ABI;

  closure->tramp = (تهی *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  بازگشت FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
تهی FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
		   تهی *user_data,
		   size_t *پشته, call_context *aregs)
{
    تهی **avalue = alloca(cif->nargs * اندازه(تهی*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    نویسه *astorage = alloca(cif->bytes);
    نویسه *ptr = astorage;
    تهی *rvalue;
    call_builder cb;
    صحیح i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_پشته = پشته;
    
    /* handle hidden argument */
    اگر (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    وگرنه
      rvalue = alloca(cif->rtype->size);

    برای (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    اگر (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
