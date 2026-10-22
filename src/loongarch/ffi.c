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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <tramp.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdint.h>

#ఒకవేళ defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#లేదావేళ defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT దశాంశం
#లేదావేళ defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT ద్విగుణ
#లేకపోతే
#దోషం unsupported LoongArch floating-బిందువు ABI
#అయితే_ముగింపు

#నిర్వచించు NARGREG 8
#నిర్వచించు STKALIGN 16
#నిర్వచించు MAXCOPYARG (2 * పరిమాణం (ద్విగుణ))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
రకనిర్వచనం నిర్మాణం call_context
{
#ఒకవేళ !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#అయితే_ముగింపు
  size_t a[10];
} call_context;

రకనిర్వచనం నిర్మాణం call_builder
{
  call_context *aregs;
  పూర్ణాంకం వాడిన_integer;
  పూర్ణాంకం వాడిన_float;
  size_t *వాడిన_దొంతర;	    /* Stack pointer.  */
  size_t *దొంతర;	    /* Place for arguments pass by reference.  */
  size_t తదుపరి_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#ఒకవేళ __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#లేకపోతే
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#అయితే_ముగింపు

#ఒకవేళ ABI_FRLEN
రకనిర్వచనం నిర్మాణం float_struct_info
{
  అక్షరం as_elements;
  అక్షరం type1;
  అక్షరం offset2;
  అక్షరం type2;
} float_struct_info;

#ఒకవేళ ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#లేకపోతే
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#అయితే_ముగింపు

స్థిర ffi_type **
flatten_struct (ffi_type *లోపల, ffi_type **బహిర్గతం, ffi_type **బహిర్గతం_చివర)
{
  పూర్ణాంకం i;

  ఒకవేళ (బహిర్గతం == బహిర్గతం_చివర)
    తిరిగి బహిర్గతం;
  ఒకవేళ (లోపల->type != FFI_TYPE_STRUCT)
    *(బహిర్గతం++) = లోపల;
  లేకపోతే
    కొరకు (i = 0; లోపల->elements[i]; i++)
      బహిర్గతం = flatten_struct (లోపల->elements[i], బహిర్గతం, బహిర్గతం_చివర);
  తిరిగి బహిర్గతం;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
స్థిర float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *శిఖరం)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  పూర్ణాంకం అంకె_floats, అంకె_ints;
  పూర్ణాంకం అంకె_fields = flatten_struct (శిఖరం, fields, fields + 3) - fields;

  ఒకవేళ (అంకె_fields == 1)
    {
      ఒకవేళ (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  లేకపోతే ఒకవేళ (అంకె_fields == 2)
    {
      అంకె_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      అంకె_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      ఒకవేళ (అంకె_floats == 0 || అంకె_floats + అంకె_ints != 2)
	తిరిగి ret;
      ఒకవేళ (cb->వాడిన_float + అంకె_floats > NARGREG
	  || cb->వాడిన_integer + (2 - అంకె_floats) > NARGREG)
	తిరిగి ret;
      ఒకవేళ (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	తిరిగి ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->కొలత, fields[1]->alignment);
      ret.as_elements = 1;
    }
  తిరిగి ret;
}
#అయితే_ముగింపు

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
స్థిర శూన్య
marshal_atom (call_builder *cb, పూర్ణాంకం type, శూన్య *దత్తాంశం)
{
  size_t విలువ = 0;
  ఎంపిక (type)
    {
    స్థితి FFI_TYPE_UINT8:
      విలువ = *(uint8_t *) దత్తాంశం;
      ఆపు;
    స్థితి FFI_TYPE_SINT8:
      విలువ = *(int8_t *) దత్తాంశం;
      ఆపు;
    స్థితి FFI_TYPE_UINT16:
      విలువ = *(uint16_t *) దత్తాంశం;
      ఆపు;
    స్థితి FFI_TYPE_SINT16:
      విలువ = *(int16_t *) దత్తాంశం;
      ఆపు;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    స్థితి FFI_TYPE_UINT32:
      విలువ = *(int32_t *) దత్తాంశం;
      ఆపు;
    స్థితి FFI_TYPE_SINT32:
      విలువ = *(int32_t *) దత్తాంశం;
      ఆపు;
#ఒకవేళ __SIZEOF_POINTER__ == 8
    స్థితి FFI_TYPE_UINT64:
      విలువ = *(uint64_t *) దత్తాంశం;
      ఆపు;
    స్థితి FFI_TYPE_SINT64:
      విలువ = *(int64_t *) దత్తాంశం;
      ఆపు;
#అయితే_ముగింపు
    స్థితి FFI_TYPE_సూచిక:
      విలువ = *(size_t *) దత్తాంశం;
      ఆపు;

#ఒకవేళ ABI_FRLEN >= 32
    స్థితి FFI_TYPE_FLOAT:
      *(దశాంశం *)(cb->aregs->fa + cb->వాడిన_float++) = *(దశాంశం *) దత్తాంశం;
      తిరిగి;
#అయితే_ముగింపు
#ఒకవేళ ABI_FRLEN >= 64
    స్థితి FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->వాడిన_float++]) = *(ద్విగుణ *) దత్తాంశం;
      తిరిగి;
#అయితే_ముగింపు
    అప్రమేయ:
      FFI_ASSERT (0);
      ఆపు;
    }

  ఒకవేళ (cb->వాడిన_integer == NARGREG)
    *cb->వాడిన_దొంతర++ = విలువ;
  లేకపోతే
    cb->aregs->a[cb->వాడిన_integer++] = విలువ;
}

