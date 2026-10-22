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
#غلطی unsupported LoongArch floating-نقطہ ABI
#جے_انت

#تعریف_کرو NARGREG 8
#تعریف_کرو STKALIGN 16
#تعریف_کرو MAXCOPYARG (2 * حجم (مزدوج))

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
#جے_انت
  size_t a[10];
} call_context;

تعريفنوع بنية call_builder
{
  call_context *aregs;
  صحيح ورتیا_integer;
  صحيح ورتیا_float;
  size_t *ورتیا_ڈھیر;	    /* Stack pointer.  */
  size_t *ڈھیر;	    /* Place for arguments pass by reference.  */
  size_t اگلا_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#إذا __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#جے_انت

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
#جے_انت

ساكن ffi_type **
flatten_struct (ffi_type *وچ, ffi_type **باہرلا, ffi_type **باہرلا_سرا)
{
  صحيح i;

  إذا (باہرلا == باہرلا_سرا)
    أرجع باہرلا;
  إذا (وچ->type != FFI_TYPE_STRUCT)
    *(باہرلا++) = وچ;
  وإلا
    لكل (i = 0; وچ->elements[i]; i++)
      باہرلا = flatten_struct (وچ->elements[i], باہرلا, باہرلا_سرا);
  أرجع باہرلا;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
ساكن float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *چوٹی)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  صحيح عدد_floats, عدد_ints;
  صحيح عدد_fields = flatten_struct (چوٹی, fields, fields + 3) - fields;

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
      إذا (cb->ورتیا_float + عدد_floats > NARGREG
	  || cb->ورتیا_integer + (2 - عدد_floats) > NARGREG)
	أرجع ret;
      إذا (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	أرجع ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->ناپ, fields[1]->alignment);
      ret.as_elements = 1;
    }
  أرجع ret;
}
#جے_انت

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
ساكن فراغ
marshal_atom (call_builder *cb, صحيح type, فراغ *انکڑے)
{
  size_t قدر = 0;
  بدل (type)
    {
    حالة FFI_TYPE_UINT8:
      قدر = *(uint8_t *) انکڑے;
      اكسر;
    حالة FFI_TYPE_SINT8:
      قدر = *(int8_t *) انکڑے;
      اكسر;
    حالة FFI_TYPE_UINT16:
      قدر = *(uint16_t *) انکڑے;
      اكسر;
    حالة FFI_TYPE_SINT16:
      قدر = *(int16_t *) انکڑے;
      اكسر;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    حالة FFI_TYPE_UINT32:
      قدر = *(int32_t *) انکڑے;
      اكسر;
    حالة FFI_TYPE_SINT32:
      قدر = *(int32_t *) انکڑے;
      اكسر;
#إذا __SIZEOF_POINTER__ == 8
    حالة FFI_TYPE_UINT64:
      قدر = *(uint64_t *) انکڑے;
      اكسر;
    حالة FFI_TYPE_SINT64:
      قدر = *(int64_t *) انکڑے;
      اكسر;
#جے_انت
    حالة FFI_TYPE_اشارہ_گر:
      قدر = *(size_t *) انکڑے;
      اكسر;

#إذا ABI_FRLEN >= 32
    حالة FFI_TYPE_FLOAT:
      *(عائم *)(cb->aregs->fa + cb->ورتیا_float++) = *(عائم *) انکڑے;
      أرجع;
#جے_انت
#إذا ABI_FRLEN >= 64
    حالة FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->ورتیا_float++]) = *(مزدوج *) انکڑے;
      أرجع;
#جے_انت
    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }

  إذا (cb->ورتیا_integer == NARGREG)
    *cb->ورتیا_ڈھیر++ = قدر;
  وإلا
    cb->aregs->a[cb->ورتیا_integer++] = قدر;
}

