/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __rtems__
ಶೂನ್ಯ rtems_cache_flush_ಅಪವರ್ತ್ಯ_ದತ್ತಾಂಶ_ಸಾಲುಗಳು( ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *, size_t );
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ <sys/syscall.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MINT__
#ಅನ್ತರ್ಭಾವ <mint/mintbind.h>
#ಅನ್ತರ್ಭಾವ <mint/ssystem.h>
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ <asm/cachectl.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ ffi_call_SYSV (extended_cif *,
		    ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ,
		    ಶೂನ್ಯ *, ಶೂನ್ಯ (*fn) ());
ಶೂನ್ಯ *ffi_prep_ವಾದಗಳು (ಶೂನ್ಯ *ರಾಶಿ, extended_cif *ecif);
ಶೂನ್ಯ ffi_ಆವರಣ_SYSV (ffi_ಆವರಣ *);
ಶೂನ್ಯ ffi_ಆವರಣ_struct_SYSV (ffi_ಆವರಣ *);
ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ffi_ಆವರಣ_SYSV_inner (ffi_ಆವರಣ *ಆವರಣ,
				     ಶೂನ್ಯ *resp, ಶೂನ್ಯ *ವಾದಗಳು);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

ಶೂನ್ಯ *
ffi_prep_ವಾದಗಳು (ಶೂನ್ಯ *ರಾಶಿ, extended_cif *ecif)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಶೂನ್ಯ **p_argv;
  ಅಕ್ಷರ *argp;
  ffi_type **p_arg;
  ಶೂನ್ಯ *struct_ಮೌಲ್ಯ_ptr;

  argp = ರಾಶಿ;

  ಆದರೆ (
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#ಆದರೆ_ಅಂತ್ಯ
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_ಮೌಲ್ಯ_ptr = ecif->rvalue;
  ಇಲ್ಲದಿದ್ದರೆ
    struct_ಮೌಲ್ಯ_ptr = ಖಾಲಿ;

  p_argv = ecif->avalue;

  ಗಾಗಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->ಗಾತ್ರ_2;
      ಪೂರ್ಣಾಂಕ type = (*p_arg)->type;

      ಆದರೆ (z < ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	{
	  ಆಯ್ಕೆ (type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	      *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ) *(SINT8 *) *p_argv;
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) *(UINT8 *) *p_argv;
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	      *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ) *(SINT16 *) *p_argv;
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) *(UINT16 *) *p_argv;
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MINT__
	      ಆದರೆ (z == 1 || z == 2)
		ಸ್ಮರಣೆ_ನಕಲು (argp + 2, *p_argv, z);
              ಇಲ್ಲದಿದ್ದರೆ
		ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
#ಇಲ್ಲದಿದ್ದರೆ
	      ಸ್ಮರಣೆ_ನಕಲು (argp + ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - z, *p_argv, z);
#ಆದರೆ_ಅಂತ್ಯ
	      ನಿಲ್ಲಿಸು;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      FFI_ASSERT (0);
	    }
	  z = ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ಆದರೆ ((ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) - 1) & z)
	    z = FFI_ALIGN(z, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
	}

      p_argv++;
      argp += z;
    }

  ಹಿಂತಿರುಗಿ struct_ಮೌಲ್ಯ_ptr;
}

#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_INT		1
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_DINT		2
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_FLOAT		4
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_DOUBLE	8
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_LDOUBLE	16
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_ಸೂಚಕ	32
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_STRUCT1	64
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_STRUCT2	128
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_SINT8		256
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_SINT16	512

/* Perform machine dependent cif processing */
ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      cif->flags = 0;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಆದರೆ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          ನಿಲ್ಲಿಸು;
        }

      ಆಯ್ಕೆ (cif->rtype->ಗಾತ್ರ_2)
	{
	ಸ್ಥಿತಿ 1:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#ಇಲ್ಲದಿದ್ದರೆ
	  cif->flags = CIF_FLAGS_STRUCT1;
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MINT__
	ಸ್ಥಿತಿ 3:
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ 4:
	  cif->flags = CIF_FLAGS_INT;
	  ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MINT__
	ಸ್ಥಿತಿ 7:
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ 8:
	  cif->flags = CIF_FLAGS_DINT;
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  cif->flags = 0;
	  ನಿಲ್ಲಿಸು;
	}
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      ನಿಲ್ಲಿಸು;

