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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

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


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_REGS FFI_TYPE_ਆਖਰੀ


ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV(ਖਾਲੀ *rvalue, ਬਿਨਾਨਿਸ਼ਾਨੀ rsize, ਬਿਨਾਨਿਸ਼ਾਨੀ flags,
			  ਖਾਲੀ(*fn)(ਖਾਲੀ), ਬਿਨਾਨਿਸ਼ਾਨੀ nbytes, extended_cif*);
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_SYSV(ਖਾਲੀ) FFI_HIDDEN;

ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
  ਚੋਣ(cif->rtype->type) {
    ਹਾਲਤ FFI_TYPE_SINT8:
    ਹਾਲਤ FFI_TYPE_UINT8:
    ਹਾਲਤ FFI_TYPE_SINT16:
    ਹਾਲਤ FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_VOID:
    ਹਾਲਤ FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      ਜੇ (cif->rtype->ਮਾਪ > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->ਬਾਈਟਾਂ += 8;
      }
      ਰੋਕੋ;

    ਮੂਲ:
      cif->flags = FFI_TYPE_UINT32;
      ਰੋਕੋ;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  ਜੇ (cif->ਬਾਈਟਾਂ < FFI_ਰਜਿਸਟਰ_NARGS * 4)
    cif->ਬਾਈਟਾਂ = FFI_ਰਜਿਸਟਰ_ਦਲੀਲਾਂ_SPACE;
  ਨਹੀਂਤਾਂ
    cif->ਬਾਈਟਾਂ = FFI_ਰਜਿਸਟਰ_ਦਲੀਲਾਂ_SPACE +
	    FFI_ALIGN(cif->ਬਾਈਟਾਂ - FFI_ਰਜਿਸਟਰ_NARGS * 4,
		      XTENSA_ਢੇਰ_ALIGNMENT);
  ਵਾਪਸ FFI_OK;
}

ਖਾਲੀ ffi_prep_ਦਲੀਲਾਂ(extended_cif *ecif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* ਢੇਰ)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *addr;
  ffi_type **ptr;

  ਸੰਘ {
    ਖਾਲੀ **v;
    ਅੱਖਰ **c;
    ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ **sc;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **uc;
    ਨਿਸ਼ਾਨੀਵਾਲਾ ਛੋਟਾ **ss;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ **us;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ **i;
    ਲੰਮਾ ਲੰਮਾ **ll;
    ਦਸ਼ਮਲਵ **f;
    ਦੁੱਗਣਾ **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ਢੇਰ & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ*)ਢੇਰ;

  /* structures with a size greater than 16 bytes are passed in memory */
  ਜੇ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->ਮਾਪ > 16)
  {
    *addr++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ecif->rvalue;
  }

  ਲਈ (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    ਚੋਣ ((*ptr)->type)
    {
      ਹਾਲਤ FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        ਰੋਕੋ;
      ਹਾਲਤ FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        ਰੋਕੋ;
      ਹਾਲਤ FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        ਰੋਕੋ;
      ਹਾਲਤ FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        ਰੋਕੋ;
      ਹਾਲਤ FFI_TYPE_FLOAT:
      ਹਾਲਤ FFI_TYPE_INT:
      ਹਾਲਤ FFI_TYPE_UINT32:
      ਹਾਲਤ FFI_TYPE_SINT32:
      ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
        *addr++ = **p_argv.i;
        ਰੋਕੋ;
      ਹਾਲਤ FFI_TYPE_DOUBLE:
      ਹਾਲਤ FFI_TYPE_UINT64:
      ਹਾਲਤ FFI_TYPE_SINT64:
        ਜੇ (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)addr & 4) != 0)
          addr++;
        *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ*)addr = **p_argv.ll;
	addr += ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ) / ਆਕਾਰ (addr);
        ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_STRUCT:
      {
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ offs;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਮਾਪ;

        ਜੇ (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) addr - (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ਢੇਰ;
        ਮਾਪ = (*ptr)->ਮਾਪ;

        /* Entire structure must fit the argument registers or referenced */
        ਜੇ (offs < FFI_ਰਜਿਸਟਰ_NARGS * 4
            && offs + ਮਾਪ > FFI_ਰਜਿਸਟਰ_NARGS * 4)
          addr = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ*) (ਢੇਰ + FFI_ਰਜਿਸਟਰ_NARGS * 4);

        ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਅੱਖਰ*) addr, *p_argv.c, ਮਾਪ);
        addr += (ਮਾਪ + 3) / 4;
        ਰੋਕੋ;
      }

      ਮੂਲ:
        FFI_ASSERT(0);
    }
  }
}


