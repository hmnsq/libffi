/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <unistd.h>
#నిర్వచితమైతే __rtems__
శూన్య rtems_cache_flush_గుణిజం_దత్తాంశం_పంక్తులు( స్థిరరాశి శూన్య *, size_t );
#లేకపోతే
#అన్తర్భావ <sys/syscall.h>
#నిర్వచితమైతే __MINT__
#అన్తర్భావ <mint/mintbind.h>
#అన్తర్భావ <mint/ssystem.h>
#లేకపోతే
#అన్తర్భావ <asm/cachectl.h>
#అయితే_ముగింపు
#అయితే_ముగింపు

శూన్య ffi_call_SYSV (extended_cif *,
		    గుర్తులేని, గుర్తులేని,
		    శూన్య *, శూన్య (*fn) ());
శూన్య *ffi_prep_వాదనలు (శూన్య *దొంతర, extended_cif *ecif);
శూన్య ffi_సంవృతం_SYSV (ffi_సంవృతం *);
శూన్య ffi_సంవృతం_struct_SYSV (ffi_సంవృతం *);
గుర్తులేని పూర్ణాంకం ffi_సంవృతం_SYSV_inner (ffi_సంవృతం *సంవృతం,
				     శూన్య *resp, శూన్య *వాదనలు);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

శూన్య *
ffi_prep_వాదనలు (శూన్య *దొంతర, extended_cif *ecif)
{
  గుర్తులేని పూర్ణాంకం i;
  శూన్య **p_argv;
  అక్షరం *argp;
  ffi_type **p_arg;
  శూన్య *struct_విలువ_ptr;

  argp = దొంతర;

  ఒకవేళ (
#నిర్వచితమైతే __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#అయితే_ముగింపు
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_విలువ_ptr = ecif->rvalue;
  లేకపోతే
    struct_విలువ_ptr = ఖాళీ;

  p_argv = ecif->avalue;

  కొరకు (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->కొలత;
      పూర్ణాంకం type = (*p_arg)->type;

      ఒకవేళ (z < పరిమాణం (పూర్ణాంకం))
	{
	  ఎంపిక (type)
	    {
	    స్థితి FFI_TYPE_SINT8:
	      *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం) *(SINT8 *) *p_argv;
	      ఆపు;

	    స్థితి FFI_TYPE_UINT8:
	      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం) *(UINT8 *) *p_argv;
	      ఆపు;

	    స్థితి FFI_TYPE_SINT16:
	      *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం) *(SINT16 *) *p_argv;
	      ఆపు;

	    స్థితి FFI_TYPE_UINT16:
	      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం) *(UINT16 *) *p_argv;
	      ఆపు;

	    స్థితి FFI_TYPE_STRUCT:
#నిర్వచితమైతే __MINT__
	      ఒకవేళ (z == 1 || z == 2)
		జ్ఞాపకం_నకలు (argp + 2, *p_argv, z);
              లేకపోతే
		జ్ఞాపకం_నకలు (argp, *p_argv, z);
#లేకపోతే
	      జ్ఞాపకం_నకలు (argp + పరిమాణం (పూర్ణాంకం) - z, *p_argv, z);
#అయితే_ముగింపు
	      ఆపు;

	    అప్రమేయ:
	      FFI_ASSERT (0);
	    }
	  z = పరిమాణం (పూర్ణాంకం);
	}
      లేకపోతే
	{
	  జ్ఞాపకం_నకలు (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ఒకవేళ ((పరిమాణం(పూర్ణాంకం) - 1) & z)
	    z = FFI_ALIGN(z, పరిమాణం(పూర్ణాంకం));
	}

      p_argv++;
      argp += z;
    }

  తిరిగి struct_విలువ_ptr;
}

#నిర్వచించు CIF_FLAGS_INT		1
#నిర్వచించు CIF_FLAGS_DINT		2
#నిర్వచించు CIF_FLAGS_FLOAT		4
#నిర్వచించు CIF_FLAGS_DOUBLE	8
#నిర్వచించు CIF_FLAGS_LDOUBLE	16
#నిర్వచించు CIF_FLAGS_సూచిక	32
#నిర్వచించు CIF_FLAGS_STRUCT1	64
#నిర్వచించు CIF_FLAGS_STRUCT2	128
#నిర్వచించు CIF_FLAGS_SINT8		256
#నిర్వచించు CIF_FLAGS_SINT16	512

