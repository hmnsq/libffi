/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2015 Michael Knyszek <mknyszek@berkeley.edu>
                         2015 Andrew Waterman <waterman@cs.berkeley.edu>
                         2018 Stef O'Rear <sorear2@gmail.com>
   Based on MIPS N32/64 port

   RISC-V Foreign Function Interface

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
#అన్తర్భావ "internal.h"
#అన్తర్భావ <tramp.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdint.h>

#ఒకవేళ __riscv_float_abi_double
#నిర్వచించు ABI_FLEN 64
రకనిర్వచనం సంఘం {
    uint64_t i;
    ద్విగుణ d;
} float_reg;
#లేదావేళ __riscv_float_abi_single
#నిర్వచించు ABI_FLEN 32
రకనిర్వచనం సంఘం {
    దశాంశం f;
} float_reg;
#అయితే_ముగింపు

#నిర్వచించు NARGREG 8
#నిర్వచించు STKALIGN 16
#నిర్వచించు MAXCOPYARG (2 * పరిమాణం(ద్విగుణ))

రకనిర్వచనం నిర్మాణం call_context
{
#ఒకవేళ ABI_FLEN
    float_reg fa[8];
#అయితే_ముగింపు
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    అక్షరం చట్రం[16];
} call_context;

రకనిర్వచనం నిర్మాణం call_builder
{
    call_context *aregs;
    పూర్ణాంకం వాడిన_integer;
    పూర్ణాంకం వాడిన_float;
    size_t *వాడిన_దొంతర;
    శూన్య *struct_దొంతర;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ఒకవేళ __SIZEOF_POINTER__ == 8
#నిర్వచించు IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#లేకపోతే
#నిర్వచించు IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#అయితే_ముగింపు

#ఒకవేళ ABI_FLEN
రకనిర్వచనం నిర్మాణం {
    అక్షరం as_elements, type1, offset2, type2;
} float_struct_info;

#ఒకవేళ ABI_FLEN >= 64
#నిర్వచించు IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#లేకపోతే
#నిర్వచించు IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#అయితే_ముగింపు

స్థిర ffi_type **flatten_struct(ffi_type *లోపల, ffi_type **బహిర్గతం, ffi_type **బహిర్గతం_చివర) {
    పూర్ణాంకం i;
    ఒకవేళ (బహిర్గతం == బహిర్గతం_చివర) తిరిగి బహిర్గతం;
    ఒకవేళ (లోపల->type != FFI_TYPE_STRUCT) {
        *(బహిర్గతం++) = లోపల;
    } లేకపోతే {
        కొరకు (i = 0; లోపల->elements[i]; i++)
            బహిర్గతం = flatten_struct(లోపల->elements[i], బహిర్గతం, బహిర్గతం_చివర);
    }
    తిరిగి బహిర్గతం;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
స్థిర float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *శిఖరం) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    పూర్ణాంకం అంకె_floats, అంకె_ints;
    పూర్ణాంకం అంకె_fields = flatten_struct(శిఖరం, fields, fields + 3) - fields;

    ఒకవేళ (అంకె_fields == 1) {
        ఒకవేళ (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } లేకపోతే ఒకవేళ (అంకె_fields == 2) {
        అంకె_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        అంకె_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        ఒకవేళ (అంకె_floats == 0 || అంకె_floats + అంకె_ints != 2)
            తిరిగి ret;
        ఒకవేళ (cb->వాడిన_float + అంకె_floats > NARGREG || cb->వాడిన_integer + (2 - అంకె_floats) > NARGREG)
            తిరిగి ret;
        ఒకవేళ (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            తిరిగి ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->కొలత, fields[1]->alignment);
        ret.as_elements = 1;
    }

    తిరిగి ret;
}

#ఒకవేళ ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
స్థిర శూన్య marshal_float(call_builder *cb, శూన్య *దత్తాంశం) {
    సంఘం {
        uint32_t i;
        దశాంశం f;
    } విలువ;

    విలువ.f = *(దశాంశం *)దత్తాంశం;
    cb->aregs->fa[cb->వాడిన_float++].i =
        UINT64_C(0xffffffff00000000) | విలువ.i;
}

స్థిర శూన్య unmarshal_float(call_builder *cb, శూన్య *దత్తాంశం) {
    సంఘం {
        uint32_t i;
        దశాంశం f;
    } విలువ;

    విలువ.i = (uint32_t)cb->aregs->fa[cb->వాడిన_float++].i;
    *(దశాంశం *)దత్తాంశం = విలువ.f;
}
#అయితే_ముగింపు
#అయితే_ముగింపు

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
స్థిర శూన్య marshal_atom(call_builder *cb, పూర్ణాంకం type, శూన్య *దత్తాంశం) {
    size_t విలువ = 0;
    ఎంపిక (type) {
        స్థితి FFI_TYPE_UINT8: విలువ = *(uint8_t *)దత్తాంశం; ఆపు;
        స్థితి FFI_TYPE_SINT8: విలువ = *(int8_t *)దత్తాంశం; ఆపు;
        స్థితి FFI_TYPE_UINT16: విలువ = *(uint16_t *)దత్తాంశం; ఆపు;
        స్థితి FFI_TYPE_SINT16: విలువ = *(int16_t *)దత్తాంశం; ఆపు;
        /* 32-bit quantities are always sign-extended in the ABI */
        స్థితి FFI_TYPE_UINT32: విలువ = *(int32_t *)దత్తాంశం; ఆపు;
        స్థితి FFI_TYPE_SINT32: విలువ = *(int32_t *)దత్తాంశం; ఆపు;
#ఒకవేళ __SIZEOF_POINTER__ == 8
        స్థితి FFI_TYPE_UINT64: విలువ = *(uint64_t *)దత్తాంశం; ఆపు;
        స్థితి FFI_TYPE_SINT64: విలువ = *(int64_t *)దత్తాంశం; ఆపు;
#అయితే_ముగింపు
        స్థితి FFI_TYPE_సూచిక: విలువ = *(size_t *)దత్తాంశం; ఆపు;

#ఒకవేళ ABI_FLEN >= 64
        స్థితి FFI_TYPE_FLOAT:
            marshal_float(cb, దత్తాంశం);
            తిరిగి;
#లేదావేళ ABI_FLEN >= 32
        స్థితి FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->వాడిన_float++].f) : "0"(*(దశాంశం *)దత్తాంశం));
            తిరిగి;
