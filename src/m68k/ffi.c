/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>
#شامل <unistd.h>
#اگر_تعریف_شدہ __rtems__
خالی rtems_cache_flush_مضرب_ڈیٹا_سطریں( ثابت خالی *, size_t );
#ورنہ
#شامل <sys/syscall.h>
#اگر_تعریف_شدہ __MINT__
#شامل <mint/mintbind.h>
#شامل <mint/ssystem.h>
#ورنہ
#شامل <asm/cachectl.h>
#اگر_ختم
#اگر_ختم

خالی ffi_call_SYSV (extended_cif *,
		    غیرعلامتی, غیرعلامتی,
		    خالی *, خالی (*fn) ());
خالی *ffi_prep_دلائل (خالی *ڈھیر, extended_cif *ecif);
خالی ffi_احاطہ_SYSV (ffi_احاطہ *);
خالی ffi_احاطہ_struct_SYSV (ffi_احاطہ *);
غیرعلامتی صحیح ffi_احاطہ_SYSV_inner (ffi_احاطہ *احاطہ,
				     خالی *resp, خالی *دلائل);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

خالی *
ffi_prep_دلائل (خالی *ڈھیر, extended_cif *ecif)
{
  غیرعلامتی صحیح i;
  خالی **p_argv;
  حرف *argp;
  ffi_type **p_arg;
  خالی *struct_قدر_ptr;

  argp = ڈھیر;

  اگر (
#اگر_تعریف_شدہ __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#اگر_ختم
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_قدر_ptr = ecif->rvalue;
  ورنہ
    struct_قدر_ptr = تہی;

  p_argv = ecif->avalue;

  برائے (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->سائز;
      صحیح type = (*p_arg)->type;

      اگر (z < حجم (صحیح))
	{
	  انتخاب (type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(علامتی صحیح *) argp = (علامتی صحیح) *(SINT8 *) *p_argv;
	      توڑو;

	    حالت FFI_TYPE_UINT8:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح) *(UINT8 *) *p_argv;
	      توڑو;

	    حالت FFI_TYPE_SINT16:
	      *(علامتی صحیح *) argp = (علامتی صحیح) *(SINT16 *) *p_argv;
	      توڑو;

	    حالت FFI_TYPE_UINT16:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح) *(UINT16 *) *p_argv;
	      توڑو;

	    حالت FFI_TYPE_STRUCT:
#اگر_تعریف_شدہ __MINT__
	      اگر (z == 1 || z == 2)
		حافظہ_نقل (argp + 2, *p_argv, z);
              ورنہ
		حافظہ_نقل (argp, *p_argv, z);
#ورنہ
	      حافظہ_نقل (argp + حجم (صحیح) - z, *p_argv, z);
#اگر_ختم
	      توڑو;

	    طےشدہ:
	      FFI_ASSERT (0);
	    }
	  z = حجم (صحیح);
	}
      ورنہ
	{
	  حافظہ_نقل (argp, *p_argv, z);

	  /* Align if necessary.  */
	  اگر ((حجم(صحیح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحیح));
	}

      p_argv++;
      argp += z;
    }

  واپس struct_قدر_ptr;
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
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      cif->flags = 0;
      توڑو;

    حالت FFI_TYPE_STRUCT:
      اگر (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          توڑو;
        }

      انتخاب (cif->rtype->سائز)
	{
	حالت 1:
#اگر_تعریف_شدہ __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#ورنہ
	  cif->flags = CIF_FLAGS_STRUCT1;
#اگر_ختم
	  توڑو;
	حالت 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  توڑو;
#اگر_تعریف_شدہ __MINT__
	حالت 3:
#اگر_ختم
	حالت 4:
	  cif->flags = CIF_FLAGS_INT;
	  توڑو;
#اگر_تعریف_شدہ __MINT__
	حالت 7:
#اگر_ختم
	حالت 8:
	  cif->flags = CIF_FLAGS_DINT;
	  توڑو;
	طےشدہ:
	  cif->flags = 0;
	  توڑو;
	}
      توڑو;

    حالت FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      توڑو;

    حالت FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      توڑو;

#اگر (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    حالت FFI_TYPE_LONGDOUBLE:
#اگر_تعریف_شدہ __MINT__
      cif->flags = 0;
#ورنہ
      cif->flags = CIF_FLAGS_LDOUBLE;
#اگر_ختم
      توڑو;
#اگر_ختم

    حالت FFI_TYPE_اشارہ_گر:
      cif->flags = CIF_FLAGS_اشارہ_گر;
      توڑو;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      توڑو;

    حالت FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      توڑو;

    حالت FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      توڑو;

    طےشدہ:
      cif->flags = CIF_FLAGS_INT;
      توڑو;
    }

  واپس FFI_OK;
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn) (), خالی *rvalue, خالی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  اگر (rvalue == تہی
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->سائز > 8)
    ecif.rvalue = alloca (cif->rtype->سائز);
  ورنہ
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->بائٹیں, cif->flags,
		     ecif.rvalue, fn);
      توڑو;

    طےشدہ:
      FFI_ASSERT (0);
      توڑو;
    }
}

