/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <unistd.h>
#جے_تعریف_شدہ __rtems__
فراغ rtems_cache_flush_مضرب_انکڑے_سطریں( ثابت فراغ *, size_t );
#وإلا
#ضمن <sys/syscall.h>
#جے_تعریف_شدہ __MINT__
#ضمن <mint/mintbind.h>
#ضمن <mint/ssystem.h>
#وإلا
#ضمن <asm/cachectl.h>
#جے_انت
#جے_انت

فراغ ffi_call_SYSV (extended_cif *,
		    بنانشانی, بنانشانی,
		    فراغ *, فراغ (*fn) ());
فراغ *ffi_prep_دلیلاں (فراغ *ڈھیر, extended_cif *ecif);
فراغ ffi_گھیرا_SYSV (ffi_گھیرا *);
فراغ ffi_گھیرا_struct_SYSV (ffi_گھیرا *);
بنانشانی صحيح ffi_گھیرا_SYSV_inner (ffi_گھیرا *گھیرا,
				     فراغ *resp, فراغ *دلیلاں);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

فراغ *
ffi_prep_دلیلاں (فراغ *ڈھیر, extended_cif *ecif)
{
  بنانشانی صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;
  فراغ *struct_قدر_ptr;

  argp = ڈھیر;

  إذا (
#جے_تعریف_شدہ __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#جے_انت
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
      size_t z = (*p_arg)->ناپ;
      صحيح type = (*p_arg)->type;

      إذا (z < حجم (صحيح))
	{
	  بدل (type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح) *(SINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(بنانشانی صحيح *) argp = (بنانشانی صحيح) *(UINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح) *(SINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(بنانشانی صحيح *) argp = (بنانشانی صحيح) *(UINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_STRUCT:
#جے_تعریف_شدہ __MINT__
	      إذا (z == 1 || z == 2)
		حافظہ_نقل (argp + 2, *p_argv, z);
              وإلا
		حافظہ_نقل (argp, *p_argv, z);
#وإلا
	      حافظہ_نقل (argp + حجم (صحيح) - z, *p_argv, z);
#جے_انت
	      اكسر;

	    افتراضي:
	      FFI_ASSERT (0);
	    }
	  z = حجم (صحيح);
	}
      وإلا
	{
	  حافظہ_نقل (argp, *p_argv, z);

	  /* Align if necessary.  */
	  إذا ((حجم(صحيح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحيح));
	}

      p_argv++;
      argp += z;
    }

  أرجع struct_قدر_ptr;
}

#تعریف_کرو CIF_FLAGS_INT		1
#تعریف_کرو CIF_FLAGS_DINT		2
#تعریف_کرو CIF_FLAGS_FLOAT		4
#تعریف_کرو CIF_FLAGS_DOUBLE	8
#تعریف_کرو CIF_FLAGS_LDOUBLE	16
#تعریف_کرو CIF_FLAGS_اشارہ_گر	32
#تعریف_کرو CIF_FLAGS_STRUCT1	64
#تعریف_کرو CIF_FLAGS_STRUCT2	128
#تعریف_کرو CIF_FLAGS_SINT8		256
#تعریف_کرو CIF_FLAGS_SINT16	512

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

      بدل (cif->rtype->ناپ)
	{
	حالة 1:
#جے_تعریف_شدہ __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#وإلا
	  cif->flags = CIF_FLAGS_STRUCT1;
#جے_انت
	  اكسر;
	حالة 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  اكسر;
#جے_تعریف_شدہ __MINT__
	حالة 3:
#جے_انت
	حالة 4:
	  cif->flags = CIF_FLAGS_INT;
	  اكسر;
#جے_تعریف_شدہ __MINT__
	حالة 7:
#جے_انت
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
#جے_تعریف_شدہ __MINT__
      cif->flags = 0;
#وإلا
      cif->flags = CIF_FLAGS_LDOUBLE;
#جے_انت
      اكسر;
#جے_انت

    حالة FFI_TYPE_اشارہ_گر:
      cif->flags = CIF_FLAGS_اشارہ_گر;
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
      && cif->rtype->ناپ > 8)
    ecif.rvalue = alloca (cif->rtype->ناپ);
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->بائٹاں, cif->flags,
		     ecif.rvalue, fn);
      اكسر;

    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

ساكن فراغ
ffi_prep_incoming_دلیلاں_SYSV (حرف *ڈھیر, فراغ **avalue, ffi_cif *cif)
{
  بنانشانی صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  argp = ڈھیر;
  p_argv = avalue;

  لكل (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ناپ;
#جے_تعریف_شدہ __MINT__
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
#جے_انت
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

بنانشانی صحيح
ffi_گھیرا_SYSV_inner (ffi_گھیرا *گھیرا, فراغ *resp, فراغ *دلیلاں)
{
  ffi_cif *cif;
  فراغ **arg_area;

  cif = گھیرا->cif;
  arg_area = (فراغ**) alloca (cif->nargs * حجم (فراغ *));

  ffi_prep_incoming_دلیلاں_SYSV(دلیلاں, arg_area, cif);

  (گھیرا->fun) (cif, resp, arg_area, گھیرا->user_انکڑے);

  أرجع cif->flags;
}

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا* گھیرا,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		      فراغ *user_انکڑے,
		      فراغ *codeloc)
{
  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  *(بنانشانی چھوٹا *)گھیرا->tramp = 0x207c;
  *(فراغ **)(گھیرا->tramp + 2) = codeloc;
  *(بنانشانی چھوٹا *)(گھیرا->tramp + 6) = 0x4ef9;

  إذا (
#جے_تعریف_شدہ __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#جے_انت
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(فراغ **)(گھیرا->tramp + 8) = ffi_گھیرا_struct_SYSV;
  وإلا
    *(فراغ **)(گھیرا->tramp + 8) = ffi_گھیرا_SYSV;

#جے_تعریف_شدہ __rtems__
  rtems_cache_flush_مضرب_انکڑے_سطریں( codeloc, FFI_TRAMPOLINE_ناپ );
#وإلاإذا defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_ناپ);
#وإلا
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_سطر,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_ناپ);
#جے_انت

  گھیرا->cif  = cif;
  گھیرا->user_انکڑے = user_انکڑے;
  گھیرا->fun  = fun;

  أرجع FFI_OK;
}
