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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdint.h>

#অন্তর্ভাব <sys/cachectl.h>

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * আকার(দ্বিগুণ))

প্রকারলেখ কাঠামো call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  অক্ষর frame[16];
} call_context;

প্রকারলেখ কাঠামো call_builder
{
  call_context *aregs;
  পূর্ণসংখ্যা used_integer;
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
  নির্বাচন (cif->rtype->type)
    {
    ক্ষেত্র FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      বিরতি;

    ক্ষেত্র FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
      বিরতি;

    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      বিরতি;

    ক্ষেত্র FFI_TYPE_FLOAT:
    পূর্বনির্ধারিত:
      cif->flags = FFI_TYPE_INT;
      বিরতি;
    }

  ফেরত FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
স্থির শূন্য marshal_atom(call_builder *cb, পূর্ণসংখ্যা type, শূন্য *তথ্য) {
  size_t মান = 0;
  নির্বাচন (type) {
    ক্ষেত্র FFI_TYPE_UINT8: মান = *(uint8_t *)তথ্য; বিরতি;
    ক্ষেত্র FFI_TYPE_SINT8: মান = *(int8_t *)তথ্য; বিরতি;
    ক্ষেত্র FFI_TYPE_UINT16: মান = *(uint16_t *)তথ্য; বিরতি;
    ক্ষেত্র FFI_TYPE_SINT16: মান = *(int16_t *)তথ্য; বিরতি;
    /* 32-bit quantities are always sign-extended in the ABI */
    ক্ষেত্র FFI_TYPE_UINT32: মান = *(int32_t *)তথ্য; বিরতি;
    ক্ষেত্র FFI_TYPE_SINT32: মান = *(int32_t *)তথ্য; বিরতি;
#যদি defined(__ARC64_ARCH64__)
    ক্ষেত্র FFI_TYPE_UINT64: মান = *(uint64_t *)তথ্য; বিরতি;
    ক্ষেত্র FFI_TYPE_SINT64: মান = *(int64_t *)তথ্য; বিরতি;
#endif
    ক্ষেত্র FFI_TYPE_POINTER: মান = *(size_t *)তথ্য; বিরতি;
    পূর্বনির্ধারিত: FFI_ASSERT(0); বিরতি;
  }

  যদি (cb->used_integer == NARGREG) {
    *cb->used_স্তূপ++ = মান;
  } অন্যথা {
    cb->aregs->r[cb->used_integer++] = মান;
  }
}

/* adds an argument to a call, or a not by reference return value */
স্থির শূন্য marshal(call_builder *cb, ffi_type *type, পূর্ণসংখ্যা var, শূন্য *তথ্য) {
  size_t realign[2];

#যদি (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  যদি (type->আকার_2 > 2 * __SIZEOF_POINTER__) {
    যদি (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &তথ্য);
    } অন্যথা {
      /* copy to stack and pass by reference */
      তথ্য = memcpy (cb->struct_স্তূপ, তথ্য, type->আকার_2);
      cb->struct_স্তূপ = (size_t *) FFI_ALIGN ((অক্ষর *) cb->struct_স্তূপ + type->আকার_2, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &তথ্য);
    }
  }
#অন্যথা
  যদি (type->type == FFI_TYPE_STRUCT) {
    যদি (var) {
      যদি (type->আকার_2 > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, তথ্য);
    } অন্যথা {
      পূর্ণসংখ্যা i;
      
      জন্য (i = 0; i < type->আকার_2; i += আকার(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, তথ্য);
        তথ্য += আকার(size_t);
      }
    }
  }
