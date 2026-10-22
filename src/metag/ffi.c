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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, extended_cif *ecif)
{
	ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
	ਰਜਿਸਟਰ ਅੱਖਰ *argp;
	ਰਜਿਸਟਰ ffi_type **p_arg;

	argp = ਢੇਰ;

	/* Store return value */
	ਜੇ ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(ਖਾਲੀ **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	ਲਈ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->ਮਾਪ;
		argp -= z;

		/* Align if necessary */
		argp = (ਅੱਖਰ *) FFI_ALIGN_ਥੱਲੇ(FFI_ALIGN_ਥੱਲੇ(argp, (*p_arg)->alignment), 4);

		ਜੇ (z < ਆਕਾਰ(ਪੂਰਨ)) {
			z = ਆਕਾਰ(ਪੂਰਨ);
			ਚੋਣ ((*p_arg)->type)
			{
			ਹਾਲਤ FFI_TYPE_SINT8:
				*(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT8 *)(* p_argv);
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_UINT8:
				*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT8 *)(* p_argv);
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_SINT16:
				*(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT16 *)(* p_argv);
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_UINT16:
				*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT16 *)(* p_argv);
			ਹਾਲਤ FFI_TYPE_STRUCT:
				ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, (*p_arg)->ਮਾਪ);
				ਰੋਕੋ;
			ਮੂਲ:
				FFI_ASSERT(0);
			}
		} ਨਹੀਂਤਾਂ ਜੇ ( z == ਆਕਾਰ(ਪੂਰਨ)) {
			*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT32 *)(* p_argv);
		} ਨਹੀਂਤਾਂ {
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	ਵਾਪਸ FFI_ALIGN(MIN(ਢੇਰ - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	ਬਿਨਾਨਿਸ਼ਾਨੀ i, ਬਾਈਟਾਂ = 0;

	ਲਈ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		ਜੇ ((*ptr)->ਮਾਪ == 0)
			ਵਾਪਸ FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		ਜੇ (((*ptr)->alignment - 1) & ਬਾਈਟਾਂ)
			ਬਾਈਟਾਂ = FFI_ALIGN(ਬਾਈਟਾਂ, (*ptr)->alignment);

		ਬਾਈਟਾਂ += FFI_ALIGN((*ptr)->ਮਾਪ, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	ਬਾਈਟਾਂ = FFI_ALIGN(ਬਾਈਟਾਂ, 8);

	/* Make space for the return structure pointer */
	ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT) {
		ਬਾਈਟਾਂ += ਆਕਾਰ(ਖਾਲੀ*);

		/* Ensure stack is aligned to an 8-byte boundary */
		ਬਾਈਟਾਂ = FFI_ALIGN(ਬਾਈਟਾਂ, 8);
	}

	cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;

	/* Set the return type flag */
	ਚੋਣ (cif->rtype->type) {
	ਹਾਲਤ FFI_TYPE_VOID:
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
		cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ) cif->rtype->type;
		ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
		cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ) FFI_TYPE_SINT64;
		ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		ਜੇ (cif->rtype->ਮਾਪ <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_INT;
		ਨਹੀਂਤਾਂ ਜੇ ((cif->rtype->ਮਾਪ > 4) && (cif->rtype->ਮਾਪ <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_DOUBLE;
		ਨਹੀਂਤਾਂ
			/* value stored in memory */
			cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_STRUCT;
		ਰੋਕੋ;
	ਮੂਲ:
		cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_INT;
		ਰੋਕੋ;
	}
	ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV(ਖਾਲੀ (*fn)(ਖਾਲੀ), extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਦੁੱਗਣਾ *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
ਖਾਲੀ ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
	extended_cif ecif;

	ਪੂਰਨ small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	ਦੁੱਗਣਾ ਆਰਜ਼ੀ;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	ਜੇ ((rvalue == ਨਿਰਾ ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->ਮਾਪ);
	ਨਹੀਂਤਾਂ ਜੇ (small_struct)
		ecif.rvalue = &ਆਰਜ਼ੀ;
	ਨਹੀਂਤਾਂ
		ecif.rvalue = rvalue;

	ਚੋਣ (cif->abi) {
	ਹਾਲਤ FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->ਬਾਈਟਾਂ, cif->flags, ecif.rvalue);
		ਰੋਕੋ;
	ਮੂਲ:
		FFI_ASSERT(0);
		ਰੋਕੋ;
	}

	ਜੇ (small_struct)
		ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, &ਆਰਜ਼ੀ, cif->rtype->ਮਾਪ);
}

/* private members */

ਸਥਿਰ ਖਾਲੀ ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV (ਅੱਖਰ *, ਖਾਲੀ **, ਖਾਲੀ **,
	ffi_cif*, ਦਸ਼ਮਲਵ *);

ਖਾਲੀ ffi_ਘੇਰਾ_SYSV (ffi_ਘੇਰਾ *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
ਬਾਹਰੀ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
ਖਾਲੀ ffi_init_trampoline(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *__tramp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ __fun, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ __ctx) {
	ਸਮ੍ਰਿਤੀ_ਨਕਲ (__tramp, ffi_metag_trampoline, ਆਕਾਰ(ffi_metag_trampoline));
	*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[40] = __ctx;
	*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ *ਘੇਰਾ,
	ffi_cif* cif,
	ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
	ਖਾਲੀ *user_ਅੰਕੜੇ,
	ਖਾਲੀ *codeloc)
{
	ਖਾਲੀ (*ਘੇਰਾ_func)(ffi_ਘੇਰਾ*) = ਨਿਰਾ;

	ਜੇ (cif->abi == FFI_SYSV)
		ਘੇਰਾ_func = &ffi_ਘੇਰਾ_SYSV;
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ FFI_BAD_ABI;

	ffi_init_trampoline(
		(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)&ਘੇਰਾ->tramp[0],
		(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਘੇਰਾ_func,
		(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)codeloc);

	ਘੇਰਾ->cif = cif;
	ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
	ਘੇਰਾ->fun = fun;

	ਵਾਪਸ FFI_OK;
}


/* This function is jumped to by the trampoline */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ffi_ਘੇਰਾ_SYSV_inner (ਘੇਰਾ, respp, ਦਲੀਲਾਂ, vfp_ਦਲੀਲਾਂ)
	ffi_ਘੇਰਾ *ਘੇਰਾ;
	ਖਾਲੀ **respp;
	ਖਾਲੀ *ਦਲੀਲਾਂ;
	ਖਾਲੀ *vfp_ਦਲੀਲਾਂ;
{
	ffi_cif *cif;
	ਖਾਲੀ **arg_area;

	cif = ਘੇਰਾ->cif;
	arg_area = (ਖਾਲੀ**) alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV(ਦਲੀਲਾਂ, respp, arg_area, cif, vfp_ਦਲੀਲਾਂ);

	(ਘੇਰਾ->fun) ( cif, *respp, arg_area, ਘੇਰਾ->user_ਅੰਕੜੇ);

	ਵਾਪਸ cif->flags;
}

ਸਥਿਰ ਖਾਲੀ ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV(ਅੱਖਰ *ਢੇਰ, ਖਾਲੀ **rvalue,
	ਖਾਲੀ **avalue, ffi_cif *cif,
	ਦਸ਼ਮਲਵ *vfp_ਢੇਰ)
{
	ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
	ਰਜਿਸਟਰ ਅੱਖਰ *argp;
	ਰਜਿਸਟਰ ffi_type **p_arg;

	/* stack points to original arguments */
	argp = ਢੇਰ;

	/* Store return value */
	ਜੇ ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(ਖਾਲੀ **) argp;
	}

	p_argv = avalue;

	ਲਈ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		ਜੇ (alignment < 4)
			alignment = 4;
		ਜੇ ((alignment - 1) & (ਬਿਨਾਨਿਸ਼ਾਨੀ)argp)
			argp = (ਅੱਖਰ *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->ਮਾਪ;
		*p_argv = (ਖਾਲੀ*) argp;
		p_argv++;
		argp -= z;
	}
	ਵਾਪਸ;
}
