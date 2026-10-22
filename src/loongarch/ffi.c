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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>

#ਜੇ defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#ਜਾਂਜੇ defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT ਦਸ਼ਮਲਵ
#ਜਾਂਜੇ defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT ਦੁੱਗਣਾ
#ਨਹੀਂਤਾਂ
#ਗਲਤੀ unsupported LoongArch floating-ਬਿੰਦੂ ABI
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NARGREG 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STKALIGN 16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXCOPYARG (2 * ਆਕਾਰ (ਦੁੱਗਣਾ))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ call_context
{
#ਜੇ !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#ਜੇ_ਅੰਤ
  size_t a[10];
} call_context;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ call_builder
{
  call_context *aregs;
  ਪੂਰਨ ਵਰਤਿਆ_integer;
  ਪੂਰਨ ਵਰਤਿਆ_float;
  size_t *ਵਰਤਿਆ_ਢੇਰ;	    /* Stack pointer.  */
  size_t *ਢੇਰ;	    /* Place for arguments pass by reference.  */
  size_t ਅਗਲਾ_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#ਜੇ __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ਨਹੀਂਤਾਂ
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#ਜੇ_ਅੰਤ

#ਜੇ ABI_FRLEN
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ float_struct_info
{
  ਅੱਖਰ as_elements;
  ਅੱਖਰ type1;
  ਅੱਖਰ offset2;
  ਅੱਖਰ type2;
} float_struct_info;

#ਜੇ ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#ਨਹੀਂਤਾਂ
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#ਜੇ_ਅੰਤ

ਸਥਿਰ ffi_type **
flatten_struct (ffi_type *ਵਿੱਚ_2, ffi_type **ਬਾਹਰਲਾ, ffi_type **ਬਾਹਰਲਾ_ਸਿਰਾ)
{
  ਪੂਰਨ i;

  ਜੇ (ਬਾਹਰਲਾ == ਬਾਹਰਲਾ_ਸਿਰਾ)
    ਵਾਪਸ ਬਾਹਰਲਾ;
  ਜੇ (ਵਿੱਚ_2->type != FFI_TYPE_STRUCT)
    *(ਬਾਹਰਲਾ++) = ਵਿੱਚ_2;
  ਨਹੀਂਤਾਂ
    ਲਈ (i = 0; ਵਿੱਚ_2->elements[i]; i++)
      ਬਾਹਰਲਾ = flatten_struct (ਵਿੱਚ_2->elements[i], ਬਾਹਰਲਾ, ਬਾਹਰਲਾ_ਸਿਰਾ);
  ਵਾਪਸ ਬਾਹਰਲਾ;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
ਸਥਿਰ float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *ਸਿਖਰ)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  ਪੂਰਨ ਅੰਕ_floats, ਅੰਕ_ints;
  ਪੂਰਨ ਅੰਕ_fields = flatten_struct (ਸਿਖਰ, fields, fields + 3) - fields;

  ਜੇ (ਅੰਕ_fields == 1)
    {
      ਜੇ (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  ਨਹੀਂਤਾਂ ਜੇ (ਅੰਕ_fields == 2)
    {
      ਅੰਕ_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      ਅੰਕ_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      ਜੇ (ਅੰਕ_floats == 0 || ਅੰਕ_floats + ਅੰਕ_ints != 2)
	ਵਾਪਸ ret;
      ਜੇ (cb->ਵਰਤਿਆ_float + ਅੰਕ_floats > NARGREG
	  || cb->ਵਰਤਿਆ_integer + (2 - ਅੰਕ_floats) > NARGREG)
	ਵਾਪਸ ret;
      ਜੇ (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	ਵਾਪਸ ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->ਮਾਪ, fields[1]->alignment);
      ret.as_elements = 1;
    }
  ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
ਸਥਿਰ ਖਾਲੀ
marshal_atom (call_builder *cb, ਪੂਰਨ type, ਖਾਲੀ *ਅੰਕੜੇ)
{
  size_t ਮੁੱਲ = 0;
  ਚੋਣ (type)
    {
    ਹਾਲਤ FFI_TYPE_UINT8:
      ਮੁੱਲ = *(uint8_t *) ਅੰਕੜੇ;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT8:
      ਮੁੱਲ = *(int8_t *) ਅੰਕੜੇ;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16:
      ਮੁੱਲ = *(uint16_t *) ਅੰਕੜੇ;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
      ਮੁੱਲ = *(int16_t *) ਅੰਕੜੇ;
      ਰੋਕੋ;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    ਹਾਲਤ FFI_TYPE_UINT32:
      ਮੁੱਲ = *(int32_t *) ਅੰਕੜੇ;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT32:
      ਮੁੱਲ = *(int32_t *) ਅੰਕੜੇ;
      ਰੋਕੋ;
#ਜੇ __SIZEOF_POINTER__ == 8
    ਹਾਲਤ FFI_TYPE_UINT64:
      ਮੁੱਲ = *(uint64_t *) ਅੰਕੜੇ;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT64:
      ਮੁੱਲ = *(int64_t *) ਅੰਕੜੇ;
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      ਮੁੱਲ = *(size_t *) ਅੰਕੜੇ;
      ਰੋਕੋ;

#ਜੇ ABI_FRLEN >= 32
    ਹਾਲਤ FFI_TYPE_FLOAT:
      *(ਦਸ਼ਮਲਵ *)(cb->aregs->fa + cb->ਵਰਤਿਆ_float++) = *(ਦਸ਼ਮਲਵ *) ਅੰਕੜੇ;
      ਵਾਪਸ;
#ਜੇ_ਅੰਤ
#ਜੇ ABI_FRLEN >= 64
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->ਵਰਤਿਆ_float++]) = *(ਦੁੱਗਣਾ *) ਅੰਕੜੇ;
      ਵਾਪਸ;
#ਜੇ_ਅੰਤ
    ਮੂਲ:
      FFI_ASSERT (0);
      ਰੋਕੋ;
    }

  ਜੇ (cb->ਵਰਤਿਆ_integer == NARGREG)
    *cb->ਵਰਤਿਆ_ਢੇਰ++ = ਮੁੱਲ;
  ਨਹੀਂਤਾਂ
    cb->aregs->a[cb->ਵਰਤਿਆ_integer++] = ਮੁੱਲ;
}

ਸਥਿਰ ਖਾਲੀ
unmarshal_atom (call_builder *cb, ਪੂਰਨ type, ਖਾਲੀ *ਅੰਕੜੇ)
{
  size_t ਮੁੱਲ;
  ਚੋਣ (type)
    {
#ਜੇ ABI_FRLEN >= 32
    ਹਾਲਤ FFI_TYPE_FLOAT:
      *(ਦਸ਼ਮਲਵ *) ਅੰਕੜੇ = *(ਦਸ਼ਮਲਵ *)(cb->aregs->fa + cb->ਵਰਤਿਆ_float++);
      ਵਾਪਸ;
#ਜੇ_ਅੰਤ
#ਜੇ ABI_FRLEN >= 64
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      *(ਦੁੱਗਣਾ *) ਅੰਕੜੇ = cb->aregs->fa[cb->ਵਰਤਿਆ_float++];
      ਵਾਪਸ;
#ਜੇ_ਅੰਤ
    }

  ਜੇ (cb->ਵਰਤਿਆ_integer == NARGREG)
    ਮੁੱਲ = *cb->ਵਰਤਿਆ_ਢੇਰ++;
  ਨਹੀਂਤਾਂ
    ਮੁੱਲ = cb->aregs->a[cb->ਵਰਤਿਆ_integer++];

  ਚੋਣ (type)
    {
    ਹਾਲਤ FFI_TYPE_UINT8:
    ਹਾਲਤ FFI_TYPE_SINT8:
    ਹਾਲਤ FFI_TYPE_UINT16:
    ਹਾਲਤ FFI_TYPE_SINT16:
    ਹਾਲਤ FFI_TYPE_UINT32:
    ਹਾਲਤ FFI_TYPE_SINT32:
#ਜੇ __SIZEOF_POINTER__ == 8
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      *(ffi_arg *)ਅੰਕੜੇ = ਮੁੱਲ;
      ਰੋਕੋ;
    ਮੂਲ:
      FFI_ASSERT (0);
      ਰੋਕੋ;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ਸਥਿਰ ਖਾਲੀ *
allocate_and_copy_struct_ਮੰਜ਼ਿਲ_ਢੇਰ (call_builder *cb, ਖਾਲੀ *ਅੰਕੜੇ,
				   ffi_type *type)
{
  size_t dest = cb->ਅਗਲਾ_struct_area - type->ਮਾਪ;

  dest = FFI_ALIGN_ਥੱਲੇ (dest, type->alignment);
  cb->ਅਗਲਾ_struct_area = dest;

  ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਅੱਖਰ *)cb->ਢੇਰ + dest, ਅੰਕੜੇ, type->ਮਾਪ);
}

/* Adds an argument to a call, or a not by reference return value.  */
ਸਥਿਰ ਖਾਲੀ
marshal (call_builder *cb, ffi_type *type, ਪੂਰਨ var, ਖਾਲੀ *ਅੰਕੜੇ)
{
  size_t realign[2];

#ਜੇ ABI_FRLEN
  ਜੇ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ਜੇ (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, ਅੰਕੜੇ);
	  ਜੇ (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((ਅੱਖਰ *) ਅੰਕੜੇ) + fsi.offset2);
	  ਵਾਪਸ;
	}
    }

  ਜੇ (!var && cb->ਵਰਤਿਆ_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, ਅੰਕੜੇ);
      ਵਾਪਸ;
    }

  ਦੁੱਗਣਾ promoted;
  ਜੇ (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(ਦਸ਼ਮਲਵ *) ਅੰਕੜੇ;
      type = &ffi_type_double;
      ਅੰਕੜੇ = &promoted;
    }
