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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

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


#నిర్వచించు FFI_TYPE_STRUCT_REGS FFI_TYPE_చివరి


బాహ్య శూన్య ffi_call_SYSV(శూన్య *rvalue, గుర్తులేని rsize, గుర్తులేని flags,
			  శూన్య(*fn)(శూన్య), గుర్తులేని nbytes, extended_cif*);
బాహ్య శూన్య ffi_సంవృతం_SYSV(శూన్య) FFI_HIDDEN;

ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif)
{
  ఎంపిక(cif->rtype->type) {
    స్థితి FFI_TYPE_SINT8:
    స్థితి FFI_TYPE_UINT8:
    స్థితి FFI_TYPE_SINT16:
    స్థితి FFI_TYPE_UINT16:
      cif->flags = cif->rtype->type;
      ఆపు;
    స్థితి FFI_TYPE_VOID:
    స్థితి FFI_TYPE_FLOAT:
      cif->flags = FFI_TYPE_UINT32;
      ఆపు;
    స్థితి FFI_TYPE_DOUBLE:
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64; // cif->rtype->type;
      ఆపు;
    స్థితి FFI_TYPE_STRUCT:
      cif->flags = FFI_TYPE_STRUCT; //_REGS;
      /* Up to 16 bytes are returned in registers */
      ఒకవేళ (cif->rtype->కొలత > 4 * 4) {
        /* returned structure is referenced by a register; use 8 bytes
           (including 4 bytes for potential additional alignment) */
        cif->flags = FFI_TYPE_STRUCT;	
        cif->బైట్లు += 8;
      }
      ఆపు;

    అప్రమేయ:
      cif->flags = FFI_TYPE_UINT32;
      ఆపు;
  }

  /* Round up stack size needed for arguments.
     Allocate FFI_REGISTER_ARGS_SPACE bytes when there are only arguments
     passed in registers, round space reserved for arguments passed on stack
     up to ABI-specified alignment.  */
  ఒకవేళ (cif->బైట్లు < FFI_రిజిస్టరు_NARGS * 4)
    cif->బైట్లు = FFI_రిజిస్టరు_వాదనలు_SPACE;
  లేకపోతే
    cif->బైట్లు = FFI_రిజిస్టరు_వాదనలు_SPACE +
	    FFI_ALIGN(cif->బైట్లు - FFI_రిజిస్టరు_NARGS * 4,
		      XTENSA_దొంతర_ALIGNMENT);
  తిరిగి FFI_OK;
}

శూన్య ffi_prep_వాదనలు(extended_cif *ecif, గుర్తులేని అక్షరం* దొంతర)
{
  గుర్తులేని పూర్ణాంకం i;
  గుర్తులేని దీర్ఘ *addr;
  ffi_type **ptr;

  సంఘం {
    శూన్య **v;
    అక్షరం **c;
    గుర్తుగల అక్షరం **sc;
    గుర్తులేని అక్షరం **uc;
    గుర్తుగల పొట్టి **ss;
    గుర్తులేని పొట్టి **us;
    గుర్తులేని పూర్ణాంకం **i;
    దీర్ఘ దీర్ఘ **ll;
    దశాంశం **f;
    ద్విగుణ **d;
  } p_argv;

  /* Verify that everything is aligned up properly */
  FFI_ASSERT (((గుర్తులేని దీర్ఘ) దొంతర & 0x7) == 0);

  p_argv.v = ecif->avalue;
  addr = (గుర్తులేని దీర్ఘ*)దొంతర;

  /* structures with a size greater than 16 bytes are passed in memory */
  ఒకవేళ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->కొలత > 16)
  {
    *addr++ = (గుర్తులేని దీర్ఘ)ecif->rvalue;
  }

  కొరకు (i = ecif->cif->nargs, ptr = ecif->cif->arg_types;
       i > 0;
       i--, ptr++, p_argv.v++)
  {
    ఎంపిక ((*ptr)->type)
    {
      స్థితి FFI_TYPE_SINT8:
        *addr++ = **p_argv.sc;
        ఆపు;
      స్థితి FFI_TYPE_UINT8:
        *addr++ = **p_argv.uc;
        ఆపు;
      స్థితి FFI_TYPE_SINT16:
        *addr++ = **p_argv.ss;
        ఆపు;
      స్థితి FFI_TYPE_UINT16:
        *addr++ = **p_argv.us;
        ఆపు;
      స్థితి FFI_TYPE_FLOAT:
      స్థితి FFI_TYPE_INT:
      స్థితి FFI_TYPE_UINT32:
      స్థితి FFI_TYPE_SINT32:
      స్థితి FFI_TYPE_సూచిక:
        *addr++ = **p_argv.i;
        ఆపు;
      స్థితి FFI_TYPE_DOUBLE:
      స్థితి FFI_TYPE_UINT64:
      స్థితి FFI_TYPE_SINT64:
        ఒకవేళ (((గుర్తులేని దీర్ఘ)addr & 4) != 0)
          addr++;
        *(గుర్తులేని దీర్ఘ దీర్ఘ*)addr = **p_argv.ll;
	addr += పరిమాణం(గుర్తులేని దీర్ఘ దీర్ఘ) / పరిమాణం (addr);
        ఆపు;

      స్థితి FFI_TYPE_STRUCT:
      {
        గుర్తులేని దీర్ఘ offs;
        గుర్తులేని దీర్ఘ కొలత;

        ఒకవేళ (((గుర్తులేని దీర్ఘ)addr & 4) != 0 && (*ptr)->alignment > 4)
          addr++;

        offs = (గుర్తులేని దీర్ఘ) addr - (గుర్తులేని దీర్ఘ) దొంతర;
        కొలత = (*ptr)->కొలత;

        /* Entire structure must fit the argument registers or referenced */
        ఒకవేళ (offs < FFI_రిజిస్టరు_NARGS * 4
            && offs + కొలత > FFI_రిజిస్టరు_NARGS * 4)
          addr = (గుర్తులేని దీర్ఘ*) (దొంతర + FFI_రిజిస్టరు_NARGS * 4);

        జ్ఞాపకం_నకలు((అక్షరం*) addr, *p_argv.c, కొలత);
        addr += (కొలత + 3) / 4;
        ఆపు;
      }

      అప్రమేయ:
        FFI_ASSERT(0);
    }
  }
}


