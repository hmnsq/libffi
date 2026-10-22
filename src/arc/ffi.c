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

#تعریف_کړه NARGREG 8
#تعریف_کړه STKALIGN 4
#تعریف_کړه MAXCOPYARG (2 * حجم(مزدوج))

تعريفنوع بنية call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  حرف چوکاټ[16];
} call_context;

تعريفنوع بنية call_builder
{
  call_context *aregs;
  صحيح کارول_شوی_integer;
  //int used_float;
  size_t *کارول_شوی_ډېری;
  فراغ *struct_ډېری;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#إذا defined(__ARC64_ARCH64__)
#تعریف_کړه IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
#تعریف_کړه IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#د_که_پای

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#إذا __BIG_ENDIAN__
#تعریف_کړه کوډ_ENDIAN(x) (x)
#وإلا
#تعریف_کړه کوډ_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#د_که_پای

/* Perform machine dependent cif processing.  */
ffi_حالت
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      cif->flags = (بینښه) cif->rtype->type;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      cif->flags = (بینښه) cif->rtype->type;
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

  أرجع FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ساكن فراغ marshal_atom(call_builder *cb, صحيح type, فراغ *معلومات) {
  size_t ارزښت = 0;
  بدل (type) {
    حالة FFI_TYPE_UINT8: ارزښت = *(uint8_t *)معلومات; اكسر;
    حالة FFI_TYPE_SINT8: ارزښت = *(int8_t *)معلومات; اكسر;
    حالة FFI_TYPE_UINT16: ارزښت = *(uint16_t *)معلومات; اكسر;
    حالة FFI_TYPE_SINT16: ارزښت = *(int16_t *)معلومات; اكسر;
    /* 32-bit quantities are always sign-extended in the ABI */
    حالة FFI_TYPE_UINT32: ارزښت = *(int32_t *)معلومات; اكسر;
    حالة FFI_TYPE_SINT32: ارزښت = *(int32_t *)معلومات; اكسر;
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_TYPE_UINT64: ارزښت = *(uint64_t *)معلومات; اكسر;
    حالة FFI_TYPE_SINT64: ارزښت = *(int64_t *)معلومات; اكسر;
#د_که_پای
    حالة FFI_TYPE_اشاره_ګر: ارزښت = *(size_t *)معلومات; اكسر;
    افتراضي: FFI_ASSERT(0); اكسر;
  }

  إذا (cb->کارول_شوی_integer == NARGREG) {
    *cb->کارول_شوی_ډېری++ = ارزښت;
  } وإلا {
    cb->aregs->r[cb->کارول_شوی_integer++] = ارزښت;
  }
}

/* adds an argument to a call, or a not by reference return value */
ساكن فراغ marshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *معلومات) {
  size_t realign[2];

#إذا (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  إذا (type->اندازه > 2 * __SIZEOF_POINTER__) {
    إذا (var) {
      marshal_atom(cb, FFI_TYPE_اشاره_ګر, &معلومات);
    } وإلا {
      /* copy to stack and pass by reference */
      معلومات = حافظه_نقل (cb->struct_ډېری, معلومات, type->اندازه);
      cb->struct_ډېری = (size_t *) FFI_ALIGN ((حرف *) cb->struct_ډېری + type->اندازه, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_اشاره_ګر, &معلومات);
    }
  }
#وإلا
  إذا (type->type == FFI_TYPE_STRUCT) {
    إذا (var) {
      إذا (type->اندازه > 0)
        marshal_atom(cb, FFI_TYPE_اشاره_ګر, معلومات);
    } وإلا {
      صحيح i;
      
      لكل (i = 0; i < type->اندازه; i += حجم(size_t)) {
        marshal_atom(cb, FFI_TYPE_اشاره_ګر, معلومات);
        معلومات += حجم(size_t);
      }
    }
  }
