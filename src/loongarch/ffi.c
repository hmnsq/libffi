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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ <tramp.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>

#ଯଦି defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#ନଚେତଯଦି defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT ଦଶମିକ
#ନଚେତଯଦି defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT ଦ୍ୱିଗୁଣ
#ଅନ୍ୟଥା
#error unsupported LoongArch floating-point ABI
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * ଆକାର (ଦ୍ୱିଗୁଣ))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା call_context
{
#ଯଦି !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#endif
  size_t a[10];
} call_context;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା call_builder
{
  call_context *aregs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା used_integer;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା used_float;
  size_t *used_ଗଦା;	    /* Stack pointer.  */
  size_t *ଗଦା;	    /* Place for arguments pass by reference.  */
  size_t next_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#ଯଦି __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ଅନ୍ୟଥା
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#ଯଦି ABI_FRLEN
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା float_struct_info
{
  ଅକ୍ଷର as_elements;
  ଅକ୍ଷର type1;
  ଅକ୍ଷର offset2;
  ଅକ୍ଷର type2;
} float_struct_info;

#ଯଦି ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#ଅନ୍ୟଥା
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

ସ୍ଥିର ffi_type **
flatten_struct (ffi_type *in, ffi_type **out, ffi_type **out_end)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି (out == out_end)
    ଫେରନ୍ତୁ out;
  ଯଦି (in->type != FFI_TYPE_STRUCT)
    *(out++) = in;
  ଅନ୍ୟଥା
    ପାଇଁ (i = 0; in->elements[i]; i++)
      out = flatten_struct (in->elements[i], out, out_end);
  ଫେରନ୍ତୁ out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
ସ୍ଥିର float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *ଶୀର୍ଷ)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_floats, ଅଙ୍କ_ints;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_fields = flatten_struct (ଶୀର୍ଷ, fields, fields + 3) - fields;

  ଯଦି (ଅଙ୍କ_fields == 1)
    {
      ଯଦି (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  ଅନ୍ୟଥା ଯଦି (ଅଙ୍କ_fields == 2)
    {
      ଅଙ୍କ_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      ଅଙ୍କ_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      ଯଦି (ଅଙ୍କ_floats == 0 || ଅଙ୍କ_floats + ଅଙ୍କ_ints != 2)
	ଫେରନ୍ତୁ ret;
      ଯଦି (cb->used_float + ଅଙ୍କ_floats > NARGREG
	  || cb->used_integer + (2 - ଅଙ୍କ_floats) > NARGREG)
	ଫେରନ୍ତୁ ret;
      ଯଦି (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	ଫେରନ୍ତୁ ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->size, fields[1]->alignment);
      ret.as_elements = 1;
    }
  ଫେରନ୍ତୁ ret;
}
#endif

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
ସ୍ଥିର ଶୂନ୍ୟ
marshal_atom (call_builder *cb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ଶୂନ୍ୟ *data)
{
  size_t ମୂଲ୍ୟ = 0;
  ବାଛ (type)
    {
    ସ୍ଥିତି FFI_TYPE_UINT8:
      ମୂଲ୍ୟ = *(uint8_t *) data;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT8:
      ମୂଲ୍ୟ = *(int8_t *) data;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT16:
      ମୂଲ୍ୟ = *(uint16_t *) data;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT16:
      ମୂଲ୍ୟ = *(int16_t *) data;
      ଭାଙ୍ଗ;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    ସ୍ଥିତି FFI_TYPE_UINT32:
      ମୂଲ୍ୟ = *(int32_t *) data;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT32:
      ମୂଲ୍ୟ = *(int32_t *) data;
      ଭାଙ୍ଗ;
#ଯଦି __SIZEOF_POINTER__ == 8
    ସ୍ଥିତି FFI_TYPE_UINT64:
      ମୂଲ୍ୟ = *(uint64_t *) data;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT64:
      ମୂଲ୍ୟ = *(int64_t *) data;
      ଭାଙ୍ଗ;
#endif
    ସ୍ଥିତି FFI_TYPE_POINTER:
      ମୂଲ୍ୟ = *(size_t *) data;
      ଭାଙ୍ଗ;

#ଯଦି ABI_FRLEN >= 32
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      *(ଦଶମିକ *)(cb->aregs->fa + cb->used_float++) = *(ଦଶମିକ *) data;
      ଫେରନ୍ତୁ;
#endif
#ଯଦି ABI_FRLEN >= 64
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->used_float++]) = *(ଦ୍ୱିଗୁଣ *) data;
      ଫେରନ୍ତୁ;
#endif
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT (0);
      ଭାଙ୍ଗ;
    }

  ଯଦି (cb->used_integer == NARGREG)
    *cb->used_ଗଦା++ = ମୂଲ୍ୟ;
  ଅନ୍ୟଥା
    cb->aregs->a[cb->used_integer++] = ମୂଲ୍ୟ;
}

