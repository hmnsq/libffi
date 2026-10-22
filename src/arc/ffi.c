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

#تعریف_کن NARGREG 8
#تعریف_کن STKALIGN 4
#تعریف_کن MAXCOPYARG (2 * حجم(مزدوج))

تعريفنوع بنية call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  حرف قاب[16];
} call_context;

تعريفنوع بنية call_builder
{
  call_context *aregs;
  صحيح استفاده_شده_integer;
  //int used_float;
  size_t *استفاده_شده_پشته;
  فراغ *struct_پشته;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#إذا defined(__ARC64_ARCH64__)
#تعریف_کن IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
#تعریف_کن IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#پایان_اگر

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#إذا __BIG_ENDIAN__
#تعریف_کن کد_ENDIAN(x) (x)
#وإلا
#تعریف_کن کد_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#پایان_اگر

/* Perform machine dependent cif processing.  */
ffi_وضعیت
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      cif->flags = (بیعلامت) cif->rtype->type;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      cif->flags = (بیعلامت) cif->rtype->type;
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
ساكن فراغ marshal_atom(call_builder *cb, صحيح type, فراغ *داده) {
  size_t مقدار = 0;
  بدل (type) {
    حالة FFI_TYPE_UINT8: مقدار = *(uint8_t *)داده; اكسر;
    حالة FFI_TYPE_SINT8: مقدار = *(int8_t *)داده; اكسر;
    حالة FFI_TYPE_UINT16: مقدار = *(uint16_t *)داده; اكسر;
    حالة FFI_TYPE_SINT16: مقدار = *(int16_t *)داده; اكسر;
    /* 32-bit quantities are always sign-extended in the ABI */
    حالة FFI_TYPE_UINT32: مقدار = *(int32_t *)داده; اكسر;
    حالة FFI_TYPE_SINT32: مقدار = *(int32_t *)داده; اكسر;
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_TYPE_UINT64: مقدار = *(uint64_t *)داده; اكسر;
    حالة FFI_TYPE_SINT64: مقدار = *(int64_t *)داده; اكسر;
#پایان_اگر
    حالة FFI_TYPE_اشاره_گر: مقدار = *(size_t *)داده; اكسر;
    افتراضي: FFI_ASSERT(0); اكسر;
  }

  إذا (cb->استفاده_شده_integer == NARGREG) {
    *cb->استفاده_شده_پشته++ = مقدار;
  } وإلا {
    cb->aregs->r[cb->استفاده_شده_integer++] = مقدار;
  }
}

/* adds an argument to a call, or a not by reference return value */
ساكن فراغ marshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *داده) {
  size_t realign[2];

#إذا (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  إذا (type->اندازه > 2 * __SIZEOF_POINTER__) {
    إذا (var) {
      marshal_atom(cb, FFI_TYPE_اشاره_گر, &داده);
    } وإلا {
      /* copy to stack and pass by reference */
      داده = کپی_حافظه (cb->struct_پشته, داده, type->اندازه);
      cb->struct_پشته = (size_t *) FFI_ALIGN ((حرف *) cb->struct_پشته + type->اندازه, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_اشاره_گر, &داده);
    }
  }
#وإلا
  إذا (type->type == FFI_TYPE_STRUCT) {
    إذا (var) {
      إذا (type->اندازه > 0)
        marshal_atom(cb, FFI_TYPE_اشاره_گر, داده);
    } وإلا {
      صحيح i;
      
      لكل (i = 0; i < type->اندازه; i += حجم(size_t)) {
        marshal_atom(cb, FFI_TYPE_اشاره_گر, داده);
        داده += حجم(size_t);
      }
    }
  }
#پایان_اگر
  وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشاره_گر) {
    marshal_atom(cb, type->type, داده);
  } وإلا {
      کپی_حافظه(realign, داده, type->اندازه);
      إذا (type->اندازه > 0)
        marshal_atom(cb, FFI_TYPE_اشاره_گر, realign);
      إذا (type->اندازه > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_اشاره_گر, realign + 1);
  }
}

ساكن فراغ unmarshal_atom(call_builder *cb, صحيح type, فراغ *داده) {
  size_t مقدار;

  إذا (cb->استفاده_شده_integer == NARGREG) {
    مقدار = *cb->استفاده_شده_پشته++;
  } وإلا {
    مقدار = cb->aregs->r[cb->استفاده_شده_integer++];
  }

  بدل (type) {
    حالة FFI_TYPE_UINT8: *(uint8_t *)داده = مقدار; اكسر;
    حالة FFI_TYPE_SINT8: *(uint8_t *)داده = مقدار; اكسر;
    حالة FFI_TYPE_UINT16: *(uint16_t *)داده = مقدار; اكسر;
    حالة FFI_TYPE_SINT16: *(uint16_t *)داده = مقدار; اكسر;
    حالة FFI_TYPE_UINT32: *(uint32_t *)داده = مقدار; اكسر;
    حالة FFI_TYPE_SINT32: *(uint32_t *)داده = مقدار; اكسر;
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_TYPE_UINT64: *(uint64_t *)داده = مقدار; اكسر;
    حالة FFI_TYPE_SINT64: *(uint64_t *)داده = مقدار; اكسر;
#پایان_اگر
    حالة FFI_TYPE_اشاره_گر: *(size_t *)داده = مقدار; اكسر;
    افتراضي: FFI_ASSERT(0); اكسر;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساكن فراغ *unmarshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *داده) {
  size_t realign[2];

#إذا defined(__ARC64_ARCH64__)
  فراغ *اشاره_گر;

  إذا (type->اندازه > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_اشاره_گر, (حرف*)&اشاره_گر);
        أرجع اشاره_گر;
    }
#وإلاإذا defined(__ARC64_ARCH32__)
  إذا (type->type == FFI_TYPE_STRUCT) {
    إذا (type->اندازه > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_اشاره_گر, &realign[0]);
      کپی_حافظه(داده, (ثابت فراغ*)realign[0], type->اندازه);
      أرجع داده;
    } وإلا {
      صحيح i;
      فراغ *pdata = داده;

      لكل (i = 0; i < type->اندازه; i += حجم(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_اشاره_گر, pdata);
        pdata += حجم(size_t);
      }
      أرجع داده;
    }
  }
