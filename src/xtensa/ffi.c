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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

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


ബാഹ്യ ശൂന്യം ffi_call_SYSV(ശൂന്യം *rvalue, unsigned rsize, unsigned flags,
			  ശൂന്യം(*fn)(ശൂന്യം), unsigned nbytes, extended_cif*);
ബാഹ്യ ശൂന്യം ffi_closure_SYSV(ശൂന്യം) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  തിരഞ്ഞെടുപ്പ്(cif->rtype->type) {
    അവസ്ഥ FFI_TYPE_SINT8:
    അവസ്ഥ FFI_TYPE_UINT8:
    അവസ്ഥ FFI_TYPE_SINT16:
    അവസ്ഥ FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_VOID:
    അവസ്ഥ FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_DOUBLE:
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      എങ്കിൽ (cif->rtype->size > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      cif->flags = FFI_TYPE_UINT32;
      നിർത്തുക;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  എങ്കിൽ (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  അല്ലെങ്കിൽ
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_അട്ടി_ALIGNMENT);
  തിരികെ FFI_OK;
}

ശൂന്യം ffi_prep_args(extended_cif *ecif, unsigned അക്ഷരം* അട്ടി)
{
  unsigned പൂർണ്ണസംഖ്യ i;
  unsigned long *addr;
  ffi_type **ptr;

  സംഘം {
    ശൂന്യം **v;
    അക്ഷരം **c;
    signed അക്ഷരം **sc;
    unsigned അക്ഷരം **uc;
    signed short **ss;
    unsigned short **us;
    unsigned പൂർണ്ണസംഖ്യ **i;
    long long **ll;
    ദശാംശം **f;
    ഇരട്ട **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((unsigned long) അട്ടി & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (unsigned long*)അട്ടി;

  /* structures with a size greater than 16 bytes are passed in memory */
  എങ്കിൽ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 16)
  {
    *addr++ = (unsigned long)ecif->rvalue;
  }

  വേണ്ടി (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    തിരഞ്ഞെടുപ്പ് ((*ptr)->type)
    {
      അവസ്ഥ FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        നിർത്തുക;
      അവസ്ഥ FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        നിർത്തുക;
      അവസ്ഥ FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        നിർത്തുക;
      അവസ്ഥ FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        നിർത്തുക;
      അവസ്ഥ FFI_TYPE_FLOAT:
      അവസ്ഥ FFI_TYPE_INT:
      അവസ്ഥ FFI_TYPE_UINT32:
      അവസ്ഥ FFI_TYPE_SINT32:
      അവസ്ഥ FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        നിർത്തുക;
      അവസ്ഥ FFI_TYPE_DOUBLE:
      അവസ്ഥ FFI_TYPE_UINT64:
      അവസ്ഥ FFI_TYPE_SINT64:
        എങ്കിൽ (((unsigned long)addr & 4) != 0)
          addr++;
        *(unsigned long long*)addr = **p_argv.ll;
	addr += വലുപ്പം(unsigned long long) / വലുപ്പം (addr);
        നിർത്തുക;

      അവസ്ഥ FFI_TYPE_STRUCT:
      {
        unsigned long offs;
        unsigned long size;

        എങ്കിൽ (((unsigned long)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (unsigned long) addr - (unsigned long) അട്ടി;
        size = (*ptr)->size;

        /* Entire structure must fit the argument registers or referenced */
        എങ്കിൽ (offs < FFI_REGISTER_NARGS * 4
            && offs + size > FFI_REGISTER_NARGS * 4)
          addr = (unsigned long*) (അട്ടി + FFI_REGISTER_NARGS * 4);

        memcpy((അക്ഷരം*) addr, *p_argv.c, size);
        addr += (size + 3) / 4;
        നിർത്തുക;
      }

      സ്ഥിരസ്ഥിതി:
        FFI_ASSERT(0);
    }
  }
}


ശൂന്യം ffi_call(ffi_cif* cif, ശൂന്യം(*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;
  unsigned long rsize = cif->rtype->size;
  പൂർണ്ണസംഖ്യ flags = cif->flags;
  ശൂന്യം *alloc = ശൂന്യ;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  എങ്കിൽ (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == ശൂന്യ))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  അല്ലെങ്കിൽ
  {
    ecif.rvalue = rvalue;
  }

  എങ്കിൽ (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  എങ്കിൽ (alloc != ശൂന്യ && rvalue != ശൂന്യ)
    memcpy(rvalue, alloc, rsize);
}

ബാഹ്യ ശൂന്യം ffi_trampoline();
ബാഹ്യ ശൂന്യം ffi_cacheflush(ശൂന്യം* start, ശൂന്യം* end);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
                      ശൂന്യം *user_data,
                      ശൂന്യം *codeloc)
{
  എങ്കിൽ (cif->abi != FFI_SYSV)
    തിരികെ FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  memcpy(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_SIZE);
  *(unsigned പൂർണ്ണസംഖ്യ*)(&closure->tramp[8]) = (unsigned പൂർണ്ണസംഖ്യ)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_SIZE);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  തിരികെ FFI_OK; 
}


long FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, ശൂന്യം **മൂല്യങ്ങൾ, ശൂന്യം *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  ശൂന്യം **avalue;
  പൂർണ്ണസംഖ്യ i, areg;

  cif = closure->cif;
  എങ്കിൽ (cif->abi != FFI_SYSV)
    തിരികെ FFI_BAD_ABI;

  areg = 0;

  പൂർണ്ണസംഖ്യ rtype = cif->rtype->type;
  എങ്കിൽ (rtype == FFI_TYPE_STRUCT && cif->rtype->size > 4 * 4)
  {
    rvalue = *മൂല്യങ്ങൾ;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * വലുപ്പം(ശൂന്യം *));

  വേണ്ടി (i = 0; i < cif->nargs; i++)
  {
    എങ്കിൽ (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    എങ്കിൽ (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    എങ്കിൽ (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      പൂർണ്ണസംഖ്യ numregs = ((arg_types[i]->size + 3) & ~3) / 4;
      എങ്കിൽ (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &മൂല്യങ്ങൾ[areg];
    areg += (arg_types[i]->size + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_data);

  തിരികെ rtype;
}
