/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2013 Tensilica, Inc.

   XTENSA Foreign Function Interface

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

/*
                                 |----------------------------------------|
                                 |                                        |
    on entry to ffi_call ---->   |----------------------------------------|
                                 | caller stack frame for registers a0-a3 |
                                 |----------------------------------------|
                                 |                                        |
                                 |         additional arguments           |
    entry of the function --->   |----------------------------------------|
                                 |    copy of function arguments a2-a7    |
                                 | -  -  -  -  -  -  -  -  -  -  -  -  -  |
                                 |                                        |

    The area below the entry line becomes the new stack frame for the function.

*/


#تعریف_کرو FFI_TYPE_STRUCT_REGS FFI_TYPE_آخری


خارجي فراغ ffi_call_SYSV(فراغ *rvalue, بنانشانی rsize, بنانشانی flags,
			  فراغ(*fn)(فراغ), بنانشانی nbytes, extended_cif*);
خارجي فراغ ffi_گھیرا_SYSV(فراغ) FFI_HIDDEN;

ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  بدل(cif->rtype->type) {
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT8:
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      اكسر;
    حالة FFI_TYPE_VOID:
    حالة FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      اكسر;
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      اكسر;
    حالة FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      إذا (cif->rtype->ناپ > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->بائٹاں += 8;
      }
      اكسر;

    افتراضي:
      cif->flags = FFI_TYPE_UINT32;
      اكسر;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  إذا (cif->بائٹاں < FFI_رجسٹر_NARGS * 4)
    cif->بائٹاں = FFI_رجسٹر_دلیلاں_SPACE;
  وإلا
    cif->بائٹاں = FFI_رجسٹر_دلیلاں_SPACE +
	    FFI_ALIGN(cif->بائٹاں - FFI_رجسٹر_NARGS * 4,
		      XTENSA_ڈھیر_ALIGNMENT);
  أرجع FFI_OK;
}

فراغ ffi_prep_دلیلاں(extended_cif *ecif, بنانشانی حرف* ڈھیر)
{
  بنانشانی صحيح i;
  بنانشانی لما *addr;
  ffi_type **ptr;

  اتحاد {
    فراغ **v;
    حرف **c;
    نشانیوالا حرف **sc;
    بنانشانی حرف **uc;
    نشانیوالا چھوٹا **ss;
    بنانشانی چھوٹا **us;
    بنانشانی صحيح **i;
    لما لما **ll;
    عائم **f;
    مزدوج **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((بنانشانی لما) ڈھیر & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (بنانشانی لما*)ڈھیر;

  /* structures with a size greater than 16 bytes are passed in memory */
  إذا (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->ناپ > 16)
  {
    *addr++ = (بنانشانی لما)ecif->rvalue;
  }

  لكل (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    بدل ((*ptr)->type)
    {
      حالة FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        اكسر;
      حالة FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        اكسر;
      حالة FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        اكسر;
      حالة FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        اكسر;
      حالة FFI_TYPE_FLOAT:
      حالة FFI_TYPE_INT:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_اشارہ_گر:
        *addr++ = **p_argv.i;
        اكسر;
      حالة FFI_TYPE_DOUBLE:
      حالة FFI_TYPE_UINT64:
      حالة FFI_TYPE_SINT64:
        إذا (((بنانشانی لما)addr & 4) != 0)
          addr++;
        *(بنانشانی لما لما*)addr = **p_argv.ll;
	addr += حجم(بنانشانی لما لما) / حجم (addr);
        اكسر;

      حالة FFI_TYPE_STRUCT:
      {
        بنانشانی لما offs;
        بنانشانی لما ناپ;

        إذا (((بنانشانی لما)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (بنانشانی لما) addr - (بنانشانی لما) ڈھیر;
        ناپ = (*ptr)->ناپ;

        /* Entire structure must fit the argument registers or referenced */
        إذا (offs < FFI_رجسٹر_NARGS * 4
            && offs + ناپ > FFI_رجسٹر_NARGS * 4)
          addr = (بنانشانی لما*) (ڈھیر + FFI_رجسٹر_NARGS * 4);

        حافظہ_نقل((حرف*) addr, *p_argv.c, ناپ);
        addr += (ناپ + 3) / 4;
        اكسر;
      }

      افتراضي:
        FFI_ASSERT(0);
    }
  }
}


فراغ ffi_call(ffi_cif* cif, فراغ(*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;
  بنانشانی لما rsize = cif->rtype->ناپ;
  صحيح flags = cif->flags;
  فراغ *alloc = عدم;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  إذا (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == عدم))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  وإلا
  {
    ecif.rvalue = rvalue;
  }

  إذا (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->بائٹاں, &ecif);

  إذا (alloc != عدم && rvalue != عدم)
    حافظہ_نقل(rvalue, alloc, rsize);
}

خارجي فراغ ffi_trampoline();
خارجي فراغ ffi_cacheflush(فراغ* شروع, فراغ* سرا);

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا* گھیرا,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
                      فراغ *user_انکڑے,
                      فراغ *codeloc)
{
  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  حافظہ_نقل(گھیرا->tramp, ffi_trampoline, FFI_TRAMPOLINE_ناپ);
  *(بنانشانی صحيح*)(&گھیرا->tramp[8]) = (بنانشانی صحيح)ffi_گھیرا_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(گھیرا->tramp, گھیرا->tramp + FFI_TRAMPOLINE_ناپ);

  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;
  أرجع FFI_OK; 
}


لما FFI_HIDDEN
ffi_گھیرا_SYSV_inner(ffi_گھیرا *گھیرا, فراغ **اقدار, فراغ *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  فراغ **avalue;
  صحيح i, areg;

  cif = گھیرا->cif;
  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  areg = 0;

  صحيح rtype = cif->rtype->type;
  إذا (rtype == FFI_TYPE_STRUCT && cif->rtype->ناپ > 4 * 4)
  {
    rvalue = *اقدار;
    areg++;
  }

  cif = گھیرا->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * حجم(فراغ *));

  لكل (i = 0; i < cif->nargs; i++)
  {
    إذا (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    إذا (areg == FFI_رجسٹر_NARGS)
      areg = (FFI_رجسٹر_دلیلاں_SPACE + 32) / 4;

    إذا (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      صحيح numregs = ((arg_types[i]->ناپ + 3) & ~3) / 4;
      إذا (areg < FFI_رجسٹر_NARGS && areg + numregs > FFI_رجسٹر_NARGS)
        areg = (FFI_رجسٹر_دلیلاں_SPACE + 32) / 4;
    }

    avalue[i] = &اقدار[areg];
    areg += (arg_types[i]->ناپ + 3) / 4;
  }

  (گھیرا->fun)(cif, rvalue, avalue, گھیرا->user_انکڑے);

  أرجع rtype;
}