#د_که_پای
  وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشاره_ګر) {
    marshal_atom(cb, type->type, معلومات);
  } وإلا {
      حافظه_نقل(realign, معلومات, type->اندازه);
      إذا (type->اندازه > 0)
        marshal_atom(cb, FFI_TYPE_اشاره_ګر, realign);
      إذا (type->اندازه > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_اشاره_ګر, realign + 1);
  }
}

ساكن فراغ unmarshal_atom(call_builder *cb, صحيح type, فراغ *معلومات) {
  size_t ارزښت;

  إذا (cb->کارول_شوی_integer == NARGREG) {
    ارزښت = *cb->کارول_شوی_ډېری++;
  } وإلا {
    ارزښت = cb->aregs->r[cb->کارول_شوی_integer++];
  }

  بدل (type) {
    حالة FFI_TYPE_UINT8: *(uint8_t *)معلومات = ارزښت; اكسر;
    حالة FFI_TYPE_SINT8: *(uint8_t *)معلومات = ارزښت; اكسر;
    حالة FFI_TYPE_UINT16: *(uint16_t *)معلومات = ارزښت; اكسر;
    حالة FFI_TYPE_SINT16: *(uint16_t *)معلومات = ارزښت; اكسر;
    حالة FFI_TYPE_UINT32: *(uint32_t *)معلومات = ارزښت; اكسر;
    حالة FFI_TYPE_SINT32: *(uint32_t *)معلومات = ارزښت; اكسر;
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_TYPE_UINT64: *(uint64_t *)معلومات = ارزښت; اكسر;
    حالة FFI_TYPE_SINT64: *(uint64_t *)معلومات = ارزښت; اكسر;
#د_که_پای
    حالة FFI_TYPE_اشاره_ګر: *(size_t *)معلومات = ارزښت; اكسر;
    افتراضي: FFI_ASSERT(0); اكسر;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساكن فراغ *unmarshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *معلومات) {
  size_t realign[2];

#إذا defined(__ARC64_ARCH64__)
  فراغ *اشاره_ګر;

  إذا (type->اندازه > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_اشاره_ګر, (حرف*)&اشاره_ګر);
        أرجع اشاره_ګر;
    }
#وإلاإذا defined(__ARC64_ARCH32__)
  إذا (type->type == FFI_TYPE_STRUCT) {
    إذا (type->اندازه > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_اشاره_ګر, &realign[0]);
      حافظه_نقل(معلومات, (ثابت فراغ*)realign[0], type->اندازه);
      أرجع معلومات;
    } وإلا {
      صحيح i;
      فراغ *pdata = معلومات;

      لكل (i = 0; i < type->اندازه; i += حجم(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_اشاره_ګر, pdata);
        pdata += حجم(size_t);
      }
      أرجع معلومات;
    }
  }
#وإلا
  إذا (type->type == FFI_TYPE_STRUCT) {

      إذا (var) {
        صحيح i;
        فراغ *pdata = معلومات;

        لكل (i = 0; i < type->اندازه; i += حجم(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_اشاره_ګر, pdata);
          pdata += حجم(size_t);
        }
        أرجع معلومات;
      } وإلا {
        إذا (type->اندازه > 0)
          unmarshal_atom(cb, FFI_TYPE_اشاره_ګر, &realign[0]);
        حافظه_نقل(معلومات, (ثابت فراغ*)realign[0], type->اندازه);
        أرجع معلومات;
      }
  }
#د_که_پای
  وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشاره_ګر) {
    unmarshal_atom(cb, type->type, معلومات);
    أرجع معلومات;
  } وإلا {
      إذا (type->اندازه > 0)
        unmarshal_atom(cb, FFI_TYPE_اشاره_ګر, realign);
      إذا (type->اندازه > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_اشاره_ګر, realign + 1);
      حافظه_نقل(معلومات, realign, type->اندازه);
      أرجع معلومات;
  }
}