స్థిర శూన్య
unmarshal_atom (call_builder *cb, పూర్ణాంకం type, శూన్య *దత్తాంశం)
{
  size_t విలువ;
  ఎంపిక (type)
    {
#ఒకవేళ ABI_FRLEN >= 32
    స్థితి FFI_TYPE_FLOAT:
      *(దశాంశం *) దత్తాంశం = *(దశాంశం *)(cb->aregs->fa + cb->వాడిన_float++);
      తిరిగి;
#అయితే_ముగింపు
#ఒకవేళ ABI_FRLEN >= 64
    స్థితి FFI_TYPE_DOUBLE:
      *(ద్విగుణ *) దత్తాంశం = cb->aregs->fa[cb->వాడిన_float++];
      తిరిగి;
#అయితే_ముగింపు
    }

  ఒకవేళ (cb->వాడిన_integer == NARGREG)
    విలువ = *cb->వాడిన_దొంతర++;
  లేకపోతే
    విలువ = cb->aregs->a[cb->వాడిన_integer++];

  ఎంపిక (type)
    {
    స్థితి FFI_TYPE_UINT8:
    స్థితి FFI_TYPE_SINT8:
    స్థితి FFI_TYPE_UINT16:
    స్థితి FFI_TYPE_SINT16:
    స్థితి FFI_TYPE_UINT32:
    స్థితి FFI_TYPE_SINT32:
#ఒకవేళ __SIZEOF_POINTER__ == 8
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
#అయితే_ముగింపు
    స్థితి FFI_TYPE_సూచిక:
      *(ffi_arg *)దత్తాంశం = విలువ;
      ఆపు;
    అప్రమేయ:
      FFI_ASSERT (0);
      ఆపు;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
స్థిర శూన్య *
allocate_and_copy_struct_గమ్యం_దొంతర (call_builder *cb, శూన్య *దత్తాంశం,
				   ffi_type *type)
{
  size_t dest = cb->తదుపరి_struct_area - type->కొలత;

  dest = FFI_ALIGN_కిందికి (dest, type->alignment);
  cb->తదుపరి_struct_area = dest;

  తిరిగి జ్ఞాపకం_నకలు ((అక్షరం *)cb->దొంతర + dest, దత్తాంశం, type->కొలత);
}

/* Adds an argument to a call, or a not by reference return value.  */
స్థిర శూన్య
marshal (call_builder *cb, ffi_type *type, పూర్ణాంకం var, శూన్య *దత్తాంశం)
{
  size_t realign[2];

#ఒకవేళ ABI_FRLEN
  ఒకవేళ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ఒకవేళ (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, దత్తాంశం);
	  ఒకవేళ (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((అక్షరం *) దత్తాంశం) + fsi.offset2);
	  తిరిగి;
	}
    }

  ఒకవేళ (!var && cb->వాడిన_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, దత్తాంశం);
      తిరిగి;
    }

  ద్విగుణ promoted;
  ఒకవేళ (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(దశాంశం *) దత్తాంశం;
      type = &ffi_type_double;
      దత్తాంశం = &promoted;
    }
