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
#غلطي unsupported LoongArch floating-نقطو ABI
#جيڪڏهن_پورو

#وصف_ڪريو NARGREG 8
#وصف_ڪريو STKALIGN 16
#وصف_ڪريو MAXCOPYARG (2 * حجم (مزدوج))

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
#جيڪڏهن_پورو
  size_t a[10];
} call_context;

تعريفنوع بنية call_builder
{
  call_context *aregs;
  صحيح استعمال_ٿيل_integer;
  صحيح استعمال_ٿيل_float;
  size_t *استعمال_ٿيل_ڍير;	    /* Stack pointer.  */
  size_t *ڍير;	    /* Place for arguments pass by reference.  */
  size_t ايندڙ_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#إذا __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#وإلا
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#جيڪڏهن_پورو

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
#جيڪڏهن_پورو

ساكن ffi_type **
flatten_struct (ffi_type *اندريون, ffi_type **ٻاهريون, ffi_type **ٻاهريون_پڇاڙي)
{
  صحيح i;

  إذا (ٻاهريون == ٻاهريون_پڇاڙي)
    أرجع ٻاهريون;
  إذا (اندريون->type != FFI_TYPE_STRUCT)
    *(ٻاهريون++) = اندريون;
  وإلا
    لكل (i = 0; اندريون->elements[i]; i++)
      ٻاهريون = flatten_struct (اندريون->elements[i], ٻاهريون, ٻاهريون_پڇاڙي);
  أرجع ٻاهريون;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
ساكن float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *چوٽي)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  صحيح انگ_floats, انگ_ints;
  صحيح انگ_fields = flatten_struct (چوٽي, fields, fields + 3) - fields;

  إذا (انگ_fields == 1)
    {
      إذا (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  وإلا إذا (انگ_fields == 2)
    {
      انگ_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      انگ_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      إذا (انگ_floats == 0 || انگ_floats + انگ_ints != 2)
	أرجع ret;
      إذا (cb->استعمال_ٿيل_float + انگ_floats > NARGREG
	  || cb->استعمال_ٿيل_integer + (2 - انگ_floats) > NARGREG)
	أرجع ret;
      إذا (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	أرجع ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->سائيز, fields[1]->alignment);
      ret.as_elements = 1;
    }
  أرجع ret;
}
#جيڪڏهن_پورو

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
ساكن فراغ
marshal_atom (call_builder *cb, صحيح type, فراغ *معلومات)
{
  size_t قدر = 0;
  بدل (type)
    {
    حالة FFI_TYPE_UINT8:
      قدر = *(uint8_t *) معلومات;
      اكسر;
    حالة FFI_TYPE_SINT8:
      قدر = *(int8_t *) معلومات;
      اكسر;
    حالة FFI_TYPE_UINT16:
      قدر = *(uint16_t *) معلومات;
      اكسر;
    حالة FFI_TYPE_SINT16:
      قدر = *(int16_t *) معلومات;
      اكسر;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    حالة FFI_TYPE_UINT32:
      قدر = *(int32_t *) معلومات;
      اكسر;
    حالة FFI_TYPE_SINT32:
      قدر = *(int32_t *) معلومات;
      اكسر;
#إذا __SIZEOF_POINTER__ == 8
    حالة FFI_TYPE_UINT64:
      قدر = *(uint64_t *) معلومات;
      اكسر;
    حالة FFI_TYPE_SINT64:
      قدر = *(int64_t *) معلومات;
      اكسر;
#جيڪڏهن_پورو
    حالة FFI_TYPE_اشارو:
      قدر = *(size_t *) معلومات;
      اكسر;

#إذا ABI_FRLEN >= 32
    حالة FFI_TYPE_FLOAT:
      *(عائم *)(cb->aregs->fa + cb->استعمال_ٿيل_float++) = *(عائم *) معلومات;
      أرجع;
#جيڪڏهن_پورو
#إذا ABI_FRLEN >= 64
    حالة FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->استعمال_ٿيل_float++]) = *(مزدوج *) معلومات;
      أرجع;
#جيڪڏهن_پورو
    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }

  إذا (cb->استعمال_ٿيل_integer == NARGREG)
    *cb->استعمال_ٿيل_ڍير++ = قدر;
  وإلا
    cb->aregs->a[cb->استعمال_ٿيل_integer++] = قدر;
}

