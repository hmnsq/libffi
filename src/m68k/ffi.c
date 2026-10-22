/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>
#شامل <unistd.h>
#ifdef __rtems__
تهی rtems_cache_flush_multiple_data_سطرها( ثابت تهی *, size_t );
#وگرنه
#شامل <sys/syscall.h>
#ifdef __MINT__
#شامل <mint/mintbind.h>
#شامل <mint/ssystem.h>
#وگرنه
#شامل <asm/cachectl.h>
#endif
#endif

تهی ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    تهی *, تهی (*fn) ());
تهی *ffi_prep_args (تهی *پشته, extended_cif *ecif);
تهی ffi_closure_SYSV (ffi_closure *);
تهی ffi_closure_struct_SYSV (ffi_closure *);
unsigned صحیح ffi_closure_SYSV_inner (ffi_closure *closure,
				     تهی *resp, تهی *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

تهی *
ffi_prep_args (تهی *پشته, extended_cif *ecif)
{
  unsigned صحیح i;
  تهی **p_argv;
  نویسه *argp;
  ffi_type **p_arg;
  تهی *struct_مقدار_ptr;

  argp = پشته;

  اگر (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_مقدار_ptr = ecif->rvalue;
  وگرنه
    struct_مقدار_ptr = پوچ;

  p_argv = ecif->avalue;

  برای (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      صحیح type = (*p_arg)->type;

      اگر (z < اندازه (صحیح))
	{
	  انتخاب (type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(signed صحیح *) argp = (signed صحیح) *(SINT8 *) *p_argv;
	      بشکن;

	    حالت FFI_TYPE_UINT8:
	      *(unsigned صحیح *) argp = (unsigned صحیح) *(UINT8 *) *p_argv;
	      بشکن;

	    حالت FFI_TYPE_SINT16:
	      *(signed صحیح *) argp = (signed صحیح) *(SINT16 *) *p_argv;
	      بشکن;

	    حالت FFI_TYPE_UINT16:
	      *(unsigned صحیح *) argp = (unsigned صحیح) *(UINT16 *) *p_argv;
	      بشکن;

	    حالت FFI_TYPE_STRUCT:
#ifdef __MINT__
	      اگر (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              وگرنه
		memcpy (argp, *p_argv, z);
#وگرنه
	      memcpy (argp + اندازه (صحیح) - z, *p_argv, z);
#endif
	      بشکن;

	    پیشفرض:
	      FFI_ASSERT (0);
	    }
	  z = اندازه (صحیح);
	}
      وگرنه
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  اگر ((اندازه(صحیح) - 1) & z)
	    z = FFI_ALIGN(z, اندازه(صحیح));
	}

      p_argv++;
      argp += z;
    }

  بازگشت struct_مقدار_ptr;
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
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      cif->flags = 0;
      بشکن;

    حالت FFI_TYPE_STRUCT:
      اگر (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          بشکن;
        }

      انتخاب (cif->rtype->size)
	{
	حالت 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#وگرنه
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  بشکن;
	حالت 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  بشکن;
#ifdef __MINT__
	حالت 3:
#endif
	حالت 4:
	  cif->flags = CIF_FLAGS_INT;
	  بشکن;
#ifdef __MINT__
	حالت 7:
#endif
	حالت 8:
	  cif->flags = CIF_FLAGS_DINT;
	  بشکن;
	پیشفرض:
	  cif->flags = 0;
	  بشکن;
	}
      بشکن;

    حالت FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      بشکن;

    حالت FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      بشکن;

#اگر (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    حالت FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#وگرنه
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      بشکن;
#endif

    حالت FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      بشکن;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      بشکن;

    حالت FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      بشکن;

    حالت FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      بشکن;

    پیشفرض:
      cif->flags = CIF_FLAGS_INT;
      بشکن;
    }

  بازگشت FFI_OK;
}

تهی
ffi_call (ffi_cif *cif, تهی (*fn) (), تهی *rvalue, تهی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  اگر (rvalue == پوچ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  وگرنه
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      بشکن;

    پیشفرض:
      FFI_ASSERT (0);
      بشکن;
    }
}

ایستا تهی
ffi_prep_incoming_args_SYSV (نویسه *پشته, تهی **avalue, ffi_cif *cif)
{
  unsigned صحیح i;
  تهی **p_argv;
  نویسه *argp;
  ffi_type **p_arg;

  argp = پشته;
  p_argv = avalue;

  برای (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
#ifdef __MINT__
      اگر (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (تهی *) (argp + 2);

	  z = 4;
	}
      وگرنه
      اگر (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (تهی *) (argp);

	  z = 4;
	}
      وگرنه
#endif
      اگر (z <= 4)
	{
	  *p_argv = (تهی *) (argp + 4 - z);

	  z = 4;
	}
      وگرنه
	{
	  *p_argv = (تهی *) argp;

	  /* Align if necessary */
	  اگر ((اندازه(صحیح) - 1) & z)
	    z = FFI_ALIGN(z, اندازه(صحیح));
	}

      p_argv++;
      argp += z;
    }
}

unsigned صحیح
ffi_closure_SYSV_inner (ffi_closure *closure, تهی *resp, تهی *args)
{
  ffi_cif *cif;
  تهی **arg_area;

  cif = closure->cif;
  arg_area = (تهی**) alloca (cif->nargs * اندازه (تهی *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  بازگشت cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  اگر (cif->abi != FFI_SYSV)
    بازگشت FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(تهی **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  اگر (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(تهی **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  وگرنه
    *(تهی **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_سطرها( codeloc, FFI_TRAMPOLINE_SIZE );
#وگرنهاگر defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#وگرنه
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_سطر,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  بازگشت FFI_OK;
}
