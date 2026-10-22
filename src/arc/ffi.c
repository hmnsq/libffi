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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/cachectl.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NARGREG 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STKALIGN 4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXCOPYARG (2 * ਆਕਾਰ(ਦੁੱਗਣਾ))

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  ਅੱਖਰ ਚੌਖਟਾ[16];
} call_context;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ call_builder
{
  call_context *aregs;
  ਪੂਰਨ ਵਰਤਿਆ_integer;
  //int used_float;
  size_t *ਵਰਤਿਆ_ਢੇਰ;
  ਖਾਲੀ *struct_ਢੇਰ;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ਜੇ defined(__ARC64_ARCH64__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#ਜੇ_ਅੰਤ

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#ਜੇ __BIG_ENDIAN__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਕੋਡ_ENDIAN(x) (x)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਕੋਡ_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#ਜੇ_ਅੰਤ

/* Perform machine dependent cif processing.  */
ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ) cif->rtype->type;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ) cif->rtype->type;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਮੂਲ:
      cif->flags = FFI_TYPE_INT;
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ਸਥਿਰ ਖਾਲੀ marshal_atom(call_builder *cb, ਪੂਰਨ type, ਖਾਲੀ *ਅੰਕੜੇ) {
  size_t ਮੁੱਲ = 0;
  ਚੋਣ (type) {
    ਹਾਲਤ FFI_TYPE_UINT8: ਮੁੱਲ = *(uint8_t *)ਅੰਕੜੇ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT8: ਮੁੱਲ = *(int8_t *)ਅੰਕੜੇ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16: ਮੁੱਲ = *(uint16_t *)ਅੰਕੜੇ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16: ਮੁੱਲ = *(int16_t *)ਅੰਕੜੇ; ਰੋਕੋ;
    /* 32-bit quantities are always sign-extended in the ABI */
    ਹਾਲਤ FFI_TYPE_UINT32: ਮੁੱਲ = *(int32_t *)ਅੰਕੜੇ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT32: ਮੁੱਲ = *(int32_t *)ਅੰਕੜੇ; ਰੋਕੋ;
#ਜੇ defined(__ARC64_ARCH64__)
    ਹਾਲਤ FFI_TYPE_UINT64: ਮੁੱਲ = *(uint64_t *)ਅੰਕੜੇ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT64: ਮੁੱਲ = *(int64_t *)ਅੰਕੜੇ; ਰੋਕੋ;
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ: ਮੁੱਲ = *(size_t *)ਅੰਕੜੇ; ਰੋਕੋ;
    ਮੂਲ: FFI_ASSERT(0); ਰੋਕੋ;
  }

  ਜੇ (cb->ਵਰਤਿਆ_integer == NARGREG) {
    *cb->ਵਰਤਿਆ_ਢੇਰ++ = ਮੁੱਲ;
  } ਨਹੀਂਤਾਂ {
    cb->aregs->r[cb->ਵਰਤਿਆ_integer++] = ਮੁੱਲ;
  }
}

/* adds an argument to a call, or a not by reference return value */
ਸਥਿਰ ਖਾਲੀ marshal(call_builder *cb, ffi_type *type, ਪੂਰਨ var, ਖਾਲੀ *ਅੰਕੜੇ) {
  size_t realign[2];

#ਜੇ (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  ਜੇ (type->ਮਾਪ > 2 * __SIZEOF_POINTER__) {
    ਜੇ (var) {
      marshal_atom(cb, FFI_TYPE_ਸੂਚਕ, &ਅੰਕੜੇ);
    } ਨਹੀਂਤਾਂ {
      /* copy to stack and pass by reference */
      ਅੰਕੜੇ = ਸਮ੍ਰਿਤੀ_ਨਕਲ (cb->struct_ਢੇਰ, ਅੰਕੜੇ, type->ਮਾਪ);
      cb->struct_ਢੇਰ = (size_t *) FFI_ALIGN ((ਅੱਖਰ *) cb->struct_ਢੇਰ + type->ਮਾਪ, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_ਸੂਚਕ, &ਅੰਕੜੇ);
    }
  }
