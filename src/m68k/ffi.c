/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <unistd.h>
#ifdef __rtems__
শূন্য rtems_cache_flush_multiple_data_সারিগুলি( ধ্রুব শূন্য *, size_t );
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
unsigned পূর্ণাঙ্ক ffi_closure_SYSV_inner (ffi_closure *closure,
				     শূন্য *resp, শূন্য *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

শূন্য *
ffi_prep_args (শূন্য *স্তূপ, extended_cif *ecif)
{
  unsigned পূর্ণাঙ্ক i;
  শূন্য **p_argv;
  বর্ণ *argp;
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
    struct_মান_ptr = রিক্ত;

  p_argv = ecif->avalue;

  প্রতি (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      পূর্ণাঙ্ক type = (*p_arg)->type;

      যদি (z < পরিমাণ (পূর্ণাঙ্ক))
	{
	  বিকল্প (type)
	    {
	    পক্ষ FFI_TYPE_SINT8:
	      *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক) *(SINT8 *) *p_argv;
	      ভঙ্গ;

	    পক্ষ FFI_TYPE_UINT8:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক) *(UINT8 *) *p_argv;
	      ভঙ্গ;

	    পক্ষ FFI_TYPE_SINT16:
	      *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক) *(SINT16 *) *p_argv;
	      ভঙ্গ;

	    পক্ষ FFI_TYPE_UINT16:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক) *(UINT16 *) *p_argv;
	      ভঙ্গ;

	    পক্ষ FFI_TYPE_STRUCT:
#ifdef __MINT__
	      যদি (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              অন্যথা
		memcpy (argp, *p_argv, z);
#অন্যথা
	      memcpy (argp + পরিমাণ (পূর্ণাঙ্ক) - z, *p_argv, z);
#endif
	      ভঙ্গ;

	    স্বভাব:
	      FFI_ASSERT (0);
	    }
	  z = পরিমাণ (পূর্ণাঙ্ক);
	}
      অন্যথা
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  যদি ((পরিমাণ(পূর্ণাঙ্ক) - 1) & z)
	    z = FFI_ALIGN(z, পরিমাণ(পূর্ণাঙ্ক));
	}

      p_argv++;
      argp += z;
    }

  প্রত্যাগম struct_মান_ptr;
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
  বিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_VOID:
      cif->flags = 0;
      ভঙ্গ;

    পক্ষ FFI_TYPE_STRUCT:
      যদি (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          ভঙ্গ;
        }

      বিকল্প (cif->rtype->size)
	{
	পক্ষ 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#অন্যথা
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  ভঙ্গ;
	পক্ষ 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  ভঙ্গ;
#ifdef __MINT__
	পক্ষ 3:
#endif
	পক্ষ 4:
	  cif->flags = CIF_FLAGS_INT;
	  ভঙ্গ;
#ifdef __MINT__
	পক্ষ 7:
#endif
	পক্ষ 8:
	  cif->flags = CIF_FLAGS_DINT;
	  ভঙ্গ;
	স্বভাব:
	  cif->flags = 0;
	  ভঙ্গ;
	}
      ভঙ্গ;

    পক্ষ FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      ভঙ্গ;

    পক্ষ FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      ভঙ্গ;

#যদি (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    পক্ষ FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#অন্যথা
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      ভঙ্গ;
#endif

    পক্ষ FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      ভঙ্গ;

    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ভঙ্গ;

    পক্ষ FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      ভঙ্গ;

    পক্ষ FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      ভঙ্গ;

    স্বভাব:
      cif->flags = CIF_FLAGS_INT;
      ভঙ্গ;
    }

  প্রত্যাগম FFI_OK;
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  যদি (rvalue == রিক্ত
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  অন্যথা
    ecif.rvalue = rvalue;

  বিকল্প (cif->abi)
    {
    পক্ষ FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      ভঙ্গ;

    স্বভাব:
      FFI_ASSERT (0);
      ভঙ্গ;
    }
}

স্থির শূন্য
ffi_prep_incoming_args_SYSV (বর্ণ *স্তূপ, শূন্য **avalue, ffi_cif *cif)
{
  unsigned পূর্ণাঙ্ক i;
  শূন্য **p_argv;
  বর্ণ *argp;
  ffi_type **p_arg;

  argp = স্তূপ;
  p_argv = avalue;

  প্রতি (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
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
	  যদি ((পরিমাণ(পূর্ণাঙ্ক) - 1) & z)
	    z = FFI_ALIGN(z, পরিমাণ(পূর্ণাঙ্ক));
	}

      p_argv++;
      argp += z;
    }
}

unsigned পূর্ণাঙ্ক
ffi_closure_SYSV_inner (ffi_closure *closure, শূন্য *resp, শূন্য *args)
{
  ffi_cif *cif;
  শূন্য **arg_area;

  cif = closure->cif;
  arg_area = (শূন্য**) alloca (cif->nargs * পরিমাণ (শূন্য *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  প্রত্যাগম cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
		      শূন্য *user_data,
		      শূন্য *codeloc)
{
  যদি (cif->abi != FFI_SYSV)
    প্রত্যাগম FFI_BAD_ABI;

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
  rtems_cache_flush_multiple_data_সারিগুলি( codeloc, FFI_TRAMPOLINE_SIZE );
#নোচেদ্যদি defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#অন্যথা
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_সারি,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  প্রত্যাগম FFI_OK;
}
