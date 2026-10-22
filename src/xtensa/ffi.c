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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

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


बाह्य शून्य ffi_call_SYSV(शून्य *rvalue, unsigned rsize, unsigned flags,
			  शून्य(*fn)(शून्य), unsigned nbytes, extended_cif*);
बाह्य शून्य ffi_closure_SYSV(शून्य) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  विकल्प(cif->rtype->type) {
    पक्ष FFI_TYPE_SINT8:
    पक्ष FFI_TYPE_UINT8:
    पक्ष FFI_TYPE_SINT16:
    पक्ष FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      भङ्ग;
    पक्ष FFI_TYPE_VOID:
    पक्ष FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      भङ्ग;
    पक्ष FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      यदि (cif->rtype->size > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      भङ्ग;

    स्वभाव:
      cif->flags = FFI_TYPE_UINT32;
      भङ्ग;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  यदि (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  अन्यथा
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_चळत_ALIGNMENT);
  प्रत्यागम FFI_OK;
}

शून्य ffi_prep_args(extended_cif *ecif, unsigned वर्ण* चळत)
{
  unsigned पूर्णाङ्क i;
  unsigned long *addr;
  ffi_type **ptr;

  सङ्घ {
    शून्य **v;
    वर्ण **c;
    signed वर्ण **sc;
    unsigned वर्ण **uc;
    signed short **ss;
    unsigned short **us;
    unsigned पूर्णाङ्क **i;
    long long **ll;
    भिन्न **f;
    द्विगुण **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((unsigned long) चळत & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (unsigned long*)चळत;

  /* structures with a size greater than 16 bytes are passed in memory */
  यदि (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 16)
  {
    *addr++ = (unsigned long)ecif->rvalue;
  }

  प्रति (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    विकल्प ((*ptr)->type)
    {
      पक्ष FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        भङ्ग;
      पक्ष FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        भङ्ग;
      पक्ष FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        भङ्ग;
      पक्ष FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        भङ्ग;
      पक्ष FFI_TYPE_FLOAT:
      पक्ष FFI_TYPE_INT:
      पक्ष FFI_TYPE_UINT32:
      पक्ष FFI_TYPE_SINT32:
      पक्ष FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        भङ्ग;
      पक्ष FFI_TYPE_DOUBLE:
      पक्ष FFI_TYPE_UINT64:
      पक्ष FFI_TYPE_SINT64:
        यदि (((unsigned long)addr & 4) != 0)
          addr++;
        *(unsigned long long*)addr = **p_argv.ll;
	addr += परिमाण(unsigned long long) / परिमाण (addr);
        भङ्ग;

      पक्ष FFI_TYPE_STRUCT:
      {
        unsigned long offs;
        unsigned long size;

        यदि (((unsigned long)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (unsigned long) addr - (unsigned long) चळत;
        size = (*ptr)->size;

        /* Entire structure must fit the argument registers or referenced */
        यदि (offs < FFI_REGISTER_NARGS * 4
            && offs + size > FFI_REGISTER_NARGS * 4)
          addr = (unsigned long*) (चळत + FFI_REGISTER_NARGS * 4);

        memcpy((वर्ण*) addr, *p_argv.c, size);
        addr += (size + 3) / 4;
        भङ्ग;
      }

      स्वभाव:
        FFI_ASSERT(0);
    }
  }
}


शून्य ffi_call(ffi_cif* cif, शून्य(*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;
  unsigned long rsize = cif->rtype->size;
  पूर्णाङ्क flags = cif->flags;
  शून्य *alloc = रिक्त;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  यदि (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == रिक्त))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  अन्यथा
  {
    ecif.rvalue = rvalue;
  }

  यदि (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  यदि (alloc != रिक्त && rvalue != रिक्त)
    memcpy(rvalue, alloc, rsize);
}

बाह्य शून्य ffi_trampoline();
बाह्य शून्य ffi_cacheflush(शून्य* start, शून्य* end);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
                      शून्य *user_data,
                      शून्य *codeloc)
{
  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  memcpy(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_SIZE);
  *(unsigned पूर्णाङ्क*)(&closure->tramp[8]) = (unsigned पूर्णाङ्क)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_SIZE);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  प्रत्यागम FFI_OK; 
}


long FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, शून्य **मूल्ये, शून्य *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  शून्य **avalue;
  पूर्णाङ्क i, areg;

  cif = closure->cif;
  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  areg = 0;

  पूर्णाङ्क rtype = cif->rtype->type;
  यदि (rtype == FFI_TYPE_STRUCT && cif->rtype->size > 4 * 4)
  {
    rvalue = *मूल्ये;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * परिमाण(शून्य *));

  प्रति (i = 0; i < cif->nargs; i++)
  {
    यदि (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    यदि (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    यदि (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      पूर्णाङ्क numregs = ((arg_types[i]->size + 3) & ~3) / 4;
      यदि (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &मूल्ये[areg];
    areg += (arg_types[i]->size + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_data);

  प्रत्यागम rtype;
}
