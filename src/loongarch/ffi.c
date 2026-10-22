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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <tramp.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>

#यदि defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#नोचेद्यदि defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT भिन्न
#नोचेद्यदि defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT द्विगुण
#अन्यथा
#दोषः unsupported LoongArch floating-point ABI
#यदि_अन्तः

#परिभाषय NARGREG 8
#परिभाषय STKALIGN 16
#परिभाषय MAXCOPYARG (2 * परिमाण (द्विगुण))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
प्रकारनिर्देश संरचना call_context
{
#यदि !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#यदि_अन्तः
  size_t a[10];
} call_context;

प्रकारनिर्देश संरचना call_builder
{
  call_context *aregs;
  पूर्णाङ्क प्रयुक्तम्_integer;
  पूर्णाङ्क प्रयुक्तम्_float;
  size_t *प्रयुक्तम्_राशिः;	    /* Stack pointer.  */
  size_t *राशिः;	    /* Place for arguments pass by reference.  */
  size_t अग्र_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#यदि __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अन्यथा
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#यदि_अन्तः

#यदि ABI_FRLEN
प्रकारनिर्देश संरचना float_struct_info
{
  वर्ण as_elements;
  वर्ण type1;
  वर्ण offset2;
  वर्ण type2;
} float_struct_info;

#यदि ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#अन्यथा
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#यदि_अन्तः

स्थिर ffi_type **
flatten_struct (ffi_type *अन्तर्, ffi_type **बहिर्गमनम्, ffi_type **बहिर्गमनम्_अन्तः)
{
  पूर्णाङ्क i;

  यदि (बहिर्गमनम् == बहिर्गमनम्_अन्तः)
    प्रत्यागम बहिर्गमनम्;
  यदि (अन्तर्->type != FFI_TYPE_STRUCT)
    *(बहिर्गमनम्++) = अन्तर्;
  अन्यथा
    प्रति (i = 0; अन्तर्->elements[i]; i++)
      बहिर्गमनम् = flatten_struct (अन्तर्->elements[i], बहिर्गमनम्, बहिर्गमनम्_अन्तः);
  प्रत्यागम बहिर्गमनम्;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
स्थिर float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *शिखरम्)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  पूर्णाङ्क अङ्कः_floats, अङ्कः_ints;
  पूर्णाङ्क अङ्कः_fields = flatten_struct (शिखरम्, fields, fields + 3) - fields;

  यदि (अङ्कः_fields == 1)
    {
      यदि (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  अन्यथा यदि (अङ्कः_fields == 2)
    {
      अङ्कः_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      अङ्कः_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      यदि (अङ्कः_floats == 0 || अङ्कः_floats + अङ्कः_ints != 2)
	प्रत्यागम ret;
      यदि (cb->प्रयुक्तम्_float + अङ्कः_floats > NARGREG
	  || cb->प्रयुक्तम्_integer + (2 - अङ्कः_floats) > NARGREG)
	प्रत्यागम ret;
      यदि (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	प्रत्यागम ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->आकार, fields[1]->alignment);
      ret.as_elements = 1;
    }
  प्रत्यागम ret;
}
#यदि_अन्तः

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
स्थिर शून्य
marshal_atom (call_builder *cb, पूर्णाङ्क type, शून्य *दत्त)
{
  size_t मानम् = 0;
  विकल्प (type)
    {
    पक्ष FFI_TYPE_UINT8:
      मानम् = *(uint8_t *) दत्त;
      भङ्ग;
    पक्ष FFI_TYPE_SINT8:
      मानम् = *(int8_t *) दत्त;
      भङ्ग;
    पक्ष FFI_TYPE_UINT16:
      मानम् = *(uint16_t *) दत्त;
      भङ्ग;
    पक्ष FFI_TYPE_SINT16:
      मानम् = *(int16_t *) दत्त;
      भङ्ग;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    पक्ष FFI_TYPE_UINT32:
      मानम् = *(int32_t *) दत्त;
      भङ्ग;
    पक्ष FFI_TYPE_SINT32:
      मानम् = *(int32_t *) दत्त;
      भङ्ग;
#यदि __SIZEOF_POINTER__ == 8
    पक्ष FFI_TYPE_UINT64:
      मानम् = *(uint64_t *) दत्त;
      भङ्ग;
    पक्ष FFI_TYPE_SINT64:
      मानम् = *(int64_t *) दत्त;
      भङ्ग;
#यदि_अन्तः
    पक्ष FFI_TYPE_POINTER:
      मानम् = *(size_t *) दत्त;
      भङ्ग;

#यदि ABI_FRLEN >= 32
    पक्ष FFI_TYPE_FLOAT:
      *(भिन्न *)(cb->aregs->fa + cb->प्रयुक्तम्_float++) = *(भिन्न *) दत्त;
      प्रत्यागम;
#यदि_अन्तः
#यदि ABI_FRLEN >= 64
    पक्ष FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->प्रयुक्तम्_float++]) = *(द्विगुण *) दत्त;
      प्रत्यागम;
#यदि_अन्तः
    स्वभाव:
      FFI_ASSERT (0);
      भङ्ग;
    }

  यदि (cb->प्रयुक्तम्_integer == NARGREG)
    *cb->प्रयुक्तम्_राशिः++ = मानम्;
  अन्यथा
    cb->aregs->a[cb->प्रयुक्तम्_integer++] = मानम्;
}

