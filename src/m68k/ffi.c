/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <unistd.h>
#ifdef __rtems__
ശൂന്യം rtems_cache_flush_multiple_data_വരികൾ( സ്ഥിരം ശൂന്യം *, size_t );
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ <sys/syscall.h>
#ifdef __MINT__
#അന്തര്ഭാവ <mint/mintbind.h>
#അന്തര്ഭാവ <mint/ssystem.h>
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ <asm/cachectl.h>
#endif
#endif

ശൂന്യം ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    ശൂന്യം *, ശൂന്യം (*fn) ());
ശൂന്യം *ffi_prep_args (ശൂന്യം *അട്ടി, extended_cif *ecif);
ശൂന്യം ffi_closure_SYSV (ffi_closure *);
ശൂന്യം ffi_closure_struct_SYSV (ffi_closure *);
unsigned പൂർണ്ണസംഖ്യ ffi_closure_SYSV_inner (ffi_closure *closure,
				     ശൂന്യം *resp, ശൂന്യം *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

ശൂന്യം *
ffi_prep_args (ശൂന്യം *അട്ടി, extended_cif *ecif)
{
  unsigned പൂർണ്ണസംഖ്യ i;
  ശൂന്യം **p_argv;
  അക്ഷരം *argp;
  ffi_type **p_arg;
  ശൂന്യം *struct_മൂല്യം_ptr;

  argp = അട്ടി;

  എങ്കിൽ (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_മൂല്യം_ptr = ecif->rvalue;
  അല്ലെങ്കിൽ
    struct_മൂല്യം_ptr = ശൂന്യ;

  p_argv = ecif->avalue;

  വേണ്ടി (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      പൂർണ്ണസംഖ്യ type = (*p_arg)->type;

      എങ്കിൽ (z < വലുപ്പം (പൂർണ്ണസംഖ്യ))
	{
	  തിരഞ്ഞെടുപ്പ് (type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT8:
	      *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ) *(SINT8 *) *p_argv;
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_UINT8:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ) *(UINT8 *) *p_argv;
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_SINT16:
	      *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ) *(SINT16 *) *p_argv;
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_UINT16:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ) *(UINT16 *) *p_argv;
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_STRUCT:
#ifdef __MINT__
	      എങ്കിൽ (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              അല്ലെങ്കിൽ
		memcpy (argp, *p_argv, z);
#അല്ലെങ്കിൽ
	      memcpy (argp + വലുപ്പം (പൂർണ്ണസംഖ്യ) - z, *p_argv, z);
#endif
	      നിർത്തുക;

	    സ്ഥിരസ്ഥിതി:
	      FFI_ASSERT (0);
	    }
	  z = വലുപ്പം (പൂർണ്ണസംഖ്യ);
	}
      അല്ലെങ്കിൽ
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  എങ്കിൽ ((വലുപ്പം(പൂർണ്ണസംഖ്യ) - 1) & z)
	    z = FFI_ALIGN(z, വലുപ്പം(പൂർണ്ണസംഖ്യ));
	}

      p_argv++;
      argp += z;
    }

  തിരികെ struct_മൂല്യം_ptr;
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
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      cif->flags = 0;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
      എങ്കിൽ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          നിർത്തുക;
        }

      തിരഞ്ഞെടുപ്പ് (cif->rtype->size)
	{
	അവസ്ഥ 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#അല്ലെങ്കിൽ
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  നിർത്തുക;
	അവസ്ഥ 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  നിർത്തുക;
#ifdef __MINT__
	അവസ്ഥ 3:
#endif
	അവസ്ഥ 4:
	  cif->flags = CIF_FLAGS_INT;
	  നിർത്തുക;
#ifdef __MINT__
	അവസ്ഥ 7:
#endif
	അവസ്ഥ 8:
	  cif->flags = CIF_FLAGS_DINT;
	  നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
	  cif->flags = 0;
	  നിർത്തുക;
	}
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      നിർത്തുക;

#എങ്കിൽ (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#അല്ലെങ്കിൽ
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      നിർത്തുക;
#endif

    അവസ്ഥ FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      cif->flags = CIF_FLAGS_INT;
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn) (), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  എങ്കിൽ (rvalue == ശൂന്യ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT (0);
      നിർത്തുക;
    }
}

സ്ഥിതം ശൂന്യം
ffi_prep_incoming_args_SYSV (അക്ഷരം *അട്ടി, ശൂന്യം **avalue, ffi_cif *cif)
{
  unsigned പൂർണ്ണസംഖ്യ i;
  ശൂന്യം **p_argv;
  അക്ഷരം *argp;
  ffi_type **p_arg;

  argp = അട്ടി;
  p_argv = avalue;

  വേണ്ടി (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
#ifdef __MINT__
      എങ്കിൽ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (ശൂന്യം *) (argp + 2);

	  z = 4;
	}
      അല്ലെങ്കിൽ
      എങ്കിൽ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (ശൂന്യം *) (argp);

	  z = 4;
	}
      അല്ലെങ്കിൽ
#endif
      എങ്കിൽ (z <= 4)
	{
	  *p_argv = (ശൂന്യം *) (argp + 4 - z);

	  z = 4;
	}
      അല്ലെങ്കിൽ
	{
	  *p_argv = (ശൂന്യം *) argp;

	  /* Align if necessary */
	  എങ്കിൽ ((വലുപ്പം(പൂർണ്ണസംഖ്യ) - 1) & z)
	    z = FFI_ALIGN(z, വലുപ്പം(പൂർണ്ണസംഖ്യ));
	}

      p_argv++;
      argp += z;
    }
}

unsigned പൂർണ്ണസംഖ്യ
ffi_closure_SYSV_inner (ffi_closure *closure, ശൂന്യം *resp, ശൂന്യം *args)
{
  ffi_cif *cif;
  ശൂന്യം **arg_area;

  cif = closure->cif;
  arg_area = (ശൂന്യം**) alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  തിരികെ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  എങ്കിൽ (cif->abi != FFI_SYSV)
    തിരികെ FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(ശൂന്യം **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  എങ്കിൽ (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(ശൂന്യം **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  അല്ലെങ്കിൽ
    *(ശൂന്യം **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_വരികൾ( codeloc, FFI_TRAMPOLINE_SIZE );
#അല്ലെങ്കിലെങ്കിൽ defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#അല്ലെങ്കിൽ
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_വരി,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  തിരികെ FFI_OK;
}
