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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

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


#define FFI_TYPE_STRUCT_REGS FFI_TYPE_LAST


બાહ્ય શૂન્ય ffi_call_SYSV(શૂન્ય *rvalue, unsigned rsize, unsigned flags,
			  શૂન્ય(*fn)(શૂન્ય), unsigned nbytes, extended_cif*);
બાહ્ય શૂન્ય ffi_closure_SYSV(શૂન્ય) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  પસંદગી(cif->rtype->type) {
    સ્થિતિ FFI_TYPE_SINT8:
    સ્થિતિ FFI_TYPE_UINT8:
    સ્થિતિ FFI_TYPE_SINT16:
    સ્થિતિ FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      તોડો;
    સ્થિતિ FFI_TYPE_VOID:
    સ્થિતિ FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      તોડો;
    સ્થિતિ FFI_TYPE_DOUBLE:
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      તોડો;
    સ્થિતિ FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      જો (cif->rtype->size > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      તોડો;

    મૂળભૂત:
      cif->flags = FFI_TYPE_UINT32;
      તોડો;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  જો (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  નહીંતર
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_ઢગલો_ALIGNMENT);
  પાછા FFI_OK;
}

શૂન્ય ffi_prep_args(extended_cif *ecif, unsigned અક્ષર* ઢગલો)
{
  unsigned પૂર્ણાંક i;
  unsigned long *addr;
  ffi_type **ptr;

  સંઘ {
    શૂન્ય **v;
    અક્ષર **c;
    signed અક્ષર **sc;
    unsigned અક્ષર **uc;
    signed short **ss;
    unsigned short **us;
    unsigned પૂર્ણાંક **i;
    long long **ll;
    દશાંશ **f;
    દ્વિગુણ **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((unsigned long) ઢગલો & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (unsigned long*)ઢગલો;

  /* structures with a size greater than 16 bytes are passed in memory */
  જો (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 16)
  {
    *addr++ = (unsigned long)ecif->rvalue;
  }

  માટે (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    પસંદગી ((*ptr)->type)
    {
      સ્થિતિ FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        તોડો;
      સ્થિતિ FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        તોડો;
      સ્થિતિ FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        તોડો;
      સ્થિતિ FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        તોડો;
      સ્થિતિ FFI_TYPE_FLOAT:
      સ્થિતિ FFI_TYPE_INT:
      સ્થિતિ FFI_TYPE_UINT32:
      સ્થિતિ FFI_TYPE_SINT32:
      સ્થિતિ FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        તોડો;
      સ્થિતિ FFI_TYPE_DOUBLE:
      સ્થિતિ FFI_TYPE_UINT64:
      સ્થિતિ FFI_TYPE_SINT64:
        જો (((unsigned long)addr & 4) != 0)
          addr++;
        *(unsigned long long*)addr = **p_argv.ll;
	addr += કદ(unsigned long long) / કદ (addr);
        તોડો;

      સ્થિતિ FFI_TYPE_STRUCT:
      {
        unsigned long offs;
        unsigned long size;

        જો (((unsigned long)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (unsigned long) addr - (unsigned long) ઢગલો;
        size = (*ptr)->size;

        /* Entire structure must fit the argument registers or referenced */
        જો (offs < FFI_REGISTER_NARGS * 4
            && offs + size > FFI_REGISTER_NARGS * 4)
          addr = (unsigned long*) (ઢગલો + FFI_REGISTER_NARGS * 4);

        memcpy((અક્ષર*) addr, *p_argv.c, size);
        addr += (size + 3) / 4;
        તોડો;
      }

      મૂળભૂત:
        FFI_ASSERT(0);
    }
  }
}


શૂન્ય ffi_call(ffi_cif* cif, શૂન્ય(*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  extended_cif ecif;
  unsigned long rsize = cif->rtype->size;
  પૂર્ણાંક flags = cif->flags;
  શૂન્ય *alloc = ખાલી;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  જો (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == ખાલી))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  નહીંતર
  {
    ecif.rvalue = rvalue;
  }

  જો (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  જો (alloc != ખાલી && rvalue != ખાલી)
    memcpy(rvalue, alloc, rsize);
}

બાહ્ય શૂન્ય ffi_trampoline();
બાહ્ય શૂન્ય ffi_cacheflush(શૂન્ય* start, શૂન્ય* end);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
                      શૂન્ય *user_data,
                      શૂન્ય *codeloc)
{
  જો (cif->abi != FFI_SYSV)
    પાછા FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  memcpy(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_SIZE);
  *(unsigned પૂર્ણાંક*)(&closure->tramp[8]) = (unsigned પૂર્ણાંક)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_SIZE);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  પાછા FFI_OK; 
}


long FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, શૂન્ય **કિંમતો, શૂન્ય *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  શૂન્ય **avalue;
  પૂર્ણાંક i, areg;

  cif = closure->cif;
  જો (cif->abi != FFI_SYSV)
    પાછા FFI_BAD_ABI;

  areg = 0;

  પૂર્ણાંક rtype = cif->rtype->type;
  જો (rtype == FFI_TYPE_STRUCT && cif->rtype->size > 4 * 4)
  {
    rvalue = *કિંમતો;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * કદ(શૂન્ય *));

  માટે (i = 0; i < cif->nargs; i++)
  {
    જો (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    જો (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    જો (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      પૂર્ણાંક numregs = ((arg_types[i]->size + 3) & ~3) / 4;
      જો (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &કિંમતો[areg];
    areg += (arg_types[i]->size + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_data);

  પાછા rtype;
}
