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


बाह्य शून्य ffi_call_SYSV(शून्य *rvalue, अचिन्हित rsize, अचिन्हित flags,
			  शून्य(*fn)(शून्य), अचिन्हित nbytes, extended_cif*);
बाह्य शून्य ffi_closure_SYSV(शून्य) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  निवड(cif->rtype->type) {
    स्थिती FFI_TYPE_SINT8:
    स्थिती FFI_TYPE_UINT8:
    स्थिती FFI_TYPE_SINT16:
    स्थिती FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      थांबा;
    स्थिती FFI_TYPE_VOID:
    स्थिती FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      थांबा;
    स्थिती FFI_TYPE_DOUBLE:
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      थांबा;
    स्थिती FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      जर (cif->rtype->size > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      थांबा;

    पूर्वनिर्धारित:
      cif->flags = FFI_TYPE_UINT32;
      थांबा;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  जर (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  अन्यथा
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_चळत_ALIGNMENT);
  परत FFI_OK;
}

शून्य ffi_prep_args(extended_cif *ecif, अचिन्हित अक्षर* चळत)
{
  अचिन्हित पूर्णांक i;
  अचिन्हित दीर्घ *addr;
  ffi_type **ptr;

  संघ {
    शून्य **v;
    अक्षर **c;
    चिन्हित अक्षर **sc;
    अचिन्हित अक्षर **uc;
    चिन्हित लघु **ss;
    अचिन्हित लघु **us;
    अचिन्हित पूर्णांक **i;
    दीर्घ दीर्घ **ll;
    दशांश **f;
    द्विगुण **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((अचिन्हित दीर्घ) चळत & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (अचिन्हित दीर्घ*)चळत;

  /* structures with a size greater than 16 bytes are passed in memory */
  जर (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 16)
  {
    *addr++ = (अचिन्हित दीर्घ)ecif->rvalue;
  }

  साठी (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    निवड ((*ptr)->type)
    {
      स्थिती FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        थांबा;
      स्थिती FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        थांबा;
      स्थिती FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        थांबा;
      स्थिती FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        थांबा;
      स्थिती FFI_TYPE_FLOAT:
      स्थिती FFI_TYPE_INT:
      स्थिती FFI_TYPE_UINT32:
      स्थिती FFI_TYPE_SINT32:
      स्थिती FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        थांबा;
      स्थिती FFI_TYPE_DOUBLE:
      स्थिती FFI_TYPE_UINT64:
      स्थिती FFI_TYPE_SINT64:
        जर (((अचिन्हित दीर्घ)addr & 4) != 0)
          addr++;
        *(अचिन्हित दीर्घ दीर्घ*)addr = **p_argv.ll;
	addr += आकार(अचिन्हित दीर्घ दीर्घ) / आकार (addr);
        थांबा;

      स्थिती FFI_TYPE_STRUCT:
      {
        अचिन्हित दीर्घ offs;
        अचिन्हित दीर्घ size;

        जर (((अचिन्हित दीर्घ)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (अचिन्हित दीर्घ) addr - (अचिन्हित दीर्घ) चळत;
        size = (*ptr)->size;

        /* Entire structure must fit the argument registers or referenced */
        जर (offs < FFI_REGISTER_NARGS * 4
            && offs + size > FFI_REGISTER_NARGS * 4)
          addr = (अचिन्हित दीर्घ*) (चळत + FFI_REGISTER_NARGS * 4);

        स_नकल((अक्षर*) addr, *p_argv.c, size);
        addr += (size + 3) / 4;
        थांबा;
      }

      पूर्वनिर्धारित:
        FFI_ASSERT(0);
    }
  }
}


शून्य ffi_call(ffi_cif* cif, शून्य(*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;
  अचिन्हित दीर्घ rsize = cif->rtype->size;
  पूर्णांक flags = cif->flags;
  शून्य *alloc = रिक्त;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  जर (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == रिक्त))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  अन्यथा
  {
    ecif.rvalue = rvalue;
  }

  जर (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  जर (alloc != रिक्त && rvalue != रिक्त)
    स_नकल(rvalue, alloc, rsize);
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
  जर (cif->abi != FFI_SYSV)
    परत FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  स_नकल(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_SIZE);
  *(अचिन्हित पूर्णांक*)(&closure->tramp[8]) = (अचिन्हित पूर्णांक)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_SIZE);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  परत FFI_OK; 
}


दीर्घ FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, शून्य **मूल्ये, शून्य *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  शून्य **avalue;
  पूर्णांक i, areg;

  cif = closure->cif;
  जर (cif->abi != FFI_SYSV)
    परत FFI_BAD_ABI;

  areg = 0;

  पूर्णांक rtype = cif->rtype->type;
  जर (rtype == FFI_TYPE_STRUCT && cif->rtype->size > 4 * 4)
  {
    rvalue = *मूल्ये;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * आकार(शून्य *));

  साठी (i = 0; i < cif->nargs; i++)
  {
    जर (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    जर (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    जर (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      पूर्णांक numregs = ((arg_types[i]->size + 3) & ~3) / 4;
      जर (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &मूल्ये[areg];
    areg += (arg_types[i]->size + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_data);

  परत rtype;
}
