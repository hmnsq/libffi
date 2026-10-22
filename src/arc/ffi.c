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

#تعریف_کرو NARGREG 8
#تعریف_کرو STKALIGN 4
#تعریف_کرو MAXCOPYARG (2 * حجم(مزدوج))

تعريفنوع بنية call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  حرف چوکھٹا[16];
} call_context;

تعريفنوع بنية call_builder
{
  call_context *aregs;
  صحيح ورتیا_integer;
  //int used_float;
  size_t *ورتیا_ڈھیر;
  فراغ *struct_ڈھیر;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#إذا defined(__ARC64_ARCH64__)
#تعریف_کرو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
#تعریف_کرو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#جے_انت

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#إذا __BIG_ENDIAN__
#تعریف_کرو کوڈ_ENDIAN(x) (x)
#وإلا
#تعریف_کرو کوڈ_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#جے_انت

/* Perform machine dependent cif processing.  */
ffi_حالت
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      cif->flags = (بنانشانی) cif->rtype->type;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      cif->flags = (بنانشانی) cif->rtype->type;
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
ساكن فراغ marshal_atom(call_builder *cb, صحيح type, فراغ *انکڑے) {
  size_t قدر = 0;
  بدل (type) {
    حالة FFI_TYPE_UINT8: قدر = *(uint8_t *)انکڑے; اكسر;
    حالة FFI_TYPE_SINT8: قدر = *(int8_t *)انکڑے; اكسر;
    حالة FFI_TYPE_UINT16: قدر = *(uint16_t *)انکڑے; اكسر;
    حالة FFI_TYPE_SINT16: قدر = *(int16_t *)انکڑے; اكسر;
    /* 32-bit quantities are always sign-extended in the ABI */
    حالة FFI_TYPE_UINT32: قدر = *(int32_t *)انکڑے; اكسر;
    حالة FFI_TYPE_SINT32: قدر = *(int32_t *)انکڑے; اكسر;
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_TYPE_UINT64: قدر = *(uint64_t *)انکڑے; اكسر;
    حالة FFI_TYPE_SINT64: قدر = *(int64_t *)انکڑے; اكسر;
#جے_انت
    حالة FFI_TYPE_اشارہ_گر: قدر = *(size_t *)انکڑے; اكسر;
    افتراضي: FFI_ASSERT(0); اكسر;
  }

  إذا (cb->ورتیا_integer == NARGREG) {
    *cb->ورتیا_ڈھیر++ = قدر;
  } وإلا {
    cb->aregs->r[cb->ورتیا_integer++] = قدر;
  }
}

/* adds an argument to a call, or a not by reference return value */
ساكن فراغ marshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *انکڑے) {
  size_t realign[2];

#إذا (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  إذا (type->ناپ > 2 * __SIZEOF_POINTER__) {
    إذا (var) {
      marshal_atom(cb, FFI_TYPE_اشارہ_گر, &انکڑے);
    } وإلا {
      /* copy to stack and pass by reference */
      انکڑے = حافظہ_نقل (cb->struct_ڈھیر, انکڑے, type->ناپ);
      cb->struct_ڈھیر = (size_t *) FFI_ALIGN ((حرف *) cb->struct_ڈھیر + type->ناپ, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_اشارہ_گر, &انکڑے);
    }
  }
#وإلا
  إذا (type->type == FFI_TYPE_STRUCT) {
    إذا (var) {
      إذا (type->ناپ > 0)
        marshal_atom(cb, FFI_TYPE_اشارہ_گر, انکڑے);
    } وإلا {
      صحيح i;
      
      لكل (i = 0; i < type->ناپ; i += حجم(size_t)) {
        marshal_atom(cb, FFI_TYPE_اشارہ_گر, انکڑے);
        انکڑے += حجم(size_t);
      }
    }
  }
#جے_انت
  وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشارہ_گر) {
    marshal_atom(cb, type->type, انکڑے);
  } وإلا {
      حافظہ_نقل(realign, انکڑے, type->ناپ);
      إذا (type->ناپ > 0)
        marshal_atom(cb, FFI_TYPE_اشارہ_گر, realign);
      إذا (type->ناپ > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_اشارہ_گر, realign + 1);
  }
}

ساكن فراغ unmarshal_atom(call_builder *cb, صحيح type, فراغ *انکڑے) {
  size_t قدر;

  إذا (cb->ورتیا_integer == NARGREG) {
    قدر = *cb->ورتیا_ڈھیر++;
  } وإلا {
    قدر = cb->aregs->r[cb->ورتیا_integer++];
  }

  بدل (type) {
    حالة FFI_TYPE_UINT8: *(uint8_t *)انکڑے = قدر; اكسر;
    حالة FFI_TYPE_SINT8: *(uint8_t *)انکڑے = قدر; اكسر;
    حالة FFI_TYPE_UINT16: *(uint16_t *)انکڑے = قدر; اكسر;
    حالة FFI_TYPE_SINT16: *(uint16_t *)انکڑے = قدر; اكسر;
    حالة FFI_TYPE_UINT32: *(uint32_t *)انکڑے = قدر; اكسر;
    حالة FFI_TYPE_SINT32: *(uint32_t *)انکڑے = قدر; اكسر;
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_TYPE_UINT64: *(uint64_t *)انکڑے = قدر; اكسر;
    حالة FFI_TYPE_SINT64: *(uint64_t *)انکڑے = قدر; اكسر;
#جے_انت
    حالة FFI_TYPE_اشارہ_گر: *(size_t *)انکڑے = قدر; اكسر;
    افتراضي: FFI_ASSERT(0); اكسر;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساكن فراغ *unmarshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *انکڑے) {
  size_t realign[2];

#إذا defined(__ARC64_ARCH64__)
  فراغ *اشارہ_گر;

  إذا (type->ناپ > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, (حرف*)&اشارہ_گر);
        أرجع اشارہ_گر;
    }
#وإلاإذا defined(__ARC64_ARCH32__)
  إذا (type->type == FFI_TYPE_STRUCT) {
    إذا (type->ناپ > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, &realign[0]);
      حافظہ_نقل(انکڑے, (ثابت فراغ*)realign[0], type->ناپ);
      أرجع انکڑے;
    } وإلا {
      صحيح i;
      فراغ *pdata = انکڑے;

      لكل (i = 0; i < type->ناپ; i += حجم(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, pdata);
        pdata += حجم(size_t);
      }
      أرجع انکڑے;
    }
  }