#అయితే_ముగింపు
#ఒకవేళ ABI_FLEN >= 64
        స్థితి FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->వాడిన_float++].d) : "0"(*(ద్విగుణ *)దత్తాంశం));
            తిరిగి;
#అయితే_ముగింపు
        అప్రమేయ: FFI_ASSERT(0); ఆపు;
    }

    ఒకవేళ (cb->వాడిన_integer == NARGREG) {
        *cb->వాడిన_దొంతర++ = విలువ;
    } లేకపోతే {
        cb->aregs->a[cb->వాడిన_integer++] = విలువ;
    }
}

స్థిర శూన్య unmarshal_atom(call_builder *cb, పూర్ణాంకం type, శూన్య *దత్తాంశం) {
    size_t విలువ;
    ఎంపిక (type) {
#ఒకవేళ ABI_FLEN >= 64
        స్థితి FFI_TYPE_FLOAT:
            unmarshal_float(cb, దత్తాంశం);
            తిరిగి;
#లేదావేళ ABI_FLEN >= 32
        స్థితి FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(దశాంశం *)దత్తాంశం) : "0"(cb->aregs->fa[cb->వాడిన_float++].f));
            తిరిగి;
#అయితే_ముగింపు
#ఒకవేళ ABI_FLEN >= 64
        స్థితి FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(ద్విగుణ *)దత్తాంశం) : "0"(cb->aregs->fa[cb->వాడిన_float++].d));
            తిరిగి;