ਖਾਲੀ ffi_call(ffi_cif* cif, ਖਾਲੀ(*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ rsize = cif->rtype->ਮਾਪ;
  ਪੂਰਨ flags = cif->flags;
  ਖਾਲੀ *alloc = ਨਿਰਾ;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  ਜੇ (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == ਨਿਰਾ))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  ਨਹੀਂਤਾਂ
  {
    ecif.rvalue = rvalue;
  }

  ਜੇ (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->ਬਾਈਟਾਂ, &ecif);

  ਜੇ (alloc != ਨਿਰਾ && rvalue != ਨਿਰਾ)
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(rvalue, alloc, rsize);
}

ਬਾਹਰੀ ਖਾਲੀ ffi_trampoline();
ਬਾਹਰੀ ਖਾਲੀ ffi_cacheflush(ਖਾਲੀ* ਸ਼ੁਰੂ, ਖਾਲੀ* ਸਿਰਾ);

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
                      ffi_cif* cif,
                      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
                      ਖਾਲੀ *user_ਅੰਕੜੇ,
                      ਖਾਲੀ *codeloc)
{
  ਜੇ (cif->abi != FFI_SYSV)
    ਵਾਪਸ FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਘੇਰਾ->tramp, ffi_trampoline, FFI_TRAMPOLINE_ਮਾਪ);
  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*)(&ਘੇਰਾ->tramp[8]) = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ffi_ਘੇਰਾ_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(ਘੇਰਾ->tramp, ਘੇਰਾ->tramp + FFI_TRAMPOLINE_ਮਾਪ);

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  ਵਾਪਸ FFI_OK; 
}


ਲੰਮਾ FFI_HIDDEN
ffi_ਘੇਰਾ_SYSV_inner(ffi_ਘੇਰਾ *ਘੇਰਾ, ਖਾਲੀ **ਮੁੱਲਾਂ, ਖਾਲੀ *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  ਖਾਲੀ **avalue;
  ਪੂਰਨ i, areg;

  cif = ਘੇਰਾ->cif;
  ਜੇ (cif->abi != FFI_SYSV)
    ਵਾਪਸ FFI_BAD_ABI;

  areg = 0;

  ਪੂਰਨ rtype = cif->rtype->type;
  ਜੇ (rtype == FFI_TYPE_STRUCT && cif->rtype->ਮਾਪ > 4 * 4)
  {
    rvalue = *ਮੁੱਲਾਂ;
    areg++;
  }

  cif = ਘੇਰਾ->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * ਆਕਾਰ(ਖਾਲੀ *));

  ਲਈ (i = 0; i < cif->nargs; i++)
  {
    ਜੇ (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    ਜੇ (areg == FFI_ਰਜਿਸਟਰ_NARGS)
      areg = (FFI_ਰਜਿਸਟਰ_ਦਲੀਲਾਂ_SPACE + 32) / 4;

    ਜੇ (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      ਪੂਰਨ numregs = ((arg_types[i]->ਮਾਪ + 3) & ~3) / 4;
      ਜੇ (areg < FFI_ਰਜਿਸਟਰ_NARGS && areg + numregs > FFI_ਰਜਿਸਟਰ_NARGS)
        areg = (FFI_ਰਜਿਸਟਰ_ਦਲੀਲਾਂ_SPACE + 32) / 4;
    }

    avalue[i] = &ਮੁੱਲਾਂ[areg];
    areg += (arg_types[i]->ਮਾਪ + 3) / 4;
  }

  (ਘੇਰਾ->fun)(cif, rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);

  ਵਾਪਸ rtype;
}
