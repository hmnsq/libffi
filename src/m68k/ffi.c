/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __rtems__
ਖਾਲੀ rtems_cache_flush_ਗੁਣਜ_ਅੰਕੜੇ_ਸਤਰਾਂ( ਅਚਲ ਖਾਲੀ *, size_t );
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/syscall.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MINT__
#ਅਨ੍ਤਰ੍ਭਾਵ <mint/mintbind.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mint/ssystem.h>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/cachectl.h>
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

ਖਾਲੀ ffi_call_SYSV (extended_cif *,
		    ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ,
		    ਖਾਲੀ *, ਖਾਲੀ (*fn) ());
ਖਾਲੀ *ffi_prep_ਦਲੀਲਾਂ (ਖਾਲੀ *ਢੇਰ, extended_cif *ecif);
ਖਾਲੀ ffi_ਘੇਰਾ_SYSV (ffi_ਘੇਰਾ *);
ਖਾਲੀ ffi_ਘੇਰਾ_struct_SYSV (ffi_ਘੇਰਾ *);
ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ffi_ਘੇਰਾ_SYSV_inner (ffi_ਘੇਰਾ *ਘੇਰਾ,
				     ਖਾਲੀ *resp, ਖਾਲੀ *ਦਲੀਲਾਂ);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

ਖਾਲੀ *
ffi_prep_ਦਲੀਲਾਂ (ਖਾਲੀ *ਢੇਰ, extended_cif *ecif)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਖਾਲੀ **p_argv;
  ਅੱਖਰ *argp;
  ffi_type **p_arg;
  ਖਾਲੀ *struct_ਮੁੱਲ_ptr;

  argp = ਢੇਰ;

  ਜੇ (
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#ਜੇ_ਅੰਤ
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_ਮੁੱਲ_ptr = ecif->rvalue;
  ਨਹੀਂਤਾਂ
    struct_ਮੁੱਲ_ptr = ਨਿਰਾ;

  p_argv = ecif->avalue;

  ਲਈ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->ਮਾਪ;
      ਪੂਰਨ type = (*p_arg)->type;

      ਜੇ (z < ਆਕਾਰ (ਪੂਰਨ))
	{
	  ਚੋਣ (type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT8:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ) *(SINT8 *) *p_argv;
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_UINT8:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) *(UINT8 *) *p_argv;
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_SINT16:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ) *(SINT16 *) *p_argv;
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_UINT16:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) *(UINT16 *) *p_argv;
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_STRUCT:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MINT__
	      ਜੇ (z == 1 || z == 2)
		ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp + 2, *p_argv, z);
              ਨਹੀਂਤਾਂ
		ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
#ਨਹੀਂਤਾਂ
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp + ਆਕਾਰ (ਪੂਰਨ) - z, *p_argv, z);
#ਜੇ_ਅੰਤ
	      ਰੋਕੋ;

	    ਮੂਲ:
	      FFI_ASSERT (0);
	    }
	  z = ਆਕਾਰ (ਪੂਰਨ);
	}
      ਨਹੀਂਤਾਂ
	{
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ਜੇ ((ਆਕਾਰ(ਪੂਰਨ) - 1) & z)
	    z = FFI_ALIGN(z, ਆਕਾਰ(ਪੂਰਨ));
	}

      p_argv++;
      argp += z;
    }

  ਵਾਪਸ struct_ਮੁੱਲ_ptr;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_INT		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_DINT		2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_FLOAT		4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_DOUBLE	8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_LDOUBLE	16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_ਸੂਚਕ	32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_STRUCT1	64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_STRUCT2	128
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_SINT8		256
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_SINT16	512

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      cif->flags = 0;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਜੇ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          ਰੋਕੋ;
        }

      ਚੋਣ (cif->rtype->ਮਾਪ)
	{
	ਹਾਲਤ 1:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#ਨਹੀਂਤਾਂ
	  cif->flags = CIF_FLAGS_STRUCT1;
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;
	ਹਾਲਤ 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  ਰੋਕੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MINT__
	ਹਾਲਤ 3:
#ਜੇ_ਅੰਤ
	ਹਾਲਤ 4:
	  cif->flags = CIF_FLAGS_INT;
	  ਰੋਕੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MINT__
	ਹਾਲਤ 7:
#ਜੇ_ਅੰਤ
	ਹਾਲਤ 8:
	  cif->flags = CIF_FLAGS_DINT;
	  ਰੋਕੋ;
	ਮੂਲ:
	  cif->flags = 0;
	  ਰੋਕੋ;
	}
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      ਰੋਕੋ;