#అయితే_ముగింపు
    }

    ఒకవేళ (cb->వాడిన_integer == NARGREG) {
        విలువ = *cb->వాడిన_దొంతర++;
    } లేకపోతే {
        విలువ = cb->aregs->a[cb->వాడిన_integer++];
    }

    ఎంపిక (type) {
        స్థితి FFI_TYPE_UINT8: *(uint8_t *)దత్తాంశం = విలువ; ఆపు;
        స్థితి FFI_TYPE_SINT8: *(uint8_t *)దత్తాంశం = విలువ; ఆపు;
        స్థితి FFI_TYPE_UINT16: *(uint16_t *)దత్తాంశం = విలువ; ఆపు;
        స్థితి FFI_TYPE_SINT16: *(uint16_t *)దత్తాంశం = విలువ; ఆపు;
        స్థితి FFI_TYPE_UINT32: *(uint32_t *)దత్తాంశం = విలువ; ఆపు;
        స్థితి FFI_TYPE_SINT32: *(uint32_t *)దత్తాంశం = విలువ; ఆపు;
#ఒకవేళ __SIZEOF_POINTER__ == 8
        స్థితి FFI_TYPE_UINT64: *(uint64_t *)దత్తాంశం = విలువ; ఆపు;
        స్థితి FFI_TYPE_SINT64: *(uint64_t *)దత్తాంశం = విలువ; ఆపు;
#అయితే_ముగింపు
        స్థితి FFI_TYPE_సూచిక: *(size_t *)దత్తాంశం = విలువ; ఆపు;
        అప్రమేయ: FFI_ASSERT(0); ఆపు;
    }
}

/* adds an argument to a call, or a not by reference return value */
స్థిర శూన్య marshal(call_builder *cb, ffi_type *type, పూర్ణాంకం var, శూన్య *దత్తాంశం) {
    size_t realign[2];

#ఒకవేళ ABI_FLEN
    ఒకవేళ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ఒకవేళ (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, దత్తాంశం);
            ఒకవేళ (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((అక్షరం*)దత్తాంశం) + fsi.offset2);
            తిరిగి;
        }
    }

    ఒకవేళ (!var && cb->వాడిన_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, దత్తాంశం);
        తిరిగి;
    }
#అయితే_ముగింపు

    ఒకవేళ (type->కొలత > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        దత్తాంశం = జ్ఞాపకం_నకలు (cb->struct_దొంతర, దత్తాంశం, type->కొలత);
        cb->struct_దొంతర = (size_t *) FFI_ALIGN ((అక్షరం *) cb->struct_దొంతర + type->కొలత, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_సూచిక, &దత్తాంశం);
    } లేకపోతే ఒకవేళ (IS_INT(type->type) || type->type == FFI_TYPE_సూచిక) {
        marshal_atom(cb, type->type, దత్తాంశం);
    } లేకపోతే {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        ఒకవేళ (type->alignment > __SIZEOF_POINTER__) {
            ఒకవేళ (var)
                cb->వాడిన_integer = FFI_ALIGN(cb->వాడిన_integer, 2);
            cb->వాడిన_దొంతర = (size_t *)FFI_ALIGN(cb->వాడిన_దొంతర, 2*__SIZEOF_POINTER__);
        }

        జ్ఞాపకం_నకలు(realign, దత్తాంశం, type->కొలత);
        ఒకవేళ (type->కొలత > 0)
            marshal_atom(cb, FFI_TYPE_సూచిక, realign);
        ఒకవేళ (type->కొలత > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_సూచిక, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
స్థిర శూన్య *unmarshal(call_builder *cb, ffi_type *type, పూర్ణాంకం var, శూన్య *దత్తాంశం) {
    size_t realign[2];
    శూన్య *సూచిక_2;

#ఒకవేళ ABI_FLEN
    ఒకవేళ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ఒకవేళ (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, దత్తాంశం);
            ఒకవేళ (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((అక్షరం*)దత్తాంశం) + fsi.offset2);
            తిరిగి దత్తాంశం;
        }
    }

    ఒకవేళ (!var && cb->వాడిన_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, దత్తాంశం);
        తిరిగి దత్తాంశం;
    }