ساكن صحيح passed_by_ref(ffi_type *type, صحيح var) {
  إذا (type->type == FFI_TYPE_STRUCT)
	  أرجع 1;
  
  أرجع type->اندازه > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
خارجي فراغ ffi_call_asm (فراغ *ډېری, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *تړون) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *تړون)
{
  صحيح return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_بایټونه = FFI_ALIGN(2 * حجم(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_بایټونه = FFI_ALIGN(cif->بایټونه, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_اندازه = arg_بایټونه + /*rval_bytes +*/ struct_بایټونه + حجم(call_context);
  size_t alloc_بنسټ = (size_t)alloca(alloc_اندازه);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.کارول_شوی_integer = 0;
  cb.aregs = (call_context*)(alloc_بنسټ + arg_بایټونه /*+ rval_bytes*/ + struct_بایټونه);
  cb.کارول_شوی_ډېری = (فراغ*)alloc_بنسټ;
  cb.struct_ډېری = (فراغ *)(alloc_بنسټ + arg_بایټونه /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  إذا (return_by_ref)
	  marshal(&cb, &ffi_type_اشاره_ګر, 0, &rvalue);

  صحيح i;
  لكل (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((فراغ *) alloc_بنسټ, cb.aregs, fn, تړون);

  cb.کارول_شوی_integer = 0;
  إذا (!return_by_ref && rvalue)
    {
	    إذا (IS_INT(cif->rtype->type)
	        && cif->rtype->اندازه < حجم (ffi_arg))
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
	    وإلا
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
	     فراغ **avalue, فراغ *تړون)
{
  ffi_call_int(cif, fn, rvalue, avalue, تړون);
}

خارجي فراغ ffi_تړون_asm(فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون * تړون, ffi_cif * cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_معلومات, فراغ *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (تړون->tramp[0]);

#إذا defined(__ARC64_ARCH64__)
  size_t پته_ffi_تړون = (size_t) ffi_تړون_asm;
#د_که_پای

  بدل (cif->abi)
    {
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = کوډ_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = کوډ_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = کوډ_ENDIAN ((uint32_t)(پته_ffi_تړون >> 32));
      tramp[3] = کوډ_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = کوډ_ENDIAN ((uint32_t)(پته_ffi_تړون & 0xffffffff));
      tramp[5] = کوډ_ENDIAN (0x20200300);	/* j [r12] */
      اكسر;
#وإلا
    حالة FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = کوډ_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = کوډ_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = کوډ_ENDIAN ((uint32_t) ffi_تړون_asm);
      اكسر;
#د_که_پای

    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  تړون->cif = cif;
  تړون->fun = fun;
  تړون->user_معلومات = user_معلومات;
  cacheflush (codeloc, FFI_TRAMPOLINE_اندازه, BCACHE);

  أرجع FFI_OK;
}

خارجي فراغ ffi_go_تړون_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_تړون (ffi_go_تړون *تړون, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_لومړی_ABI || cif->abi >= FFI_وروستی_ABI)
    أرجع FFI_BAD_ABI;

  تړون->tramp = (فراغ *) ffi_go_تړون_asm;
  تړون->cif = cif;
  تړون->fun = fun;

  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
فراغ FFI_HIDDEN
ffi_تړون_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_معلومات,
		   size_t *ډېری, call_context *aregs)
{
    فراغ **avalue = alloca(cif->nargs * حجم(فراغ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    حرف *astorage = alloca(cif->بایټونه);
    حرف *ptr = astorage;
    فراغ *rvalue;
    call_builder cb;
    صحيح i;

    cb.aregs = aregs;
    cb.کارول_شوی_integer = 0;
    cb.کارول_شوی_ډېری = ډېری;
    
    /* handle hidden argument */
    إذا (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_اشاره_ګر, 0, &rvalue);
    وإلا
      rvalue = alloca(cif->rtype->اندازه);

    لكل (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->اندازه;
    }

    fun (cif, rvalue, avalue, user_معلومات);

    إذا (cif->rtype->type != FFI_TYPE_VOID) {
        cb.کارول_شوی_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