ساكن فراغ
unmarshal_atom (call_builder *cb, صحيح type, فراغ *انکڑے)
{
  size_t قدر;
  بدل (type)
    {
#إذا ABI_FRLEN >= 32
    حالة FFI_TYPE_FLOAT:
      *(عائم *) انکڑے = *(عائم *)(cb->aregs->fa + cb->ورتیا_float++);
      أرجع;
#جے_انت
#إذا ABI_FRLEN >= 64
    حالة FFI_TYPE_DOUBLE:
      *(مزدوج *) انکڑے = cb->aregs->fa[cb->ورتیا_float++];
      أرجع;
#جے_انت
    }

  إذا (cb->ورتیا_integer == NARGREG)
    قدر = *cb->ورتیا_ڈھیر++;
  وإلا
    قدر = cb->aregs->a[cb->ورتیا_integer++];

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
#جے_انت
    حالة FFI_TYPE_اشارہ_گر:
      *(ffi_arg *)انکڑے = قدر;
      اكسر;
    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ساكن فراغ *
allocate_and_copy_struct_منزل_ڈھیر (call_builder *cb, فراغ *انکڑے,
				   ffi_type *type)
{
  size_t dest = cb->اگلا_struct_area - type->ناپ;

  dest = FFI_ALIGN_تھلے (dest, type->alignment);
  cb->اگلا_struct_area = dest;

  أرجع حافظہ_نقل ((حرف *)cb->ڈھیر + dest, انکڑے, type->ناپ);
}

/* Adds an argument to a call, or a not by reference return value.  */
ساكن فراغ
marshal (call_builder *cb, ffi_type *type, صحيح var, فراغ *انکڑے)
{
  size_t realign[2];

#إذا ABI_FRLEN
  إذا (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      إذا (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, انکڑے);
	  إذا (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((حرف *) انکڑے) + fsi.offset2);
	  أرجع;
	}
    }

  إذا (!var && cb->ورتیا_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, انکڑے);
      أرجع;
    }

  مزدوج promoted;
  إذا (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(عائم *) انکڑے;
      type = &ffi_type_double;
      انکڑے = &promoted;
    }
#جے_انت

  إذا (type->ناپ > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_منزل_ڈھیر (cb, انکڑے, type);
      انکڑے = (حرف *)cb->ڈھیر + cb->اگلا_struct_area;
      marshal_atom (cb, FFI_TYPE_اشارہ_گر, &انکڑے);
    }
  وإلا إذا (IS_INT (type->type) || type->type == FFI_TYPE_اشارہ_گر)
    marshal_atom (cb, type->type, انکڑے);
  وإلا
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      إذا (type->alignment > __SIZEOF_POINTER__)
	{
	  إذا (var)
	    cb->ورتیا_integer = FFI_ALIGN (cb->ورتیا_integer, 2);
	  cb->ورتیا_ڈھیر
	    = (size_t *) FFI_ALIGN (cb->ورتیا_ڈھیر, 2 * __SIZEOF_POINTER__);
	}

      حافظہ_نقل (realign, انکڑے, type->ناپ);
      إذا (type->ناپ > 0)
	marshal_atom (cb, FFI_TYPE_اشارہ_گر, realign);
      إذا (type->ناپ > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_اشارہ_گر, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
ساكن فراغ *
unmarshal (call_builder *cb, ffi_type *type, صحيح var, فراغ *انکڑے)
{
  size_t realign[2];
  فراغ *اشارہ_گر;

#إذا ABI_FRLEN
  إذا (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      إذا (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, انکڑے);
	  إذا (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((حرف *) انکڑے) + fsi.offset2);
	  أرجع انکڑے;
	}
    }

  إذا (!var && cb->ورتیا_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, انکڑے);
      أرجع انکڑے;
    }

  إذا (var && type->type == FFI_TYPE_FLOAT)
    {
      صحيح m = cb->ورتیا_integer;
      فراغ *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->ورتیا_ڈھیر + m - NARGREG + 1;
      *(عائم *) promoted = *(مزدوج *) promoted;
    }
#جے_انت

  إذا (type->ناپ > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_اشارہ_گر, (حرف *) &اشارہ_گر);
      أرجع اشارہ_گر;
    }
  وإلا إذا (IS_INT (type->type) || type->type == FFI_TYPE_اشارہ_گر)
    {
      unmarshal_atom (cb, type->type, انکڑے);
      أرجع انکڑے;
    }
  وإلا
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      إذا (type->alignment > __SIZEOF_POINTER__)
	{
	  إذا (var)
	    cb->ورتیا_integer = FFI_ALIGN (cb->ورتیا_integer, 2);
	  cb->ورتیا_ڈھیر
	    = (size_t *) FFI_ALIGN (cb->ورتیا_ڈھیر, 2 * __SIZEOF_POINTER__);
	}

      إذا (type->ناپ > 0)
	unmarshal_atom (cb, FFI_TYPE_اشارہ_گر, realign);
      إذا (type->ناپ > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_اشارہ_گر, realign + 1);
      حافظہ_نقل (انکڑے, realign, type->ناپ);
      أرجع انکڑے;
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
#جے_انت

  أرجع type->ناپ > 2 * __SIZEOF_POINTER__;
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
ffi_prep_cif_machdep_var (ffi_cif *cif, بنانشانی صحيح nfixedargs,
			  بنانشانی صحيح ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  أرجع FFI_OK;
}

