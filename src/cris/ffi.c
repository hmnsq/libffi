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

#define ರಾಶಿ_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

ಸ್ಥಿರ ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != ರಿಕ್ತ);

  FFI_ASSERT (arg->elements != ರಿಕ್ತ);
  FFI_ASSERT (arg->size == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  ಯಾವತ್ ((*ptr) != ರಿಕ್ತ)
    {
      ಯದಿ (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	ಪ್ರತ್ಯಾಗಮ FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->size += (*ptr)->size;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  ಯದಿ (arg->size == 0)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_TYPEDEF;
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಪೂರ್ಣಾಙ್ಕ
ffi_prep_args (ವರ್ಣ *ರಾಶಿ, extended_cif * ecif)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  unsigned ಪೂರ್ಣಾಙ್ಕ struct_ಎಣಿಕೆ = 0;
  ಶೂನ್ಯ **p_argv;
  ವರ್ಣ *argp;
  ffi_type **p_arg;

  argp = ರಾಶಿ;

  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      ವಿಕಲ್ಪ ((*p_arg)->type)
	{
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->size;
	    ಯದಿ (z <= 4)
	      {
		memcpy (argp, *p_argv, z);
		z = 4;
	      }
	    ಅನ್ಯಥಾ ಯದಿ (z <= 8)
	      {
		memcpy (argp, *p_argv, z);
		z = 8;
	      }
	    ಅನ್ಯಥಾ
	      {
		unsigned ಪೂರ್ಣಾಙ್ಕ ui_Loc_On_ರಾಶಿ;
		z = ಪರಿಮಾಣ (ಶೂನ್ಯ *);
		ui_Loc_On_ರಾಶಿ = 4 * ecif->cif->nargs + struct_ಎಣಿಕೆ;
		struct_ಎಣಿಕೆ = struct_ಎಣಿಕೆ + (*p_arg)->size;
		*(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp =
		  (unsigned ಪೂರ್ಣಾಙ್ಕ) (UINT32 *) (ರಾಶಿ + ui_Loc_On_ರಾಶಿ);
		memcpy ((ರಾಶಿ + ui_Loc_On_ರಾಶಿ), *p_argv, (*p_arg)->size);
	      }
	    ಭಙ್ಗ;
	  }
	ಸ್ವಭಾವ:
	  z = (*p_arg)->size;
	  ಯದಿ (z < ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	    {
	      ವಿಕಲ್ಪ ((*p_arg)->type)
		{
		ಪಕ್ಷ FFI_TYPE_SINT8:
		  *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ) *(SINT8 *) (*p_argv);
		  ಭಙ್ಗ;

		ಪಕ್ಷ FFI_TYPE_UINT8:
		  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp =
		    (unsigned ಪೂರ್ಣಾಙ್ಕ) *(UINT8 *) (*p_argv);
		  ಭಙ್ಗ;

		ಪಕ್ಷ FFI_TYPE_SINT16:
		  *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ) *(SINT16 *) (*p_argv);
		  ಭಙ್ಗ;

		ಪಕ್ಷ FFI_TYPE_UINT16:
		  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp =
		    (unsigned ಪೂರ್ಣಾಙ್ಕ) *(UINT16 *) (*p_argv);
		  ಭಙ್ಗ;

		ಸ್ವಭಾವ:
		  FFI_ASSERT (0);
		}
	      z = ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	    }
	  ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	    *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ) *(UINT32 *) (*p_argv);
	  ಅನ್ಯಥಾ
	    memcpy (argp, *p_argv, z);
	  ಭಙ್ಗ;
	}
      p_argv++;
      argp += z;
    }

  ಪ್ರತ್ಯಾಗಮ (struct_ಎಣಿಕೆ);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, unsigned ಪೂರ್ಣಾಙ್ಕ isvariadic,
		   unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs, unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  unsigned bytes = 0;
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ffi_type **ptr;

  FFI_ASSERT (cif != ರಿಕ್ತ);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  ಯದಿ ((cif->rtype->size == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  ಪ್ರತಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ಯದಿ (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	ಪ್ರತ್ಯಾಗಮ FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      ಯದಿ (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      ಯದಿ ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  ಯದಿ ((*ptr)->size > 8)
	    {
	      bytes += (*ptr)->size;
	      bytes += ಪರಿಮಾಣ (ಶೂನ್ಯ *);
	    }
	  ಅನ್ಯಥಾ
	    {
	      ಯದಿ ((*ptr)->size > 4)
		bytes += 8;
	      ಅನ್ಯಥಾ
		bytes += 4;
	    }
	}
      ಅನ್ಯಥಾ
	bytes += ರಾಶಿ_ARG_SIZE ((*ptr)->size);
    }

  cif->bytes = bytes;

  ಪ್ರತ್ಯಾಗಮ ffi_prep_cif_machdep (cif);
}

ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
    ಪಕ್ಷ FFI_TYPE_STRUCT:
    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
      cif->flags = (unsigned) cif->rtype->type;
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      cif->flags = FFI_TYPE_INT;
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV (ಪೂರ್ಣಾಙ್ಕ (*)(ವರ್ಣ *, extended_cif *),
			   extended_cif *,
			   unsigned, unsigned, unsigned *, ಶೂನ್ಯ (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

ಶೂನ್ಯ
ffi_call (ffi_cif * cif, ಶೂನ್ಯ (*fn) (), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ಯದಿ ((rvalue == ರಿಕ್ತ) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      FFI_ASSERT (0);
      ಭಙ್ಗ;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
ಬಾಹ್ಯ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
ಬಾಹ್ಯ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

ಸ್ಥಿರ unsigned long long
ffi_prep_closure_inner (ಶೂನ್ಯ **params, ffi_closure* closure)
{
  ವರ್ಣ *register_args = (ವರ್ಣ *) params;
  ಶೂನ್ಯ *struct_ret = params[5];
  ವರ್ಣ *ರಾಶಿ_args = params[6];
  ವರ್ಣ *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  ಶೂನ್ಯ **avalue = alloca (closure->cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ *));
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ doing_regs;
  long long llret = 0;

  /* Find the address of each argument.  */
  ಪ್ರತಿ (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      ಯದಿ (arg_types[i]->size <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      ಅನ್ಯಥಾ ಯದಿ (arg_types[i]->size <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      ಅನ್ಯಥಾ
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(ಶೂನ್ಯ **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      ಯದಿ (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = ರಾಶಿ_args + ((ptr > register_args + 4*4) ? 4 : 0);
	  doing_regs = 0;
	}
    }

  /* Invoke the closure.  */
  (closure->fun) (cif,

		  cif->rtype->type == FFI_TYPE_STRUCT
		  /* The caller allocated space for the return
		     structure, and passed a pointer to this space in
		     R9.  */
		  ? struct_ret

		  /* We take advantage of being able to ignore that
		     the high part isn't set if the return value is
		     not in R10:R11, but in R10 only.  */
		  : (ಶೂನ್ಯ *) &llret,

		  avalue, closure->user_data);

  ಪ್ರತ್ಯಾಗಮ llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  ಶೂನ್ಯ *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;
  memcpy (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_SIZE);
  memcpy (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, ಪರಿಮಾಣ (ಶೂನ್ಯ *));
  memcpy (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, ಪರಿಮಾಣ (ಶೂನ್ಯ *));

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}
