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


#परिभाषय FFI_TYPE_STRUCT_REGS FFI_TYPE_चरमः


बाह्य शून्य ffi_call_SYSV(शून्य *rvalue, अचिह्नित rsize, अचिह्नित flags,
			  शून्य(*fn)(शून्य), अचिह्नित nbytes, extended_cif*);
बाह्य शून्य ffi_संवृतिः_SYSV(शून्य) FFI_HIDDEN;

ffi_स्थितिः ffi_prep_cif_machdep(ffi_cif *cif)
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
      यदि (cif->rtype->आकार > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->अष्टकानि += 8;
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
  यदि (cif->अष्टकानि < FFI_पञ्जिकाकोशः_NARGS * 4)
    cif->अष्टकानि = FFI_पञ्जिकाकोशः_तर्काः_SPACE;
  अन्यथा
    cif->अष्टकानि = FFI_पञ्जिकाकोशः_तर्काः_SPACE +
	    FFI_ALIGN(cif->अष्टकानि - FFI_पञ्जिकाकोशः_NARGS * 4,
		      XTENSA_राशिः_ALIGNMENT);
  प्रत्यागम FFI_OK;
}

शून्य ffi_prep_तर्काः(extended_cif *ecif, अचिह्नित वर्ण* राशिः)
{
  अचिह्नित पूर्णाङ्क i;
  अचिह्नित दीर्घ *addr;
  ffi_type **ptr;

  सङ्घ {
    शून्य **v;
    वर्ण **c;
    चिह्नित वर्ण **sc;
    अचिह्नित वर्ण **uc;
    चिह्नित ह्रस्व **ss;
    अचिह्नित ह्रस्व **us;
    अचिह्नित पूर्णाङ्क **i;
    दीर्घ दीर्घ **ll;
    भिन्न **f;
    द्विगुण **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((अचिह्नित दीर्घ) राशिः & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (अचिह्नित दीर्घ*)राशिः;

  /* structures with a size greater than 16 bytes are passed in memory */
  यदि (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->आकार > 16)
  {
    *addr++ = (अचिह्नित दीर्घ)ecif->rvalue;
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
        यदि (((अचिह्नित दीर्घ)addr & 4) != 0)
          addr++;
        *(अचिह्नित दीर्घ दीर्घ*)addr = **p_argv.ll;
	addr += परिमाण(अचिह्नित दीर्घ दीर्घ) / परिमाण (addr);
        भङ्ग;

      पक्ष FFI_TYPE_STRUCT:
      {
        अचिह्नित दीर्घ offs;
        अचिह्नित दीर्घ आकार;

        यदि (((अचिह्नित दीर्घ)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (अचिह्नित दीर्घ) addr - (अचिह्नित दीर्घ) राशिः;
        आकार = (*ptr)->आकार;

        /* Entire structure must fit the argument registers or referenced */
        यदि (offs < FFI_पञ्जिकाकोशः_NARGS * 4
            && offs + आकार > FFI_पञ्जिकाकोशः_NARGS * 4)
          addr = (अचिह्नित दीर्घ*) (राशिः + FFI_पञ्जिकाकोशः_NARGS * 4);

        स्मृतिं_प्रतिलिख((वर्ण*) addr, *p_argv.c, आकार);
        addr += (आकार + 3) / 4;
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
  अचिह्नित दीर्घ rsize = cif->rtype->आकार;
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

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->अष्टकानि, &ecif);

  यदि (alloc != रिक्त && rvalue != रिक्त)
    स्मृतिं_प्रतिलिख(rvalue, alloc, rsize);
}

बाह्य शून्य ffi_trampoline();
बाह्य शून्य ffi_cacheflush(शून्य* start, शून्य* अन्तः);

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः* संवृतिः,
                      ffi_cif* cif,
                      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
                      शून्य *user_दत्त,
                      शून्य *codeloc)
{
  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  स्मृतिं_प्रतिलिख(संवृतिः->tramp, ffi_trampoline, FFI_TRAMPOLINE_आकार);
  *(अचिह्नित पूर्णाङ्क*)(&संवृतिः->tramp[8]) = (अचिह्नित पूर्णाङ्क)ffi_संवृतिः_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(संवृतिः->tramp, संवृतिः->tramp + FFI_TRAMPOLINE_आकार);

  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;
  प्रत्यागम FFI_OK; 
}


दीर्घ FFI_HIDDEN
ffi_संवृतिः_SYSV_inner(ffi_संवृतिः *संवृतिः, शून्य **मानानि, शून्य *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  शून्य **avalue;
  पूर्णाङ्क i, areg;

  cif = संवृतिः->cif;
  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  areg = 0;

  पूर्णाङ्क rtype = cif->rtype->type;
  यदि (rtype == FFI_TYPE_STRUCT && cif->rtype->आकार > 4 * 4)
  {
    rvalue = *मानानि;
    areg++;
  }

  cif = संवृतिः->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * परिमाण(शून्य *));

  प्रति (i = 0; i < cif->nargs; i++)
  {
    यदि (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    यदि (areg == FFI_पञ्जिकाकोशः_NARGS)
      areg = (FFI_पञ्जिकाकोशः_तर्काः_SPACE + 32) / 4;

    यदि (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      पूर्णाङ्क numregs = ((arg_types[i]->आकार + 3) & ~3) / 4;
      यदि (areg < FFI_पञ्जिकाकोशः_NARGS && areg + numregs > FFI_पञ्जिकाकोशः_NARGS)
        areg = (FFI_पञ्जिकाकोशः_तर्काः_SPACE + 32) / 4;
    }

    avalue[i] = &मानानि[areg];
    areg += (arg_types[i]->आकार + 3) / 4;
  }

  (संवृतिः->fun)(cif, rvalue, avalue, संवृतिः->user_दत्त);

  प्रत्यागम rtype;
}
