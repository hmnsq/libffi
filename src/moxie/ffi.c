/* -----------------------------------------------------------------------
   ffi.c - Copyright (C) 2012, 2013, 2018, 2021, 2022, 2026  Anthony Green

   Moxie Foreign Function Interface

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

#అన్తర్భావ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

శూన్య *ffi_prep_వాదనలు(అక్షరం *దొంతర, extended_cif *ecif)
{
  నమోదు గుర్తులేని పూర్ణాంకం i;
  నమోదు శూన్య **p_argv;
  నమోదు అక్షరం *argp;
  నమోదు ffi_type **p_arg;
  నమోదు పూర్ణాంకం లెక్క = 0;

  p_argv = ecif->avalue;
  argp = దొంతర;

  ఒకవేళ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(శూన్య **) argp = ecif->rvalue;
      argp += 4;
    }

  కొరకు (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->కొలత;

      ఒకవేళ ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = పరిమాణం(శూన్య*);
	  *(శూన్య **) argp = *p_argv;
	}
      లేకపోతే ఒకవేళ (z < పరిమాణం(పూర్ణాంకం))
	{
	  z = పరిమాణం(పూర్ణాంకం);
	  ఎంపిక ((*p_arg)->type)
	    {
	    స్థితి FFI_TYPE_SINT8:
	      *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT8 *)(* p_argv);
	      ఆపు;

	    స్థితి FFI_TYPE_UINT8:
	      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT8 *)(* p_argv);
	      ఆపు;

	    స్థితి FFI_TYPE_SINT16:
	      *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT16 *)(* p_argv);
	      ఆపు;

	    స్థితి FFI_TYPE_UINT16:
	      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT16 *)(* p_argv);
	      ఆపు;

	    అప్రమేయ:
	      FFI_ASSERT(0);
	    }
	}
      లేకపోతే ఒకవేళ (z == పరిమాణం(పూర్ణాంకం))
	{
	  *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT32 *)(* p_argv);
	}
      లేకపోతే
	{
	  జ్ఞాపకం_నకలు(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      లెక్క += z;
    }

  తిరిగి (దొంతర + ((లెక్క > 24) ? 24 : FFI_ALIGN_కిందికి(లెక్క, 8)));
}

/* Perform machine dependent cif processing */
ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif)
{
  ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  లేకపోతే
    cif->flags = cif->rtype->కొలత;

  cif->బైట్లు = FFI_ALIGN (cif->బైట్లు, 8);

  తిరిగి FFI_OK;
}

బాహ్య శూన్య ffi_call_EABI(శూన్య *(*)(అక్షరం *, extended_cif *),
			  extended_cif *,
			  గుర్తులేని, గుర్తులేని,
			  గుర్తులేని *,
			  శూన్య (*fn)(శూన్య));

