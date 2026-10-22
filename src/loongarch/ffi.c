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
#تېروتنه unsupported LoongArch floating-ټکی ABI
#د_که_پای

#تعریف_کړه NARGREG 8
#تعریف_کړه STKALIGN 16
#تعریف_کړه MAXCOPYARG (2 * حجم (مزدوج))

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
#د_که_پای
  size_t a[10];
} call_context;

تعريفنوع بنية call_builder
{
  call_context *aregs;
  صحيح کارول_شوی_integer;
  صحيح کارول_شوی_float;
  size_t *کارول_شوی_ډېری;	    /* Stack pointer.  */
  size_t *ډېری;	    /* Place for arguments pass by reference.  */
  size_t بل_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#إذا __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#د_که_پای

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
#د_که_پای

ساكن ffi_type **
flatten_struct (ffi_type *دننه, ffi_type **بهر, ffi_type **بهر_پای_ټکی)
{
  صحيح i;

  إذا (بهر == بهر_پای_ټکی)
    أرجع بهر;
  إذا (دننه->type != FFI_TYPE_STRUCT)
    *(بهر++) = دننه;
  وإلا
    لكل (i = 0; دننه->elements[i]; i++)
      بهر = flatten_struct (دننه->elements[i], بهر, بهر_پای_ټکی);
  أرجع بهر;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
ساكن float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *سر_2)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  صحيح عدد_floats, عدد_ints;
  صحيح عدد_fields = flatten_struct (سر_2, fields, fields + 3) - fields;

  إذا (عدد_fields == 1)
    {
      إذا (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  وإلا إذا (عدد_fields == 2)
    {
      عدد_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      عدد_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      إذا (عدد_floats == 0 || عدد_floats + عدد_ints != 2)
	أرجع ret;
      إذا (cb->کارول_شوی_float + عدد_floats > NARGREG
	  || cb->کارول_شوی_integer + (2 - عدد_floats) > NARGREG)
	أرجع ret;
      إذا (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	أرجع ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->اندازه, fields[1]->alignment);
      ret.as_elements = 1;
    }
  أرجع ret;
}
#د_که_پای

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
ساكن فراغ
marshal_atom (call_builder *cb, صحيح type, فراغ *معلومات)
{
  size_t ارزښت = 0;
  بدل (type)
    {
    حالة FFI_TYPE_UINT8:
      ارزښت = *(uint8_t *) معلومات;
      اكسر;
    حالة FFI_TYPE_SINT8:
      ارزښت = *(int8_t *) معلومات;
      اكسر;
    حالة FFI_TYPE_UINT16:
      ارزښت = *(uint16_t *) معلومات;
      اكسر;
    حالة FFI_TYPE_SINT16:
      ارزښت = *(int16_t *) معلومات;
      اكسر;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    حالة FFI_TYPE_UINT32:
      ارزښت = *(int32_t *) معلومات;
      اكسر;
    حالة FFI_TYPE_SINT32:
      ارزښت = *(int32_t *) معلومات;
      اكسر;
#إذا __SIZEOF_POINTER__ == 8
    حالة FFI_TYPE_UINT64:
      ارزښت = *(uint64_t *) معلومات;
      اكسر;
    حالة FFI_TYPE_SINT64:
      ارزښت = *(int64_t *) معلومات;
      اكسر;
#د_که_پای
    حالة FFI_TYPE_اشاره_ګر:
      ارزښت = *(size_t *) معلومات;
      اكسر;

#إذا ABI_FRLEN >= 32
    حالة FFI_TYPE_FLOAT:
      *(عائم *)(cb->aregs->fa + cb->کارول_شوی_float++) = *(عائم *) معلومات;
      أرجع;
#د_که_پای
#إذا ABI_FRLEN >= 64
    حالة FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->کارول_شوی_float++]) = *(مزدوج *) معلومات;
      أرجع;
#د_که_پای
    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }

  إذا (cb->کارول_شوی_integer == NARGREG)
    *cb->کارول_شوی_ډېری++ = ارزښت;
  وإلا
    cb->aregs->a[cb->کارول_شوی_integer++] = ارزښت;
}

