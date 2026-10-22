/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>
#ifdef __rtems__
शून्य rtems_cache_flush_multiple_data_पङ्क्तयः( ध्रुव शून्य *, size_t );
#अञ्ञथा
#अन्तर्भाव <sys/syscall.h>
#ifdef __MINT__
#अन्तर्भाव <mint/mintbind.h>
#अन्तर्भाव <mint/ssystem.h>
#अञ्ञथा
#अन्तर्भाव <asm/cachectl.h>
#endif
#endif

शून्य ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    शून्य *, शून्य (*fn) ());
शून्य *ffi_prep_args (शून्य *राशिः, extended_cif *ecif);
शून्य ffi_closure_SYSV (ffi_closure *);
शून्य ffi_closure_struct_SYSV (ffi_closure *);
unsigned पूर्णाङ्क ffi_closure_SYSV_inner (ffi_closure *closure,
				     शून्य *resp, शून्य *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

शून्य *
ffi_prep_args (शून्य *राशिः, extended_cif *ecif)
{
  unsigned पूर्णाङ्क i;
  शून्य **p_argv;
  वर्ण *argp;
  ffi_type **p_arg;
  शून्य *struct_मानम्_ptr;

  argp = राशिः;

  सचे (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_मानम्_ptr = ecif->rvalue;
  अञ्ञथा
    struct_मानम्_ptr = रिक्त;

  p_argv = ecif->avalue;

  अत्थाय (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      पूर्णाङ्क type = (*p_arg)->type;

      सचे (z < परिमाण (पूर्णाङ्क))
	{
	  विकल्प (type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	      *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क) *(SINT8 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_UINT8:
	      *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क) *(UINT8 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_SINT16:
	      *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क) *(SINT16 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_UINT16:
	      *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क) *(UINT16 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_STRUCT:
#ifdef __MINT__
	      सचे (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              अञ्ञथा
		memcpy (argp, *p_argv, z);
#अञ्ञथा
	      memcpy (argp + परिमाण (पूर्णाङ्क) - z, *p_argv, z);
#endif
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT (0);
	    }
	  z = परिमाण (पूर्णाङ्क);
	}
      अञ्ञथा
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  सचे ((परिमाण(पूर्णाङ्क) - 1) & z)
	    z = FFI_ALIGN(z, परिमाण(पूर्णाङ्क));
	}

      p_argv++;
      argp += z;
    }

  पटिनिवत्त struct_मानम्_ptr;
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
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      cif->flags = 0;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      सचे (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          भङ्ग;
        }

      विकल्प (cif->rtype->size)
	{
	पक्ष 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#अञ्ञथा
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  भङ्ग;
	पक्ष 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  भङ्ग;
#ifdef __MINT__
	पक्ष 3:
#endif
	पक्ष 4:
	  cif->flags = CIF_FLAGS_INT;
	  भङ्ग;
#ifdef __MINT__
	पक्ष 7:
#endif
	पक्ष 8:
	  cif->flags = CIF_FLAGS_DINT;
	  भङ्ग;
	स्वभाव:
	  cif->flags = 0;
	  भङ्ग;
	}
      भङ्ग;

    पक्ष FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      भङ्ग;

    पक्ष FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      भङ्ग;

#सचे (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    पक्ष FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#अञ्ञथा
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      भङ्ग;
#endif

    पक्ष FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      भङ्ग;

    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      भङ्ग;

    पक्ष FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      भङ्ग;

    पक्ष FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      भङ्ग;

    स्वभाव:
      cif->flags = CIF_FLAGS_INT;
      भङ्ग;
    }

  पटिनिवत्त FFI_OK;
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  सचे (rvalue == रिक्त
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  अञ्ञथा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      भङ्ग;

    स्वभाव:
      FFI_ASSERT (0);
      भङ्ग;
    }
}

स्थिर शून्य
ffi_prep_incoming_args_SYSV (वर्ण *राशिः, शून्य **avalue, ffi_cif *cif)
{
  unsigned पूर्णाङ्क i;
  शून्य **p_argv;
  वर्ण *argp;
  ffi_type **p_arg;

  argp = राशिः;
  p_argv = avalue;

  अत्थाय (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
#ifdef __MINT__
      सचे (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (शून्य *) (argp + 2);

	  z = 4;
	}
      अञ्ञथा
      सचे (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (शून्य *) (argp);

	  z = 4;
	}
      अञ्ञथा
#endif
      सचे (z <= 4)
	{
	  *p_argv = (शून्य *) (argp + 4 - z);

	  z = 4;
	}
      अञ्ञथा
	{
	  *p_argv = (शून्य *) argp;

	  /* Align if necessary */
	  सचे ((परिमाण(पूर्णाङ्क) - 1) & z)
	    z = FFI_ALIGN(z, परिमाण(पूर्णाङ्क));
	}

      p_argv++;
      argp += z;
    }
}

unsigned पूर्णाङ्क
ffi_closure_SYSV_inner (ffi_closure *closure, शून्य *resp, शून्य *args)
{
  ffi_cif *cif;
  शून्य **arg_area;

  cif = closure->cif;
  arg_area = (शून्य**) alloca (cif->nargs * परिमाण (शून्य *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  पटिनिवत्त cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  सचे (cif->abi != FFI_SYSV)
    पटिनिवत्त FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(शून्य **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  सचे (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(शून्य **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  अञ्ञथा
    *(शून्य **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_पङ्क्तयः( codeloc, FFI_TRAMPOLINE_SIZE );
#नोचेद्यदि defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#अञ्ञथा
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_पङ्क्तिः,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  पटिनिवत्त FFI_OK;
}
