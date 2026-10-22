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

#include <ffi.h>
#include <ffi_common.h>

#define STACK_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

static ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != NULL);

  FFI_ASSERT (arg->elements != NULL);
  FFI_ASSERT (arg->size == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  අතරතුර ((*ptr) != NULL)
    {
      නම් (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	ආපසු FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->size += (*ptr)->size;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  නම් (arg->size == 0)
    ආපසු FFI_BAD_TYPEDEF;
  නැතහොත්
    ආපසු FFI_OK;
}

පූර්ණ_සංඛ්‍යා
ffi_prep_args (var_na *stack, extended_cif * ecif)
{
  unsigned පූර්ණ_සංඛ්‍යා i;
  unsigned පූර්ණ_සංඛ්‍යා struct_count = 0;
  void **p_argv;
  var_na *argp;
  ffi_type **p_arg;

  argp = stack;

  p_argv = ecif->avalue;

  සඳහා (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      switch ((*p_arg)->type)
	{
	pak_sa FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->size;
	    නම් (z <= 4)
	      {
		memcpy (argp, *p_argv, z);
		z = 4;
	      }
	    නැතහොත් නම් (z <= 8)
	      {
		memcpy (argp, *p_argv, z);
		z = 8;
	      }
	    නැතහොත්
	      {
		unsigned පූර්ණ_සංඛ්‍යා uiLocOnStack;
		z = parimaa_na (void *);
		uiLocOnStack = 4 * ecif->cif->nargs + struct_count;
		struct_count = struct_count + (*p_arg)->size;
		*(unsigned පූර්ණ_සංඛ්‍යා *) argp =
		  (unsigned පූර්ණ_සංඛ්‍යා) (UINT32 *) (stack + uiLocOnStack);
		memcpy ((stack + uiLocOnStack), *p_argv, (*p_arg)->size);
	      }
	    bha_nga;
	  }
	default:
	  z = (*p_arg)->size;
	  නම් (z < parimaa_na (පූර්ණ_සංඛ්‍යා))
	    {
	      switch ((*p_arg)->type)
		{
		pak_sa FFI_TYPE_SINT8:
		  *(signed පූර්ණ_සංඛ්‍යා *) argp = (signed පූර්ණ_සංඛ්‍යා) *(SINT8 *) (*p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_UINT8:
		  *(unsigned පූර්ණ_සංඛ්‍යා *) argp =
		    (unsigned පූර්ණ_සංඛ්‍යා) *(UINT8 *) (*p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_SINT16:
		  *(signed පූර්ණ_සංඛ්‍යා *) argp = (signed පූර්ණ_සංඛ්‍යා) *(SINT16 *) (*p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_UINT16:
		  *(unsigned පූර්ණ_සංඛ්‍යා *) argp =
		    (unsigned පූර්ණ_සංඛ්‍යා) *(UINT16 *) (*p_argv);
		  bha_nga;

		default:
		  FFI_ASSERT (0);
		}
	      z = parimaa_na (පූර්ණ_සංඛ්‍යා);
	    }
	  නැතහොත් නම් (z == parimaa_na (පූර්ණ_සංඛ්‍යා))
	    *(unsigned පූර්ණ_සංඛ්‍යා *) argp = (unsigned පූර්ණ_සංඛ්‍යා) *(UINT32 *) (*p_argv);
	  නැතහොත්
	    memcpy (argp, *p_argv, z);
	  bha_nga;
	}
      p_argv++;
      argp += z;
    }

  ආපසු (struct_count);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, unsigned පූර්ණ_සංඛ්‍යා isvariadic,
		   unsigned පූර්ණ_සංඛ්‍යා nfixedargs, unsigned පූර්ණ_සංඛ්‍යා ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  unsigned bytes = 0;
  unsigned පූර්ණ_සංඛ්‍යා i;
  ffi_type **ptr;

  FFI_ASSERT (cif != NULL);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  නම් ((cif->rtype->size == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    ආපසු FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  සඳහා (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      නම් (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	ආපසු FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      නම් (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      නම් ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  නම් ((*ptr)->size > 8)
	    {
	      bytes += (*ptr)->size;
	      bytes += parimaa_na (void *);
	    }
	  නැතහොත්
	    {
	      නම් ((*ptr)->size > 4)
		bytes += 8;
	      නැතහොත්
		bytes += 4;
	    }
	}
      නැතහොත්
	bytes += STACK_ARG_SIZE ((*ptr)->size);
    }

  cif->bytes = bytes;

  ආපසු ffi_prep_cif_machdep (cif);
}

ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
    pak_sa FFI_TYPE_STRUCT:
    pak_sa FFI_TYPE_FLOAT:
    pak_sa FFI_TYPE_DOUBLE:
    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
      cif->flags = (unsigned) cif->rtype->type;
      bha_nga;

    default:
      cif->flags = FFI_TYPE_INT;
      bha_nga;
    }

  ආපසු FFI_OK;
}

extern void ffi_call_SYSV (පූර්ණ_සංඛ්‍යා (*)(var_na *, extended_cif *),
			   extended_cif *,
			   unsigned, unsigned, unsigned *, void (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

void
ffi_call (ffi_cif * cif, void (*fn) (), void *rvalue, void **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  නම් ((rvalue == NULL) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  නැතහොත්
    ecif.rvalue = rvalue;

  switch (cif->abi)
    {
    pak_sa FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      bha_nga;
    default:
      FFI_ASSERT (0);
      bha_nga;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
extern const var_na ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
extern const පූර්ණ_සංඛ්‍යා ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
extern const පූර්ණ_සංඛ්‍යා ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

static unsigned long long
ffi_prep_closure_inner (void **params, ffi_closure* closure)
{
  var_na *register_args = (var_na *) params;
  void *struct_ret = params[5];
  var_na *stack_args = params[6];
  var_na *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  void **avalue = alloca (closure->cif->nargs * parimaa_na(void *));
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා doing_regs;
  long long llret = 0;

  /* Find the address of each argument.  */
  සඳහා (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      නම් (arg_types[i]->size <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      නැතහොත් නම් (arg_types[i]->size <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      නැතහොත්
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(void **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      නම් (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = stack_args + ((ptr > register_args + 4*4) ? 4 : 0);
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
		  : (void *) &llret,

		  avalue, closure->user_data);

  ආපසු llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      void (*fun)(ffi_cif *, void *, void **, void*),
		      void *user_data,
		      void *codeloc)
{
  void *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;
  memcpy (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_SIZE);
  memcpy (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, parimaa_na (void *));
  memcpy (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, parimaa_na (void *));

  ආපසු FFI_OK;
}