#وإلا
  إذا (type->type == FFI_TYPE_STRUCT) {

      إذا (var) {
        صحيح i;
        فراغ *pdata = داده;

        لكل (i = 0; i < type->اندازه; i += حجم(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_اشاره_گر, pdata);
          pdata += حجم(size_t);
        }
        أرجع داده;
      } وإلا {
        إذا (type->اندازه > 0)
          unmarshal_atom(cb, FFI_TYPE_اشاره_گر, &realign[0]);
        کپی_حافظه(داده, (ثابت فراغ*)realign[0], type->اندازه);
        أرجع داده;
      }
  }
#پایان_اگر
  وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشاره_گر) {
    unmarshal_atom(cb, type->type, داده);
    أرجع داده;
  } وإلا {
      إذا (type->اندازه > 0)
        unmarshal_atom(cb, FFI_TYPE_اشاره_گر, realign);
      إذا (type->اندازه > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_اشاره_گر, realign + 1);
      کپی_حافظه(داده, realign, type->اندازه);
      أرجع داده;
  }
}

ساكن صحيح passed_by_ref(ffi_type *type, صحيح var) {
  إذا (type->type == FFI_TYPE_STRUCT)
	  أرجع 1;
  
  أرجع type->اندازه > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
خارجي فراغ ffi_call_asm (فراغ *پشته, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *بستگی) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *بستگی)
{
  صحيح return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_بایتها = FFI_ALIGN(2 * حجم(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_بایتها = FFI_ALIGN(cif->بایتها, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_اندازه = arg_بایتها + /*rval_bytes +*/ struct_بایتها + حجم(call_context);
  size_t alloc_پایه = (size_t)alloca(alloc_اندازه);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.استفاده_شده_integer = 0;
  cb.aregs = (call_context*)(alloc_پایه + arg_بایتها /*+ rval_bytes*/ + struct_بایتها);
  cb.استفاده_شده_پشته = (فراغ*)alloc_پایه;
  cb.struct_پشته = (فراغ *)(alloc_پایه + arg_بایتها /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  إذا (return_by_ref)
	  marshal(&cb, &ffi_type_اشاره_گر, 0, &rvalue);

  صحيح i;
  لكل (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((فراغ *) alloc_پایه, cb.aregs, fn, بستگی);

  cb.استفاده_شده_integer = 0;
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
	     فراغ **avalue, فراغ *بستگی)
{
  ffi_call_int(cif, fn, rvalue, avalue, بستگی);
}

خارجي فراغ ffi_بستگی_asm(فراغ) FFI_HIDDEN;

ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی * بستگی, ffi_cif * cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_داده, فراغ *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (بستگی->tramp[0]);

#إذا defined(__ARC64_ARCH64__)
  size_t آدرس_ffi_بستگی = (size_t) ffi_بستگی_asm;
#پایان_اگر

  بدل (cif->abi)
    {
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = کد_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = کد_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = کد_ENDIAN ((uint32_t)(آدرس_ffi_بستگی >> 32));
      tramp[3] = کد_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = کد_ENDIAN ((uint32_t)(آدرس_ffi_بستگی & 0xffffffff));
      tramp[5] = کد_ENDIAN (0x20200300);	/* j [r12] */
      اكسر;
#وإلا
    حالة FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = کد_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = کد_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = کد_ENDIAN ((uint32_t) ffi_بستگی_asm);
      اكسر;
#پایان_اگر

    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  بستگی->cif = cif;
  بستگی->fun = fun;
  بستگی->user_داده = user_داده;
  cacheflush (codeloc, FFI_TRAMPOLINE_اندازه, BCACHE);

  أرجع FFI_OK;
}

خارجي فراغ ffi_go_بستگی_asm (فراغ) FFI_HIDDEN;

ffi_وضعیت
ffi_prep_go_بستگی (ffi_go_بستگی *بستگی, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_اولی_ABI || cif->abi >= FFI_آخرین_ABI)
    أرجع FFI_BAD_ABI;

  بستگی->tramp = (فراغ *) ffi_go_بستگی_asm;
  بستگی->cif = cif;
  بستگی->fun = fun;

  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
فراغ FFI_HIDDEN
ffi_بستگی_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_داده,
		   size_t *پشته, call_context *aregs)
{
    فراغ **avalue = alloca(cif->nargs * حجم(فراغ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    حرف *astorage = alloca(cif->بایتها);
    حرف *ptr = astorage;
    فراغ *rvalue;
    call_builder cb;
    صحيح i;

    cb.aregs = aregs;
    cb.استفاده_شده_integer = 0;
    cb.استفاده_شده_پشته = پشته;
    
    /* handle hidden argument */
    إذا (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_اشاره_گر, 0, &rvalue);
    وإلا
      rvalue = alloca(cif->rtype->اندازه);

    لكل (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->اندازه;
    }

    fun (cif, rvalue, avalue, user_داده);

    إذا (cif->rtype->type != FFI_TYPE_VOID) {
        cb.استفاده_شده_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
