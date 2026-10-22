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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

unsigned ಪೂರ್ಣಾಙ್ಕ ffi_prep_args(ವರ್ಣ *ರಾಶಿ, extended_cif *ecif)
{
	ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಪಞ್ಜಿಕಾ ಶೂನ್ಯ **p_argv;
	ಪಞ್ಜಿಕಾ ವರ್ಣ *argp;
	ಪಞ್ಜಿಕಾ ffi_type **p_arg;

	argp = ರಾಶಿ;

	/* Store return value */
	ಯದಿ ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(ಶೂನ್ಯ **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	ಪ್ರತಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->size;
		argp -= z;

		/* Align if necessary */
		argp = (ವರ್ಣ *) FFI_ALIGN_DOWN(FFI_ALIGN_DOWN(argp, (*p_arg)->alignment), 4);

		ಯದಿ (z < ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)) {
			z = ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ);
			ವಿಕಲ್ಪ ((*p_arg)->type)
			{
			ಪಕ್ಷ FFI_TYPE_SINT8:
				*(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT8 *)(* p_argv);
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_UINT8:
				*(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT8 *)(* p_argv);
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_SINT16:
				*(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT16 *)(* p_argv);
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_UINT16:
				*(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT16 *)(* p_argv);
			ಪಕ್ಷ FFI_TYPE_STRUCT:
				memcpy(argp, *p_argv, (*p_arg)->size);
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
				FFI_ASSERT(0);
			}
		} ಅನ್ಯಥಾ ಯದಿ ( z == ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)) {
			*(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT32 *)(* p_argv);
		} ಅನ್ಯಥಾ {
			memcpy(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	ಪ್ರತ್ಯಾಗಮ FFI_ALIGN(MIN(ರಾಶಿ - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	unsigned i, bytes = 0;

	ಪ್ರತಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		ಯದಿ ((*ptr)->size == 0)
			ಪ್ರತ್ಯಾಗಮ FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		ಯದಿ (((*ptr)->alignment - 1) & bytes)
			bytes = FFI_ALIGN(bytes, (*ptr)->alignment);

		bytes += FFI_ALIGN((*ptr)->size, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	bytes = FFI_ALIGN(bytes, 8);

	/* Make space for the return structure pointer */
	ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT) {
		bytes += ಪರಿಮಾಣ(ಶೂನ್ಯ*);

		/* Ensure stack is aligned to an 8-byte boundary */
		bytes = FFI_ALIGN(bytes, 8);
	}

	cif->bytes = bytes;

	/* Set the return type flag */
	ವಿಕಲ್ಪ (cif->rtype->type) {
	ಪಕ್ಷ FFI_TYPE_VOID:
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
		cif->flags = (unsigned) cif->rtype->type;
		ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
		cif->flags = (unsigned) FFI_TYPE_SINT64;
		ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		ಯದಿ (cif->rtype->size <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (unsigned)FFI_TYPE_INT;
		ಅನ್ಯಥಾ ಯದಿ ((cif->rtype->size > 4) && (cif->rtype->size <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (unsigned)FFI_TYPE_DOUBLE;
		ಅನ್ಯಥಾ
			/* value stored in memory */
			cif->flags = (unsigned)FFI_TYPE_STRUCT;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		cif->flags = (unsigned)FFI_TYPE_INT;
		ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), extended_cif *, unsigned, unsigned, ದ್ವಿಗುಣ *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
	extended_cif ecif;

	ಪೂರ್ಣಾಙ್ಕ small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	ದ್ವಿಗುಣ temp;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	ಯದಿ ((rvalue == ರಿಕ್ತ ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->size);
	ಅನ್ಯಥಾ ಯದಿ (small_struct)
		ecif.rvalue = &temp;
	ಅನ್ಯಥಾ
		ecif.rvalue = rvalue;

	ವಿಕಲ್ಪ (cif->abi) {
	ಪಕ್ಷ FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		FFI_ASSERT(0);
		ಭಙ್ಗ;
	}

	ಯದಿ (small_struct)
		memcpy (rvalue, &temp, cif->rtype->size);
}

/* private members */

ಸ್ಥಿರ ಶೂನ್ಯ ffi_prep_incoming_args_SYSV (ವರ್ಣ *, ಶೂನ್ಯ **, ಶೂನ್ಯ **,
	ffi_cif*, ಭಿನ್ನ *);

ಶೂನ್ಯ ffi_closure_SYSV (ffi_closure *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
ಬಾಹ್ಯ unsigned ಪೂರ್ಣಾಙ್ಕ ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
ಶೂನ್ಯ ffi_init_trampoline(unsigned ವರ್ಣ *__tramp, unsigned ಪೂರ್ಣಾಙ್ಕ __fun, unsigned ಪೂರ್ಣಾಙ್ಕ __ctx) {
	memcpy (__tramp, ffi_metag_trampoline, ಪರಿಮಾಣ(ffi_metag_trampoline));
	*(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[40] = __ctx;
	*(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
	ffi_cif* cif,
	ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
	ಶೂನ್ಯ *user_data,
	ಶೂನ್ಯ *codeloc)
{
	ಶೂನ್ಯ (*closure_func)(ffi_closure*) = ರಿಕ್ತ;

	ಯದಿ (cif->abi == FFI_SYSV)
		closure_func = &ffi_closure_SYSV;
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

	ffi_init_trampoline(
		(unsigned ವರ್ಣ*)&closure->tramp[0],
		(unsigned ಪೂರ್ಣಾಙ್ಕ)closure_func,
		(unsigned ಪೂರ್ಣಾಙ್ಕ)codeloc);

	closure->cif = cif;
	closure->user_data = user_data;
	closure->fun = fun;

	ಪ್ರತ್ಯಾಗಮ FFI_OK;
}


/* This function is jumped to by the trampoline */
unsigned ಪೂರ್ಣಾಙ್ಕ ffi_closure_SYSV_inner (closure, respp, args, vfp_args)
	ffi_closure *closure;
	ಶೂನ್ಯ **respp;
	ಶೂನ್ಯ *args;
	ಶೂನ್ಯ *vfp_args;
{
	ffi_cif *cif;
	ಶೂನ್ಯ **arg_area;

	cif = closure->cif;
	arg_area = (ಶೂನ್ಯ**) alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif, vfp_args);

	(closure->fun) ( cif, *respp, arg_area, closure->user_data);

	ಪ್ರತ್ಯಾಗಮ cif->flags;
}

ಸ್ಥಿರ ಶೂನ್ಯ ffi_prep_incoming_args_SYSV(ವರ್ಣ *ರಾಶಿ, ಶೂನ್ಯ **rvalue,
	ಶೂನ್ಯ **avalue, ffi_cif *cif,
	ಭಿನ್ನ *vfp_ರಾಶಿ)
{
	ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಪಞ್ಜಿಕಾ ಶೂನ್ಯ **p_argv;
	ಪಞ್ಜಿಕಾ ವರ್ಣ *argp;
	ಪಞ್ಜಿಕಾ ffi_type **p_arg;

	/* stack points to original arguments */
	argp = ರಾಶಿ;

	/* Store return value */
	ಯದಿ ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(ಶೂನ್ಯ **) argp;
	}

	p_argv = avalue;

	ಪ್ರತಿ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		ಯದಿ (alignment < 4)
			alignment = 4;
		ಯದಿ ((alignment - 1) & (unsigned)argp)
			argp = (ವರ್ಣ *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->size;
		*p_argv = (ಶೂನ್ಯ*) argp;
		p_argv++;
		argp -= z;
	}
	ಪ್ರತ್ಯಾಗಮ;
}