ساكن فراغ
unmarshal_atom (call_builder *cb, صحيح type, فراغ *معلومات)
{
  size_t ارزښت;
  بدل (type)
    {
#إذا ABI_FRLEN >= 32
    حالة FFI_TYPE_FLOAT:
      *(عائم *) معلومات = *(عائم *)(cb->aregs->fa + cb->کارول_شوی_float++);
      أرجع;
#د_که_پای
#إذا ABI_FRLEN >= 64
    حالة FFI_TYPE_DOUBLE:
      *(مزدوج *) معلومات = cb->aregs->fa[cb->کارول_شوی_float++];
      أرجع;
#د_که_پای
    }

  إذا (cb->کارول_شوی_integer == NARGREG)
    ارزښت = *cb->کارول_شوی_ډېری++;
  وإلا
    ارزښت = cb->aregs->a[cb->کارول_شوی_integer++];

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
#د_که_پای
    حالة FFI_TYPE_اشاره_ګر:
      *(ffi_arg *)معلومات = ارزښت;
      اكسر;
    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ساكن فراغ *
allocate_and_copy_struct_منزل_ډېری (call_builder *cb, فراغ *معلومات,
				   ffi_type *type)
{
  size_t dest = cb->بل_struct_area - type->اندازه;

  dest = FFI_ALIGN_ښکته (dest, type->alignment);
  cb->بل_struct_area = dest;

  أرجع حافظه_نقل ((حرف *)cb->ډېری + dest, معلومات, type->اندازه);
}

/* Adds an argument to a call, or a not by reference return value.  */
ساكن فراغ
marshal (call_builder *cb, ffi_type *type, صحيح var, فراغ *معلومات)
{
  size_t realign[2];

#إذا ABI_FRLEN
  إذا (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      إذا (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, معلومات);
	  إذا (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((حرف *) معلومات) + fsi.offset2);
	  أرجع;
	}
    }

  إذا (!var && cb->کارول_شوی_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, معلومات);
      أرجع;
    }

  مزدوج promoted;
  إذا (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(عائم *) معلومات;
      type = &ffi_type_double;
      معلومات = &promoted;
    }
#د_که_پای

  إذا (type->اندازه > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_منزل_ډېری (cb, معلومات, type);
      معلومات = (حرف *)cb->ډېری + cb->بل_struct_area;
      marshal_atom (cb, FFI_TYPE_اشاره_ګر, &معلومات);
    }
  وإلا إذا (IS_INT (type->type) || type->type == FFI_TYPE_اشاره_ګر)
    marshal_atom (cb, type->type, معلومات);
  وإلا
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      إذا (type->alignment > __SIZEOF_POINTER__)
	{
	  إذا (var)
	    cb->کارول_شوی_integer = FFI_ALIGN (cb->کارول_شوی_integer, 2);
	  cb->کارول_شوی_ډېری
	    = (size_t *) FFI_ALIGN (cb->کارول_شوی_ډېری, 2 * __SIZEOF_POINTER__);
	}

      حافظه_نقل (realign, معلومات, type->اندازه);
      إذا (type->اندازه > 0)
	marshal_atom (cb, FFI_TYPE_اشاره_ګر, realign);
      إذا (type->اندازه > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_اشاره_ګر, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
ساكن فراغ *
unmarshal (call_builder *cb, ffi_type *type, صحيح var, فراغ *معلومات)
{
  size_t realign[2];
  فراغ *اشاره_ګر;

#إذا ABI_FRLEN
  إذا (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      إذا (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, معلومات);
	  إذا (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((حرف *) معلومات) + fsi.offset2);
	  أرجع معلومات;
	}
    }

  إذا (!var && cb->کارول_شوی_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, معلومات);
      أرجع معلومات;
    }

  إذا (var && type->type == FFI_TYPE_FLOAT)
    {
      صحيح m = cb->کارول_شوی_integer;
      فراغ *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->کارول_شوی_ډېری + m - NARGREG + 1;
      *(عائم *) promoted = *(مزدوج *) promoted;
    }
#د_که_پای

  إذا (type->اندازه > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_اشاره_ګر, (حرف *) &اشاره_ګر);
      أرجع اشاره_ګر;
    }
  وإلا إذا (IS_INT (type->type) || type->type == FFI_TYPE_اشاره_ګر)
    {
      unmarshal_atom (cb, type->type, معلومات);
      أرجع معلومات;
    }
  وإلا
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      إذا (type->alignment > __SIZEOF_POINTER__)
	{
	  إذا (var)
	    cb->کارول_شوی_integer = FFI_ALIGN (cb->کارول_شوی_integer, 2);
	  cb->کارول_شوی_ډېری
	    = (size_t *) FFI_ALIGN (cb->کارول_شوی_ډېری, 2 * __SIZEOF_POINTER__);
	}

      إذا (type->اندازه > 0)
	unmarshal_atom (cb, FFI_TYPE_اشاره_ګر, realign);
      إذا (type->اندازه > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_اشاره_ګر, realign + 1);
      حافظه_نقل (معلومات, realign, type->اندازه);
      أرجع معلومات;
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
	أرجع 0;
    }
#د_که_پای

  أرجع type->اندازه > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_حالت
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  أرجع FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_حالت
ffi_prep_cif_machdep_var (ffi_cif *cif, بینښه صحيح nfixedargs,
			  بینښه صحيح ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  أرجع FFI_OK;
}

