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


#تعریف_کرو FFI_TYPE_STRUCT_REGS FFI_TYPE_آخری


خارجی خالی ffi_call_SYSV(خالی *rvalue, غیرعلامتی rsize, غیرعلامتی flags,
			  خالی(*fn)(خالی), غیرعلامتی nbytes, extended_cif*);
خارجی خالی ffi_احاطہ_SYSV(خالی) FFI_HIDDEN;

ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  انتخاب(cif->rtype->type) {
    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      توڑو;
    حالت FFI_TYPE_VOID:
    حالت FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      توڑو;
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      توڑو;
    حالت FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      اگر (cif->rtype->سائز > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->بائٹیں += 8;
      }
      توڑو;

    طےشدہ:
      cif->flags = FFI_TYPE_UINT32;
      توڑو;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  اگر (cif->بائٹیں < FFI_رجسٹر_NARGS * 4)
    cif->بائٹیں = FFI_رجسٹر_دلائل_SPACE;
  ورنہ
    cif->بائٹیں = FFI_رجسٹر_دلائل_SPACE +
	    FFI_ALIGN(cif->بائٹیں - FFI_رجسٹر_NARGS * 4,
		      XTENSA_ڈھیر_ALIGNMENT);
  واپس FFI_OK;
}

خالی ffi_prep_دلائل(extended_cif *ecif, غیرعلامتی حرف* ڈھیر)
{
  غیرعلامتی صحیح i;
  غیرعلامتی طویل *addr;
  ffi_type **ptr;

  اتحاد {
    خالی **v;
    حرف **c;
    علامتی حرف **sc;
    غیرعلامتی حرف **uc;
    علامتی مختصر **ss;
    غیرعلامتی مختصر **us;
    غیرعلامتی صحیح **i;
    طویل طویل **ll;
    اعشاری **f;
    دوہرا **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((غیرعلامتی طویل) ڈھیر & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (غیرعلامتی طویل*)ڈھیر;

  /* structures with a size greater than 16 bytes are passed in memory */
  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->سائز > 16)
  {
    *addr++ = (غیرعلامتی طویل)ecif->rvalue;
  }

  برائے (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    انتخاب ((*ptr)->type)
    {
      حالت FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        توڑو;
      حالت FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        توڑو;
      حالت FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        توڑو;
      حالت FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        توڑو;
      حالت FFI_TYPE_FLOAT:
      حالت FFI_TYPE_INT:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_اشارہ_گر:
        *addr++ = **p_argv.i;
        توڑو;
      حالت FFI_TYPE_DOUBLE:
      حالت FFI_TYPE_UINT64:
      حالت FFI_TYPE_SINT64:
        اگر (((غیرعلامتی طویل)addr & 4) != 0)
          addr++;
        *(غیرعلامتی طویل طویل*)addr = **p_argv.ll;
	addr += حجم(غیرعلامتی طویل طویل) / حجم (addr);
        توڑو;

      حالت FFI_TYPE_STRUCT:
      {
        غیرعلامتی طویل offs;
        غیرعلامتی طویل سائز;

        اگر (((غیرعلامتی طویل)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (غیرعلامتی طویل) addr - (غیرعلامتی طویل) ڈھیر;
        سائز = (*ptr)->سائز;

        /* Entire structure must fit the argument registers or referenced */
        اگر (offs < FFI_رجسٹر_NARGS * 4
            && offs + سائز > FFI_رجسٹر_NARGS * 4)
          addr = (غیرعلامتی طویل*) (ڈھیر + FFI_رجسٹر_NARGS * 4);

        حافظہ_نقل((حرف*) addr, *p_argv.c, سائز);
        addr += (سائز + 3) / 4;
        توڑو;
      }

      طےشدہ:
        FFI_ASSERT(0);
    }
  }
}


خالی ffi_call(ffi_cif* cif, خالی(*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  extended_cif ecif;
  غیرعلامتی طویل rsize = cif->rtype->سائز;
  صحیح flags = cif->flags;
  خالی *alloc = تہی;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  اگر (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == تہی))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  ورنہ
  {
    ecif.rvalue = rvalue;
  }

  اگر (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->بائٹیں, &ecif);

  اگر (alloc != تہی && rvalue != تہی)
    حافظہ_نقل(rvalue, alloc, rsize);
}

خارجی خالی ffi_trampoline();
خارجی خالی ffi_cacheflush(خالی* آغاز, خالی* انتہا);

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
                      ffi_cif* cif,
                      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
                      خالی *user_ڈیٹا,
                      خالی *codeloc)
{
  اگر (cif->abi != FFI_SYSV)
    واپس FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  حافظہ_نقل(احاطہ->tramp, ffi_trampoline, FFI_TRAMPOLINE_سائز);
  *(غیرعلامتی صحیح*)(&احاطہ->tramp[8]) = (غیرعلامتی صحیح)ffi_احاطہ_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(احاطہ->tramp, احاطہ->tramp + FFI_TRAMPOLINE_سائز);

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;
  واپس FFI_OK; 
}


طویل FFI_HIDDEN
ffi_احاطہ_SYSV_inner(ffi_احاطہ *احاطہ, خالی **اقدار, خالی *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  خالی **avalue;
  صحیح i, areg;

  cif = احاطہ->cif;
  اگر (cif->abi != FFI_SYSV)
    واپس FFI_BAD_ABI;

  areg = 0;

  صحیح rtype = cif->rtype->type;
  اگر (rtype == FFI_TYPE_STRUCT && cif->rtype->سائز > 4 * 4)
  {
    rvalue = *اقدار;
    areg++;
  }

  cif = احاطہ->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * حجم(خالی *));

  برائے (i = 0; i < cif->nargs; i++)
  {
    اگر (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    اگر (areg == FFI_رجسٹر_NARGS)
      areg = (FFI_رجسٹر_دلائل_SPACE + 32) / 4;

    اگر (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      صحیح numregs = ((arg_types[i]->سائز + 3) & ~3) / 4;
      اگر (areg < FFI_رجسٹر_NARGS && areg + numregs > FFI_رجسٹر_NARGS)
        areg = (FFI_رجسٹر_دلائل_SPACE + 32) / 4;
    }

    avalue[i] = &اقدار[areg];
    areg += (arg_types[i]->سائز + 3) / 4;
  }

  (احاطہ->fun)(cif, rvalue, avalue, احاطہ->user_ڈیٹا);

  واپس rtype;
}
