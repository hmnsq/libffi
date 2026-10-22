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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#define ઢગલો_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

સ્થિર ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != ખાલી);

  FFI_ASSERT (arg->elements != ખાલી);
  FFI_ASSERT (arg->size == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  જ્યારે ((*ptr) != ખાલી)
    {
      જો (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	પાછા FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->size += (*ptr)->size;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  જો (arg->size == 0)
    પાછા FFI_BAD_TYPEDEF;
  નહીંતર
    પાછા FFI_OK;
}

પૂર્ણાંક
ffi_prep_args (અક્ષર *ઢગલો, extended_cif * ecif)
{
  unsigned પૂર્ણાંક i;
  unsigned પૂર્ણાંક struct_ગણતરી = 0;
  શૂન્ય **p_argv;
  અક્ષર *argp;
  ffi_type **p_arg;

  argp = ઢગલો;

  p_argv = ecif->avalue;

  માટે (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      પસંદગી ((*p_arg)->type)
	{
	સ્થિતિ FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->size;
	    જો (z <= 4)
	      {
		memcpy (argp, *p_argv, z);
		z = 4;
	      }
	    નહીંતર જો (z <= 8)
	      {
		memcpy (argp, *p_argv, z);
		z = 8;
	      }
	    નહીંતર
	      {
		unsigned પૂર્ણાંક ui_Loc_On_ઢગલો;
		z = કદ (શૂન્ય *);
		ui_Loc_On_ઢગલો = 4 * ecif->cif->nargs + struct_ગણતરી;
		struct_ગણતરી = struct_ગણતરી + (*p_arg)->size;
		*(unsigned પૂર્ણાંક *) argp =
		  (unsigned પૂર્ણાંક) (UINT32 *) (ઢગલો + ui_Loc_On_ઢગલો);
		memcpy ((ઢગલો + ui_Loc_On_ઢગલો), *p_argv, (*p_arg)->size);
	      }
	    તોડો;
	  }
	મૂળભૂત:
	  z = (*p_arg)->size;
	  જો (z < કદ (પૂર્ણાંક))
	    {
	      પસંદગી ((*p_arg)->type)
		{
		સ્થિતિ FFI_TYPE_SINT8:
		  *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક) *(SINT8 *) (*p_argv);
		  તોડો;

		સ્થિતિ FFI_TYPE_UINT8:
		  *(unsigned પૂર્ણાંક *) argp =
		    (unsigned પૂર્ણાંક) *(UINT8 *) (*p_argv);
		  તોડો;

		સ્થિતિ FFI_TYPE_SINT16:
		  *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક) *(SINT16 *) (*p_argv);
		  તોડો;

		સ્થિતિ FFI_TYPE_UINT16:
		  *(unsigned પૂર્ણાંક *) argp =
		    (unsigned પૂર્ણાંક) *(UINT16 *) (*p_argv);
		  તોડો;

		મૂળભૂત:
		  FFI_ASSERT (0);
		}
	      z = કદ (પૂર્ણાંક);
	    }
	  નહીંતર જો (z == કદ (પૂર્ણાંક))
	    *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક) *(UINT32 *) (*p_argv);
	  નહીંતર
	    memcpy (argp, *p_argv, z);
	  તોડો;
	}
      p_argv++;
      argp += z;
    }

  પાછા (struct_ગણતરી);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, unsigned પૂર્ણાંક isvariadic,
		   unsigned પૂર્ણાંક nfixedargs, unsigned પૂર્ણાંક ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  unsigned bytes = 0;
  unsigned પૂર્ણાંક i;
  ffi_type **ptr;

  FFI_ASSERT (cif != ખાલી);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  જો ((cif->rtype->size == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    પાછા FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  માટે (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      જો (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	પાછા FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      જો (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      જો ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  જો ((*ptr)->size > 8)
	    {
	      bytes += (*ptr)->size;
	      bytes += કદ (શૂન્ય *);
	    }
	  નહીંતર
	    {
	      જો ((*ptr)->size > 4)
		bytes += 8;
	      નહીંતર
		bytes += 4;
	    }
	}
      નહીંતર
	bytes += ઢગલો_ARG_SIZE ((*ptr)->size);
    }

  cif->bytes = bytes;

  પાછા ffi_prep_cif_machdep (cif);
}

ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
    સ્થિતિ FFI_TYPE_STRUCT:
    સ્થિતિ FFI_TYPE_FLOAT:
    સ્થિતિ FFI_TYPE_DOUBLE:
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
      cif->flags = (unsigned) cif->rtype->type;
      તોડો;

    મૂળભૂત:
      cif->flags = FFI_TYPE_INT;
      તોડો;
    }

  પાછા FFI_OK;
}

બાહ્ય શૂન્ય ffi_call_SYSV (પૂર્ણાંક (*)(અક્ષર *, extended_cif *),
			   extended_cif *,
			   unsigned, unsigned, unsigned *, શૂન્ય (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

શૂન્ય
ffi_call (ffi_cif * cif, શૂન્ય (*fn) (), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  જો ((rvalue == ખાલી) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  નહીંતર
    ecif.rvalue = rvalue;

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      તોડો;
    મૂળભૂત:
      FFI_ASSERT (0);
      તોડો;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
બાહ્ય અચલ અક્ષર ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
બાહ્ય અચલ પૂર્ણાંક ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
બાહ્ય અચલ પૂર્ણાંક ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

સ્થિર unsigned long long
ffi_prep_closure_inner (શૂન્ય **params, ffi_closure* closure)
{
  અક્ષર *register_args = (અક્ષર *) params;
  શૂન્ય *struct_ret = params[5];
  અક્ષર *ઢગલો_args = params[6];
  અક્ષર *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  શૂન્ય **avalue = alloca (closure->cif->nargs * કદ(શૂન્ય *));
  પૂર્ણાંક i;
  પૂર્ણાંક doing_regs;
  long long llret = 0;

  /* Find the address of each argument.  */
  માટે (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      જો (arg_types[i]->size <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      નહીંતર જો (arg_types[i]->size <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      નહીંતર
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(શૂન્ય **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      જો (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = ઢગલો_args + ((ptr > register_args + 4*4) ? 4 : 0);
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
		  : (શૂન્ય *) &llret,

		  avalue, closure->user_data);

  પાછા llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  શૂન્ય *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;
  memcpy (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_SIZE);
  memcpy (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, કદ (શૂન્ય *));
  memcpy (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, કદ (શૂન્ય *));

  પાછા FFI_OK;
}
