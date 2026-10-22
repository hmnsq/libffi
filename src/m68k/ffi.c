/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>
#समावेश <unistd.h>
#अगर_घोषित __rtems__
व्योम rtems_cache_flush_multiple_तथ्य_पंक्तियाँ( स्थिर व्योम *, माप_प्रकार );
#अन्यथा
#समावेश <sys/syscall.h>
#अगर_घोषित __MINT__
#समावेश <mint/mintbind.h>
#समावेश <mint/ssystem.h>
#अन्यथा
#समावेश <asm/cachectl.h>
#पूर्ण_अगर
#पूर्ण_अगर

व्योम ffi_call_SYSV (extended_cif *,
		    अचिन्हित, अचिन्हित,
		    व्योम *, व्योम (*fn) ());
व्योम *ffi_prep_args (व्योम *ढेर, extended_cif *ecif);
व्योम ffi_closure_SYSV (ffi_closure *);
व्योम ffi_closure_struct_SYSV (ffi_closure *);
अचिन्हित पूर्णांक ffi_closure_SYSV_inner (ffi_closure *closure,
				     व्योम *resp, व्योम *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

व्योम *
ffi_prep_args (व्योम *ढेर, extended_cif *ecif)
{
  अचिन्हित पूर्णांक i;
  व्योम **p_argv;
  अक्षर *argp;
  ffi_type **p_arg;
  व्योम *struct_मूल्य_ptr;

  argp = ढेर;

  अगर (
#अगर_घोषित __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#पूर्ण_अगर
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_मूल्य_ptr = ecif->rvalue;
  अन्यथा
    struct_मूल्य_ptr = शून्य;

  p_argv = ecif->avalue;

  क्रम (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      माप_प्रकार z = (*p_arg)->आकार;
      पूर्णांक type = (*p_arg)->type;

      अगर (z < माप (पूर्णांक))
	{
	  चयन (type)
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
#अगर_घोषित __MINT__
	      अगर (z == 1 || z == 2)
		स_नकल (argp + 2, *p_argv, z);
              अन्यथा
		स_नकल (argp, *p_argv, z);
#अन्यथा
	      स_नकल (argp + माप (पूर्णांक) - z, *p_argv, z);
#पूर्ण_अगर
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

#घोषणा CIF_FLAGS_INT		1
#घोषणा CIF_FLAGS_DINT		2
#घोषणा CIF_FLAGS_FLOAT		4
#घोषणा CIF_FLAGS_DOUBLE	8
#घोषणा CIF_FLAGS_LDOUBLE	16
#घोषणा CIF_FLAGS_POINTER	32
#घोषणा CIF_FLAGS_STRUCT1	64
#घोषणा CIF_FLAGS_STRUCT2	128
#घोषणा CIF_FLAGS_SINT8		256
#घोषणा CIF_FLAGS_SINT16	512

/* Perform machine dependent cif processing */
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

      चयन (cif->rtype->आकार)
	{
	हाल 1:
#अगर_घोषित __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#अन्यथा
	  cif->flags = CIF_FLAGS_STRUCT1;
#पूर्ण_अगर
	  अवरोध;
	हाल 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  अवरोध;
#अगर_घोषित __MINT__
	हाल 3:
#पूर्ण_अगर
	हाल 4:
	  cif->flags = CIF_FLAGS_INT;
	  अवरोध;
#अगर_घोषित __MINT__
	हाल 7:
#पूर्ण_अगर
	हाल 8:
	  cif->flags = CIF_FLAGS_DINT;
	  अवरोध;
	शेष:
	  cif->flags = 0;
	  अवरोध;
	}
      अवरोध;

    हाल FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      अवरोध;

    हाल FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      अवरोध;

#अगर (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    हाल FFI_TYPE_LONGDOUBLE:
#अगर_घोषित __MINT__
      cif->flags = 0;
#अन्यथा
      cif->flags = CIF_FLAGS_LDOUBLE;
#पूर्ण_अगर
      अवरोध;
#पूर्ण_अगर

    हाल FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      अवरोध;

    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      अवरोध;

    हाल FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      अवरोध;

    हाल FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      अवरोध;

    शेष:
      cif->flags = CIF_FLAGS_INT;
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
      && cif->rtype->आकार > 8)
    ecif.rvalue = alloca (cif->rtype->आकार);
  अन्यथा
    ecif.rvalue = rvalue;

  चयन (cif->abi)
    {
    हाल FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      अवरोध;

    शेष:
      FFI_ASSERT (0);
      अवरोध;
    }
}

अटल व्योम
ffi_prep_incoming_args_SYSV (अक्षर *ढेर, व्योम **avalue, ffi_cif *cif)
{
  अचिन्हित पूर्णांक i;
  व्योम **p_argv;
  अक्षर *argp;
  ffi_type **p_arg;

  argp = ढेर;
  p_argv = avalue;

  क्रम (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->आकार;
#अगर_घोषित __MINT__
      अगर (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (व्योम *) (argp + 2);

	  z = 4;
	}
      अन्यथा
      अगर (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (व्योम *) (argp);

	  z = 4;
	}
      अन्यथा
#पूर्ण_अगर
      अगर (z <= 4)
	{
	  *p_argv = (व्योम *) (argp + 4 - z);

	  z = 4;
	}
      अन्यथा
	{
	  *p_argv = (व्योम *) argp;

	  /* Align if necessary */
	  अगर ((माप(पूर्णांक) - 1) & z)
	    z = FFI_ALIGN(z, माप(पूर्णांक));
	}

      p_argv++;
      argp += z;
    }
}

अचिन्हित पूर्णांक
ffi_closure_SYSV_inner (ffi_closure *closure, व्योम *resp, व्योम *args)
{
  ffi_cif *cif;
  व्योम **arg_area;

  cif = closure->cif;
  arg_area = (व्योम**) alloca (cif->nargs * माप (व्योम *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_तथ्य);

  वापस cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  अगर (cif->abi != FFI_SYSV)
    वापस FFI_BAD_ABI;

  *(अचिन्हित लघु *)closure->tramp = 0x207c;
  *(व्योम **)(closure->tramp + 2) = codeloc;
  *(अचिन्हित लघु *)(closure->tramp + 6) = 0x4ef9;

  अगर (
#अगर_घोषित __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#पूर्ण_अगर
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(व्योम **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  अन्यथा
    *(व्योम **)(closure->tramp + 8) = ffi_closure_SYSV;

#अगर_घोषित __rtems__
  rtems_cache_flush_multiple_तथ्य_पंक्तियाँ( codeloc, FFI_TRAMPOLINE_आकार );
#या_अगर defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_आकार);
#अन्यथा
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_पंक्ति,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_आकार);
#पूर्ण_अगर

  closure->cif  = cif;
  closure->user_तथ्य = user_तथ्य;
  closure->fun  = fun;

  वापस FFI_OK;
}