#అయితే_ముగింపు

  ఒకవేళ (type->కొలత > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_గమ్యం_దొంతర (cb, దత్తాంశం, type);
      దత్తాంశం = (అక్షరం *)cb->దొంతర + cb->తదుపరి_struct_area;
      marshal_atom (cb, FFI_TYPE_సూచిక, &దత్తాంశం);
    }
  లేకపోతే ఒకవేళ (IS_INT (type->type) || type->type == FFI_TYPE_సూచిక)
    marshal_atom (cb, type->type, దత్తాంశం);
  లేకపోతే
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      ఒకవేళ (type->alignment > __SIZEOF_POINTER__)
	{
	  ఒకవేళ (var)
	    cb->వాడిన_integer = FFI_ALIGN (cb->వాడిన_integer, 2);
	  cb->వాడిన_దొంతర
	    = (size_t *) FFI_ALIGN (cb->వాడిన_దొంతర, 2 * __SIZEOF_POINTER__);
	}

      జ్ఞాపకం_నకలు (realign, దత్తాంశం, type->కొలత);
      ఒకవేళ (type->కొలత > 0)
	marshal_atom (cb, FFI_TYPE_సూచిక, realign);
      ఒకవేళ (type->కొలత > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_సూచిక, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
స్థిర శూన్య *
unmarshal (call_builder *cb, ffi_type *type, పూర్ణాంకం var, శూన్య *దత్తాంశం)
{
  size_t realign[2];
  శూన్య *సూచిక_2;

#ఒకవేళ ABI_FRLEN
  ఒకవేళ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ఒకవేళ (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, దత్తాంశం);
	  ఒకవేళ (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((అక్షరం *) దత్తాంశం) + fsi.offset2);
	  తిరిగి దత్తాంశం;
	}
    }

  ఒకవేళ (!var && cb->వాడిన_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, దత్తాంశం);
      తిరిగి దత్తాంశం;
    }

  ఒకవేళ (var && type->type == FFI_TYPE_FLOAT)
    {
      పూర్ణాంకం m = cb->వాడిన_integer;
      శూన్య *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->వాడిన_దొంతర + m - NARGREG + 1;
      *(దశాంశం *) promoted = *(ద్విగుణ *) promoted;
    }
#అయితే_ముగింపు

  ఒకవేళ (type->కొలత > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_సూచిక, (అక్షరం *) &సూచిక_2);
      తిరిగి సూచిక_2;
    }
  లేకపోతే ఒకవేళ (IS_INT (type->type) || type->type == FFI_TYPE_సూచిక)
    {
      unmarshal_atom (cb, type->type, దత్తాంశం);
      తిరిగి దత్తాంశం;
    }
  లేకపోతే
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      ఒకవేళ (type->alignment > __SIZEOF_POINTER__)
	{
	  ఒకవేళ (var)
	    cb->వాడిన_integer = FFI_ALIGN (cb->వాడిన_integer, 2);
	  cb->వాడిన_దొంతర
	    = (size_t *) FFI_ALIGN (cb->వాడిన_దొంతర, 2 * __SIZEOF_POINTER__);
	}

      ఒకవేళ (type->కొలత > 0)
	unmarshal_atom (cb, FFI_TYPE_సూచిక, realign);
      ఒకవేళ (type->కొలత > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_సూచిక, realign + 1);
      జ్ఞాపకం_నకలు (దత్తాంశం, realign, type->కొలత);
      తిరిగి దత్తాంశం;
    }
}

స్థిర పూర్ణాంకం
passed_by_ref (call_builder *cb, ffi_type *type, పూర్ణాంకం var)
{
#ఒకవేళ ABI_FRLEN
  ఒకవేళ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      ఒకవేళ (fsi.as_elements)
	తిరిగి 0;
    }
#అయితే_ముగింపు

  తిరిగి type->కొలత > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_స్థితి
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  తిరిగి FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_స్థితి
ffi_prep_cif_machdep_var (ffi_cif *cif, గుర్తులేని పూర్ణాంకం nfixedargs,
			  గుర్తులేని పూర్ణాంకం ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  తిరిగి FFI_OK;
}

/* Low level routine for calling functions.  */
బాహ్య శూన్య ffi_call_asm (శూన్య *దొంతర, నిర్మాణం call_context *regs,
			  శూన్య (*fn) (శూన్య), శూన్య *సంవృతం) FFI_HIDDEN;

