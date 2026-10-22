/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <unistd.h>
#که_تعریف_شوی __rtems__
فراغ rtems_cache_flush_مضرب_معلومات_کرښې( ثابت فراغ *, size_t );
#وإلا
#ضمن <sys/syscall.h>
#که_تعریف_شوی __MINT__
#ضمن <mint/mintbind.h>
#ضمن <mint/ssystem.h>
#وإلا
#ضمن <asm/cachectl.h>
#د_که_پای
#د_که_پای

فراغ ffi_call_SYSV (extended_cif *,
		    بینښه, بینښه,
		    فراغ *, فراغ (*fn) ());
فراغ *ffi_prep_دلیلونه (فراغ *ډېری, extended_cif *ecif);
فراغ ffi_تړون_SYSV (ffi_تړون *);
فراغ ffi_تړون_struct_SYSV (ffi_تړون *);
بینښه صحيح ffi_تړون_SYSV_inner (ffi_تړون *تړون,
				     فراغ *resp, فراغ *دلیلونه);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

فراغ *
ffi_prep_دلیلونه (فراغ *ډېری, extended_cif *ecif)
{
  بینښه صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;
  فراغ *struct_ارزښت_ptr;

  argp = ډېری;

  إذا (
#که_تعریف_شوی __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#د_که_پای
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_ارزښت_ptr = ecif->rvalue;
  وإلا
    struct_ارزښت_ptr = عدم;

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
	      *(نښهدار صحيح *) argp = (نښهدار صحيح) *(SINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(بینښه صحيح *) argp = (بینښه صحيح) *(UINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(نښهدار صحيح *) argp = (نښهدار صحيح) *(SINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(بینښه صحيح *) argp = (بینښه صحيح) *(UINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_STRUCT:
#که_تعریف_شوی __MINT__
	      إذا (z == 1 || z == 2)
		حافظه_نقل (argp + 2, *p_argv, z);
              وإلا
		حافظه_نقل (argp, *p_argv, z);
#وإلا
	      حافظه_نقل (argp + حجم (صحيح) - z, *p_argv, z);
#د_که_پای
	      اكسر;

	    افتراضي:
	      FFI_ASSERT (0);
	    }
	  z = حجم (صحيح);
	}
      وإلا
	{
	  حافظه_نقل (argp, *p_argv, z);

	  /* Align if necessary.  */
	  إذا ((حجم(صحيح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحيح));
	}

      p_argv++;
      argp += z;
    }

  أرجع struct_ارزښت_ptr;
}

#تعریف_کړه CIF_FLAGS_INT		1
#تعریف_کړه CIF_FLAGS_DINT		2
#تعریف_کړه CIF_FLAGS_FLOAT		4
#تعریف_کړه CIF_FLAGS_DOUBLE	8
#تعریف_کړه CIF_FLAGS_LDOUBLE	16
#تعریف_کړه CIF_FLAGS_اشاره_ګر	32
#تعریف_کړه CIF_FLAGS_STRUCT1	64
#تعریف_کړه CIF_FLAGS_STRUCT2	128
#تعریف_کړه CIF_FLAGS_SINT8		256
#تعریف_کړه CIF_FLAGS_SINT16	512

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

      بدل (cif->rtype->اندازه)
	{
	حالة 1:
#که_تعریف_شوی __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#وإلا
	  cif->flags = CIF_FLAGS_STRUCT1;
#د_که_پای
	  اكسر;
	حالة 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  اكسر;
#که_تعریف_شوی __MINT__
	حالة 3:
#د_که_پای
	حالة 4:
	  cif->flags = CIF_FLAGS_INT;
	  اكسر;
#که_تعریف_شوی __MINT__
	حالة 7:
#د_که_پای
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
#که_تعریف_شوی __MINT__
      cif->flags = 0;
#وإلا
      cif->flags = CIF_FLAGS_LDOUBLE;
#د_که_پای
      اكسر;
#د_که_پای

    حالة FFI_TYPE_اشاره_ګر:
      cif->flags = CIF_FLAGS_اشاره_ګر;
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
      ffi_call_SYSV (&ecif, cif->بایټونه, cif->flags,
		     ecif.rvalue, fn);
      اكسر;

    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

ساكن فراغ
ffi_prep_incoming_دلیلونه_SYSV (حرف *ډېری, فراغ **avalue, ffi_cif *cif)
{
  بینښه صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  argp = ډېری;
  p_argv = avalue;

  لكل (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->اندازه;
#که_تعریف_شوی __MINT__
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
#د_که_پای
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

بینښه صحيح
ffi_تړون_SYSV_inner (ffi_تړون *تړون, فراغ *resp, فراغ *دلیلونه)
{
  ffi_cif *cif;
  فراغ **arg_area;

  cif = تړون->cif;
  arg_area = (فراغ**) alloca (cif->nargs * حجم (فراغ *));

  ffi_prep_incoming_دلیلونه_SYSV(دلیلونه, arg_area, cif);

  (تړون->fun) (cif, resp, arg_area, تړون->user_معلومات);

  أرجع cif->flags;
}

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون* تړون,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  *(بینښه لنډ *)تړون->tramp = 0x207c;
  *(فراغ **)(تړون->tramp + 2) = codeloc;
  *(بینښه لنډ *)(تړون->tramp + 6) = 0x4ef9;

  إذا (
#که_تعریف_شوی __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#د_که_پای
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(فراغ **)(تړون->tramp + 8) = ffi_تړون_struct_SYSV;
  وإلا
    *(فراغ **)(تړون->tramp + 8) = ffi_تړون_SYSV;

#که_تعریف_شوی __rtems__
  rtems_cache_flush_مضرب_معلومات_کرښې( codeloc, FFI_TRAMPOLINE_اندازه );
#وإلاإذا defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_اندازه);
#وإلا
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_کرښه,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_اندازه);
#د_که_پای

  تړون->cif  = cif;
  تړون->user_معلومات = user_معلومات;
  تړون->fun  = fun;

  أرجع FFI_OK;
}
