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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>
#समावेश <unistd.h>

#घोषणा CIF_FLAGS_CHAR		1	/* for struct only */
#घोषणा CIF_FLAGS_SHORT		2	/* for struct only */
#घोषणा CIF_FLAGS_INT		4
#घोषणा CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

व्योम ffi_call_elfbsd (extended_cif *, अचिन्हित, अचिन्हित, व्योम *,
		       व्योम (*) ());
व्योम *ffi_prep_args (extended_cif *ecif, व्योम *ढेर);

व्योम *
ffi_prep_args (extended_cif *ecif, व्योम *ढेर)
{
  अचिन्हित पूर्णांक i;
  व्योम **p_argv;
  अक्षर *argp;
  ffi_type **p_arg;
  व्योम *struct_मूल्य_ptr;

  argp = ढेर;

  अगर (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_मूल्य_ptr = ecif->rvalue;
  अन्यथा
    struct_मूल्य_ptr = शून्य;

  p_argv = ecif->avalue;

  क्रम (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->आकार;
      अगर (z < माप (पूर्णांक))
	{
	  चयन ((*p_arg)->type)
	    {
	    हाल FFI_TYPE_SINT8:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक) *(SINT8 *) *p_argv;
	      अवरोध;

	    हाल FFI_TYPE_UINT8:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक) *(UINT8 *) *p_argv;
	      अवरोध;

	    हाल FFI_TYPE_SINT16:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक) *(SINT16 *) *p_argv;
	      अवरोध;

	    हाल FFI_TYPE_UINT16:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक) *(UINT16 *) *p_argv;
	      अवरोध;

	    हाल FFI_TYPE_STRUCT:
	      स_नकल (argp, *p_argv, z);
	      अवरोध;

	    शेष:
	      FFI_ASSERT (0);
	    }
	  z = माप (पूर्णांक);
	}
      अन्यथा
	{
	  स_नकल (argp, *p_argv, z);

	  /* Align if necessary.  */
	  अगर ((माप(पूर्णांक) - 1) & z)
	    z = FFI_ALIGN(z, माप(पूर्णांक));
	}

      p_argv++;
      argp += z;
    }

  वापस struct_मूल्य_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_VOID:
      cif->flags = 0;
      अवरोध;

    हाल FFI_TYPE_STRUCT:
      अगर (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  अवरोध;
	}

      अगर (cif->rtype->आकार == माप (अक्षर))
	cif->flags = CIF_FLAGS_CHAR;
      अन्यथा अगर (cif->rtype->आकार == माप (लघु))
	cif->flags = CIF_FLAGS_SHORT;
      अन्यथा अगर (cif->rtype->आकार == माप (पूर्णांक))
	cif->flags = CIF_FLAGS_INT;
      अन्यथा अगर (cif->rtype->आकार == 2 * माप (पूर्णांक))
	cif->flags = CIF_FLAGS_DINT;
      अन्यथा
	cif->flags = 0;
      अवरोध;

    शेष:
      अगर (cif->rtype->आकार <= माप (पूर्णांक))
	cif->flags = CIF_FLAGS_INT;
      अन्यथा
	cif->flags = CIF_FLAGS_DINT;
      अवरोध;
    }

  वापस FFI_OK;
}

व्योम
ffi_call (ffi_cif *cif, व्योम (*fn) (), व्योम *rvalue, व्योम **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  अगर (rvalue == शून्य
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->आकार);
  अन्यथा
    ecif.rvalue = rvalue;

  चयन (cif->abi)
    {
    हाल FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      अवरोध;

    शेष:
      FFI_ASSERT (0);
      अवरोध;
    }
}

/*
 * Closure API
 */

व्योम ffi_closure_elfbsd (व्योम);
व्योम ffi_closure_struct_elfbsd (व्योम);
अचिन्हित पूर्णांक ffi_closure_elfbsd_inner (ffi_closure *, व्योम *, अक्षर *);

अटल व्योम
ffi_prep_closure_elfbsd (ffi_cif *cif, व्योम **avalue, अक्षर *stackp)
{
  अचिन्हित पूर्णांक i;
  व्योम **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  क्रम (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->आकार;
      *p_argv = stackp;

      /* Align if necessary */
      अगर ((माप (पूर्णांक) - 1) & z)
	z = FFI_ALIGN(z, माप (पूर्णांक));

      p_argv++;
      stackp += z;
    }
}

अचिन्हित पूर्णांक
ffi_closure_elfbsd_inner (ffi_closure *closure, व्योम *resp, अक्षर *ढेर)
{
  ffi_cif *cif;
  व्योम **arg_area;

  cif = closure->cif;
  arg_area = (व्योम **) alloca (cif->nargs * माप (व्योम *));

  ffi_prep_closure_elfbsd (cif, arg_area, ढेर);

  (closure->fun) (cif, resp, arg_area, closure->user_तथ्य);

  वापस cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      व्योम (*fun)(ffi_cif *, व्योम *, व्योम **, व्योम *),
		      व्योम *user_तथ्य, व्योम *codeloc)
{
  अक्षर *tramp = (अक्षर *) codeloc;
  व्योम *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(अचिन्हित लघु *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(अचिन्हित पूर्णांक *)(tramp + 4) = (अचिन्हित पूर्णांक) closure;
  tramp[8] = 0x50;

  अगर (cif->rtype->type == FFI_TYPE_STRUCT
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
  closure->user_तथ्य = user_तथ्य;
  closure->fun = fun;

  वापस FFI_OK;
}
