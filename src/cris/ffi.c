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

#define پشته_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

ایستا ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != پوچ);

  FFI_ASSERT (arg->elements != پوچ);
  FFI_ASSERT (arg->size == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  وقتی ((*ptr) != پوچ)
    {
      اگر (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	بازگشت FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->size += (*ptr)->size;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  اگر (arg->size == 0)
    بازگشت FFI_BAD_TYPEDEF;
  وگرنه
    بازگشت FFI_OK;
}

صحیح
ffi_prep_args (نویسه *پشته, extended_cif * ecif)
{
  unsigned صحیح i;
  unsigned صحیح struct_شمار = 0;
  تهی **p_argv;
  نویسه *argp;
  ffi_type **p_arg;

  argp = پشته;

  p_argv = ecif->avalue;

  برای (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      انتخاب ((*p_arg)->type)
	{
	حالت FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->size;
	    اگر (z <= 4)
	      {
		memcpy (argp, *p_argv, z);
		z = 4;
	      }
	    وگرنه اگر (z <= 8)
	      {
		memcpy (argp, *p_argv, z);
		z = 8;
	      }
	    وگرنه
	      {
		unsigned صحیح ui_Loc_On_پشته;
		z = اندازه (تهی *);
		ui_Loc_On_پشته = 4 * ecif->cif->nargs + struct_شمار;
		struct_شمار = struct_شمار + (*p_arg)->size;
		*(unsigned صحیح *) argp =
		  (unsigned صحیح) (UINT32 *) (پشته + ui_Loc_On_پشته);
		memcpy ((پشته + ui_Loc_On_پشته), *p_argv, (*p_arg)->size);
	      }
	    بشکن;
	  }
	پیشفرض:
	  z = (*p_arg)->size;
	  اگر (z < اندازه (صحیح))
	    {
	      انتخاب ((*p_arg)->type)
		{
		حالت FFI_TYPE_SINT8:
		  *(signed صحیح *) argp = (signed صحیح) *(SINT8 *) (*p_argv);
		  بشکن;

		حالت FFI_TYPE_UINT8:
		  *(unsigned صحیح *) argp =
		    (unsigned صحیح) *(UINT8 *) (*p_argv);
		  بشکن;

		حالت FFI_TYPE_SINT16:
		  *(signed صحیح *) argp = (signed صحیح) *(SINT16 *) (*p_argv);
		  بشکن;

		حالت FFI_TYPE_UINT16:
		  *(unsigned صحیح *) argp =
		    (unsigned صحیح) *(UINT16 *) (*p_argv);
		  بشکن;

		پیشفرض:
		  FFI_ASSERT (0);
		}
	      z = اندازه (صحیح);
	    }
	  وگرنه اگر (z == اندازه (صحیح))
	    *(unsigned صحیح *) argp = (unsigned صحیح) *(UINT32 *) (*p_argv);
	  وگرنه
	    memcpy (argp, *p_argv, z);
	  بشکن;
	}
      p_argv++;
      argp += z;
    }

  بازگشت (struct_شمار);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, unsigned صحیح isvariadic,
		   unsigned صحیح nfixedargs, unsigned صحیح ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  unsigned bytes = 0;
  unsigned صحیح i;
  ffi_type **ptr;

  FFI_ASSERT (cif != پوچ);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  اگر ((cif->rtype->size == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    بازگشت FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  برای (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      اگر (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	بازگشت FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      اگر (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      اگر ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  اگر ((*ptr)->size > 8)
	    {
	      bytes += (*ptr)->size;
	      bytes += اندازه (تهی *);
	    }
	  وگرنه
	    {
	      اگر ((*ptr)->size > 4)
		bytes += 8;
	      وگرنه
		bytes += 4;
	    }
	}
      وگرنه
	bytes += پشته_ARG_SIZE ((*ptr)->size);
    }

  cif->bytes = bytes;

  بازگشت ffi_prep_cif_machdep (cif);
}

ffi_status
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
      cif->flags = (unsigned) cif->rtype->type;
      بشکن;

    پیشفرض:
      cif->flags = FFI_TYPE_INT;
      بشکن;
    }

  بازگشت FFI_OK;
}

بیرونی تهی ffi_call_SYSV (صحیح (*)(نویسه *, extended_cif *),
			   extended_cif *,
			   unsigned, unsigned, unsigned *, تهی (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

تهی
ffi_call (ffi_cif * cif, تهی (*fn) (), تهی *rvalue, تهی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  اگر ((rvalue == پوچ) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  وگرنه
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      بشکن;
    پیشفرض:
      FFI_ASSERT (0);
      بشکن;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
بیرونی ثابت نویسه ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
بیرونی ثابت صحیح ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
بیرونی ثابت صحیح ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

ایستا unsigned long long
ffi_prep_closure_inner (تهی **params, ffi_closure* closure)
{
  نویسه *register_args = (نویسه *) params;
  تهی *struct_ret = params[5];
  نویسه *پشته_args = params[6];
  نویسه *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  تهی **avalue = alloca (closure->cif->nargs * اندازه(تهی *));
  صحیح i;
  صحیح doing_regs;
  long long llret = 0;

  /* Find the address of each argument.  */
  برای (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      اگر (arg_types[i]->size <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      وگرنه اگر (arg_types[i]->size <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      وگرنه
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(تهی **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      اگر (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = پشته_args + ((ptr > register_args + 4*4) ? 4 : 0);
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
		  : (تهی *) &llret,

		  avalue, closure->user_data);

  بازگشت llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      تهی (*fun)(ffi_cif *, تهی *, تهی **, تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  تهی *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;
  memcpy (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_SIZE);
  memcpy (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, اندازه (تهی *));
  memcpy (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, اندازه (تهی *));

  بازگشت FFI_OK;
}