#ਜੇ_ਅੰਤ

  ਜੇ (type->ਮਾਪ > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_ਮੰਜ਼ਿਲ_ਢੇਰ (cb, ਅੰਕੜੇ, type);
      ਅੰਕੜੇ = (ਅੱਖਰ *)cb->ਢੇਰ + cb->ਅਗਲਾ_struct_area;
      marshal_atom (cb, FFI_TYPE_ਸੂਚਕ, &ਅੰਕੜੇ);
    }
  ਨਹੀਂਤਾਂ ਜੇ (IS_INT (type->type) || type->type == FFI_TYPE_ਸੂਚਕ)
    marshal_atom (cb, type->type, ਅੰਕੜੇ);
  ਨਹੀਂਤਾਂ
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      ਜੇ (type->alignment > __SIZEOF_POINTER__)
	{
	  ਜੇ (var)
	    cb->ਵਰਤਿਆ_integer = FFI_ALIGN (cb->ਵਰਤਿਆ_integer, 2);
	  cb->ਵਰਤਿਆ_ਢੇਰ
	    = (size_t *) FFI_ALIGN (cb->ਵਰਤਿਆ_ਢੇਰ, 2 * __SIZEOF_POINTER__);
	}

      ਸਮ੍ਰਿਤੀ_ਨਕਲ (realign, ਅੰਕੜੇ, type->ਮਾਪ);
      ਜੇ (type->ਮਾਪ > 0)
	marshal_atom (cb, FFI_TYPE_ਸੂਚਕ, realign);
      ਜੇ (type->ਮਾਪ > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_ਸੂਚਕ, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
ਸਥਿਰ ਖਾਲੀ *
unmarshal (call_builder *cb, ffi_type *type, ਪੂਰਨ var, ਖਾਲੀ *ਅੰਕੜੇ)
{
  size_t realign[2];
  ਖਾਲੀ *ਸੂਚਕ;

#ਜੇ ABI_FRLEN
  ਜੇ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ਜੇ (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, ਅੰਕੜੇ);
	  ਜੇ (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((ਅੱਖਰ *) ਅੰਕੜੇ) + fsi.offset2);
	  ਵਾਪਸ ਅੰਕੜੇ;
	}
    }

  ਜੇ (!var && cb->ਵਰਤਿਆ_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, ਅੰਕੜੇ);
      ਵਾਪਸ ਅੰਕੜੇ;
    }

  ਜੇ (var && type->type == FFI_TYPE_FLOAT)
    {
      ਪੂਰਨ m = cb->ਵਰਤਿਆ_integer;
      ਖਾਲੀ *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->ਵਰਤਿਆ_ਢੇਰ + m - NARGREG + 1;
      *(ਦਸ਼ਮਲਵ *) promoted = *(ਦੁੱਗਣਾ *) promoted;
    }