/* Perform machine dependent cif processing */
ffi_స్థితి
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
      cif->flags = 0;
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
      ఒకవేళ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          ఆపు;
        }

      ఎంపిక (cif->rtype->కొలత)
	{
	స్థితి 1:
#నిర్వచితమైతే __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#లేకపోతే
	  cif->flags = CIF_FLAGS_STRUCT1;
#అయితే_ముగింపు
	  ఆపు;
	స్థితి 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  ఆపు;
#నిర్వచితమైతే __MINT__
	స్థితి 3:
#అయితే_ముగింపు
	స్థితి 4:
	  cif->flags = CIF_FLAGS_INT;
	  ఆపు;
#నిర్వచితమైతే __MINT__
	స్థితి 7:
#అయితే_ముగింపు
	స్థితి 8:
	  cif->flags = CIF_FLAGS_DINT;
	  ఆపు;
	అప్రమేయ:
	  cif->flags = 0;
	  ఆపు;
	}
      ఆపు;

    స్థితి FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      ఆపు;

    స్థితి FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      ఆపు;

#ఒకవేళ (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    స్థితి FFI_TYPE_LONGDOUBLE:
#నిర్వచితమైతే __MINT__
      cif->flags = 0;
#లేకపోతే
      cif->flags = CIF_FLAGS_LDOUBLE;
#అయితే_ముగింపు
      ఆపు;
#అయితే_ముగింపు

    స్థితి FFI_TYPE_సూచిక:
      cif->flags = CIF_FLAGS_సూచిక;
      ఆపు;

    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ఆపు;

    స్థితి FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      ఆపు;

    స్థితి FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      ఆపు;

    అప్రమేయ:
      cif->flags = CIF_FLAGS_INT;
      ఆపు;
    }

  తిరిగి FFI_OK;
}

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn) (), శూన్య *rvalue, శూన్య **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ఒకవేళ (rvalue == ఖాళీ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->కొలత > 8)
    ecif.rvalue = alloca (cif->rtype->కొలత);
  లేకపోతే
    ecif.rvalue = rvalue;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->బైట్లు, cif->flags,
		     ecif.rvalue, fn);
      ఆపు;

    అప్రమేయ:
      FFI_ASSERT (0);
      ఆపు;
    }
}

స్థిర శూన్య
ffi_prep_incoming_వాదనలు_SYSV (అక్షరం *దొంతర, శూన్య **avalue, ffi_cif *cif)
{
  గుర్తులేని పూర్ణాంకం i;
  శూన్య **p_argv;
  అక్షరం *argp;
  ffi_type **p_arg;

  argp = దొంతర;
  p_argv = avalue;

  కొరకు (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->కొలత;
#నిర్వచితమైతే __MINT__
      ఒకవేళ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (శూన్య *) (argp + 2);

	  z = 4;
	}
      లేకపోతే
      ఒకవేళ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (శూన్య *) (argp);

	  z = 4;
	}
      లేకపోతే
#అయితే_ముగింపు
      ఒకవేళ (z <= 4)
	{
	  *p_argv = (శూన్య *) (argp + 4 - z);

	  z = 4;
	}
      లేకపోతే
	{
	  *p_argv = (శూన్య *) argp;

	  /* Align if necessary */
	  ఒకవేళ ((పరిమాణం(పూర్ణాంకం) - 1) & z)
	    z = FFI_ALIGN(z, పరిమాణం(పూర్ణాంకం));
	}

      p_argv++;
      argp += z;
    }
}

గుర్తులేని పూర్ణాంకం
ffi_సంవృతం_SYSV_inner (ffi_సంవృతం *సంవృతం, శూన్య *resp, శూన్య *వాదనలు)
{
  ffi_cif *cif;
  శూన్య **arg_area;

  cif = సంవృతం->cif;
  arg_area = (శూన్య**) alloca (cif->nargs * పరిమాణం (శూన్య *));

  ffi_prep_incoming_వాదనలు_SYSV(వాదనలు, arg_area, cif);

  (సంవృతం->fun) (cif, resp, arg_area, సంవృతం->user_దత్తాంశం);

  తిరిగి cif->flags;
}

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  ఒకవేళ (cif->abi != FFI_SYSV)
    తిరిగి FFI_BAD_ABI;

  *(గుర్తులేని పొట్టి *)సంవృతం->tramp = 0x207c;
  *(శూన్య **)(సంవృతం->tramp + 2) = codeloc;
  *(గుర్తులేని పొట్టి *)(సంవృతం->tramp + 6) = 0x4ef9;

  ఒకవేళ (
#నిర్వచితమైతే __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#అయితే_ముగింపు
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(శూన్య **)(సంవృతం->tramp + 8) = ffi_సంవృతం_struct_SYSV;
  లేకపోతే
    *(శూన్య **)(సంవృతం->tramp + 8) = ffi_సంవృతం_SYSV;

#నిర్వచితమైతే __rtems__
  rtems_cache_flush_గుణిజం_దత్తాంశం_పంక్తులు( codeloc, FFI_TRAMPOLINE_కొలత );
#లేదావేళ defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_కొలత);
#లేకపోతే
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_పంక్తి,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_కొలత);
#అయితే_ముగింపు

  సంవృతం->cif  = cif;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  సంవృతం->fun  = fun;

  తిరిగి FFI_OK;
}
