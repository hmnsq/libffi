/*
 * Copyright (c) 2013 Miodrag Vallat.  <miod@openbsd.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * vax Foreign Function Interface
 *
 * This file attempts to provide all the FFI entry points which can reliably
 * be implemented in C.
 */

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <unistd.h>

#నిర్వచించు CIF_FLAGS_CHAR		1	/* for struct only */
#నిర్వచించు CIF_FLAGS_SHORT		2	/* for struct only */
#నిర్వచించు CIF_FLAGS_INT		4
#నిర్వచించు CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

శూన్య ffi_call_elfbsd (extended_cif *, గుర్తులేని, గుర్తులేని, శూన్య *,
		       శూన్య (*) ());
శూన్య *ffi_prep_వాదనలు (extended_cif *ecif, శూన్య *దొంతర);

శూన్య *
ffi_prep_వాదనలు (extended_cif *ecif, శూన్య *దొంతర)
{
  గుర్తులేని పూర్ణాంకం i;
  శూన్య **p_argv;
  అక్షరం *argp;
  ffi_type **p_arg;
  శూన్య *struct_విలువ_ptr;

  argp = దొంతర;

  ఒకవేళ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_విలువ_ptr = ecif->rvalue;
  లేకపోతే
    struct_విలువ_ptr = ఖాళీ;

  p_argv = ecif->avalue;

  కొరకు (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->కొలత;
      ఒకవేళ (z < పరిమాణం (పూర్ణాంకం))
	{
	  ఎంపిక ((*p_arg)->type)
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
	      జ్ఞాపకం_నకలు (argp, *p_argv, z);
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

      ఒకవేళ (cif->rtype->కొలత == పరిమాణం (అక్షరం))
	cif->flags = CIF_FLAGS_CHAR;
      లేకపోతే ఒకవేళ (cif->rtype->కొలత == పరిమాణం (పొట్టి))
	cif->flags = CIF_FLAGS_SHORT;
      లేకపోతే ఒకవేళ (cif->rtype->కొలత == పరిమాణం (పూర్ణాంకం))
	cif->flags = CIF_FLAGS_INT;
      లేకపోతే ఒకవేళ (cif->rtype->కొలత == 2 * పరిమాణం (పూర్ణాంకం))
	cif->flags = CIF_FLAGS_DINT;
      లేకపోతే
	cif->flags = 0;
      ఆపు;

    అప్రమేయ:
      ఒకవేళ (cif->rtype->కొలత <= పరిమాణం (పూర్ణాంకం))
	cif->flags = CIF_FLAGS_INT;
      లేకపోతే
	cif->flags = CIF_FLAGS_DINT;
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
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->కొలత);
  లేకపోతే
    ecif.rvalue = rvalue;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->బైట్లు, cif->flags, ecif.rvalue, fn);
      ఆపు;

    అప్రమేయ:
      FFI_ASSERT (0);
      ఆపు;
    }
}

/*
 * Closure API
 */

శూన్య ffi_సంవృతం_elfbsd (శూన్య);
శూన్య ffi_సంవృతం_struct_elfbsd (శూన్య);
గుర్తులేని పూర్ణాంకం ffi_సంవృతం_elfbsd_inner (ffi_సంవృతం *, శూన్య *, అక్షరం *);

స్థిర శూన్య
ffi_prep_సంవృతం_elfbsd (ffi_cif *cif, శూన్య **avalue, అక్షరం *stackp)
{
  గుర్తులేని పూర్ణాంకం i;
  శూన్య **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  కొరకు (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->కొలత;
      *p_argv = stackp;

      /* Align if necessary */
      ఒకవేళ ((పరిమాణం (పూర్ణాంకం) - 1) & z)
	z = FFI_ALIGN(z, పరిమాణం (పూర్ణాంకం));

      p_argv++;
      stackp += z;
    }
}

గుర్తులేని పూర్ణాంకం
ffi_సంవృతం_elfbsd_inner (ffi_సంవృతం *సంవృతం, శూన్య *resp, అక్షరం *దొంతర)
{
  ffi_cif *cif;
  శూన్య **arg_area;

  cif = సంవృతం->cif;
  arg_area = (శూన్య **) alloca (cif->nargs * పరిమాణం (శూన్య *));

  ffi_prep_సంవృతం_elfbsd (cif, arg_area, దొంతర);

  (సంవృతం->fun) (cif, resp, arg_area, సంవృతం->user_దత్తాంశం);

  తిరిగి cif->flags;
}

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం *సంవృతం, ffi_cif *cif,
		      శూన్య (*fun)(ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
		      శూన్య *user_దత్తాంశం, శూన్య *codeloc)
{
  అక్షరం *tramp = (అక్షరం *) codeloc;
  శూన్య *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(గుర్తులేని పొట్టి *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(గుర్తులేని పూర్ణాంకం *)(tramp + 4) = (గుర్తులేని పూర్ణాంకం) సంవృతం;
  tramp[8] = 0x50;

  ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_సంవృతం_struct_elfbsd;
  లేకపోతే
    fn = &ffi_సంవృతం_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(గుర్తులేని పూర్ణాంకం *)(tramp + 11) = (గుర్తులేని పూర్ణాంకం)fn + 2 -
				  (గుర్తులేని పూర్ణాంకం)tramp - 9 - 6;

  సంవృతం->cif = cif;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}
