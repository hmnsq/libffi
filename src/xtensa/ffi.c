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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

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


வெளி வெறுமை ffi_call_SYSV(வெறுமை *rvalue, unsigned rsize, unsigned flags,
			  வெறுமை(*fn)(வெறுமை), unsigned nbytes, extended_cif*);
வெளி வெறுமை ffi_closure_SYSV(வெறுமை) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  தேர்வு(cif->rtype->type) {
    நிலை FFI_TYPE_SINT8:
    நிலை FFI_TYPE_UINT8:
    நிலை FFI_TYPE_SINT16:
    நிலை FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      நிறுத்து;
    நிலை FFI_TYPE_VOID:
    நிலை FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      நிறுத்து;
    நிலை FFI_TYPE_DOUBLE:
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      நிறுத்து;
    நிலை FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      எனில் (cif->rtype->size > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      நிறுத்து;

    இயல்பு:
      cif->flags = FFI_TYPE_UINT32;
      நிறுத்து;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  எனில் (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  இல்லையெனில்
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_அடுக்கு_ALIGNMENT);
  திரும்பு FFI_OK;
}

வெறுமை ffi_prep_args(extended_cif *ecif, unsigned எழுத்து* அடுக்கு)
{
  unsigned முழு i;
  unsigned long *addr;
  ffi_type **ptr;

  ஒன்றிப்பு {
    வெறுமை **v;
    எழுத்து **c;
    signed எழுத்து **sc;
    unsigned எழுத்து **uc;
    signed short **ss;
    unsigned short **us;
    unsigned முழு **i;
    long long **ll;
    தசம **f;
    இரட்டை **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((unsigned long) அடுக்கு & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (unsigned long*)அடுக்கு;

  /* structures with a size greater than 16 bytes are passed in memory */
  எனில் (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 16)
  {
    *addr++ = (unsigned long)ecif->rvalue;
  }

  க்கு (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    தேர்வு ((*ptr)->type)
    {
      நிலை FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        நிறுத்து;
      நிலை FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        நிறுத்து;
      நிலை FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        நிறுத்து;
      நிலை FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        நிறுத்து;
      நிலை FFI_TYPE_FLOAT:
      நிலை FFI_TYPE_INT:
      நிலை FFI_TYPE_UINT32:
      நிலை FFI_TYPE_SINT32:
      நிலை FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        நிறுத்து;
      நிலை FFI_TYPE_DOUBLE:
      நிலை FFI_TYPE_UINT64:
      நிலை FFI_TYPE_SINT64:
        எனில் (((unsigned long)addr & 4) != 0)
          addr++;
        *(unsigned long long*)addr = **p_argv.ll;
	addr += அளவு(unsigned long long) / அளவு (addr);
        நிறுத்து;

      நிலை FFI_TYPE_STRUCT:
      {
        unsigned long offs;
        unsigned long size;

        எனில் (((unsigned long)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (unsigned long) addr - (unsigned long) அடுக்கு;
        size = (*ptr)->size;

        /* Entire structure must fit the argument registers or referenced */
        எனில் (offs < FFI_REGISTER_NARGS * 4
            && offs + size > FFI_REGISTER_NARGS * 4)
          addr = (unsigned long*) (அடுக்கு + FFI_REGISTER_NARGS * 4);

        memcpy((எழுத்து*) addr, *p_argv.c, size);
        addr += (size + 3) / 4;
        நிறுத்து;
      }

      இயல்பு:
        FFI_ASSERT(0);
    }
  }
}


வெறுமை ffi_call(ffi_cif* cif, வெறுமை(*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  extended_cif ecif;
  unsigned long rsize = cif->rtype->size;
  முழு flags = cif->flags;
  வெறுமை *alloc = வெற்று;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  எனில் (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == வெற்று))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  இல்லையெனில்
  {
    ecif.rvalue = rvalue;
  }

  எனில் (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  எனில் (alloc != வெற்று && rvalue != வெற்று)
    memcpy(rvalue, alloc, rsize);
}

வெளி வெறுமை ffi_trampoline();
வெளி வெறுமை ffi_cacheflush(வெறுமை* start, வெறுமை* end);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
                      வெறுமை *user_data,
                      வெறுமை *codeloc)
{
  எனில் (cif->abi != FFI_SYSV)
    திரும்பு FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  memcpy(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_SIZE);
  *(unsigned முழு*)(&closure->tramp[8]) = (unsigned முழு)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_SIZE);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  திரும்பு FFI_OK; 
}


long FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, வெறுமை **மதிப்புகள், வெறுமை *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  வெறுமை **avalue;
  முழு i, areg;

  cif = closure->cif;
  எனில் (cif->abi != FFI_SYSV)
    திரும்பு FFI_BAD_ABI;

  areg = 0;

  முழு rtype = cif->rtype->type;
  எனில் (rtype == FFI_TYPE_STRUCT && cif->rtype->size > 4 * 4)
  {
    rvalue = *மதிப்புகள்;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * அளவு(வெறுமை *));

  க்கு (i = 0; i < cif->nargs; i++)
  {
    எனில் (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    எனில் (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    எனில் (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      முழு numregs = ((arg_types[i]->size + 3) & ~3) / 4;
      எனில் (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &மதிப்புகள்[areg];
    areg += (arg_types[i]->size + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_data);

  திரும்பு rtype;
}
