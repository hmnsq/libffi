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

#وصف_ڪريو MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

بنانشان صحيح ffi_prep_دليلون(حرف *ڍير, extended_cif *ecif)
{
	سجل بنانشان صحيح i;
	سجل فراغ **p_argv;
	سجل حرف *argp;
	سجل ffi_type **p_arg;

	argp = ڍير;

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
		z = (*p_arg)->سائيز;
		argp -= z;

		/* Align if necessary */
		argp = (حرف *) FFI_ALIGN_هيٺ_ڏانهن(FFI_ALIGN_هيٺ_ڏانهن(argp, (*p_arg)->alignment), 4);

		إذا (z < حجم(صحيح)) {
			z = حجم(صحيح);
			بدل ((*p_arg)->type)
			{
			حالة FFI_TYPE_SINT8:
				*(نشانوارو صحيح *) argp = (نشانوارو صحيح)*(SINT8 *)(* p_argv);
				اكسر;
			حالة FFI_TYPE_UINT8:
				*(بنانشان صحيح *) argp = (بنانشان صحيح)*(UINT8 *)(* p_argv);
				اكسر;
			حالة FFI_TYPE_SINT16:
				*(نشانوارو صحيح *) argp = (نشانوارو صحيح)*(SINT16 *)(* p_argv);
				اكسر;
			حالة FFI_TYPE_UINT16:
				*(بنانشان صحيح *) argp = (بنانشان صحيح)*(UINT16 *)(* p_argv);
			حالة FFI_TYPE_STRUCT:
				يادگيري_نقل(argp, *p_argv, (*p_arg)->سائيز);
				اكسر;
			افتراضي:
				FFI_ASSERT(0);
			}
		} وإلا إذا ( z == حجم(صحيح)) {
			*(بنانشان صحيح *) argp = (بنانشان صحيح)*(UINT32 *)(* p_argv);
		} وإلا {
			يادگيري_نقل(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	أرجع FFI_ALIGN(MIN(ڍير - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	بنانشان i, بائيٽون = 0;

	لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		إذا ((*ptr)->سائيز == 0)
			أرجع FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		إذا (((*ptr)->alignment - 1) & بائيٽون)
			بائيٽون = FFI_ALIGN(بائيٽون, (*ptr)->alignment);

		بائيٽون += FFI_ALIGN((*ptr)->سائيز, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	بائيٽون = FFI_ALIGN(بائيٽون, 8);

	/* Make space for the return structure pointer */
	إذا (cif->rtype->type == FFI_TYPE_STRUCT) {
		بائيٽون += حجم(فراغ*);

		/* Ensure stack is aligned to an 8-byte boundary */
		بائيٽون = FFI_ALIGN(بائيٽون, 8);
	}

	cif->بائيٽون = بائيٽون;

	/* Set the return type flag */
	بدل (cif->rtype->type) {
	حالة FFI_TYPE_VOID:
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
		cif->flags = (بنانشان) cif->rtype->type;
		اكسر;
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
		cif->flags = (بنانشان) FFI_TYPE_SINT64;
		اكسر;
	حالة FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		إذا (cif->rtype->سائيز <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (بنانشان)FFI_TYPE_INT;
		وإلا إذا ((cif->rtype->سائيز > 4) && (cif->rtype->سائيز <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (بنانشان)FFI_TYPE_DOUBLE;
		وإلا
			/* value stored in memory */
			cif->flags = (بنانشان)FFI_TYPE_STRUCT;
		اكسر;
	افتراضي:
		cif->flags = (بنانشان)FFI_TYPE_INT;
		اكسر;
	}
	أرجع FFI_OK;
}

خارجي فراغ ffi_call_SYSV(فراغ (*fn)(فراغ), extended_cif *, بنانشان, بنانشان, مزدوج *);

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

	مزدوج عارضي;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	إذا ((rvalue == عدم ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->سائيز);
	وإلا إذا (small_struct)
		ecif.rvalue = &عارضي;
	وإلا
		ecif.rvalue = rvalue;

	بدل (cif->abi) {
	حالة FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->بائيٽون, cif->flags, ecif.rvalue);
		اكسر;
	افتراضي:
		FFI_ASSERT(0);
		اكسر;
	}

	إذا (small_struct)
		يادگيري_نقل (rvalue, &عارضي, cif->rtype->سائيز);
}

/* private members */

ساكن فراغ ffi_prep_incoming_دليلون_SYSV (حرف *, فراغ **, فراغ **,
	ffi_cif*, عائم *);

فراغ ffi_گهيرو_SYSV (ffi_گهيرو *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
خارجي بنانشان صحيح ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
فراغ ffi_init_trampoline(بنانشان حرف *__tramp, بنانشان صحيح __fun, بنانشان صحيح __ctx) {
	يادگيري_نقل (__tramp, ffi_metag_trampoline, حجم(ffi_metag_trampoline));
	*(بنانشان صحيح*) &__tramp[40] = __ctx;
	*(بنانشان صحيح*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو *گهيرو,
	ffi_cif* cif,
	فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
	فراغ *user_معلومات,
	فراغ *codeloc)
{
	فراغ (*گهيرو_func)(ffi_گهيرو*) = عدم;

	إذا (cif->abi == FFI_SYSV)
		گهيرو_func = &ffi_گهيرو_SYSV;
	وإلا
		أرجع FFI_BAD_ABI;

	ffi_init_trampoline(
		(بنانشان حرف*)&گهيرو->tramp[0],
		(بنانشان صحيح)گهيرو_func,
		(بنانشان صحيح)codeloc);

	گهيرو->cif = cif;
	گهيرو->user_معلومات = user_معلومات;
	گهيرو->fun = fun;

	أرجع FFI_OK;
}


/* This function is jumped to by the trampoline */
بنانشان صحيح ffi_گهيرو_SYSV_inner (گهيرو, respp, دليلون, vfp_دليلون)
	ffi_گهيرو *گهيرو;
	فراغ **respp;
	فراغ *دليلون;
	فراغ *vfp_دليلون;
{
	ffi_cif *cif;
	فراغ **arg_area;

	cif = گهيرو->cif;
	arg_area = (فراغ**) alloca (cif->nargs * حجم (فراغ*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_دليلون_SYSV(دليلون, respp, arg_area, cif, vfp_دليلون);

	(گهيرو->fun) ( cif, *respp, arg_area, گهيرو->user_معلومات);

	أرجع cif->flags;
}

ساكن فراغ ffi_prep_incoming_دليلون_SYSV(حرف *ڍير, فراغ **rvalue,
	فراغ **avalue, ffi_cif *cif,
	عائم *vfp_ڍير)
{
	سجل بنانشان صحيح i;
	سجل فراغ **p_argv;
	سجل حرف *argp;
	سجل ffi_type **p_arg;

	/* stack points to original arguments */
	argp = ڍير;

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
		إذا ((alignment - 1) & (بنانشان)argp)
			argp = (حرف *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->سائيز;
		*p_argv = (فراغ*) argp;
		p_argv++;
		argp -= z;
	}
	أرجع;
}
