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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

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


ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV(ଶୂନ୍ୟ *rvalue, unsigned rsize, unsigned flags,
			  ଶୂନ୍ୟ(*fn)(ଶୂନ୍ୟ), unsigned nbytes, extended_cif*);
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_SYSV(ଶୂନ୍ୟ) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ବାଛ(cif->rtype->type) {
    ସ୍ଥିତି FFI_TYPE_SINT8:
    ସ୍ଥିତି FFI_TYPE_UINT8:
    ସ୍ଥିତି FFI_TYPE_SINT16:
    ସ୍ଥିତି FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_VOID:
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      ଯଦି (cif->rtype->size > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      cif->flags = FFI_TYPE_UINT32;
      ଭାଙ୍ଗ;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  ଯଦି (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  ଅନ୍ୟଥା
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_ଗଦା_ALIGNMENT);
  ଫେରନ୍ତୁ FFI_OK;
}

ଶୂନ୍ୟ ffi_prep_args(extended_cif *ecif, unsigned ଅକ୍ଷର* ଗଦା)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  unsigned long *addr;
  ffi_type **ptr;

  ସଂଘ {
    ଶୂନ୍ୟ **v;
    ଅକ୍ଷର **c;
    signed ଅକ୍ଷର **sc;
    unsigned ଅକ୍ଷର **uc;
    signed short **ss;
    unsigned short **us;
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା **i;
    long long **ll;
    ଦଶମିକ **f;
    ଦ୍ୱିଗୁଣ **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((unsigned long) ଗଦା & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (unsigned long*)ଗଦା;

  /* structures with a size greater than 16 bytes are passed in memory */
  ଯଦି (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 16)
  {
    *addr++ = (unsigned long)ecif->rvalue;
  }

  ପାଇଁ (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    ବାଛ ((*ptr)->type)
    {
      ସ୍ଥିତି FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_FLOAT:
      ସ୍ଥିତି FFI_TYPE_INT:
      ସ୍ଥିତି FFI_TYPE_UINT32:
      ସ୍ଥିତି FFI_TYPE_SINT32:
      ସ୍ଥିତି FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_DOUBLE:
      ସ୍ଥିତି FFI_TYPE_UINT64:
      ସ୍ଥିତି FFI_TYPE_SINT64:
        ଯଦି (((unsigned long)addr & 4) != 0)
          addr++;
        *(unsigned long long*)addr = **p_argv.ll;
	addr += ଆକାର(unsigned long long) / ଆକାର (addr);
        ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_STRUCT:
      {
        unsigned long offs;
        unsigned long size;

        ଯଦି (((unsigned long)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (unsigned long) addr - (unsigned long) ଗଦା;
        size = (*ptr)->size;

        /* Entire structure must fit the argument registers or referenced */
        ଯଦି (offs < FFI_REGISTER_NARGS * 4
            && offs + size > FFI_REGISTER_NARGS * 4)
          addr = (unsigned long*) (ଗଦା + FFI_REGISTER_NARGS * 4);

        memcpy((ଅକ୍ଷର*) addr, *p_argv.c, size);
        addr += (size + 3) / 4;
        ଭାଙ୍ଗ;
      }

      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        FFI_ASSERT(0);
    }
  }
}


ଶୂନ୍ୟ ffi_call(ffi_cif* cif, ଶୂନ୍ୟ(*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;
  unsigned long rsize = cif->rtype->size;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = cif->flags;
  ଶୂନ୍ୟ *alloc = ଖାଲି;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  ଯଦି (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == ଖାଲି))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  ଅନ୍ୟଥା
  {
    ecif.rvalue = rvalue;
  }

  ଯଦି (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  ଯଦି (alloc != ଖାଲି && rvalue != ଖାଲି)
    memcpy(rvalue, alloc, rsize);
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_trampoline();
ବାହ୍ୟ ଶୂନ୍ୟ ffi_cacheflush(ଶୂନ୍ୟ* start, ଶୂନ୍ୟ* end);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
                      ଶୂନ୍ୟ *user_data,
                      ଶୂନ୍ୟ *codeloc)
{
  ଯଦି (cif->abi != FFI_SYSV)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  memcpy(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_SIZE);
  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)(&closure->tramp[8]) = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_SIZE);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  ଫେରନ୍ତୁ FFI_OK; 
}


long FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, ଶୂନ୍ୟ **ମୂଲ୍ୟଗୁଡ଼ିକ, ଶୂନ୍ୟ *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  ଶୂନ୍ୟ **avalue;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, areg;

  cif = closure->cif;
  ଯଦି (cif->abi != FFI_SYSV)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  areg = 0;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rtype = cif->rtype->type;
  ଯଦି (rtype == FFI_TYPE_STRUCT && cif->rtype->size > 4 * 4)
  {
    rvalue = *ମୂଲ୍ୟଗୁଡ଼ିକ;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * ଆକାର(ଶୂନ୍ୟ *));

  ପାଇଁ (i = 0; i < cif->nargs; i++)
  {
    ଯଦି (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    ଯଦି (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    ଯଦି (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା numregs = ((arg_types[i]->size + 3) & ~3) / 4;
      ଯଦି (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &ମୂଲ୍ୟଗୁଡ଼ିକ[areg];
    areg += (arg_types[i]->size + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_data);

  ଫେରନ୍ତୁ rtype;
}
