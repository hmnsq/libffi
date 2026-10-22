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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_ARG_ಗಾತ್ರ(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

ಸ್ಥಿರ ffi_ಸ್ಥಿತಿ
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != ಖಾಲಿ);

  FFI_ASSERT (arg->elements != ಖಾಲಿ);
  FFI_ASSERT (arg->ಗಾತ್ರ_2 == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  ಆಗ ((*ptr) != ಖಾಲಿ)
    {
      ಆದರೆ (((*ptr)->ಗಾತ್ರ_2 == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->ಗಾತ್ರ_2 += (*ptr)->ಗಾತ್ರ_2;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  ಆದರೆ (arg->ಗಾತ್ರ_2 == 0)
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ FFI_OK;
}

ಪೂರ್ಣಾಂಕ
ffi_prep_ವಾದಗಳು (ಅಕ್ಷರ *ರಾಶಿ, extended_cif * ecif)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ struct_ಎಣಿಕೆ = 0;
  ಶೂನ್ಯ **p_argv;
  ಅಕ್ಷರ *argp;
  ffi_type **p_arg;

  argp = ರಾಶಿ;

  p_argv = ecif->avalue;

  ಗಾಗಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      ಆಯ್ಕೆ ((*p_arg)->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->ಗಾತ್ರ_2;
	    ಆದರೆ (z <= 4)
	      {
		ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
		z = 4;
	      }
	    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z <= 8)
	      {
		ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
		z = 8;
	      }
	    ಇಲ್ಲದಿದ್ದರೆ
	      {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ui_Loc_On_ರಾಶಿ;
		z = ಗಾತ್ರ (ಶೂನ್ಯ *);
		ui_Loc_On_ರಾಶಿ = 4 * ecif->cif->nargs + struct_ಎಣಿಕೆ;
		struct_ಎಣಿಕೆ = struct_ಎಣಿಕೆ + (*p_arg)->ಗಾತ್ರ_2;
		*(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp =
		  (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) (UINT32 *) (ರಾಶಿ + ui_Loc_On_ರಾಶಿ);
		ಸ್ಮರಣೆ_ನಕಲು ((ರಾಶಿ + ui_Loc_On_ರಾಶಿ), *p_argv, (*p_arg)->ಗಾತ್ರ_2);
	      }
	    ನಿಲ್ಲಿಸು;
	  }
	ಪೂರ್ವನಿಯೋಜಿತ:
	  z = (*p_arg)->ಗಾತ್ರ_2;
	  ಆದರೆ (z < ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	    {
	      ಆಯ್ಕೆ ((*p_arg)->type)
		{
		ಸ್ಥಿತಿ FFI_TYPE_SINT8:
		  *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ) *(SINT8 *) (*p_argv);
		  ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ FFI_TYPE_UINT8:
		  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp =
		    (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) *(UINT8 *) (*p_argv);
		  ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ FFI_TYPE_SINT16:
		  *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ) *(SINT16 *) (*p_argv);
		  ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ FFI_TYPE_UINT16:
		  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp =
		    (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) *(UINT16 *) (*p_argv);
		  ನಿಲ್ಲಿಸು;

		ಪೂರ್ವನಿಯೋಜಿತ:
		  FFI_ASSERT (0);
		}
	      z = ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	    }
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z == ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	    *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) *(UINT32 *) (*p_argv);
	  ಇಲ್ಲದಿದ್ದರೆ
	    ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
	  ನಿಲ್ಲಿಸು;
	}
      p_argv++;
      argp += z;
    }

  ಹಿಂತಿರುಗಿ (struct_ಎಣಿಕೆ);
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ isvariadic,
		   ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು = 0;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ffi_type **ptr;

  FFI_ASSERT (cif != ಖಾಲಿ);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_ಮೊದಲ_ABI && abi < FFI_ಕೊನೆಯ_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  ಆದರೆ ((cif->rtype->ಗಾತ್ರ_2 == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  ಗಾಗಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ಆದರೆ (((*ptr)->ಗಾತ್ರ_2 == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      ಆದರೆ (((*ptr)->alignment - 1) & ಬೈಟುಗಳು)
	ಬೈಟುಗಳು = FFI_ALIGN (ಬೈಟುಗಳು, (*ptr)->alignment);
      ಆದರೆ ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  ಆದರೆ ((*ptr)->ಗಾತ್ರ_2 > 8)
	    {
	      ಬೈಟುಗಳು += (*ptr)->ಗಾತ್ರ_2;
	      ಬೈಟುಗಳು += ಗಾತ್ರ (ಶೂನ್ಯ *);
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      ಆದರೆ ((*ptr)->ಗಾತ್ರ_2 > 4)
		ಬೈಟುಗಳು += 8;
	      ಇಲ್ಲದಿದ್ದರೆ
		ಬೈಟುಗಳು += 4;
	    }
	}
      ಇಲ್ಲದಿದ್ದರೆ
	ಬೈಟುಗಳು += ರಾಶಿ_ARG_ಗಾತ್ರ ((*ptr)->ಗಾತ್ರ_2);
    }

  cif->ಬೈಟುಗಳು = ಬೈಟುಗಳು;

  ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep (cif);
}

ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep (ffi_cif * cif)
{
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      cif->flags = (ಚಿಹ್ನೆರಹಿತ) cif->rtype->type;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      cif->flags = FFI_TYPE_INT;
      ನಿಲ್ಲಿಸು;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV (ಪೂರ್ಣಾಂಕ (*)(ಅಕ್ಷರ *, extended_cif *),
			   extended_cif *,
			   ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ *, ಶೂನ್ಯ (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

ಶೂನ್ಯ
ffi_call (ffi_cif * cif, ಶೂನ್ಯ (*fn) (), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ಆದರೆ ((rvalue == ಖಾಲಿ) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_SYSV:
      ffi_call_SYSV (ffi_prep_ವಾದಗಳು, &ecif, cif->ಬೈಟುಗಳು,
		     cif->flags, ecif.rvalue, fn);
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT (0);
      ನಿಲ್ಲಿಸು;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ffi_cris_trampoline_fn_ಸ್ಥಾನಾಂತರ
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ffi_cris_trampoline_ಆವರಣ_ಸ್ಥಾನಾಂತರ
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ
ffi_prep_ಆವರಣ_inner (ಶೂನ್ಯ **params, ffi_ಆವರಣ* ಆವರಣ)
{
  ಅಕ್ಷರ *ರಿಜಿಸ್ಟರ್_ವಾದಗಳು = (ಅಕ್ಷರ *) params;
  ಶೂನ್ಯ *struct_ret = params[5];
  ಅಕ್ಷರ *ರಾಶಿ_ವಾದಗಳು = params[6];
  ಅಕ್ಷರ *ptr = ರಿಜಿಸ್ಟರ್_ವಾದಗಳು;
  ffi_cif *cif = ಆವರಣ->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  ಶೂನ್ಯ **avalue = alloca (ಆವರಣ->cif->nargs * ಗಾತ್ರ(ಶೂನ್ಯ *));
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ doing_regs;
  ದೀರ್ಘ ದೀರ್ಘ llret = 0;

  /* Find the address of each argument.  */
  ಗಾಗಿ (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      ಆದರೆ (arg_types[i]->ಗಾತ್ರ_2 <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg_types[i]->ಗಾತ್ರ_2 <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(ಶೂನ್ಯ **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      ಆದರೆ (doing_regs && ptr >= ರಿಜಿಸ್ಟರ್_ವಾದಗಳು + 4*4)
	{
	  ptr = ರಾಶಿ_ವಾದಗಳು + ((ptr > ರಿಜಿಸ್ಟರ್_ವಾದಗಳು + 4*4) ? 4 : 0);
	  doing_regs = 0;
	}
    }

  /* Invoke the closure.  */
  (ಆವರಣ->fun) (cif,

		  cif->rtype->type == FFI_TYPE_STRUCT
		  /* The caller allocated space for the return
		     structure, and passed a pointer to this space in
		     R9.  */
		  ? struct_ret

		  /* We take advantage of being able to ignore that
		     the high part isn't set if the return value is
		     not in R10:R11, but in R10 only.  */
		  : (ಶೂನ್ಯ *) &llret,

		  avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);

  ಹಿಂತಿರುಗಿ llret;
}

/* API function: Prepare the trampoline.  */

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಶೂನ್ಯ *innerfn = ffi_prep_ಆವರಣ_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  ಆವರಣ->cif  = cif;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
  ಆವರಣ->fun  = fun;
  ಸ್ಮರಣೆ_ನಕಲು (ಆವರಣ->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_ಸಂಕೇತ_PART_ಗಾತ್ರ);
  ಸ್ಮರಣೆ_ನಕಲು (ಆವರಣ->tramp + ffi_cris_trampoline_fn_ಸ್ಥಾನಾಂತರ,
	  &innerfn, ಗಾತ್ರ (ಶೂನ್ಯ *));
  ಸ್ಮರಣೆ_ನಕಲು (ಆವರಣ->tramp + ffi_cris_trampoline_ಆವರಣ_ಸ್ಥಾನಾಂತರ,
	  &codeloc, ಗಾತ್ರ (ಶೂನ್ಯ *));

  ಹಿಂತಿರುಗಿ FFI_OK;
}
