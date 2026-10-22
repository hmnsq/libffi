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

#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <tramp.h>

#شامل <stdlib.h>
#شامل <stdint.h>

#اگر defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#وگرنهاگر defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT اعشاری
#وگرنهاگر defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT دوگانه
#وگرنه
#error unsupported LoongArch floating-point ABI
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * اندازه (دوگانه))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
تعریفنوع ساختار call_context
{
#اگر !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#endif
  size_t a[10];
} call_context;

تعریفنوع ساختار call_builder
{
  call_context *aregs;
  صحیح used_integer;
  صحیح used_float;
  size_t *used_پشته;	    /* Stack pointer.  */
  size_t *پشته;	    /* Place for arguments pass by reference.  */
  size_t next_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#اگر __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وگرنه
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#اگر ABI_FRLEN
تعریفنوع ساختار float_struct_info
{
  نویسه as_elements;
  نویسه type1;
  نویسه offset2;
  نویسه type2;
} float_struct_info;

#اگر ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#وگرنه
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

ایستا ffi_type **
flatten_struct (ffi_type *in, ffi_type **out, ffi_type **out_end)
{
  صحیح i;

  اگر (out == out_end)
    بازگشت out;
  اگر (in->type != FFI_TYPE_STRUCT)
    *(out++) = in;
  وگرنه
    برای (i = 0; in->elements[i]; i++)
      out = flatten_struct (in->elements[i], out, out_end);
  بازگشت out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
ایستا float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *بالاترین)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  صحیح رقم_floats, رقم_ints;
  صحیح رقم_fields = flatten_struct (بالاترین, fields, fields + 3) - fields;

  اگر (رقم_fields == 1)
    {
      اگر (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  وگرنه اگر (رقم_fields == 2)
    {
      رقم_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      رقم_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      اگر (رقم_floats == 0 || رقم_floats + رقم_ints != 2)
	بازگشت ret;
      اگر (cb->used_float + رقم_floats > NARGREG
	  || cb->used_integer + (2 - رقم_floats) > NARGREG)
	بازگشت ret;
      اگر (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	بازگشت ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->size, fields[1]->alignment);
      ret.as_elements = 1;
    }
  بازگشت ret;
}
#endif

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
ایستا تهی
marshal_atom (call_builder *cb, صحیح type, تهی *data)
{
  size_t مقدار = 0;
  انتخاب (type)
    {
    حالت FFI_TYPE_UINT8:
      مقدار = *(uint8_t *) data;
      بشکن;
    حالت FFI_TYPE_SINT8:
      مقدار = *(int8_t *) data;
      بشکن;
    حالت FFI_TYPE_UINT16:
      مقدار = *(uint16_t *) data;
      بشکن;
    حالت FFI_TYPE_SINT16:
      مقدار = *(int16_t *) data;
      بشکن;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    حالت FFI_TYPE_UINT32:
      مقدار = *(int32_t *) data;
      بشکن;
    حالت FFI_TYPE_SINT32:
      مقدار = *(int32_t *) data;
      بشکن;
#اگر __SIZEOF_POINTER__ == 8
    حالت FFI_TYPE_UINT64:
      مقدار = *(uint64_t *) data;
      بشکن;
    حالت FFI_TYPE_SINT64:
      مقدار = *(int64_t *) data;
      بشکن;
#endif
    حالت FFI_TYPE_POINTER:
      مقدار = *(size_t *) data;
      بشکن;

#اگر ABI_FRLEN >= 32
    حالت FFI_TYPE_FLOAT:
      *(اعشاری *)(cb->aregs->fa + cb->used_float++) = *(اعشاری *) data;
      بازگشت;
#endif
#اگر ABI_FRLEN >= 64
    حالت FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->used_float++]) = *(دوگانه *) data;
      بازگشت;
#endif
    پیشفرض:
      FFI_ASSERT (0);
      بشکن;
    }

  اگر (cb->used_integer == NARGREG)
    *cb->used_پشته++ = مقدار;
  وگرنه
    cb->aregs->a[cb->used_integer++] = مقدار;
}

