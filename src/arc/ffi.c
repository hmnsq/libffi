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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>

#ಅನ್ತರ್ಭಾವ <sys/cachectl.h>

#ವ್ಯಾಖ್ಯಾನಿಸು NARGREG 8
#ವ್ಯಾಖ್ಯಾನಿಸು STKALIGN 4
#ವ್ಯಾಖ್ಯಾನಿಸು MAXCOPYARG (2 * ಗಾತ್ರ(ದ್ವಿಗುಣ))

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  ಅಕ್ಷರ ಚೌಕಟ್ಟು[16];
} call_context;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ call_builder
{
  call_context *aregs;
  ಪೂರ್ಣಾಂಕ ಬಳಸಿದ_integer;
  //int used_float;
  size_t *ಬಳಸಿದ_ರಾಶಿ;
  ಶೂನ್ಯ *struct_ರಾಶಿ;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ಆದರೆ defined(__ARC64_ARCH64__)
#ವ್ಯಾಖ್ಯಾನಿಸು IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#ಆದರೆ_ಅಂತ್ಯ

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#ಆದರೆ __BIG_ENDIAN__
#ವ್ಯಾಖ್ಯಾನಿಸು ಸಂಕೇತ_ENDIAN(x) (x)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ಸಂಕೇತ_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#ಆದರೆ_ಅಂತ್ಯ

/* Perform machine dependent cif processing.  */
ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      cif->flags = (ಚಿಹ್ನೆರಹಿತ) cif->rtype->type;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      cif->flags = (ಚಿಹ್ನೆರಹಿತ) cif->rtype->type;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಪೂರ್ವನಿಯೋಜಿತ:
      cif->flags = FFI_TYPE_INT;
      ನಿಲ್ಲಿಸು;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ಸ್ಥಿರ ಶೂನ್ಯ marshal_atom(call_builder *cb, ಪೂರ್ಣಾಂಕ type, ಶೂನ್ಯ *ದತ್ತಾಂಶ) {
  size_t ಮೌಲ್ಯ = 0;
  ಆಯ್ಕೆ (type) {
    ಸ್ಥಿತಿ FFI_TYPE_UINT8: ಮೌಲ್ಯ = *(uint8_t *)ದತ್ತಾಂಶ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8: ಮೌಲ್ಯ = *(int8_t *)ದತ್ತಾಂಶ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16: ಮೌಲ್ಯ = *(uint16_t *)ದತ್ತಾಂಶ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16: ಮೌಲ್ಯ = *(int16_t *)ದತ್ತಾಂಶ; ನಿಲ್ಲಿಸು;
    /* 32-bit quantities are always sign-extended in the ABI */
    ಸ್ಥಿತಿ FFI_TYPE_UINT32: ಮೌಲ್ಯ = *(int32_t *)ದತ್ತಾಂಶ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT32: ಮೌಲ್ಯ = *(int32_t *)ದತ್ತಾಂಶ; ನಿಲ್ಲಿಸು;
#ಆದರೆ defined(__ARC64_ARCH64__)
    ಸ್ಥಿತಿ FFI_TYPE_UINT64: ಮೌಲ್ಯ = *(uint64_t *)ದತ್ತಾಂಶ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT64: ಮೌಲ್ಯ = *(int64_t *)ದತ್ತಾಂಶ; ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ: ಮೌಲ್ಯ = *(size_t *)ದತ್ತಾಂಶ; ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ: FFI_ASSERT(0); ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ (cb->ಬಳಸಿದ_integer == NARGREG) {
    *cb->ಬಳಸಿದ_ರಾಶಿ++ = ಮೌಲ್ಯ;
  } ಇಲ್ಲದಿದ್ದರೆ {
    cb->aregs->r[cb->ಬಳಸಿದ_integer++] = ಮೌಲ್ಯ;
  }
}

/* adds an argument to a call, or a not by reference return value */
ಸ್ಥಿರ ಶೂನ್ಯ marshal(call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಂಕ var, ಶೂನ್ಯ *ದತ್ತಾಂಶ) {
  size_t realign[2];

#ಆದರೆ (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  ಆದರೆ (type->ಗಾತ್ರ_2 > 2 * __SIZEOF_POINTER__) {
    ಆದರೆ (var) {
      marshal_atom(cb, FFI_TYPE_ಸೂಚಕ, &ದತ್ತಾಂಶ);
    } ಇಲ್ಲದಿದ್ದರೆ {
      /* copy to stack and pass by reference */
      ದತ್ತಾಂಶ = ಸ್ಮರಣೆ_ನಕಲು (cb->struct_ರಾಶಿ, ದತ್ತಾಂಶ, type->ಗಾತ್ರ_2);
      cb->struct_ರಾಶಿ = (size_t *) FFI_ALIGN ((ಅಕ್ಷರ *) cb->struct_ರಾಶಿ + type->ಗಾತ್ರ_2, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_ಸೂಚಕ, &ದತ್ತಾಂಶ);
    }
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (type->type == FFI_TYPE_STRUCT) {
    ಆದರೆ (var) {
      ಆದರೆ (type->ಗಾತ್ರ_2 > 0)
        marshal_atom(cb, FFI_TYPE_ಸೂಚಕ, ದತ್ತಾಂಶ);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಪೂರ್ಣಾಂಕ i;
      
      ಗಾಗಿ (i = 0; i < type->ಗಾತ್ರ_2; i += ಗಾತ್ರ(size_t)) {
        marshal_atom(cb, FFI_TYPE_ಸೂಚಕ, ದತ್ತಾಂಶ);
        ದತ್ತಾಂಶ += ಗಾತ್ರ(size_t);
      }
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_INT(type->type) || type->type == FFI_TYPE_ಸೂಚಕ) {
    marshal_atom(cb, type->type, ದತ್ತಾಂಶ);
  } ಇಲ್ಲದಿದ್ದರೆ {
      ಸ್ಮರಣೆ_ನಕಲು(realign, ದತ್ತಾಂಶ, type->ಗಾತ್ರ_2);
      ಆದರೆ (type->ಗಾತ್ರ_2 > 0)
        marshal_atom(cb, FFI_TYPE_ಸೂಚಕ, realign);
      ಆದರೆ (type->ಗಾತ್ರ_2 > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_ಸೂಚಕ, realign + 1);
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ unmarshal_atom(call_builder *cb, ಪೂರ್ಣಾಂಕ type, ಶೂನ್ಯ *ದತ್ತಾಂಶ) {
  size_t ಮೌಲ್ಯ;

  ಆದರೆ (cb->ಬಳಸಿದ_integer == NARGREG) {
    ಮೌಲ್ಯ = *cb->ಬಳಸಿದ_ರಾಶಿ++;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಮೌಲ್ಯ = cb->aregs->r[cb->ಬಳಸಿದ_integer++];
  }

  ಆಯ್ಕೆ (type) {
    ಸ್ಥಿತಿ FFI_TYPE_UINT8: *(uint8_t *)ದತ್ತಾಂಶ = ಮೌಲ್ಯ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8: *(uint8_t *)ದತ್ತಾಂಶ = ಮೌಲ್ಯ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16: *(uint16_t *)ದತ್ತಾಂಶ = ಮೌಲ್ಯ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16: *(uint16_t *)ದತ್ತಾಂಶ = ಮೌಲ್ಯ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT32: *(uint32_t *)ದತ್ತಾಂಶ = ಮೌಲ್ಯ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT32: *(uint32_t *)ದತ್ತಾಂಶ = ಮೌಲ್ಯ; ನಿಲ್ಲಿಸು;
#ಆದರೆ defined(__ARC64_ARCH64__)
    ಸ್ಥಿತಿ FFI_TYPE_UINT64: *(uint64_t *)ದತ್ತಾಂಶ = ಮೌಲ್ಯ; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT64: *(uint64_t *)ದತ್ತಾಂಶ = ಮೌಲ್ಯ; ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ: *(size_t *)ದತ್ತಾಂಶ = ಮೌಲ್ಯ; ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ: FFI_ASSERT(0); ನಿಲ್ಲಿಸು;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ಸ್ಥಿರ ಶೂನ್ಯ *unmarshal(call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಂಕ var, ಶೂನ್ಯ *ದತ್ತಾಂಶ) {
  size_t realign[2];

#ಆದರೆ defined(__ARC64_ARCH64__)
  ಶೂನ್ಯ *ಸೂಚಕ_2;

  ಆದರೆ (type->ಗಾತ್ರ_2 > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_ಸೂಚಕ, (ಅಕ್ಷರ*)&ಸೂಚಕ_2);
        ಹಿಂತಿರುಗಿ ಸೂಚಕ_2;
    }
#ಅಥವಾಆದರೆ defined(__ARC64_ARCH32__)
  ಆದರೆ (type->type == FFI_TYPE_STRUCT) {
    ಆದರೆ (type->ಗಾತ್ರ_2 > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_ಸೂಚಕ, &realign[0]);
      ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)realign[0], type->ಗಾತ್ರ_2);
      ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಪೂರ್ಣಾಂಕ i;
      ಶೂನ್ಯ *pdata = ದತ್ತಾಂಶ;

      ಗಾಗಿ (i = 0; i < type->ಗಾತ್ರ_2; i += ಗಾತ್ರ(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_ಸೂಚಕ, pdata);
        pdata += ಗಾತ್ರ(size_t);
      }
      ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
    }
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (type->type == FFI_TYPE_STRUCT) {

      ಆದರೆ (var) {
        ಪೂರ್ಣಾಂಕ i;
        ಶೂನ್ಯ *pdata = ದತ್ತಾಂಶ;

        ಗಾಗಿ (i = 0; i < type->ಗಾತ್ರ_2; i += ಗಾತ್ರ(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_ಸೂಚಕ, pdata);
          pdata += ಗಾತ್ರ(size_t);
        }
        ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (type->ಗಾತ್ರ_2 > 0)
          unmarshal_atom(cb, FFI_TYPE_ಸೂಚಕ, &realign[0]);
        ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)realign[0], type->ಗಾತ್ರ_2);
        ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
      }
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_INT(type->type) || type->type == FFI_TYPE_ಸೂಚಕ) {
    unmarshal_atom(cb, type->type, ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
  } ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (type->ಗಾತ್ರ_2 > 0)
        unmarshal_atom(cb, FFI_TYPE_ಸೂಚಕ, realign);
      ಆದರೆ (type->ಗಾತ್ರ_2 > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_ಸೂಚಕ, realign + 1);
      ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ, realign, type->ಗಾತ್ರ_2);
      ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
  }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ passed_by_ref(ffi_type *type, ಪೂರ್ಣಾಂಕ var) {
  ಆದರೆ (type->type == FFI_TYPE_STRUCT)
	  ಹಿಂತಿರುಗಿ 1;
  
  ಹಿಂತಿರುಗಿ type->ಗಾತ್ರ_2 > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_asm (ಶೂನ್ಯ *ರಾಶಿ, ರಚನೆ call_context *regs,
			  ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *ಆವರಣ) FFI_HIDDEN;

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue,
	      ಶೂನ್ಯ *ಆವರಣ)
{
  ಪೂರ್ಣಾಂಕ return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_ಬೈಟುಗಳು = FFI_ALIGN(2 * ಗಾತ್ರ(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_ಬೈಟುಗಳು = FFI_ALIGN(cif->ಬೈಟುಗಳು, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_ಗಾತ್ರ = arg_ಬೈಟುಗಳು + /*rval_bytes +*/ struct_ಬೈಟುಗಳು + ಗಾತ್ರ(call_context);
  size_t alloc_ತಳ = (size_t)alloca(alloc_ಗಾತ್ರ);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.ಬಳಸಿದ_integer = 0;
  cb.aregs = (call_context*)(alloc_ತಳ + arg_ಬೈಟುಗಳು /*+ rval_bytes*/ + struct_ಬೈಟುಗಳು);
  cb.ಬಳಸಿದ_ರಾಶಿ = (ಶೂನ್ಯ*)alloc_ತಳ;
  cb.struct_ರಾಶಿ = (ಶೂನ್ಯ *)(alloc_ತಳ + arg_ಬೈಟುಗಳು /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  ಆದರೆ (return_by_ref)
	  marshal(&cb, &ffi_type_ಸೂಚಕ, 0, &rvalue);

  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((ಶೂನ್ಯ *) alloc_ತಳ, cb.aregs, fn, ಆವರಣ);

  cb.ಬಳಸಿದ_integer = 0;
  ಆದರೆ (!return_by_ref && rvalue)
    {
	    ಆದರೆ (IS_INT(cif->rtype->type)
	        && cif->rtype->ಗಾತ್ರ_2 < ಗಾತ್ರ (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    ಆಯ್ಕೆ (cif->rtype->type) {
	      ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	      ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	      ಸ್ಥಿತಿ FFI_TYPE_SINT32:
		      unmarshal_atom (&cb, (ಗಾತ್ರ (ffi_arg) > 4
                          ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				                  rvalue);
		      ನಿಲ್ಲಿಸು;
	      ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	      ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	      ಸ್ಥಿತಿ FFI_TYPE_UINT32:
		      unmarshal_atom (&cb, (ಗಾತ್ರ (ffi_arg) > 4
				                  ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				                  rvalue);
		      ನಿಲ್ಲಿಸು;
	      }
	    }
	    ಇಲ್ಲದಿದ್ದರೆ
	      unmarshal(&cb, cif->rtype, 0, rvalue);
    }
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ಖಾಲಿ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int(cif, fn, rvalue, avalue, ಆವರಣ);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_asm(ಶೂನ್ಯ) FFI_HIDDEN;

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ * ಆವರಣ, ffi_cif * cif,
		      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಶೂನ್ಯ *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (ಆವರಣ->tramp[0]);

#ಆದರೆ defined(__ARC64_ARCH64__)
  size_t ವಿಳಾಸ_ffi_ಆವರಣ = (size_t) ffi_ಆವರಣ_asm;
#ಆದರೆ_ಅಂತ್ಯ

  ಆಯ್ಕೆ (cif->abi)
    {
#ಆದರೆ defined(__ARC64_ARCH64__)
    ಸ್ಥಿತಿ FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = ಸಂಕೇತ_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = ಸಂಕೇತ_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = ಸಂಕೇತ_ENDIAN ((uint32_t)(ವಿಳಾಸ_ffi_ಆವರಣ >> 32));
      tramp[3] = ಸಂಕೇತ_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = ಸಂಕೇತ_ENDIAN ((uint32_t)(ವಿಳಾಸ_ffi_ಆವರಣ & 0xffffffff));
      tramp[5] = ಸಂಕೇತ_ENDIAN (0x20200300);	/* j [r12] */
      ನಿಲ್ಲಿಸು;
#ಇಲ್ಲದಿದ್ದರೆ
    ಸ್ಥಿತಿ FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = ಸಂಕೇತ_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = ಸಂಕೇತ_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = ಸಂಕೇತ_ENDIAN ((uint32_t) ffi_ಆವರಣ_asm);
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
    }

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
  cacheflush (codeloc, FFI_TRAMPOLINE_ಗಾತ್ರ, BCACHE);

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_asm (ಶೂನ್ಯ) FFI_HIDDEN;

ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ *ಆವರಣ, ffi_cif *cif,
		     ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *))
{
  ಆದರೆ (cif->abi <= FFI_ಮೊದಲ_ABI || cif->abi >= FFI_ಕೊನೆಯ_ABI)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಆವರಣ->tramp = (ಶೂನ್ಯ *) ffi_go_ಆವರಣ_asm;
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
ಶೂನ್ಯ FFI_HIDDEN
ffi_ಆವರಣ_inner (ffi_cif *cif,
		   ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		   ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		   size_t *ರಾಶಿ, call_context *aregs)
{
    ಶೂನ್ಯ **avalue = alloca(cif->nargs * ಗಾತ್ರ(ಶೂನ್ಯ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    ಅಕ್ಷರ *astorage = alloca(cif->ಬೈಟುಗಳು);
    ಅಕ್ಷರ *ptr = astorage;
    ಶೂನ್ಯ *rvalue;
    call_builder cb;
    ಪೂರ್ಣಾಂಕ i;

    cb.aregs = aregs;
    cb.ಬಳಸಿದ_integer = 0;
    cb.ಬಳಸಿದ_ರಾಶಿ = ರಾಶಿ;
    
    /* handle hidden argument */
    ಆದರೆ (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_ಸೂಚಕ, 0, &rvalue);
    ಇಲ್ಲದಿದ್ದರೆ
      rvalue = alloca(cif->rtype->ಗಾತ್ರ_2);

    ಗಾಗಿ (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->ಗಾತ್ರ_2;
    }

    fun (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

    ಆದರೆ (cif->rtype->type != FFI_TYPE_VOID) {
        cb.ಬಳಸಿದ_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