#ਨਹੀਂਤਾਂ
  ਜੇ (type->type == FFI_TYPE_STRUCT) {
    ਜੇ (var) {
      ਜੇ (type->ਮਾਪ > 0)
        marshal_atom(cb, FFI_TYPE_ਸੂਚਕ, ਅੰਕੜੇ);
    } ਨਹੀਂਤਾਂ {
      ਪੂਰਨ i;
      
      ਲਈ (i = 0; i < type->ਮਾਪ; i += ਆਕਾਰ(size_t)) {
        marshal_atom(cb, FFI_TYPE_ਸੂਚਕ, ਅੰਕੜੇ);
        ਅੰਕੜੇ += ਆਕਾਰ(size_t);
      }
    }
  }
#ਜੇ_ਅੰਤ
  ਨਹੀਂਤਾਂ ਜੇ (IS_INT(type->type) || type->type == FFI_TYPE_ਸੂਚਕ) {
    marshal_atom(cb, type->type, ਅੰਕੜੇ);
  } ਨਹੀਂਤਾਂ {
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(realign, ਅੰਕੜੇ, type->ਮਾਪ);
      ਜੇ (type->ਮਾਪ > 0)
        marshal_atom(cb, FFI_TYPE_ਸੂਚਕ, realign);
      ਜੇ (type->ਮਾਪ > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_ਸੂਚਕ, realign + 1);
  }
}

