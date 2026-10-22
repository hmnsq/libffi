/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2013  Synopsys, Inc. (www.synopsys.com)
   
   ARC Foreign Function Interface 

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL RENESAS TECHNOLOGY BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdint.h>

#అన్తర్భావ <sys/cachectl.h>

#నిర్వచించు NARGREG 8
#నిర్వచించు STKALIGN 4
#నిర్వచించు MAXCOPYARG (2 * పరిమాణం(ద్విగుణ))

రకనిర్వచనం నిర్మాణం call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  అక్షరం చట్రం[16];
} call_context;

రకనిర్వచనం నిర్మాణం call_builder
{
  call_context *aregs;
  పూర్ణాంకం వాడిన_integer;
  //int used_float;
  size_t *వాడిన_దొంతర;
  శూన్య *struct_దొంతర;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ఒకవేళ defined(__ARC64_ARCH64__)
#నిర్వచించు IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#లేకపోతే
#నిర్వచించు IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#అయితే_ముగింపు

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#ఒకవేళ __BIG_ENDIAN__
#నిర్వచించు సంకేతం_ENDIAN(x) (x)
#లేకపోతే
#నిర్వచించు సంకేతం_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#అయితే_ముగింపు

/* Perform machine dependent cif processing.  */
ffi_స్థితి
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
      cif->flags = (గుర్తులేని) cif->rtype->type;
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
      cif->flags = (గుర్తులేని) cif->rtype->type;
      ఆపు;

    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ఆపు;

    స్థితి FFI_TYPE_FLOAT:
    అప్రమేయ:
      cif->flags = FFI_TYPE_INT;
      ఆపు;
    }

  తిరిగి FFI_OK;
}

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
#ఒకవేళ defined(__ARC64_ARCH64__)
    స్థితి FFI_TYPE_UINT64: విలువ = *(uint64_t *)దత్తాంశం; ఆపు;
    స్థితి FFI_TYPE_SINT64: విలువ = *(int64_t *)దత్తాంశం; ఆపు;
#అయితే_ముగింపు
    స్థితి FFI_TYPE_సూచిక: విలువ = *(size_t *)దత్తాంశం; ఆపు;
    అప్రమేయ: FFI_ASSERT(0); ఆపు;
  }

  ఒకవేళ (cb->వాడిన_integer == NARGREG) {
    *cb->వాడిన_దొంతర++ = విలువ;
  } లేకపోతే {
    cb->aregs->r[cb->వాడిన_integer++] = విలువ;
  }
}

/* adds an argument to a call, or a not by reference return value */
స్థిర శూన్య marshal(call_builder *cb, ffi_type *type, పూర్ణాంకం var, శూన్య *దత్తాంశం) {
  size_t realign[2];

#ఒకవేళ (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  ఒకవేళ (type->కొలత > 2 * __SIZEOF_POINTER__) {
    ఒకవేళ (var) {
      marshal_atom(cb, FFI_TYPE_సూచిక, &దత్తాంశం);
    } లేకపోతే {
      /* copy to stack and pass by reference */
      దత్తాంశం = జ్ఞాపకం_నకలు (cb->struct_దొంతర, దత్తాంశం, type->కొలత);
      cb->struct_దొంతర = (size_t *) FFI_ALIGN ((అక్షరం *) cb->struct_దొంతర + type->కొలత, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_సూచిక, &దత్తాంశం);
    }
  }
#లేకపోతే
  ఒకవేళ (type->type == FFI_TYPE_STRUCT) {
    ఒకవేళ (var) {
      ఒకవేళ (type->కొలత > 0)
        marshal_atom(cb, FFI_TYPE_సూచిక, దత్తాంశం);
    } లేకపోతే {
      పూర్ణాంకం i;
      
      కొరకు (i = 0; i < type->కొలత; i += పరిమాణం(size_t)) {
        marshal_atom(cb, FFI_TYPE_సూచిక, దత్తాంశం);
        దత్తాంశం += పరిమాణం(size_t);
      }
    }
  }
#అయితే_ముగింపు
  లేకపోతే ఒకవేళ (IS_INT(type->type) || type->type == FFI_TYPE_సూచిక) {
    marshal_atom(cb, type->type, దత్తాంశం);
  } లేకపోతే {
      జ్ఞాపకం_నకలు(realign, దత్తాంశం, type->కొలత);
      ఒకవేళ (type->కొలత > 0)
        marshal_atom(cb, FFI_TYPE_సూచిక, realign);
      ఒకవేళ (type->కొలత > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_సూచిక, realign + 1);
  }
}