ସ୍ଥିର ଶୂନ୍ୟ
unmarshal_atom (call_builder *cb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ଶୂନ୍ୟ *data)
{
  size_t ମୂଲ୍ୟ;
  ବାଛ (type)
    {
#ଯଦି ABI_FRLEN >= 32
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      *(ଦଶମିକ *) data = *(ଦଶମିକ *)(cb->aregs->fa + cb->used_float++);
      ଫେରନ୍ତୁ;
#endif
#ଯଦି ABI_FRLEN >= 64
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      *(ଦ୍ୱିଗୁଣ *) data = cb->aregs->fa[cb->used_float++];
      ଫେରନ୍ତୁ;
#endif
    }

  ଯଦି (cb->used_integer == NARGREG)
    ମୂଲ୍ୟ = *cb->used_ଗଦା++;
  ଅନ୍ୟଥା
    ମୂଲ୍ୟ = cb->aregs->a[cb->used_integer++];

  ବାଛ (type)
    {
    ସ୍ଥିତି FFI_TYPE_UINT8:
    ସ୍ଥିତି FFI_TYPE_SINT8:
    ସ୍ଥିତି FFI_TYPE_UINT16:
    ସ୍ଥିତି FFI_TYPE_SINT16:
    ସ୍ଥିତି FFI_TYPE_UINT32:
    ସ୍ଥିତି FFI_TYPE_SINT32:
#ଯଦି __SIZEOF_POINTER__ == 8
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_SINT64:
#endif
    ସ୍ଥିତି FFI_TYPE_POINTER:
      *(ffi_arg *)data = ମୂଲ୍ୟ;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT (0);
      ଭାଙ୍ଗ;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ସ୍ଥିର ଶୂନ୍ୟ *
allocate_and_copy_struct_to_ଗଦା (call_builder *cb, ଶୂନ୍ୟ *data,
				   ffi_type *type)
{
  size_t dest = cb->next_struct_area - type->size;

  dest = FFI_ALIGN_DOWN (dest, type->alignment);
  cb->next_struct_area = dest;

  ଫେରନ୍ତୁ memcpy ((ଅକ୍ଷର *)cb->ଗଦା + dest, data, type->size);
}

/* Adds an argument to a call, or a not by reference return value.  */
ସ୍ଥିର ଶୂନ୍ୟ
marshal (call_builder *cb, ffi_type *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା var, ଶୂନ୍ୟ *data)
{
  size_t realign[2];

#ଯଦି ABI_FRLEN
  ଯଦି (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ଯଦି (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, data);
	  ଯଦି (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((ଅକ୍ଷର *) data) + fsi.offset2);
	  ଫେରନ୍ତୁ;
	}
    }

  ଯଦି (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, data);
      ଫେରନ୍ତୁ;
    }

  ଦ୍ୱିଗୁଣ promoted;
  ଯଦି (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(ଦଶମିକ *) data;
      type = &ffi_type_double;
      data = &promoted;
    }
#endif

  ଯଦି (type->size > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_to_ଗଦା (cb, data, type);
      data = (ଅକ୍ଷର *)cb->ଗଦା + cb->next_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &data);
    }
  ଅନ୍ୟଥା ଯଦି (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, data);
  ଅନ୍ୟଥା
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      ଯଦି (type->alignment > __SIZEOF_POINTER__)
	{
	  ଯଦି (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_ଗଦା
	    = (size_t *) FFI_ALIGN (cb->used_ଗଦା, 2 * __SIZEOF_POINTER__);
	}

      memcpy (realign, data, type->size);
      ଯଦି (type->size > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      ଯଦି (type->size > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
ସ୍ଥିର ଶୂନ୍ୟ *
unmarshal (call_builder *cb, ffi_type *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା var, ଶୂନ୍ୟ *data)
{
  size_t realign[2];
  ଶୂନ୍ୟ *pointer;

#ଯଦି ABI_FRLEN
  ଯଦି (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ଯଦି (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, data);
	  ଯଦି (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((ଅକ୍ଷର *) data) + fsi.offset2);
	  ଫେରନ୍ତୁ data;
	}
    }

  ଯଦି (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, data);
      ଫେରନ୍ତୁ data;
    }

  ଯଦି (var && type->type == FFI_TYPE_FLOAT)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା m = cb->used_integer;
      ଶୂନ୍ୟ *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->used_ଗଦା + m - NARGREG + 1;
      *(ଦଶମିକ *) promoted = *(ଦ୍ୱିଗୁଣ *) promoted;
    }
#endif

  ଯଦି (type->size > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (ଅକ୍ଷର *) &pointer);
      ଫେରନ୍ତୁ pointer;
    }
  ଅନ୍ୟଥା ଯଦି (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, data);
      ଫେରନ୍ତୁ data;
    }
  ଅନ୍ୟଥା
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      ଯଦି (type->alignment > __SIZEOF_POINTER__)
	{
	  ଯଦି (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_ଗଦା
	    = (size_t *) FFI_ALIGN (cb->used_ଗଦା, 2 * __SIZEOF_POINTER__);
	}

      ଯଦି (type->size > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      ଯଦି (type->size > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      memcpy (data, realign, type->size);
      ଫେରନ୍ତୁ data;
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
passed_by_ref (call_builder *cb, ffi_type *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା var)
{
#ଯଦି ABI_FRLEN
  ଯଦି (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ଯଦି (fsi.as_elements)
	ଫେରନ୍ତୁ 0;
    }
#endif

  ଫେରନ୍ତୁ type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  ଫେରନ୍ତୁ FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_status
ffi_prep_cif_machdep_var (ffi_cif *cif, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs,
			  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  ଫେରନ୍ତୁ FFI_OK;
}

/* Low level routine for calling functions.  */
ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_asm (ଶୂନ୍ୟ *ଗଦା, ସଂରଚନା call_context *regs,
			  ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *closure) FFI_HIDDEN;

ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue,
	      ଶୂନ୍ୟ *closure)
{
  size_t arg_bytes = FFI_ALIGN (cif->bytes, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_bytes = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * ଆକାର(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_bytes = 0;
  ଯଦି (rvalue == ଖାଲି && cif->rtype->size > 2 * __SIZEOF_POINTER__)
    rval_bytes = FFI_ALIGN (cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + extra_bytes + rval_bytes + ଆକାର (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_base;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  ଯଦି (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_base = (size_t) alloca (alloc_size);
  ଅନ୍ୟଥା
    alloc_base = FFI_ALIGN (alloca (alloc_size + STKALIGN - 1), STKALIGN);

  ଯଦି (rval_bytes)
    rvalue = (ଶୂନ୍ୟ *) (alloc_base + arg_bytes);

  call_builder cb;
  cb.used_float = cb.used_integer = 0;
  cb.aregs = (call_context *) (alloc_base + arg_bytes + extra_bytes + rval_bytes);
  cb.used_ଗଦା = (size_t *)((ଅକ୍ଷର *) alloc_base);
  cb.ଗଦା = (size_t *)((ଅକ୍ଷର *) alloc_base + extra_bytes + rval_bytes);
  cb.next_struct_area = arg_bytes;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  ଯଦି (return_by_ref)
    cb.aregs->a[cb.used_integer++] = (size_t)rvalue;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((ଶୂନ୍ୟ *) alloc_base, cb.aregs, fn, closure);

  cb.used_float = cb.used_integer = 0;
  ଯଦି (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ଖାଲି);
}

ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue,
	     ଶୂନ୍ୟ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_asm (ଶୂନ୍ୟ) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
		      ଶୂନ୍ୟ *user_data, ଶୂନ୍ୟ *codeloc)
{
  uint32_t *tramp = (uint32_t *) &closure->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

  ଯଦି (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
  ଯଦି (ffi_tramp_is_present(closure))
    {
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_asm, closure);
      ଯାଅ out;
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

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_asm (ଶୂନ୍ୟ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *))
{
  ଯଦି (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  closure->tramp = (ଶୂନ୍ୟ *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;
  ଫେରନ୍ତୁ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
ଶୂନ୍ୟ FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
		   ଶୂନ୍ୟ *user_data, size_t *ଗଦା, call_context *aregs)
{
  ଶୂନ୍ୟ **avalue = alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  ଅକ୍ଷର *astorage = alloca (cif->nargs * MAXCOPYARG);
  ଶୂନ୍ୟ *rvalue;
  call_builder cb;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା return_by_ref;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  cb.aregs = aregs;
  cb.used_integer = cb.used_float = 0;
  cb.used_ଗଦା = ଗଦା;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  ଯଦି (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  ଅନ୍ୟଥା
    rvalue = alloca (cif->rtype->size);

  ପାଇଁ (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_data);

  ଯଦି (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.used_integer = cb.used_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ଶୂନ୍ୟ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ବାହ୍ୟ ଶୂନ୍ୟ *trampoline_code_table;

  *tramp_size = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_size = 1 << 16;
  ଫେରନ୍ତୁ &trampoline_code_table;
}
#endif