स्थिर शून्य
unmarshal_atom (call_builder *cb, पूर्णाङ्क type, शून्य *दत्त)
{
  size_t मानम्;
  विकल्प (type)
    {
#यदि ABI_FRLEN >= 32
    पक्ष FFI_TYPE_FLOAT:
      *(भिन्न *) दत्त = *(भिन्न *)(cb->aregs->fa + cb->प्रयुक्तम्_float++);
      प्रत्यागम;
#यदि_अन्तः
#यदि ABI_FRLEN >= 64
    पक्ष FFI_TYPE_DOUBLE:
      *(द्विगुण *) दत्त = cb->aregs->fa[cb->प्रयुक्तम्_float++];
      प्रत्यागम;
#यदि_अन्तः
    }

  यदि (cb->प्रयुक्तम्_integer == NARGREG)
    मानम् = *cb->प्रयुक्तम्_राशिः++;
  अन्यथा
    मानम् = cb->aregs->a[cb->प्रयुक्तम्_integer++];

  विकल्प (type)
    {
    पक्ष FFI_TYPE_UINT8:
    पक्ष FFI_TYPE_SINT8:
    पक्ष FFI_TYPE_UINT16:
    पक्ष FFI_TYPE_SINT16:
    पक्ष FFI_TYPE_UINT32:
    पक्ष FFI_TYPE_SINT32:
#यदि __SIZEOF_POINTER__ == 8
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
#यदि_अन्तः
    पक्ष FFI_TYPE_POINTER:
      *(ffi_arg *)दत्त = मानम्;
      भङ्ग;
    स्वभाव:
      FFI_ASSERT (0);
      भङ्ग;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
स्थिर शून्य *
allocate_and_copy_struct_गन्तव्यम्_राशिः (call_builder *cb, शून्य *दत्त,
				   ffi_type *type)
{
  size_t dest = cb->अग्र_struct_area - type->आकार;

  dest = FFI_ALIGN_अधस्तात् (dest, type->alignment);
  cb->अग्र_struct_area = dest;

  प्रत्यागम स्मृतिं_प्रतिलिख ((वर्ण *)cb->राशिः + dest, दत्त, type->आकार);
}

/* Adds an argument to a call, or a not by reference return value.  */
स्थिर शून्य
marshal (call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *दत्त)
{
  size_t realign[2];

#यदि ABI_FRLEN
  यदि (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      यदि (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, दत्त);
	  यदि (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((वर्ण *) दत्त) + fsi.offset2);
	  प्रत्यागम;
	}
    }

  यदि (!var && cb->प्रयुक्तम्_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, दत्त);
      प्रत्यागम;
    }

  द्विगुण promoted;
  यदि (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(भिन्न *) दत्त;
      type = &ffi_type_double;
      दत्त = &promoted;
    }
#यदि_अन्तः

  यदि (type->आकार > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_गन्तव्यम्_राशिः (cb, दत्त, type);
      दत्त = (वर्ण *)cb->राशिः + cb->अग्र_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &दत्त);
    }
  अन्यथा यदि (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, दत्त);
  अन्यथा
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      यदि (type->alignment > __SIZEOF_POINTER__)
	{
	  यदि (var)
	    cb->प्रयुक्तम्_integer = FFI_ALIGN (cb->प्रयुक्तम्_integer, 2);
	  cb->प्रयुक्तम्_राशिः
	    = (size_t *) FFI_ALIGN (cb->प्रयुक्तम्_राशिः, 2 * __SIZEOF_POINTER__);
	}

      स्मृतिं_प्रतिलिख (realign, दत्त, type->आकार);
      यदि (type->आकार > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      यदि (type->आकार > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
स्थिर शून्य *
unmarshal (call_builder *cb, ffi_type *type, पूर्णाङ्क var, शून्य *दत्त)
{
  size_t realign[2];
  शून्य *pointer;

#यदि ABI_FRLEN
  यदि (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      यदि (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, दत्त);
	  यदि (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((वर्ण *) दत्त) + fsi.offset2);
	  प्रत्यागम दत्त;
	}
    }

  यदि (!var && cb->प्रयुक्तम्_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, दत्त);
      प्रत्यागम दत्त;
    }

  यदि (var && type->type == FFI_TYPE_FLOAT)
    {
      पूर्णाङ्क m = cb->प्रयुक्तम्_integer;
      शून्य *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->प्रयुक्तम्_राशिः + m - NARGREG + 1;
      *(भिन्न *) promoted = *(द्विगुण *) promoted;
    }
#यदि_अन्तः

  यदि (type->आकार > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (वर्ण *) &pointer);
      प्रत्यागम pointer;
    }
  अन्यथा यदि (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, दत्त);
      प्रत्यागम दत्त;
    }
  अन्यथा
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      यदि (type->alignment > __SIZEOF_POINTER__)
	{
	  यदि (var)
	    cb->प्रयुक्तम्_integer = FFI_ALIGN (cb->प्रयुक्तम्_integer, 2);
	  cb->प्रयुक्तम्_राशिः
	    = (size_t *) FFI_ALIGN (cb->प्रयुक्तम्_राशिः, 2 * __SIZEOF_POINTER__);
	}

      यदि (type->आकार > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      यदि (type->आकार > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      स्मृतिं_प्रतिलिख (दत्त, realign, type->आकार);
      प्रत्यागम दत्त;
    }
}

