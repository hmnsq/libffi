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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>

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


বাহ্য শূন্য ffi_call_SYSV(শূন্য *rvalue, unsigned rsize, unsigned flags,
			  শূন্য(*fn)(শূন্য), unsigned nbytes, extended_cif*);
বাহ্য শূন্য ffi_closure_SYSV(শূন্য) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ৱিকল্প(cif->rtype->type) {
    পক্ষ FFI_TYPE_SINT8:
    পক্ষ FFI_TYPE_UINT8:
    পক্ষ FFI_TYPE_SINT16:
    পক্ষ FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      ভঙ্গ;
    পক্ষ FFI_TYPE_VOID:
    পক্ষ FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      ভঙ্গ;
    পক্ষ FFI_TYPE_DOUBLE:
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      ভঙ্গ;
    পক্ষ FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      যদি (cif->rtype->size > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      ভঙ্গ;

    স্ৱভাৱ:
      cif->flags = FFI_TYPE_UINT32;
      ভঙ্গ;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  যদি (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  অন্যথা
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_স্তূপ_ALIGNMENT);
  ঘূৰাই FFI_OK;
}

শূন্য ffi_prep_args(extended_cif *ecif, unsigned ৱর্ণ* স্তূপ)
{
  unsigned পূর্ণাঙ্ক i;
  unsigned long *addr;
  ffi_type **ptr;

  সঙ্ঘ {
    শূন্য **v;
    ৱর্ণ **c;
    signed ৱর্ণ **sc;
    unsigned ৱর্ণ **uc;
    signed short **ss;
    unsigned short **us;
    unsigned পূর্ণাঙ্ক **i;
    long long **ll;
    ভিন্ন **f;
    দ্ৱিগুণ **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((unsigned long) স্তূপ & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (unsigned long*)স্তূপ;

  /* structures with a size greater than 16 bytes are passed in memory */
  যদি (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 16)
  {
    *addr++ = (unsigned long)ecif->rvalue;
  }

  বাবে (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    ৱিকল্প ((*ptr)->type)
    {
      পক্ষ FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        ভঙ্গ;
      পক্ষ FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        ভঙ্গ;
      পক্ষ FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        ভঙ্গ;
      পক্ষ FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        ভঙ্গ;
      পক্ষ FFI_TYPE_FLOAT:
      পক্ষ FFI_TYPE_INT:
      পক্ষ FFI_TYPE_UINT32:
      পক্ষ FFI_TYPE_SINT32:
      পক্ষ FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        ভঙ্গ;
      পক্ষ FFI_TYPE_DOUBLE:
      পক্ষ FFI_TYPE_UINT64:
      পক্ষ FFI_TYPE_SINT64:
        যদি (((unsigned long)addr & 4) != 0)
          addr++;
        *(unsigned long long*)addr = **p_argv.ll;
	addr += পরিমাণ(unsigned long long) / পরিমাণ (addr);
        ভঙ্গ;

      পক্ষ FFI_TYPE_STRUCT:
      {
        unsigned long offs;
        unsigned long size;

        যদি (((unsigned long)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (unsigned long) addr - (unsigned long) স্তূপ;
        size = (*ptr)->size;

        /* Entire structure must fit the argument registers or referenced */
        যদি (offs < FFI_REGISTER_NARGS * 4
            && offs + size > FFI_REGISTER_NARGS * 4)
          addr = (unsigned long*) (স্তূপ + FFI_REGISTER_NARGS * 4);

        memcpy((ৱর্ণ*) addr, *p_argv.c, size);
        addr += (size + 3) / 4;
        ভঙ্গ;
      }

      স্ৱভাৱ:
        FFI_ASSERT(0);
    }
  }
}


শূন্য ffi_call(ffi_cif* cif, শূন্য(*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;
  unsigned long rsize = cif->rtype->size;
  পূর্ণাঙ্ক flags = cif->flags;
  শূন্য *alloc = রিক্ত;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  যদি (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == রিক্ত))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  অন্যথা
  {
    ecif.rvalue = rvalue;
  }

  যদি (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  যদি (alloc != রিক্ত && rvalue != রিক্ত)
    memcpy(rvalue, alloc, rsize);
}

বাহ্য শূন্য ffi_trampoline();
বাহ্য শূন্য ffi_cacheflush(শূন্য* start, শূন্য* end);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
                      শূন্য *user_data,
                      শূন্য *codeloc)
{
  যদি (cif->abi != FFI_SYSV)
    ঘূৰাই FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  memcpy(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_SIZE);
  *(unsigned পূর্ণাঙ্ক*)(&closure->tramp[8]) = (unsigned পূর্ণাঙ্ক)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_SIZE);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  ঘূৰাই FFI_OK; 
}


long FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, শূন্য **মানসমূহ, শূন্য *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  শূন্য **avalue;
  পূর্ণাঙ্ক i, areg;

  cif = closure->cif;
  যদি (cif->abi != FFI_SYSV)
    ঘূৰাই FFI_BAD_ABI;

  areg = 0;

  পূর্ণাঙ্ক rtype = cif->rtype->type;
  যদি (rtype == FFI_TYPE_STRUCT && cif->rtype->size > 4 * 4)
  {
    rvalue = *মানসমূহ;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * পরিমাণ(শূন্য *));

  বাবে (i = 0; i < cif->nargs; i++)
  {
    যদি (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    যদি (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    যদি (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      পূর্ণাঙ্ক numregs = ((arg_types[i]->size + 3) & ~3) / 4;
      যদি (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &মানসমূহ[areg];
    areg += (arg_types[i]->size + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_data);

  ঘূৰাই rtype;
}
