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

#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <tramp.h>

#ضمن <stdlib.h>
#ضمن <stdint.h>

#إذا defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#وإلاإذا defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT عائم
#وإلاإذا defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT مزدوج
#وإلا
#error unsupported LoongArch floating-point ABI
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * حجم (مزدوج))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
تعريفنوع بنية call_context
{
#إذا !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#endif
  size_t a[10];
} call_context;

تعريفنوع بنية call_builder
{
  call_context *aregs;
  صحيح used_integer;
  صحيح used_float;
  size_t *used_مكدس;	    /* Stack pointer.  */
  size_t *مكدس;	    /* Place for arguments pass by reference.  */
  size_t next_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#إذا __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#إذا ABI_FRLEN
تعريفنوع بنية float_struct_info
{
  حرف as_elements;
  حرف type1;
  حرف offset2;
  حرف type2;
} float_struct_info;

#إذا ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#وإلا
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

ساكن ffi_type **
flatten_struct (ffi_type *in, ffi_type **out, ffi_type **out_end)
{
  صحيح i;

  إذا (out == out_end)
    إرجاع out;
  إذا (in->type != FFI_TYPE_STRUCT)
    *(out++) = in;
  وإلا
    لكل (i = 0; in->elements[i]; i++)
      out = flatten_struct (in->elements[i], out, out_end);
  إرجاع out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
ساكن float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *قمة)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  صحيح رقم_floats, رقم_ints;
  صحيح رقم_fields = flatten_struct (قمة, fields, fields + 3) - fields;

  إذا (رقم_fields == 1)
    {
      إذا (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  وإلا إذا (رقم_fields == 2)
    {
      رقم_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      رقم_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      إذا (رقم_floats == 0 || رقم_floats + رقم_ints != 2)
	إرجاع ret;
      إذا (cb->used_float + رقم_floats > NARGREG
	  || cb->used_integer + (2 - رقم_floats) > NARGREG)
	إرجاع ret;
      إذا (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	إرجاع ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->size, fields[1]->alignment);
      ret.as_elements = 1;
    }
  إرجاع ret;
}
#endif

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
ساكن فراغ
marshal_atom (call_builder *cb, صحيح type, فراغ *data)
{
  size_t قيمة = 0;
  بدل (type)
    {
    حالة FFI_TYPE_UINT8:
      قيمة = *(uint8_t *) data;
      اكسر;
    حالة FFI_TYPE_SINT8:
      قيمة = *(int8_t *) data;
      اكسر;
    حالة FFI_TYPE_UINT16:
      قيمة = *(uint16_t *) data;
      اكسر;
    حالة FFI_TYPE_SINT16:
      قيمة = *(int16_t *) data;
      اكسر;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    حالة FFI_TYPE_UINT32:
      قيمة = *(int32_t *) data;
      اكسر;
    حالة FFI_TYPE_SINT32:
      قيمة = *(int32_t *) data;
      اكسر;
#إذا __SIZEOF_POINTER__ == 8
    حالة FFI_TYPE_UINT64:
      قيمة = *(uint64_t *) data;
      اكسر;
    حالة FFI_TYPE_SINT64:
      قيمة = *(int64_t *) data;
      اكسر;
#endif
    حالة FFI_TYPE_POINTER:
      قيمة = *(size_t *) data;
      اكسر;

#إذا ABI_FRLEN >= 32
    حالة FFI_TYPE_FLOAT:
      *(عائم *)(cb->aregs->fa + cb->used_float++) = *(عائم *) data;
      إرجاع;
#endif
#إذا ABI_FRLEN >= 64
    حالة FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->used_float++]) = *(مزدوج *) data;
      إرجاع;
#endif
    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }

  إذا (cb->used_integer == NARGREG)
    *cb->used_مكدس++ = قيمة;
  وإلا
    cb->aregs->a[cb->used_integer++] = قيمة;
}

