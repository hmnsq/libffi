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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <tramp.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdint.h>

#যদি defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#নোচেদ্যদি defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT ভিন্ন
#নোচেদ্যদি defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT দ্বিগুণ
#অন্যথা
#error unsupported LoongArch floating-point ABI
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * পরিমাণ (দ্বিগুণ))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
প্রকারনির্দেশ সংরচনা call_context
{
#যদি !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#endif
  size_t a[10];
} call_context;

প্রকারনির্দেশ সংরচনা call_builder
{
  call_context *aregs;
  পূর্ণাঙ্ক used_integer;
  পূর্ণাঙ্ক used_float;
  size_t *used_স্তূপ;	    /* Stack pointer.  */
  size_t *স্তূপ;	    /* Place for arguments pass by reference.  */
  size_t next_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#যদি __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#অন্যথা
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#যদি ABI_FRLEN
প্রকারনির্দেশ সংরচনা float_struct_info
{
  বর্ণ as_elements;
  বর্ণ type1;
  বর্ণ offset2;
  বর্ণ type2;
} float_struct_info;

#যদি ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#অন্যথা
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

স্থির ffi_type **
flatten_struct (ffi_type *in, ffi_type **out, ffi_type **out_end)
{
  পূর্ণাঙ্ক i;

  যদি (out == out_end)
    প্রত্যাগম out;
  যদি (in->type != FFI_TYPE_STRUCT)
    *(out++) = in;
  অন্যথা
    প্রতি (i = 0; in->elements[i]; i++)
      out = flatten_struct (in->elements[i], out, out_end);
  প্রত্যাগম out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
স্থির float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *শীর্ষ)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  পূর্ণাঙ্ক অঙ্ক_floats, অঙ্ক_ints;
  পূর্ণাঙ্ক অঙ্ক_fields = flatten_struct (শীর্ষ, fields, fields + 3) - fields;

  যদি (অঙ্ক_fields == 1)
    {
      যদি (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  অন্যথা যদি (অঙ্ক_fields == 2)
    {
      অঙ্ক_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      অঙ্ক_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      যদি (অঙ্ক_floats == 0 || অঙ্ক_floats + অঙ্ক_ints != 2)
	প্রত্যাগম ret;
      যদি (cb->used_float + অঙ্ক_floats > NARGREG
	  || cb->used_integer + (2 - অঙ্ক_floats) > NARGREG)
	প্রত্যাগম ret;
      যদি (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	প্রত্যাগম ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->size, fields[1]->alignment);
      ret.as_elements = 1;
    }
  প্রত্যাগম ret;
}
#endif

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
স্থির শূন্য
marshal_atom (call_builder *cb, পূর্ণাঙ্ক type, শূন্য *data)
{
  size_t মান = 0;
  বিকল্প (type)
    {
    পক্ষ FFI_TYPE_UINT8:
      মান = *(uint8_t *) data;
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT8:
      মান = *(int8_t *) data;
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT16:
      মান = *(uint16_t *) data;
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT16:
      মান = *(int16_t *) data;
      ভঙ্গ;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    পক্ষ FFI_TYPE_UINT32:
      মান = *(int32_t *) data;
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT32:
      মান = *(int32_t *) data;
      ভঙ্গ;
#যদি __SIZEOF_POINTER__ == 8
    পক্ষ FFI_TYPE_UINT64:
      মান = *(uint64_t *) data;
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT64:
      মান = *(int64_t *) data;
      ভঙ্গ;
#endif
    পক্ষ FFI_TYPE_POINTER:
      মান = *(size_t *) data;
      ভঙ্গ;

#যদি ABI_FRLEN >= 32
    পক্ষ FFI_TYPE_FLOAT:
      *(ভিন্ন *)(cb->aregs->fa + cb->used_float++) = *(ভিন্ন *) data;
      প্রত্যাগম;
#endif
#যদি ABI_FRLEN >= 64
    পক্ষ FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->used_float++]) = *(দ্বিগুণ *) data;
      প্রত্যাগম;
#endif
    স্বভাব:
      FFI_ASSERT (0);
      ভঙ্গ;
    }

  যদি (cb->used_integer == NARGREG)
    *cb->used_স্তূপ++ = মান;
  অন্যথা
    cb->aregs->a[cb->used_integer++] = মান;
}

