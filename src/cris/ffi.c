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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#define স্তূপ_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

স্থির ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != রিক্ত);

  FFI_ASSERT (arg->elements != রিক্ত);
  FFI_ASSERT (arg->size == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  যাবত্ ((*ptr) != রিক্ত)
    {
      যদি (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	প্রত্যাগম FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->size += (*ptr)->size;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  যদি (arg->size == 0)
    প্রত্যাগম FFI_BAD_TYPEDEF;
  অন্যথা
    প্রত্যাগম FFI_OK;
}

পূর্ণাঙ্ক
ffi_prep_args (বর্ণ *স্তূপ, extended_cif * ecif)
{
  unsigned পূর্ণাঙ্ক i;
  unsigned পূর্ণাঙ্ক struct_গুনতি = 0;
  শূন্য **p_argv;
  বর্ণ *argp;
  ffi_type **p_arg;

  argp = স্তূপ;

  p_argv = ecif->avalue;

  প্রতি (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      বিকল্প ((*p_arg)->type)
	{
	পক্ষ FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->size;
	    যদি (z <= 4)
	      {
		memcpy (argp, *p_argv, z);
		z = 4;
	      }
	    অন্যথা যদি (z <= 8)
	      {
		memcpy (argp, *p_argv, z);
		z = 8;
	      }
	    অন্যথা
	      {
		unsigned পূর্ণাঙ্ক ui_Loc_On_স্তূপ;
		z = পরিমাণ (শূন্য *);
		ui_Loc_On_স্তূপ = 4 * ecif->cif->nargs + struct_গুনতি;
		struct_গুনতি = struct_গুনতি + (*p_arg)->size;
		*(unsigned পূর্ণাঙ্ক *) argp =
		  (unsigned পূর্ণাঙ্ক) (UINT32 *) (স্তূপ + ui_Loc_On_স্তূপ);
		memcpy ((স্তূপ + ui_Loc_On_স্তূপ), *p_argv, (*p_arg)->size);
	      }
	    ভঙ্গ;
	  }
	স্বভাব:
	  z = (*p_arg)->size;
	  যদি (z < পরিমাণ (পূর্ণাঙ্ক))
	    {
	      বিকল্প ((*p_arg)->type)
		{
		পক্ষ FFI_TYPE_SINT8:
		  *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক) *(SINT8 *) (*p_argv);
		  ভঙ্গ;

		পক্ষ FFI_TYPE_UINT8:
		  *(unsigned পূর্ণাঙ্ক *) argp =
		    (unsigned পূর্ণাঙ্ক) *(UINT8 *) (*p_argv);
		  ভঙ্গ;

		পক্ষ FFI_TYPE_SINT16:
		  *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক) *(SINT16 *) (*p_argv);
		  ভঙ্গ;

		পক্ষ FFI_TYPE_UINT16:
		  *(unsigned পূর্ণাঙ্ক *) argp =
		    (unsigned পূর্ণাঙ্ক) *(UINT16 *) (*p_argv);
		  ভঙ্গ;

		স্বভাব:
		  FFI_ASSERT (0);
		}
	      z = পরিমাণ (পূর্ণাঙ্ক);
	    }
	  অন্যথা যদি (z == পরিমাণ (পূর্ণাঙ্ক))
	    *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক) *(UINT32 *) (*p_argv);
	  অন্যথা
	    memcpy (argp, *p_argv, z);
	  ভঙ্গ;
	}
      p_argv++;
      argp += z;
    }

  প্রত্যাগম (struct_গুনতি);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, unsigned পূর্ণাঙ্ক isvariadic,
		   unsigned পূর্ণাঙ্ক nfixedargs, unsigned পূর্ণাঙ্ক ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  unsigned bytes = 0;
  unsigned পূর্ণাঙ্ক i;
  ffi_type **ptr;

  FFI_ASSERT (cif != রিক্ত);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  যদি ((cif->rtype->size == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    প্রত্যাগম FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  প্রতি (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      যদি (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	প্রত্যাগম FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      যদি (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      যদি ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  যদি ((*ptr)->size > 8)
	    {
	      bytes += (*ptr)->size;
	      bytes += পরিমাণ (শূন্য *);
	    }
	  অন্যথা
	    {
	      যদি ((*ptr)->size > 4)
		bytes += 8;
	      অন্যথা
		bytes += 4;
	    }
	}
      অন্যথা
	bytes += স্তূপ_ARG_SIZE ((*ptr)->size);
    }

  cif->bytes = bytes;

  প্রত্যাগম ffi_prep_cif_machdep (cif);
}

ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  বিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_VOID:
    পক্ষ FFI_TYPE_STRUCT:
    পক্ষ FFI_TYPE_FLOAT:
    পক্ষ FFI_TYPE_DOUBLE:
    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
      cif->flags = (unsigned) cif->rtype->type;
      ভঙ্গ;

    স্বভাব:
      cif->flags = FFI_TYPE_INT;
      ভঙ্গ;
    }

  প্রত্যাগম FFI_OK;
}

বাহ্য শূন্য ffi_call_SYSV (পূর্ণাঙ্ক (*)(বর্ণ *, extended_cif *),
			   extended_cif *,
			   unsigned, unsigned, unsigned *, শূন্য (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

শূন্য
ffi_call (ffi_cif * cif, শূন্য (*fn) (), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  যদি ((rvalue == রিক্ত) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  অন্যথা
    ecif.rvalue = rvalue;

  বিকল্প (cif->abi)
    {
    পক্ষ FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      ভঙ্গ;
    স্বভাব:
      FFI_ASSERT (0);
      ভঙ্গ;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
বাহ্য ধ্রুব বর্ণ ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
বাহ্য ধ্রুব পূর্ণাঙ্ক ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
বাহ্য ধ্রুব পূর্ণাঙ্ক ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

স্থির unsigned long long
ffi_prep_closure_inner (শূন্য **params, ffi_closure* closure)
{
  বর্ণ *register_args = (বর্ণ *) params;
  শূন্য *struct_ret = params[5];
  বর্ণ *স্তূপ_args = params[6];
  বর্ণ *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  শূন্য **avalue = alloca (closure->cif->nargs * পরিমাণ(শূন্য *));
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক doing_regs;
  long long llret = 0;

  /* Find the address of each argument.  */
  প্রতি (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      যদি (arg_types[i]->size <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      অন্যথা যদি (arg_types[i]->size <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      অন্যথা
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(শূন্য **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      যদি (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = স্তূপ_args + ((ptr > register_args + 4*4) ? 4 : 0);
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
		  : (শূন্য *) &llret,

		  avalue, closure->user_data);

  প্রত্যাগম llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif *, শূন্য *, শূন্য **, শূন্য*),
		      শূন্য *user_data,
		      শূন্য *codeloc)
{
  শূন্য *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;
  memcpy (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_SIZE);
  memcpy (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, পরিমাণ (শূন্য *));
  memcpy (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, পরিমাণ (শূন্য *));

  প্রত্যাগম FFI_OK;
}
