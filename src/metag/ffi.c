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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>

#تعریف_کرو MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

بنانشانی صحيح ffi_prep_دلیلاں(حرف *ڈھیر, extended_cif *ecif)
{
	سجل بنانشانی صحيح i;
	سجل فراغ **p_argv;
	سجل حرف *argp;
	سجل ffi_type **p_arg;

	argp = ڈھیر;

	/* Store return value */
	إذا ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(فراغ **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->ناپ;
		argp -= z;

		/* Align if necessary */
		argp = (حرف *) FFI_ALIGN_تھلے(FFI_ALIGN_تھلے(argp, (*p_arg)->alignment), 4);

		إذا (z < حجم(صحيح)) {
			z = حجم(صحيح);
			بدل ((*p_arg)->type)
			{
			حالة FFI_TYPE_SINT8:
				*(نشانیوالا صحيح *) argp = (نشانیوالا صحيح)*(SINT8 *)(* p_argv);
				اكسر;
			حالة FFI_TYPE_UINT8:
				*(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT8 *)(* p_argv);
				اكسر;
			حالة FFI_TYPE_SINT16:
				*(نشانیوالا صحيح *) argp = (نشانیوالا صحيح)*(SINT16 *)(* p_argv);
				اكسر;
			حالة FFI_TYPE_UINT16:
				*(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT16 *)(* p_argv);
			حالة FFI_TYPE_STRUCT:
				حافظہ_نقل(argp, *p_argv, (*p_arg)->ناپ);
				اكسر;
			افتراضي:
				FFI_ASSERT(0);
			}
		} وإلا إذا ( z == حجم(صحيح)) {
			*(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT32 *)(* p_argv);
		} وإلا {
			حافظہ_نقل(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	أرجع FFI_ALIGN(MIN(ڈھیر - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	بنانشانی i, بائٹاں = 0;

	لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		إذا ((*ptr)->ناپ == 0)
			أرجع FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		إذا (((*ptr)->alignment - 1) & بائٹاں)
			بائٹاں = FFI_ALIGN(بائٹاں, (*ptr)->alignment);

		بائٹاں += FFI_ALIGN((*ptr)->ناپ, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	بائٹاں = FFI_ALIGN(بائٹاں, 8);

	/* Make space for the return structure pointer */
	إذا (cif->rtype->type == FFI_TYPE_STRUCT) {
		بائٹاں += حجم(فراغ*);

		/* Ensure stack is aligned to an 8-byte boundary */
		بائٹاں = FFI_ALIGN(بائٹاں, 8);
	}

	cif->بائٹاں = بائٹاں;

	/* Set the return type flag */
	بدل (cif->rtype->type) {
	حالة FFI_TYPE_VOID:
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
		cif->flags = (بنانشانی) cif->rtype->type;
		اكسر;
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
		cif->flags = (بنانشانی) FFI_TYPE_SINT64;
		اكسر;
	حالة FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		إذا (cif->rtype->ناپ <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (بنانشانی)FFI_TYPE_INT;
		وإلا إذا ((cif->rtype->ناپ > 4) && (cif->rtype->ناپ <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (بنانشانی)FFI_TYPE_DOUBLE;
		وإلا
			/* value stored in memory */
			cif->flags = (بنانشانی)FFI_TYPE_STRUCT;
		اكسر;
	افتراضي:
		cif->flags = (بنانشانی)FFI_TYPE_INT;
		اكسر;
	}
	أرجع FFI_OK;
}

خارجي فراغ ffi_call_SYSV(فراغ (*fn)(فراغ), extended_cif *, بنانشانی, بنانشانی, مزدوج *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
	extended_cif ecif;

	صحيح small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	مزدوج عارضی;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	إذا ((rvalue == عدم ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->ناپ);
	وإلا إذا (small_struct)
		ecif.rvalue = &عارضی;
	وإلا
		ecif.rvalue = rvalue;

	بدل (cif->abi) {
	حالة FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->بائٹاں, cif->flags, ecif.rvalue);
		اكسر;
	افتراضي:
		FFI_ASSERT(0);
		اكسر;
	}

	إذا (small_struct)
		حافظہ_نقل (rvalue, &عارضی, cif->rtype->ناپ);
}

/* private members */

ساكن فراغ ffi_prep_incoming_دلیلاں_SYSV (حرف *, فراغ **, فراغ **,
	ffi_cif*, عائم *);

فراغ ffi_گھیرا_SYSV (ffi_گھیرا *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
خارجي بنانشانی صحيح ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
فراغ ffi_init_trampoline(بنانشانی حرف *__tramp, بنانشانی صحيح __fun, بنانشانی صحيح __ctx) {
	حافظہ_نقل (__tramp, ffi_metag_trampoline, حجم(ffi_metag_trampoline));
	*(بنانشانی صحيح*) &__tramp[40] = __ctx;
	*(بنانشانی صحيح*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا *گھیرا,
	ffi_cif* cif,
	فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
	فراغ *user_انکڑے,
	فراغ *codeloc)
{
	فراغ (*گھیرا_func)(ffi_گھیرا*) = عدم;

	إذا (cif->abi == FFI_SYSV)
		گھیرا_func = &ffi_گھیرا_SYSV;
	وإلا
		أرجع FFI_BAD_ABI;

	ffi_init_trampoline(
		(بنانشانی حرف*)&گھیرا->tramp[0],
		(بنانشانی صحيح)گھیرا_func,
		(بنانشانی صحيح)codeloc);

	گھیرا->cif = cif;
	گھیرا->user_انکڑے = user_انکڑے;
	گھیرا->fun = fun;

	أرجع FFI_OK;
}


/* This function is jumped to by the trampoline */
بنانشانی صحيح ffi_گھیرا_SYSV_inner (گھیرا, respp, دلیلاں, vfp_دلیلاں)
	ffi_گھیرا *گھیرا;
	فراغ **respp;
	فراغ *دلیلاں;
	فراغ *vfp_دلیلاں;
{
	ffi_cif *cif;
	فراغ **arg_area;

	cif = گھیرا->cif;
	arg_area = (فراغ**) alloca (cif->nargs * حجم (فراغ*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_دلیلاں_SYSV(دلیلاں, respp, arg_area, cif, vfp_دلیلاں);

	(گھیرا->fun) ( cif, *respp, arg_area, گھیرا->user_انکڑے);

	أرجع cif->flags;
}

ساكن فراغ ffi_prep_incoming_دلیلاں_SYSV(حرف *ڈھیر, فراغ **rvalue,
	فراغ **avalue, ffi_cif *cif,
	عائم *vfp_ڈھیر)
{
	سجل بنانشانی صحيح i;
	سجل فراغ **p_argv;
	سجل حرف *argp;
	سجل ffi_type **p_arg;

	/* stack points to original arguments */
	argp = ڈھیر;

	/* Store return value */
	إذا ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(فراغ **) argp;
	}

	p_argv = avalue;

	لكل (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		إذا (alignment < 4)
			alignment = 4;
		إذا ((alignment - 1) & (بنانشانی)argp)
			argp = (حرف *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->ناپ;
		*p_argv = (فراغ*) argp;
		p_argv++;
		argp -= z;
	}
	أرجع;
}