ایستا تهی
unmarshal_atom (call_builder *cb, صحیح type, تهی *data)
{
  size_t مقدار;
  انتخاب (type)
    {
#اگر ABI_FRLEN >= 32
    حالت FFI_TYPE_FLOAT:
      *(اعشاری *) data = *(اعشاری *)(cb->aregs->fa + cb->used_float++);
      بازگشت;
#endif
#اگر ABI_FRLEN >= 64
    حالت FFI_TYPE_DOUBLE:
      *(دوگانه *) data = cb->aregs->fa[cb->used_float++];
      بازگشت;
#endif
    }

  اگر (cb->used_integer == NARGREG)
    مقدار = *cb->used_پشته++;
  وگرنه
    مقدار = cb->aregs->a[cb->used_integer++];

  انتخاب (type)
    {
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_UINT16:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_SINT32:
#اگر __SIZEOF_POINTER__ == 8
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
#endif
    حالت FFI_TYPE_POINTER:
      *(ffi_arg *)data = مقدار;
      بشکن;
    پیشفرض:
      FFI_ASSERT (0);
      بشکن;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ایستا تهی *
allocate_and_copy_struct_to_پشته (call_builder *cb, تهی *data,
				   ffi_type *type)
{
  size_t dest = cb->next_struct_area - type->size;

  dest = FFI_ALIGN_DOWN (dest, type->alignment);
  cb->next_struct_area = dest;

  بازگشت memcpy ((نویسه *)cb->پشته + dest, data, type->size);
}

/* Adds an argument to a call, or a not by reference return value.  */
ایستا تهی
marshal (call_builder *cb, ffi_type *type, صحیح var, تهی *data)
{
  size_t realign[2];

#اگر ABI_FRLEN
  اگر (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      اگر (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, data);
	  اگر (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((نویسه *) data) + fsi.offset2);
	  بازگشت;
	}
    }

  اگر (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, data);
      بازگشت;
    }

  دوگانه promoted;
  اگر (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(اعشاری *) data;
      type = &ffi_type_double;
      data = &promoted;
    }
#endif

  اگر (type->size > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_to_پشته (cb, data, type);
      data = (نویسه *)cb->پشته + cb->next_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &data);
    }
  وگرنه اگر (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, data);
  وگرنه
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      اگر (type->alignment > __SIZEOF_POINTER__)
	{
	  اگر (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_پشته
	    = (size_t *) FFI_ALIGN (cb->used_پشته, 2 * __SIZEOF_POINTER__);
	}

      memcpy (realign, data, type->size);
      اگر (type->size > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      اگر (type->size > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
ایستا تهی *
unmarshal (call_builder *cb, ffi_type *type, صحیح var, تهی *data)
{
  size_t realign[2];
  تهی *pointer;

#اگر ABI_FRLEN
  اگر (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      اگر (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, data);
	  اگر (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((نویسه *) data) + fsi.offset2);
	  بازگشت data;
	}
    }

  اگر (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, data);
      بازگشت data;
    }

  اگر (var && type->type == FFI_TYPE_FLOAT)
    {
      صحیح m = cb->used_integer;
      تهی *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->used_پشته + m - NARGREG + 1;
      *(اعشاری *) promoted = *(دوگانه *) promoted;
    }
#endif

  اگر (type->size > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (نویسه *) &pointer);
      بازگشت pointer;
    }
  وگرنه اگر (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, data);
      بازگشت data;
    }
  وگرنه
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      اگر (type->alignment > __SIZEOF_POINTER__)
	{
	  اگر (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_پشته
	    = (size_t *) FFI_ALIGN (cb->used_پشته, 2 * __SIZEOF_POINTER__);
	}

      اگر (type->size > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      اگر (type->size > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      memcpy (data, realign, type->size);
      بازگشت data;
    }
}

ایستا صحیح
passed_by_ref (call_builder *cb, ffi_type *type, صحیح var)
{
#اگر ABI_FRLEN
  اگر (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      اگر (fsi.as_elements)
	بازگشت 0;
    }
#endif

  بازگشت type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  بازگشت FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_status
ffi_prep_cif_machdep_var (ffi_cif *cif, unsigned صحیح nfixedargs,
			  unsigned صحیح ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  بازگشت FFI_OK;
}

/* Low level routine for calling functions.  */
بیرونی تهی ffi_call_asm (تهی *پشته, ساختار call_context *regs,
			  تهی (*fn) (تهی), تهی *closure) FFI_HIDDEN;

ایستا تهی
ffi_call_int (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue, تهی **avalue,
	      تهی *closure)
{
  size_t arg_bytes = FFI_ALIGN (cif->bytes, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_bytes = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * اندازه(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_bytes = 0;
  اگر (rvalue == پوچ && cif->rtype->size > 2 * __SIZEOF_POINTER__)
    rval_bytes = FFI_ALIGN (cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + extra_bytes + rval_bytes + اندازه (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_base;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  اگر (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_base = (size_t) alloca (alloc_size);
  وگرنه
    alloc_base = FFI_ALIGN (alloca (alloc_size + STKALIGN - 1), STKALIGN);

  اگر (rval_bytes)
    rvalue = (تهی *) (alloc_base + arg_bytes);

  call_builder cb;
  cb.used_float = cb.used_integer = 0;
  cb.aregs = (call_context *) (alloc_base + arg_bytes + extra_bytes + rval_bytes);
  cb.used_پشته = (size_t *)((نویسه *) alloc_base);
  cb.پشته = (size_t *)((نویسه *) alloc_base + extra_bytes + rval_bytes);
  cb.next_struct_area = arg_bytes;

  صحیح return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  اگر (return_by_ref)
    cb.aregs->a[cb.used_integer++] = (size_t)rvalue;

  صحیح i;
  برای (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((تهی *) alloc_base, cb.aregs, fn, closure);

  cb.used_float = cb.used_integer = 0;
  اگر (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

تهی
ffi_call (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue, تهی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, پوچ);
}

تهی
ffi_call_go (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue, تهی **avalue,
	     تهی *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

بیرونی تهی ffi_closure_asm (تهی) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
		      تهی *user_data, تهی *codeloc)
{
  uint32_t *tramp = (uint32_t *) &closure->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

  اگر (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    بازگشت FFI_BAD_ABI;

#اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(closure))
    {
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_asm, closure);
      برو out;
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

#اگر defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  بازگشت FFI_OK;
}

بیرونی تهی ffi_go_closure_asm (تهی) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *))
{
  اگر (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    بازگشت FFI_BAD_ABI;

  closure->tramp = (تهی *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;
  بازگشت FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
تهی FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
		   تهی *user_data, size_t *پشته, call_context *aregs)
{
  تهی **avalue = alloca (cif->nargs * اندازه (تهی *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  نویسه *astorage = alloca (cif->nargs * MAXCOPYARG);
  تهی *rvalue;
  call_builder cb;
  صحیح return_by_ref;
  صحیح i;

  cb.aregs = aregs;
  cb.used_integer = cb.used_float = 0;
  cb.used_پشته = پشته;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  اگر (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  وگرنه
    rvalue = alloca (cif->rtype->size);

  برای (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_data);

  اگر (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.used_integer = cb.used_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#اگر defined(FFI_EXEC_STATIC_TRAMP)
تهی *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  بیرونی تهی *trampoline_code_table;

  *tramp_size = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_size = 1 << 16;
  بازگشت &trampoline_code_table;
}
#endif
