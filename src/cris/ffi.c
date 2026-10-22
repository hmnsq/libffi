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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#define அடுக்கு_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

நிலையான ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != வெற்று);

  FFI_ASSERT (arg->elements != வெற்று);
  FFI_ASSERT (arg->size == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  போது ((*ptr) != வெற்று)
    {
      எனில் (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	திரும்பு FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->size += (*ptr)->size;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  எனில் (arg->size == 0)
    திரும்பு FFI_BAD_TYPEDEF;
  இல்லையெனில்
    திரும்பு FFI_OK;
}

முழு
ffi_prep_args (எழுத்து *அடுக்கு, extended_cif * ecif)
{
  unsigned முழு i;
  unsigned முழு struct_மொத்தம் = 0;
  வெறுமை **p_argv;
  எழுத்து *argp;
  ffi_type **p_arg;

  argp = அடுக்கு;

  p_argv = ecif->avalue;

  க்கு (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      தேர்வு ((*p_arg)->type)
	{
	நிலை FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->size;
	    எனில் (z <= 4)
	      {
		memcpy (argp, *p_argv, z);
		z = 4;
	      }
	    இல்லையெனில் எனில் (z <= 8)
	      {
		memcpy (argp, *p_argv, z);
		z = 8;
	      }
	    இல்லையெனில்
	      {
		unsigned முழு ui_Loc_On_அடுக்கு;
		z = அளவு (வெறுமை *);
		ui_Loc_On_அடுக்கு = 4 * ecif->cif->nargs + struct_மொத்தம்;
		struct_மொத்தம் = struct_மொத்தம் + (*p_arg)->size;
		*(unsigned முழு *) argp =
		  (unsigned முழு) (UINT32 *) (அடுக்கு + ui_Loc_On_அடுக்கு);
		memcpy ((அடுக்கு + ui_Loc_On_அடுக்கு), *p_argv, (*p_arg)->size);
	      }
	    நிறுத்து;
	  }
	இயல்பு:
	  z = (*p_arg)->size;
	  எனில் (z < அளவு (முழு))
	    {
	      தேர்வு ((*p_arg)->type)
		{
		நிலை FFI_TYPE_SINT8:
		  *(signed முழு *) argp = (signed முழு) *(SINT8 *) (*p_argv);
		  நிறுத்து;

		நிலை FFI_TYPE_UINT8:
		  *(unsigned முழு *) argp =
		    (unsigned முழு) *(UINT8 *) (*p_argv);
		  நிறுத்து;

		நிலை FFI_TYPE_SINT16:
		  *(signed முழு *) argp = (signed முழு) *(SINT16 *) (*p_argv);
		  நிறுத்து;

		நிலை FFI_TYPE_UINT16:
		  *(unsigned முழு *) argp =
		    (unsigned முழு) *(UINT16 *) (*p_argv);
		  நிறுத்து;

		இயல்பு:
		  FFI_ASSERT (0);
		}
	      z = அளவு (முழு);
	    }
	  இல்லையெனில் எனில் (z == அளவு (முழு))
	    *(unsigned முழு *) argp = (unsigned முழு) *(UINT32 *) (*p_argv);
	  இல்லையெனில்
	    memcpy (argp, *p_argv, z);
	  நிறுத்து;
	}
      p_argv++;
      argp += z;
    }

  திரும்பு (struct_மொத்தம்);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, unsigned முழு isvariadic,
		   unsigned முழு nfixedargs, unsigned முழு ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  unsigned bytes = 0;
  unsigned முழு i;
  ffi_type **ptr;

  FFI_ASSERT (cif != வெற்று);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  எனில் ((cif->rtype->size == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    திரும்பு FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  க்கு (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      எனில் (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	திரும்பு FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      எனில் (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      எனில் ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  எனில் ((*ptr)->size > 8)
	    {
	      bytes += (*ptr)->size;
	      bytes += அளவு (வெறுமை *);
	    }
	  இல்லையெனில்
	    {
	      எனில் ((*ptr)->size > 4)
		bytes += 8;
	      இல்லையெனில்
		bytes += 4;
	    }
	}
      இல்லையெனில்
	bytes += அடுக்கு_ARG_SIZE ((*ptr)->size);
    }

  cif->bytes = bytes;

  திரும்பு ffi_prep_cif_machdep (cif);
}

ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_VOID:
    நிலை FFI_TYPE_STRUCT:
    நிலை FFI_TYPE_FLOAT:
    நிலை FFI_TYPE_DOUBLE:
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
      cif->flags = (unsigned) cif->rtype->type;
      நிறுத்து;

    இயல்பு:
      cif->flags = FFI_TYPE_INT;
      நிறுத்து;
    }

  திரும்பு FFI_OK;
}

வெளி வெறுமை ffi_call_SYSV (முழு (*)(எழுத்து *, extended_cif *),
			   extended_cif *,
			   unsigned, unsigned, unsigned *, வெறுமை (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

வெறுமை
ffi_call (ffi_cif * cif, வெறுமை (*fn) (), வெறுமை *rvalue, வெறுமை **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  எனில் ((rvalue == வெற்று) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  இல்லையெனில்
    ecif.rvalue = rvalue;

  தேர்வு (cif->abi)
    {
    நிலை FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      நிறுத்து;
    இயல்பு:
      FFI_ASSERT (0);
      நிறுத்து;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
வெளி மாறிலி எழுத்து ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
வெளி மாறிலி முழு ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
வெளி மாறிலி முழு ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

நிலையான unsigned long long
ffi_prep_closure_inner (வெறுமை **params, ffi_closure* closure)
{
  எழுத்து *register_args = (எழுத்து *) params;
  வெறுமை *struct_ret = params[5];
  எழுத்து *அடுக்கு_args = params[6];
  எழுத்து *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  வெறுமை **avalue = alloca (closure->cif->nargs * அளவு(வெறுமை *));
  முழு i;
  முழு doing_regs;
  long long llret = 0;

  /* Find the address of each argument.  */
  க்கு (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      எனில் (arg_types[i]->size <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      இல்லையெனில் எனில் (arg_types[i]->size <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      இல்லையெனில்
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(வெறுமை **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      எனில் (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = அடுக்கு_args + ((ptr > register_args + 4*4) ? 4 : 0);
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
		  : (வெறுமை *) &llret,

		  avalue, closure->user_data);

  திரும்பு llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  வெறுமை *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;
  memcpy (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_SIZE);
  memcpy (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, அளவு (வெறுமை *));
  memcpy (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, அளவு (வெறுமை *));

  திரும்பு FFI_OK;
}
