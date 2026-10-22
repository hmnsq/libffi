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

#ಆದರೆ defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#ಅಥವಾಆದರೆ defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT ದಶಮಾಂಶ
#ಅಥವಾಆದರೆ defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT ದ್ವಿಗುಣ
#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ unsupported LoongArch floating-ಬಿಂದು ABI
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು NARGREG 8
#ವ್ಯಾಖ್ಯಾನಿಸು STKALIGN 16
#ವ್ಯಾಖ್ಯಾನಿಸು MAXCOPYARG (2 * ಗಾತ್ರ (ದ್ವಿಗುಣ))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ call_context
{
#ಆದರೆ !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#ಆದರೆ_ಅಂತ್ಯ
  size_t a[10];
} call_context;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ call_builder
{
  call_context *aregs;
  ಪೂರ್ಣಾಂಕ ಬಳಸಿದ_integer;
  ಪೂರ್ಣಾಂಕ ಬಳಸಿದ_float;
  size_t *ಬಳಸಿದ_ರಾಶಿ;	    /* Stack pointer.  */
  size_t *ರಾಶಿ;	    /* Place for arguments pass by reference.  */
  size_t ಮುಂದಿನ_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#ಆದರೆ __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ಇಲ್ಲದಿದ್ದರೆ
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ABI_FRLEN
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ float_struct_info
{
  ಅಕ್ಷರ as_elements;
  ಅಕ್ಷರ type1;
  ಅಕ್ಷರ offset2;
  ಅಕ್ಷರ type2;
} float_struct_info;

#ಆದರೆ ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#ಇಲ್ಲದಿದ್ದರೆ
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ffi_type **
flatten_struct (ffi_type *ಒಳ, ffi_type **ಹೊರಹೋಗು, ffi_type **ಹೊರಹೋಗು_ಕೊನೆ)
{
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ (ಹೊರಹೋಗು == ಹೊರಹೋಗು_ಕೊನೆ)
    ಹಿಂತಿರುಗಿ ಹೊರಹೋಗು;
  ಆದರೆ (ಒಳ->type != FFI_TYPE_STRUCT)
    *(ಹೊರಹೋಗು++) = ಒಳ;
  ಇಲ್ಲದಿದ್ದರೆ
    ಗಾಗಿ (i = 0; ಒಳ->elements[i]; i++)
      ಹೊರಹೋಗು = flatten_struct (ಒಳ->elements[i], ಹೊರಹೋಗು, ಹೊರಹೋಗು_ಕೊನೆ);
  ಹಿಂತಿರುಗಿ ಹೊರಹೋಗು;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
ಸ್ಥಿರ float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *ತುದಿ)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  ಪೂರ್ಣಾಂಕ ಅಂಕೆ_floats, ಅಂಕೆ_ints;
  ಪೂರ್ಣಾಂಕ ಅಂಕೆ_fields = flatten_struct (ತುದಿ, fields, fields + 3) - fields;

  ಆದರೆ (ಅಂಕೆ_fields == 1)
    {
      ಆದರೆ (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಂಕೆ_fields == 2)
    {
      ಅಂಕೆ_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      ಅಂಕೆ_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      ಆದರೆ (ಅಂಕೆ_floats == 0 || ಅಂಕೆ_floats + ಅಂಕೆ_ints != 2)
	ಹಿಂತಿರುಗಿ ret;
      ಆದರೆ (cb->ಬಳಸಿದ_float + ಅಂಕೆ_floats > NARGREG
	  || cb->ಬಳಸಿದ_integer + (2 - ಅಂಕೆ_floats) > NARGREG)
	ಹಿಂತಿರುಗಿ ret;
      ಆದರೆ (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	ಹಿಂತಿರುಗಿ ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->ಗಾತ್ರ_2, fields[1]->alignment);
      ret.as_elements = 1;
    }
  ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
ಸ್ಥಿರ ಶೂನ್ಯ
marshal_atom (call_builder *cb, ಪೂರ್ಣಾಂಕ type, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
  size_t ಮೌಲ್ಯ = 0;
  ಆಯ್ಕೆ (type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      ಮೌಲ್ಯ = *(uint8_t *) ದತ್ತಾಂಶ;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      ಮೌಲ್ಯ = *(int8_t *) ದತ್ತಾಂಶ;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ಮೌಲ್ಯ = *(uint16_t *) ದತ್ತಾಂಶ;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      ಮೌಲ್ಯ = *(int16_t *) ದತ್ತಾಂಶ;
      ನಿಲ್ಲಿಸು;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ಮೌಲ್ಯ = *(int32_t *) ದತ್ತಾಂಶ;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      ಮೌಲ್ಯ = *(int32_t *) ದತ್ತಾಂಶ;
      ನಿಲ್ಲಿಸು;
#ಆದರೆ __SIZEOF_POINTER__ == 8
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      ಮೌಲ್ಯ = *(uint64_t *) ದತ್ತಾಂಶ;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      ಮೌಲ್ಯ = *(int64_t *) ದತ್ತಾಂಶ;
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      ಮೌಲ್ಯ = *(size_t *) ದತ್ತಾಂಶ;
      ನಿಲ್ಲಿಸು;

#ಆದರೆ ABI_FRLEN >= 32
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      *(ದಶಮಾಂಶ *)(cb->aregs->fa + cb->ಬಳಸಿದ_float++) = *(ದಶಮಾಂಶ *) ದತ್ತಾಂಶ;
      ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ABI_FRLEN >= 64
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->ಬಳಸಿದ_float++]) = *(ದ್ವಿಗುಣ *) ದತ್ತಾಂಶ;
      ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT (0);
      ನಿಲ್ಲಿಸು;
    }

  ಆದರೆ (cb->ಬಳಸಿದ_integer == NARGREG)
    *cb->ಬಳಸಿದ_ರಾಶಿ++ = ಮೌಲ್ಯ;
  ಇಲ್ಲದಿದ್ದರೆ
    cb->aregs->a[cb->ಬಳಸಿದ_integer++] = ಮೌಲ್ಯ;
}

