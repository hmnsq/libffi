/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>
#ifdef __rtems__
शून्य rtems_cache_flush_multiple_data_ओळी( स्थिरांक शून्य *, माप_प्रकार );
#अन्यथा
#अन्तर्भाव <sys/syscall.h>
#ifdef __MINT__
#अन्तर्भाव <mint/mintbind.h>
#अन्तर्भाव <mint/ssystem.h>
#अन्यथा
#अन्तर्भाव <asm/cachectl.h>
#endif
#endif

शून्य ffi_call_SYSV (extended_cif *,
		    अचिन्हित, अचिन्हित,
		    शून्य *, शून्य (*fn) ());
शून्य *ffi_prep_args (शून्य *चळत, extended_cif *ecif);
शून्य ffi_closure_SYSV (ffi_closure *);
शून्य ffi_closure_struct_SYSV (ffi_closure *);
अचिन्हित पूर्णांक ffi_closure_SYSV_inner (ffi_closure *closure,
				     शून्य *resp, शून्य *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

शून्य *
ffi_prep_args (शून्य *चळत, extended_cif *ecif)
{
  अचिन्हित पूर्णांक i;
  शून्य **p_argv;
  अक्षर *argp;
  ffi_type **p_arg;
  शून्य *struct_मूल्य_ptr;

  argp = चळत;

  जर (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_मूल्य_ptr = ecif->rvalue;
  अन्यथा
    struct_मूल्य_ptr = रिक्त;

  p_argv = ecif->avalue;

  साठी (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      माप_प्रकार z = (*p_arg)->size;
      पूर्णांक type = (*p_arg)->type;

      जर (z < आकार (पूर्णांक))
	{
	  निवड (type)
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
#ifdef __MINT__
	      जर (z == 1 || z == 2)
		स_नकल (argp + 2, *p_argv, z);
              अन्यथा
		स_नकल (argp, *p_argv, z);
#अन्यथा
	      स_नकल (argp + आकार (पूर्णांक) - z, *p_argv, z);
#endif
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

#define CIF_FLAGS_INT		1
#define CIF_FLAGS_DINT		2
#define CIF_FLAGS_FLOAT		4
#define CIF_FLAGS_DOUBLE	8
#define CIF_FLAGS_LDOUBLE	16
#define CIF_FLAGS_POINTER	32
#define CIF_FLAGS_STRUCT1	64
#define CIF_FLAGS_STRUCT2	128
#define CIF_FLAGS_SINT8		256
#define CIF_FLAGS_SINT16	512

/* Perform machine dependent cif processing */
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

      निवड (cif->rtype->size)
	{
	स्थिती 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#अन्यथा
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  थांबा;
	स्थिती 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  थांबा;
#ifdef __MINT__
	स्थिती 3:
#endif
	स्थिती 4:
	  cif->flags = CIF_FLAGS_INT;
	  थांबा;
#ifdef __MINT__
	स्थिती 7:
#endif
	स्थिती 8:
	  cif->flags = CIF_FLAGS_DINT;
	  थांबा;
	पूर्वनिर्धारित:
	  cif->flags = 0;
	  थांबा;
	}
      थांबा;

    स्थिती FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      थांबा;

    स्थिती FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      थांबा;

#जर (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    स्थिती FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#अन्यथा
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      थांबा;
#endif

    स्थिती FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      थांबा;

    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      थांबा;

    स्थिती FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      थांबा;

    स्थिती FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      थांबा;

    पूर्वनिर्धारित:
      cif->flags = CIF_FLAGS_INT;
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
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  अन्यथा
    ecif.rvalue = rvalue;

  निवड (cif->abi)
    {
    स्थिती FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      थांबा;

    पूर्वनिर्धारित:
      FFI_ASSERT (0);
      थांबा;
    }
}

स्थिर शून्य
ffi_prep_incoming_args_SYSV (अक्षर *चळत, शून्य **avalue, ffi_cif *cif)
{
  अचिन्हित पूर्णांक i;
  शून्य **p_argv;
  अक्षर *argp;
  ffi_type **p_arg;

  argp = चळत;
  p_argv = avalue;

  साठी (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->size;
#ifdef __MINT__
      जर (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (शून्य *) (argp + 2);

	  z = 4;
	}
      अन्यथा
      जर (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (शून्य *) (argp);

	  z = 4;
	}
      अन्यथा
#endif
      जर (z <= 4)
	{
	  *p_argv = (शून्य *) (argp + 4 - z);

	  z = 4;
	}
      अन्यथा
	{
	  *p_argv = (शून्य *) argp;

	  /* Align if necessary */
	  जर ((आकार(पूर्णांक) - 1) & z)
	    z = FFI_ALIGN(z, आकार(पूर्णांक));
	}

      p_argv++;
      argp += z;
    }
}

अचिन्हित पूर्णांक
ffi_closure_SYSV_inner (ffi_closure *closure, शून्य *resp, शून्य *args)
{
  ffi_cif *cif;
  शून्य **arg_area;

  cif = closure->cif;
  arg_area = (शून्य**) alloca (cif->nargs * आकार (शून्य *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  परत cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  जर (cif->abi != FFI_SYSV)
    परत FFI_BAD_ABI;

  *(अचिन्हित लघु *)closure->tramp = 0x207c;
  *(शून्य **)(closure->tramp + 2) = codeloc;
  *(अचिन्हित लघु *)(closure->tramp + 6) = 0x4ef9;

  जर (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(शून्य **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  अन्यथा
    *(शून्य **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_ओळी( codeloc, FFI_TRAMPOLINE_SIZE );
#किंवाजर defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#अन्यथा
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_ओळ,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  परत FFI_OK;
}
