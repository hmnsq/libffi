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

#تعریف_کرو NARGREG 8
#تعریف_کرو STKALIGN 4
#تعریف_کرو MAXCOPYARG (2 * حجم(دوہرا))

قسمتعریف ساخت call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  حرف چوکھٹا[16];
} call_context;

قسمتعریف ساخت call_builder
{
  call_context *aregs;
  صحیح مستعمل_integer;
  //int used_float;
  size_t *مستعمل_ڈھیر;
  خالی *struct_ڈھیر;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#اگر defined(__ARC64_ARCH64__)
#تعریف_کرو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ورنہ
#تعریف_کرو IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#اگر_ختم

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#اگر __BIG_ENDIAN__
#تعریف_کرو رمز_ENDIAN(x) (x)
#ورنہ
#تعریف_کرو رمز_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#اگر_ختم

/* Perform machine dependent cif processing.  */
ffi_حالت
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      cif->flags = (غیرعلامتی) cif->rtype->type;
      توڑو;

    حالت FFI_TYPE_STRUCT:
      cif->flags = (غیرعلامتی) cif->rtype->type;
      توڑو;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      توڑو;

    حالت FFI_TYPE_FLOAT:
    طےشدہ:
      cif->flags = FFI_TYPE_INT;
      توڑو;
    }

  واپس FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ساکن خالی marshal_atom(call_builder *cb, صحیح type, خالی *ڈیٹا) {
  size_t قدر = 0;
  انتخاب (type) {
    حالت FFI_TYPE_UINT8: قدر = *(uint8_t *)ڈیٹا; توڑو;
    حالت FFI_TYPE_SINT8: قدر = *(int8_t *)ڈیٹا; توڑو;
    حالت FFI_TYPE_UINT16: قدر = *(uint16_t *)ڈیٹا; توڑو;
    حالت FFI_TYPE_SINT16: قدر = *(int16_t *)ڈیٹا; توڑو;
    /* 32-bit quantities are always sign-extended in the ABI */
    حالت FFI_TYPE_UINT32: قدر = *(int32_t *)ڈیٹا; توڑو;
    حالت FFI_TYPE_SINT32: قدر = *(int32_t *)ڈیٹا; توڑو;
#اگر defined(__ARC64_ARCH64__)
    حالت FFI_TYPE_UINT64: قدر = *(uint64_t *)ڈیٹا; توڑو;
    حالت FFI_TYPE_SINT64: قدر = *(int64_t *)ڈیٹا; توڑو;
#اگر_ختم
    حالت FFI_TYPE_اشارہ_گر: قدر = *(size_t *)ڈیٹا; توڑو;
    طےشدہ: FFI_ASSERT(0); توڑو;
  }

  اگر (cb->مستعمل_integer == NARGREG) {
    *cb->مستعمل_ڈھیر++ = قدر;
  } ورنہ {
    cb->aregs->r[cb->مستعمل_integer++] = قدر;
  }
}

/* adds an argument to a call, or a not by reference return value */
ساکن خالی marshal(call_builder *cb, ffi_type *type, صحیح var, خالی *ڈیٹا) {
  size_t realign[2];

#اگر (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  اگر (type->سائز > 2 * __SIZEOF_POINTER__) {
    اگر (var) {
      marshal_atom(cb, FFI_TYPE_اشارہ_گر, &ڈیٹا);
    } ورنہ {
      /* copy to stack and pass by reference */
      ڈیٹا = حافظہ_نقل (cb->struct_ڈھیر, ڈیٹا, type->سائز);
      cb->struct_ڈھیر = (size_t *) FFI_ALIGN ((حرف *) cb->struct_ڈھیر + type->سائز, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_اشارہ_گر, &ڈیٹا);
    }
  }
#ورنہ
  اگر (type->type == FFI_TYPE_STRUCT) {
    اگر (var) {
      اگر (type->سائز > 0)
        marshal_atom(cb, FFI_TYPE_اشارہ_گر, ڈیٹا);
    } ورنہ {
      صحیح i;
      
      برائے (i = 0; i < type->سائز; i += حجم(size_t)) {
        marshal_atom(cb, FFI_TYPE_اشارہ_گر, ڈیٹا);
        ڈیٹا += حجم(size_t);
      }
    }
  }
