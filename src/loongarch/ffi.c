/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2022 Xu Chenghua <xuchenghua@loongson.cn>
                         2022 Cheng Lulu <chenglulu@loongson.cn>
   Based on RISC-V port

   LoongArch Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>

#ಯದಿ defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#ನೋಚೇದ್ಯದಿ defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT ಭಿನ್ನ
#ನೋಚೇದ್ಯದಿ defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT ದ್ವಿಗುಣ
#ಅನ್ಯಥಾ
#error unsupported LoongArch floating-point ABI
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * ಪರಿಮಾಣ (ದ್ವಿಗುಣ))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ call_context
{
#ಯದಿ !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#endif
  size_t a[10];
} call_context;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ call_builder
{
  call_context *aregs;
  ಪೂರ್ಣಾಙ್ಕ used_integer;
  ಪೂರ್ಣಾಙ್ಕ used_float;
  size_t *used_ರಾಶಿ;	    /* Stack pointer.  */
  size_t *ರಾಶಿ;	    /* Place for arguments pass by reference.  */
  size_t next_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#ಯದಿ __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ಅನ್ಯಥಾ
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#ಯದಿ ABI_FRLEN
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ float_struct_info
{
  ವರ್ಣ as_elements;
  ವರ್ಣ type1;
  ವರ್ಣ offset2;
  ವರ್ಣ type2;
} float_struct_info;

#ಯದಿ ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#ಅನ್ಯಥಾ
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

ಸ್ಥಿರ ffi_type **
flatten_struct (ffi_type *in, ffi_type **out, ffi_type **out_end)
{
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ (out == out_end)
    ಪ್ರತ್ಯಾಗಮ out;
  ಯದಿ (in->type != FFI_TYPE_STRUCT)
    *(out++) = in;
  ಅನ್ಯಥಾ
    ಪ್ರತಿ (i = 0; in->elements[i]; i++)
      out = flatten_struct (in->elements[i], out, out_end);
  ಪ್ರತ್ಯಾಗಮ out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
ಸ್ಥಿರ float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *ತುದಿ)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_floats, ಅಂಕೆ_ints;
  ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_fields = flatten_struct (ತುದಿ, fields, fields + 3) - fields;

  ಯದಿ (ಅಂಕೆ_fields == 1)
    {
      ಯದಿ (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  ಅನ್ಯಥಾ ಯದಿ (ಅಂಕೆ_fields == 2)
    {
      ಅಂಕೆ_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      ಅಂಕೆ_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      ಯದಿ (ಅಂಕೆ_floats == 0 || ಅಂಕೆ_floats + ಅಂಕೆ_ints != 2)
	ಪ್ರತ್ಯಾಗಮ ret;
      ಯದಿ (cb->used_float + ಅಂಕೆ_floats > NARGREG
	  || cb->used_integer + (2 - ಅಂಕೆ_floats) > NARGREG)
	ಪ್ರತ್ಯಾಗಮ ret;
      ಯದಿ (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	ಪ್ರತ್ಯಾಗಮ ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->size, fields[1]->alignment);
      ret.as_elements = 1;
    }
  ಪ್ರತ್ಯಾಗಮ ret;
}
#endif

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
ಸ್ಥಿರ ಶೂನ್ಯ
marshal_atom (call_builder *cb, ಪೂರ್ಣಾಙ್ಕ type, ಶೂನ್ಯ *data)
{
  size_t ಮೌಲ್ಯ = 0;
  ವಿಕಲ್ಪ (type)
    {
    ಪಕ್ಷ FFI_TYPE_UINT8:
      ಮೌಲ್ಯ = *(uint8_t *) data;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT8:
      ಮೌಲ್ಯ = *(int8_t *) data;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      ಮೌಲ್ಯ = *(uint16_t *) data;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      ಮೌಲ್ಯ = *(int16_t *) data;
      ಭಙ್ಗ;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    ಪಕ್ಷ FFI_TYPE_UINT32:
      ಮೌಲ್ಯ = *(int32_t *) data;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT32:
      ಮೌಲ್ಯ = *(int32_t *) data;
      ಭಙ್ಗ;
#ಯದಿ __SIZEOF_POINTER__ == 8
    ಪಕ್ಷ FFI_TYPE_UINT64:
      ಮೌಲ್ಯ = *(uint64_t *) data;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT64:
      ಮೌಲ್ಯ = *(int64_t *) data;
      ಭಙ್ಗ;
#endif
    ಪಕ್ಷ FFI_TYPE_POINTER:
      ಮೌಲ್ಯ = *(size_t *) data;
      ಭಙ್ಗ;

#ಯದಿ ABI_FRLEN >= 32
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      *(ಭಿನ್ನ *)(cb->aregs->fa + cb->used_float++) = *(ಭಿನ್ನ *) data;
      ಪ್ರತ್ಯಾಗಮ;
#endif
#ಯದಿ ABI_FRLEN >= 64
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->used_float++]) = *(ದ್ವಿಗುಣ *) data;
      ಪ್ರತ್ಯಾಗಮ;
#endif
    ಸ್ವಭಾವ:
      FFI_ASSERT (0);
      ಭಙ್ಗ;
    }

  ಯದಿ (cb->used_integer == NARGREG)
    *cb->used_ರಾಶಿ++ = ಮೌಲ್ಯ;
  ಅನ್ಯಥಾ
    cb->aregs->a[cb->used_integer++] = ಮೌಲ್ಯ;
}