স্থির শূন্য
unmarshal_atom (call_builder *cb, পূর্ণাঙ্ক type, শূন্য *data)
{
  size_t মান;
  বিকল্প (type)
    {
#যদি ABI_FRLEN >= 32
    পক্ষ FFI_TYPE_FLOAT:
      *(ভিন্ন *) data = *(ভিন্ন *)(cb->aregs->fa + cb->used_float++);
      প্রত্যাগম;
#endif
#যদি ABI_FRLEN >= 64
    পক্ষ FFI_TYPE_DOUBLE:
      *(দ্বিগুণ *) data = cb->aregs->fa[cb->used_float++];
      প্রত্যাগম;
#endif
    }

  যদি (cb->used_integer == NARGREG)
    মান = *cb->used_স্তূপ++;
  অন্যথা
    মান = cb->aregs->a[cb->used_integer++];

  বিকল্প (type)
    {
    পক্ষ FFI_TYPE_UINT8:
    পক্ষ FFI_TYPE_SINT8:
    পক্ষ FFI_TYPE_UINT16:
    পক্ষ FFI_TYPE_SINT16:
    পক্ষ FFI_TYPE_UINT32:
    পক্ষ FFI_TYPE_SINT32:
#যদি __SIZEOF_POINTER__ == 8
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_SINT64:
#endif
    পক্ষ FFI_TYPE_POINTER:
      *(ffi_arg *)data = মান;
      ভঙ্গ;
    স্বভাব:
      FFI_ASSERT (0);
      ভঙ্গ;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
স্থির শূন্য *
allocate_and_copy_struct_to_স্তূপ (call_builder *cb, শূন্য *data,
				   ffi_type *type)
{
  size_t dest = cb->next_struct_area - type->size;

  dest = FFI_ALIGN_DOWN (dest, type->alignment);
  cb->next_struct_area = dest;

  প্রত্যাগম memcpy ((বর্ণ *)cb->স্তূপ + dest, data, type->size);
}

/* Adds an argument to a call, or a not by reference return value.  */
স্থির শূন্য
marshal (call_builder *cb, ffi_type *type, পূর্ণাঙ্ক var, শূন্য *data)
{
  size_t realign[2];

#যদি ABI_FRLEN
  যদি (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      যদি (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, data);
	  যদি (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((বর্ণ *) data) + fsi.offset2);
	  প্রত্যাগম;
	}
    }

  যদি (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, data);
      প্রত্যাগম;
    }

  দ্বিগুণ promoted;
  যদি (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(ভিন্ন *) data;
      type = &ffi_type_double;
      data = &promoted;
    }
#endif

  যদি (type->size > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_to_স্তূপ (cb, data, type);
      data = (বর্ণ *)cb->স্তূপ + cb->next_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &data);
    }
  অন্যথা যদি (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, data);
  অন্যথা
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      যদি (type->alignment > __SIZEOF_POINTER__)
	{
	  যদি (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_স্তূপ
	    = (size_t *) FFI_ALIGN (cb->used_স্তূপ, 2 * __SIZEOF_POINTER__);
	}

      memcpy (realign, data, type->size);
      যদি (type->size > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      যদি (type->size > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
স্থির শূন্য *
unmarshal (call_builder *cb, ffi_type *type, পূর্ণাঙ্ক var, শূন্য *data)
{
  size_t realign[2];
  শূন্য *pointer;

#যদি ABI_FRLEN
  যদি (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      যদি (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, data);
	  যদি (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((বর্ণ *) data) + fsi.offset2);
	  প্রত্যাগম data;
	}
    }

  যদি (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, data);
      প্রত্যাগম data;
    }

  যদি (var && type->type == FFI_TYPE_FLOAT)
    {
      পূর্ণাঙ্ক m = cb->used_integer;
      শূন্য *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->used_স্তূপ + m - NARGREG + 1;
      *(ভিন্ন *) promoted = *(দ্বিগুণ *) promoted;
    }
#endif

  যদি (type->size > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (বর্ণ *) &pointer);
      প্রত্যাগম pointer;
    }
  অন্যথা যদি (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, data);
      প্রত্যাগম data;
    }
  অন্যথা
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      যদি (type->alignment > __SIZEOF_POINTER__)
	{
	  যদি (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_স্তূপ
	    = (size_t *) FFI_ALIGN (cb->used_স্তূপ, 2 * __SIZEOF_POINTER__);
	}

      যদি (type->size > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      যদি (type->size > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      memcpy (data, realign, type->size);
      প্রত্যাগম data;
    }
}

স্থির পূর্ণাঙ্ক
passed_by_ref (call_builder *cb, ffi_type *type, পূর্ণাঙ্ক var)
{
#যদি ABI_FRLEN
  যদি (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      যদি (fsi.as_elements)
	প্রত্যাগম 0;
    }
#endif

  প্রত্যাগম type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  প্রত্যাগম FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_status
ffi_prep_cif_machdep_var (ffi_cif *cif, unsigned পূর্ণাঙ্ক nfixedargs,
			  unsigned পূর্ণাঙ্ক ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  প্রত্যাগম FFI_OK;
}

/* Low level routine for calling functions.  */
বাহ্য শূন্য ffi_call_asm (শূন্য *স্তূপ, সংরচনা call_context *regs,
			  শূন্য (*fn) (শূন্য), শূন্য *closure) FFI_HIDDEN;

স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue,
	      শূন্য *closure)
{
  size_t arg_bytes = FFI_ALIGN (cif->bytes, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_bytes = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * পরিমাণ(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_bytes = 0;
  যদি (rvalue == রিক্ত && cif->rtype->size > 2 * __SIZEOF_POINTER__)
    rval_bytes = FFI_ALIGN (cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + extra_bytes + rval_bytes + পরিমাণ (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_base;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  যদি (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_base = (size_t) alloca (alloc_size);
  অন্যথা
    alloc_base = FFI_ALIGN (alloca (alloc_size + STKALIGN - 1), STKALIGN);

  যদি (rval_bytes)
    rvalue = (শূন্য *) (alloc_base + arg_bytes);

  call_builder cb;
  cb.used_float = cb.used_integer = 0;
  cb.aregs = (call_context *) (alloc_base + arg_bytes + extra_bytes + rval_bytes);
  cb.used_স্তূপ = (size_t *)((বর্ণ *) alloc_base);
  cb.স্তূপ = (size_t *)((বর্ণ *) alloc_base + extra_bytes + rval_bytes);
  cb.next_struct_area = arg_bytes;

  পূর্ণাঙ্ক return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  যদি (return_by_ref)
    cb.aregs->a[cb.used_integer++] = (size_t)rvalue;

  পূর্ণাঙ্ক i;
  প্রতি (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((শূন্য *) alloc_base, cb.aregs, fn, closure);

  cb.used_float = cb.used_integer = 0;
  যদি (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, রিক্ত);
}

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue,
	     শূন্য *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

বাহ্য শূন্য ffi_closure_asm (শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		      শূন্য *user_data, শূন্য *codeloc)
{
  uint32_t *tramp = (uint32_t *) &closure->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

  যদি (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    প্রত্যাগম FFI_BAD_ABI;

#যদি defined(FFI_EXEC_STATIC_TRAMP)
  যদি (ffi_tramp_is_present(closure))
    {
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_asm, closure);
      গচ্ছ out;
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

#যদি defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  প্রত্যাগম FFI_OK;
}

বাহ্য শূন্য ffi_go_closure_asm (শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *))
{
  যদি (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    প্রত্যাগম FFI_BAD_ABI;

  closure->tramp = (শূন্য *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;
  প্রত্যাগম FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
শূন্য FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		   শূন্য *user_data, size_t *স্তূপ, call_context *aregs)
{
  শূন্য **avalue = alloca (cif->nargs * পরিমাণ (শূন্য *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  বর্ণ *astorage = alloca (cif->nargs * MAXCOPYARG);
  শূন্য *rvalue;
  call_builder cb;
  পূর্ণাঙ্ক return_by_ref;
  পূর্ণাঙ্ক i;

  cb.aregs = aregs;
  cb.used_integer = cb.used_float = 0;
  cb.used_স্তূপ = স্তূপ;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  যদি (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  অন্যথা
    rvalue = alloca (cif->rtype->size);

  প্রতি (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_data);

  যদি (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.used_integer = cb.used_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#যদি defined(FFI_EXEC_STATIC_TRAMP)
শূন্য *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  বাহ্য শূন্য *trampoline_code_table;

  *tramp_size = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_size = 1 << 16;
  প্রত্যাগম &trampoline_code_table;
}
#endif
