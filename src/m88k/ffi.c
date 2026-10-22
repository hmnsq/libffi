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
 * m88k Foreign Function Interface
 *
 * This file attempts to provide all the FFI entry points which can reliably
 * be implemented in C.
 *
 * Only OpenBSD/m88k is currently supported; other platforms (such as
 * Motorola's SysV/m88k) could be supported with the following tweaks:
 *
 * - non-OpenBSD systems use an `outgoing parameter area' as part of the
 *   88BCS calling convention, which is not supported under OpenBSD from
 *   release 3.6 onwards.  Supporting it should be as easy as taking it
 *   into account when adjusting the stack, in the assembly code.
 *
 * - the logic deciding whether a function argument gets passed through
 *   registers, or on the stack, has changed several times in OpenBSD in
 *   edge cases (especially for structs larger than 32 bytes being passed
 *   by value). The code below attemps to match the logic used by the
 *   system compiler of OpenBSD 5.3, i.e. gcc 3.3.6 with many m88k backend
 *   fixes.
 */

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <unistd.h>

శూన్య ffi_call_OBSD (గుర్తులేని పూర్ణాంకం, extended_cif *, గుర్తులేని పూర్ణాంకం, శూన్య *,
		    శూన్య (*fn) ());
శూన్య *ffi_prep_వాదనలు (శూన్య *, extended_cif *);
శూన్య ffi_సంవృతం_OBSD (ffi_సంవృతం *);
శూన్య ffi_సంవృతం_struct_OBSD (ffi_సంవృతం *);
గుర్తులేని పూర్ణాంకం ffi_సంవృతం_OBSD_inner (ffi_సంవృతం *, శూన్య *, గుర్తులేని పూర్ణాంకం *,
				     అక్షరం *);
శూన్య ffi_cacheflush_OBSD (గుర్తులేని పూర్ణాంకం, గుర్తులేని పూర్ణాంకం);

#నిర్వచించు CIF_FLAGS_INT		(1 << 0)
#నిర్వచించు CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

శూన్య *
ffi_prep_వాదనలు (శూన్య *దొంతర, extended_cif *ecif)
{
  గుర్తులేని పూర్ణాంకం i;
  శూన్య **p_argv;
  అక్షరం *argp, *stackp;
  గుర్తులేని పూర్ణాంకం *regp;
  గుర్తులేని పూర్ణాంకం regused;
  ffi_type **p_arg;
  శూన్య *struct_విలువ_ptr;

  regp = (గుర్తులేని పూర్ణాంకం *)దొంతర;
  stackp = (అక్షరం *)(regp + 8);
  regused = 0;

  ఒకవేళ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_విలువ_ptr = ecif->rvalue;
  లేకపోతే
    struct_విలువ_ptr = ఖాళీ;

  p_argv = ecif->avalue;

  కొరకు (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      గుర్తులేని పొట్టి t, a;

      z = (*p_arg)->కొలత;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument can be passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      ఒకవేళ (t == FFI_TYPE_STRUCT)
	{
	  ఒకవేళ (z == పరిమాణం (పూర్ణాంకం) && a == పరిమాణం (పూర్ణాంకం) && regused < 8)
	    argp = (అక్షరం *)regp;
	  లేకపోతే
	    argp = stackp;
	}
      లేకపోతే
	{
	  ఒకవేళ (z > పరిమాణం (పూర్ణాంకం) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ఒకవేళ (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ఒకవేళ (regused < 8)
	    argp = (అక్షరం *)regp;
	  లేకపోతే
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ఒకవేళ (argp == stackp && a > పరిమాణం (పూర్ణాంకం))
	{
	  stackp = (అక్షరం *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ఎంపిక (t)
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

	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_సూచిక:
	  *(గుర్తులేని పూర్ణాంకం *) argp = *(గుర్తులేని పూర్ణాంకం *) *p_argv;
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_STRUCT:
	  జ్ఞాపకం_నకలు (argp, *p_argv, z);
	  ఆపు;

	అప్రమేయ:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      ఒకవేళ ((పరిమాణం (పూర్ణాంకం) - 1) & z)
	z = FFI_ALIGN(z, పరిమాణం (పూర్ణాంకం));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      ఒకవేళ (argp == (అక్షరం *)regp && regused < 8)
	{
	  regp += z / పరిమాణం (పూర్ణాంకం);
	  regused += z / పరిమాణం (పూర్ణాంకం);
	}
      లేకపోతే
	stackp += z;
    }

  తిరిగి struct_విలువ_ptr;
}

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
      ఒకవేళ (cif->rtype->కొలత == పరిమాణం (పూర్ణాంకం) &&
	  cif->rtype->alignment == పరిమాణం (పూర్ణాంకం))
	cif->flags = CIF_FLAGS_INT;
      లేకపోతే
	cif->flags = 0;
      ఆపు;

    స్థితి FFI_TYPE_DOUBLE:
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
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
      && (cif->rtype->కొలత != పరిమాణం (పూర్ణాంకం)
	  || cif->rtype->alignment != పరిమాణం (పూర్ణాంకం)))
    ecif.rvalue = alloca (cif->rtype->కొలత);
  లేకపోతే
    ecif.rvalue = rvalue;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_OBSD:
      ffi_call_OBSD (cif->బైట్లు, &ecif, cif->flags, ecif.rvalue, fn);
      ఆపు;

    అప్రమేయ:
      FFI_ASSERT (0);
      ఆపు;
    }
}

/*
 * Closure API
 */

స్థిర శూన్య
ffi_prep_సంవృతం_వాదనలు_OBSD (ffi_cif *cif, శూన్య **avalue, గుర్తులేని పూర్ణాంకం *regp,
			    అక్షరం *stackp)
{
  గుర్తులేని పూర్ణాంకం i;
  శూన్య **p_argv;
  అక్షరం *argp;
  గుర్తులేని పూర్ణాంకం regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  కొరకు (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      గుర్తులేని పొట్టి t, a;

      z = (*p_arg)->కొలత;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument has been passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      ఒకవేళ (t == FFI_TYPE_STRUCT)
	{
	  ఒకవేళ (z == పరిమాణం (పూర్ణాంకం) && a == పరిమాణం (పూర్ణాంకం) && regused < 8)
	    argp = (అక్షరం *)regp;
	  లేకపోతే
	    argp = stackp;
	}
      లేకపోతే
	{
	  ఒకవేళ (z > పరిమాణం (పూర్ణాంకం) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ఒకవేళ (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ఒకవేళ (regused < 8)
	    argp = (అక్షరం *)regp;
	  లేకపోతే
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ఒకవేళ (argp == stackp && a > పరిమాణం (పూర్ణాంకం))
	{
	  stackp = (అక్షరం *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ఒకవేళ (z < పరిమాణం (పూర్ణాంకం) && t != FFI_TYPE_STRUCT)
	*p_argv = (శూన్య *) (argp + పరిమాణం (పూర్ణాంకం) - z);
      లేకపోతే
	*p_argv = (శూన్య *) argp;

      /* Align if necessary */
      ఒకవేళ ((పరిమాణం (పూర్ణాంకం) - 1) & z)
	z = FFI_ALIGN(z, పరిమాణం (పూర్ణాంకం));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      ఒకవేళ (argp == (అక్షరం *)regp && regused < 8)
	{
	  regp += z / పరిమాణం (పూర్ణాంకం);
	  regused += z / పరిమాణం (పూర్ణాంకం);
	}
      లేకపోతే
	stackp += z;
    }
}

గుర్తులేని పూర్ణాంకం
ffi_సంవృతం_OBSD_inner (ffi_సంవృతం *సంవృతం, శూన్య *resp, గుర్తులేని పూర్ణాంకం *regp,
			అక్షరం *stackp)
{
  ffi_cif *cif;
  శూన్య **arg_area;

  cif = సంవృతం->cif;
  arg_area = (శూన్య**) alloca (cif->nargs * పరిమాణం (శూన్య *));

  ffi_prep_సంవృతం_వాదనలు_OBSD(cif, arg_area, regp, stackp);

  (సంవృతం->fun) (cif, resp, arg_area, సంవృతం->user_దత్తాంశం);

  తిరిగి cif->flags;
}

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం, ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
		      శూన్య *user_దత్తాంశం, శూన్య *codeloc)
{
  గుర్తులేని పూర్ణాంకం *tramp = (గుర్తులేని పూర్ణాంకం *) codeloc;
  శూన్య *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_సంవృతం_struct_OBSD;
  లేకపోతే
    fn = &ffi_సంవృతం_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((గుర్తులేని పూర్ణాంకం)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((గుర్తులేని పూర్ణాంకం)సంవృతం >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((గుర్తులేని పూర్ణాంకం)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((గుర్తులేని పూర్ణాంకం)సంవృతం & 0xffff);

  ffi_cacheflush_OBSD((గుర్తులేని పూర్ణాంకం)codeloc, FFI_TRAMPOLINE_కొలత);

  సంవృతం->cif  = cif;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  సంవృతం->fun  = fun;

  తిరిగి FFI_OK;
}