ਸਥਿਰ ਖਾਲੀ unmarshal_atom(call_builder *cb, ਪੂਰਨ type, ਖਾਲੀ *ਅੰਕੜੇ) {
  size_t ਮੁੱਲ;

  ਜੇ (cb->ਵਰਤਿਆ_integer == NARGREG) {
    ਮੁੱਲ = *cb->ਵਰਤਿਆ_ਢੇਰ++;
  } ਨਹੀਂਤਾਂ {
    ਮੁੱਲ = cb->aregs->r[cb->ਵਰਤਿਆ_integer++];
  }

  ਚੋਣ (type) {
    ਹਾਲਤ FFI_TYPE_UINT8: *(uint8_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT8: *(uint8_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16: *(uint16_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16: *(uint16_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT32: *(uint32_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT32: *(uint32_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
#ਜੇ defined(__ARC64_ARCH64__)
    ਹਾਲਤ FFI_TYPE_UINT64: *(uint64_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT64: *(uint64_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ: *(size_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
    ਮੂਲ: FFI_ASSERT(0); ਰੋਕੋ;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ਸਥਿਰ ਖਾਲੀ *unmarshal(call_builder *cb, ffi_type *type, ਪੂਰਨ var, ਖਾਲੀ *ਅੰਕੜੇ) {
  size_t realign[2];

#ਜੇ defined(__ARC64_ARCH64__)
  ਖਾਲੀ *ਸੂਚਕ;

  ਜੇ (type->ਮਾਪ > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_ਸੂਚਕ, (ਅੱਖਰ*)&ਸੂਚਕ);
        ਵਾਪਸ ਸੂਚਕ;
    }
#ਜਾਂਜੇ defined(__ARC64_ARCH32__)
  ਜੇ (type->type == FFI_TYPE_STRUCT) {
    ਜੇ (type->ਮਾਪ > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_ਸੂਚਕ, &realign[0]);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ, (ਅਚਲ ਖਾਲੀ*)realign[0], type->ਮਾਪ);
      ਵਾਪਸ ਅੰਕੜੇ;
    } ਨਹੀਂਤਾਂ {
      ਪੂਰਨ i;
      ਖਾਲੀ *pdata = ਅੰਕੜੇ;

      ਲਈ (i = 0; i < type->ਮਾਪ; i += ਆਕਾਰ(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_ਸੂਚਕ, pdata);
        pdata += ਆਕਾਰ(size_t);
      }
      ਵਾਪਸ ਅੰਕੜੇ;
    }
  }
#ਨਹੀਂਤਾਂ
  ਜੇ (type->type == FFI_TYPE_STRUCT) {

      ਜੇ (var) {
        ਪੂਰਨ i;
        ਖਾਲੀ *pdata = ਅੰਕੜੇ;

        ਲਈ (i = 0; i < type->ਮਾਪ; i += ਆਕਾਰ(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_ਸੂਚਕ, pdata);
          pdata += ਆਕਾਰ(size_t);
        }
        ਵਾਪਸ ਅੰਕੜੇ;
      } ਨਹੀਂਤਾਂ {
        ਜੇ (type->ਮਾਪ > 0)
          unmarshal_atom(cb, FFI_TYPE_ਸੂਚਕ, &realign[0]);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ, (ਅਚਲ ਖਾਲੀ*)realign[0], type->ਮਾਪ);
        ਵਾਪਸ ਅੰਕੜੇ;
      }
  }
#ਜੇ_ਅੰਤ
  ਨਹੀਂਤਾਂ ਜੇ (IS_INT(type->type) || type->type == FFI_TYPE_ਸੂਚਕ) {
    unmarshal_atom(cb, type->type, ਅੰਕੜੇ);
    ਵਾਪਸ ਅੰਕੜੇ;
  } ਨਹੀਂਤਾਂ {
      ਜੇ (type->ਮਾਪ > 0)
        unmarshal_atom(cb, FFI_TYPE_ਸੂਚਕ, realign);
      ਜੇ (type->ਮਾਪ > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_ਸੂਚਕ, realign + 1);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ, realign, type->ਮਾਪ);
      ਵਾਪਸ ਅੰਕੜੇ;
  }
}

ਸਥਿਰ ਪੂਰਨ passed_by_ref(ffi_type *type, ਪੂਰਨ var) {
  ਜੇ (type->type == FFI_TYPE_STRUCT)
	  ਵਾਪਸ 1;
  
  ਵਾਪਸ type->ਮਾਪ > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
ਬਾਹਰੀ ਖਾਲੀ ffi_call_asm (ਖਾਲੀ *ਢੇਰ, ਬਣਤਰ call_context *regs,
			  ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *ਘੇਰਾ) FFI_HIDDEN;

ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue,
	      ਖਾਲੀ *ਘੇਰਾ)
{
  ਪੂਰਨ return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_ਬਾਈਟਾਂ = FFI_ALIGN(2 * ਆਕਾਰ(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_ਬਾਈਟਾਂ = FFI_ALIGN(cif->ਬਾਈਟਾਂ, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_ਮਾਪ = arg_ਬਾਈਟਾਂ + /*rval_bytes +*/ struct_ਬਾਈਟਾਂ + ਆਕਾਰ(call_context);
  size_t alloc_ਅਧਾਰ = (size_t)alloca(alloc_ਮਾਪ);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.ਵਰਤਿਆ_integer = 0;
  cb.aregs = (call_context*)(alloc_ਅਧਾਰ + arg_ਬਾਈਟਾਂ /*+ rval_bytes*/ + struct_ਬਾਈਟਾਂ);
  cb.ਵਰਤਿਆ_ਢੇਰ = (ਖਾਲੀ*)alloc_ਅਧਾਰ;
  cb.struct_ਢੇਰ = (ਖਾਲੀ *)(alloc_ਅਧਾਰ + arg_ਬਾਈਟਾਂ /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  ਜੇ (return_by_ref)
	  marshal(&cb, &ffi_type_ਸੂਚਕ, 0, &rvalue);

  ਪੂਰਨ i;
  ਲਈ (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((ਖਾਲੀ *) alloc_ਅਧਾਰ, cb.aregs, fn, ਘੇਰਾ);

  cb.ਵਰਤਿਆ_integer = 0;
  ਜੇ (!return_by_ref && rvalue)
    {
	    ਜੇ (IS_INT(cif->rtype->type)
	        && cif->rtype->ਮਾਪ < ਆਕਾਰ (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    ਚੋਣ (cif->rtype->type) {
	      ਹਾਲਤ FFI_TYPE_SINT8:
	      ਹਾਲਤ FFI_TYPE_SINT16:
	      ਹਾਲਤ FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (ਆਕਾਰ (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      ਰੋਕੋ;
	      ਹਾਲਤ FFI_TYPE_UINT8:
	      ਹਾਲਤ FFI_TYPE_UINT16:
	      ਹਾਲਤ FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (ਆਕਾਰ (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      ਰੋਕੋ;
	      }
	    }
	    ਨਹੀਂਤਾਂ
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ਨਿਰਾ);
}

ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int(cif, fn, rvalue, avalue, ਘੇਰਾ);
}

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_asm(ਖਾਲੀ) FFI_HIDDEN;

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ * ਘੇਰਾ, ffi_cif * cif,
		      ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		      ਖਾਲੀ *user_ਅੰਕੜੇ, ਖਾਲੀ *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (ਘੇਰਾ->tramp[0]);

#ਜੇ defined(__ARC64_ARCH64__)
  size_t ਪਤਾ_ffi_ਘੇਰਾ = (size_t) ffi_ਘੇਰਾ_asm;
#ਜੇ_ਅੰਤ

  ਚੋਣ (cif->abi)
    {
#ਜੇ defined(__ARC64_ARCH64__)
    ਹਾਲਤ FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = ਕੋਡ_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = ਕੋਡ_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = ਕੋਡ_ENDIAN ((uint32_t)(ਪਤਾ_ffi_ਘੇਰਾ >> 32));
      tramp[3] = ਕੋਡ_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = ਕੋਡ_ENDIAN ((uint32_t)(ਪਤਾ_ffi_ਘੇਰਾ & 0xffffffff));
      tramp[5] = ਕੋਡ_ENDIAN (0x20200300);	/* j [r12] */
      ਰੋਕੋ;
#ਨਹੀਂਤਾਂ
    ਹਾਲਤ FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = ਕੋਡ_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = ਕੋਡ_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = ਕੋਡ_ENDIAN ((uint32_t) ffi_ਘੇਰਾ_asm);
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ

    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_ABI;
    }

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  cacheflush (codeloc, FFI_TRAMPOLINE_ਮਾਪ, BCACHE);

  ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_asm (ਖਾਲੀ) FFI_HIDDEN;

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ *ਘੇਰਾ, ffi_cif *cif,
		     ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *))
{
  ਜੇ (cif->abi <= FFI_ਪਹਿਲਾ_ABI || cif->abi >= FFI_ਆਖਰੀ_ABI)
    ਵਾਪਸ FFI_BAD_ABI;

  ਘੇਰਾ->tramp = (ਖਾਲੀ *) ffi_go_ਘੇਰਾ_asm;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
ਖਾਲੀ FFI_HIDDEN
ffi_ਘੇਰਾ_inner (ffi_cif *cif,
		   ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		   ਖਾਲੀ *user_ਅੰਕੜੇ,
		   size_t *ਢੇਰ, call_context *aregs)
{
    ਖਾਲੀ **avalue = alloca(cif->nargs * ਆਕਾਰ(ਖਾਲੀ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    ਅੱਖਰ *astorage = alloca(cif->ਬਾਈਟਾਂ);
    ਅੱਖਰ *ptr = astorage;
    ਖਾਲੀ *rvalue;
    call_builder cb;
    ਪੂਰਨ i;

    cb.aregs = aregs;
    cb.ਵਰਤਿਆ_integer = 0;
    cb.ਵਰਤਿਆ_ਢੇਰ = ਢੇਰ;
    
    /* handle hidden argument */
    ਜੇ (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_ਸੂਚਕ, 0, &rvalue);
    ਨਹੀਂਤਾਂ
      rvalue = alloca(cif->rtype->ਮਾਪ);

    ਲਈ (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->ਮਾਪ;
    }

    fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);

    ਜੇ (cif->rtype->type != FFI_TYPE_VOID) {
        cb.ਵਰਤਿਆ_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