ಸ್ಥಿರ ಶೂನ್ಯ
unmarshal_atom (call_builder *cb, ಪೂರ್ಣಾಙ್ಕ type, ಶೂನ್ಯ *data)
{
  size_t ಮೌಲ್ಯ;
  ವಿಕಲ್ಪ (type)
    {
#ಯದಿ ABI_FRLEN >= 32
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      *(ಭಿನ್ನ *) data = *(ಭಿನ್ನ *)(cb->aregs->fa + cb->used_float++);
      ಪ್ರತ್ಯಾಗಮ;
#endif
#ಯದಿ ABI_FRLEN >= 64
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      *(ದ್ವಿಗುಣ *) data = cb->aregs->fa[cb->used_float++];
      ಪ್ರತ್ಯಾಗಮ;
#endif
    }

  ಯದಿ (cb->used_integer == NARGREG)
    ಮೌಲ್ಯ = *cb->used_ರಾಶಿ++;
  ಅನ್ಯಥಾ
    ಮೌಲ್ಯ = cb->aregs->a[cb->used_integer++];

  ವಿಕಲ್ಪ (type)
    {
    ಪಕ್ಷ FFI_TYPE_UINT8:
    ಪಕ್ಷ FFI_TYPE_SINT8:
    ಪಕ್ಷ FFI_TYPE_UINT16:
    ಪಕ್ಷ FFI_TYPE_SINT16:
    ಪಕ್ಷ FFI_TYPE_UINT32:
    ಪಕ್ಷ FFI_TYPE_SINT32:
#ಯದಿ __SIZEOF_POINTER__ == 8
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_SINT64:
#endif
    ಪಕ್ಷ FFI_TYPE_POINTER:
      *(ffi_arg *)data = ಮೌಲ್ಯ;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      FFI_ASSERT (0);
      ಭಙ್ಗ;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ಸ್ಥಿರ ಶೂನ್ಯ *
allocate_and_copy_struct_to_ರಾಶಿ (call_builder *cb, ಶೂನ್ಯ *data,
				   ffi_type *type)
{
  size_t dest = cb->next_struct_area - type->size;

  dest = FFI_ALIGN_DOWN (dest, type->alignment);
  cb->next_struct_area = dest;

  ಪ್ರತ್ಯಾಗಮ memcpy ((ವರ್ಣ *)cb->ರಾಶಿ + dest, data, type->size);
}

/* Adds an argument to a call, or a not by reference return value.  */
ಸ್ಥಿರ ಶೂನ್ಯ
marshal (call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಙ್ಕ var, ಶೂನ್ಯ *data)
{
  size_t realign[2];

#ಯದಿ ABI_FRLEN
  ಯದಿ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ಯದಿ (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, data);
	  ಯದಿ (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((ವರ್ಣ *) data) + fsi.offset2);
	  ಪ್ರತ್ಯಾಗಮ;
	}
    }

  ಯದಿ (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, data);
      ಪ್ರತ್ಯಾಗಮ;
    }

  ದ್ವಿಗುಣ promoted;
  ಯದಿ (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(ಭಿನ್ನ *) data;
      type = &ffi_type_double;
      data = &promoted;
    }
