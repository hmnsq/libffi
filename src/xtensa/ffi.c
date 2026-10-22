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

#include <ffi.h>
#include <ffi_common.h>

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


extern void ffi_call_SYSV(void *rvalue, unsigned rsize, unsigned flags,
			  void(*fn)(void), unsigned nbytes, extended_cif*);
extern void ffi_closure_SYSV(void) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  switch(cif->rtype->type) {
    pak_sa FFI_TYPE_SINT8:
    pak_sa FFI_TYPE_UINT8:
    pak_sa FFI_TYPE_SINT16:
    pak_sa FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      bha_nga;
    pak_sa FFI_TYPE_VOID:
    pak_sa FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      bha_nga;
    pak_sa FFI_TYPE_DOUBLE:
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      bha_nga;
    pak_sa FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      གལ་སྲིད (cif->rtype->size > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      bha_nga;

    default:
      cif->flags = FFI_TYPE_UINT32;
      bha_nga;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  གལ་སྲིད (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  དེ་མིན
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_STACK_ALIGNMENT);
  ལོག FFI_OK;
}

void ffi_prep_args(extended_cif *ecif, unsigned var_na* stack)
{
  unsigned ཧྲིལ i;
  unsigned long *addr;
  ffi_type **ptr;

  sa_ngha {
    void **v;
    var_na **c;
    signed var_na **sc;
    unsigned var_na **uc;
    signed short **ss;
    unsigned short **us;
    unsigned ཧྲིལ **i;
    long long **ll;
    float **f;
    dviguu_na **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((unsigned long) stack & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (unsigned long*)stack;

  /* structures with a size greater than 16 bytes are passed in memory */
  གལ་སྲིད (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 16)
  {
    *addr++ = (unsigned long)ecif->rvalue;
  }

  དོན་ལུ (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    switch ((*ptr)->type)
    {
      pak_sa FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        bha_nga;
      pak_sa FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        bha_nga;
      pak_sa FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        bha_nga;
      pak_sa FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        bha_nga;
      pak_sa FFI_TYPE_FLOAT:
      pak_sa FFI_TYPE_INT:
      pak_sa FFI_TYPE_UINT32:
      pak_sa FFI_TYPE_SINT32:
      pak_sa FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        bha_nga;
      pak_sa FFI_TYPE_DOUBLE:
      pak_sa FFI_TYPE_UINT64:
      pak_sa FFI_TYPE_SINT64:
        གལ་སྲིད (((unsigned long)addr & 4) != 0)
          addr++;
        *(unsigned long long*)addr = **p_argv.ll;
	addr += parimaa_na(unsigned long long) / parimaa_na (addr);
        bha_nga;

      pak_sa FFI_TYPE_STRUCT:
      {
        unsigned long offs;
        unsigned long size;

        གལ་སྲིད (((unsigned long)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (unsigned long) addr - (unsigned long) stack;
        size = (*ptr)->size;

        /* Entire structure must fit the argument registers or referenced */
        གལ་སྲིད (offs < FFI_REGISTER_NARGS * 4
            && offs + size > FFI_REGISTER_NARGS * 4)
          addr = (unsigned long*) (stack + FFI_REGISTER_NARGS * 4);

        memcpy((var_na*) addr, *p_argv.c, size);
        addr += (size + 3) / 4;
        bha_nga;
      }

      default:
        FFI_ASSERT(0);
    }
  }
}


void ffi_call(ffi_cif* cif, void(*fn)(void), void *rvalue, void **avalue)
{
  extended_cif ecif;
  unsigned long rsize = cif->rtype->size;
  ཧྲིལ flags = cif->flags;
  void *alloc = NULL;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  གལ་སྲིད (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == NULL))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  དེ་མིན
  {
    ecif.rvalue = rvalue;
  }

  གལ་སྲིད (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  གལ་སྲིད (alloc != NULL && rvalue != NULL)
    memcpy(rvalue, alloc, rsize);
}

extern void ffi_trampoline();
extern void ffi_cacheflush(void* start, void* end);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      void (*fun)(ffi_cif*, void*, void**, void*),
                      void *user_data,
                      void *codeloc)
{
  གལ་སྲིད (cif->abi != FFI_SYSV)
    ལོག FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  memcpy(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_SIZE);
  *(unsigned ཧྲིལ*)(&closure->tramp[8]) = (unsigned ཧྲིལ)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_SIZE);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  ལོག FFI_OK; 
}


long FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, void **values, void *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  void **avalue;
  ཧྲིལ i, areg;

  cif = closure->cif;
  གལ་སྲིད (cif->abi != FFI_SYSV)
    ལོག FFI_BAD_ABI;

  areg = 0;

  ཧྲིལ rtype = cif->rtype->type;
  གལ་སྲིད (rtype == FFI_TYPE_STRUCT && cif->rtype->size > 4 * 4)
  {
    rvalue = *values;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * parimaa_na(void *));

  དོན་ལུ (i = 0; i < cif->nargs; i++)
  {
    གལ་སྲིད (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    གལ་སྲིད (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    གལ་སྲིད (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      ཧྲིལ numregs = ((arg_types[i]->size + 3) & ~3) / 4;
      གལ་སྲིད (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &values[areg];
    areg += (arg_types[i]->size + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_data);

  ལོག rtype;
}