ساکن خالی
ffi_prep_incoming_دلائل_SYSV (حرف *ڈھیر, خالی **avalue, ffi_cif *cif)
{
  غیرعلامتی صحیح i;
  خالی **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  argp = ڈھیر;
  p_argv = avalue;

  برائے (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->سائز;
#اگر_تعریف_شدہ __MINT__
      اگر (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (خالی *) (argp + 2);

	  z = 4;
	}
      ورنہ
      اگر (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (خالی *) (argp);

	  z = 4;
	}
      ورنہ
#اگر_ختم
      اگر (z <= 4)
	{
	  *p_argv = (خالی *) (argp + 4 - z);

	  z = 4;
	}
      ورنہ
	{
	  *p_argv = (خالی *) argp;

	  /* Align if necessary */
	  اگر ((حجم(صحیح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحیح));
	}

      p_argv++;
      argp += z;
    }
}

غیرعلامتی صحیح
ffi_احاطہ_SYSV_inner (ffi_احاطہ *احاطہ, خالی *resp, خالی *دلائل)
{
  ffi_cif *cif;
  خالی **arg_area;

  cif = احاطہ->cif;
  arg_area = (خالی**) alloca (cif->nargs * حجم (خالی *));

  ffi_prep_incoming_دلائل_SYSV(دلائل, arg_area, cif);

  (احاطہ->fun) (cif, resp, arg_area, احاطہ->user_ڈیٹا);

  واپس cif->flags;
}

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  اگر (cif->abi != FFI_SYSV)
    واپس FFI_BAD_ABI;

  *(غیرعلامتی مختصر *)احاطہ->tramp = 0x207c;
  *(خالی **)(احاطہ->tramp + 2) = codeloc;
  *(غیرعلامتی مختصر *)(احاطہ->tramp + 6) = 0x4ef9;

  اگر (
#اگر_تعریف_شدہ __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#اگر_ختم
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(خالی **)(احاطہ->tramp + 8) = ffi_احاطہ_struct_SYSV;
  ورنہ
    *(خالی **)(احاطہ->tramp + 8) = ffi_احاطہ_SYSV;

#اگر_تعریف_شدہ __rtems__
  rtems_cache_flush_مضرب_ڈیٹا_سطریں( codeloc, FFI_TRAMPOLINE_سائز );
#وگرنہ_اگر defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_سائز);
#ورنہ
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_سطر,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_سائز);
#اگر_ختم

  احاطہ->cif  = cif;
  احاطہ->user_ڈیٹا = user_ڈیٹا;
  احاطہ->fun  = fun;

  واپس FFI_OK;
}