శూన్య ffi_call(ffi_cif *cif,
	      శూన్య (*fn)(శూన్య),
	      శూన్య *rvalue,
	      శూన్య **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  శూన్య **avalue_copy = ఖాళీ;
  పూర్ణాంకం i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ఒకవేళ ((rvalue == ఖాళీ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->కొలత);
    }
  లేకపోతే
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  కొరకు (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      పూర్ణాంకం కొలత = at->కొలత;
      ఒకవేళ (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          అక్షరం *argcopy = alloca (కొలత);
          ఒకవేళ (avalue_copy == ఖాళీ)
            {
              avalue_copy = alloca (nargs * పరిమాణం (శూన్య *));
              జ్ఞాపకం_నకలు (avalue_copy, avalue, nargs * పరిమాణం (శూన్య *));
              avalue = avalue_copy;
            }
          జ్ఞాపకం_నకలు (argcopy, avalue[i], కొలత);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_EABI:
      ffi_call_EABI(ffi_prep_వాదనలు, &ecif, cif->బైట్లు,
		    cif->flags, ecif.rvalue, fn);
      ఆపు;
    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }
}

శూన్య ffi_సంవృతం_eabi (గుర్తులేని arg1, గుర్తులేని arg2, గుర్తులేని arg3,
		       గుర్తులేని arg4, గుర్తులేని arg5, గుర్తులేని arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  నమోదు ffi_సంవృతం *creg __asm__ ("$r12");
  ffi_సంవృతం *సంవృతం = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  నమోదు అక్షరం *చట్రం_సూచిక __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  శూన్య *struct_rvalue = (శూన్య *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  అక్షరం *దొంతర_వాదనలు = చట్రం_సూచిక + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  గుర్తులేని రిజిస్టరు_వాదనలు[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  అక్షరం *రిజిస్టరు_వాదనలు_ptr = (అక్షరం *) రిజిస్టరు_వాదనలు;

  ffi_cif *cif = సంవృతం->cif;
  ffi_type **arg_types = cif->arg_types;
  శూన్య **avalue = alloca (cif->nargs * పరిమాణం(శూన్య *));
  అక్షరం *ptr = (అక్షరం *) రిజిస్టరు_వాదనలు;
  పూర్ణాంకం i;

  /* preserve struct type return pointer passing */
  ఒకవేళ ((cif->rtype != ఖాళీ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    రిజిస్టరు_వాదనలు_ptr = (అక్షరం *)&రిజిస్టరు_వాదనలు[1];
  }

  /* Find the address of each argument.  */
  కొరకు (i = 0; i < cif->nargs; i++)
    {
      ఎంపిక (arg_types[i]->type)
	{
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  ఆపు;
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  ఆపు;
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_సూచిక:
	  avalue[i] = ptr;
	  ఆపు;
	స్థితి FFI_TYPE_STRUCT:
          {
            ఒకవేళ (arg_types[i]->కొలత > 4)
              {
                శూన్య *copy = alloca(arg_types[i]->కొలత);
                జ్ఞాపకం_నకలు(copy, *(శూన్య**)ptr, arg_types[i]->కొలత);
                avalue[i] = copy;
              }
            లేకపోతే
              avalue[i] = *(శూన్య**)ptr;
          }
	  ఆపు;
	అప్రమేయ:
	  /* This is an 8-byte value.  */
	  ఒకవేళ (ptr == (అక్షరం *) &రిజిస్టరు_వాదనలు[5])
	    {
	      /* The value is split across two locations */
	      గుర్తులేని *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(గుర్తులేని *) ptr;
	      ip[1] = *(గుర్తులేని *) దొంతర_వాదనలు;
	    }
	  లేకపోతే
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  ఆపు;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ఒకవేళ (ptr == (అక్షరం *) &రిజిస్టరు_వాదనలు[6])
	ptr = దొంతర_వాదనలు;
      లేకపోతే ఒకవేళ (ptr == (అక్షరం *) &రిజిస్టరు_వాదనలు[7])
	ptr = దొంతర_వాదనలు + 4;
    }

  /* Invoke the closure.  */
  ఒకవేళ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (సంవృతం->fun) (cif, struct_rvalue, avalue, సంవృతం->user_దత్తాంశం);
    }
  లేకపోతే
    {
      /* Allocate space for the return value and call the function.  */
      దీర్ఘ దీర్ఘ rvalue;
      (సంవృతం->fun) (cif, &rvalue, avalue, సంవృతం->user_దత్తాంశం);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  గుర్తులేని పొట్టి *tramp = (గుర్తులేని పొట్టి *) &సంవృతం->tramp[0];
  గుర్తులేని దీర్ఘ fn = (దీర్ఘ) ffi_సంవృతం_eabi;
  గుర్తులేని దీర్ఘ cls = (దీర్ఘ) codeloc;

  ఒకవేళ (cif->abi != FFI_EABI)
    తిరిగి FFI_BAD_ABI;

  fn = (గుర్తులేని దీర్ఘ) ffi_సంవృతం_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}