#اگر_ختم
  ورنہ اگر (IS_INT(type->type) || type->type == FFI_TYPE_اشارہ_گر) {
    marshal_atom(cb, type->type, ڈیٹا);
  } ورنہ {
      حافظہ_نقل(realign, ڈیٹا, type->سائز);
      اگر (type->سائز > 0)
        marshal_atom(cb, FFI_TYPE_اشارہ_گر, realign);
      اگر (type->سائز > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_اشارہ_گر, realign + 1);
  }
}

ساکن خالی unmarshal_atom(call_builder *cb, صحیح type, خالی *ڈیٹا) {
  size_t قدر;

  اگر (cb->مستعمل_integer == NARGREG) {
    قدر = *cb->مستعمل_ڈھیر++;
  } ورنہ {
    قدر = cb->aregs->r[cb->مستعمل_integer++];
  }

  انتخاب (type) {
    حالت FFI_TYPE_UINT8: *(uint8_t *)ڈیٹا = قدر; توڑو;
    حالت FFI_TYPE_SINT8: *(uint8_t *)ڈیٹا = قدر; توڑو;
    حالت FFI_TYPE_UINT16: *(uint16_t *)ڈیٹا = قدر; توڑو;
    حالت FFI_TYPE_SINT16: *(uint16_t *)ڈیٹا = قدر; توڑو;
    حالت FFI_TYPE_UINT32: *(uint32_t *)ڈیٹا = قدر; توڑو;
    حالت FFI_TYPE_SINT32: *(uint32_t *)ڈیٹا = قدر; توڑو;
#اگر defined(__ARC64_ARCH64__)
    حالت FFI_TYPE_UINT64: *(uint64_t *)ڈیٹا = قدر; توڑو;
    حالت FFI_TYPE_SINT64: *(uint64_t *)ڈیٹا = قدر; توڑو;
#اگر_ختم
    حالت FFI_TYPE_اشارہ_گر: *(size_t *)ڈیٹا = قدر; توڑو;
    طےشدہ: FFI_ASSERT(0); توڑو;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ساکن خالی *unmarshal(call_builder *cb, ffi_type *type, صحیح var, خالی *ڈیٹا) {
  size_t realign[2];

#اگر defined(__ARC64_ARCH64__)
  خالی *اشارہ_گر;

  اگر (type->سائز > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, (حرف*)&اشارہ_گر);
        واپس اشارہ_گر;
    }
#وگرنہ_اگر defined(__ARC64_ARCH32__)
  اگر (type->type == FFI_TYPE_STRUCT) {
    اگر (type->سائز > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, &realign[0]);
      حافظہ_نقل(ڈیٹا, (ثابت خالی*)realign[0], type->سائز);
      واپس ڈیٹا;
    } ورنہ {
      صحیح i;
      خالی *pdata = ڈیٹا;

      برائے (i = 0; i < type->سائز; i += حجم(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, pdata);
        pdata += حجم(size_t);
      }
      واپس ڈیٹا;
    }
  }
#ورنہ
  اگر (type->type == FFI_TYPE_STRUCT) {

      اگر (var) {
        صحیح i;
        خالی *pdata = ڈیٹا;

        برائے (i = 0; i < type->سائز; i += حجم(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, pdata);
          pdata += حجم(size_t);
        }
        واپس ڈیٹا;
      } ورنہ {
        اگر (type->سائز > 0)
          unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, &realign[0]);
        حافظہ_نقل(ڈیٹا, (ثابت خالی*)realign[0], type->سائز);
        واپس ڈیٹا;
      }
  }
#اگر_ختم
  ورنہ اگر (IS_INT(type->type) || type->type == FFI_TYPE_اشارہ_گر) {
    unmarshal_atom(cb, type->type, ڈیٹا);
    واپس ڈیٹا;
  } ورنہ {
      اگر (type->سائز > 0)
        unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, realign);
      اگر (type->سائز > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_اشارہ_گر, realign + 1);
      حافظہ_نقل(ڈیٹا, realign, type->سائز);
      واپس ڈیٹا;
  }
}

