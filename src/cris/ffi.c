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

#परिभाषय राशिः_ARG_आकार(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

स्थिर ffi_स्थितिः
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != रिक्त);

  FFI_ASSERT (arg->elements != रिक्त);
  FFI_ASSERT (arg->आकार == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  यावत् ((*ptr) != रिक्त)
    {
      यदि (((*ptr)->आकार == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	प्रत्यागम FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->आकार += (*ptr)->आकार;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  यदि (arg->आकार == 0)
    प्रत्यागम FFI_BAD_TYPEDEF;
  अन्यथा
    प्रत्यागम FFI_OK;
}

पूर्णाङ्क
ffi_prep_तर्काः (वर्ण *राशिः, extended_cif * ecif)
{
  अचिह्नित पूर्णाङ्क i;
  अचिह्नित पूर्णाङ्क struct_गणनम् = 0;
  शून्य **p_argv;
  वर्ण *argp;
  ffi_type **p_arg;

  argp = राशिः;

  p_argv = ecif->avalue;

  प्रति (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      विकल्प ((*p_arg)->type)
	{
	पक्ष FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->आकार;
	    यदि (z <= 4)
	      {
		स्मृतिं_प्रतिलिख (argp, *p_argv, z);
		z = 4;
	      }
	    अन्यथा यदि (z <= 8)
	      {
		स्मृतिं_प्रतिलिख (argp, *p_argv, z);
		z = 8;
	      }
	    अन्यथा
	      {
		अचिह्नित पूर्णाङ्क ui_Loc_On_राशिः;
		z = परिमाण (शून्य *);
		ui_Loc_On_राशिः = 4 * ecif->cif->nargs + struct_गणनम्;
		struct_गणनम् = struct_गणनम् + (*p_arg)->आकार;
		*(अचिह्नित पूर्णाङ्क *) argp =
		  (अचिह्नित पूर्णाङ्क) (UINT32 *) (राशिः + ui_Loc_On_राशिः);
		स्मृतिं_प्रतिलिख ((राशिः + ui_Loc_On_राशिः), *p_argv, (*p_arg)->आकार);
	      }
	    भङ्ग;
	  }
	स्वभाव:
	  z = (*p_arg)->आकार;
	  यदि (z < परिमाण (पूर्णाङ्क))
	    {
	      विकल्प ((*p_arg)->type)
		{
		पक्ष FFI_TYPE_SINT8:
		  *(चिह्नित पूर्णाङ्क *) argp = (चिह्नित पूर्णाङ्क) *(SINT8 *) (*p_argv);
		  भङ्ग;

		पक्ष FFI_TYPE_UINT8:
		  *(अचिह्नित पूर्णाङ्क *) argp =
		    (अचिह्नित पूर्णाङ्क) *(UINT8 *) (*p_argv);
		  भङ्ग;

		पक्ष FFI_TYPE_SINT16:
		  *(चिह्नित पूर्णाङ्क *) argp = (चिह्नित पूर्णाङ्क) *(SINT16 *) (*p_argv);
		  भङ्ग;

		पक्ष FFI_TYPE_UINT16:
		  *(अचिह्नित पूर्णाङ्क *) argp =
		    (अचिह्नित पूर्णाङ्क) *(UINT16 *) (*p_argv);
		  भङ्ग;

		स्वभाव:
		  FFI_ASSERT (0);
		}
	      z = परिमाण (पूर्णाङ्क);
	    }
	  अन्यथा यदि (z == परिमाण (पूर्णाङ्क))
	    *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क) *(UINT32 *) (*p_argv);
	  अन्यथा
	    स्मृतिं_प्रतिलिख (argp, *p_argv, z);
	  भङ्ग;
	}
      p_argv++;
      argp += z;
    }

  प्रत्यागम (struct_गणनम्);
}

ffi_स्थितिः FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, अचिह्नित पूर्णाङ्क isvariadic,
		   अचिह्नित पूर्णाङ्क nfixedargs, अचिह्नित पूर्णाङ्क ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  अचिह्नित अष्टकानि = 0;
  अचिह्नित पूर्णाङ्क i;
  ffi_type **ptr;

  FFI_ASSERT (cif != रिक्त);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_चरमः_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  यदि ((cif->rtype->आकार == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    प्रत्यागम FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  प्रति (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      यदि (((*ptr)->आकार == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	प्रत्यागम FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      यदि (((*ptr)->alignment - 1) & अष्टकानि)
	अष्टकानि = FFI_ALIGN (अष्टकानि, (*ptr)->alignment);
      यदि ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  यदि ((*ptr)->आकार > 8)
	    {
	      अष्टकानि += (*ptr)->आकार;
	      अष्टकानि += परिमाण (शून्य *);
	    }
	  अन्यथा
	    {
	      यदि ((*ptr)->आकार > 4)
		अष्टकानि += 8;
	      अन्यथा
		अष्टकानि += 4;
	    }
	}
      अन्यथा
	अष्टकानि += राशिः_ARG_आकार ((*ptr)->आकार);
    }

  cif->अष्टकानि = अष्टकानि;

  प्रत्यागम ffi_prep_cif_machdep (cif);
}

ffi_स्थितिः
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
      cif->flags = (अचिह्नित) cif->rtype->type;
      भङ्ग;

    स्वभाव:
      cif->flags = FFI_TYPE_INT;
      भङ्ग;
    }

  प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_call_SYSV (पूर्णाङ्क (*)(वर्ण *, extended_cif *),
			   extended_cif *,
			   अचिह्नित, अचिह्नित, अचिह्नित *, शून्य (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

शून्य
ffi_call (ffi_cif * cif, शून्य (*fn) (), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  यदि ((rvalue == रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->आकार);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_SYSV:
      ffi_call_SYSV (ffi_prep_तर्काः, &ecif, cif->अष्टकानि,
		     cif->flags, ecif.rvalue, fn);
      भङ्ग;
    स्वभाव:
      FFI_ASSERT (0);
      भङ्ग;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
बाह्य ध्रुव वर्ण ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
बाह्य ध्रुव पूर्णाङ्क ffi_cris_trampoline_fn_विस्थापनम्
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
बाह्य ध्रुव पूर्णाङ्क ffi_cris_trampoline_संवृतिः_विस्थापनम्
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

स्थिर अचिह्नित दीर्घ दीर्घ
ffi_prep_संवृतिः_inner (शून्य **params, ffi_संवृतिः* संवृतिः)
{
  वर्ण *पञ्जिकाकोशः_तर्काः = (वर्ण *) params;
  शून्य *struct_ret = params[5];
  वर्ण *राशिः_तर्काः = params[6];
  वर्ण *ptr = पञ्जिकाकोशः_तर्काः;
  ffi_cif *cif = संवृतिः->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  शून्य **avalue = alloca (संवृतिः->cif->nargs * परिमाण(शून्य *));
  पूर्णाङ्क i;
  पूर्णाङ्क doing_regs;
  दीर्घ दीर्घ llret = 0;

  /* Find the address of each argument.  */
  प्रति (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      यदि (arg_types[i]->आकार <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      अन्यथा यदि (arg_types[i]->आकार <= 8)
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
      यदि (doing_regs && ptr >= पञ्जिकाकोशः_तर्काः + 4*4)
	{
	  ptr = राशिः_तर्काः + ((ptr > पञ्जिकाकोशः_तर्काः + 4*4) ? 4 : 0);
	  doing_regs = 0;
	}
    }

  /* Invoke the closure.  */
  (संवृतिः->fun) (cif,

		  cif->rtype->type == FFI_TYPE_STRUCT
		  /* The caller allocated space for the return
		     structure, and passed a pointer to this space in
		     R9.  */
		  ? struct_ret

		  /* We take advantage of being able to ignore that
		     the high part isn't set if the return value is
		     not in R10:R11, but in R10 only.  */
		  : (शून्य *) &llret,

		  avalue, संवृतिः->user_दत्त);

  प्रत्यागम llret;
}

/* API function: Prepare the trampoline.  */

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः* संवृतिः,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif *, शून्य *, शून्य **, शून्य*),
		      शून्य *user_दत्त,
		      शून्य *codeloc)
{
  शून्य *innerfn = ffi_prep_संवृतिः_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  संवृतिः->cif  = cif;
  संवृतिः->user_दत्त = user_दत्त;
  संवृतिः->fun  = fun;
  स्मृतिं_प्रतिलिख (संवृतिः->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_संहिता_PART_आकार);
  स्मृतिं_प्रतिलिख (संवृतिः->tramp + ffi_cris_trampoline_fn_विस्थापनम्,
	  &innerfn, परिमाण (शून्य *));
  स्मृतिं_प्रतिलिख (संवृतिः->tramp + ffi_cris_trampoline_संवृतिः_विस्थापनम्,
	  &codeloc, परिमाण (शून्य *));

  प्रत्यागम FFI_OK;
}