#ਜੇ_ਅੰਤ

  ਜੇ (type->ਮਾਪ > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_ਸੂਚਕ, (ਅੱਖਰ *) &ਸੂਚਕ);
      ਵਾਪਸ ਸੂਚਕ;
    }
  ਨਹੀਂਤਾਂ ਜੇ (IS_INT (type->type) || type->type == FFI_TYPE_ਸੂਚਕ)
    {
      unmarshal_atom (cb, type->type, ਅੰਕੜੇ);
      ਵਾਪਸ ਅੰਕੜੇ;
    }
  ਨਹੀਂਤਾਂ
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      ਜੇ (type->alignment > __SIZEOF_POINTER__)
	{
	  ਜੇ (var)
	    cb->ਵਰਤਿਆ_integer = FFI_ALIGN (cb->ਵਰਤਿਆ_integer, 2);
	  cb->ਵਰਤਿਆ_ਢੇਰ
	    = (size_t *) FFI_ALIGN (cb->ਵਰਤਿਆ_ਢੇਰ, 2 * __SIZEOF_POINTER__);
	}

      ਜੇ (type->ਮਾਪ > 0)
	unmarshal_atom (cb, FFI_TYPE_ਸੂਚਕ, realign);
      ਜੇ (type->ਮਾਪ > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_ਸੂਚਕ, realign + 1);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ (ਅੰਕੜੇ, realign, type->ਮਾਪ);
      ਵਾਪਸ ਅੰਕੜੇ;
    }
}