ಸ್ಥಿರ ಶೂನ್ಯ
unmarshal_atom (call_builder *cb, ಪೂರ್ಣಾಂಕ type, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
  size_t ಮೌಲ್ಯ;
  ಆಯ್ಕೆ (type)
    {
#ಆದರೆ ABI_FRLEN >= 32
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      *(ದಶಮಾಂಶ *) ದತ್ತಾಂಶ = *(ದಶಮಾಂಶ *)(cb->aregs->fa + cb->ಬಳಸಿದ_float++);
      ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ABI_FRLEN >= 64
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      *(ದ್ವಿಗುಣ *) ದತ್ತಾಂಶ = cb->aregs->fa[cb->ಬಳಸಿದ_float++];
      ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
    }

  ಆದರೆ (cb->ಬಳಸಿದ_integer == NARGREG)
    ಮೌಲ್ಯ = *cb->ಬಳಸಿದ_ರಾಶಿ++;
  ಇಲ್ಲದಿದ್ದರೆ
    ಮೌಲ್ಯ = cb->aregs->a[cb->ಬಳಸಿದ_integer++];

  ಆಯ್ಕೆ (type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
#ಆದರೆ __SIZEOF_POINTER__ == 8
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      *(ffi_arg *)ದತ್ತಾಂಶ = ಮೌಲ್ಯ;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT (0);
      ನಿಲ್ಲಿಸು;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ಸ್ಥಿರ ಶೂನ್ಯ *
allocate_and_copy_struct_ಗಮ್ಯ_ರಾಶಿ (call_builder *cb, ಶೂನ್ಯ *ದತ್ತಾಂಶ,
				   ffi_type *type)
{
  size_t dest = cb->ಮುಂದಿನ_struct_area - type->ಗಾತ್ರ_2;

  dest = FFI_ALIGN_ಕೆಳಕ್ಕೆ (dest, type->alignment);
  cb->ಮುಂದಿನ_struct_area = dest;

  ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ನಕಲು ((ಅಕ್ಷರ *)cb->ರಾಶಿ + dest, ದತ್ತಾಂಶ, type->ಗಾತ್ರ_2);
}

/* Adds an argument to a call, or a not by reference return value.  */
ಸ್ಥಿರ ಶೂನ್ಯ
marshal (call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಂಕ var, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
  size_t realign[2];

#ಆದರೆ ABI_FRLEN
  ಆದರೆ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ಆದರೆ (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, ದತ್ತಾಂಶ);
	  ಆದರೆ (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((ಅಕ್ಷರ *) ದತ್ತಾಂಶ) + fsi.offset2);
	  ಹಿಂತಿರುಗಿ;
	}
    }

  ಆದರೆ (!var && cb->ಬಳಸಿದ_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, ದತ್ತಾಂಶ);
      ಹಿಂತಿರುಗಿ;
    }

  ದ್ವಿಗುಣ promoted;
  ಆದರೆ (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(ದಶಮಾಂಶ *) ದತ್ತಾಂಶ;
      type = &ffi_type_double;
      ದತ್ತಾಂಶ = &promoted;
    }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (type->ಗಾತ್ರ_2 > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_ಗಮ್ಯ_ರಾಶಿ (cb, ದತ್ತಾಂಶ, type);
      ದತ್ತಾಂಶ = (ಅಕ್ಷರ *)cb->ರಾಶಿ + cb->ಮುಂದಿನ_struct_area;
      marshal_atom (cb, FFI_TYPE_ಸೂಚಕ, &ದತ್ತಾಂಶ);
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_INT (type->type) || type->type == FFI_TYPE_ಸೂಚಕ)
    marshal_atom (cb, type->type, ದತ್ತಾಂಶ);
  ಇಲ್ಲದಿದ್ದರೆ
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      ಆದರೆ (type->alignment > __SIZEOF_POINTER__)
	{
	  ಆದರೆ (var)
	    cb->ಬಳಸಿದ_integer = FFI_ALIGN (cb->ಬಳಸಿದ_integer, 2);
	  cb->ಬಳಸಿದ_ರಾಶಿ
	    = (size_t *) FFI_ALIGN (cb->ಬಳಸಿದ_ರಾಶಿ, 2 * __SIZEOF_POINTER__);
	}

      ಸ್ಮರಣೆ_ನಕಲು (realign, ದತ್ತಾಂಶ, type->ಗಾತ್ರ_2);
      ಆದರೆ (type->ಗಾತ್ರ_2 > 0)
	marshal_atom (cb, FFI_TYPE_ಸೂಚಕ, realign);
      ಆದರೆ (type->ಗಾತ್ರ_2 > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_ಸೂಚಕ, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
ಸ್ಥಿರ ಶೂನ್ಯ *
unmarshal (call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಂಕ var, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
  size_t realign[2];
  ಶೂನ್ಯ *ಸೂಚಕ_2;

#ಆದರೆ ABI_FRLEN
  ಆದರೆ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ಆದರೆ (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, ದತ್ತಾಂಶ);
	  ಆದರೆ (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((ಅಕ್ಷರ *) ದತ್ತಾಂಶ) + fsi.offset2);
	  ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
	}
    }

  ಆದರೆ (!var && cb->ಬಳಸಿದ_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, ದತ್ತಾಂಶ);
      ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
    }

  ಆದರೆ (var && type->type == FFI_TYPE_FLOAT)
    {
      ಪೂರ್ಣಾಂಕ m = cb->ಬಳಸಿದ_integer;
      ಶೂನ್ಯ *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->ಬಳಸಿದ_ರಾಶಿ + m - NARGREG + 1;
      *(ದಶಮಾಂಶ *) promoted = *(ದ್ವಿಗುಣ *) promoted;
    }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (type->ಗಾತ್ರ_2 > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_ಸೂಚಕ, (ಅಕ್ಷರ *) &ಸೂಚಕ_2);
      ಹಿಂತಿರುಗಿ ಸೂಚಕ_2;
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_INT (type->type) || type->type == FFI_TYPE_ಸೂಚಕ)
    {
      unmarshal_atom (cb, type->type, ದತ್ತಾಂಶ);
      ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      ಆದರೆ (type->alignment > __SIZEOF_POINTER__)
	{
	  ಆದರೆ (var)
	    cb->ಬಳಸಿದ_integer = FFI_ALIGN (cb->ಬಳಸಿದ_integer, 2);
	  cb->ಬಳಸಿದ_ರಾಶಿ
	    = (size_t *) FFI_ALIGN (cb->ಬಳಸಿದ_ರಾಶಿ, 2 * __SIZEOF_POINTER__);
	}

      ಆದರೆ (type->ಗಾತ್ರ_2 > 0)
	unmarshal_atom (cb, FFI_TYPE_ಸೂಚಕ, realign);
      ಆದರೆ (type->ಗಾತ್ರ_2 > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_ಸೂಚಕ, realign + 1);
      ಸ್ಮರಣೆ_ನಕಲು (ದತ್ತಾಂಶ, realign, type->ಗಾತ್ರ_2);
      ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
passed_by_ref (call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಂಕ var)
{
#ಆದರೆ ABI_FRLEN
  ಆದರೆ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ಆದರೆ (fsi.as_elements)
	ಹಿಂತಿರುಗಿ 0;
    }
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ type->ಗಾತ್ರ_2 > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep_var (ffi_cif *cif, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs,
			  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* Low level routine for calling functions.  */
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_asm (ಶೂನ್ಯ *ರಾಶಿ, ರಚನೆ call_context *regs,
			  ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *ಆವರಣ) FFI_HIDDEN;

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue,
	      ಶೂನ್ಯ *ಆವರಣ)
{
  size_t arg_ಬೈಟುಗಳು = FFI_ALIGN (cif->ಬೈಟುಗಳು, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_ಬೈಟುಗಳು = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * ಗಾತ್ರ(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_ಬೈಟುಗಳು = 0;
  ಆದರೆ (rvalue == ಖಾಲಿ && cif->rtype->ಗಾತ್ರ_2 > 2 * __SIZEOF_POINTER__)
    rval_ಬೈಟುಗಳು = FFI_ALIGN (cif->rtype->ಗಾತ್ರ_2, STKALIGN);
  size_t alloc_ಗಾತ್ರ = arg_ಬೈಟುಗಳು + extra_ಬೈಟುಗಳು + rval_ಬೈಟುಗಳು + ಗಾತ್ರ (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_ತಳ;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  ಆದರೆ (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_ತಳ = (size_t) alloca (alloc_ಗಾತ್ರ);
  ಇಲ್ಲದಿದ್ದರೆ
    alloc_ತಳ = FFI_ALIGN (alloca (alloc_ಗಾತ್ರ + STKALIGN - 1), STKALIGN);

  ಆದರೆ (rval_ಬೈಟುಗಳು)
    rvalue = (ಶೂನ್ಯ *) (alloc_ತಳ + arg_ಬೈಟುಗಳು);

  call_builder cb;
  cb.ಬಳಸಿದ_float = cb.ಬಳಸಿದ_integer = 0;
  cb.aregs = (call_context *) (alloc_ತಳ + arg_ಬೈಟುಗಳು + extra_ಬೈಟುಗಳು + rval_ಬೈಟುಗಳು);
  cb.ಬಳಸಿದ_ರಾಶಿ = (size_t *)((ಅಕ್ಷರ *) alloc_ತಳ);
  cb.ರಾಶಿ = (size_t *)((ಅಕ್ಷರ *) alloc_ತಳ + extra_ಬೈಟುಗಳು + rval_ಬೈಟುಗಳು);
  cb.ಮುಂದಿನ_struct_area = arg_ಬೈಟುಗಳು;

  ಪೂರ್ಣಾಂಕ return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  ಆದರೆ (return_by_ref)
    cb.aregs->a[cb.ಬಳಸಿದ_integer++] = (size_t)rvalue;

  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((ಶೂನ್ಯ *) alloc_ತಳ, cb.aregs, fn, ಆವರಣ);

  cb.ಬಳಸಿದ_float = cb.ಬಳಸಿದ_integer = 0;
  ಆದರೆ (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಖಾಲಿ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue,
	     ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಆವರಣ);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_asm (ಶೂನ್ಯ) FFI_HIDDEN;

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ *ಆವರಣ, ffi_cif *cif,
		      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಶೂನ್ಯ *codeloc)
{
  uint32_t *tramp = (uint32_t *) &ಆವರಣ->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_ಆವರಣ_asm;

  ಆದರೆ (cif->abi <= FFI_ಮೊದಲ_ABI || cif->abi >= FFI_ಕೊನೆಯ_ABI)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
  ಆದರೆ (ffi_tramp_is_present(ಆವರಣ))
    {
      ffi_tramp_ಗಣ_parms (ಆವರಣ->ftramp, ffi_ಆವರಣ_asm, ಆವರಣ);
      ಹೋಗು ಹೊರಹೋಗು;
    }
#ಆದರೆ_ಅಂತ್ಯ

  /* Fill the dynamic trampoline.  We will call ffi_closure_inner with codeloc,
     not closure, but as long as the memory is readable it should work.  */
  tramp[0] = 0x1800000c; /* pcaddi $t0, 0 (i.e. $t0 <- tramp) */
  tramp[1] = 0x28c0418d; /* ld.d   $t1, $t0, 16 */
  tramp[2] = 0x4c0001a0; /* jirl   $zero, $t1, 0 */
  tramp[3] = 0x03400000; /* nop */
  tramp[4] = fn;
  tramp[5] = fn >> 32;

  __builtin___clear_cache (codeloc, codeloc + FFI_TRAMPOLINE_ಗಾತ್ರ);

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಹೊರಹೋಗು:
#ಆದರೆ_ಅಂತ್ಯ
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

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
   registers will be reloaded from aregs.  */
ಶೂನ್ಯ FFI_HIDDEN
ffi_ಆವರಣ_inner (ffi_cif *cif,
		   ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		   ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, size_t *ರಾಶಿ, call_context *aregs)
{
  ಶೂನ್ಯ **avalue = alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  ಅಕ್ಷರ *astorage = alloca (cif->nargs * MAXCOPYARG);
  ಶೂನ್ಯ *rvalue;
  call_builder cb;
  ಪೂರ್ಣಾಂಕ return_by_ref;
  ಪೂರ್ಣಾಂಕ i;

  cb.aregs = aregs;
  cb.ಬಳಸಿದ_integer = cb.ಬಳಸಿದ_float = 0;
  cb.ಬಳಸಿದ_ರಾಶಿ = ರಾಶಿ;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  ಆದರೆ (return_by_ref)
    unmarshal (&cb, &ffi_type_ಸೂಚಕ, 0, &rvalue);
  ಇಲ್ಲದಿದ್ದರೆ
    rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);

  ಗಾಗಿ (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

  ಆದರೆ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.ಬಳಸಿದ_integer = cb.ಬಳಸಿದ_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_ಗಾತ್ರ, size_t *map_ಗಾತ್ರ)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;

  *tramp_ಗಾತ್ರ = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_ಗಾತ್ರ = 1 << 16;
  ಹಿಂತಿರುಗಿ &trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;
}
#ಆದರೆ_ಅಂತ್ಯ
