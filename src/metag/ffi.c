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

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

unsigned صحیح ffi_prep_args(نویسه *پشته, extended_cif *ecif)
{
	ثبات unsigned صحیح i;
	ثبات تهی **p_argv;
	ثبات نویسه *argp;
	ثبات ffi_type **p_arg;

	argp = پشته;

	/* Store return value */
	اگر ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(تهی **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	برای (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->size;
		argp -= z;

		/* Align if necessary */
		argp = (نویسه *) FFI_ALIGN_DOWN(FFI_ALIGN_DOWN(argp, (*p_arg)->alignment), 4);

		اگر (z < اندازه(صحیح)) {
			z = اندازه(صحیح);
			انتخاب ((*p_arg)->type)
			{
			حالت FFI_TYPE_SINT8:
				*(signed صحیح *) argp = (signed صحیح)*(SINT8 *)(* p_argv);
				بشکن;
			حالت FFI_TYPE_UINT8:
				*(unsigned صحیح *) argp = (unsigned صحیح)*(UINT8 *)(* p_argv);
				بشکن;
			حالت FFI_TYPE_SINT16:
				*(signed صحیح *) argp = (signed صحیح)*(SINT16 *)(* p_argv);
				بشکن;
			حالت FFI_TYPE_UINT16:
				*(unsigned صحیح *) argp = (unsigned صحیح)*(UINT16 *)(* p_argv);
			حالت FFI_TYPE_STRUCT:
				memcpy(argp, *p_argv, (*p_arg)->size);
				بشکن;
			پیشفرض:
				FFI_ASSERT(0);
			}
		} وگرنه اگر ( z == اندازه(صحیح)) {
			*(unsigned صحیح *) argp = (unsigned صحیح)*(UINT32 *)(* p_argv);
		} وگرنه {
			memcpy(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	بازگشت FFI_ALIGN(MIN(پشته - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	unsigned i, bytes = 0;

	برای (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		اگر ((*ptr)->size == 0)
			بازگشت FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		اگر (((*ptr)->alignment - 1) & bytes)
			bytes = FFI_ALIGN(bytes, (*ptr)->alignment);

		bytes += FFI_ALIGN((*ptr)->size, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	bytes = FFI_ALIGN(bytes, 8);

	/* Make space for the return structure pointer */
	اگر (cif->rtype->type == FFI_TYPE_STRUCT) {
		bytes += اندازه(تهی*);

		/* Ensure stack is aligned to an 8-byte boundary */
		bytes = FFI_ALIGN(bytes, 8);
	}

	cif->bytes = bytes;

	/* Set the return type flag */
	انتخاب (cif->rtype->type) {
	حالت FFI_TYPE_VOID:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
		cif->flags = (unsigned) cif->rtype->type;
		بشکن;
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
		cif->flags = (unsigned) FFI_TYPE_SINT64;
		بشکن;
	حالت FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		اگر (cif->rtype->size <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (unsigned)FFI_TYPE_INT;
		وگرنه اگر ((cif->rtype->size > 4) && (cif->rtype->size <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (unsigned)FFI_TYPE_DOUBLE;
		وگرنه
			/* value stored in memory */
			cif->flags = (unsigned)FFI_TYPE_STRUCT;
		بشکن;
	پیشفرض:
		cif->flags = (unsigned)FFI_TYPE_INT;
		بشکن;
	}
	بازگشت FFI_OK;
}

بیرونی تهی ffi_call_SYSV(تهی (*fn)(تهی), extended_cif *, unsigned, unsigned, دوگانه *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
تهی ffi_call(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
	extended_cif ecif;

	صحیح small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	دوگانه temp;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	اگر ((rvalue == پوچ ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->size);
	وگرنه اگر (small_struct)
		ecif.rvalue = &temp;
	وگرنه
		ecif.rvalue = rvalue;

	انتخاب (cif->abi) {
	حالت FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
		بشکن;
	پیشفرض:
		FFI_ASSERT(0);
		بشکن;
	}

	اگر (small_struct)
		memcpy (rvalue, &temp, cif->rtype->size);
}

/* private members */

ایستا تهی ffi_prep_incoming_args_SYSV (نویسه *, تهی **, تهی **,
	ffi_cif*, اعشاری *);

تهی ffi_closure_SYSV (ffi_closure *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
بیرونی unsigned صحیح ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
تهی ffi_init_trampoline(unsigned نویسه *__tramp, unsigned صحیح __fun, unsigned صحیح __ctx) {
	memcpy (__tramp, ffi_metag_trampoline, اندازه(ffi_metag_trampoline));
	*(unsigned صحیح*) &__tramp[40] = __ctx;
	*(unsigned صحیح*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
	ffi_cif* cif,
	تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*),
	تهی *user_data,
	تهی *codeloc)
{
	تهی (*closure_func)(ffi_closure*) = پوچ;

	اگر (cif->abi == FFI_SYSV)
		closure_func = &ffi_closure_SYSV;
	وگرنه
		بازگشت FFI_BAD_ABI;

	ffi_init_trampoline(
		(unsigned نویسه*)&closure->tramp[0],
		(unsigned صحیح)closure_func,
		(unsigned صحیح)codeloc);

	closure->cif = cif;
	closure->user_data = user_data;
	closure->fun = fun;

	بازگشت FFI_OK;
}


/* This function is jumped to by the trampoline */
unsigned صحیح ffi_closure_SYSV_inner (closure, respp, args, vfp_args)
	ffi_closure *closure;
	تهی **respp;
	تهی *args;
	تهی *vfp_args;
{
	ffi_cif *cif;
	تهی **arg_area;

	cif = closure->cif;
	arg_area = (تهی**) alloca (cif->nargs * اندازه (تهی*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif, vfp_args);

	(closure->fun) ( cif, *respp, arg_area, closure->user_data);

	بازگشت cif->flags;
}

ایستا تهی ffi_prep_incoming_args_SYSV(نویسه *پشته, تهی **rvalue,
	تهی **avalue, ffi_cif *cif,
	اعشاری *vfp_پشته)
{
	ثبات unsigned صحیح i;
	ثبات تهی **p_argv;
	ثبات نویسه *argp;
	ثبات ffi_type **p_arg;

	/* stack points to original arguments */
	argp = پشته;

	/* Store return value */
	اگر ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(تهی **) argp;
	}

	p_argv = avalue;

	برای (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		اگر (alignment < 4)
			alignment = 4;
		اگر ((alignment - 1) & (unsigned)argp)
			argp = (نویسه *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->size;
		*p_argv = (تهی*) argp;
		p_argv++;
		argp -= z;
	}
	بازگشت;
}
