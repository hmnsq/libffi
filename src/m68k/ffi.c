/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ifdef __rtems__
ଶୂନ୍ୟ rtems_cache_flush_multiple_data_ଧାଡ଼ିଗୁଡ଼ିକ( ଅଚଳ ଶୂନ୍ୟ *, size_t );
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ <sys/syscall.h>
#ifdef __MINT__
#ଅନ୍ତର୍ଭାଵ <mint/mintbind.h>
#ଅନ୍ତର୍ଭାଵ <mint/ssystem.h>
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ <asm/cachectl.h>
#endif
#endif

ଶୂନ୍ୟ ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    ଶୂନ୍ୟ *, ଶୂନ୍ୟ (*fn) ());
ଶୂନ୍ୟ *ffi_prep_args (ଶୂନ୍ୟ *ଗଦା, extended_cif *ecif);
ଶୂନ୍ୟ ffi_closure_SYSV (ffi_closure *);
ଶୂନ୍ୟ ffi_closure_struct_SYSV (ffi_closure *);
unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_closure_SYSV_inner (ffi_closure *closure,
				     ଶୂନ୍ୟ *resp, ଶୂନ୍ୟ *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

ଶୂନ୍ୟ *
ffi_prep_args (ଶୂନ୍ୟ *ଗଦା, extended_cif *ecif)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଶୂନ୍ୟ **p_argv;
  ଅକ୍ଷର *argp;
  ffi_type **p_arg;
  ଶୂନ୍ୟ *struct_ମୂଲ୍ୟ_ptr;

  argp = ଗଦା;

  ଯଦି (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_ମୂଲ୍ୟ_ptr = ecif->rvalue;
  ଅନ୍ୟଥା
    struct_ମୂଲ୍ୟ_ptr = ଖାଲି;

  p_argv = ecif->avalue;

  ପାଇଁ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା type = (*p_arg)->type;

      ଯଦି (z < ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  ବାଛ (type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(SINT8 *) *p_argv;
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_UINT8:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(UINT8 *) *p_argv;
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_SINT16:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(SINT16 *) *p_argv;
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_UINT16:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(UINT16 *) *p_argv;
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_STRUCT:
#ifdef __MINT__
	      ଯଦି (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              ଅନ୍ୟଥା
		memcpy (argp, *p_argv, z);
#ଅନ୍ୟଥା
	      memcpy (argp + ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - z, *p_argv, z);
#endif
	      ଭାଙ୍ଗ;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT (0);
	    }
	  z = ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	}
      ଅନ୍ୟଥା
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ଯଦି ((ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) & z)
	    z = FFI_ALIGN(z, ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
	}

      p_argv++;
      argp += z;
    }

  ଫେରନ୍ତୁ struct_ମୂଲ୍ୟ_ptr;
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
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      cif->flags = 0;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
      ଯଦି (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          ଭାଙ୍ଗ;
        }

      ବାଛ (cif->rtype->size)
	{
	ସ୍ଥିତି 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#ଅନ୍ୟଥା
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  ଭାଙ୍ଗ;
#ifdef __MINT__
	ସ୍ଥିତି 3:
#endif
	ସ୍ଥିତି 4:
	  cif->flags = CIF_FLAGS_INT;
	  ଭାଙ୍ଗ;
#ifdef __MINT__
	ସ୍ଥିତି 7:
#endif
	ସ୍ଥିତି 8:
	  cif->flags = CIF_FLAGS_DINT;
	  ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  cif->flags = 0;
	  ଭାଙ୍ଗ;
	}
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      ଭାଙ୍ଗ;

#ଯଦି (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#ଅନ୍ୟଥା
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      ଭାଙ୍ଗ;
#endif

    ସ୍ଥିତି FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      cif->flags = CIF_FLAGS_INT;
      ଭାଙ୍ଗ;
    }

  ଫେରନ୍ତୁ FFI_OK;
}

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ଯଦି (rvalue == ଖାଲି
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT (0);
      ଭାଙ୍ଗ;
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
ffi_prep_incoming_args_SYSV (ଅକ୍ଷର *ଗଦା, ଶୂନ୍ୟ **avalue, ffi_cif *cif)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଶୂନ୍ୟ **p_argv;
  ଅକ୍ଷର *argp;
  ffi_type **p_arg;

  argp = ଗଦା;
  p_argv = avalue;

  ପାଇଁ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
#ifdef __MINT__
      ଯଦି (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (ଶୂନ୍ୟ *) (argp + 2);

	  z = 4;
	}
      ଅନ୍ୟଥା
      ଯଦି (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (ଶୂନ୍ୟ *) (argp);

	  z = 4;
	}
      ଅନ୍ୟଥା
#endif
      ଯଦି (z <= 4)
	{
	  *p_argv = (ଶୂନ୍ୟ *) (argp + 4 - z);

	  z = 4;
	}
      ଅନ୍ୟଥା
	{
	  *p_argv = (ଶୂନ୍ୟ *) argp;

	  /* Align if necessary */
	  ଯଦି ((ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) & z)
	    z = FFI_ALIGN(z, ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
	}

      p_argv++;
      argp += z;
    }
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_SYSV_inner (ffi_closure *closure, ଶୂନ୍ୟ *resp, ଶୂନ୍ୟ *args)
{
  ffi_cif *cif;
  ଶୂନ୍ୟ **arg_area;

  cif = closure->cif;
  arg_area = (ଶୂନ୍ୟ**) alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  ଫେରନ୍ତୁ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  ଯଦି (cif->abi != FFI_SYSV)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(ଶୂନ୍ୟ **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  ଯଦି (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(ଶୂନ୍ୟ **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  ଅନ୍ୟଥା
    *(ଶୂନ୍ୟ **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_ଧାଡ଼ିଗୁଡ଼ିକ( codeloc, FFI_TRAMPOLINE_SIZE );
#ନଚେତଯଦି defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#ଅନ୍ୟଥା
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_ଧାଡ଼ି,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ଫେରନ୍ତୁ FFI_OK;
}
