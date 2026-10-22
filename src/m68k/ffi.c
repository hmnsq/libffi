/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <unistd.h>
#اگر_تعریف_شده __rtems__
فراغ rtems_cache_flush_مضرب_داده_سطرها( ثابت فراغ *, size_t );
#وإلا
#ضمن <sys/syscall.h>
#اگر_تعریف_شده __MINT__
#ضمن <mint/mintbind.h>
#ضمن <mint/ssystem.h>
#وإلا
#ضمن <asm/cachectl.h>
#پایان_اگر
#پایان_اگر

فراغ ffi_call_SYSV (extended_cif *,
		    بیعلامت, بیعلامت,
		    فراغ *, فراغ (*fn) ());
فراغ *ffi_prep_آرگومانها (فراغ *پشته, extended_cif *ecif);
فراغ ffi_بستگی_SYSV (ffi_بستگی *);
فراغ ffi_بستگی_struct_SYSV (ffi_بستگی *);
بیعلامت صحيح ffi_بستگی_SYSV_inner (ffi_بستگی *بستگی,
				     فراغ *resp, فراغ *آرگومانها);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

فراغ *
ffi_prep_آرگومانها (فراغ *پشته, extended_cif *ecif)
{
  بیعلامت صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;
  فراغ *struct_مقدار_ptr;

  argp = پشته;

  إذا (
#اگر_تعریف_شده __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#پایان_اگر
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_مقدار_ptr = ecif->rvalue;
  وإلا
    struct_مقدار_ptr = عدم;

  p_argv = ecif->avalue;

  لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->اندازه;
      صحيح type = (*p_arg)->type;

      إذا (z < حجم (صحيح))
	{
	  بدل (type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(علامتدار صحيح *) argp = (علامتدار صحيح) *(SINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(بیعلامت صحيح *) argp = (بیعلامت صحيح) *(UINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(علامتدار صحيح *) argp = (علامتدار صحيح) *(SINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(بیعلامت صحيح *) argp = (بیعلامت صحيح) *(UINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_STRUCT:
#اگر_تعریف_شده __MINT__
	      إذا (z == 1 || z == 2)
		کپی_حافظه (argp + 2, *p_argv, z);
              وإلا
		کپی_حافظه (argp, *p_argv, z);
#وإلا
	      کپی_حافظه (argp + حجم (صحيح) - z, *p_argv, z);
#پایان_اگر
	      اكسر;

	    افتراضي:
	      FFI_ASSERT (0);
	    }
	  z = حجم (صحيح);
	}
      وإلا
	{
	  کپی_حافظه (argp, *p_argv, z);

	  /* Align if necessary.  */
	  إذا ((حجم(صحيح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحيح));
	}

      p_argv++;
      argp += z;
    }

  أرجع struct_مقدار_ptr;
}

#تعریف_کن CIF_FLAGS_INT		1
#تعریف_کن CIF_FLAGS_DINT		2
#تعریف_کن CIF_FLAGS_FLOAT		4
#تعریف_کن CIF_FLAGS_DOUBLE	8
#تعریف_کن CIF_FLAGS_LDOUBLE	16
#تعریف_کن CIF_FLAGS_اشاره_گر	32
#تعریف_کن CIF_FLAGS_STRUCT1	64
#تعریف_کن CIF_FLAGS_STRUCT2	128
#تعریف_کن CIF_FLAGS_SINT8		256
#تعریف_کن CIF_FLAGS_SINT16	512

/* Perform machine dependent cif processing */
ffi_وضعیت
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

      بدل (cif->rtype->اندازه)
	{
	حالة 1:
#اگر_تعریف_شده __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#وإلا
	  cif->flags = CIF_FLAGS_STRUCT1;
#پایان_اگر
	  اكسر;
	حالة 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  اكسر;
#اگر_تعریف_شده __MINT__
	حالة 3:
#پایان_اگر
	حالة 4:
	  cif->flags = CIF_FLAGS_INT;
	  اكسر;
#اگر_تعریف_شده __MINT__
	حالة 7:
#پایان_اگر
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
#اگر_تعریف_شده __MINT__
      cif->flags = 0;
#وإلا
      cif->flags = CIF_FLAGS_LDOUBLE;
#پایان_اگر
      اكسر;
#پایان_اگر

    حالة FFI_TYPE_اشاره_گر:
      cif->flags = CIF_FLAGS_اشاره_گر;
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
      && cif->rtype->اندازه > 8)
    ecif.rvalue = alloca (cif->rtype->اندازه);
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->بایتها, cif->flags,
		     ecif.rvalue, fn);
      اكسر;

    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

ساكن فراغ
ffi_prep_incoming_آرگومانها_SYSV (حرف *پشته, فراغ **avalue, ffi_cif *cif)
{
  بیعلامت صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  argp = پشته;
  p_argv = avalue;

  لكل (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->اندازه;
#اگر_تعریف_شده __MINT__
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
#پایان_اگر
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

بیعلامت صحيح
ffi_بستگی_SYSV_inner (ffi_بستگی *بستگی, فراغ *resp, فراغ *آرگومانها)
{
  ffi_cif *cif;
  فراغ **arg_area;

  cif = بستگی->cif;
  arg_area = (فراغ**) alloca (cif->nargs * حجم (فراغ *));

  ffi_prep_incoming_آرگومانها_SYSV(آرگومانها, arg_area, cif);

  (بستگی->fun) (cif, resp, arg_area, بستگی->user_داده);

  أرجع cif->flags;
}

ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی* بستگی,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		      فراغ *user_داده,
		      فراغ *codeloc)
{
  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  *(بیعلامت کوتاه *)بستگی->tramp = 0x207c;
  *(فراغ **)(بستگی->tramp + 2) = codeloc;
  *(بیعلامت کوتاه *)(بستگی->tramp + 6) = 0x4ef9;

  إذا (
#اگر_تعریف_شده __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#پایان_اگر
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(فراغ **)(بستگی->tramp + 8) = ffi_بستگی_struct_SYSV;
  وإلا
    *(فراغ **)(بستگی->tramp + 8) = ffi_بستگی_SYSV;

#اگر_تعریف_شده __rtems__
  rtems_cache_flush_مضرب_داده_سطرها( codeloc, FFI_TRAMPOLINE_اندازه );
#وإلاإذا defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_اندازه);
#وإلا
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_سطر,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_اندازه);
#پایان_اگر

  بستگی->cif  = cif;
  بستگی->user_داده = user_داده;
  بستگی->fun  = fun;

  أرجع FFI_OK;
}