శూన్య ffi_call(ffi_cif* cif, శూన్య(*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  extended_cif ecif;
  గుర్తులేని దీర్ఘ rsize = cif->rtype->కొలత;
  పూర్ణాంకం flags = cif->flags;
  శూన్య *alloc = ఖాళీ;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* Note that for structures that are returned in registers (size <= 16 bytes)
     we allocate a temporary buffer and use memcpy to copy it to the final 
     destination. The reason is that the target address might be misaligned or
     the length not a multiple of 4 bytes. Handling all those cases would be
     very complex.  */

  ఒకవేళ (flags == FFI_TYPE_STRUCT && (rsize <= 16 || rvalue == ఖాళీ))
  {
    alloc = alloca(FFI_ALIGN(rsize, 4));
    ecif.rvalue = alloc;
  }
  లేకపోతే
  {
    ecif.rvalue = rvalue;
  }

  ఒకవేళ (cif->abi != FFI_SYSV)
    FFI_ASSERT(0);

  ffi_call_SYSV (ecif.rvalue, rsize, cif->flags, fn, cif->బైట్లు, &ecif);

  ఒకవేళ (alloc != ఖాళీ && rvalue != ఖాళీ)
    జ్ఞాపకం_నకలు(rvalue, alloc, rsize);
}

బాహ్య శూన్య ffi_trampoline();
బాహ్య శూన్య ffi_cacheflush(శూన్య* ప్రారంభం, శూన్య* చివర);

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
                      ffi_cif* cif,
                      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
                      శూన్య *user_దత్తాంశం,
                      శూన్య *codeloc)
{
  ఒకవేళ (cif->abi != FFI_SYSV)
    తిరిగి FFI_BAD_ABI;

  /* copye trampoline to stack and patch 'ffi_closure_SYSV' pointer */
  జ్ఞాపకం_నకలు(సంవృతం->tramp, ffi_trampoline, FFI_TRAMPOLINE_కొలత);
  *(గుర్తులేని పూర్ణాంకం*)(&సంవృతం->tramp[8]) = (గుర్తులేని పూర్ణాంకం)ffi_సంవృతం_SYSV;

  // Do we have this function?
  // __builtin___clear_cache(closer->tramp, closer->tramp + FFI_TRAMPOLINE_SIZE)
  ffi_cacheflush(సంవృతం->tramp, సంవృతం->tramp + FFI_TRAMPOLINE_కొలత);

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  తిరిగి FFI_OK; 
}


దీర్ఘ FFI_HIDDEN
ffi_సంవృతం_SYSV_inner(ffi_సంవృతం *సంవృతం, శూన్య **విలువలు, శూన్య *rvalue)
{
  ffi_cif *cif;
  ffi_type **arg_types;
  శూన్య **avalue;
  పూర్ణాంకం i, areg;

  cif = సంవృతం->cif;
  ఒకవేళ (cif->abi != FFI_SYSV)
    తిరిగి FFI_BAD_ABI;

  areg = 0;

  పూర్ణాంకం rtype = cif->rtype->type;
  ఒకవేళ (rtype == FFI_TYPE_STRUCT && cif->rtype->కొలత > 4 * 4)
  {
    rvalue = *విలువలు;
    areg++;
  }

  cif = సంవృతం->cif; 
  arg_types = cif->arg_types;
  avalue = alloca(cif->nargs * పరిమాణం(శూన్య *));

  కొరకు (i = 0; i < cif->nargs; i++)
  {
    ఒకవేళ (arg_types[i]->alignment == 8 && (areg & 1) != 0)
      areg++;

    // skip the entry a1, * framework, see ffi_trampoline
    ఒకవేళ (areg == FFI_రిజిస్టరు_NARGS)
      areg = (FFI_రిజిస్టరు_వాదనలు_SPACE + 32) / 4;

    ఒకవేళ (arg_types[i]->type == FFI_TYPE_STRUCT)
    {
      పూర్ణాంకం numregs = ((arg_types[i]->కొలత + 3) & ~3) / 4;
      ఒకవేళ (areg < FFI_రిజిస్టరు_NARGS && areg + numregs > FFI_రిజిస్టరు_NARGS)
        areg = (FFI_రిజిస్టరు_వాదనలు_SPACE + 32) / 4;
    }

    avalue[i] = &విలువలు[areg];
    areg += (arg_types[i]->కొలత + 3) / 4;
  }

  (సంవృతం->fun)(cif, rvalue, avalue, సంవృతం->user_దత్తాంశం);

  తిరిగి rtype;
}
