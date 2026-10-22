/* ----------------------------------------------------------------------
  ffi.c - Copyright (c) 2013 Imagination Technologies

  Meta Foreign Function Interface
  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  `Software''), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED `AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL SIMON POSNJAK BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
----------------------------------------------------------------------- */

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>

#నిర్వచించు MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

గుర్తులేని పూర్ణాంకం ffi_prep_వాదనలు(అక్షరం *దొంతర, extended_cif *ecif)
{
	నమోదు గుర్తులేని పూర్ణాంకం i;
	నమోదు శూన్య **p_argv;
	నమోదు అక్షరం *argp;
	నమోదు ffi_type **p_arg;

	argp = దొంతర;

	/* Store return value */
	ఒకవేళ ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(శూన్య **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	కొరకు (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->కొలత;
		argp -= z;

		/* Align if necessary */
		argp = (అక్షరం *) FFI_ALIGN_కిందికి(FFI_ALIGN_కిందికి(argp, (*p_arg)->alignment), 4);

		ఒకవేళ (z < పరిమాణం(పూర్ణాంకం)) {
			z = పరిమాణం(పూర్ణాంకం);
			ఎంపిక ((*p_arg)->type)
			{
			స్థితి FFI_TYPE_SINT8:
				*(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT8 *)(* p_argv);
				ఆపు;
			స్థితి FFI_TYPE_UINT8:
				*(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT8 *)(* p_argv);
				ఆపు;
			స్థితి FFI_TYPE_SINT16:
				*(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT16 *)(* p_argv);
				ఆపు;
			స్థితి FFI_TYPE_UINT16:
				*(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT16 *)(* p_argv);
			స్థితి FFI_TYPE_STRUCT:
				జ్ఞాపకం_నకలు(argp, *p_argv, (*p_arg)->కొలత);
				ఆపు;
			అప్రమేయ:
				FFI_ASSERT(0);
			}
		} లేకపోతే ఒకవేళ ( z == పరిమాణం(పూర్ణాంకం)) {
			*(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT32 *)(* p_argv);
		} లేకపోతే {
			జ్ఞాపకం_నకలు(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	తిరిగి FFI_ALIGN(MIN(దొంతర - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	గుర్తులేని i, బైట్లు = 0;

	కొరకు (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		ఒకవేళ ((*ptr)->కొలత == 0)
			తిరిగి FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		ఒకవేళ (((*ptr)->alignment - 1) & బైట్లు)
			బైట్లు = FFI_ALIGN(బైట్లు, (*ptr)->alignment);

		బైట్లు += FFI_ALIGN((*ptr)->కొలత, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	బైట్లు = FFI_ALIGN(బైట్లు, 8);

	/* Make space for the return structure pointer */
	ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT) {
		బైట్లు += పరిమాణం(శూన్య*);

		/* Ensure stack is aligned to an 8-byte boundary */
		బైట్లు = FFI_ALIGN(బైట్లు, 8);
	}

	cif->బైట్లు = బైట్లు;

	/* Set the return type flag */
	ఎంపిక (cif->rtype->type) {
	స్థితి FFI_TYPE_VOID:
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_DOUBLE:
		cif->flags = (గుర్తులేని) cif->rtype->type;
		ఆపు;
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
		cif->flags = (గుర్తులేని) FFI_TYPE_SINT64;
		ఆపు;
	స్థితి FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		ఒకవేళ (cif->rtype->కొలత <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (గుర్తులేని)FFI_TYPE_INT;
		లేకపోతే ఒకవేళ ((cif->rtype->కొలత > 4) && (cif->rtype->కొలత <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (గుర్తులేని)FFI_TYPE_DOUBLE;
		లేకపోతే
			/* value stored in memory */
			cif->flags = (గుర్తులేని)FFI_TYPE_STRUCT;
		ఆపు;
	అప్రమేయ:
		cif->flags = (గుర్తులేని)FFI_TYPE_INT;
		ఆపు;
	}
	తిరిగి FFI_OK;
}

బాహ్య శూన్య ffi_call_SYSV(శూన్య (*fn)(శూన్య), extended_cif *, గుర్తులేని, గుర్తులేని, ద్విగుణ *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
శూన్య ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
	extended_cif ecif;

	పూర్ణాంకం small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	ద్విగుణ తాత్కాలిక;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	ఒకవేళ ((rvalue == ఖాళీ ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->కొలత);
	లేకపోతే ఒకవేళ (small_struct)
		ecif.rvalue = &తాత్కాలిక;
	లేకపోతే
		ecif.rvalue = rvalue;

	ఎంపిక (cif->abi) {
	స్థితి FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->బైట్లు, cif->flags, ecif.rvalue);
		ఆపు;
	అప్రమేయ:
		FFI_ASSERT(0);
		ఆపు;
	}

	ఒకవేళ (small_struct)
		జ్ఞాపకం_నకలు (rvalue, &తాత్కాలిక, cif->rtype->కొలత);
}

/* private members */

స్థిర శూన్య ffi_prep_incoming_వాదనలు_SYSV (అక్షరం *, శూన్య **, శూన్య **,
	ffi_cif*, దశాంశం *);

శూన్య ffi_సంవృతం_SYSV (ffi_సంవృతం *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
బాహ్య గుర్తులేని పూర్ణాంకం ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
శూన్య ffi_init_trampoline(గుర్తులేని అక్షరం *__tramp, గుర్తులేని పూర్ణాంకం __fun, గుర్తులేని పూర్ణాంకం __ctx) {
	జ్ఞాపకం_నకలు (__tramp, ffi_metag_trampoline, పరిమాణం(ffi_metag_trampoline));
	*(గుర్తులేని పూర్ణాంకం*) &__tramp[40] = __ctx;
	*(గుర్తులేని పూర్ణాంకం*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం *సంవృతం,
	ffi_cif* cif,
	శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
	శూన్య *user_దత్తాంశం,
	శూన్య *codeloc)
{
	శూన్య (*సంవృతం_func)(ffi_సంవృతం*) = ఖాళీ;

	ఒకవేళ (cif->abi == FFI_SYSV)
		సంవృతం_func = &ffi_సంవృతం_SYSV;
	లేకపోతే
		తిరిగి FFI_BAD_ABI;

	ffi_init_trampoline(
		(గుర్తులేని అక్షరం*)&సంవృతం->tramp[0],
		(గుర్తులేని పూర్ణాంకం)సంవృతం_func,
		(గుర్తులేని పూర్ణాంకం)codeloc);

	సంవృతం->cif = cif;
	సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
	సంవృతం->fun = fun;

	తిరిగి FFI_OK;
}


/* This function is jumped to by the trampoline */
గుర్తులేని పూర్ణాంకం ffi_సంవృతం_SYSV_inner (సంవృతం, respp, వాదనలు, vfp_వాదనలు)
	ffi_సంవృతం *సంవృతం;
	శూన్య **respp;
	శూన్య *వాదనలు;
	శూన్య *vfp_వాదనలు;
{
	ffi_cif *cif;
	శూన్య **arg_area;

	cif = సంవృతం->cif;
	arg_area = (శూన్య**) alloca (cif->nargs * పరిమాణం (శూన్య*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_వాదనలు_SYSV(వాదనలు, respp, arg_area, cif, vfp_వాదనలు);

	(సంవృతం->fun) ( cif, *respp, arg_area, సంవృతం->user_దత్తాంశం);

	తిరిగి cif->flags;
}

స్థిర శూన్య ffi_prep_incoming_వాదనలు_SYSV(అక్షరం *దొంతర, శూన్య **rvalue,
	శూన్య **avalue, ffi_cif *cif,
	దశాంశం *vfp_దొంతర)
{
	నమోదు గుర్తులేని పూర్ణాంకం i;
	నమోదు శూన్య **p_argv;
	నమోదు అక్షరం *argp;
	నమోదు ffi_type **p_arg;

	/* stack points to original arguments */
	argp = దొంతర;

	/* Store return value */
	ఒకవేళ ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(శూన్య **) argp;
	}

	p_argv = avalue;

	కొరకు (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		ఒకవేళ (alignment < 4)
			alignment = 4;
		ఒకవేళ ((alignment - 1) & (గుర్తులేని)argp)
			argp = (అక్షరం *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->కొలత;
		*p_argv = (శూన్య*) argp;
		p_argv++;
		argp -= z;
	}
	తిరిగి;
}
