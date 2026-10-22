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

#شامل <ffi.h>
#شامل <ffi_common.h>

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


بیرونی تهی ffi_call_SYSV(تهی *rvalue, unsigned rsize, unsigned flags,
			  تهی(*fn)(تهی), unsigned nbytes, extended_cif*);
بیرونی تهی ffi_closure_SYSV(تهی) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  انتخاب(cif->rtype->type) {
    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      بشکن;
    حالت FFI_TYPE_VOID:
    حالت FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      بشکن;
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      بشکن;
    حالت FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      اگر (cif->rtype->size > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      بشکن;

    پیشفرض:
      cif->flags = FFI_TYPE_UINT32;
      بشکن;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  اگر (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  وگرنه
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_پشته_ALIGNMENT);
  بازگشت FFI_OK;
}

تهی ffi_prep_args(extended_cif *ecif, unsigned نویسه* پشته)
{
  unsigned صحیح i;
  unsigned long *addr;
  ffi_type **ptr;

  اجتماع {
    تهی **v;
    نویسه **c;
    signed نویسه **sc;
    unsigned نویسه **uc;
    signed short **ss;
    unsigned short **us;
    unsigned صحیح **i;
    long long **ll;
    اعشاری **f;
    دوگانه **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((unsigned long) پشته & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (unsigned long*)پشته;

  /* structures with a size greater than 16 bytes are passed in memory */
  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 16)
  {
    *addr++ = (unsigned long)ecif->rvalue;
  }

  برای (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    انتخاب ((*ptr)->type)
    {
      حالت FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        بشکن;
      حالت FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        بشکن;
      حالت FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        بشکن;
      حالت FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        بشکن;
      حالت FFI_TYPE_FLOAT:
      حالت FFI_TYPE_INT:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        بشکن;
      حالت FFI_TYPE_DOUBLE:
      حالت FFI_TYPE_UINT64:
      حالت FFI_TYPE_SINT64:
        اگر (((unsigned long)addr & 4) != 0)
          addr++;
        *(unsigned long long*)addr = **p_argv.ll;
	addr += اندازه(unsigned long long) / اندازه (addr);
        بشکن;

      حالت FFI_TYPE_STRUCT:
      {
        unsigned long offs;
        unsigned long size;

        اگر (((unsigned long)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (unsigned long) addr - (unsigned long) پشته;
        size = (*ptr)->size;

        /* Entire structure must fit the argument registers or referenced */
        اگر (offs < FFI_REGISTER_NARGS * 4
            && offs + size > FFI_REGISTER_NARGS * 4)
          addr = (unsigned long*) (پشته + FFI_REGISTER_NARGS * 4);

        memcpy((نویسه*) addr, *p_argv.c, size);
        addr += (size + 3) / 4;
        بشکن;
      }

      پیشفرض:
        FFI_ASSERT(0);
    }
  }
}


تهی ffi_call(ffi_cif* cif, تهی(*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  extended_cif ecif;
  unsigned long rsize = cif->rtype->size;
  صحیح flags = cif->flags;
  تهی *alloc = پوچ;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  اگر (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == پوچ))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  وگرنه
  {
    ecif.rvalue = rvalue;
  }

  اگر (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  اگر (alloc != پوچ && rvalue != پوچ)
    memcpy(rvalue, alloc, rsize);
}

بیرونی تهی ffi_trampoline();
بیرونی تهی ffi_cacheflush(تهی* start, تهی* end);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
                      تهی *user_data,
                      تهی *codeloc)
{
  اگر (cif->abi != FFI_SYSV)
    بازگشت FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  memcpy(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_SIZE);
  *(unsigned صحیح*)(&closure->tramp[8]) = (unsigned صحیح)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_SIZE);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  بازگشت FFI_OK; 
}


long FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, تهی **مقادیر, تهی *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  تهی **avalue;
  صحیح i, areg;

  cif = closure->cif;
  اگر (cif->abi != FFI_SYSV)
    بازگشت FFI_BAD_ABI;

  areg = 0;

  صحیح rtype = cif->rtype->type;
  اگر (rtype == FFI_TYPE_STRUCT && cif->rtype->size > 4 * 4)
  {
    rvalue = *مقادیر;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * اندازه(تهی *));

  برای (i = 0; i < cif->nargs; i++)
  {
    اگر (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    اگر (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    اگر (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      صحیح numregs = ((arg_types[i]->size + 3) & ~3) / 4;
      اگر (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &مقادیر[areg];
    areg += (arg_types[i]->size + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_data);

  بازگشت rtype;
}