ساكن فراغ
unmarshal_atom (call_builder *cb, صحيح type, فراغ *data)
{
  size_t قيمة;
  بدل (type)
    {
#إذا ABI_FRLEN >= 32
    حالة FFI_TYPE_FLOAT:
      *(عائم *) data = *(عائم *)(cb->aregs->fa + cb->used_float++);
      إرجاع;
#endif
#إذا ABI_FRLEN >= 64
    حالة FFI_TYPE_DOUBLE:
      *(مزدوج *) data = cb->aregs->fa[cb->used_float++];
      إرجاع;
#endif
    }

  إذا (cb->used_integer == NARGREG)
    قيمة = *cb->used_مكدس++;
  وإلا
    قيمة = cb->aregs->a[cb->used_integer++];

  بدل (type)
    {
    حالة FFI_TYPE_UINT8:
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT16:
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_SINT32:
#إذا __SIZEOF_POINTER__ == 8
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
#endif
    حالة FFI_TYPE_POINTER:
      *(ffi_arg *)data = قيمة;
      اكسر;
    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ساكن فراغ *
allocate_and_copy_struct_to_مكدس (call_builder *cb, فراغ *data,
				   ffi_type *type)
{
  size_t dest = cb->next_struct_area - type->size;

  dest = FFI_ALIGN_DOWN (dest, type->alignment);
  cb->next_struct_area = dest;

  إرجاع memcpy ((حرف *)cb->مكدس + dest, data, type->size);
}

/* Adds an argument to a call, or a not by reference return value.  */
ساكن فراغ
marshal (call_builder *cb, ffi_type *type, صحيح var, فراغ *data)
{
  size_t realign[2];

#إذا ABI_FRLEN
  إذا (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      إذا (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, data);
	  إذا (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((حرف *) data) + fsi.offset2);
	  إرجاع;
	}
    }

  إذا (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, data);
      إرجاع;
    }

  مزدوج promoted;
  إذا (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(عائم *) data;
      type = &ffi_type_double;
      data = &promoted;
    }
#endif

  إذا (type->size > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_to_مكدس (cb, data, type);
      data = (حرف *)cb->مكدس + cb->next_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &data);
    }
  وإلا إذا (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, data);
  وإلا
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      إذا (type->alignment > __SIZEOF_POINTER__)
	{
	  إذا (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_مكدس
	    = (size_t *) FFI_ALIGN (cb->used_مكدس, 2 * __SIZEOF_POINTER__);
	}

      memcpy (realign, data, type->size);
      إذا (type->size > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      إذا (type->size > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
ساكن فراغ *
unmarshal (call_builder *cb, ffi_type *type, صحيح var, فراغ *data)
{
  size_t realign[2];
  فراغ *pointer;

#إذا ABI_FRLEN
  إذا (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      إذا (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, data);
	  إذا (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((حرف *) data) + fsi.offset2);
	  إرجاع data;
	}
    }

  إذا (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, data);
      إرجاع data;
    }

  إذا (var && type->type == FFI_TYPE_FLOAT)
    {
      صحيح m = cb->used_integer;
      فراغ *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->used_مكدس + m - NARGREG + 1;
      *(عائم *) promoted = *(مزدوج *) promoted;
    }
#endif

  إذا (type->size > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (حرف *) &pointer);
      إرجاع pointer;
    }
  وإلا إذا (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, data);
      إرجاع data;
    }
  وإلا
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      إذا (type->alignment > __SIZEOF_POINTER__)
	{
	  إذا (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_مكدس
	    = (size_t *) FFI_ALIGN (cb->used_مكدس, 2 * __SIZEOF_POINTER__);
	}

      إذا (type->size > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      إذا (type->size > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      memcpy (data, realign, type->size);
      إرجاع data;
    }
}

ساكن صحيح
passed_by_ref (call_builder *cb, ffi_type *type, صحيح var)
{
#إذا ABI_FRLEN
  إذا (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      إذا (fsi.as_elements)
	إرجاع 0;
    }
#endif

  إرجاع type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  إرجاع FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_status
ffi_prep_cif_machdep_var (ffi_cif *cif, unsigned صحيح nfixedargs,
			  unsigned صحيح ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  إرجاع FFI_OK;
}

/* Low level routine for calling functions.  */
خارجي فراغ ffi_call_asm (فراغ *مكدس, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *closure) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *closure)
{
  size_t arg_bytes = FFI_ALIGN (cif->bytes, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_bytes = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * حجم(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_bytes = 0;
  إذا (rvalue == عدم && cif->rtype->size > 2 * __SIZEOF_POINTER__)
    rval_bytes = FFI_ALIGN (cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + extra_bytes + rval_bytes + حجم (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_base;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  إذا (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_base = (size_t) alloca (alloc_size);
  وإلا
    alloc_base = FFI_ALIGN (alloca (alloc_size + STKALIGN - 1), STKALIGN);

  إذا (rval_bytes)
    rvalue = (فراغ *) (alloc_base + arg_bytes);

  call_builder cb;
  cb.used_float = cb.used_integer = 0;
  cb.aregs = (call_context *) (alloc_base + arg_bytes + extra_bytes + rval_bytes);
  cb.used_مكدس = (size_t *)((حرف *) alloc_base);
  cb.مكدس = (size_t *)((حرف *) alloc_base + extra_bytes + rval_bytes);
  cb.next_struct_area = arg_bytes;

  صحيح return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  إذا (return_by_ref)
    cb.aregs->a[cb.used_integer++] = (size_t)rvalue;

  صحيح i;
  لكل (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((فراغ *) alloc_base, cb.aregs, fn, closure);

  cb.used_float = cb.used_integer = 0;
  إذا (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	     فراغ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

خارجي فراغ ffi_closure_asm (فراغ) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_data, فراغ *codeloc)
{
  uint32_t *tramp = (uint32_t *) &closure->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

  إذا (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    إرجاع FFI_BAD_ABI;

#إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(closure))
    {
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_asm, closure);
      اذهب out;
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

#إذا defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  إرجاع FFI_OK;
}

خارجي فراغ ffi_go_closure_asm (فراغ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    إرجاع FFI_BAD_ABI;

  closure->tramp = (فراغ *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;
  إرجاع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
فراغ FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_data, size_t *مكدس, call_context *aregs)
{
  فراغ **avalue = alloca (cif->nargs * حجم (فراغ *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  حرف *astorage = alloca (cif->nargs * MAXCOPYARG);
  فراغ *rvalue;
  call_builder cb;
  صحيح return_by_ref;
  صحيح i;

  cb.aregs = aregs;
  cb.used_integer = cb.used_float = 0;
  cb.used_مكدس = مكدس;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  إذا (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  وإلا
    rvalue = alloca (cif->rtype->size);

  لكل (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_data);

  إذا (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.used_integer = cb.used_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  خارجي فراغ *trampoline_code_table;

  *tramp_size = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_size = 1 << 16;
  إرجاع &trampoline_code_table;
}
#endif
