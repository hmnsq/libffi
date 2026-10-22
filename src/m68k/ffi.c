/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ifdef __rtems__
ಶೂನ್ಯ rtems_cache_flush_multiple_data_ಸಾಲುಗಳು( ಧ್ರುವ ಶೂನ್ಯ *, size_t );
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ <sys/syscall.h>
#ifdef __MINT__
#ಅನ್ತರ್ಭಾವ <mint/mintbind.h>
#ಅನ್ತರ್ಭಾವ <mint/ssystem.h>
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ <asm/cachectl.h>
#endif
#endif

ಶೂನ್ಯ ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    ಶೂನ್ಯ *, ಶೂನ್ಯ (*fn) ());
ಶೂನ್ಯ *ffi_prep_args (ಶೂನ್ಯ *ರಾಶಿ, extended_cif *ecif);
ಶೂನ್ಯ ffi_closure_SYSV (ffi_closure *);
ಶೂನ್ಯ ffi_closure_struct_SYSV (ffi_closure *);
unsigned ಪೂರ್ಣಾಙ್ಕ ffi_closure_SYSV_inner (ffi_closure *closure,
				     ಶೂನ್ಯ *resp, ಶೂನ್ಯ *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

ಶೂನ್ಯ *
ffi_prep_args (ಶೂನ್ಯ *ರಾಶಿ, extended_cif *ecif)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಶೂನ್ಯ **p_argv;
  ವರ್ಣ *argp;
  ffi_type **p_arg;
  ಶೂನ್ಯ *struct_ಮೌಲ್ಯ_ptr;

  argp = ರಾಶಿ;

  ಯದಿ (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_ಮೌಲ್ಯ_ptr = ecif->rvalue;
  ಅನ್ಯಥಾ
    struct_ಮೌಲ್ಯ_ptr = ರಿಕ್ತ;

  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      ಪೂರ್ಣಾಙ್ಕ type = (*p_arg)->type;

      ಯದಿ (z < ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	{
	  ವಿಕಲ್ಪ (type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	      *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ) *(SINT8 *) *p_argv;
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_UINT8:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ) *(UINT8 *) *p_argv;
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_SINT16:
	      *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ) *(SINT16 *) *p_argv;
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_UINT16:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ) *(UINT16 *) *p_argv;
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_STRUCT:
#ifdef __MINT__
	      ಯದಿ (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              ಅನ್ಯಥಾ
		memcpy (argp, *p_argv, z);
#ಅನ್ಯಥಾ
	      memcpy (argp + ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - z, *p_argv, z);
#endif
	      ಭಙ್ಗ;

	    ಸ್ವಭಾವ:
	      FFI_ASSERT (0);
	    }
	  z = ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	}
      ಅನ್ಯಥಾ
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ಯದಿ ((ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) - 1) & z)
	    z = FFI_ALIGN(z, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
	}

      p_argv++;
      argp += z;
    }

  ಪ್ರತ್ಯಾಗಮ struct_ಮೌಲ್ಯ_ptr;
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
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      cif->flags = 0;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      ಯದಿ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          ಭಙ್ಗ;
        }

      ವಿಕಲ್ಪ (cif->rtype->size)
	{
	ಪಕ್ಷ 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#ಅನ್ಯಥಾ
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  ಭಙ್ಗ;
	ಪಕ್ಷ 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  ಭಙ್ಗ;
#ifdef __MINT__
	ಪಕ್ಷ 3:
#endif
	ಪಕ್ಷ 4:
	  cif->flags = CIF_FLAGS_INT;
	  ಭಙ್ಗ;
#ifdef __MINT__
	ಪಕ್ಷ 7:
#endif
	ಪಕ್ಷ 8:
	  cif->flags = CIF_FLAGS_DINT;
	  ಭಙ್ಗ;
	ಸ್ವಭಾವ:
	  cif->flags = 0;
	  ಭಙ್ಗ;
	}
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      ಭಙ್ಗ;

#ಯದಿ (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#ಅನ್ಯಥಾ
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      ಭಙ್ಗ;
#endif

    ಪಕ್ಷ FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      cif->flags = CIF_FLAGS_INT;
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ಯದಿ (rvalue == ರಿಕ್ತ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      FFI_ASSERT (0);
      ಭಙ್ಗ;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_incoming_args_SYSV (ವರ್ಣ *ರಾಶಿ, ಶೂನ್ಯ **avalue, ffi_cif *cif)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಶೂನ್ಯ **p_argv;
  ವರ್ಣ *argp;
  ffi_type **p_arg;

  argp = ರಾಶಿ;
  p_argv = avalue;

  ಪ್ರತಿ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
#ifdef __MINT__
      ಯದಿ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (ಶೂನ್ಯ *) (argp + 2);

	  z = 4;
	}
      ಅನ್ಯಥಾ
      ಯದಿ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (ಶೂನ್ಯ *) (argp);

	  z = 4;
	}
      ಅನ್ಯಥಾ
#endif
      ಯದಿ (z <= 4)
	{
	  *p_argv = (ಶೂನ್ಯ *) (argp + 4 - z);

	  z = 4;
	}
      ಅನ್ಯಥಾ
	{
	  *p_argv = (ಶೂನ್ಯ *) argp;

	  /* Align if necessary */
	  ಯದಿ ((ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) - 1) & z)
	    z = FFI_ALIGN(z, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
	}

      p_argv++;
      argp += z;
    }
}

unsigned ಪೂರ್ಣಾಙ್ಕ
ffi_closure_SYSV_inner (ffi_closure *closure, ಶೂನ್ಯ *resp, ಶೂನ್ಯ *args)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **arg_area;

  cif = closure->cif;
  arg_area = (ಶೂನ್ಯ**) alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  ಪ್ರತ್ಯಾಗಮ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  ಯದಿ (cif->abi != FFI_SYSV)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(ಶೂನ್ಯ **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  ಯದಿ (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(ಶೂನ್ಯ **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  ಅನ್ಯಥಾ
    *(ಶೂನ್ಯ **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_ಸಾಲುಗಳು( codeloc, FFI_TRAMPOLINE_SIZE );
#ನೋಚೇದ್ಯದಿ defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#ಅನ್ಯಥಾ
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_ಸಾಲು,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}
