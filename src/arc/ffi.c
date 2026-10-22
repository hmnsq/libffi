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

#وصف_ڪريو NARGREG 8
#وصف_ڪريو STKALIGN 4
#وصف_ڪريو MAXCOPYARG (2 * حجم(مزدوج))

تعريفنوع بنية call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  حرف چوکٺو[16];
} call_context;

تعريفنوع بنية call_builder
{
  call_context *aregs;
  صحيح استعمال_ٿيل_integer;
  //int used_float;
  size_t *استعمال_ٿيل_ڍير;
  فراغ *struct_ڍير;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#إذا defined(__ARC64_ARCH64__)
#وصف_ڪريو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
#وصف_ڪريو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#جيڪڏهن_پورو

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#إذا __BIG_ENDIAN__
#وصف_ڪريو ڪوڊ_ENDIAN(x) (x)
#وإلا
#وصف_ڪريو ڪوڊ_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#جيڪڏهن_پورو

/* Perform machine dependent cif processing.  */
ffi_حالت
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      cif->flags = (بنانشان) cif->rtype->type;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      cif->flags = (بنانشان) cif->rtype->type;
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
  size_t قدر = 0;
  بدل (type) {
    حالة FFI_TYPE_UINT8: قدر = *(uint8_t *)معلومات; اكسر;
    حالة FFI_TYPE_SINT8: قدر = *(int8_t *)معلومات; اكسر;
    حالة FFI_TYPE_UINT16: قدر = *(uint16_t *)معلومات; اكسر;
    حالة FFI_TYPE_SINT16: قدر = *(int16_t *)معلومات; اكسر;
    /* 32-bit quantities are always sign-extended in the ABI */
    حالة FFI_TYPE_UINT32: قدر = *(int32_t *)معلومات; اكسر;
    حالة FFI_TYPE_SINT32: قدر = *(int32_t *)معلومات; اكسر;
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_TYPE_UINT64: قدر = *(uint64_t *)معلومات; اكسر;
    حالة FFI_TYPE_SINT64: قدر = *(int64_t *)معلومات; اكسر;
#جيڪڏهن_پورو
    حالة FFI_TYPE_اشارو: قدر = *(size_t *)معلومات; اكسر;
    افتراضي: FFI_ASSERT(0); اكسر;
  }

  إذا (cb->استعمال_ٿيل_integer == NARGREG) {
    *cb->استعمال_ٿيل_ڍير++ = قدر;
  } وإلا {
    cb->aregs->r[cb->استعمال_ٿيل_integer++] = قدر;
  }
}

/* adds an argument to a call, or a not by reference return value */
ساكن فراغ marshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *معلومات) {
  size_t realign[2];

#إذا (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  إذا (type->سائيز > 2 * __SIZEOF_POINTER__) {
    إذا (var) {
      marshal_atom(cb, FFI_TYPE_اشارو, &معلومات);
    } وإلا {
      /* copy to stack and pass by reference */
      معلومات = يادگيري_نقل (cb->struct_ڍير, معلومات, type->سائيز);
      cb->struct_ڍير = (size_t *) FFI_ALIGN ((حرف *) cb->struct_ڍير + type->سائيز, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_اشارو, &معلومات);
    }
  }
#وإلا
  إذا (type->type == FFI_TYPE_STRUCT) {
    إذا (var) {
      إذا (type->سائيز > 0)
        marshal_atom(cb, FFI_TYPE_اشارو, معلومات);
    } وإلا {
      صحيح i;
      
      لكل (i = 0; i < type->سائيز; i += حجم(size_t)) {
        marshal_atom(cb, FFI_TYPE_اشارو, معلومات);
        معلومات += حجم(size_t);
      }
    }
  }
