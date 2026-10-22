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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <stdint.h>

#ضمن <sys/cachectl.h>

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * حجم(مزدوج))

تعريفنوع بنية call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  حرف frame[16];
} call_context;

تعريفنوع بنية call_builder
{
  call_context *aregs;
  صحيح used_integer;
  //int used_float;
  size_t *used_stack;
  فراغ *struct_stack;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ܐܢ defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ܐܚܪܢܐ
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#ܐܢ __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#ܐܚܪܢܐ
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
      اكسر;

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      اكسر;

    حالة FFI_TYPE_FLOAT:
    افتراضي:
      cif->flags = FFI_TYPE_INT;
      اكسر;
    }

  ܦܢܐ FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ساكن فراغ marshal_atom(call_builder *cb, صحيح type, فراغ *data) {
  size_t value = 0;
  بدل (type) {
    حالة FFI_TYPE_UINT8: value = *(uint8_t *)data; اكسر;
    حالة FFI_TYPE_SINT8: value = *(int8_t *)data; اكسر;
    حالة FFI_TYPE_UINT16: value = *(uint16_t *)data; اكسر;
    حالة FFI_TYPE_SINT16: value = *(int16_t *)data; اكسر;
    /* 32-bit quantities are always sign-extended in the ABI */
    حالة FFI_TYPE_UINT32: value = *(int32_t *)data; اكسر;
    حالة FFI_TYPE_SINT32: value = *(int32_t *)data; اكسر;
#ܐܢ defined(__ARC64_ARCH64__)
    حالة FFI_TYPE_UINT64: value = *(uint64_t *)data; اكسر;
    حالة FFI_TYPE_SINT64: value = *(int64_t *)data; اكسر;
#endif
    حالة FFI_TYPE_POINTER: value = *(size_t *)data; اكسر;
    افتراضي: FFI_ASSERT(0); اكسر;
  }

  ܐܢ (cb->used_integer == NARGREG) {
    *cb->used_stack++ = value;
  } ܐܚܪܢܐ {
    cb->aregs->r[cb->used_integer++] = value;
  }
}

