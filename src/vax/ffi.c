/*
 * Copyright (c) 2013 Miodrag Vallat.  <miod@openbsd.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * vax Foreign Function Interface
 *
 * This file attempts to provide all the FFI entry points which can reliably
 * be implemented in C.
 */

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>

#define CIF_FLAGS_CHAR		1	/* for struct only */
#define CIF_FLAGS_SHORT		2	/* for struct only */
#define CIF_FLAGS_INT		4
#define CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

शून्य ffi_call_elfbsd (extended_cif *, अचिन्हित, अचिन्हित, शून्य *,
		       शून्य (*) ());
शून्य *ffi_prep_args (extended_cif *ecif, शून्य *चळत);

शून्य *
ffi_prep_args (extended_cif *ecif, शून्य *चळत)
{
  अचिन्हित पूर्णांक i;
  शून्य **p_argv;
  अक्षर *argp;
  ffi_type **p_arg;
  शून्य *struct_मूल्य_ptr;

  argp = चळत;

  जर (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_मूल्य_ptr = ecif->rvalue;
  अन्यथा
    struct_मूल्य_ptr = रिक्त;

  p_argv = ecif->avalue;

  साठी (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->size;
      जर (z < आकार (पूर्णांक))
	{
	  निवड ((*p_arg)->type)
	    {
	    स्थिती FFI_TYPE_SINT8:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक) *(SINT8 *) *p_argv;
	      थांबा;

	    स्थिती FFI_TYPE_UINT8:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक) *(UINT8 *) *p_argv;
	      थांबा;

	    स्थिती FFI_TYPE_SINT16:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक) *(SINT16 *) *p_argv;
	      थांबा;

	    स्थिती FFI_TYPE_UINT16:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक) *(UINT16 *) *p_argv;
	      थांबा;

	    स्थिती FFI_TYPE_STRUCT:
	      स_नकल (argp, *p_argv, z);
	      थांबा;

	    पूर्वनिर्धारित:
	      FFI_ASSERT (0);
	    }
	  z = आकार (पूर्णांक);
	}
      अन्यथा
	{
	  स_नकल (argp, *p_argv, z);

	  /* Align if necessary.  */
	  जर ((आकार(पूर्णांक) - 1) & z)
	    z = FFI_ALIGN(z, आकार(पूर्णांक));
	}

      p_argv++;
      argp += z;
    }

  परत struct_मूल्य_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
      cif->flags = 0;
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
      जर (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  थांबा;
	}

      जर (cif->rtype->size == आकार (अक्षर))
	cif->flags = CIF_FLAGS_CHAR;
      अन्यथा जर (cif->rtype->size == आकार (लघु))
	cif->flags = CIF_FLAGS_SHORT;
      अन्यथा जर (cif->rtype->size == आकार (पूर्णांक))
	cif->flags = CIF_FLAGS_INT;
      अन्यथा जर (cif->rtype->size == 2 * आकार (पूर्णांक))
	cif->flags = CIF_FLAGS_DINT;
      अन्यथा
	cif->flags = 0;
      थांबा;

    पूर्वनिर्धारित:
      जर (cif->rtype->size <= आकार (पूर्णांक))
	cif->flags = CIF_FLAGS_INT;
      अन्यथा
	cif->flags = CIF_FLAGS_DINT;
      थांबा;
    }

  परत FFI_OK;
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  जर (rvalue == रिक्त
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->size);
  अन्यथा
    ecif.rvalue = rvalue;

  निवड (cif->abi)
    {
    स्थिती FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      थांबा;

    पूर्वनिर्धारित:
      FFI_ASSERT (0);
      थांबा;
    }
}

/*
 * Closure API
 */

शून्य ffi_closure_elfbsd (शून्य);
शून्य ffi_closure_struct_elfbsd (शून्य);
अचिन्हित पूर्णांक ffi_closure_elfbsd_inner (ffi_closure *, शून्य *, अक्षर *);

स्थिर शून्य
ffi_prep_closure_elfbsd (ffi_cif *cif, शून्य **avalue, अक्षर *stackp)
{
  अचिन्हित पूर्णांक i;
  शून्य **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  साठी (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->size;
      *p_argv = stackp;

      /* Align if necessary */
      जर ((आकार (पूर्णांक) - 1) & z)
	z = FFI_ALIGN(z, आकार (पूर्णांक));

      p_argv++;
      stackp += z;
    }
}

अचिन्हित पूर्णांक
ffi_closure_elfbsd_inner (ffi_closure *closure, शून्य *resp, अक्षर *चळत)
{
  ffi_cif *cif;
  शून्य **arg_area;

  cif = closure->cif;
  arg_area = (शून्य **) alloca (cif->nargs * आकार (शून्य *));

  ffi_prep_closure_elfbsd (cif, arg_area, चळत);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  परत cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      शून्य (*fun)(ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_data, शून्य *codeloc)
{
  अक्षर *tramp = (अक्षर *) codeloc;
  शून्य *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(अचिन्हित लघु *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(अचिन्हित पूर्णांक *)(tramp + 4) = (अचिन्हित पूर्णांक) closure;
  tramp[8] = 0x50;

  जर (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  अन्यथा
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(अचिन्हित पूर्णांक *)(tramp + 11) = (अचिन्हित पूर्णांक)fn + 2 -
				  (अचिन्हित पूर्णांक)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  परत FFI_OK;
}
