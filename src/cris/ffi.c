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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#define थाक_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

स्थिर ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != रिक्त);

  FFI_ASSERT (arg->elements != रिक्त);
  FFI_ASSERT (arg->size == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  जबसम्म ((*ptr) != रिक्त)
    {
      यदि (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	फर्काऊ FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->size += (*ptr)->size;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  यदि (arg->size == 0)
    फर्काऊ FFI_BAD_TYPEDEF;
  अन्यथा
    फर्काऊ FFI_OK;
}

पूर्णाङ्क
ffi_prep_args (वर्ण *थाक, extended_cif * ecif)
{
  अचिन्हित पूर्णाङ्क i;
  अचिन्हित पूर्णाङ्क struct_गन्ती = 0;
  शून्य **p_argv;
  वर्ण *argp;
  ffi_type **p_arg;

  argp = थाक;

  p_argv = ecif->avalue;

  लागि (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      माप_प्रकार z;

      विकल्प ((*p_arg)->type)
	{
	पक्ष FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->size;
	    यदि (z <= 4)
	      {
		स_नकल (argp, *p_argv, z);
		z = 4;
	      }
	    अन्यथा यदि (z <= 8)
	      {
		स_नकल (argp, *p_argv, z);
		z = 8;
	      }
	    अन्यथा
	      {
		अचिन्हित पूर्णाङ्क ui_Loc_On_थाक;
		z = परिमाण (शून्य *);
		ui_Loc_On_थाक = 4 * ecif->cif->nargs + struct_गन्ती;
		struct_गन्ती = struct_गन्ती + (*p_arg)->size;
		*(अचिन्हित पूर्णाङ्क *) argp =
		  (अचिन्हित पूर्णाङ्क) (UINT32 *) (थाक + ui_Loc_On_थाक);
		स_नकल ((थाक + ui_Loc_On_थाक), *p_argv, (*p_arg)->size);
	      }
	    रोक;
	  }
	स्वभाव:
	  z = (*p_arg)->size;
	  यदि (z < परिमाण (पूर्णाङ्क))
	    {
	      विकल्प ((*p_arg)->type)
		{
		पक्ष FFI_TYPE_SINT8:
		  *(चिन्हित पूर्णाङ्क *) argp = (चिन्हित पूर्णाङ्क) *(SINT8 *) (*p_argv);
		  रोक;

		पक्ष FFI_TYPE_UINT8:
		  *(अचिन्हित पूर्णाङ्क *) argp =
		    (अचिन्हित पूर्णाङ्क) *(UINT8 *) (*p_argv);
		  रोक;

		पक्ष FFI_TYPE_SINT16:
		  *(चिन्हित पूर्णाङ्क *) argp = (चिन्हित पूर्णाङ्क) *(SINT16 *) (*p_argv);
		  रोक;

		पक्ष FFI_TYPE_UINT16:
		  *(अचिन्हित पूर्णाङ्क *) argp =
		    (अचिन्हित पूर्णाङ्क) *(UINT16 *) (*p_argv);
		  रोक;

		स्वभाव:
		  FFI_ASSERT (0);
		}
	      z = परिमाण (पूर्णाङ्क);
	    }
	  अन्यथा यदि (z == परिमाण (पूर्णाङ्क))
	    *(अचिन्हित पूर्णाङ्क *) argp = (अचिन्हित पूर्णाङ्क) *(UINT32 *) (*p_argv);
	  अन्यथा
	    स_नकल (argp, *p_argv, z);
	  रोक;
	}
      p_argv++;
      argp += z;
    }

  फर्काऊ (struct_गन्ती);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, अचिन्हित पूर्णाङ्क isvariadic,
		   अचिन्हित पूर्णाङ्क nfixedargs, अचिन्हित पूर्णाङ्क ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  अचिन्हित bytes = 0;
  अचिन्हित पूर्णाङ्क i;
  ffi_type **ptr;

  FFI_ASSERT (cif != रिक्त);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  यदि ((cif->rtype->size == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    फर्काऊ FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  लागि (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      यदि (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	फर्काऊ FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      यदि (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      यदि ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  यदि ((*ptr)->size > 8)
	    {
	      bytes += (*ptr)->size;
	      bytes += परिमाण (शून्य *);
	    }
	  अन्यथा
	    {
	      यदि ((*ptr)->size > 4)
		bytes += 8;
	      अन्यथा
		bytes += 4;
	    }
	}
      अन्यथा
	bytes += थाक_ARG_SIZE ((*ptr)->size);
    }

  cif->bytes = bytes;

  फर्काऊ ffi_prep_cif_machdep (cif);
}

ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
    पक्ष FFI_TYPE_STRUCT:
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
      cif->flags = (अचिन्हित) cif->rtype->type;
      रोक;

    स्वभाव:
      cif->flags = FFI_TYPE_INT;
      रोक;
    }

  फर्काऊ FFI_OK;
}

बाह्य शून्य ffi_call_SYSV (पूर्णाङ्क (*)(वर्ण *, extended_cif *),
			   extended_cif *,
			   अचिन्हित, अचिन्हित, अचिन्हित *, शून्य (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

शून्य
ffi_call (ffi_cif * cif, शून्य (*fn) (), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  यदि ((rvalue == रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      रोक;
    स्वभाव:
      FFI_ASSERT (0);
      रोक;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
बाह्य अचर वर्ण ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
बाह्य अचर पूर्णाङ्क ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
बाह्य अचर पूर्णाङ्क ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

स्थिर अचिन्हित दीर्घ दीर्घ
ffi_prep_closure_inner (शून्य **params, ffi_closure* closure)
{
  वर्ण *register_args = (वर्ण *) params;
  शून्य *struct_ret = params[5];
  वर्ण *थाक_args = params[6];
  वर्ण *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  शून्य **avalue = alloca (closure->cif->nargs * परिमाण(शून्य *));
  पूर्णाङ्क i;
  पूर्णाङ्क doing_regs;
  दीर्घ दीर्घ llret = 0;

  /* Find the address of each argument.  */
  लागि (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      यदि (arg_types[i]->size <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      अन्यथा यदि (arg_types[i]->size <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      अन्यथा
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(शून्य **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      यदि (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = थाक_args + ((ptr > register_args + 4*4) ? 4 : 0);
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
		  : (शून्य *) &llret,

		  avalue, closure->user_data);

  फर्काऊ llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif *, शून्य *, शून्य **, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  शून्य *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;
  स_नकल (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_SIZE);
  स_नकल (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, परिमाण (शून्य *));
  स_नकल (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, परिमाण (शून्य *));

  फर्काऊ FFI_OK;
}
