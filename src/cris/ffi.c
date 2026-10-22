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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#घोषणा ढेर_ARG_आकार(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

अटल ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != शून्य);

  FFI_ASSERT (arg->elements != शून्य);
  FFI_ASSERT (arg->आकार == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  जबतक ((*ptr) != शून्य)
    {
      अगर (((*ptr)->आकार == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	वापस FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->आकार += (*ptr)->आकार;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  अगर (arg->आकार == 0)
    वापस FFI_BAD_TYPEDEF;
  अन्यथा
    वापस FFI_OK;
}

पूर्णांक
ffi_prep_args (अक्षर *ढेर, extended_cif * ecif)
{
  अचिन्हित पूर्णांक i;
  अचिन्हित पूर्णांक struct_गिनती = 0;
  व्योम **p_argv;
  अक्षर *argp;
  ffi_type **p_arg;

  argp = ढेर;

  p_argv = ecif->avalue;

  क्रम (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      माप_प्रकार z;

      चयन ((*p_arg)->type)
	{
	हाल FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->आकार;
	    अगर (z <= 4)
	      {
		स_नकल (argp, *p_argv, z);
		z = 4;
	      }
	    अन्यथा अगर (z <= 8)
	      {
		स_नकल (argp, *p_argv, z);
		z = 8;
	      }
	    अन्यथा
	      {
		अचिन्हित पूर्णांक ui_Loc_On_ढेर;
		z = माप (व्योम *);
		ui_Loc_On_ढेर = 4 * ecif->cif->nargs + struct_गिनती;
		struct_गिनती = struct_गिनती + (*p_arg)->आकार;
		*(अचिन्हित पूर्णांक *) argp =
		  (अचिन्हित पूर्णांक) (UINT32 *) (ढेर + ui_Loc_On_ढेर);
		स_नकल ((ढेर + ui_Loc_On_ढेर), *p_argv, (*p_arg)->आकार);
	      }
	    अवरोध;
	  }
	शेष:
	  z = (*p_arg)->आकार;
	  अगर (z < माप (पूर्णांक))
	    {
	      चयन ((*p_arg)->type)
		{
		हाल FFI_TYPE_SINT8:
		  *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक) *(SINT8 *) (*p_argv);
		  अवरोध;

		हाल FFI_TYPE_UINT8:
		  *(अचिन्हित पूर्णांक *) argp =
		    (अचिन्हित पूर्णांक) *(UINT8 *) (*p_argv);
		  अवरोध;

		हाल FFI_TYPE_SINT16:
		  *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक) *(SINT16 *) (*p_argv);
		  अवरोध;

		हाल FFI_TYPE_UINT16:
		  *(अचिन्हित पूर्णांक *) argp =
		    (अचिन्हित पूर्णांक) *(UINT16 *) (*p_argv);
		  अवरोध;

		शेष:
		  FFI_ASSERT (0);
		}
	      z = माप (पूर्णांक);
	    }
	  अन्यथा अगर (z == माप (पूर्णांक))
	    *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक) *(UINT32 *) (*p_argv);
	  अन्यथा
	    स_नकल (argp, *p_argv, z);
	  अवरोध;
	}
      p_argv++;
      argp += z;
    }

  वापस (struct_गिनती);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, अचिन्हित पूर्णांक isvariadic,
		   अचिन्हित पूर्णांक nfixedargs, अचिन्हित पूर्णांक ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  अचिन्हित bytes = 0;
  अचिन्हित पूर्णांक i;
  ffi_type **ptr;

  FFI_ASSERT (cif != शून्य);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  अगर ((cif->rtype->आकार == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    वापस FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  क्रम (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      अगर (((*ptr)->आकार == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	वापस FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      अगर (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      अगर ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  अगर ((*ptr)->आकार > 8)
	    {
	      bytes += (*ptr)->आकार;
	      bytes += माप (व्योम *);
	    }
	  अन्यथा
	    {
	      अगर ((*ptr)->आकार > 4)
		bytes += 8;
	      अन्यथा
		bytes += 4;
	    }
	}
      अन्यथा
	bytes += ढेर_ARG_आकार ((*ptr)->आकार);
    }

  cif->bytes = bytes;

  वापस ffi_prep_cif_machdep (cif);
}

ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_VOID:
    हाल FFI_TYPE_STRUCT:
    हाल FFI_TYPE_FLOAT:
    हाल FFI_TYPE_DOUBLE:
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
      cif->flags = (अचिन्हित) cif->rtype->type;
      अवरोध;

    शेष:
      cif->flags = FFI_TYPE_INT;
      अवरोध;
    }

  वापस FFI_OK;
}

बाह्य व्योम ffi_call_SYSV (पूर्णांक (*)(अक्षर *, extended_cif *),
			   extended_cif *,
			   अचिन्हित, अचिन्हित, अचिन्हित *, व्योम (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

व्योम
ffi_call (ffi_cif * cif, व्योम (*fn) (), व्योम *rvalue, व्योम **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  अगर ((rvalue == शून्य) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->आकार);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  चयन (cif->abi)
    {
    हाल FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      अवरोध;
    शेष:
      FFI_ASSERT (0);
      अवरोध;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
बाह्य स्थिर अक्षर ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
बाह्य स्थिर पूर्णांक ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
बाह्य स्थिर पूर्णांक ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

अटल अचिन्हित दीर्घ दीर्घ
ffi_prep_closure_inner (व्योम **params, ffi_closure* closure)
{
  अक्षर *register_args = (अक्षर *) params;
  व्योम *struct_ret = params[5];
  अक्षर *ढेर_args = params[6];
  अक्षर *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  व्योम **avalue = alloca (closure->cif->nargs * माप(व्योम *));
  पूर्णांक i;
  पूर्णांक doing_regs;
  दीर्घ दीर्घ llret = 0;

  /* Find the address of each argument.  */
  क्रम (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      अगर (arg_types[i]->आकार <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      अन्यथा अगर (arg_types[i]->आकार <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      अन्यथा
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(व्योम **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      अगर (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = ढेर_args + ((ptr > register_args + 4*4) ? 4 : 0);
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
		  : (व्योम *) &llret,

		  avalue, closure->user_तथ्य);

  वापस llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      व्योम (*fun)(ffi_cif *, व्योम *, व्योम **, व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  व्योम *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_तथ्य = user_तथ्य;
  closure->fun  = fun;
  स_नकल (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_आकार);
  स_नकल (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, माप (व्योम *));
  स_नकल (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, माप (व्योम *));

  वापस FFI_OK;
}
