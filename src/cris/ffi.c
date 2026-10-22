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

#شامل <ffi.h>
#شامل <ffi_common.h>

#تعریف_کرو ڈھیر_ARG_سائز(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

ساکن ffi_حالت
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != تہی);

  FFI_ASSERT (arg->elements != تہی);
  FFI_ASSERT (arg->سائز == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  جب ((*ptr) != تہی)
    {
      اگر (((*ptr)->سائز == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	واپس FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->سائز += (*ptr)->سائز;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  اگر (arg->سائز == 0)
    واپس FFI_BAD_TYPEDEF;
  ورنہ
    واپس FFI_OK;
}

صحیح
ffi_prep_دلائل (حرف *ڈھیر, extended_cif * ecif)
{
  غیرعلامتی صحیح i;
  غیرعلامتی صحیح struct_گنتی = 0;
  خالی **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  argp = ڈھیر;

  p_argv = ecif->avalue;

  برائے (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      انتخاب ((*p_arg)->type)
	{
	حالت FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->سائز;
	    اگر (z <= 4)
	      {
		حافظہ_نقل (argp, *p_argv, z);
		z = 4;
	      }
	    ورنہ اگر (z <= 8)
	      {
		حافظہ_نقل (argp, *p_argv, z);
		z = 8;
	      }
	    ورنہ
	      {
		غیرعلامتی صحیح ui_Loc_On_ڈھیر;
		z = حجم (خالی *);
		ui_Loc_On_ڈھیر = 4 * ecif->cif->nargs + struct_گنتی;
		struct_گنتی = struct_گنتی + (*p_arg)->سائز;
		*(غیرعلامتی صحیح *) argp =
		  (غیرعلامتی صحیح) (UINT32 *) (ڈھیر + ui_Loc_On_ڈھیر);
		حافظہ_نقل ((ڈھیر + ui_Loc_On_ڈھیر), *p_argv, (*p_arg)->سائز);
	      }
	    توڑو;
	  }
	طےشدہ:
	  z = (*p_arg)->سائز;
	  اگر (z < حجم (صحیح))
	    {
	      انتخاب ((*p_arg)->type)
		{
		حالت FFI_TYPE_SINT8:
		  *(علامتی صحیح *) argp = (علامتی صحیح) *(SINT8 *) (*p_argv);
		  توڑو;

		حالت FFI_TYPE_UINT8:
		  *(غیرعلامتی صحیح *) argp =
		    (غیرعلامتی صحیح) *(UINT8 *) (*p_argv);
		  توڑو;

		حالت FFI_TYPE_SINT16:
		  *(علامتی صحیح *) argp = (علامتی صحیح) *(SINT16 *) (*p_argv);
		  توڑو;

		حالت FFI_TYPE_UINT16:
		  *(غیرعلامتی صحیح *) argp =
		    (غیرعلامتی صحیح) *(UINT16 *) (*p_argv);
		  توڑو;

		طےشدہ:
		  FFI_ASSERT (0);
		}
	      z = حجم (صحیح);
	    }
	  ورنہ اگر (z == حجم (صحیح))
	    *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح) *(UINT32 *) (*p_argv);
	  ورنہ
	    حافظہ_نقل (argp, *p_argv, z);
	  توڑو;
	}
      p_argv++;
      argp += z;
    }

  واپس (struct_گنتی);
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, غیرعلامتی صحیح isvariadic,
		   غیرعلامتی صحیح nfixedargs, غیرعلامتی صحیح ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  غیرعلامتی بائٹیں = 0;
  غیرعلامتی صحیح i;
  ffi_type **ptr;

  FFI_ASSERT (cif != تہی);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_پہلا_ABI && abi < FFI_آخری_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  اگر ((cif->rtype->سائز == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    واپس FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  برائے (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      اگر (((*ptr)->سائز == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	واپس FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      اگر (((*ptr)->alignment - 1) & بائٹیں)
	بائٹیں = FFI_ALIGN (بائٹیں, (*ptr)->alignment);
      اگر ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  اگر ((*ptr)->سائز > 8)
	    {
	      بائٹیں += (*ptr)->سائز;
	      بائٹیں += حجم (خالی *);
	    }
	  ورنہ
	    {
	      اگر ((*ptr)->سائز > 4)
		بائٹیں += 8;
	      ورنہ
		بائٹیں += 4;
	    }
	}
      ورنہ
	بائٹیں += ڈھیر_ARG_سائز ((*ptr)->سائز);
    }

  cif->بائٹیں = بائٹیں;

  واپس ffi_prep_cif_machdep (cif);
}

ffi_حالت
ffi_prep_cif_machdep (ffi_cif * cif)
{
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
    حالت FFI_TYPE_STRUCT:
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags = (غیرعلامتی) cif->rtype->type;
      توڑو;

    طےشدہ:
      cif->flags = FFI_TYPE_INT;
      توڑو;
    }

  واپس FFI_OK;
}

خارجی خالی ffi_call_SYSV (صحیح (*)(حرف *, extended_cif *),
			   extended_cif *,
			   غیرعلامتی, غیرعلامتی, غیرعلامتی *, خالی (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

خالی
ffi_call (ffi_cif * cif, خالی (*fn) (), خالی *rvalue, خالی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  اگر ((rvalue == تہی) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->سائز);
    }
  ورنہ
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
      ffi_call_SYSV (ffi_prep_دلائل, &ecif, cif->بائٹیں,
		     cif->flags, ecif.rvalue, fn);
      توڑو;
    طےشدہ:
      FFI_ASSERT (0);
      توڑو;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
خارجی ثابت حرف ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
خارجی ثابت صحیح ffi_cris_trampoline_fn_انحراف
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
خارجی ثابت صحیح ffi_cris_trampoline_احاطہ_انحراف
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

ساکن غیرعلامتی طویل طویل
ffi_prep_احاطہ_inner (خالی **params, ffi_احاطہ* احاطہ)
{
  حرف *رجسٹر_دلائل = (حرف *) params;
  خالی *struct_ret = params[5];
  حرف *ڈھیر_دلائل = params[6];
  حرف *ptr = رجسٹر_دلائل;
  ffi_cif *cif = احاطہ->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  خالی **avalue = alloca (احاطہ->cif->nargs * حجم(خالی *));
  صحیح i;
  صحیح doing_regs;
  طویل طویل llret = 0;

  /* Find the address of each argument.  */
  برائے (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      اگر (arg_types[i]->سائز <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      ورنہ اگر (arg_types[i]->سائز <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      ورنہ
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(خالی **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      اگر (doing_regs && ptr >= رجسٹر_دلائل + 4*4)
	{
	  ptr = ڈھیر_دلائل + ((ptr > رجسٹر_دلائل + 4*4) ? 4 : 0);
	  doing_regs = 0;
	}
    }

  /* Invoke the closure.  */
  (احاطہ->fun) (cif,

		  cif->rtype->type == FFI_TYPE_STRUCT
		  /* The caller allocated space for the return
		     structure, and passed a pointer to this space in
		     R9.  */
		  ? struct_ret

		  /* We take advantage of being able to ignore that
		     the high part isn't set if the return value is
		     not in R10:R11, but in R10 only.  */
		  : (خالی *) &llret,

		  avalue, احاطہ->user_ڈیٹا);

  واپس llret;
}

/* API function: Prepare the trampoline.  */

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif *, خالی *, خالی **, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  خالی *innerfn = ffi_prep_احاطہ_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  احاطہ->cif  = cif;
  احاطہ->user_ڈیٹا = user_ڈیٹا;
  احاطہ->fun  = fun;
  حافظہ_نقل (احاطہ->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_رمز_PART_سائز);
  حافظہ_نقل (احاطہ->tramp + ffi_cris_trampoline_fn_انحراف,
	  &innerfn, حجم (خالی *));
  حافظہ_نقل (احاطہ->tramp + ffi_cris_trampoline_احاطہ_انحراف,
	  &codeloc, حجم (خالی *));

  واپس FFI_OK;
}