స్థిర శూన్య
ffi_call_int (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue,
	      శూన్య *సంవృతం)
{
  size_t arg_బైట్లు = FFI_ALIGN (cif->బైట్లు, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_బైట్లు = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * పరిమాణం(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_బైట్లు = 0;
  ఒకవేళ (rvalue == ఖాళీ && cif->rtype->కొలత > 2 * __SIZEOF_POINTER__)
    rval_బైట్లు = FFI_ALIGN (cif->rtype->కొలత, STKALIGN);
  size_t alloc_కొలత = arg_బైట్లు + extra_బైట్లు + rval_బైట్లు + పరిమాణం (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_పునాది;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  ఒకవేళ (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_పునాది = (size_t) alloca (alloc_కొలత);
  లేకపోతే
    alloc_పునాది = FFI_ALIGN (alloca (alloc_కొలత + STKALIGN - 1), STKALIGN);

  ఒకవేళ (rval_బైట్లు)
    rvalue = (శూన్య *) (alloc_పునాది + arg_బైట్లు);

  call_builder cb;
  cb.వాడిన_float = cb.వాడిన_integer = 0;
  cb.aregs = (call_context *) (alloc_పునాది + arg_బైట్లు + extra_బైట్లు + rval_బైట్లు);
  cb.వాడిన_దొంతర = (size_t *)((అక్షరం *) alloc_పునాది);
  cb.దొంతర = (size_t *)((అక్షరం *) alloc_పునాది + extra_బైట్లు + rval_బైట్లు);
  cb.తదుపరి_struct_area = arg_బైట్లు;

  పూర్ణాంకం return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  ఒకవేళ (return_by_ref)
    cb.aregs->a[cb.వాడిన_integer++] = (size_t)rvalue;

  పూర్ణాంకం i;
  కొరకు (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((శూన్య *) alloc_పునాది, cb.aregs, fn, సంవృతం);

  cb.వాడిన_float = cb.వాడిన_integer = 0;
  ఒకవేళ (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ఖాళీ);
}

శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue,
	     శూన్య *సంవృతం)
{
  ffi_call_int (cif, fn, rvalue, avalue, సంవృతం);
}

బాహ్య శూన్య ffi_సంవృతం_asm (శూన్య) FFI_HIDDEN;

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం *సంవృతం, ffi_cif *cif,
		      శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
		      శూన్య *user_దత్తాంశం, శూన్య *codeloc)
{
  uint32_t *tramp = (uint32_t *) &సంవృతం->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_సంవృతం_asm;

  ఒకవేళ (cif->abi <= FFI_మొదటి_ABI || cif->abi >= FFI_చివరి_ABI)
    తిరిగి FFI_BAD_ABI;

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
  ఒకవేళ (ffi_tramp_is_present(సంవృతం))
    {
      ffi_tramp_సమితి_parms (సంవృతం->ftramp, ffi_సంవృతం_asm, సంవృతం);
      వెళ్ళు బహిర్గతం;
    }
#అయితే_ముగింపు

  /* Fill the dynamic trampoline.  We will call ffi_closure_inner with codeloc,
     not closure, but as long as the memory is readable it should work.  */
  tramp[0] = 0x1800000c; /* pcaddi $t0, 0 (i.e. $t0 <- tramp) */
  tramp[1] = 0x28c0418d; /* ld.d   $t1, $t0, 16 */
  tramp[2] = 0x4c0001a0; /* jirl   $zero, $t1, 0 */
  tramp[3] = 0x03400000; /* nop */
  tramp[4] = fn;
  tramp[5] = fn >> 32;

  __builtin___clear_cache (codeloc, codeloc + FFI_TRAMPOLINE_కొలత);

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
బహిర్గతం:
#అయితే_ముగింపు
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}

బాహ్య శూన్య ffi_go_సంవృతం_asm (శూన్య) FFI_HIDDEN;

ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం *సంవృతం, ffi_cif *cif,
		     శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *))
{
  ఒకవేళ (cif->abi <= FFI_మొదటి_ABI || cif->abi >= FFI_చివరి_ABI)
    తిరిగి FFI_BAD_ABI;

  సంవృతం->tramp = (శూన్య *) ffi_go_సంవృతం_asm;
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  తిరిగి FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
శూన్య FFI_HIDDEN
ffi_సంవృతం_inner (ffi_cif *cif,
		   శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
		   శూన్య *user_దత్తాంశం, size_t *దొంతర, call_context *aregs)
{
  శూన్య **avalue = alloca (cif->nargs * పరిమాణం (శూన్య *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  అక్షరం *astorage = alloca (cif->nargs * MAXCOPYARG);
  శూన్య *rvalue;
  call_builder cb;
  పూర్ణాంకం return_by_ref;
  పూర్ణాంకం i;

  cb.aregs = aregs;
  cb.వాడిన_integer = cb.వాడిన_float = 0;
  cb.వాడిన_దొంతర = దొంతర;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  ఒకవేళ (return_by_ref)
    unmarshal (&cb, &ffi_type_సూచిక, 0, &rvalue);
  లేకపోతే
    rvalue = alloca (cif->rtype->కొలత);

  కొరకు (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_దత్తాంశం);

  ఒకవేళ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.వాడిన_integer = cb.వాడిన_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
శూన్య *
ffi_tramp_arch (size_t *tramp_కొలత, size_t *map_కొలత)
{
  బాహ్య శూన్య *trampoline_సంకేతం_పట్టిక;

  *tramp_కొలత = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_కొలత = 1 << 16;
  తిరిగి &trampoline_సంకేతం_పట్టిక;
}
#అయితే_ముగింపు
