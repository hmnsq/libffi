/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 1998 Cygnus Solutions
           Copyright (c) 2004 Simon Posnjak
	   Copyright (c) 2005 Axis Communications AB
	   Copyright (C) 2007 Free Software Foundation, Inc.

   CRIS Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL SIMON POSNJAK BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#నిర్వచించు దొంతర_ARG_కొలత(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

స్థిర ffi_స్థితి
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != ఖాళీ);

  FFI_ASSERT (arg->elements != ఖాళీ);
  FFI_ASSERT (arg->కొలత == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  అయితే ((*ptr) != ఖాళీ)
    {
      ఒకవేళ (((*ptr)->కొలత == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	తిరిగి FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->కొలత += (*ptr)->కొలత;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  ఒకవేళ (arg->కొలత == 0)
    తిరిగి FFI_BAD_TYPEDEF;
  లేకపోతే
    తిరిగి FFI_OK;
}

పూర్ణాంకం
ffi_prep_వాదనలు (అక్షరం *దొంతర, extended_cif * ecif)
{
  గుర్తులేని పూర్ణాంకం i;
  గుర్తులేని పూర్ణాంకం struct_లెక్క = 0;
  శూన్య **p_argv;
  అక్షరం *argp;
  ffi_type **p_arg;

  argp = దొంతర;

  p_argv = ecif->avalue;

  కొరకు (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      ఎంపిక ((*p_arg)->type)
	{
	స్థితి FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->కొలత;
	    ఒకవేళ (z <= 4)
	      {
		జ్ఞాపకం_నకలు (argp, *p_argv, z);
		z = 4;
	      }
	    లేకపోతే ఒకవేళ (z <= 8)
	      {
		జ్ఞాపకం_నకలు (argp, *p_argv, z);
		z = 8;
	      }
	    లేకపోతే
	      {
		గుర్తులేని పూర్ణాంకం ui_Loc_On_దొంతర;
		z = పరిమాణం (శూన్య *);
		ui_Loc_On_దొంతర = 4 * ecif->cif->nargs + struct_లెక్క;
		struct_లెక్క = struct_లెక్క + (*p_arg)->కొలత;
		*(గుర్తులేని పూర్ణాంకం *) argp =
		  (గుర్తులేని పూర్ణాంకం) (UINT32 *) (దొంతర + ui_Loc_On_దొంతర);
		జ్ఞాపకం_నకలు ((దొంతర + ui_Loc_On_దొంతర), *p_argv, (*p_arg)->కొలత);
	      }
	    ఆపు;
	  }
	అప్రమేయ:
	  z = (*p_arg)->కొలత;
	  ఒకవేళ (z < పరిమాణం (పూర్ణాంకం))
	    {
	      ఎంపిక ((*p_arg)->type)
		{
		స్థితి FFI_TYPE_SINT8:
		  *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం) *(SINT8 *) (*p_argv);
		  ఆపు;

		స్థితి FFI_TYPE_UINT8:
		  *(గుర్తులేని పూర్ణాంకం *) argp =
		    (గుర్తులేని పూర్ణాంకం) *(UINT8 *) (*p_argv);
		  ఆపు;

		స్థితి FFI_TYPE_SINT16:
		  *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం) *(SINT16 *) (*p_argv);
		  ఆపు;

		స్థితి FFI_TYPE_UINT16:
		  *(గుర్తులేని పూర్ణాంకం *) argp =
		    (గుర్తులేని పూర్ణాంకం) *(UINT16 *) (*p_argv);
		  ఆపు;

		అప్రమేయ:
		  FFI_ASSERT (0);
		}
	      z = పరిమాణం (పూర్ణాంకం);
	    }
	  లేకపోతే ఒకవేళ (z == పరిమాణం (పూర్ణాంకం))
	    *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం) *(UINT32 *) (*p_argv);
	  లేకపోతే
	    జ్ఞాపకం_నకలు (argp, *p_argv, z);
	  ఆపు;
	}
      p_argv++;
      argp += z;
    }

  తిరిగి (struct_లెక్క);
}

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, గుర్తులేని పూర్ణాంకం isvariadic,
		   గుర్తులేని పూర్ణాంకం nfixedargs, గుర్తులేని పూర్ణాంకం ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  గుర్తులేని బైట్లు = 0;
  గుర్తులేని పూర్ణాంకం i;
  ffi_type **ptr;

  FFI_ASSERT (cif != ఖాళీ);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_మొదటి_ABI && abi < FFI_చివరి_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  ఒకవేళ ((cif->rtype->కొలత == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    తిరిగి FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  కొరకు (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ఒకవేళ (((*ptr)->కొలత == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	తిరిగి FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      ఒకవేళ (((*ptr)->alignment - 1) & బైట్లు)
	బైట్లు = FFI_ALIGN (బైట్లు, (*ptr)->alignment);
      ఒకవేళ ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  ఒకవేళ ((*ptr)->కొలత > 8)
	    {
	      బైట్లు += (*ptr)->కొలత;
	      బైట్లు += పరిమాణం (శూన్య *);
	    }
	  లేకపోతే
	    {
	      ఒకవేళ ((*ptr)->కొలత > 4)
		బైట్లు += 8;
	      లేకపోతే
		బైట్లు += 4;
	    }
	}
      లేకపోతే
	బైట్లు += దొంతర_ARG_కొలత ((*ptr)->కొలత);
    }

  cif->బైట్లు = బైట్లు;

  తిరిగి ffi_prep_cif_machdep (cif);
}

ffi_స్థితి
ffi_prep_cif_machdep (ffi_cif * cif)
{
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
    స్థితి FFI_TYPE_STRUCT:
    స్థితి FFI_TYPE_FLOAT:
    స్థితి FFI_TYPE_DOUBLE:
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      cif->flags = (గుర్తులేని) cif->rtype->type;
      ఆపు;

    అప్రమేయ:
      cif->flags = FFI_TYPE_INT;
      ఆపు;
    }

  తిరిగి FFI_OK;
}

బాహ్య శూన్య ffi_call_SYSV (పూర్ణాంకం (*)(అక్షరం *, extended_cif *),
			   extended_cif *,
			   గుర్తులేని, గుర్తులేని, గుర్తులేని *, శూన్య (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

శూన్య
ffi_call (ffi_cif * cif, శూన్య (*fn) (), శూన్య *rvalue, శూన్య **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ఒకవేళ ((rvalue == ఖాళీ) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->కొలత);
    }
  లేకపోతే
    ecif.rvalue = rvalue;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_SYSV:
      ffi_call_SYSV (ffi_prep_వాదనలు, &ecif, cif->బైట్లు,
		     cif->flags, ecif.rvalue, fn);
      ఆపు;
    అప్రమేయ:
      FFI_ASSERT (0);
      ఆపు;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
బాహ్య స్థిరరాశి అక్షరం ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
బాహ్య స్థిరరాశి పూర్ణాంకం ffi_cris_trampoline_fn_స్థానభ్రంశం
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
బాహ్య స్థిరరాశి పూర్ణాంకం ffi_cris_trampoline_సంవృతం_స్థానభ్రంశం
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

స్థిర గుర్తులేని దీర్ఘ దీర్ఘ
ffi_prep_సంవృతం_inner (శూన్య **params, ffi_సంవృతం* సంవృతం)
{
  అక్షరం *రిజిస్టరు_వాదనలు = (అక్షరం *) params;
  శూన్య *struct_ret = params[5];
  అక్షరం *దొంతర_వాదనలు = params[6];
  అక్షరం *ptr = రిజిస్టరు_వాదనలు;
  ffi_cif *cif = సంవృతం->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  శూన్య **avalue = alloca (సంవృతం->cif->nargs * పరిమాణం(శూన్య *));
  పూర్ణాంకం i;
  పూర్ణాంకం doing_regs;
  దీర్ఘ దీర్ఘ llret = 0;

  /* Find the address of each argument.  */
  కొరకు (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      ఒకవేళ (arg_types[i]->కొలత <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      లేకపోతే ఒకవేళ (arg_types[i]->కొలత <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      లేకపోతే
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(శూన్య **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      ఒకవేళ (doing_regs && ptr >= రిజిస్టరు_వాదనలు + 4*4)
	{
	  ptr = దొంతర_వాదనలు + ((ptr > రిజిస్టరు_వాదనలు + 4*4) ? 4 : 0);
	  doing_regs = 0;
	}
    }

  /* Invoke the closure.  */
  (సంవృతం->fun) (cif,

		  cif->rtype->type == FFI_TYPE_STRUCT
		  /* The caller allocated space for the return
		     structure, and passed a pointer to this space in
		     R9.  */
		  ? struct_ret

		  /* We take advantage of being able to ignore that
		     the high part isn't set if the return value is
		     not in R10:R11, but in R10 only.  */
		  : (శూన్య *) &llret,

		  avalue, సంవృతం->user_దత్తాంశం);

  తిరిగి llret;
}

/* API function: Prepare the trampoline.  */

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif *, శూన్య *, శూన్య **, శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  శూన్య *innerfn = ffi_prep_సంవృతం_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  సంవృతం->cif  = cif;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  సంవృతం->fun  = fun;
  జ్ఞాపకం_నకలు (సంవృతం->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_సంకేతం_PART_కొలత);
  జ్ఞాపకం_నకలు (సంవృతం->tramp + ffi_cris_trampoline_fn_స్థానభ్రంశం,
	  &innerfn, పరిమాణం (శూన్య *));
  జ్ఞాపకం_నకలు (సంవృతం->tramp + ffi_cris_trampoline_సంవృతం_స్థానభ్రంశం,
	  &codeloc, పరిమాణం (శూన్య *));

  తిరిగి FFI_OK;
}