#అయితే_ముగింపు

    ఒకవేళ (type->కొలత > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_సూచిక, (అక్షరం*)&సూచిక_2);
        తిరిగి సూచిక_2;
    } లేకపోతే ఒకవేళ (IS_INT(type->type) || type->type == FFI_TYPE_సూచిక) {
        unmarshal_atom(cb, type->type, దత్తాంశం);
        తిరిగి దత్తాంశం;
    } లేకపోతే {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        ఒకవేళ (type->alignment > __SIZEOF_POINTER__) {
            ఒకవేళ (var)
                cb->వాడిన_integer = FFI_ALIGN(cb->వాడిన_integer, 2);
            cb->వాడిన_దొంతర = (size_t *)FFI_ALIGN(cb->వాడిన_దొంతర, 2*__SIZEOF_POINTER__);
        }

        ఒకవేళ (type->కొలత > 0)
            unmarshal_atom(cb, FFI_TYPE_సూచిక, realign);
        ఒకవేళ (type->కొలత > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_సూచిక, realign + 1);
        జ్ఞాపకం_నకలు(దత్తాంశం, realign, type->కొలత);
        తిరిగి దత్తాంశం;
    }
}

స్థిర పూర్ణాంకం passed_by_ref(call_builder *cb, ffi_type *type, పూర్ణాంకం var) {
#ఒకవేళ ABI_FLEN
    ఒకవేళ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ఒకవేళ (fsi.as_elements) తిరిగి 0;
    }
#అయితే_ముగింపు

    తిరిగి type->కొలత > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    తిరిగి FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_స్థితి ffi_prep_cif_machdep_var(ffi_cif *cif, గుర్తులేని పూర్ణాంకం nfixedargs, గుర్తులేని పూర్ణాంకం ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    తిరిగి FFI_OK;
}

/* Low level routine for calling functions */
బాహ్య శూన్య ffi_call_asm (శూన్య *దొంతర, నిర్మాణం call_context *regs,
			  శూన్య (*fn) (శూన్య), శూన్య *సంవృతం) FFI_HIDDEN;