ਸਥਿਰ ਪੂਰਨ
passed_by_ref (call_builder *cb, ffi_type *type, ਪੂਰਨ var)
{
#ਜੇ ABI_FRLEN
  ਜੇ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ਜੇ (fsi.as_elements)
	ਵਾਪਸ 0;
    }
#ਜੇ_ਅੰਤ

  ਵਾਪਸ type->ਮਾਪ > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  ਵਾਪਸ FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep_var (ffi_cif *cif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  ਵਾਪਸ FFI_OK;
}

/* Low level routine for calling functions.  */
ਬਾਹਰੀ ਖਾਲੀ ffi_call_asm (ਖਾਲੀ *ਢੇਰ, ਬਣਤਰ call_context *regs,
			  ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *ਘੇਰਾ) FFI_HIDDEN;

ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue,
	      ਖਾਲੀ *ਘੇਰਾ)
{
  size_t arg_ਬਾਈਟਾਂ = FFI_ALIGN (cif->ਬਾਈਟਾਂ, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_ਬਾਈਟਾਂ = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * ਆਕਾਰ(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_ਬਾਈਟਾਂ = 0;
  ਜੇ (rvalue == ਨਿਰਾ && cif->rtype->ਮਾਪ > 2 * __SIZEOF_POINTER__)
    rval_ਬਾਈਟਾਂ = FFI_ALIGN (cif->rtype->ਮਾਪ, STKALIGN);
  size_t alloc_ਮਾਪ = arg_ਬਾਈਟਾਂ + extra_ਬਾਈਟਾਂ + rval_ਬਾਈਟਾਂ + ਆਕਾਰ (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_ਅਧਾਰ;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  ਜੇ (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_ਅਧਾਰ = (size_t) alloca (alloc_ਮਾਪ);
  ਨਹੀਂਤਾਂ
    alloc_ਅਧਾਰ = FFI_ALIGN (alloca (alloc_ਮਾਪ + STKALIGN - 1), STKALIGN);

  ਜੇ (rval_ਬਾਈਟਾਂ)
    rvalue = (ਖਾਲੀ *) (alloc_ਅਧਾਰ + arg_ਬਾਈਟਾਂ);

  call_builder cb;
  cb.ਵਰਤਿਆ_float = cb.ਵਰਤਿਆ_integer = 0;
  cb.aregs = (call_context *) (alloc_ਅਧਾਰ + arg_ਬਾਈਟਾਂ + extra_ਬਾਈਟਾਂ + rval_ਬਾਈਟਾਂ);
  cb.ਵਰਤਿਆ_ਢੇਰ = (size_t *)((ਅੱਖਰ *) alloc_ਅਧਾਰ);
  cb.ਢੇਰ = (size_t *)((ਅੱਖਰ *) alloc_ਅਧਾਰ + extra_ਬਾਈਟਾਂ + rval_ਬਾਈਟਾਂ);
  cb.ਅਗਲਾ_struct_area = arg_ਬਾਈਟਾਂ;

  ਪੂਰਨ return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  ਜੇ (return_by_ref)
    cb.aregs->a[cb.ਵਰਤਿਆ_integer++] = (size_t)rvalue;

  ਪੂਰਨ i;
  ਲਈ (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((ਖਾਲੀ *) alloc_ਅਧਾਰ, cb.aregs, fn, ਘੇਰਾ);

  cb.ਵਰਤਿਆ_float = cb.ਵਰਤਿਆ_integer = 0;
  ਜੇ (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਨਿਰਾ);
}

ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue,
	     ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਘੇਰਾ);
}

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_asm (ਖਾਲੀ) FFI_HIDDEN;

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ *ਘੇਰਾ, ffi_cif *cif,
		      ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		      ਖਾਲੀ *user_ਅੰਕੜੇ, ਖਾਲੀ *codeloc)
{
  uint32_t *tramp = (uint32_t *) &ਘੇਰਾ->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_ਘੇਰਾ_asm;

  ਜੇ (cif->abi <= FFI_ਪਹਿਲਾ_ABI || cif->abi >= FFI_ਆਖਰੀ_ABI)
    ਵਾਪਸ FFI_BAD_ABI;

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
  ਜੇ (ffi_tramp_is_present(ਘੇਰਾ))
    {
      ffi_tramp_ਸੈੱਟ_parms (ਘੇਰਾ->ftramp, ffi_ਘੇਰਾ_asm, ਘੇਰਾ);
      ਜਾਓ ਬਾਹਰਲਾ;
    }
#ਜੇ_ਅੰਤ

  /* Fill the dynamic trampoline.  We will call ffi_closure_inner with codeloc,
     not closure, but as long as the memory is readable it should work.  */
  tramp[0] = 0x1800000c; /* pcaddi $t0, 0 (i.e. $t0 <- tramp) */
  tramp[1] = 0x28c0418d; /* ld.d   $t1, $t0, 16 */
  tramp[2] = 0x4c0001a0; /* jirl   $zero, $t1, 0 */
  tramp[3] = 0x03400000; /* nop */
  tramp[4] = fn;
  tramp[5] = fn >> 32;

  __builtin___clear_cache (codeloc, codeloc + FFI_TRAMPOLINE_ਮਾਪ);

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਬਾਹਰਲਾ:
#ਜੇ_ਅੰਤ
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

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
   registers will be reloaded from aregs.  */
ਖਾਲੀ FFI_HIDDEN
ffi_ਘੇਰਾ_inner (ffi_cif *cif,
		   ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		   ਖਾਲੀ *user_ਅੰਕੜੇ, size_t *ਢੇਰ, call_context *aregs)
{
  ਖਾਲੀ **avalue = alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  ਅੱਖਰ *astorage = alloca (cif->nargs * MAXCOPYARG);
  ਖਾਲੀ *rvalue;
  call_builder cb;
  ਪੂਰਨ return_by_ref;
  ਪੂਰਨ i;

  cb.aregs = aregs;
  cb.ਵਰਤਿਆ_integer = cb.ਵਰਤਿਆ_float = 0;
  cb.ਵਰਤਿਆ_ਢੇਰ = ਢੇਰ;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  ਜੇ (return_by_ref)
    unmarshal (&cb, &ffi_type_ਸੂਚਕ, 0, &rvalue);
  ਨਹੀਂਤਾਂ
    rvalue = alloca (cif->rtype->ਮਾਪ);

  ਲਈ (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);

  ਜੇ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.ਵਰਤਿਆ_integer = cb.ਵਰਤਿਆ_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਖਾਲੀ *
ffi_tramp_arch (size_t *tramp_ਮਾਪ, size_t *map_ਮਾਪ)
{
  ਬਾਹਰੀ ਖਾਲੀ *trampoline_ਕੋਡ_ਸਾਰਣੀ;

  *tramp_ਮਾਪ = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_ਮਾਪ = 1 << 16;
  ਵਾਪਸ &trampoline_ਕੋਡ_ਸਾਰਣੀ;
}
#ਜੇ_ਅੰਤ
