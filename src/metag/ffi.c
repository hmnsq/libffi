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

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

unsigned puur_naa_nka ffi_prep_args(var_na *stack, extended_cif *ecif)
{
	pa_njikaa unsigned puur_naa_nka i;
	pa_njikaa void **p_argv;
	pa_njikaa var_na *argp;
	pa_njikaa ffi_type **p_arg;

	argp = stack;

	/* Store return value */
	ပေႃး ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(void **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	for (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->size;
		argp -= z;

		/* Align if necessary */
		argp = (var_na *) FFI_ALIGN_DOWN(FFI_ALIGN_DOWN(argp, (*p_arg)->alignment), 4);

		ပေႃး (z < parimaa_na(puur_naa_nka)) {
			z = parimaa_na(puur_naa_nka);
			switch ((*p_arg)->type)
			{
			pak_sa FFI_TYPE_SINT8:
				*(signed puur_naa_nka *) argp = (signed puur_naa_nka)*(SINT8 *)(* p_argv);
				bha_nga;
			pak_sa FFI_TYPE_UINT8:
				*(unsigned puur_naa_nka *) argp = (unsigned puur_naa_nka)*(UINT8 *)(* p_argv);
				bha_nga;
			pak_sa FFI_TYPE_SINT16:
				*(signed puur_naa_nka *) argp = (signed puur_naa_nka)*(SINT16 *)(* p_argv);
				bha_nga;
			pak_sa FFI_TYPE_UINT16:
				*(unsigned puur_naa_nka *) argp = (unsigned puur_naa_nka)*(UINT16 *)(* p_argv);
			pak_sa FFI_TYPE_STRUCT:
				memcpy(argp, *p_argv, (*p_arg)->size);
				bha_nga;
			default:
				FFI_ASSERT(0);
			}
		} ပေႃးဢမ်ႇ ပေႃး ( z == parimaa_na(puur_naa_nka)) {
			*(unsigned puur_naa_nka *) argp = (unsigned puur_naa_nka)*(UINT32 *)(* p_argv);
		} ပေႃးဢမ်ႇ {
			memcpy(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	ပွၵ်ႈ FFI_ALIGN(MIN(stack - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	unsigned i, bytes = 0;

	for (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		ပေႃး ((*ptr)->size == 0)
			ပွၵ်ႈ FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		ပေႃး (((*ptr)->alignment - 1) & bytes)
			bytes = FFI_ALIGN(bytes, (*ptr)->alignment);

		bytes += FFI_ALIGN((*ptr)->size, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	bytes = FFI_ALIGN(bytes, 8);

	/* Make space for the return structure pointer */
	ပေႃး (cif->rtype->type == FFI_TYPE_STRUCT) {
		bytes += parimaa_na(void*);

		/* Ensure stack is aligned to an 8-byte boundary */
		bytes = FFI_ALIGN(bytes, 8);
	}

	cif->bytes = bytes;

	/* Set the return type flag */
	switch (cif->rtype->type) {
	pak_sa FFI_TYPE_VOID:
	pak_sa FFI_TYPE_FLOAT:
	pak_sa FFI_TYPE_DOUBLE:
		cif->flags = (unsigned) cif->rtype->type;
		bha_nga;
	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_UINT64:
		cif->flags = (unsigned) FFI_TYPE_SINT64;
		bha_nga;
	pak_sa FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		ပေႃး (cif->rtype->size <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (unsigned)FFI_TYPE_INT;
		ပေႃးဢမ်ႇ ပေႃး ((cif->rtype->size > 4) && (cif->rtype->size <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (unsigned)FFI_TYPE_DOUBLE;
		ပေႃးဢမ်ႇ
			/* value stored in memory */
			cif->flags = (unsigned)FFI_TYPE_STRUCT;
		bha_nga;
	default:
		cif->flags = (unsigned)FFI_TYPE_INT;
		bha_nga;
	}
	ပွၵ်ႈ FFI_OK;
}

extern void ffi_call_SYSV(void (*fn)(void), extended_cif *, unsigned, unsigned, dviguu_na *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
	extended_cif ecif;

	puur_naa_nka small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	dviguu_na temp;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	ပေႃး ((rvalue == NULL ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->size);
	ပေႃးဢမ်ႇ ပေႃး (small_struct)
		ecif.rvalue = &temp;
	ပေႃးဢမ်ႇ
		ecif.rvalue = rvalue;

	switch (cif->abi) {
	pak_sa FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
		bha_nga;
	default:
		FFI_ASSERT(0);
		bha_nga;
	}

	ပေႃး (small_struct)
		memcpy (rvalue, &temp, cif->rtype->size);
}

/* private members */

static void ffi_prep_incoming_args_SYSV (var_na *, void **, void **,
	ffi_cif*, float *);

void ffi_closure_SYSV (ffi_closure *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
extern unsigned puur_naa_nka ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
void ffi_init_trampoline(unsigned var_na *__tramp, unsigned puur_naa_nka __fun, unsigned puur_naa_nka __ctx) {
	memcpy (__tramp, ffi_metag_trampoline, parimaa_na(ffi_metag_trampoline));
	*(unsigned puur_naa_nka*) &__tramp[40] = __ctx;
	*(unsigned puur_naa_nka*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
	ffi_cif* cif,
	void (*fun)(ffi_cif*,void*,void**,void*),
	void *user_data,
	void *codeloc)
{
	void (*closure_func)(ffi_closure*) = NULL;

	ပေႃး (cif->abi == FFI_SYSV)
		closure_func = &ffi_closure_SYSV;
	ပေႃးဢမ်ႇ
		ပွၵ်ႈ FFI_BAD_ABI;

	ffi_init_trampoline(
		(unsigned var_na*)&closure->tramp[0],
		(unsigned puur_naa_nka)closure_func,
		(unsigned puur_naa_nka)codeloc);

	closure->cif = cif;
	closure->user_data = user_data;
	closure->fun = fun;

	ပွၵ်ႈ FFI_OK;
}


/* This function is jumped to by the trampoline */
unsigned puur_naa_nka ffi_closure_SYSV_inner (closure, respp, args, vfp_args)
	ffi_closure *closure;
	void **respp;
	void *args;
	void *vfp_args;
{
	ffi_cif *cif;
	void **arg_area;

	cif = closure->cif;
	arg_area = (void**) alloca (cif->nargs * parimaa_na (void*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif, vfp_args);

	(closure->fun) ( cif, *respp, arg_area, closure->user_data);

	ပွၵ်ႈ cif->flags;
}

static void ffi_prep_incoming_args_SYSV(var_na *stack, void **rvalue,
	void **avalue, ffi_cif *cif,
	float *vfp_stack)
{
	pa_njikaa unsigned puur_naa_nka i;
	pa_njikaa void **p_argv;
	pa_njikaa var_na *argp;
	pa_njikaa ffi_type **p_arg;

	/* stack points to original arguments */
	argp = stack;

	/* Store return value */
	ပေႃး ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(void **) argp;
	}

	p_argv = avalue;

	for (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		ပေႃး (alignment < 4)
			alignment = 4;
		ပေႃး ((alignment - 1) & (unsigned)argp)
			argp = (var_na *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->size;
		*p_argv = (void*) argp;
		p_argv++;
		argp -= z;
	}
	ပွၵ်ႈ;
}