స్థిర శూన్య
ffi_call_int (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue,
	      శూన్య *సంవృతం)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_బైట్లు = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * పరిమాణం(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_బైట్లు = FFI_ALIGN (cif->బైట్లు, STKALIGN);
    size_t rval_బైట్లు = 0;
    ఒకవేళ (rvalue == ఖాళీ && cif->rtype->కొలత > 2*__SIZEOF_POINTER__)
        rval_బైట్లు = FFI_ALIGN(cif->rtype->కొలత, STKALIGN);
    size_t alloc_కొలత = arg_బైట్లు + rval_బైట్లు + struct_బైట్లు + పరిమాణం(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_పునాది;
    /* Argument region must be 16-byte aligned */
    ఒకవేళ (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_పునాది = (size_t)alloca(alloc_కొలత);
    } లేకపోతే {
        alloc_పునాది = FFI_ALIGN(alloca(alloc_కొలత + STKALIGN - 1), STKALIGN);
    }

    ఒకవేళ (rval_బైట్లు)
        rvalue = (శూన్య*)(alloc_పునాది + arg_బైట్లు);

    call_builder cb;
    cb.వాడిన_float = cb.వాడిన_integer = 0;
    cb.aregs = (call_context*)(alloc_పునాది + arg_బైట్లు + rval_బైట్లు + struct_బైట్లు);
    cb.వాడిన_దొంతర = (శూన్య*)alloc_పునాది;
    cb.struct_దొంతర = (శూన్య *) (alloc_పునాది + arg_బైట్లు + rval_బైట్లు);

    పూర్ణాంకం return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    ఒకవేళ (return_by_ref)
        marshal(&cb, &ffi_type_సూచిక, 0, &rvalue);

    పూర్ణాంకం i;
    కొరకు (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((శూన్య *) alloc_పునాది, cb.aregs, fn, సంవృతం);

    cb.వాడిన_float = cb.వాడిన_integer = 0;
    ఒకవేళ (!return_by_ref && rvalue)
      {
	ఒకవేళ (IS_INT(cif->rtype->type)
	    && cif->rtype->కొలత < పరిమాణం (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    ఎంపిక (cif->rtype->type)
	      {
	      స్థితి FFI_TYPE_SINT8:
	      స్థితి FFI_TYPE_SINT16:
	      స్థితి FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (పరిమాణం (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		ఆపు;
	      స్థితి FFI_TYPE_UINT8:
	      స్థితి FFI_TYPE_UINT16:
	      స్థితి FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (పరిమాణం (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		ఆపు;
	      }
	  }
	లేకపోతే
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ఖాళీ);
}

శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
  ffi_call_int(cif, fn, rvalue, avalue, సంవృతం);
}

బాహ్య శూన్య ffi_సంవృతం_asm(శూన్య) FFI_HIDDEN;

ffi_స్థితి ffi_prep_సంవృతం_loc(ffi_సంవృతం *సంవృతం, ffi_cif *cif, శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*), శూన్య *user_దత్తాంశం, శూన్య *codeloc)
{
    ఒకవేళ (cif->abi <= FFI_మొదటి_ABI || cif->abi >= FFI_చివరి_ABI)
        తిరిగి FFI_BAD_ABI;

#నిర్వచితమైతే FFI_EXEC_STATIC_TRAMP
  ఒకవేళ (ffi_tramp_is_present (సంవృతం))
    {
      /* Initialize the static trampoline's parameters. */
      శూన్య (*dest)(శూన్య) = ffi_సంవృతం_asm;
      ffi_tramp_సమితి_parms (సంవృతం->ftramp, dest, సంవృతం);
    }
  లేకపోతే
#అయితే_ముగింపు
    {
      uint32_t *tramp = (uint32_t *) &సంవృతం->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_సంవృతం_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#ఒకవేళ __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#లేకపోతే
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#అయితే_ముగింపు
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#ఒకవేళ !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_కొలత);
#అయితే_ముగింపు
    }

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
   registers will be reloaded from aregs. */
శూన్య FFI_HIDDEN
ffi_సంవృతం_inner (ffi_cif *cif,
		   శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
		   శూన్య *user_దత్తాంశం,
		   size_t *దొంతర, call_context *aregs)
{
    శూన్య **avalue = alloca(cif->nargs * పరిమాణం(శూన్య*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    అక్షరం *astorage = alloca(cif->nargs * MAXCOPYARG);
    శూన్య *rvalue;
    call_builder cb;
    పూర్ణాంకం return_by_ref;
    పూర్ణాంకం i;

    cb.aregs = aregs;
    cb.వాడిన_integer = cb.వాడిన_float = 0;
    cb.వాడిన_దొంతర = దొంతర;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    ఒకవేళ (return_by_ref)
        unmarshal(&cb, &ffi_type_సూచిక, 0, &rvalue);
    లేకపోతే
        rvalue = alloca(cif->rtype->కొలత);

    కొరకు (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_దత్తాంశం);

    ఒకవేళ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.వాడిన_integer = cb.వాడిన_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#నిర్వచితమైతే FFI_EXEC_STATIC_TRAMP
శూన్య *
ffi_tramp_arch (size_t *tramp_కొలత, size_t *map_కొలత)
{
  బాహ్య శూన్య *trampoline_సంకేతం_పట్టిక;
  *tramp_కొలత = RISCV_TRAMP_కొలత;
  *map_కొలత = RISCV_TRAMP_MAP_కొలత;
  తిరిగి &trampoline_సంకేతం_పట్టిక;
}
#అయితే_ముగింపు