ساکن صحیح passed_by_ref(ffi_type *type, صحیح var) {
  اگر (type->type == FFI_TYPE_STRUCT)
	  واپس 1;
  
  واپس type->سائز > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
خارجی خالی ffi_call_asm (خالی *ڈھیر, ساخت call_context *regs,
			  خالی (*fn) (خالی), خالی *احاطہ) FFI_HIDDEN;

ساکن خالی
ffi_call_int (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue, خالی **avalue,
	      خالی *احاطہ)
{
  صحیح return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_بائٹیں = FFI_ALIGN(2 * حجم(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_بائٹیں = FFI_ALIGN(cif->بائٹیں, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_سائز = arg_بائٹیں + /*rval_bytes +*/ struct_بائٹیں + حجم(call_context);
  size_t alloc_بنیاد = (size_t)alloca(alloc_سائز);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.مستعمل_integer = 0;
  cb.aregs = (call_context*)(alloc_بنیاد + arg_بائٹیں /*+ rval_bytes*/ + struct_بائٹیں);
  cb.مستعمل_ڈھیر = (خالی*)alloc_بنیاد;
  cb.struct_ڈھیر = (خالی *)(alloc_بنیاد + arg_بائٹیں /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  اگر (return_by_ref)
	  marshal(&cb, &ffi_type_اشارہ_گر, 0, &rvalue);

  صحیح i;
  برائے (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((خالی *) alloc_بنیاد, cb.aregs, fn, احاطہ);

  cb.مستعمل_integer = 0;
  اگر (!return_by_ref && rvalue)
    {
	    اگر (IS_INT(cif->rtype->type)
	        && cif->rtype->سائز < حجم (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    انتخاب (cif->rtype->type) {
	      حالت FFI_TYPE_SINT8:
	      حالت FFI_TYPE_SINT16:
	      حالت FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (حجم (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      توڑو;
	      حالت FFI_TYPE_UINT8:
	      حالت FFI_TYPE_UINT16:
	      حالت FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (حجم (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      توڑو;
	      }
	    }
	    ورنہ
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, تہی);
}

خالی
ffi_call_go (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  ffi_call_int(cif, fn, rvalue, avalue, احاطہ);
}

خارجی خالی ffi_احاطہ_asm(خالی) FFI_HIDDEN;

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ * احاطہ, ffi_cif * cif,
		      خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
		      خالی *user_ڈیٹا, خالی *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (احاطہ->tramp[0]);

#اگر defined(__ARC64_ARCH64__)
  size_t پتہ_ffi_احاطہ = (size_t) ffi_احاطہ_asm;
#اگر_ختم

  انتخاب (cif->abi)
    {
#اگر defined(__ARC64_ARCH64__)
    حالت FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = رمز_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = رمز_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = رمز_ENDIAN ((uint32_t)(پتہ_ffi_احاطہ >> 32));
      tramp[3] = رمز_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = رمز_ENDIAN ((uint32_t)(پتہ_ffi_احاطہ & 0xffffffff));
      tramp[5] = رمز_ENDIAN (0x20200300);	/* j [r12] */
      توڑو;
#ورنہ
    حالت FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = رمز_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = رمز_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = رمز_ENDIAN ((uint32_t) ffi_احاطہ_asm);
      توڑو;
#اگر_ختم

    طےشدہ:
      واپس FFI_BAD_ABI;
    }

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;
  cacheflush (codeloc, FFI_TRAMPOLINE_سائز, BCACHE);

  واپس FFI_OK;
}

خارجی خالی ffi_go_احاطہ_asm (خالی) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ *احاطہ, ffi_cif *cif,
		     خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *))
{
  اگر (cif->abi <= FFI_پہلا_ABI || cif->abi >= FFI_آخری_ABI)
    واپس FFI_BAD_ABI;

  احاطہ->tramp = (خالی *) ffi_go_احاطہ_asm;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
خالی FFI_HIDDEN
ffi_احاطہ_inner (ffi_cif *cif,
		   خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
		   خالی *user_ڈیٹا,
		   size_t *ڈھیر, call_context *aregs)
{
    خالی **avalue = alloca(cif->nargs * حجم(خالی*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    حرف *astorage = alloca(cif->بائٹیں);
    حرف *ptr = astorage;
    خالی *rvalue;
    call_builder cb;
    صحیح i;

    cb.aregs = aregs;
    cb.مستعمل_integer = 0;
    cb.مستعمل_ڈھیر = ڈھیر;
    
    /* handle hidden argument */
    اگر (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_اشارہ_گر, 0, &rvalue);
    ورنہ
      rvalue = alloca(cif->rtype->سائز);

    برائے (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->سائز;
    }

    fun (cif, rvalue, avalue, user_ڈیٹا);

    اگر (cif->rtype->type != FFI_TYPE_VOID) {
        cb.مستعمل_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
