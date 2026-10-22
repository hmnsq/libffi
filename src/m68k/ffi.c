/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <unistd.h>
#ifdef __rtems__
શૂન્ય rtems_cache_flush_multiple_data_લીટીઓ( અચલ શૂન્ય *, size_t );
#નહીંતર
#અન્તર્ભાવ <sys/syscall.h>
#ifdef __MINT__
#અન્તર્ભાવ <mint/mintbind.h>
#અન્તર્ભાવ <mint/ssystem.h>
#નહીંતર
#અન્તર્ભાવ <asm/cachectl.h>
#endif
#endif

શૂન્ય ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    શૂન્ય *, શૂન્ય (*fn) ());
શૂન્ય *ffi_prep_args (શૂન્ય *ઢગલો, extended_cif *ecif);
શૂન્ય ffi_closure_SYSV (ffi_closure *);
શૂન્ય ffi_closure_struct_SYSV (ffi_closure *);
unsigned પૂર્ણાંક ffi_closure_SYSV_inner (ffi_closure *closure,
				     શૂન્ય *resp, શૂન્ય *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

શૂન્ય *
ffi_prep_args (શૂન્ય *ઢગલો, extended_cif *ecif)
{
  unsigned પૂર્ણાંક i;
  શૂન્ય **p_argv;
  અક્ષર *argp;
  ffi_type **p_arg;
  શૂન્ય *struct_કિંમત_ptr;

  argp = ઢગલો;

  જો (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_કિંમત_ptr = ecif->rvalue;
  નહીંતર
    struct_કિંમત_ptr = ખાલી;

  p_argv = ecif->avalue;

  માટે (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      પૂર્ણાંક type = (*p_arg)->type;

      જો (z < કદ (પૂર્ણાંક))
	{
	  પસંદગી (type)
	    {
	    સ્થિતિ FFI_TYPE_SINT8:
	      *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક) *(SINT8 *) *p_argv;
	      તોડો;

	    સ્થિતિ FFI_TYPE_UINT8:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક) *(UINT8 *) *p_argv;
	      તોડો;

	    સ્થિતિ FFI_TYPE_SINT16:
	      *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક) *(SINT16 *) *p_argv;
	      તોડો;

	    સ્થિતિ FFI_TYPE_UINT16:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક) *(UINT16 *) *p_argv;
	      તોડો;

	    સ્થિતિ FFI_TYPE_STRUCT:
#ifdef __MINT__
	      જો (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              નહીંતર
		memcpy (argp, *p_argv, z);
#નહીંતર
	      memcpy (argp + કદ (પૂર્ણાંક) - z, *p_argv, z);
#endif
	      તોડો;

	    મૂળભૂત:
	      FFI_ASSERT (0);
	    }
	  z = કદ (પૂર્ણાંક);
	}
      નહીંતર
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  જો ((કદ(પૂર્ણાંક) - 1) & z)
	    z = FFI_ALIGN(z, કદ(પૂર્ણાંક));
	}

      p_argv++;
      argp += z;
    }

  પાછા struct_કિંમત_ptr;
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
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      cif->flags = 0;
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
      જો (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          તોડો;
        }

      પસંદગી (cif->rtype->size)
	{
	સ્થિતિ 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#નહીંતર
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  તોડો;
	સ્થિતિ 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  તોડો;
#ifdef __MINT__
	સ્થિતિ 3:
#endif
	સ્થિતિ 4:
	  cif->flags = CIF_FLAGS_INT;
	  તોડો;
#ifdef __MINT__
	સ્થિતિ 7:
#endif
	સ્થિતિ 8:
	  cif->flags = CIF_FLAGS_DINT;
	  તોડો;
	મૂળભૂત:
	  cif->flags = 0;
	  તોડો;
	}
      તોડો;

    સ્થિતિ FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      તોડો;

    સ્થિતિ FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      તોડો;

#જો (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#નહીંતર
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      તોડો;
#endif

    સ્થિતિ FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      તોડો;

    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      તોડો;

    સ્થિતિ FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      તોડો;

    સ્થિતિ FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      તોડો;

    મૂળભૂત:
      cif->flags = CIF_FLAGS_INT;
      તોડો;
    }

  પાછા FFI_OK;
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn) (), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  જો (rvalue == ખાલી
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  નહીંતર
    ecif.rvalue = rvalue;

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      તોડો;

    મૂળભૂત:
      FFI_ASSERT (0);
      તોડો;
    }
}

સ્થિર શૂન્ય
ffi_prep_incoming_args_SYSV (અક્ષર *ઢગલો, શૂન્ય **avalue, ffi_cif *cif)
{
  unsigned પૂર્ણાંક i;
  શૂન્ય **p_argv;
  અક્ષર *argp;
  ffi_type **p_arg;

  argp = ઢગલો;
  p_argv = avalue;

  માટે (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
#ifdef __MINT__
      જો (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (શૂન્ય *) (argp + 2);

	  z = 4;
	}
      નહીંતર
      જો (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (શૂન્ય *) (argp);

	  z = 4;
	}
      નહીંતર
#endif
      જો (z <= 4)
	{
	  *p_argv = (શૂન્ય *) (argp + 4 - z);

	  z = 4;
	}
      નહીંતર
	{
	  *p_argv = (શૂન્ય *) argp;

	  /* Align if necessary */
	  જો ((કદ(પૂર્ણાંક) - 1) & z)
	    z = FFI_ALIGN(z, કદ(પૂર્ણાંક));
	}

      p_argv++;
      argp += z;
    }
}

unsigned પૂર્ણાંક
ffi_closure_SYSV_inner (ffi_closure *closure, શૂન્ય *resp, શૂન્ય *args)
{
  ffi_cif *cif;
  શૂન્ય **arg_area;

  cif = closure->cif;
  arg_area = (શૂન્ય**) alloca (cif->nargs * કદ (શૂન્ય *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  પાછા cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  જો (cif->abi != FFI_SYSV)
    પાછા FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(શૂન્ય **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  જો (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(શૂન્ય **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  નહીંતર
    *(શૂન્ય **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_લીટીઓ( codeloc, FFI_TRAMPOLINE_SIZE );
#અથવાજો defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#નહીંતર
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_લીટી,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  પાછા FFI_OK;
}