#وإلا
  إذا (type->type == FFI_TYPE_STRUCT) {

      إذا (var) {
        صحيح i;
        فراغ *pdata = انکڑے;

        لكل (i = 0; i < type->ناپ; i += حجم(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, pdata);
          pdata += حجم(size_t);
        }
        أرجع انکڑے;
      } وإلا {
        إذا (type->ناپ > 0)
          unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, &realign[0]);
        حافظہ_نقل(انکڑے, (ثابت فراغ*)realign[0], type->ناپ);
        أرجع انکڑے;
      }
  }
#جے_انت
  وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشارہ_گر) {
    unmarshal_atom(cb, type->type, انکڑے);
    أرجع انکڑے;
  } وإلا {
      إذا (type->ناپ > 0)
        unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, realign);
      إذا (type->ناپ > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, realign + 1);
      حافظہ_نقل(انکڑے, realign, type->ناپ);
      أرجع انکڑے;
  }
}

ساكن صحيح passed_by_ref(ffi_type *type, صحيح var) {
  إذا (type->type == FFI_TYPE_STRUCT)
	  أرجع 1;
  
  أرجع type->ناپ > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
خارجي فراغ ffi_call_asm (فراغ *ڈھیر, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *گھیرا) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *گھیرا)
{
  صحيح return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_بائٹاں = FFI_ALIGN(2 * حجم(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_بائٹاں = FFI_ALIGN(cif->بائٹاں, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_ناپ = arg_بائٹاں + /*rval_bytes +*/ struct_بائٹاں + حجم(call_context);
  size_t alloc_ادھار = (size_t)alloca(alloc_ناپ);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.ورتیا_integer = 0;
  cb.aregs = (call_context*)(alloc_ادھار + arg_بائٹاں /*+ rval_bytes*/ + struct_بائٹاں);
  cb.ورتیا_ڈھیر = (فراغ*)alloc_ادھار;
  cb.struct_ڈھیر = (فراغ *)(alloc_ادھار + arg_بائٹاں /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  إذا (return_by_ref)
	  marshal(&cb, &ffi_type_اشارہ_گر, 0, &rvalue);

  صحيح i;
  لكل (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((فراغ *) alloc_ادھار, cb.aregs, fn, گھیرا);

  cb.ورتیا_integer = 0;
  إذا (!return_by_ref && rvalue)
    {
	    إذا (IS_INT(cif->rtype->type)
	        && cif->rtype->ناپ < حجم (ffi_arg))
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
	     فراغ **avalue, فراغ *گھیرا)
{
  ffi_call_int(cif, fn, rvalue, avalue, گھیرا);
}

خارجي فراغ ffi_گھیرا_asm(فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا * گھیرا, ffi_cif * cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_انکڑے, فراغ *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (گھیرا->tramp[0]);

#إذا defined(__ARC64_ARCH64__)
  size_t پتہ_ffi_گھیرا = (size_t) ffi_گھیرا_asm;
#جے_انت

  بدل (cif->abi)
    {
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = کوڈ_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = کوڈ_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = کوڈ_ENDIAN ((uint32_t)(پتہ_ffi_گھیرا >> 32));
      tramp[3] = کوڈ_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = کوڈ_ENDIAN ((uint32_t)(پتہ_ffi_گھیرا & 0xffffffff));
      tramp[5] = کوڈ_ENDIAN (0x20200300);	/* j [r12] */
      اكسر;
#وإلا
    حالة FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = کوڈ_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = کوڈ_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = کوڈ_ENDIAN ((uint32_t) ffi_گھیرا_asm);
      اكسر;
#جے_انت

    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;
  cacheflush (codeloc, FFI_TRAMPOLINE_ناپ, BCACHE);

  أرجع FFI_OK;
}

خارجي فراغ ffi_go_گھیرا_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_گھیرا (ffi_go_گھیرا *گھیرا, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_پہلا_ABI || cif->abi >= FFI_آخری_ABI)
    أرجع FFI_BAD_ABI;

  گھیرا->tramp = (فراغ *) ffi_go_گھیرا_asm;
  گھیرا->cif = cif;
  گھیرا->fun = fun;

  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
فراغ FFI_HIDDEN
ffi_گھیرا_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_انکڑے,
		   size_t *ڈھیر, call_context *aregs)
{
    فراغ **avalue = alloca(cif->nargs * حجم(فراغ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    حرف *astorage = alloca(cif->بائٹاں);
    حرف *ptr = astorage;
    فراغ *rvalue;
    call_builder cb;
    صحيح i;

    cb.aregs = aregs;
    cb.ورتیا_integer = 0;
    cb.ورتیا_ڈھیر = ڈھیر;
    
    /* handle hidden argument */
    إذا (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_اشارہ_گر, 0, &rvalue);
    وإلا
      rvalue = alloca(cif->rtype->ناپ);

    لكل (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->ناپ;
    }

    fun (cif, rvalue, avalue, user_انکڑے);

    إذا (cif->rtype->type != FFI_TYPE_VOID) {
        cb.ورتیا_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
