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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

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


ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ *rvalue, unsigned rsize, unsigned flags,
			  ಶೂನ್ಯ(*fn)(ಶೂನ್ಯ), unsigned nbytes, extended_cif*);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_SYSV(ಶೂನ್ಯ) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ವಿಕಲ್ಪ(cif->rtype->type) {
    ಪಕ್ಷ FFI_TYPE_SINT8:
    ಪಕ್ಷ FFI_TYPE_UINT8:
    ಪಕ್ಷ FFI_TYPE_SINT16:
    ಪಕ್ಷ FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_VOID:
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      ಯದಿ (cif->rtype->size > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      cif->flags = FFI_TYPE_UINT32;
      ಭಙ್ಗ;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  ಯದಿ (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  ಅನ್ಯಥಾ
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_ರಾಶಿ_ALIGNMENT);
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಶೂನ್ಯ ffi_prep_args(extended_cif *ecif, unsigned ವರ್ಣ* ರಾಶಿ)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  unsigned long *addr;
  ffi_type **ptr;

  ಸಙ್ಘ {
    ಶೂನ್ಯ **v;
    ವರ್ಣ **c;
    signed ವರ್ಣ **sc;
    unsigned ವರ್ಣ **uc;
    signed short **ss;
    unsigned short **us;
    unsigned ಪೂರ್ಣಾಙ್ಕ **i;
    long long **ll;
    ಭಿನ್ನ **f;
    ದ್ವಿಗುಣ **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((unsigned long) ರಾಶಿ & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (unsigned long*)ರಾಶಿ;

  /* structures with a size greater than 16 bytes are passed in memory */
  ಯದಿ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 16)
  {
    *addr++ = (unsigned long)ecif->rvalue;
  }

  ಪ್ರತಿ (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    ವಿಕಲ್ಪ ((*ptr)->type)
    {
      ಪಕ್ಷ FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_FLOAT:
      ಪಕ್ಷ FFI_TYPE_INT:
      ಪಕ್ಷ FFI_TYPE_UINT32:
      ಪಕ್ಷ FFI_TYPE_SINT32:
      ಪಕ್ಷ FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_DOUBLE:
      ಪಕ್ಷ FFI_TYPE_UINT64:
      ಪಕ್ಷ FFI_TYPE_SINT64:
        ಯದಿ (((unsigned long)addr & 4) != 0)
          addr++;
        *(unsigned long long*)addr = **p_argv.ll;
	addr += ಪರಿಮಾಣ(unsigned long long) / ಪರಿಮಾಣ (addr);
        ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_STRUCT:
      {
        unsigned long offs;
        unsigned long size;

        ಯದಿ (((unsigned long)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (unsigned long) addr - (unsigned long) ರಾಶಿ;
        size = (*ptr)->size;

        /* Entire structure must fit the argument registers or referenced */
        ಯದಿ (offs < FFI_REGISTER_NARGS * 4
            && offs + size > FFI_REGISTER_NARGS * 4)
          addr = (unsigned long*) (ರಾಶಿ + FFI_REGISTER_NARGS * 4);

        memcpy((ವರ್ಣ*) addr, *p_argv.c, size);
        addr += (size + 3) / 4;
        ಭಙ್ಗ;
      }

      ಸ್ವಭಾವ:
        FFI_ASSERT(0);
    }
  }
}


ಶೂನ್ಯ ffi_call(ffi_cif* cif, ಶೂನ್ಯ(*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;
  unsigned long rsize = cif->rtype->size;
  ಪೂರ್ಣಾಙ್ಕ flags = cif->flags;
  ಶೂನ್ಯ *alloc = ರಿಕ್ತ;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  ಯದಿ (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == ರಿಕ್ತ))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  ಅನ್ಯಥಾ
  {
    ecif.rvalue = rvalue;
  }

  ಯದಿ (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  ಯದಿ (alloc != ರಿಕ್ತ && rvalue != ರಿಕ್ತ)
    memcpy(rvalue, alloc, rsize);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_trampoline();
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_cacheflush(ಶೂನ್ಯ* start, ಶೂನ್ಯ* end);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
                      ಶೂನ್ಯ *user_data,
                      ಶೂನ್ಯ *codeloc)
{
  ಯದಿ (cif->abi != FFI_SYSV)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  memcpy(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_SIZE);
  *(unsigned ಪೂರ್ಣಾಙ್ಕ*)(&closure->tramp[8]) = (unsigned ಪೂರ್ಣಾಙ್ಕ)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_SIZE);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  ಪ್ರತ್ಯಾಗಮ FFI_OK; 
}


long FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, ಶೂನ್ಯ **ಮೌಲ್ಯಗಳು, ಶೂನ್ಯ *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  ಶೂನ್ಯ **avalue;
  ಪೂರ್ಣಾಙ್ಕ i, areg;

  cif = closure->cif;
  ಯದಿ (cif->abi != FFI_SYSV)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  areg = 0;

  ಪೂರ್ಣಾಙ್ಕ rtype = cif->rtype->type;
  ಯದಿ (rtype == FFI_TYPE_STRUCT && cif->rtype->size > 4 * 4)
  {
    rvalue = *ಮೌಲ್ಯಗಳು;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ *));

  ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
  {
    ಯದಿ (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    ಯದಿ (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    ಯದಿ (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      ಪೂರ್ಣಾಙ್ಕ numregs = ((arg_types[i]->size + 3) & ~3) / 4;
      ಯದಿ (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &ಮೌಲ್ಯಗಳು[areg];
    areg += (arg_types[i]->size + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_data);

  ಪ್ರತ್ಯಾಗಮ rtype;
}