/* Low level routine for calling functions.  */
خارجي فراغ ffi_call_asm (فراغ *ډېری, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *تړون) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *تړون)
{
  size_t arg_بایټونه = FFI_ALIGN (cif->بایټونه, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_بایټونه = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * حجم(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_بایټونه = 0;
  إذا (rvalue == عدم && cif->rtype->اندازه > 2 * __SIZEOF_POINTER__)
    rval_بایټونه = FFI_ALIGN (cif->rtype->اندازه, STKALIGN);
  size_t alloc_اندازه = arg_بایټونه + extra_بایټونه + rval_بایټونه + حجم (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_بنسټ;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  إذا (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_بنسټ = (size_t) alloca (alloc_اندازه);
  وإلا
    alloc_بنسټ = FFI_ALIGN (alloca (alloc_اندازه + STKALIGN - 1), STKALIGN);

  إذا (rval_بایټونه)
    rvalue = (فراغ *) (alloc_بنسټ + arg_بایټونه);

  call_builder cb;
  cb.کارول_شوی_float = cb.کارول_شوی_integer = 0;
  cb.aregs = (call_context *) (alloc_بنسټ + arg_بایټونه + extra_بایټونه + rval_بایټونه);
  cb.کارول_شوی_ډېری = (size_t *)((حرف *) alloc_بنسټ);
  cb.ډېری = (size_t *)((حرف *) alloc_بنسټ + extra_بایټونه + rval_بایټونه);
  cb.بل_struct_area = arg_بایټونه;

  صحيح return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  إذا (return_by_ref)
    cb.aregs->a[cb.کارول_شوی_integer++] = (size_t)rvalue;

  صحيح i;
  لكل (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((فراغ *) alloc_بنسټ, cb.aregs, fn, تړون);

  cb.کارول_شوی_float = cb.کارول_شوی_integer = 0;
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
	     فراغ *تړون)
{
  ffi_call_int (cif, fn, rvalue, avalue, تړون);
}

خارجي فراغ ffi_تړون_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون *تړون, ffi_cif *cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_معلومات, فراغ *codeloc)
{
  uint32_t *tramp = (uint32_t *) &تړون->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_تړون_asm;

  إذا (cif->abi <= FFI_لومړی_ABI || cif->abi >= FFI_وروستی_ABI)
    أرجع FFI_BAD_ABI;

#إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(تړون))
    {
      ffi_tramp_سېټ_parms (تړون->ftramp, ffi_تړون_asm, تړون);
      اذهب بهر;
    }
#د_که_پای

  /* Fill the dynamic trampoline.  We will call ffi_closure_inner with codeloc,
     not closure, but as long as the memory is readable it should work.  */
  tramp[0] = 0x1800000c; /* pcaddi $t0, 0 (i.e. $t0 <- tramp) */
  tramp[1] = 0x28c0418d; /* ld.d   $t1, $t0, 16 */
  tramp[2] = 0x4c0001a0; /* jirl   $zero, $t1, 0 */
  tramp[3] = 0x03400000; /* nop */
  tramp[4] = fn;
  tramp[5] = fn >> 32;

  __builtin___clear_cache (codeloc, codeloc + FFI_TRAMPOLINE_اندازه);

#إذا defined(FFI_EXEC_STATIC_TRAMP)
بهر:
#د_که_پای
  تړون->cif = cif;
  تړون->fun = fun;
  تړون->user_معلومات = user_معلومات;

  أرجع FFI_OK;
}

خارجي فراغ ffi_go_تړون_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_تړون (ffi_go_تړون *تړون, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_لومړی_ABI || cif->abi >= FFI_وروستی_ABI)
    أرجع FFI_BAD_ABI;

  تړون->tramp = (فراغ *) ffi_go_تړون_asm;
  تړون->cif = cif;
  تړون->fun = fun;
  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
فراغ FFI_HIDDEN
ffi_تړون_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_معلومات, size_t *ډېری, call_context *aregs)
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
  cb.کارول_شوی_integer = cb.کارول_شوی_float = 0;
  cb.کارول_شوی_ډېری = ډېری;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  إذا (return_by_ref)
    unmarshal (&cb, &ffi_type_اشاره_ګر, 0, &rvalue);
  وإلا
    rvalue = alloca (cif->rtype->اندازه);

  لكل (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_معلومات);

  إذا (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.کارول_شوی_integer = cb.کارول_شوی_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_اندازه, size_t *map_اندازه)
{
  خارجي فراغ *trampoline_کوډ_جدول;

  *tramp_اندازه = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_اندازه = 1 << 16;
  أرجع &trampoline_کوډ_جدول;
}
#د_که_پای
