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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#define ଗଦା_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

ସ୍ଥିର ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != ଖାଲି);

  FFI_ASSERT (arg->elements != ଖାଲି);
  FFI_ASSERT (arg->size == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  ଯେତେବେଳେ ((*ptr) != ଖାଲି)
    {
      ଯଦି (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->size += (*ptr)->size;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  ଯଦି (arg->size == 0)
    ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ FFI_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_prep_args (ଅକ୍ଷର *ଗଦା, extended_cif * ecif)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା struct_ଗଣନା = 0;
  ଶୂନ୍ୟ **p_argv;
  ଅକ୍ଷର *argp;
  ffi_type **p_arg;

  argp = ଗଦା;

  p_argv = ecif->avalue;

  ପାଇଁ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      ବାଛ ((*p_arg)->type)
	{
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->size;
	    ଯଦି (z <= 4)
	      {
		memcpy (argp, *p_argv, z);
		z = 4;
	      }
	    ଅନ୍ୟଥା ଯଦି (z <= 8)
	      {
		memcpy (argp, *p_argv, z);
		z = 8;
	      }
	    ଅନ୍ୟଥା
	      {
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ui_Loc_On_ଗଦା;
		z = ଆକାର (ଶୂନ୍ୟ *);
		ui_Loc_On_ଗଦା = 4 * ecif->cif->nargs + struct_ଗଣନା;
		struct_ଗଣନା = struct_ଗଣନା + (*p_arg)->size;
		*(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp =
		  (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (UINT32 *) (ଗଦା + ui_Loc_On_ଗଦା);
		memcpy ((ଗଦା + ui_Loc_On_ଗଦା), *p_argv, (*p_arg)->size);
	      }
	    ଭାଙ୍ଗ;
	  }
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  z = (*p_arg)->size;
	  ଯଦି (z < ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	    {
	      ବାଛ ((*p_arg)->type)
		{
		ସ୍ଥିତି FFI_TYPE_SINT8:
		  *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(SINT8 *) (*p_argv);
		  ଭାଙ୍ଗ;

		ସ୍ଥିତି FFI_TYPE_UINT8:
		  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp =
		    (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(UINT8 *) (*p_argv);
		  ଭାଙ୍ଗ;

		ସ୍ଥିତି FFI_TYPE_SINT16:
		  *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(SINT16 *) (*p_argv);
		  ଭାଙ୍ଗ;

		ସ୍ଥିତି FFI_TYPE_UINT16:
		  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp =
		    (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(UINT16 *) (*p_argv);
		  ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		  FFI_ASSERT (0);
		}
	      z = ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	    }
	  ଅନ୍ୟଥା ଯଦି (z == ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(UINT32 *) (*p_argv);
	  ଅନ୍ୟଥା
	    memcpy (argp, *p_argv, z);
	  ଭାଙ୍ଗ;
	}
      p_argv++;
      argp += z;
    }

  ଫେରନ୍ତୁ (struct_ଗଣନା);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା isvariadic,
		   unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  unsigned bytes = 0;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ffi_type **ptr;

  FFI_ASSERT (cif != ଖାଲି);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  ଯଦି ((cif->rtype->size == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  ପାଇଁ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ଯଦି (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      ଯଦି (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      ଯଦି ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  ଯଦି ((*ptr)->size > 8)
	    {
	      bytes += (*ptr)->size;
	      bytes += ଆକାର (ଶୂନ୍ୟ *);
	    }
	  ଅନ୍ୟଥା
	    {
	      ଯଦି ((*ptr)->size > 4)
		bytes += 8;
	      ଅନ୍ୟଥା
		bytes += 4;
	    }
	}
      ଅନ୍ୟଥା
	bytes += ଗଦା_ARG_SIZE ((*ptr)->size);
    }

  cif->bytes = bytes;

  ଫେରନ୍ତୁ ffi_prep_cif_machdep (cif);
}

ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
    ସ୍ଥିତି FFI_TYPE_STRUCT:
    ସ୍ଥିତି FFI_TYPE_FLOAT:
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
      cif->flags = (unsigned) cif->rtype->type;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      cif->flags = FFI_TYPE_INT;
      ଭାଙ୍ଗ;
    }

  ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV (ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*)(ଅକ୍ଷର *, extended_cif *),
			   extended_cif *,
			   unsigned, unsigned, unsigned *, ଶୂନ୍ୟ (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

ଶୂନ୍ୟ
ffi_call (ffi_cif * cif, ଶୂନ୍ୟ (*fn) (), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ଯଦି ((rvalue == ଖାଲି) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT (0);
      ଭାଙ୍ଗ;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
ବାହ୍ୟ ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
ବାହ୍ୟ ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

ସ୍ଥିର unsigned long long
ffi_prep_closure_inner (ଶୂନ୍ୟ **params, ffi_closure* closure)
{
  ଅକ୍ଷର *register_args = (ଅକ୍ଷର *) params;
  ଶୂନ୍ୟ *struct_ret = params[5];
  ଅକ୍ଷର *ଗଦା_args = params[6];
  ଅକ୍ଷର *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  ଶୂନ୍ୟ **avalue = alloca (closure->cif->nargs * ଆକାର(ଶୂନ୍ୟ *));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା doing_regs;
  long long llret = 0;

  /* Find the address of each argument.  */
  ପାଇଁ (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      ଯଦି (arg_types[i]->size <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      ଅନ୍ୟଥା ଯଦି (arg_types[i]->size <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      ଅନ୍ୟଥା
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(ଶୂନ୍ୟ **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      ଯଦି (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = ଗଦା_args + ((ptr > register_args + 4*4) ? 4 : 0);
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
		  : (ଶୂନ୍ୟ *) &llret,

		  avalue, closure->user_data);

  ଫେରନ୍ତୁ llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  ଶୂନ୍ୟ *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;
  memcpy (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_SIZE);
  memcpy (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, ଆକାର (ଶୂନ୍ୟ *));
  memcpy (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, ଆକାର (ଶୂନ୍ୟ *));

  ଫେରନ୍ତୁ FFI_OK;
}