#endif
  অন্যথা যদি (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, তথ্য);
  } অন্যথা {
      memcpy(realign, তথ্য, type->আকার_2);
      যদি (type->আকার_2 > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      যদি (type->আকার_2 > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

স্থির শূন্য unmarshal_atom(call_builder *cb, পূর্ণসংখ্যা type, শূন্য *তথ্য) {
  size_t মান;

  যদি (cb->used_integer == NARGREG) {
    মান = *cb->used_স্তূপ++;
  } অন্যথা {
    মান = cb->aregs->r[cb->used_integer++];
  }

  নির্বাচন (type) {
    ক্ষেত্র FFI_TYPE_UINT8: *(uint8_t *)তথ্য = মান; বিরতি;
    ক্ষেত্র FFI_TYPE_SINT8: *(uint8_t *)তথ্য = মান; বিরতি;
    ক্ষেত্র FFI_TYPE_UINT16: *(uint16_t *)তথ্য = মান; বিরতি;
    ক্ষেত্র FFI_TYPE_SINT16: *(uint16_t *)তথ্য = মান; বিরতি;
    ক্ষেত্র FFI_TYPE_UINT32: *(uint32_t *)তথ্য = মান; বিরতি;
    ক্ষেত্র FFI_TYPE_SINT32: *(uint32_t *)তথ্য = মান; বিরতি;
#যদি defined(__ARC64_ARCH64__)
    ক্ষেত্র FFI_TYPE_UINT64: *(uint64_t *)তথ্য = মান; বিরতি;
    ক্ষেত্র FFI_TYPE_SINT64: *(uint64_t *)তথ্য = মান; বিরতি;
#endif
    ক্ষেত্র FFI_TYPE_POINTER: *(size_t *)তথ্য = মান; বিরতি;
    পূর্বনির্ধারিত: FFI_ASSERT(0); বিরতি;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
স্থির শূন্য *unmarshal(call_builder *cb, ffi_type *type, পূর্ণসংখ্যা var, শূন্য *তথ্য) {
  size_t realign[2];

#যদি defined(__ARC64_ARCH64__)
  শূন্য *pointer;

  যদি (type->আকার_2 > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (অক্ষর*)&pointer);
        ফেরত pointer;
    }
#নতুবা_যদি defined(__ARC64_ARCH32__)
  যদি (type->type == FFI_TYPE_STRUCT) {
    যদি (type->আকার_2 > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(তথ্য, (ধ্রুবক শূন্য*)realign[0], type->আকার_2);
      ফেরত তথ্য;
    } অন্যথা {
      পূর্ণসংখ্যা i;
      শূন্য *pdata = তথ্য;

      জন্য (i = 0; i < type->আকার_2; i += আকার(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += আকার(size_t);
      }
      ফেরত তথ্য;
    }
  }
#অন্যথা
  যদি (type->type == FFI_TYPE_STRUCT) {

      যদি (var) {
        পূর্ণসংখ্যা i;
        শূন্য *pdata = তথ্য;

        জন্য (i = 0; i < type->আকার_2; i += আকার(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += আকার(size_t);
        }
        ফেরত তথ্য;
      } অন্যথা {
        যদি (type->আকার_2 > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(তথ্য, (ধ্রুবক শূন্য*)realign[0], type->আকার_2);
        ফেরত তথ্য;
      }
  }
#endif
  অন্যথা যদি (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, তথ্য);
    ফেরত তথ্য;
  } অন্যথা {
      যদি (type->আকার_2 > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      যদি (type->আকার_2 > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(তথ্য, realign, type->আকার_2);
      ফেরত তথ্য;
  }
}

স্থির পূর্ণসংখ্যা passed_by_ref(ffi_type *type, পূর্ণসংখ্যা var) {
  যদি (type->type == FFI_TYPE_STRUCT)
	  ফেরত 1;
  
  ফেরত type->আকার_2 > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
বাহ্যিক শূন্য ffi_call_asm (শূন্য *স্তূপ, কাঠামো call_context *regs,
			  শূন্য (*fn) (শূন্য), শূন্য *closure) FFI_HIDDEN;

স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue,
	      শূন্য *closure)
{
  পূর্ণসংখ্যা return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * আকার(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_আকার = arg_bytes + /*rval_bytes +*/ struct_bytes + আকার(call_context);
  size_t alloc_base = (size_t)alloca(alloc_আকার);

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

  পূর্ণসংখ্যা i;
  জন্য (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((শূন্য *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  যদি (!return_by_ref && rvalue)
    {
	    যদি (IS_INT(cif->rtype->type)
	        && cif->rtype->আকার_2 < আকার (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    নির্বাচন (cif->rtype->type) {
	      ক্ষেত্র FFI_TYPE_SINT8:
	      ক্ষেত্র FFI_TYPE_SINT16:
	      ক্ষেত্র FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (আকার (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      বিরতি;
	      ক্ষেত্র FFI_TYPE_UINT8:
	      ক্ষেত্র FFI_TYPE_UINT16:
	      ক্ষেত্র FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (আকার (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      বিরতি;
	      }
	    }
	    অন্যথা
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ফাঁকা);
}

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

বাহ্যিক শূন্য ffi_closure_asm(শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		      শূন্য *user_তথ্য, শূন্য *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#যদি defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  নির্বাচন (cif->abi)
    {
#যদি defined(__ARC64_ARCH64__)
    ক্ষেত্র FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      বিরতি;
#অন্যথা
    ক্ষেত্র FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      বিরতি;
#endif

    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;
  cacheflush (codeloc, FFI_TRAMPOLINE_আকার, BCACHE);

  ফেরত FFI_OK;
}

বাহ্যিক শূন্য ffi_go_closure_asm (শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *))
{
  যদি (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ফেরত FFI_BAD_ABI;

  closure->tramp = (শূন্য *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ফেরত FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
শূন্য FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		   শূন্য *user_তথ্য,
		   size_t *স্তূপ, call_context *aregs)
{
    শূন্য **avalue = alloca(cif->nargs * আকার(শূন্য*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    অক্ষর *astorage = alloca(cif->bytes);
    অক্ষর *ptr = astorage;
    শূন্য *rvalue;
    call_builder cb;
    পূর্ণসংখ্যা i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_স্তূপ = স্তূপ;
    
    /* handle hidden argument */
    যদি (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    অন্যথা
      rvalue = alloca(cif->rtype->আকার_2);

    জন্য (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->আকার_2;
    }

    fun (cif, rvalue, avalue, user_তথ্য);

    যদি (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
