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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

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


#घोषणा FFI_TYPE_STRUCT_REGS FFI_TYPE_LAST


बाह्य व्योम ffi_call_SYSV(व्योम *rvalue, अचिन्हित rsize, अचिन्हित flags,
			  व्योम(*fn)(व्योम), अचिन्हित nbytes, extended_cif*);
बाह्य व्योम ffi_closure_SYSV(व्योम) FFI_HIDDEN;

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  चयन(cif->rtype->type) {
    हाल FFI_TYPE_SINT8:
    हाल FFI_TYPE_UINT8:
    हाल FFI_TYPE_SINT16:
    हाल FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      अवरोध;
    हाल FFI_TYPE_VOID:
    हाल FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      अवरोध;
    हाल FFI_TYPE_DOUBLE:
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      अवरोध;
    हाल FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      अगर (cif->rtype->आकार > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->bytes += 8;
      }
      अवरोध;

    शेष:
      cif->flags = FFI_TYPE_UINT32;
      अवरोध;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  अगर (cif->bytes < FFI_REGISTER_NARGS * 4)
    cif->bytes = FFI_REGISTER_ARGS_SPACE;
  अन्यथा
    cif->bytes = FFI_REGISTER_ARGS_SPACE +
	    FFI_ALIGN(cif->bytes - FFI_REGISTER_NARGS * 4,
		      XTENSA_ढेर_ALIGNMENT);
  वापस FFI_OK;
}

व्योम ffi_prep_args(extended_cif *ecif, अचिन्हित अक्षर* ढेर)
{
  अचिन्हित पूर्णांक i;
  अचिन्हित दीर्घ *addr;
  ffi_type **ptr;

  जोड़ {
    व्योम **v;
    अक्षर **c;
    चिन्हित अक्षर **sc;
    अचिन्हित अक्षर **uc;
    चिन्हित लघु **ss;
    अचिन्हित लघु **us;
    अचिन्हित पूर्णांक **i;
    दीर्घ दीर्घ **ll;
    भग्न **f;
    द्विगुन **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((अचिन्हित दीर्घ) ढेर & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (अचिन्हित दीर्घ*)ढेर;

  /* structures with a size greater than 16 bytes are passed in memory */
  अगर (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->आकार > 16)
  {
    *addr++ = (अचिन्हित दीर्घ)ecif->rvalue;
  }

  क्रम (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    चयन ((*ptr)->type)
    {
      हाल FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        अवरोध;
      हाल FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        अवरोध;
      हाल FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        अवरोध;
      हाल FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        अवरोध;
      हाल FFI_TYPE_FLOAT:
      हाल FFI_TYPE_INT:
      हाल FFI_TYPE_UINT32:
      हाल FFI_TYPE_SINT32:
      हाल FFI_TYPE_POINTER:
        *addr++ = **p_argv.i;
        अवरोध;
      हाल FFI_TYPE_DOUBLE:
      हाल FFI_TYPE_UINT64:
      हाल FFI_TYPE_SINT64:
        अगर (((अचिन्हित दीर्घ)addr & 4) != 0)
          addr++;
        *(अचिन्हित दीर्घ दीर्घ*)addr = **p_argv.ll;
	addr += माप(अचिन्हित दीर्घ दीर्घ) / माप (addr);
        अवरोध;

      हाल FFI_TYPE_STRUCT:
      {
        अचिन्हित दीर्घ offs;
        अचिन्हित दीर्घ आकार;

        अगर (((अचिन्हित दीर्घ)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (अचिन्हित दीर्घ) addr - (अचिन्हित दीर्घ) ढेर;
        आकार = (*ptr)->आकार;

        /* Entire structure must fit the argument registers or referenced */
        अगर (offs < FFI_REGISTER_NARGS * 4
            && offs + आकार > FFI_REGISTER_NARGS * 4)
          addr = (अचिन्हित दीर्घ*) (ढेर + FFI_REGISTER_NARGS * 4);

        स_नकल((अक्षर*) addr, *p_argv.c, आकार);
        addr += (आकार + 3) / 4;
        अवरोध;
      }

      शेष:
        FFI_ASSERT(0);
    }
  }
}


व्योम ffi_call(ffi_cif* cif, व्योम(*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  extended_cif ecif;
  अचिन्हित दीर्घ rsize = cif->rtype->आकार;
  पूर्णांक flags = cif->flags;
  व्योम *alloc = शून्य;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  अगर (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == शून्य))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  अन्यथा
  {
    ecif.rvalue = rvalue;
  }

  अगर (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->bytes, &ecif);

  अगर (alloc != शून्य && rvalue != शून्य)
    स_नकल(rvalue, alloc, rsize);
}

बाह्य व्योम ffi_trampoline();
बाह्य व्योम ffi_cacheflush(व्योम* start, व्योम* end);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
                      व्योम *user_तथ्य,
                      व्योम *codeloc)
{
  अगर (cif->abi != FFI_SYSV)
    वापस FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  स_नकल(closure->tramp, ffi_trampoline, FFI_TRAMPOLINE_आकार);
  *(अचिन्हित पूर्णांक*)(&closure->tramp[8]) = (अचिन्हित पूर्णांक)ffi_closure_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(closure->tramp, closure->tramp + FFI_TRAMPOLINE_आकार);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;
  वापस FFI_OK; 
}


दीर्घ FFI_HIDDEN
ffi_closure_SYSV_inner(ffi_closure *closure, व्योम **मूल्यावली, व्योम *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  व्योम **avalue;
  पूर्णांक i, areg;

  cif = closure->cif;
  अगर (cif->abi != FFI_SYSV)
    वापस FFI_BAD_ABI;

  areg = 0;

  पूर्णांक rtype = cif->rtype->type;
  अगर (rtype == FFI_TYPE_STRUCT && cif->rtype->आकार > 4 * 4)
  {
    rvalue = *मूल्यावली;
    areg++;
  }

  cif = closure->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * माप(व्योम *));

  क्रम (i = 0; i < cif->nargs; i++)
  {
    अगर (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    अगर (areg == FFI_REGISTER_NARGS)
      areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;

    अगर (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      पूर्णांक numregs = ((arg_types[i]->आकार + 3) & ~3) / 4;
      अगर (areg < FFI_REGISTER_NARGS && areg + numregs > FFI_REGISTER_NARGS)
        areg = (FFI_REGISTER_ARGS_SPACE + 32) / 4;
    }

    avalue[i] = &मूल्यावली[areg];
    areg += (arg_types[i]->आकार + 3) / 4;
  }

  (closure->fun)(cif, rvalue, avalue, closure->user_तथ्य);

  वापस rtype;
}
