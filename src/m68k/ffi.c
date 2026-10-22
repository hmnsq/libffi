/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <unistd.h>
#ifdef __rtems__
வெறுமை rtems_cache_flush_multiple_data_வரிகள்( மாறிலி வெறுமை *, size_t );
#இல்லையெனில்
#அந்தர்பாவ <sys/syscall.h>
#ifdef __MINT__
#அந்தர்பாவ <mint/mintbind.h>
#அந்தர்பாவ <mint/ssystem.h>
#இல்லையெனில்
#அந்தர்பாவ <asm/cachectl.h>
#endif
#endif

வெறுமை ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    வெறுமை *, வெறுமை (*fn) ());
வெறுமை *ffi_prep_args (வெறுமை *அடுக்கு, extended_cif *ecif);
வெறுமை ffi_closure_SYSV (ffi_closure *);
வெறுமை ffi_closure_struct_SYSV (ffi_closure *);
unsigned முழு ffi_closure_SYSV_inner (ffi_closure *closure,
				     வெறுமை *resp, வெறுமை *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

வெறுமை *
ffi_prep_args (வெறுமை *அடுக்கு, extended_cif *ecif)
{
  unsigned முழு i;
  வெறுமை **p_argv;
  எழுத்து *argp;
  ffi_type **p_arg;
  வெறுமை *struct_மதிப்பு_ptr;

  argp = அடுக்கு;

  எனில் (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_மதிப்பு_ptr = ecif->rvalue;
  இல்லையெனில்
    struct_மதிப்பு_ptr = வெற்று;

  p_argv = ecif->avalue;

  க்கு (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      முழு type = (*p_arg)->type;

      எனில் (z < அளவு (முழு))
	{
	  தேர்வு (type)
	    {
	    நிலை FFI_TYPE_SINT8:
	      *(signed முழு *) argp = (signed முழு) *(SINT8 *) *p_argv;
	      நிறுத்து;

	    நிலை FFI_TYPE_UINT8:
	      *(unsigned முழு *) argp = (unsigned முழு) *(UINT8 *) *p_argv;
	      நிறுத்து;

	    நிலை FFI_TYPE_SINT16:
	      *(signed முழு *) argp = (signed முழு) *(SINT16 *) *p_argv;
	      நிறுத்து;

	    நிலை FFI_TYPE_UINT16:
	      *(unsigned முழு *) argp = (unsigned முழு) *(UINT16 *) *p_argv;
	      நிறுத்து;

	    நிலை FFI_TYPE_STRUCT:
#ifdef __MINT__
	      எனில் (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              இல்லையெனில்
		memcpy (argp, *p_argv, z);
#இல்லையெனில்
	      memcpy (argp + அளவு (முழு) - z, *p_argv, z);
#endif
	      நிறுத்து;

	    இயல்பு:
	      FFI_ASSERT (0);
	    }
	  z = அளவு (முழு);
	}
      இல்லையெனில்
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  எனில் ((அளவு(முழு) - 1) & z)
	    z = FFI_ALIGN(z, அளவு(முழு));
	}

      p_argv++;
      argp += z;
    }

  திரும்பு struct_மதிப்பு_ptr;
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
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      cif->flags = 0;
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
      எனில் (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          நிறுத்து;
        }

      தேர்வு (cif->rtype->size)
	{
	நிலை 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#இல்லையெனில்
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  நிறுத்து;
	நிலை 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  நிறுத்து;
#ifdef __MINT__
	நிலை 3:
#endif
	நிலை 4:
	  cif->flags = CIF_FLAGS_INT;
	  நிறுத்து;
#ifdef __MINT__
	நிலை 7:
#endif
	நிலை 8:
	  cif->flags = CIF_FLAGS_DINT;
	  நிறுத்து;
	இயல்பு:
	  cif->flags = 0;
	  நிறுத்து;
	}
      நிறுத்து;

    நிலை FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      நிறுத்து;

    நிலை FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      நிறுத்து;

#எனில் (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    நிலை FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#இல்லையெனில்
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      நிறுத்து;
#endif

    நிலை FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      நிறுத்து;

    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      நிறுத்து;

    நிலை FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      நிறுத்து;

    நிலை FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      நிறுத்து;

    இயல்பு:
      cif->flags = CIF_FLAGS_INT;
      நிறுத்து;
    }

  திரும்பு FFI_OK;
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn) (), வெறுமை *rvalue, வெறுமை **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  எனில் (rvalue == வெற்று
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  இல்லையெனில்
    ecif.rvalue = rvalue;

  தேர்வு (cif->abi)
    {
    நிலை FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      நிறுத்து;

    இயல்பு:
      FFI_ASSERT (0);
      நிறுத்து;
    }
}

நிலையான வெறுமை
ffi_prep_incoming_args_SYSV (எழுத்து *அடுக்கு, வெறுமை **avalue, ffi_cif *cif)
{
  unsigned முழு i;
  வெறுமை **p_argv;
  எழுத்து *argp;
  ffi_type **p_arg;

  argp = அடுக்கு;
  p_argv = avalue;

  க்கு (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
#ifdef __MINT__
      எனில் (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (வெறுமை *) (argp + 2);

	  z = 4;
	}
      இல்லையெனில்
      எனில் (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (வெறுமை *) (argp);

	  z = 4;
	}
      இல்லையெனில்
#endif
      எனில் (z <= 4)
	{
	  *p_argv = (வெறுமை *) (argp + 4 - z);

	  z = 4;
	}
      இல்லையெனில்
	{
	  *p_argv = (வெறுமை *) argp;

	  /* Align if necessary */
	  எனில் ((அளவு(முழு) - 1) & z)
	    z = FFI_ALIGN(z, அளவு(முழு));
	}

      p_argv++;
      argp += z;
    }
}

unsigned முழு
ffi_closure_SYSV_inner (ffi_closure *closure, வெறுமை *resp, வெறுமை *args)
{
  ffi_cif *cif;
  வெறுமை **arg_area;

  cif = closure->cif;
  arg_area = (வெறுமை**) alloca (cif->nargs * அளவு (வெறுமை *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  திரும்பு cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  எனில் (cif->abi != FFI_SYSV)
    திரும்பு FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(வெறுமை **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  எனில் (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(வெறுமை **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  இல்லையெனில்
    *(வெறுமை **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_வரிகள்( codeloc, FFI_TRAMPOLINE_SIZE );
#அல்லதெனில் defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#இல்லையெனில்
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_வரி,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  திரும்பு FFI_OK;
}