/* Low level routine for calling functions.  */
خارجي فراغ ffi_call_asm (فراغ *ڈھیر, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *گھیرا) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *گھیرا)
{
  size_t arg_بائٹاں = FFI_ALIGN (cif->بائٹاں, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_بائٹاں = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * حجم(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_بائٹاں = 0;
  إذا (rvalue == عدم && cif->rtype->ناپ > 2 * __SIZEOF_POINTER__)
    rval_بائٹاں = FFI_ALIGN (cif->rtype->ناپ, STKALIGN);
  size_t alloc_ناپ = arg_بائٹاں + extra_بائٹاں + rval_بائٹاں + حجم (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_ادھار;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  إذا (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_ادھار = (size_t) alloca (alloc_ناپ);
  وإلا
    alloc_ادھار = FFI_ALIGN (alloca (alloc_ناپ + STKALIGN - 1), STKALIGN);

  إذا (rval_بائٹاں)
    rvalue = (فراغ *) (alloc_ادھار + arg_بائٹاں);

  call_builder cb;
  cb.ورتیا_float = cb.ورتیا_integer = 0;
  cb.aregs = (call_context *) (alloc_ادھار + arg_بائٹاں + extra_بائٹاں + rval_بائٹاں);
  cb.ورتیا_ڈھیر = (size_t *)((حرف *) alloc_ادھار);
  cb.ڈھیر = (size_t *)((حرف *) alloc_ادھار + extra_بائٹاں + rval_بائٹاں);
  cb.اگلا_struct_area = arg_بائٹاں;

  صحيح return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  إذا (return_by_ref)
    cb.aregs->a[cb.ورتیا_integer++] = (size_t)rvalue;

  صحيح i;
  لكل (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((فراغ *) alloc_ادھار, cb.aregs, fn, گھیرا);

  cb.ورتیا_float = cb.ورتیا_integer = 0;
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
	     فراغ *گھیرا)
{
  ffi_call_int (cif, fn, rvalue, avalue, گھیرا);
}

خارجي فراغ ffi_گھیرا_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا *گھیرا, ffi_cif *cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_انکڑے, فراغ *codeloc)
{
  uint32_t *tramp = (uint32_t *) &گھیرا->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_گھیرا_asm;

  إذا (cif->abi <= FFI_پہلا_ABI || cif->abi >= FFI_آخری_ABI)
    أرجع FFI_BAD_ABI;

#إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(گھیرا))
    {
      ffi_tramp_سیٹ_parms (گھیرا->ftramp, ffi_گھیرا_asm, گھیرا);
      اذهب باہرلا;
    }
#جے_انت

  /* Fill the dynamic trampoline.  We will call ffi_closure_inner with codeloc,
     not closure, but as long as the memory is readable it should work.  */
  tramp[0] = 0x1800000c; /* pcaddi $t0, 0 (i.e. $t0 <- tramp) */
  tramp[1] = 0x28c0418d; /* ld.d   $t1, $t0, 16 */
  tramp[2] = 0x4c0001a0; /* jirl   $zero, $t1, 0 */
  tramp[3] = 0x03400000; /* nop */
  tramp[4] = fn;
  tramp[5] = fn >> 32;

  __builtin___clear_cache (codeloc, codeloc + FFI_TRAMPOLINE_ناپ);

#إذا defined(FFI_EXEC_STATIC_TRAMP)
باہرلا:
#جے_انت
  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;

  أرجع FFI_OK;
}

خارجي فراغ ffi_go_گھیرا_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_گھیرا (ffi_go_گھیرا *گھیرا, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_پہلا_ABI || cif->abi >= FFI_آخری_ABI)
    أرجع FFI_BAD_ABI;

  گھیرا->tramp = (فراغ *) ffi_go_گھیرا_asm;
  گھیرا->cif = cif;
  گھیرا->fun = fun;
  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
فراغ FFI_HIDDEN
ffi_گھیرا_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_انکڑے, size_t *ڈھیر, call_context *aregs)
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
  cb.ورتیا_integer = cb.ورتیا_float = 0;
  cb.ورتیا_ڈھیر = ڈھیر;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  إذا (return_by_ref)
    unmarshal (&cb, &ffi_type_اشارہ_گر, 0, &rvalue);
  وإلا
    rvalue = alloca (cif->rtype->ناپ);

  لكل (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_انکڑے);

  إذا (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.ورتیا_integer = cb.ورتیا_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_ناپ, size_t *map_ناپ)
{
  خارجي فراغ *trampoline_کوڈ_جدول;

  *tramp_ناپ = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_ناپ = 1 << 16;
  أرجع &trampoline_کوڈ_جدول;
}
#جے_انت