ساكن فراغ
unmarshal_atom (call_builder *cb, صحيح type, فراغ *معلومات)
{
  size_t قدر;
  بدل (type)
    {
#إذا ABI_FRLEN >= 32
    حالة FFI_TYPE_FLOAT:
      *(عائم *) معلومات = *(عائم *)(cb->aregs->fa + cb->استعمال_ٿيل_float++);
      أرجع;
#جيڪڏهن_پورو
#إذا ABI_FRLEN >= 64
    حالة FFI_TYPE_DOUBLE:
      *(مزدوج *) معلومات = cb->aregs->fa[cb->استعمال_ٿيل_float++];
      أرجع;
#جيڪڏهن_پورو
    }

  إذا (cb->استعمال_ٿيل_integer == NARGREG)
    قدر = *cb->استعمال_ٿيل_ڍير++;
  وإلا
    قدر = cb->aregs->a[cb->استعمال_ٿيل_integer++];

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
#جيڪڏهن_پورو
    حالة FFI_TYPE_اشارو:
      *(ffi_arg *)معلومات = قدر;
      اكسر;
    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ساكن فراغ *
allocate_and_copy_struct_منزل_ڍير (call_builder *cb, فراغ *معلومات,
				   ffi_type *type)
{
  size_t dest = cb->ايندڙ_struct_area - type->سائيز;

  dest = FFI_ALIGN_هيٺ_ڏانهن (dest, type->alignment);
  cb->ايندڙ_struct_area = dest;

  أرجع يادگيري_نقل ((حرف *)cb->ڍير + dest, معلومات, type->سائيز);
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

  إذا (!var && cb->استعمال_ٿيل_float < NARGREG
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
#جيڪڏهن_پورو

  إذا (type->سائيز > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_منزل_ڍير (cb, معلومات, type);
      معلومات = (حرف *)cb->ڍير + cb->ايندڙ_struct_area;
      marshal_atom (cb, FFI_TYPE_اشارو, &معلومات);
    }
  وإلا إذا (IS_INT (type->type) || type->type == FFI_TYPE_اشارو)
    marshal_atom (cb, type->type, معلومات);
  وإلا
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      إذا (type->alignment > __SIZEOF_POINTER__)
	{
	  إذا (var)
	    cb->استعمال_ٿيل_integer = FFI_ALIGN (cb->استعمال_ٿيل_integer, 2);
	  cb->استعمال_ٿيل_ڍير
	    = (size_t *) FFI_ALIGN (cb->استعمال_ٿيل_ڍير, 2 * __SIZEOF_POINTER__);
	}

      يادگيري_نقل (realign, معلومات, type->سائيز);
      إذا (type->سائيز > 0)
	marshal_atom (cb, FFI_TYPE_اشارو, realign);
      إذا (type->سائيز > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_اشارو, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
ساكن فراغ *
unmarshal (call_builder *cb, ffi_type *type, صحيح var, فراغ *معلومات)
{
  size_t realign[2];
  فراغ *اشارو;

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

  إذا (!var && cb->استعمال_ٿيل_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, معلومات);
      أرجع معلومات;
    }

  إذا (var && type->type == FFI_TYPE_FLOAT)
    {
      صحيح m = cb->استعمال_ٿيل_integer;
      فراغ *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->استعمال_ٿيل_ڍير + m - NARGREG + 1;
      *(عائم *) promoted = *(مزدوج *) promoted;
    }
#جيڪڏهن_پورو

  إذا (type->سائيز > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_اشارو, (حرف *) &اشارو);
      أرجع اشارو;
    }
  وإلا إذا (IS_INT (type->type) || type->type == FFI_TYPE_اشارو)
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
	    cb->استعمال_ٿيل_integer = FFI_ALIGN (cb->استعمال_ٿيل_integer, 2);
	  cb->استعمال_ٿيل_ڍير
	    = (size_t *) FFI_ALIGN (cb->استعمال_ٿيل_ڍير, 2 * __SIZEOF_POINTER__);
	}

      إذا (type->سائيز > 0)
	unmarshal_atom (cb, FFI_TYPE_اشارو, realign);
      إذا (type->سائيز > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_اشارو, realign + 1);
      يادگيري_نقل (معلومات, realign, type->سائيز);
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
#جيڪڏهن_پورو

  أرجع type->سائيز > 2 * __SIZEOF_POINTER__;
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
ffi_prep_cif_machdep_var (ffi_cif *cif, بنانشان صحيح nfixedargs,
			  بنانشان صحيح ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  أرجع FFI_OK;
}

/* Low level routine for calling functions.  */
خارجي فراغ ffi_call_asm (فراغ *ڍير, بنية call_context *regs,
			  فراغ (*fn) (فراغ), فراغ *گهيرو) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	      فراغ *گهيرو)
{
  size_t arg_بائيٽون = FFI_ALIGN (cif->بائيٽون, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_بائيٽون = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * حجم(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_بائيٽون = 0;
  إذا (rvalue == عدم && cif->rtype->سائيز > 2 * __SIZEOF_POINTER__)
    rval_بائيٽون = FFI_ALIGN (cif->rtype->سائيز, STKALIGN);
  size_t alloc_سائيز = arg_بائيٽون + extra_بائيٽون + rval_بائيٽون + حجم (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_بنياد;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  إذا (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_بنياد = (size_t) alloca (alloc_سائيز);
  وإلا
    alloc_بنياد = FFI_ALIGN (alloca (alloc_سائيز + STKALIGN - 1), STKALIGN);

  إذا (rval_بائيٽون)
    rvalue = (فراغ *) (alloc_بنياد + arg_بائيٽون);

  call_builder cb;
  cb.استعمال_ٿيل_float = cb.استعمال_ٿيل_integer = 0;
  cb.aregs = (call_context *) (alloc_بنياد + arg_بائيٽون + extra_بائيٽون + rval_بائيٽون);
  cb.استعمال_ٿيل_ڍير = (size_t *)((حرف *) alloc_بنياد);
  cb.ڍير = (size_t *)((حرف *) alloc_بنياد + extra_بائيٽون + rval_بائيٽون);
  cb.ايندڙ_struct_area = arg_بائيٽون;

  صحيح return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  إذا (return_by_ref)
    cb.aregs->a[cb.استعمال_ٿيل_integer++] = (size_t)rvalue;

  صحيح i;
  لكل (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((فراغ *) alloc_بنياد, cb.aregs, fn, گهيرو);

  cb.استعمال_ٿيل_float = cb.استعمال_ٿيل_integer = 0;
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
	     فراغ *گهيرو)
{
  ffi_call_int (cif, fn, rvalue, avalue, گهيرو);
}

خارجي فراغ ffi_گهيرو_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو *گهيرو, ffi_cif *cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_معلومات, فراغ *codeloc)
{
  uint32_t *tramp = (uint32_t *) &گهيرو->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_گهيرو_asm;

  إذا (cif->abi <= FFI_پهريون_ABI || cif->abi >= FFI_آخري_ABI)
    أرجع FFI_BAD_ABI;

#إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(گهيرو))
    {
      ffi_tramp_سيٽ_parms (گهيرو->ftramp, ffi_گهيرو_asm, گهيرو);
      اذهب ٻاهريون;
    }
#جيڪڏهن_پورو

  /* Fill the dynamic trampoline.  We will call ffi_closure_inner with codeloc,
     not closure, but as long as the memory is readable it should work.  */
  tramp[0] = 0x1800000c; /* pcaddi $t0, 0 (i.e. $t0 <- tramp) */
  tramp[1] = 0x28c0418d; /* ld.d   $t1, $t0, 16 */
  tramp[2] = 0x4c0001a0; /* jirl   $zero, $t1, 0 */
  tramp[3] = 0x03400000; /* nop */
  tramp[4] = fn;
  tramp[5] = fn >> 32;

  __builtin___clear_cache (codeloc, codeloc + FFI_TRAMPOLINE_سائيز);

#إذا defined(FFI_EXEC_STATIC_TRAMP)
ٻاهريون:
#جيڪڏهن_پورو
  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

  أرجع FFI_OK;
}

خارجي فراغ ffi_go_گهيرو_asm (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_گهيرو (ffi_go_گهيرو *گهيرو, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  إذا (cif->abi <= FFI_پهريون_ABI || cif->abi >= FFI_آخري_ABI)
    أرجع FFI_BAD_ABI;

  گهيرو->tramp = (فراغ *) ffi_go_گهيرو_asm;
  گهيرو->cif = cif;
  گهيرو->fun = fun;
  أرجع FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
فراغ FFI_HIDDEN
ffi_گهيرو_inner (ffi_cif *cif,
		   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		   فراغ *user_معلومات, size_t *ڍير, call_context *aregs)
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
  cb.استعمال_ٿيل_integer = cb.استعمال_ٿيل_float = 0;
  cb.استعمال_ٿيل_ڍير = ڍير;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  إذا (return_by_ref)
    unmarshal (&cb, &ffi_type_اشارو, 0, &rvalue);
  وإلا
    rvalue = alloca (cif->rtype->سائيز);

  لكل (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_معلومات);

  إذا (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.استعمال_ٿيل_integer = cb.استعمال_ٿيل_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_سائيز, size_t *map_سائيز)
{
  خارجي فراغ *trampoline_ڪوڊ_جدول;

  *tramp_سائيز = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_سائيز = 1 << 16;
  أرجع &trampoline_ڪوڊ_جدول;
}
#جيڪڏهن_پورو
