/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <unistd.h>
#جيڪڏهن_وصف_ٿيل __rtems__
فراغ rtems_cache_flush_گڻو_معلومات_سٽون( ثابت فراغ *, size_t );
#وإلا
#ضمن <sys/syscall.h>
#جيڪڏهن_وصف_ٿيل __MINT__
#ضمن <mint/mintbind.h>
#ضمن <mint/ssystem.h>
#وإلا
#ضمن <asm/cachectl.h>
#جيڪڏهن_پورو
#جيڪڏهن_پورو

فراغ ffi_call_SYSV (extended_cif *,
		    بنانشان, بنانشان,
		    فراغ *, فراغ (*fn) ());
فراغ *ffi_prep_دليلون (فراغ *ڍير, extended_cif *ecif);
فراغ ffi_گهيرو_SYSV (ffi_گهيرو *);
فراغ ffi_گهيرو_struct_SYSV (ffi_گهيرو *);
بنانشان صحيح ffi_گهيرو_SYSV_inner (ffi_گهيرو *گهيرو,
				     فراغ *resp, فراغ *دليلون);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

فراغ *
ffi_prep_دليلون (فراغ *ڍير, extended_cif *ecif)
{
  بنانشان صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;
  فراغ *struct_قدر_ptr;

  argp = ڍير;

  إذا (
#جيڪڏهن_وصف_ٿيل __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#جيڪڏهن_پورو
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_قدر_ptr = ecif->rvalue;
  وإلا
    struct_قدر_ptr = عدم;

  p_argv = ecif->avalue;

  لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->سائيز;
      صحيح type = (*p_arg)->type;

      إذا (z < حجم (صحيح))
	{
	  بدل (type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(نشانوارو صحيح *) argp = (نشانوارو صحيح) *(SINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(بنانشان صحيح *) argp = (بنانشان صحيح) *(UINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(نشانوارو صحيح *) argp = (نشانوارو صحيح) *(SINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(بنانشان صحيح *) argp = (بنانشان صحيح) *(UINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_STRUCT:
#جيڪڏهن_وصف_ٿيل __MINT__
	      إذا (z == 1 || z == 2)
		يادگيري_نقل (argp + 2, *p_argv, z);
              وإلا
		يادگيري_نقل (argp, *p_argv, z);
#وإلا
	      يادگيري_نقل (argp + حجم (صحيح) - z, *p_argv, z);
#جيڪڏهن_پورو
	      اكسر;

	    افتراضي:
	      FFI_ASSERT (0);
	    }
	  z = حجم (صحيح);
	}
      وإلا
	{
	  يادگيري_نقل (argp, *p_argv, z);

	  /* Align if necessary.  */
	  إذا ((حجم(صحيح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحيح));
	}

      p_argv++;
      argp += z;
    }

  أرجع struct_قدر_ptr;
}

#وصف_ڪريو CIF_FLAGS_INT		1
#وصف_ڪريو CIF_FLAGS_DINT		2
#وصف_ڪريو CIF_FLAGS_FLOAT		4
#وصف_ڪريو CIF_FLAGS_DOUBLE	8
#وصف_ڪريو CIF_FLAGS_LDOUBLE	16
#وصف_ڪريو CIF_FLAGS_اشارو	32
#وصف_ڪريو CIF_FLAGS_STRUCT1	64
#وصف_ڪريو CIF_FLAGS_STRUCT2	128
#وصف_ڪريو CIF_FLAGS_SINT8		256
#وصف_ڪريو CIF_FLAGS_SINT16	512

/* Perform machine dependent cif processing */
ffi_حالت
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      cif->flags = 0;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      إذا (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          اكسر;
        }

      بدل (cif->rtype->سائيز)
	{
	حالة 1:
#جيڪڏهن_وصف_ٿيل __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#وإلا
	  cif->flags = CIF_FLAGS_STRUCT1;
#جيڪڏهن_پورو
	  اكسر;
	حالة 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  اكسر;
#جيڪڏهن_وصف_ٿيل __MINT__
	حالة 3:
#جيڪڏهن_پورو
	حالة 4:
	  cif->flags = CIF_FLAGS_INT;
	  اكسر;
#جيڪڏهن_وصف_ٿيل __MINT__
	حالة 7:
#جيڪڏهن_پورو
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

#إذا (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    حالة FFI_TYPE_LONGDOUBLE:
#جيڪڏهن_وصف_ٿيل __MINT__
      cif->flags = 0;
#وإلا
      cif->flags = CIF_FLAGS_LDOUBLE;
#جيڪڏهن_پورو
      اكسر;
#جيڪڏهن_پورو

    حالة FFI_TYPE_اشارو:
      cif->flags = CIF_FLAGS_اشارو;
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

  أرجع FFI_OK;
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  إذا (rvalue == عدم
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->سائيز > 8)
    ecif.rvalue = alloca (cif->rtype->سائيز);
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->بائيٽون, cif->flags,
		     ecif.rvalue, fn);
      اكسر;

    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

ساكن فراغ
ffi_prep_incoming_دليلون_SYSV (حرف *ڍير, فراغ **avalue, ffi_cif *cif)
{
  بنانشان صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  argp = ڍير;
  p_argv = avalue;

  لكل (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->سائيز;
#جيڪڏهن_وصف_ٿيل __MINT__
      إذا (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (فراغ *) (argp + 2);

	  z = 4;
	}
      وإلا
      إذا (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (فراغ *) (argp);

	  z = 4;
	}
      وإلا
#جيڪڏهن_پورو
      إذا (z <= 4)
	{
	  *p_argv = (فراغ *) (argp + 4 - z);

	  z = 4;
	}
      وإلا
	{
	  *p_argv = (فراغ *) argp;

	  /* Align if necessary */
	  إذا ((حجم(صحيح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحيح));
	}

      p_argv++;
      argp += z;
    }
}

بنانشان صحيح
ffi_گهيرو_SYSV_inner (ffi_گهيرو *گهيرو, فراغ *resp, فراغ *دليلون)
{
  ffi_cif *cif;
  فراغ **arg_area;

  cif = گهيرو->cif;
  arg_area = (فراغ**) alloca (cif->nargs * حجم (فراغ *));

  ffi_prep_incoming_دليلون_SYSV(دليلون, arg_area, cif);

  (گهيرو->fun) (cif, resp, arg_area, گهيرو->user_معلومات);

  أرجع cif->flags;
}

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو* گهيرو,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  *(بنانشان ننڍو *)گهيرو->tramp = 0x207c;
  *(فراغ **)(گهيرو->tramp + 2) = codeloc;
  *(بنانشان ننڍو *)(گهيرو->tramp + 6) = 0x4ef9;

  إذا (
#جيڪڏهن_وصف_ٿيل __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#جيڪڏهن_پورو
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(فراغ **)(گهيرو->tramp + 8) = ffi_گهيرو_struct_SYSV;
  وإلا
    *(فراغ **)(گهيرو->tramp + 8) = ffi_گهيرو_SYSV;

#جيڪڏهن_وصف_ٿيل __rtems__
  rtems_cache_flush_گڻو_معلومات_سٽون( codeloc, FFI_TRAMPOLINE_سائيز );
#وإلاإذا defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_سائيز);
#وإلا
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_سٽ,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_سائيز);
#جيڪڏهن_پورو

  گهيرو->cif  = cif;
  گهيرو->user_معلومات = user_معلومات;
  گهيرو->fun  = fun;

  أرجع FFI_OK;
}