స్థిర శూన్య unmarshal_atom(call_builder *cb, పూర్ణాంకం type, శూన్య *దత్తాంశం) {
  size_t విలువ;

  ఒకవేళ (cb->వాడిన_integer == NARGREG) {
    విలువ = *cb->వాడిన_దొంతర++;
  } లేకపోతే {
    విలువ = cb->aregs->r[cb->వాడిన_integer++];
  }

  ఎంపిక (type) {
    స్థితి FFI_TYPE_UINT8: *(uint8_t *)దత్తాంశం = విలువ; ఆపు;
    స్థితి FFI_TYPE_SINT8: *(uint8_t *)దత్తాంశం = విలువ; ఆపు;
    స్థితి FFI_TYPE_UINT16: *(uint16_t *)దత్తాంశం = విలువ; ఆపు;
    స్థితి FFI_TYPE_SINT16: *(uint16_t *)దత్తాంశం = విలువ; ఆపు;
    స్థితి FFI_TYPE_UINT32: *(uint32_t *)దత్తాంశం = విలువ; ఆపు;
    స్థితి FFI_TYPE_SINT32: *(uint32_t *)దత్తాంశం = విలువ; ఆపు;
#ఒకవేళ defined(__ARC64_ARCH64__)
    స్థితి FFI_TYPE_UINT64: *(uint64_t *)దత్తాంశం = విలువ; ఆపు;
    స్థితి FFI_TYPE_SINT64: *(uint64_t *)దత్తాంశం = విలువ; ఆపు;
#అయితే_ముగింపు
    స్థితి FFI_TYPE_సూచిక: *(size_t *)దత్తాంశం = విలువ; ఆపు;
    అప్రమేయ: FFI_ASSERT(0); ఆపు;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
స్థిర శూన్య *unmarshal(call_builder *cb, ffi_type *type, పూర్ణాంకం var, శూన్య *దత్తాంశం) {
  size_t realign[2];

#ఒకవేళ defined(__ARC64_ARCH64__)
  శూన్య *సూచిక_2;

  ఒకవేళ (type->కొలత > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_సూచిక, (అక్షరం*)&సూచిక_2);
        తిరిగి సూచిక_2;
    }
#లేదావేళ defined(__ARC64_ARCH32__)
  ఒకవేళ (type->type == FFI_TYPE_STRUCT) {
    ఒకవేళ (type->కొలత > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_సూచిక, &realign[0]);
      జ్ఞాపకం_నకలు(దత్తాంశం, (స్థిరరాశి శూన్య*)realign[0], type->కొలత);
      తిరిగి దత్తాంశం;
    } లేకపోతే {
      పూర్ణాంకం i;
      శూన్య *pdata = దత్తాంశం;

      కొరకు (i = 0; i < type->కొలత; i += పరిమాణం(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_సూచిక, pdata);
        pdata += పరిమాణం(size_t);
      }
      తిరిగి దత్తాంశం;
    }
  }
#లేకపోతే
  ఒకవేళ (type->type == FFI_TYPE_STRUCT) {

      ఒకవేళ (var) {
        పూర్ణాంకం i;
        శూన్య *pdata = దత్తాంశం;

        కొరకు (i = 0; i < type->కొలత; i += పరిమాణం(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_సూచిక, pdata);
          pdata += పరిమాణం(size_t);
        }
        తిరిగి దత్తాంశం;
      } లేకపోతే {
        ఒకవేళ (type->కొలత > 0)
          unmarshal_atom(cb, FFI_TYPE_సూచిక, &realign[0]);
        జ్ఞాపకం_నకలు(దత్తాంశం, (స్థిరరాశి శూన్య*)realign[0], type->కొలత);
        తిరిగి దత్తాంశం;
      }
  }
#అయితే_ముగింపు
  లేకపోతే ఒకవేళ (IS_INT(type->type) || type->type == FFI_TYPE_సూచిక) {
    unmarshal_atom(cb, type->type, దత్తాంశం);
    తిరిగి దత్తాంశం;
  } లేకపోతే {
      ఒకవేళ (type->కొలత > 0)
        unmarshal_atom(cb, FFI_TYPE_సూచిక, realign);
      ఒకవేళ (type->కొలత > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_సూచిక, realign + 1);
      జ్ఞాపకం_నకలు(దత్తాంశం, realign, type->కొలత);
      తిరిగి దత్తాంశం;
  }
}

