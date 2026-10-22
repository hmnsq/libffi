/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>
#यदि_परिभाषितम् __rtems__
शून्य rtems_cache_flush_गुणजः_दत्त_पङ्क्तयः( ध्रुव शून्य *, size_t );
#अन्यथा
#अन्तर्भाव <sys/syscall.h>
#यदि_परिभाषितम् __MINT__
#अन्तर्भाव <mint/mintbind.h>
#अन्तर्भाव <mint/ssystem.h>
#अन्यथा
#अन्तर्भाव <asm/cachectl.h>
#यदि_अन्तः
#यदि_अन्तः

शून्य ffi_call_SYSV (extended_cif *,
		    अचिह्नित, अचिह्नित,
		    शून्य *, शून्य (*fn) ());
शून्य *ffi_prep_तर्काः (शून्य *राशिः, extended_cif *ecif);
शून्य ffi_संवृतिः_SYSV (ffi_संवृतिः *);
शून्य ffi_संवृतिः_struct_SYSV (ffi_संवृतिः *);
अचिह्नित पूर्णाङ्क ffi_संवृतिः_SYSV_inner (ffi_संवृतिः *संवृतिः,
				     शून्य *resp, शून्य *तर्काः);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

शून्य *
ffi_prep_तर्काः (शून्य *राशिः, extended_cif *ecif)
{
  अचिह्नित पूर्णाङ्क i;
  शून्य **p_argv;
  वर्ण *argp;
  ffi_type **p_arg;
  शून्य *struct_मानम्_ptr;

  argp = राशिः;

  यदि (
#यदि_परिभाषितम् __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#यदि_अन्तः
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_मानम्_ptr = ecif->rvalue;
  अन्यथा
    struct_मानम्_ptr = रिक्त;

  p_argv = ecif->avalue;

  प्रति (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->आकार;
      पूर्णाङ्क type = (*p_arg)->type;

      यदि (z < परिमाण (पूर्णाङ्क))
	{
	  विकल्प (type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	      *(चिह्नित पूर्णाङ्क *) argp = (चिह्नित पूर्णाङ्क) *(SINT8 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_UINT8:
	      *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क) *(UINT8 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_SINT16:
	      *(चिह्नित पूर्णाङ्क *) argp = (चिह्नित पूर्णाङ्क) *(SINT16 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_UINT16:
	      *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क) *(UINT16 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_STRUCT:
#यदि_परिभाषितम् __MINT__
	      यदि (z == 1 || z == 2)
		स्मृतिं_प्रतिलिख (argp + 2, *p_argv, z);
              अन्यथा
		स्मृतिं_प्रतिलिख (argp, *p_argv, z);
#अन्यथा
	      स्मृतिं_प्रतिलिख (argp + परिमाण (पूर्णाङ्क) - z, *p_argv, z);
#यदि_अन्तः
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT (0);
	    }
	  z = परिमाण (पूर्णाङ्क);
	}
      अन्यथा
	{
	  स्मृतिं_प्रतिलिख (argp, *p_argv, z);

	  /* Align if necessary.  */
	  यदि ((परिमाण(पूर्णाङ्क) - 1) & z)
	    z = FFI_ALIGN(z, परिमाण(पूर्णाङ्क));
	}

      p_argv++;
      argp += z;
    }

  प्रत्यागम struct_मानम्_ptr;
}

#परिभाषय CIF_FLAGS_INT		1
#परिभाषय CIF_FLAGS_DINT		2
#परिभाषय CIF_FLAGS_FLOAT		4
#परिभाषय CIF_FLAGS_DOUBLE	8
#परिभाषय CIF_FLAGS_LDOUBLE	16
#परिभाषय CIF_FLAGS_POINTER	32
#परिभाषय CIF_FLAGS_STRUCT1	64
#परिभाषय CIF_FLAGS_STRUCT2	128
#परिभाषय CIF_FLAGS_SINT8		256
#परिभाषय CIF_FLAGS_SINT16	512

/* Perform machine dependent cif processing */
ffi_स्थितिः
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      cif->flags = 0;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      यदि (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          भङ्ग;
        }

      विकल्प (cif->rtype->आकार)
	{
	पक्ष 1:
#यदि_परिभाषितम् __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#अन्यथा
	  cif->flags = CIF_FLAGS_STRUCT1;
#यदि_अन्तः
	  भङ्ग;
	पक्ष 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  भङ्ग;
#यदि_परिभाषितम् __MINT__
	पक्ष 3:
#यदि_अन्तः
	पक्ष 4:
	  cif->flags = CIF_FLAGS_INT;
	  भङ्ग;
#यदि_परिभाषितम् __MINT__
	पक्ष 7:
#यदि_अन्तः
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

#यदि (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    पक्ष FFI_TYPE_LONGDOUBLE:
#यदि_परिभाषितम् __MINT__
      cif->flags = 0;
#अन्यथा
      cif->flags = CIF_FLAGS_LDOUBLE;
#यदि_अन्तः
      भङ्ग;
#यदि_अन्तः

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

  प्रत्यागम FFI_OK;
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  यदि (rvalue == रिक्त
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->आकार > 8)
    ecif.rvalue = alloca (cif->rtype->आकार);
  अन्यथा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->अष्टकानि, cif->flags,
		     ecif.rvalue, fn);
      भङ्ग;

    स्वभाव:
      FFI_ASSERT (0);
      भङ्ग;
    }
}

स्थिर शून्य
ffi_prep_incoming_तर्काः_SYSV (वर्ण *राशिः, शून्य **avalue, ffi_cif *cif)
{
  अचिह्नित पूर्णाङ्क i;
  शून्य **p_argv;
  वर्ण *argp;
  ffi_type **p_arg;

  argp = राशिः;
  p_argv = avalue;

  प्रति (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->आकार;
#यदि_परिभाषितम् __MINT__
      यदि (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (शून्य *) (argp + 2);

	  z = 4;
	}
      अन्यथा
      यदि (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (शून्य *) (argp);

	  z = 4;
	}
      अन्यथा
#यदि_अन्तः
      यदि (z <= 4)
	{
	  *p_argv = (शून्य *) (argp + 4 - z);

	  z = 4;
	}
      अन्यथा
	{
	  *p_argv = (शून्य *) argp;

	  /* Align if necessary */
	  यदि ((परिमाण(पूर्णाङ्क) - 1) & z)
	    z = FFI_ALIGN(z, परिमाण(पूर्णाङ्क));
	}

      p_argv++;
      argp += z;
    }
}

अचिह्नित पूर्णाङ्क
ffi_संवृतिः_SYSV_inner (ffi_संवृतिः *संवृतिः, शून्य *resp, शून्य *तर्काः)
{
  ffi_cif *cif;
  शून्य **arg_area;

  cif = संवृतिः->cif;
  arg_area = (शून्य**) alloca (cif->nargs * परिमाण (शून्य *));

  ffi_prep_incoming_तर्काः_SYSV(तर्काः, arg_area, cif);

  (संवृतिः->fun) (cif, resp, arg_area, संवृतिः->user_दत्त);

  प्रत्यागम cif->flags;
}

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः* संवृतिः,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		      शून्य *user_दत्त,
		      शून्य *codeloc)
{
  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  *(अचिह्नित ह्रस्व *)संवृतिः->tramp = 0x207c;
  *(शून्य **)(संवृतिः->tramp + 2) = codeloc;
  *(अचिह्नित ह्रस्व *)(संवृतिः->tramp + 6) = 0x4ef9;

  यदि (
#यदि_परिभाषितम् __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#यदि_अन्तः
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(शून्य **)(संवृतिः->tramp + 8) = ffi_संवृतिः_struct_SYSV;
  अन्यथा
    *(शून्य **)(संवृतिः->tramp + 8) = ffi_संवृतिः_SYSV;

#यदि_परिभाषितम् __rtems__
  rtems_cache_flush_गुणजः_दत्त_पङ्क्तयः( codeloc, FFI_TRAMPOLINE_आकार );
#नोचेद्यदि defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_आकार);
#अन्यथा
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_पङ्क्तिः,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_आकार);
#यदि_अन्तः

  संवृतिः->cif  = cif;
  संवृतिः->user_दत्त = user_दत्त;
  संवृतिः->fun  = fun;

  प्रत्यागम FFI_OK;
}
