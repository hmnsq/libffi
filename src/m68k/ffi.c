/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <unistd.h>
#ifdef __rtems__
فراغ rtems_cache_flush_multiple_data_lines( ثابت فراغ *, size_t );
#אחרת
#ضمن <sys/syscall.h>
#ifdef __MINT__
#ضمن <mint/mintbind.h>
#ضمن <mint/ssystem.h>
#אחרת
#ضمن <asm/cachectl.h>
#endif
#endif

فراغ ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    فراغ *, فراغ (*fn) ());
فراغ *ffi_prep_args (فراغ *stack, extended_cif *ecif);
فراغ ffi_closure_SYSV (ffi_closure *);
فراغ ffi_closure_struct_SYSV (ffi_closure *);
unsigned صحيح ffi_closure_SYSV_inner (ffi_closure *closure,
				     فراغ *resp, فراغ *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

فراغ *
ffi_prep_args (فراغ *stack, extended_cif *ecif)
{
  unsigned صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;
  فراغ *struct_value_ptr;

  argp = stack;

  אם (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_value_ptr = ecif->rvalue;
  אחרת
    struct_value_ptr = عدم;

  p_argv = ecif->avalue;

  עבור (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      صحيح type = (*p_arg)->type;

      אם (z < حجم (صحيح))
	{
	  بدل (type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(signed صحيح *) argp = (signed صحيح) *(SINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(unsigned صحيح *) argp = (unsigned صحيح) *(UINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(signed صحيح *) argp = (signed صحيح) *(SINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(unsigned صحيح *) argp = (unsigned صحيح) *(UINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_STRUCT:
#ifdef __MINT__
	      אם (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              אחרת
		memcpy (argp, *p_argv, z);
#אחרת
	      memcpy (argp + حجم (صحيح) - z, *p_argv, z);
#endif
	      اكسر;

	    افتراضي:
	      FFI_ASSERT (0);
	    }
	  z = حجم (صحيح);
	}
      אחרת
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  אם ((حجم(صحيح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحيح));
	}

      p_argv++;
      argp += z;
    }

  החזר struct_value_ptr;
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
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      cif->flags = 0;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      אם (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          اكسر;
        }

      بدل (cif->rtype->size)
	{
	حالة 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#אחרת
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  اكسر;
	حالة 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  اكسر;
#ifdef __MINT__
	حالة 3:
#endif
	حالة 4:
	  cif->flags = CIF_FLAGS_INT;
	  اكسر;
#ifdef __MINT__
	حالة 7:
#endif
	حالة 8:
	  cif->flags = CIF_FLAGS_DINT;
	  اكسر;
	افتراضي:
	  cif->flags = 0;
	  اكسر;
	}
      اكسر;

    حالة FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      اكسر;

    حالة FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      اكسر;

#אם (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    حالة FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#אחרת
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      اكسر;
#endif

    حالة FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      اكسر;

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      اكسر;

    حالة FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      اكسر;

    حالة FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      اكسر;

    افتراضي:
      cif->flags = CIF_FLAGS_INT;
      اكسر;
    }

  החזר FFI_OK;
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  אם (rvalue == عدم
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  אחרת
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      اكسر;

    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

ساكن فراغ
ffi_prep_incoming_args_SYSV (حرف *stack, فراغ **avalue, ffi_cif *cif)
{
  unsigned صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  argp = stack;
  p_argv = avalue;

  עבור (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
#ifdef __MINT__
      אם (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (فراغ *) (argp + 2);

	  z = 4;
	}
      אחרת
      אם (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (فراغ *) (argp);

	  z = 4;
	}
      אחרת
#endif
      אם (z <= 4)
	{
	  *p_argv = (فراغ *) (argp + 4 - z);

	  z = 4;
	}
      אחרת
	{
	  *p_argv = (فراغ *) argp;

	  /* Align if necessary */
	  אם ((حجم(صحيح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحيح));
	}

      p_argv++;
      argp += z;
    }
}

unsigned صحيح
ffi_closure_SYSV_inner (ffi_closure *closure, فراغ *resp, فراغ *args)
{
  ffi_cif *cif;
  فراغ **arg_area;

  cif = closure->cif;
  arg_area = (فراغ**) alloca (cif->nargs * حجم (فراغ *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  החזר cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		      فراغ *user_data,
		      فراغ *codeloc)
{
  אם (cif->abi != FFI_SYSV)
    החזר FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(فراغ **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  אם (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(فراغ **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  אחרת
    *(فراغ **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_lines( codeloc, FFI_TRAMPOLINE_SIZE );
#وإلاإذا defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#אחרת
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_LINE,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  החזר FFI_OK;
}