#ਜੇ (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MINT__
      cif->flags = 0;
#ਨਹੀਂਤਾਂ
      cif->flags = CIF_FLAGS_LDOUBLE;
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ

    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      cif->flags = CIF_FLAGS_ਸੂਚਕ;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      ਰੋਕੋ;

    ਮੂਲ:
      cif->flags = CIF_FLAGS_INT;
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn) (), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ਜੇ (rvalue == ਨਿਰਾ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->ਮਾਪ > 8)
    ecif.rvalue = alloca (cif->rtype->ਮਾਪ);
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->ਬਾਈਟਾਂ, cif->flags,
		     ecif.rvalue, fn);
      ਰੋਕੋ;

    ਮੂਲ:
      FFI_ASSERT (0);
      ਰੋਕੋ;
    }
}

ਸਥਿਰ ਖਾਲੀ
ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV (ਅੱਖਰ *ਢੇਰ, ਖਾਲੀ **avalue, ffi_cif *cif)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਖਾਲੀ **p_argv;
  ਅੱਖਰ *argp;
  ffi_type **p_arg;

  argp = ਢੇਰ;
  p_argv = avalue;

  ਲਈ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ਮਾਪ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MINT__
      ਜੇ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (ਖਾਲੀ *) (argp + 2);

	  z = 4;
	}
      ਨਹੀਂਤਾਂ
      ਜੇ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (ਖਾਲੀ *) (argp);

	  z = 4;
	}
      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
      ਜੇ (z <= 4)
	{
	  *p_argv = (ਖਾਲੀ *) (argp + 4 - z);

	  z = 4;
	}
      ਨਹੀਂਤਾਂ
	{
	  *p_argv = (ਖਾਲੀ *) argp;

	  /* Align if necessary */
	  ਜੇ ((ਆਕਾਰ(ਪੂਰਨ) - 1) & z)
	    z = FFI_ALIGN(z, ਆਕਾਰ(ਪੂਰਨ));
	}

      p_argv++;
      argp += z;
    }
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
ffi_ਘੇਰਾ_SYSV_inner (ffi_ਘੇਰਾ *ਘੇਰਾ, ਖਾਲੀ *resp, ਖਾਲੀ *ਦਲੀਲਾਂ)
{
  ffi_cif *cif;
  ਖਾਲੀ **arg_area;

  cif = ਘੇਰਾ->cif;
  arg_area = (ਖਾਲੀ**) alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));

  ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV(ਦਲੀਲਾਂ, arg_area, cif);

  (ਘੇਰਾ->fun) (cif, resp, arg_area, ਘੇਰਾ->user_ਅੰਕੜੇ);

  ਵਾਪਸ cif->flags;
}

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਜੇ (cif->abi != FFI_SYSV)
    ਵਾਪਸ FFI_BAD_ABI;

  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *)ਘੇਰਾ->tramp = 0x207c;
  *(ਖਾਲੀ **)(ਘੇਰਾ->tramp + 2) = codeloc;
  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *)(ਘੇਰਾ->tramp + 6) = 0x4ef9;

  ਜੇ (
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#ਜੇ_ਅੰਤ
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(ਖਾਲੀ **)(ਘੇਰਾ->tramp + 8) = ffi_ਘੇਰਾ_struct_SYSV;
  ਨਹੀਂਤਾਂ
    *(ਖਾਲੀ **)(ਘੇਰਾ->tramp + 8) = ffi_ਘੇਰਾ_SYSV;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __rtems__
  rtems_cache_flush_ਗੁਣਜ_ਅੰਕੜੇ_ਸਤਰਾਂ( codeloc, FFI_TRAMPOLINE_ਮਾਪ );
#ਜਾਂਜੇ defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_ਮਾਪ);
#ਨਹੀਂਤਾਂ
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_ਸਤਰ,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_ਮਾਪ);
#ਜੇ_ਅੰਤ

  ਘੇਰਾ->cif  = cif;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  ਘੇਰਾ->fun  = fun;

  ਵਾਪਸ FFI_OK;
}