स्थिर पूर्णाङ्क
passed_by_ref (call_builder *cb, ffi_type *type, पूर्णाङ्क var)
{
#यदि ABI_FRLEN
  यदि (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      यदि (fsi.as_elements)
	प्रत्यागम 0;
    }
#यदि_अन्तः

  प्रत्यागम type->आकार > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_स्थितिः
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  प्रत्यागम FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_स्थितिः
ffi_prep_cif_machdep_var (ffi_cif *cif, अचिह्नित पूर्णाङ्क nfixedargs,
			  अचिह्नित पूर्णाङ्क ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  प्रत्यागम FFI_OK;
}

/* Low level routine for calling functions.  */
बाह्य शून्य ffi_call_asm (शून्य *राशिः, संरचना call_context *regs,
			  शून्य (*fn) (शून्य), शून्य *संवृतिः) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	      शून्य *संवृतिः)
{
  size_t arg_अष्टकानि = FFI_ALIGN (cif->अष्टकानि, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_अष्टकानि = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * परिमाण(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_अष्टकानि = 0;
  यदि (rvalue == रिक्त && cif->rtype->आकार > 2 * __SIZEOF_POINTER__)
    rval_अष्टकानि = FFI_ALIGN (cif->rtype->आकार, STKALIGN);
  size_t alloc_आकार = arg_अष्टकानि + extra_अष्टकानि + rval_अष्टकानि + परिमाण (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_आधारः;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  यदि (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_आधारः = (size_t) alloca (alloc_आकार);
  अन्यथा
    alloc_आधारः = FFI_ALIGN (alloca (alloc_आकार + STKALIGN - 1), STKALIGN);

  यदि (rval_अष्टकानि)
    rvalue = (शून्य *) (alloc_आधारः + arg_अष्टकानि);

  call_builder cb;
  cb.प्रयुक्तम्_float = cb.प्रयुक्तम्_integer = 0;
  cb.aregs = (call_context *) (alloc_आधारः + arg_अष्टकानि + extra_अष्टकानि + rval_अष्टकानि);
  cb.प्रयुक्तम्_राशिः = (size_t *)((वर्ण *) alloc_आधारः);
  cb.राशिः = (size_t *)((वर्ण *) alloc_आधारः + extra_अष्टकानि + rval_अष्टकानि);
  cb.अग्र_struct_area = arg_अष्टकानि;

  पूर्णाङ्क return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  यदि (return_by_ref)
    cb.aregs->a[cb.प्रयुक्तम्_integer++] = (size_t)rvalue;

  पूर्णाङ्क i;
  प्रति (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((शून्य *) alloc_आधारः, cb.aregs, fn, संवृतिः);

  cb.प्रयुक्तम्_float = cb.प्रयुक्तम्_integer = 0;
  यदि (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	     शून्य *संवृतिः)
{
  ffi_call_int (cif, fn, rvalue, avalue, संवृतिः);
}

बाह्य शून्य ffi_संवृतिः_asm (शून्य) FFI_HIDDEN;

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः *संवृतिः, ffi_cif *cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_दत्त, शून्य *codeloc)
{
  uint32_t *tramp = (uint32_t *) &संवृतिः->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_संवृतिः_asm;

  यदि (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_चरमः_ABI)
    प्रत्यागम FFI_BAD_ABI;

#यदि defined(FFI_EXEC_STATIC_TRAMP)
  यदि (ffi_tramp_is_present(संवृतिः))
    {
      ffi_tramp_समुच्चयः_parms (संवृतिः->ftramp, ffi_संवृतिः_asm, संवृतिः);
      गच्छ बहिर्गमनम्;
    }
#यदि_अन्तः

  /* Fill the dynamic trampoline.  We will call ffi_closure_inner with codeloc,
     not closure, but as long as the memory is readable it should work.  */
  tramp[0] = 0x1800000c; /* pcaddi $t0, 0 (i.e. $t0 <- tramp) */
  tramp[1] = 0x28c0418d; /* ld.d   $t1, $t0, 16 */
  tramp[2] = 0x4c0001a0; /* jirl   $zero, $t1, 0 */
  tramp[3] = 0x03400000; /* nop */
  tramp[4] = fn;
  tramp[5] = fn >> 32;

  __builtin___clear_cache (codeloc, codeloc + FFI_TRAMPOLINE_आकार);

#यदि defined(FFI_EXEC_STATIC_TRAMP)
बहिर्गमनम्:
#यदि_अन्तः
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;

  प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_go_संवृतिः_asm (शून्य) FFI_HIDDEN;

ffi_स्थितिः
ffi_prep_go_संवृतिः (ffi_go_संवृतिः *संवृतिः, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  यदि (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_चरमः_ABI)
    प्रत्यागम FFI_BAD_ABI;

  संवृतिः->tramp = (शून्य *) ffi_go_संवृतिः_asm;
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  प्रत्यागम FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
शून्य FFI_HIDDEN
ffi_संवृतिः_inner (ffi_cif *cif,
		   शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		   शून्य *user_दत्त, size_t *राशिः, call_context *aregs)
{
  शून्य **avalue = alloca (cif->nargs * परिमाण (शून्य *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  वर्ण *astorage = alloca (cif->nargs * MAXCOPYARG);
  शून्य *rvalue;
  call_builder cb;
  पूर्णाङ्क return_by_ref;
  पूर्णाङ्क i;

  cb.aregs = aregs;
  cb.प्रयुक्तम्_integer = cb.प्रयुक्तम्_float = 0;
  cb.प्रयुक्तम्_राशिः = राशिः;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  यदि (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  अन्यथा
    rvalue = alloca (cif->rtype->आकार);

  प्रति (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_दत्त);

  यदि (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.प्रयुक्तम्_integer = cb.प्रयुक्तम्_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#यदि defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (size_t *tramp_आकार, size_t *map_आकार)
{
  बाह्य शून्य *trampoline_संहिता_सारणी;

  *tramp_आकार = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_आकार = 1 << 16;
  प्रत्यागम &trampoline_संहिता_सारणी;
}
#यदि_अन्तः
