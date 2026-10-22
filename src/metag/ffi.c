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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>

#تعریف_کرو MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

غیرعلامتی صحیح ffi_prep_دلائل(حرف *ڈھیر, extended_cif *ecif)
{
	اندراج غیرعلامتی صحیح i;
	اندراج خالی **p_argv;
	اندراج حرف *argp;
	اندراج ffi_type **p_arg;

	argp = ڈھیر;

	/* Store return value */
	اگر ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(خالی **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	برائے (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->سائز;
		argp -= z;

		/* Align if necessary */
		argp = (حرف *) FFI_ALIGN_نیچے_کو(FFI_ALIGN_نیچے_کو(argp, (*p_arg)->alignment), 4);

		اگر (z < حجم(صحیح)) {
			z = حجم(صحیح);
			انتخاب ((*p_arg)->type)
			{
			حالت FFI_TYPE_SINT8:
				*(علامتی صحیح *) argp = (علامتی صحیح)*(SINT8 *)(* p_argv);
				توڑو;
			حالت FFI_TYPE_UINT8:
				*(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT8 *)(* p_argv);
				توڑو;
			حالت FFI_TYPE_SINT16:
				*(علامتی صحیح *) argp = (علامتی صحیح)*(SINT16 *)(* p_argv);
				توڑو;
			حالت FFI_TYPE_UINT16:
				*(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT16 *)(* p_argv);
			حالت FFI_TYPE_STRUCT:
				حافظہ_نقل(argp, *p_argv, (*p_arg)->سائز);
				توڑو;
			طےشدہ:
				FFI_ASSERT(0);
			}
		} ورنہ اگر ( z == حجم(صحیح)) {
			*(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT32 *)(* p_argv);
		} ورنہ {
			حافظہ_نقل(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	واپس FFI_ALIGN(MIN(ڈھیر - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	غیرعلامتی i, بائٹیں = 0;

	برائے (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		اگر ((*ptr)->سائز == 0)
			واپس FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		اگر (((*ptr)->alignment - 1) & بائٹیں)
			بائٹیں = FFI_ALIGN(بائٹیں, (*ptr)->alignment);

		بائٹیں += FFI_ALIGN((*ptr)->سائز, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	بائٹیں = FFI_ALIGN(بائٹیں, 8);

	/* Make space for the return structure pointer */
	اگر (cif->rtype->type == FFI_TYPE_STRUCT) {
		بائٹیں += حجم(خالی*);

		/* Ensure stack is aligned to an 8-byte boundary */
		بائٹیں = FFI_ALIGN(بائٹیں, 8);
	}

	cif->بائٹیں = بائٹیں;

	/* Set the return type flag */
	انتخاب (cif->rtype->type) {
	حالت FFI_TYPE_VOID:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
		cif->flags = (غیرعلامتی) cif->rtype->type;
		توڑو;
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
		cif->flags = (غیرعلامتی) FFI_TYPE_SINT64;
		توڑو;
	حالت FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		اگر (cif->rtype->سائز <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (غیرعلامتی)FFI_TYPE_INT;
		ورنہ اگر ((cif->rtype->سائز > 4) && (cif->rtype->سائز <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (غیرعلامتی)FFI_TYPE_DOUBLE;
		ورنہ
			/* value stored in memory */
			cif->flags = (غیرعلامتی)FFI_TYPE_STRUCT;
		توڑو;
	طےشدہ:
		cif->flags = (غیرعلامتی)FFI_TYPE_INT;
		توڑو;
	}
	واپس FFI_OK;
}

خارجی خالی ffi_call_SYSV(خالی (*fn)(خالی), extended_cif *, غیرعلامتی, غیرعلامتی, دوہرا *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
خالی ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
	extended_cif ecif;

	صحیح small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	دوہرا عارضی;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	اگر ((rvalue == تہی ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->سائز);
	ورنہ اگر (small_struct)
		ecif.rvalue = &عارضی;
	ورنہ
		ecif.rvalue = rvalue;

	انتخاب (cif->abi) {
	حالت FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->بائٹیں, cif->flags, ecif.rvalue);
		توڑو;
	طےشدہ:
		FFI_ASSERT(0);
		توڑو;
	}

	اگر (small_struct)
		حافظہ_نقل (rvalue, &عارضی, cif->rtype->سائز);
}

/* private members */

ساکن خالی ffi_prep_incoming_دلائل_SYSV (حرف *, خالی **, خالی **,
	ffi_cif*, اعشاری *);

خالی ffi_احاطہ_SYSV (ffi_احاطہ *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
خارجی غیرعلامتی صحیح ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
خالی ffi_init_trampoline(غیرعلامتی حرف *__tramp, غیرعلامتی صحیح __fun, غیرعلامتی صحیح __ctx) {
	حافظہ_نقل (__tramp, ffi_metag_trampoline, حجم(ffi_metag_trampoline));
	*(غیرعلامتی صحیح*) &__tramp[40] = __ctx;
	*(غیرعلامتی صحیح*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ *احاطہ,
	ffi_cif* cif,
	خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
	خالی *user_ڈیٹا,
	خالی *codeloc)
{
	خالی (*احاطہ_func)(ffi_احاطہ*) = تہی;

	اگر (cif->abi == FFI_SYSV)
		احاطہ_func = &ffi_احاطہ_SYSV;
	ورنہ
		واپس FFI_BAD_ABI;

	ffi_init_trampoline(
		(غیرعلامتی حرف*)&احاطہ->tramp[0],
		(غیرعلامتی صحیح)احاطہ_func,
		(غیرعلامتی صحیح)codeloc);

	احاطہ->cif = cif;
	احاطہ->user_ڈیٹا = user_ڈیٹا;
	احاطہ->fun = fun;

	واپس FFI_OK;
}


/* This function is jumped to by the trampoline */
غیرعلامتی صحیح ffi_احاطہ_SYSV_inner (احاطہ, respp, دلائل, vfp_دلائل)
	ffi_احاطہ *احاطہ;
	خالی **respp;
	خالی *دلائل;
	خالی *vfp_دلائل;
{
	ffi_cif *cif;
	خالی **arg_area;

	cif = احاطہ->cif;
	arg_area = (خالی**) alloca (cif->nargs * حجم (خالی*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_دلائل_SYSV(دلائل, respp, arg_area, cif, vfp_دلائل);

	(احاطہ->fun) ( cif, *respp, arg_area, احاطہ->user_ڈیٹا);

	واپس cif->flags;
}

ساکن خالی ffi_prep_incoming_دلائل_SYSV(حرف *ڈھیر, خالی **rvalue,
	خالی **avalue, ffi_cif *cif,
	اعشاری *vfp_ڈھیر)
{
	اندراج غیرعلامتی صحیح i;
	اندراج خالی **p_argv;
	اندراج حرف *argp;
	اندراج ffi_type **p_arg;

	/* stack points to original arguments */
	argp = ڈھیر;

	/* Store return value */
	اگر ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(خالی **) argp;
	}

	p_argv = avalue;

	برائے (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		اگر (alignment < 4)
			alignment = 4;
		اگر ((alignment - 1) & (غیرعلامتی)argp)
			argp = (حرف *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->سائز;
		*p_argv = (خالی*) argp;
		p_argv++;
		argp -= z;
	}
	واپس;
}