#ಆದರೆ (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MINT__
      cif->flags = 0;
#ಇಲ್ಲದಿದ್ದರೆ
      cif->flags = CIF_FLAGS_LDOUBLE;
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      cif->flags = CIF_FLAGS_ಸೂಚಕ;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      cif->flags = CIF_FLAGS_INT;
      ನಿಲ್ಲಿಸು;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ಆದರೆ (rvalue == ಖಾಲಿ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->ಗಾತ್ರ_2 > 8)
    ecif.rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->ಬೈಟುಗಳು, cif->flags,
		     ecif.rvalue, fn);
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT (0);
      ನಿಲ್ಲಿಸು;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_incoming_ವಾದಗಳು_SYSV (ಅಕ್ಷರ *ರಾಶಿ, ಶೂನ್ಯ **avalue, ffi_cif *cif)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಶೂನ್ಯ **p_argv;
  ಅಕ್ಷರ *argp;
  ffi_type **p_arg;

  argp = ರಾಶಿ;
  p_argv = avalue;

  ಗಾಗಿ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ಗಾತ್ರ_2;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MINT__
      ಆದರೆ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (ಶೂನ್ಯ *) (argp + 2);

	  z = 4;
	}
      ಇಲ್ಲದಿದ್ದರೆ
      ಆದರೆ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (ಶೂನ್ಯ *) (argp);

	  z = 4;
	}
      ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ (z <= 4)
	{
	  *p_argv = (ಶೂನ್ಯ *) (argp + 4 - z);

	  z = 4;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  *p_argv = (ಶೂನ್ಯ *) argp;

	  /* Align if necessary */
	  ಆದರೆ ((ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) - 1) & z)
	    z = FFI_ALIGN(z, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
	}

      p_argv++;
      argp += z;
    }
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_SYSV_inner (ffi_ಆವರಣ *ಆವರಣ, ಶೂನ್ಯ *resp, ಶೂನ್ಯ *ವಾದಗಳು)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **arg_area;

  cif = ಆವರಣ->cif;
  arg_area = (ಶೂನ್ಯ**) alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));

  ffi_prep_incoming_ವಾದಗಳು_SYSV(ವಾದಗಳು, arg_area, cif);

  (ಆವರಣ->fun) (cif, resp, arg_area, ಆವರಣ->user_ದತ್ತಾಂಶ);

  ಹಿಂತಿರುಗಿ cif->flags;
}

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಆದರೆ (cif->abi != FFI_SYSV)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  *(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *)ಆವರಣ->tramp = 0x207c;
  *(ಶೂನ್ಯ **)(ಆವರಣ->tramp + 2) = codeloc;
  *(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *)(ಆವರಣ->tramp + 6) = 0x4ef9;

  ಆದರೆ (
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#ಆದರೆ_ಅಂತ್ಯ
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(ಶೂನ್ಯ **)(ಆವರಣ->tramp + 8) = ffi_ಆವರಣ_struct_SYSV;
  ಇಲ್ಲದಿದ್ದರೆ
    *(ಶೂನ್ಯ **)(ಆವರಣ->tramp + 8) = ffi_ಆವರಣ_SYSV;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __rtems__
  rtems_cache_flush_ಅಪವರ್ತ್ಯ_ದತ್ತಾಂಶ_ಸಾಲುಗಳು( codeloc, FFI_TRAMPOLINE_ಗಾತ್ರ );
#ಅಥವಾಆದರೆ defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_ಗಾತ್ರ);
#ಇಲ್ಲದಿದ್ದರೆ
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_ಸಾಲು,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_ಗಾತ್ರ);
#ಆದರೆ_ಅಂತ್ಯ

  ಆವರಣ->cif  = cif;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
  ಆವರಣ->fun  = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}
