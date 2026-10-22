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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_ARG_ਮਾਪ(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

ਸਥਿਰ ffi_ਸਥਿਤੀ
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != ਨਿਰਾ);

  FFI_ASSERT (arg->elements != ਨਿਰਾ);
  FFI_ASSERT (arg->ਮਾਪ == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  ਜਦੋਂ ((*ptr) != ਨਿਰਾ)
    {
      ਜੇ (((*ptr)->ਮਾਪ == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	ਵਾਪਸ FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->ਮਾਪ += (*ptr)->ਮਾਪ;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  ਜੇ (arg->ਮਾਪ == 0)
    ਵਾਪਸ FFI_BAD_TYPEDEF;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ FFI_OK;
}

ਪੂਰਨ
ffi_prep_ਦਲੀਲਾਂ (ਅੱਖਰ *ਢੇਰ, extended_cif * ecif)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ struct_ਗਿਣਤੀ = 0;
  ਖਾਲੀ **p_argv;
  ਅੱਖਰ *argp;
  ffi_type **p_arg;

  argp = ਢੇਰ;

  p_argv = ecif->avalue;

  ਲਈ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      ਚੋਣ ((*p_arg)->type)
	{
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->ਮਾਪ;
	    ਜੇ (z <= 4)
	      {
		ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
		z = 4;
	      }
	    ਨਹੀਂਤਾਂ ਜੇ (z <= 8)
	      {
		ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
		z = 8;
	      }
	    ਨਹੀਂਤਾਂ
	      {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ui_Loc_On_ਢੇਰ;
		z = ਆਕਾਰ (ਖਾਲੀ *);
		ui_Loc_On_ਢੇਰ = 4 * ecif->cif->nargs + struct_ਗਿਣਤੀ;
		struct_ਗਿਣਤੀ = struct_ਗਿਣਤੀ + (*p_arg)->ਮਾਪ;
		*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp =
		  (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) (UINT32 *) (ਢੇਰ + ui_Loc_On_ਢੇਰ);
		ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਢੇਰ + ui_Loc_On_ਢੇਰ), *p_argv, (*p_arg)->ਮਾਪ);
	      }
	    ਰੋਕੋ;
	  }
	ਮੂਲ:
	  z = (*p_arg)->ਮਾਪ;
	  ਜੇ (z < ਆਕਾਰ (ਪੂਰਨ))
	    {
	      ਚੋਣ ((*p_arg)->type)
		{
		ਹਾਲਤ FFI_TYPE_SINT8:
		  *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ) *(SINT8 *) (*p_argv);
		  ਰੋਕੋ;

		ਹਾਲਤ FFI_TYPE_UINT8:
		  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp =
		    (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) *(UINT8 *) (*p_argv);
		  ਰੋਕੋ;

		ਹਾਲਤ FFI_TYPE_SINT16:
		  *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ) *(SINT16 *) (*p_argv);
		  ਰੋਕੋ;

		ਹਾਲਤ FFI_TYPE_UINT16:
		  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp =
		    (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) *(UINT16 *) (*p_argv);
		  ਰੋਕੋ;

		ਮੂਲ:
		  FFI_ASSERT (0);
		}
	      z = ਆਕਾਰ (ਪੂਰਨ);
	    }
	  ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ (ਪੂਰਨ))
	    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) *(UINT32 *) (*p_argv);
	  ਨਹੀਂਤਾਂ
	    ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
	  ਰੋਕੋ;
	}
      p_argv++;
      argp += z;
    }

  ਵਾਪਸ (struct_ਗਿਣਤੀ);
}

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ isvariadic,
		   ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ffi_type **ptr;

  FFI_ASSERT (cif != ਨਿਰਾ);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_ਪਹਿਲਾ_ABI && abi < FFI_ਆਖਰੀ_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  ਜੇ ((cif->rtype->ਮਾਪ == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    ਵਾਪਸ FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  ਲਈ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ਜੇ (((*ptr)->ਮਾਪ == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	ਵਾਪਸ FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      ਜੇ (((*ptr)->alignment - 1) & ਬਾਈਟਾਂ)
	ਬਾਈਟਾਂ = FFI_ALIGN (ਬਾਈਟਾਂ, (*ptr)->alignment);
      ਜੇ ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  ਜੇ ((*ptr)->ਮਾਪ > 8)
	    {
	      ਬਾਈਟਾਂ += (*ptr)->ਮਾਪ;
	      ਬਾਈਟਾਂ += ਆਕਾਰ (ਖਾਲੀ *);
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      ਜੇ ((*ptr)->ਮਾਪ > 4)
		ਬਾਈਟਾਂ += 8;
	      ਨਹੀਂਤਾਂ
		ਬਾਈਟਾਂ += 4;
	    }
	}
      ਨਹੀਂਤਾਂ
	ਬਾਈਟਾਂ += ਢੇਰ_ARG_ਮਾਪ ((*ptr)->ਮਾਪ);
    }

  cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;

  ਵਾਪਸ ffi_prep_cif_machdep (cif);
}

ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep (ffi_cif * cif)
{
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
    ਹਾਲਤ FFI_TYPE_STRUCT:
    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ) cif->rtype->type;
      ਰੋਕੋ;

    ਮੂਲ:
      cif->flags = FFI_TYPE_INT;
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV (ਪੂਰਨ (*)(ਅੱਖਰ *, extended_cif *),
			   extended_cif *,
			   ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *, ਖਾਲੀ (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

ਖਾਲੀ
ffi_call (ffi_cif * cif, ਖਾਲੀ (*fn) (), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ਜੇ ((rvalue == ਨਿਰਾ) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->ਮਾਪ);
    }
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_SYSV:
      ffi_call_SYSV (ffi_prep_ਦਲੀਲਾਂ, &ecif, cif->ਬਾਈਟਾਂ,
		     cif->flags, ecif.rvalue, fn);
      ਰੋਕੋ;
    ਮੂਲ:
      FFI_ASSERT (0);
      ਰੋਕੋ;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
ਬਾਹਰੀ ਅਚਲ ਪੂਰਨ ffi_cris_trampoline_fn_ਵਿੱਥ
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
ਬਾਹਰੀ ਅਚਲ ਪੂਰਨ ffi_cris_trampoline_ਘੇਰਾ_ਵਿੱਥ
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ
ffi_prep_ਘੇਰਾ_inner (ਖਾਲੀ **params, ffi_ਘੇਰਾ* ਘੇਰਾ)
{
  ਅੱਖਰ *ਰਜਿਸਟਰ_ਦਲੀਲਾਂ = (ਅੱਖਰ *) params;
  ਖਾਲੀ *struct_ret = params[5];
  ਅੱਖਰ *ਢੇਰ_ਦਲੀਲਾਂ = params[6];
  ਅੱਖਰ *ptr = ਰਜਿਸਟਰ_ਦਲੀਲਾਂ;
  ffi_cif *cif = ਘੇਰਾ->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  ਖਾਲੀ **avalue = alloca (ਘੇਰਾ->cif->nargs * ਆਕਾਰ(ਖਾਲੀ *));
  ਪੂਰਨ i;
  ਪੂਰਨ doing_regs;
  ਲੰਮਾ ਲੰਮਾ llret = 0;

  /* Find the address of each argument.  */
  ਲਈ (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      ਜੇ (arg_types[i]->ਮਾਪ <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      ਨਹੀਂਤਾਂ ਜੇ (arg_types[i]->ਮਾਪ <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      ਨਹੀਂਤਾਂ
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(ਖਾਲੀ **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      ਜੇ (doing_regs && ptr >= ਰਜਿਸਟਰ_ਦਲੀਲਾਂ + 4*4)
	{
	  ptr = ਢੇਰ_ਦਲੀਲਾਂ + ((ptr > ਰਜਿਸਟਰ_ਦਲੀਲਾਂ + 4*4) ? 4 : 0);
	  doing_regs = 0;
	}
    }

  /* Invoke the closure.  */
  (ਘੇਰਾ->fun) (cif,

		  cif->rtype->type == FFI_TYPE_STRUCT
		  /* The caller allocated space for the return
		     structure, and passed a pointer to this space in
		     R9.  */
		  ? struct_ret

		  /* We take advantage of being able to ignore that
		     the high part isn't set if the return value is
		     not in R10:R11, but in R10 only.  */
		  : (ਖਾਲੀ *) &llret,

		  avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);

  ਵਾਪਸ llret;
}

/* API function: Prepare the trampoline.  */

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਖਾਲੀ *innerfn = ffi_prep_ਘੇਰਾ_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  ਘੇਰਾ->cif  = cif;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  ਘੇਰਾ->fun  = fun;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ (ਘੇਰਾ->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_ਕੋਡ_PART_ਮਾਪ);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ (ਘੇਰਾ->tramp + ffi_cris_trampoline_fn_ਵਿੱਥ,
	  &innerfn, ਆਕਾਰ (ਖਾਲੀ *));
  ਸਮ੍ਰਿਤੀ_ਨਕਲ (ਘੇਰਾ->tramp + ffi_cris_trampoline_ਘੇਰਾ_ਵਿੱਥ,
	  &codeloc, ਆਕਾਰ (ਖਾਲੀ *));

  ਵਾਪਸ FFI_OK;
}
