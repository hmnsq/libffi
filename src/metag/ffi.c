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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

unsigned முழு ffi_prep_args(எழுத்து *அடுக்கு, extended_cif *ecif)
{
	பதிவு unsigned முழு i;
	பதிவு வெறுமை **p_argv;
	பதிவு எழுத்து *argp;
	பதிவு ffi_type **p_arg;

	argp = அடுக்கு;

	/* Store return value */
	எனில் ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(வெறுமை **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	க்கு (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->size;
		argp -= z;

		/* Align if necessary */
		argp = (எழுத்து *) FFI_ALIGN_DOWN(FFI_ALIGN_DOWN(argp, (*p_arg)->alignment), 4);

		எனில் (z < அளவு(முழு)) {
			z = அளவு(முழு);
			தேர்வு ((*p_arg)->type)
			{
			நிலை FFI_TYPE_SINT8:
				*(signed முழு *) argp = (signed முழு)*(SINT8 *)(* p_argv);
				நிறுத்து;
			நிலை FFI_TYPE_UINT8:
				*(unsigned முழு *) argp = (unsigned முழு)*(UINT8 *)(* p_argv);
				நிறுத்து;
			நிலை FFI_TYPE_SINT16:
				*(signed முழு *) argp = (signed முழு)*(SINT16 *)(* p_argv);
				நிறுத்து;
			நிலை FFI_TYPE_UINT16:
				*(unsigned முழு *) argp = (unsigned முழு)*(UINT16 *)(* p_argv);
			நிலை FFI_TYPE_STRUCT:
				memcpy(argp, *p_argv, (*p_arg)->size);
				நிறுத்து;
			இயல்பு:
				FFI_ASSERT(0);
			}
		} இல்லையெனில் எனில் ( z == அளவு(முழு)) {
			*(unsigned முழு *) argp = (unsigned முழு)*(UINT32 *)(* p_argv);
		} இல்லையெனில் {
			memcpy(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	திரும்பு FFI_ALIGN(MIN(அடுக்கு - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	unsigned i, bytes = 0;

	க்கு (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		எனில் ((*ptr)->size == 0)
			திரும்பு FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		எனில் (((*ptr)->alignment - 1) & bytes)
			bytes = FFI_ALIGN(bytes, (*ptr)->alignment);

		bytes += FFI_ALIGN((*ptr)->size, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	bytes = FFI_ALIGN(bytes, 8);

	/* Make space for the return structure pointer */
	எனில் (cif->rtype->type == FFI_TYPE_STRUCT) {
		bytes += அளவு(வெறுமை*);

		/* Ensure stack is aligned to an 8-byte boundary */
		bytes = FFI_ALIGN(bytes, 8);
	}

	cif->bytes = bytes;

	/* Set the return type flag */
	தேர்வு (cif->rtype->type) {
	நிலை FFI_TYPE_VOID:
	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_DOUBLE:
		cif->flags = (unsigned) cif->rtype->type;
		நிறுத்து;
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
		cif->flags = (unsigned) FFI_TYPE_SINT64;
		நிறுத்து;
	நிலை FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		எனில் (cif->rtype->size <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (unsigned)FFI_TYPE_INT;
		இல்லையெனில் எனில் ((cif->rtype->size > 4) && (cif->rtype->size <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (unsigned)FFI_TYPE_DOUBLE;
		இல்லையெனில்
			/* value stored in memory */
			cif->flags = (unsigned)FFI_TYPE_STRUCT;
		நிறுத்து;
	இயல்பு:
		cif->flags = (unsigned)FFI_TYPE_INT;
		நிறுத்து;
	}
	திரும்பு FFI_OK;
}

வெளி வெறுமை ffi_call_SYSV(வெறுமை (*fn)(வெறுமை), extended_cif *, unsigned, unsigned, இரட்டை *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
வெறுமை ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
	extended_cif ecif;

	முழு small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	இரட்டை temp;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	எனில் ((rvalue == வெற்று ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->size);
	இல்லையெனில் எனில் (small_struct)
		ecif.rvalue = &temp;
	இல்லையெனில்
		ecif.rvalue = rvalue;

	தேர்வு (cif->abi) {
	நிலை FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
		நிறுத்து;
	இயல்பு:
		FFI_ASSERT(0);
		நிறுத்து;
	}

	எனில் (small_struct)
		memcpy (rvalue, &temp, cif->rtype->size);
}

/* private members */

நிலையான வெறுமை ffi_prep_incoming_args_SYSV (எழுத்து *, வெறுமை **, வெறுமை **,
	ffi_cif*, தசம *);

வெறுமை ffi_closure_SYSV (ffi_closure *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
வெளி unsigned முழு ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
வெறுமை ffi_init_trampoline(unsigned எழுத்து *__tramp, unsigned முழு __fun, unsigned முழு __ctx) {
	memcpy (__tramp, ffi_metag_trampoline, அளவு(ffi_metag_trampoline));
	*(unsigned முழு*) &__tramp[40] = __ctx;
	*(unsigned முழு*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
	ffi_cif* cif,
	வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
	வெறுமை *user_data,
	வெறுமை *codeloc)
{
	வெறுமை (*closure_func)(ffi_closure*) = வெற்று;

	எனில் (cif->abi == FFI_SYSV)
		closure_func = &ffi_closure_SYSV;
	இல்லையெனில்
		திரும்பு FFI_BAD_ABI;

	ffi_init_trampoline(
		(unsigned எழுத்து*)&closure->tramp[0],
		(unsigned முழு)closure_func,
		(unsigned முழு)codeloc);

	closure->cif = cif;
	closure->user_data = user_data;
	closure->fun = fun;

	திரும்பு FFI_OK;
}


/* This function is jumped to by the trampoline */
unsigned முழு ffi_closure_SYSV_inner (closure, respp, args, vfp_args)
	ffi_closure *closure;
	வெறுமை **respp;
	வெறுமை *args;
	வெறுமை *vfp_args;
{
	ffi_cif *cif;
	வெறுமை **arg_area;

	cif = closure->cif;
	arg_area = (வெறுமை**) alloca (cif->nargs * அளவு (வெறுமை*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif, vfp_args);

	(closure->fun) ( cif, *respp, arg_area, closure->user_data);

	திரும்பு cif->flags;
}

நிலையான வெறுமை ffi_prep_incoming_args_SYSV(எழுத்து *அடுக்கு, வெறுமை **rvalue,
	வெறுமை **avalue, ffi_cif *cif,
	தசம *vfp_அடுக்கு)
{
	பதிவு unsigned முழு i;
	பதிவு வெறுமை **p_argv;
	பதிவு எழுத்து *argp;
	பதிவு ffi_type **p_arg;

	/* stack points to original arguments */
	argp = அடுக்கு;

	/* Store return value */
	எனில் ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(வெறுமை **) argp;
	}

	p_argv = avalue;

	க்கு (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		எனில் (alignment < 4)
			alignment = 4;
		எனில் ((alignment - 1) & (unsigned)argp)
			argp = (எழுத்து *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->size;
		*p_argv = (வெறுமை*) argp;
		p_argv++;
		argp -= z;
	}
	திரும்பு;
}