#جيڪڏهن_پورو
  وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشارو) {
    marshal_atom(cb, type->type, معلومات);
  } وإلا {
      يادگيري_نقل(realign, معلومات, type->سائيز);
      إذا (type->سائيز > 0)
        marshal_atom(cb, FFI_TYPE_اشارو, realign);
      إذا (type->سائيز > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_اشارو, realign + 1);
  }
}

ساكن فراغ unmarshal_atom(call_builder *cb, صحيح type, فراغ *معلومات) {
  size_t قدر;

  إذا (cb->استعمال_ٿيل_integer == NARGREG) {
    قدر = *cb->استعمال_ٿيل_ڍير++;
  } وإلا {
    قدر = cb->aregs->r[cb->استعمال_ٿيل_integer++];
  }

  بدل (type) {
    حالة FFI_TYPE_UINT8: *(uint8_t *)معلومات = قدر; اكسر;
    حالة FFI_TYPE_SINT8: *(uint8_t *)معلومات = قدر; اكسر;
    حالة FFI_TYPE_UINT16: *(uint16_t *)معلومات = قدر; اكسر;
    حالة FFI_TYPE_SINT16: *(uint16_t *)معلومات = قدر; اكسر;
    حالة FFI_TYPE_UINT32: *(uint32_t *)معلومات = قدر; اكسر;
    حالة FFI_TYPE_SINT32: *(uint32_t *)معلومات = قدر; اكسر;
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_TYPE_UINT64: *(uint64_t *)معلومات = قدر; اكسر;
    حالة FFI_TYPE_SINT64: *(uint64_t *)معلومات = قدر; اكسر;
#جيڪڏهن_پورو
    حالة FFI_TYPE_اشارو: *(size_t *)معلومات = قدر; اكسر;
    افتراضي: FFI_ASSERT(0); اكسر;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساكن فراغ *unmarshal(call_builder *cb, ffi_type *type, صحيح var, فراغ *معلومات) {
  size_t realign[2];

#إذا defined(__ARC64_ARCH64__)
  فراغ *اشارو;

  إذا (type->سائيز > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_اشارو, (حرف*)&اشارو);
        أرجع اشارو;
    }
#وإلاإذا defined(__ARC64_ARCH32__)
  إذا (type->type == FFI_TYPE_STRUCT) {
    إذا (type->سائيز > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_اشارو, &realign[0]);
      يادگيري_نقل(معلومات, (ثابت فراغ*)realign[0], type->سائيز);
      أرجع معلومات;
    } وإلا {
      صحيح i;
      فراغ *pdata = معلومات;

      لكل (i = 0; i < type->سائيز; i += حجم(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_اشارو, pdata);
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

        لكل (i = 0; i < type->سائيز; i += حجم(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_اشارو, pdata);
          pdata += حجم(size_t);
        }
        أرجع معلومات;
      } وإلا {
        إذا (type->سائيز > 0)
          unmarshal_atom(cb, FFI_TYPE_اشارو, &realign[0]);
        يادگيري_نقل(معلومات, (ثابت فراغ*)realign[0], type->سائيز);
        أرجع معلومات;
      }
  }
#جيڪڏهن_پورو
  وإلا إذا (IS_INT(type->type) || type->type == FFI_TYPE_اشارو) {
    unmarshal_atom(cb, type->type, معلومات);
    أرجع معلومات;
  } وإلا {
      إذا (type->سائيز > 0)
        unmarshal_atom(cb, FFI_TYPE_اشارو, realign);
      إذا (type->سائيز > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_اشارو, realign + 1);
      يادگيري_نقل(معلومات, realign, type->سائيز);
      أرجع معلومات;
  }
}

ساكن صحيح passed_by_ref(ffi_type *type, صحيح var) {
  إذا (type->type == FFI_TYPE_STRUCT)
	  أرجع 1;
  
  أرجع type->سائيز > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
خارجي فراغ ffi_call_asm (فراغ *ڍير, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *گهيرو) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *گهيرو)
{
  صحيح return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_بائيٽون = FFI_ALIGN(2 * حجم(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_بائيٽون = FFI_ALIGN(cif->بائيٽون, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_سائيز = arg_بائيٽون + /*rval_bytes +*/ struct_بائيٽون + حجم(call_context);
  size_t alloc_بنياد = (size_t)alloca(alloc_سائيز);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.استعمال_ٿيل_integer = 0;
  cb.aregs = (call_context*)(alloc_بنياد + arg_بائيٽون /*+ rval_bytes*/ + struct_بائيٽون);
  cb.استعمال_ٿيل_ڍير = (فراغ*)alloc_بنياد;
  cb.struct_ڍير = (فراغ *)(alloc_بنياد + arg_بائيٽون /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  إذا (return_by_ref)
	  marshal(&cb, &ffi_type_اشارو, 0, &rvalue);

  صحيح i;
  لكل (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((فراغ *) alloc_بنياد, cb.aregs, fn, گهيرو);

  cb.استعمال_ٿيل_integer = 0;
  إذا (!return_by_ref && rvalue)
    {
	    إذا (IS_INT(cif->rtype->type)
	        && cif->rtype->سائيز < حجم (ffi_arg))
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
	     فراغ **avalue, فراغ *گهيرو)
{
  ffi_call_int(cif, fn, rvalue, avalue, گهيرو);
}

خارجي فراغ ffi_گهيرو_asm(فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو * گهيرو, ffi_cif * cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_معلومات, فراغ *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (گهيرو->tramp[0]);

#إذا defined(__ARC64_ARCH64__)
  size_t پتو_ffi_گهيرو = (size_t) ffi_گهيرو_asm;
#جيڪڏهن_پورو

  بدل (cif->abi)
    {
#إذا defined(__ARC64_ARCH64__)
    حالة FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = ڪوڊ_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = ڪوڊ_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = ڪوڊ_ENDIAN ((uint32_t)(پتو_ffi_گهيرو >> 32));
      tramp[3] = ڪوڊ_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = ڪوڊ_ENDIAN ((uint32_t)(پتو_ffi_گهيرو & 0xffffffff));
      tramp[5] = ڪوڊ_ENDIAN (0x20200300);	/* j [r12] */
      اكسر;
#وإلا
    حالة FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = ڪوڊ_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = ڪوڊ_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = ڪوڊ_ENDIAN ((uint32_t) ffi_گهيرو_asm);
      اكسر;
#جيڪڏهن_پورو

    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;
  cacheflush (codeloc, FFI_TRAMPOLINE_سائيز, BCACHE);

  أرجع FFI_OK;
}

خارجي فراغ ffi_go_گهيرو_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_گهيرو (ffi_go_گهيرو *گهيرو, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_پهريون_ABI || cif->abi >= FFI_آخري_ABI)
    أرجع FFI_BAD_ABI;

  گهيرو->tramp = (فراغ *) ffi_go_گهيرو_asm;
  گهيرو->cif = cif;
  گهيرو->fun = fun;

  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
فراغ FFI_HIDDEN
ffi_گهيرو_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_معلومات,
		   size_t *ڍير, call_context *aregs)
{
    فراغ **avalue = alloca(cif->nargs * حجم(فراغ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    حرف *astorage = alloca(cif->بائيٽون);
    حرف *ptr = astorage;
    فراغ *rvalue;
    call_builder cb;
    صحيح i;

    cb.aregs = aregs;
    cb.استعمال_ٿيل_integer = 0;
    cb.استعمال_ٿيل_ڍير = ڍير;
    
    /* handle hidden argument */
    إذا (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_اشارو, 0, &rvalue);
    وإلا
      rvalue = alloca(cif->rtype->سائيز);

    لكل (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->سائيز;
    }

    fun (cif, rvalue, avalue, user_معلومات);

    إذا (cif->rtype->type != FFI_TYPE_VOID) {
        cb.استعمال_ٿيل_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