స్థిర పూర్ణాంకం passed_by_ref(ffi_type *type, పూర్ణాంకం var) {
  ఒకవేళ (type->type == FFI_TYPE_STRUCT)
	  తిరిగి 1;
  
  తిరిగి type->కొలత > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
బాహ్య శూన్య ffi_call_asm (శూన్య *దొంతర, నిర్మాణం call_context *regs,
			  శూన్య (*fn) (శూన్య), శూన్య *సంవృతం) FFI_HIDDEN;

స్థిర శూన్య
ffi_call_int (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue,
	      శూన్య *సంవృతం)
{
  పూర్ణాంకం return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_బైట్లు = FFI_ALIGN(2 * పరిమాణం(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_బైట్లు = FFI_ALIGN(cif->బైట్లు, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_కొలత = arg_బైట్లు + /*rval_bytes +*/ struct_బైట్లు + పరిమాణం(call_context);
  size_t alloc_పునాది = (size_t)alloca(alloc_కొలత);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.వాడిన_integer = 0;
  cb.aregs = (call_context*)(alloc_పునాది + arg_బైట్లు /*+ rval_bytes*/ + struct_బైట్లు);
  cb.వాడిన_దొంతర = (శూన్య*)alloc_పునాది;
  cb.struct_దొంతర = (శూన్య *)(alloc_పునాది + arg_బైట్లు /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  ఒకవేళ (return_by_ref)
	  marshal(&cb, &ffi_type_సూచిక, 0, &rvalue);

  పూర్ణాంకం i;
  కొరకు (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((శూన్య *) alloc_పునాది, cb.aregs, fn, సంవృతం);

  cb.వాడిన_integer = 0;
  ఒకవేళ (!return_by_ref && rvalue)
    {
	    ఒకవేళ (IS_INT(cif->rtype->type)
	        && cif->rtype->కొలత < పరిమాణం (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    ఎంపిక (cif->rtype->type) {
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

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం * సంవృతం, ffi_cif * cif,
		      శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
		      శూన్య *user_దత్తాంశం, శూన్య *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (సంవృతం->tramp[0]);

#ఒకవేళ defined(__ARC64_ARCH64__)
  size_t చిరునామా_ffi_సంవృతం = (size_t) ffi_సంవృతం_asm;
#అయితే_ముగింపు

  ఎంపిక (cif->abi)
    {
#ఒకవేళ defined(__ARC64_ARCH64__)
    స్థితి FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = సంకేతం_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = సంకేతం_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = సంకేతం_ENDIAN ((uint32_t)(చిరునామా_ffi_సంవృతం >> 32));
      tramp[3] = సంకేతం_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = సంకేతం_ENDIAN ((uint32_t)(చిరునామా_ffi_సంవృతం & 0xffffffff));
      tramp[5] = సంకేతం_ENDIAN (0x20200300);	/* j [r12] */
      ఆపు;
#లేకపోతే
    స్థితి FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = సంకేతం_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = సంకేతం_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = సంకేతం_ENDIAN ((uint32_t) ffi_సంవృతం_asm);
      ఆపు;
#అయితే_ముగింపు

    అప్రమేయ:
      తిరిగి FFI_BAD_ABI;
    }

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  cacheflush (codeloc, FFI_TRAMPOLINE_కొలత, BCACHE);

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
    అక్షరం *astorage = alloca(cif->బైట్లు);
    అక్షరం *ptr = astorage;
    శూన్య *rvalue;
    call_builder cb;
    పూర్ణాంకం i;

    cb.aregs = aregs;
    cb.వాడిన_integer = 0;
    cb.వాడిన_దొంతర = దొంతర;
    
    /* handle hidden argument */
    ఒకవేళ (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_సూచిక, 0, &rvalue);
    లేకపోతే
      rvalue = alloca(cif->rtype->కొలత);

    కొరకు (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->కొలత;
    }

    fun (cif, rvalue, avalue, user_దత్తాంశం);

    ఒకవేళ (cif->rtype->type != FFI_TYPE_VOID) {
        cb.వాడిన_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
