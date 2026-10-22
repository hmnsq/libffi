/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <unistd.h>
#ifdef __rtems__
শূন্য rtems_cache_flush_multiple_তথ্য_সারিগুলি( ধ্রুবক শূন্য *, size_t );
#অন্যথা
#অন্তর্ভাব <sys/syscall.h>
#ifdef __MINT__
#অন্তর্ভাব <mint/mintbind.h>
#অন্তর্ভাব <mint/ssystem.h>
#অন্যথা
#অন্তর্ভাব <asm/cachectl.h>
#endif
#endif

শূন্য ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    শূন্য *, শূন্য (*fn) ());
শূন্য *ffi_prep_args (শূন্য *স্তূপ, extended_cif *ecif);
শূন্য ffi_closure_SYSV (ffi_closure *);
শূন্য ffi_closure_struct_SYSV (ffi_closure *);
unsigned পূর্ণসংখ্যা ffi_closure_SYSV_inner (ffi_closure *closure,
				     শূন্য *resp, শূন্য *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

শূন্য *
ffi_prep_args (শূন্য *স্তূপ, extended_cif *ecif)
{
  unsigned পূর্ণসংখ্যা i;
  শূন্য **p_argv;
  অক্ষর *argp;
  ffi_type **p_arg;
  শূন্য *struct_মান_ptr;

  argp = স্তূপ;

  যদি (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_মান_ptr = ecif->rvalue;
  অন্যথা
    struct_মান_ptr = ফাঁকা;

  p_argv = ecif->avalue;

  জন্য (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->আকার_2;
      পূর্ণসংখ্যা type = (*p_arg)->type;

      যদি (z < আকার (পূর্ণসংখ্যা))
	{
	  নির্বাচন (type)
	    {
	    ক্ষেত্র FFI_TYPE_SINT8:
	      *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা) *(SINT8 *) *p_argv;
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_UINT8:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা) *(UINT8 *) *p_argv;
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_SINT16:
	      *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা) *(SINT16 *) *p_argv;
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_UINT16:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা) *(UINT16 *) *p_argv;
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_STRUCT:
#ifdef __MINT__
	      যদি (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              অন্যথা
		memcpy (argp, *p_argv, z);
#অন্যথা
	      memcpy (argp + আকার (পূর্ণসংখ্যা) - z, *p_argv, z);
#endif
	      বিরতি;

	    পূর্বনির্ধারিত:
	      FFI_ASSERT (0);
	    }
	  z = আকার (পূর্ণসংখ্যা);
	}
      অন্যথা
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  যদি ((আকার(পূর্ণসংখ্যা) - 1) & z)
	    z = FFI_ALIGN(z, আকার(পূর্ণসংখ্যা));
	}

      p_argv++;
      argp += z;
    }

  ফেরত struct_মান_ptr;
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
  নির্বাচন (cif->rtype->type)
    {
    ক্ষেত্র FFI_TYPE_VOID:
      cif->flags = 0;
      বিরতি;

    ক্ষেত্র FFI_TYPE_STRUCT:
      যদি (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          বিরতি;
        }

      নির্বাচন (cif->rtype->আকার_2)
	{
	ক্ষেত্র 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#অন্যথা
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  বিরতি;
	ক্ষেত্র 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  বিরতি;
#ifdef __MINT__
	ক্ষেত্র 3:
#endif
	ক্ষেত্র 4:
	  cif->flags = CIF_FLAGS_INT;
	  বিরতি;
#ifdef __MINT__
	ক্ষেত্র 7:
#endif
	ক্ষেত্র 8:
	  cif->flags = CIF_FLAGS_DINT;
	  বিরতি;
	পূর্বনির্ধারিত:
	  cif->flags = 0;
	  বিরতি;
	}
      বিরতি;

    ক্ষেত্র FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      বিরতি;

    ক্ষেত্র FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      বিরতি;

#যদি (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#অন্যথা
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      বিরতি;
#endif

    ক্ষেত্র FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      বিরতি;

    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      বিরতি;

    ক্ষেত্র FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      বিরতি;

    ক্ষেত্র FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      বিরতি;

    পূর্বনির্ধারিত:
      cif->flags = CIF_FLAGS_INT;
      বিরতি;
    }

  ফেরত FFI_OK;
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  যদি (rvalue == ফাঁকা
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->আকার_2 > 8)
    ecif.rvalue = alloca (cif->rtype->আকার_2);
  অন্যথা
    ecif.rvalue = rvalue;

  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      বিরতি;

    পূর্বনির্ধারিত:
      FFI_ASSERT (0);
      বিরতি;
    }
}

স্থির শূন্য
ffi_prep_incoming_args_SYSV (অক্ষর *স্তূপ, শূন্য **avalue, ffi_cif *cif)
{
  unsigned পূর্ণসংখ্যা i;
  শূন্য **p_argv;
  অক্ষর *argp;
  ffi_type **p_arg;

  argp = স্তূপ;
  p_argv = avalue;

  জন্য (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->আকার_2;
#ifdef __MINT__
      যদি (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (শূন্য *) (argp + 2);

	  z = 4;
	}
      অন্যথা
      যদি (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (শূন্য *) (argp);

	  z = 4;
	}
      অন্যথা
#endif
      যদি (z <= 4)
	{
	  *p_argv = (শূন্য *) (argp + 4 - z);

	  z = 4;
	}
      অন্যথা
	{
	  *p_argv = (শূন্য *) argp;

	  /* Align if necessary */
	  যদি ((আকার(পূর্ণসংখ্যা) - 1) & z)
	    z = FFI_ALIGN(z, আকার(পূর্ণসংখ্যা));
	}

      p_argv++;
      argp += z;
    }
}

unsigned পূর্ণসংখ্যা
ffi_closure_SYSV_inner (ffi_closure *closure, শূন্য *resp, শূন্য *args)
{
  ffi_cif *cif;
  শূন্য **arg_area;

  cif = closure->cif;
  arg_area = (শূন্য**) alloca (cif->nargs * আকার (শূন্য *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_তথ্য);

  ফেরত cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc)
{
  যদি (cif->abi != FFI_SYSV)
    ফেরত FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(শূন্য **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  যদি (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(শূন্য **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  অন্যথা
    *(শূন্য **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_তথ্য_সারিগুলি( codeloc, FFI_TRAMPOLINE_আকার );
#নতুবা_যদি defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_আকার);
#অন্যথা
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_সারি,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_আকার);
#endif

  closure->cif  = cif;
  closure->user_তথ্য = user_তথ্য;
  closure->fun  = fun;

  ফেরত FFI_OK;
}
