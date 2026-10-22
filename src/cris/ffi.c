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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#define അട്ടി_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

സ്ഥിതം ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != ശൂന്യ);

  FFI_ASSERT (arg->elements != ശൂന്യ);
  FFI_ASSERT (arg->size == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  ആകുമ്പോൾ ((*ptr) != ശൂന്യ)
    {
      എങ്കിൽ (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	തിരികെ FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->size += (*ptr)->size;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  എങ്കിൽ (arg->size == 0)
    തിരികെ FFI_BAD_TYPEDEF;
  അല്ലെങ്കിൽ
    തിരികെ FFI_OK;
}

പൂർണ്ണസംഖ്യ
ffi_prep_args (അക്ഷരം *അട്ടി, extended_cif * ecif)
{
  unsigned പൂർണ്ണസംഖ്യ i;
  unsigned പൂർണ്ണസംഖ്യ struct_എണ്ണം = 0;
  ശൂന്യം **p_argv;
  അക്ഷരം *argp;
  ffi_type **p_arg;

  argp = അട്ടി;

  p_argv = ecif->avalue;

  വേണ്ടി (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
	{
	അവസ്ഥ FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->size;
	    എങ്കിൽ (z <= 4)
	      {
		memcpy (argp, *p_argv, z);
		z = 4;
	      }
	    അല്ലെങ്കിൽ എങ്കിൽ (z <= 8)
	      {
		memcpy (argp, *p_argv, z);
		z = 8;
	      }
	    അല്ലെങ്കിൽ
	      {
		unsigned പൂർണ്ണസംഖ്യ ui_Loc_On_അട്ടി;
		z = വലുപ്പം (ശൂന്യം *);
		ui_Loc_On_അട്ടി = 4 * ecif->cif->nargs + struct_എണ്ണം;
		struct_എണ്ണം = struct_എണ്ണം + (*p_arg)->size;
		*(unsigned പൂർണ്ണസംഖ്യ *) argp =
		  (unsigned പൂർണ്ണസംഖ്യ) (UINT32 *) (അട്ടി + ui_Loc_On_അട്ടി);
		memcpy ((അട്ടി + ui_Loc_On_അട്ടി), *p_argv, (*p_arg)->size);
	      }
	    നിർത്തുക;
	  }
	സ്ഥിരസ്ഥിതി:
	  z = (*p_arg)->size;
	  എങ്കിൽ (z < വലുപ്പം (പൂർണ്ണസംഖ്യ))
	    {
	      തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
		{
		അവസ്ഥ FFI_TYPE_SINT8:
		  *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ) *(SINT8 *) (*p_argv);
		  നിർത്തുക;

		അവസ്ഥ FFI_TYPE_UINT8:
		  *(unsigned പൂർണ്ണസംഖ്യ *) argp =
		    (unsigned പൂർണ്ണസംഖ്യ) *(UINT8 *) (*p_argv);
		  നിർത്തുക;

		അവസ്ഥ FFI_TYPE_SINT16:
		  *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ) *(SINT16 *) (*p_argv);
		  നിർത്തുക;

		അവസ്ഥ FFI_TYPE_UINT16:
		  *(unsigned പൂർണ്ണസംഖ്യ *) argp =
		    (unsigned പൂർണ്ണസംഖ്യ) *(UINT16 *) (*p_argv);
		  നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
		  FFI_ASSERT (0);
		}
	      z = വലുപ്പം (പൂർണ്ണസംഖ്യ);
	    }
	  അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം (പൂർണ്ണസംഖ്യ))
	    *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ) *(UINT32 *) (*p_argv);
	  അല്ലെങ്കിൽ
	    memcpy (argp, *p_argv, z);
	  നിർത്തുക;
	}
      p_argv++;
      argp += z;
    }

  തിരികെ (struct_എണ്ണം);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, unsigned പൂർണ്ണസംഖ്യ isvariadic,
		   unsigned പൂർണ്ണസംഖ്യ nfixedargs, unsigned പൂർണ്ണസംഖ്യ ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  unsigned bytes = 0;
  unsigned പൂർണ്ണസംഖ്യ i;
  ffi_type **ptr;

  FFI_ASSERT (cif != ശൂന്യ);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  എങ്കിൽ ((cif->rtype->size == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    തിരികെ FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  വേണ്ടി (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      എങ്കിൽ (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	തിരികെ FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      എങ്കിൽ (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      എങ്കിൽ ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  എങ്കിൽ ((*ptr)->size > 8)
	    {
	      bytes += (*ptr)->size;
	      bytes += വലുപ്പം (ശൂന്യം *);
	    }
	  അല്ലെങ്കിൽ
	    {
	      എങ്കിൽ ((*ptr)->size > 4)
		bytes += 8;
	      അല്ലെങ്കിൽ
		bytes += 4;
	    }
	}
      അല്ലെങ്കിൽ
	bytes += അട്ടി_ARG_SIZE ((*ptr)->size);
    }

  cif->bytes = bytes;

  തിരികെ ffi_prep_cif_machdep (cif);
}

ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
    അവസ്ഥ FFI_TYPE_STRUCT:
    അവസ്ഥ FFI_TYPE_FLOAT:
    അവസ്ഥ FFI_TYPE_DOUBLE:
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
      cif->flags = (unsigned) cif->rtype->type;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      cif->flags = FFI_TYPE_INT;
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

ബാഹ്യ ശൂന്യം ffi_call_SYSV (പൂർണ്ണസംഖ്യ (*)(അക്ഷരം *, extended_cif *),
			   extended_cif *,
			   unsigned, unsigned, unsigned *, ശൂന്യം (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

ശൂന്യം
ffi_call (ffi_cif * cif, ശൂന്യം (*fn) (), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  എങ്കിൽ ((rvalue == ശൂന്യ) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT (0);
      നിർത്തുക;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
ബാഹ്യ സ്ഥിരം അക്ഷരം ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
ബാഹ്യ സ്ഥിരം പൂർണ്ണസംഖ്യ ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
ബാഹ്യ സ്ഥിരം പൂർണ്ണസംഖ്യ ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

സ്ഥിതം unsigned long long
ffi_prep_closure_inner (ശൂന്യം **params, ffi_closure* closure)
{
  അക്ഷരം *register_args = (അക്ഷരം *) params;
  ശൂന്യം *struct_ret = params[5];
  അക്ഷരം *അട്ടി_args = params[6];
  അക്ഷരം *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  ശൂന്യം **avalue = alloca (closure->cif->nargs * വലുപ്പം(ശൂന്യം *));
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ doing_regs;
  long long llret = 0;

  /* Find the address of each argument.  */
  വേണ്ടി (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      എങ്കിൽ (arg_types[i]->size <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      അല്ലെങ്കിൽ എങ്കിൽ (arg_types[i]->size <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      അല്ലെങ്കിൽ
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(ശൂന്യം **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      എങ്കിൽ (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = അട്ടി_args + ((ptr > register_args + 4*4) ? 4 : 0);
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
		  : (ശൂന്യം *) &llret,

		  avalue, closure->user_data);

  തിരികെ llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ശൂന്യം (*fun)(ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  ശൂന്യം *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;
  memcpy (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_SIZE);
  memcpy (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, വലുപ്പം (ശൂന്യം *));
  memcpy (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, വലുപ്പം (ശൂന്യം *));

  തിരികെ FFI_OK;
}
