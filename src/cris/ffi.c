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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#تعریف_کړه ډېری_ARG_اندازه(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

ساكن ffi_حالت
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != عدم);

  FFI_ASSERT (arg->elements != عدم);
  FFI_ASSERT (arg->اندازه == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  طالما ((*ptr) != عدم)
    {
      إذا (((*ptr)->اندازه == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	أرجع FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->اندازه += (*ptr)->اندازه;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  إذا (arg->اندازه == 0)
    أرجع FFI_BAD_TYPEDEF;
  وإلا
    أرجع FFI_OK;
}

صحيح
ffi_prep_دلیلونه (حرف *ډېری, extended_cif * ecif)
{
  بینښه صحيح i;
  بینښه صحيح struct_شمېر = 0;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  argp = ډېری;

  p_argv = ecif->avalue;

  لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      بدل ((*p_arg)->type)
	{
	حالة FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->اندازه;
	    إذا (z <= 4)
	      {
		حافظه_نقل (argp, *p_argv, z);
		z = 4;
	      }
	    وإلا إذا (z <= 8)
	      {
		حافظه_نقل (argp, *p_argv, z);
		z = 8;
	      }
	    وإلا
	      {
		بینښه صحيح ui_Loc_On_ډېری;
		z = حجم (فراغ *);
		ui_Loc_On_ډېری = 4 * ecif->cif->nargs + struct_شمېر;
		struct_شمېر = struct_شمېر + (*p_arg)->اندازه;
		*(بینښه صحيح *) argp =
		  (بینښه صحيح) (UINT32 *) (ډېری + ui_Loc_On_ډېری);
		حافظه_نقل ((ډېری + ui_Loc_On_ډېری), *p_argv, (*p_arg)->اندازه);
	      }
	    اكسر;
	  }
	افتراضي:
	  z = (*p_arg)->اندازه;
	  إذا (z < حجم (صحيح))
	    {
	      بدل ((*p_arg)->type)
		{
		حالة FFI_TYPE_SINT8:
		  *(نښهدار صحيح *) argp = (نښهدار صحيح) *(SINT8 *) (*p_argv);
		  اكسر;

		حالة FFI_TYPE_UINT8:
		  *(بینښه صحيح *) argp =
		    (بینښه صحيح) *(UINT8 *) (*p_argv);
		  اكسر;

		حالة FFI_TYPE_SINT16:
		  *(نښهدار صحيح *) argp = (نښهدار صحيح) *(SINT16 *) (*p_argv);
		  اكسر;

		حالة FFI_TYPE_UINT16:
		  *(بینښه صحيح *) argp =
		    (بینښه صحيح) *(UINT16 *) (*p_argv);
		  اكسر;

		افتراضي:
		  FFI_ASSERT (0);
		}
	      z = حجم (صحيح);
	    }
	  وإلا إذا (z == حجم (صحيح))
	    *(بینښه صحيح *) argp = (بینښه صحيح) *(UINT32 *) (*p_argv);
	  وإلا
	    حافظه_نقل (argp, *p_argv, z);
	  اكسر;
	}
      p_argv++;
      argp += z;
    }

  أرجع (struct_شمېر);
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, بینښه صحيح isvariadic,
		   بینښه صحيح nfixedargs, بینښه صحيح ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  بینښه بایټونه = 0;
  بینښه صحيح i;
  ffi_type **ptr;

  FFI_ASSERT (cif != عدم);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_لومړی_ABI && abi < FFI_وروستی_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  إذا ((cif->rtype->اندازه == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    أرجع FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      إذا (((*ptr)->اندازه == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	أرجع FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      إذا (((*ptr)->alignment - 1) & بایټونه)
	بایټونه = FFI_ALIGN (بایټونه, (*ptr)->alignment);
      إذا ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  إذا ((*ptr)->اندازه > 8)
	    {
	      بایټونه += (*ptr)->اندازه;
	      بایټونه += حجم (فراغ *);
	    }
	  وإلا
	    {
	      إذا ((*ptr)->اندازه > 4)
		بایټونه += 8;
	      وإلا
		بایټونه += 4;
	    }
	}
      وإلا
	بایټونه += ډېری_ARG_اندازه ((*ptr)->اندازه);
    }

  cif->بایټونه = بایټونه;

  أرجع ffi_prep_cif_machdep (cif);
}

ffi_حالت
ffi_prep_cif_machdep (ffi_cif * cif)
{
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
    حالة FFI_TYPE_STRUCT:
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags = (بینښه) cif->rtype->type;
      اكسر;

    افتراضي:
      cif->flags = FFI_TYPE_INT;
      اكسر;
    }

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_SYSV (صحيح (*)(حرف *, extended_cif *),
			   extended_cif *,
			   بینښه, بینښه, بینښه *, فراغ (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

فراغ
ffi_call (ffi_cif * cif, فراغ (*fn) (), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  إذا ((rvalue == عدم) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->اندازه);
    }
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
      ffi_call_SYSV (ffi_prep_دلیلونه, &ecif, cif->بایټونه,
		     cif->flags, ecif.rvalue, fn);
      اكسر;
    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
خارجي ثابت حرف ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
خارجي ثابت صحيح ffi_cris_trampoline_fn_دننه_واټن
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
خارجي ثابت صحيح ffi_cris_trampoline_تړون_دننه_واټن
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

ساكن بینښه اوږد اوږد
ffi_prep_تړون_inner (فراغ **params, ffi_تړون* تړون)
{
  حرف *ثبتوونکی_دلیلونه = (حرف *) params;
  فراغ *struct_ret = params[5];
  حرف *ډېری_دلیلونه = params[6];
  حرف *ptr = ثبتوونکی_دلیلونه;
  ffi_cif *cif = تړون->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  فراغ **avalue = alloca (تړون->cif->nargs * حجم(فراغ *));
  صحيح i;
  صحيح doing_regs;
  اوږد اوږد llret = 0;

  /* Find the address of each argument.  */
  لكل (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      إذا (arg_types[i]->اندازه <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      وإلا إذا (arg_types[i]->اندازه <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      وإلا
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(فراغ **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      إذا (doing_regs && ptr >= ثبتوونکی_دلیلونه + 4*4)
	{
	  ptr = ډېری_دلیلونه + ((ptr > ثبتوونکی_دلیلونه + 4*4) ? 4 : 0);
	  doing_regs = 0;
	}
    }

  /* Invoke the closure.  */
  (تړون->fun) (cif,

		  cif->rtype->type == FFI_TYPE_STRUCT
		  /* The caller allocated space for the return
		     structure, and passed a pointer to this space in
		     R9.  */
		  ? struct_ret

		  /* We take advantage of being able to ignore that
		     the high part isn't set if the return value is
		     not in R10:R11, but in R10 only.  */
		  : (فراغ *) &llret,

		  avalue, تړون->user_معلومات);

  أرجع llret;
}

/* API function: Prepare the trampoline.  */

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون* تړون,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif *, فراغ *, فراغ **, فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  فراغ *innerfn = ffi_prep_تړون_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  تړون->cif  = cif;
  تړون->user_معلومات = user_معلومات;
  تړون->fun  = fun;
  حافظه_نقل (تړون->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_کوډ_PART_اندازه);
  حافظه_نقل (تړون->tramp + ffi_cris_trampoline_fn_دننه_واټن,
	  &innerfn, حجم (فراغ *));
  حافظه_نقل (تړون->tramp + ffi_cris_trampoline_تړون_دننه_واټن,
	  &codeloc, حجم (فراغ *));

  أرجع FFI_OK;
}