#endif

  ಯದಿ (type->size > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_to_ರಾಶಿ (cb, data, type);
      data = (ವರ್ಣ *)cb->ರಾಶಿ + cb->next_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &data);
    }
  ಅನ್ಯಥಾ ಯದಿ (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, data);
  ಅನ್ಯಥಾ
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      ಯದಿ (type->alignment > __SIZEOF_POINTER__)
	{
	  ಯದಿ (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_ರಾಶಿ
	    = (size_t *) FFI_ALIGN (cb->used_ರಾಶಿ, 2 * __SIZEOF_POINTER__);
	}

      memcpy (realign, data, type->size);
      ಯದಿ (type->size > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      ಯದಿ (type->size > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
ಸ್ಥಿರ ಶೂನ್ಯ *
unmarshal (call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಙ್ಕ var, ಶೂನ್ಯ *data)
{
  size_t realign[2];
  ಶೂನ್ಯ *pointer;

#ಯದಿ ABI_FRLEN
  ಯದಿ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ಯದಿ (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, data);
	  ಯದಿ (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((ವರ್ಣ *) data) + fsi.offset2);
	  ಪ್ರತ್ಯಾಗಮ data;
	}
    }

  ಯದಿ (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, data);
      ಪ್ರತ್ಯಾಗಮ data;
    }

  ಯದಿ (var && type->type == FFI_TYPE_FLOAT)
    {
      ಪೂರ್ಣಾಙ್ಕ m = cb->used_integer;
      ಶೂನ್ಯ *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->used_ರಾಶಿ + m - NARGREG + 1;
      *(ಭಿನ್ನ *) promoted = *(ದ್ವಿಗುಣ *) promoted;
    }
#endif

  ಯದಿ (type->size > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (ವರ್ಣ *) &pointer);
      ಪ್ರತ್ಯಾಗಮ pointer;
    }
  ಅನ್ಯಥಾ ಯದಿ (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, data);
      ಪ್ರತ್ಯಾಗಮ data;
    }
  ಅನ್ಯಥಾ
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      ಯದಿ (type->alignment > __SIZEOF_POINTER__)
	{
	  ಯದಿ (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_ರಾಶಿ
	    = (size_t *) FFI_ALIGN (cb->used_ರಾಶಿ, 2 * __SIZEOF_POINTER__);
	}

      ಯದಿ (type->size > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      ಯದಿ (type->size > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      memcpy (data, realign, type->size);
      ಪ್ರತ್ಯಾಗಮ data;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
passed_by_ref (call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಙ್ಕ var)
{
#ಯದಿ ABI_FRLEN
  ಯದಿ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ಯದಿ (fsi.as_elements)
	ಪ್ರತ್ಯಾಗಮ 0;
    }
#endif

  ಪ್ರತ್ಯಾಗಮ type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_status
ffi_prep_cif_machdep_var (ffi_cif *cif, unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs,
			  unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Low level routine for calling functions.  */
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_asm (ಶೂನ್ಯ *ರಾಶಿ, ಸಂರಚನಾ call_context *regs,
			  ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *closure) FFI_HIDDEN;

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue,
	      ಶೂನ್ಯ *closure)
{
  size_t arg_bytes = FFI_ALIGN (cif->bytes, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_bytes = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * ಪರಿಮಾಣ(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_bytes = 0;
  ಯದಿ (rvalue == ರಿಕ್ತ && cif->rtype->size > 2 * __SIZEOF_POINTER__)
    rval_bytes = FFI_ALIGN (cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + extra_bytes + rval_bytes + ಪರಿಮಾಣ (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_base;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  ಯದಿ (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_base = (size_t) alloca (alloc_size);
  ಅನ್ಯಥಾ
    alloc_base = FFI_ALIGN (alloca (alloc_size + STKALIGN - 1), STKALIGN);

  ಯದಿ (rval_bytes)
    rvalue = (ಶೂನ್ಯ *) (alloc_base + arg_bytes);

  call_builder cb;
  cb.used_float = cb.used_integer = 0;
  cb.aregs = (call_context *) (alloc_base + arg_bytes + extra_bytes + rval_bytes);
  cb.used_ರಾಶಿ = (size_t *)((ವರ್ಣ *) alloc_base);
  cb.ರಾಶಿ = (size_t *)((ವರ್ಣ *) alloc_base + extra_bytes + rval_bytes);
  cb.next_struct_area = arg_bytes;

  ಪೂರ್ಣಾಙ್ಕ return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  ಯದಿ (return_by_ref)
    cb.aregs->a[cb.used_integer++] = (size_t)rvalue;

  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((ಶೂನ್ಯ *) alloc_base, cb.aregs, fn, closure);

  cb.used_float = cb.used_integer = 0;
  ಯದಿ (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ರಿಕ್ತ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue,
	     ಶೂನ್ಯ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_asm (ಶೂನ್ಯ) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_data, ಶೂನ್ಯ *codeloc)
{
  uint32_t *tramp = (uint32_t *) &closure->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

  ಯದಿ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
  ಯದಿ (ffi_tramp_is_present(closure))
    {
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_asm, closure);
      ಗಚ್ಛ out;
    }
#endif

  /* Fill the dynamic trampoline.  We will call ffi_closure_inner with codeloc,
     not closure, but as long as the memory is readable it should work.  */
  tramp[0] = 0x1800000c; /* pcaddi $t0, 0 (i.e. $t0 <- tramp) */
  tramp[1] = 0x28c0418d; /* ld.d   $t1, $t0, 16 */
  tramp[2] = 0x4c0001a0; /* jirl   $zero, $t1, 0 */
  tramp[3] = 0x03400000; /* nop */
  tramp[4] = fn;
  tramp[5] = fn >> 32;

  __builtin___clear_cache (codeloc, codeloc + FFI_TRAMPOLINE_SIZE);

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_asm (ಶೂನ್ಯ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *))
{
  ಯದಿ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  closure->tramp = (ಶೂನ್ಯ *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
ಶೂನ್ಯ FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		   ಶೂನ್ಯ *user_data, size_t *ರಾಶಿ, call_context *aregs)
{
  ಶೂನ್ಯ **avalue = alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  ವರ್ಣ *astorage = alloca (cif->nargs * MAXCOPYARG);
  ಶೂನ್ಯ *rvalue;
  call_builder cb;
  ಪೂರ್ಣಾಙ್ಕ return_by_ref;
  ಪೂರ್ಣಾಙ್ಕ i;

  cb.aregs = aregs;
  cb.used_integer = cb.used_float = 0;
  cb.used_ರಾಶಿ = ರಾಶಿ;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  ಯದಿ (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  ಅನ್ಯಥಾ
    rvalue = alloca (cif->rtype->size);

  ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_data);

  ಯದಿ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.used_integer = cb.used_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_code_table;

  *tramp_size = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_size = 1 << 16;
  ಪ್ರತ್ಯಾಗಮ &trampoline_code_table;
}
#endif