/* adds an argument to a call, or a not by reference return value */
ساكن فراغ marshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *data) {
  size_t realign[2];

#ܐܢ (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  ܐܢ (type->size > 2 * __SIZEOF_POINTER__) {
    ܐܢ (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } ܐܚܪܢܐ {
      /* copy to stack and pass by reference */
      data = memcpy (cb->struct_stack, data, type->size);
      cb->struct_stack = (size_t *) FFI_ALIGN ((حرف *) cb->struct_stack + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#ܐܚܪܢܐ
  ܐܢ (type->type == FFI_TYPE_STRUCT) {
    ܐܢ (var) {
      ܐܢ (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } ܐܚܪܢܐ {
      صحيح i;
      
      لكل (i = 0; i < type->size; i += حجم(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += حجم(size_t);
      }
    }
  }
#endif
  ܐܚܪܢܐ ܐܢ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } ܐܚܪܢܐ {
      memcpy(realign, data, type->size);
      ܐܢ (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      ܐܢ (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

ساكن فراغ unmarshal_atom(call_builder *cb, صحيح type, فراغ *data) {
  size_t value;

  ܐܢ (cb->used_integer == NARGREG) {
    value = *cb->used_stack++;
  } ܐܚܪܢܐ {
    value = cb->aregs->r[cb->used_integer++];
  }

  بدل (type) {
    حالة FFI_TYPE_UINT8: *(uint8_t *)data = value; اكسر;
    حالة FFI_TYPE_SINT8: *(uint8_t *)data = value; اكسر;
    حالة FFI_TYPE_UINT16: *(uint16_t *)data = value; اكسر;
    حالة FFI_TYPE_SINT16: *(uint16_t *)data = value; اكسر;
    حالة FFI_TYPE_UINT32: *(uint32_t *)data = value; اكسر;
    حالة FFI_TYPE_SINT32: *(uint32_t *)data = value; اكسر;
#ܐܢ defined(__ARC64_ARCH64__)
    حالة FFI_TYPE_UINT64: *(uint64_t *)data = value; اكسر;
    حالة FFI_TYPE_SINT64: *(uint64_t *)data = value; اكسر;
#endif
    حالة FFI_TYPE_POINTER: *(size_t *)data = value; اكسر;
    افتراضي: FFI_ASSERT(0); اكسر;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساكن فراغ *unmarshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *data) {
  size_t realign[2];

#ܐܢ defined(__ARC64_ARCH64__)
  فراغ *pointer;

  ܐܢ (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (حرف*)&pointer);
        ܦܢܐ pointer;
    }
#وإلاإذا defined(__ARC64_ARCH32__)
  ܐܢ (type->type == FFI_TYPE_STRUCT) {
    ܐܢ (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(data, (ثابت فراغ*)realign[0], type->size);
      ܦܢܐ data;
    } ܐܚܪܢܐ {
      صحيح i;
      فراغ *pdata = data;

      لكل (i = 0; i < type->size; i += حجم(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += حجم(size_t);
      }
      ܦܢܐ data;
    }
  }
#ܐܚܪܢܐ
  ܐܢ (type->type == FFI_TYPE_STRUCT) {

      ܐܢ (var) {
        صحيح i;
        فراغ *pdata = data;

        لكل (i = 0; i < type->size; i += حجم(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += حجم(size_t);
        }
        ܦܢܐ data;
      } ܐܚܪܢܐ {
        ܐܢ (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(data, (ثابت فراغ*)realign[0], type->size);
        ܦܢܐ data;
      }
  }
#endif
  ܐܚܪܢܐ ܐܢ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    ܦܢܐ data;
  } ܐܚܪܢܐ {
      ܐܢ (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      ܐܢ (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(data, realign, type->size);
      ܦܢܐ data;
  }
}

ساكن صحيح passed_by_ref(ffi_type *type, صحيح var) {
  ܐܢ (type->type == FFI_TYPE_STRUCT)
	  ܦܢܐ 1;
  
  ܦܢܐ type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
خارجي فراغ ffi_call_asm (فراغ *stack, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *closure) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *closure)
{
  صحيح return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * حجم(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + حجم(call_context);
  size_t alloc_base = (size_t)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_stack = (فراغ*)alloc_base;
  cb.struct_stack = (فراغ *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  ܐܢ (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  صحيح i;
  لكل (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((فراغ *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  ܐܢ (!return_by_ref && rvalue)
    {
	    ܐܢ (IS_INT(cif->rtype->type)
	        && cif->rtype->size < حجم (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    بدل (cif->rtype->type) {
	      حالة FFI_TYPE_SINT8:
	      حالة FFI_TYPE_SINT16:
	      حالة FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (حجم (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      اكسر;
	      حالة FFI_TYPE_UINT8:
	      حالة FFI_TYPE_UINT16:
	      حالة FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (حجم (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      اكسر;
	      }
	    }
	    ܐܚܪܢܐ
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

خارجي فراغ ffi_closure_asm(فراغ) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_data, فراغ *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#ܐܢ defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  بدل (cif->abi)
    {
#ܐܢ defined(__ARC64_ARCH64__)
    حالة FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      اكسر;
#ܐܚܪܢܐ
    حالة FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      اكسر;
#endif

    افتراضي:
      ܦܢܐ FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  ܦܢܐ FFI_OK;
}

خارجي فراغ ffi_go_closure_asm (فراغ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  ܐܢ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ܦܢܐ FFI_BAD_ABI;

  closure->tramp = (فراغ *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ܦܢܐ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
فراغ FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_data,
		   size_t *stack, call_context *aregs)
{
    فراغ **avalue = alloca(cif->nargs * حجم(فراغ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    حرف *astorage = alloca(cif->bytes);
    حرف *ptr = astorage;
    فراغ *rvalue;
    call_builder cb;
    صحيح i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_stack = stack;
    
    /* handle hidden argument */
    ܐܢ (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    ܐܚܪܢܐ
      rvalue = alloca(cif->rtype->size);

    لكل (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    ܐܢ (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
