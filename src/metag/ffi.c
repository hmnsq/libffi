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

unsigned ကိန်းပြည့် ffi_prep_args(var_na *stack, extended_cif *ecif)
{
	pa_njikaa unsigned ကိန်းပြည့် i;
	pa_njikaa void **p_argv;
	pa_njikaa var_na *argp;
	pa_njikaa ffi_type **p_arg;

	argp = stack;

	/* Store return value */
	အကယ်၍ ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(void **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	အတွက် (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->size;
		argp -= z;

		/* Align if necessary */
		argp = (var_na *) FFI_ALIGN_DOWN(FFI_ALIGN_DOWN(argp, (*p_arg)->alignment), 4);

		အကယ်၍ (z < parimaa_na(ကိန်းပြည့်)) {
			z = parimaa_na(ကိန်းပြည့်);
			switch ((*p_arg)->type)
			{
			pak_sa FFI_TYPE_SINT8:
				*(signed ကိန်းပြည့် *) argp = (signed ကိန်းပြည့်)*(SINT8 *)(* p_argv);
				bha_nga;
			pak_sa FFI_TYPE_UINT8:
				*(unsigned ကိန်းပြည့် *) argp = (unsigned ကိန်းပြည့်)*(UINT8 *)(* p_argv);
				bha_nga;
			pak_sa FFI_TYPE_SINT16:
				*(signed ကိန်းပြည့် *) argp = (signed ကိန်းပြည့်)*(SINT16 *)(* p_argv);
				bha_nga;
			pak_sa FFI_TYPE_UINT16:
				*(unsigned ကိန်းပြည့် *) argp = (unsigned ကိန်းပြည့်)*(UINT16 *)(* p_argv);
			pak_sa FFI_TYPE_STRUCT:
				memcpy(argp, *p_argv, (*p_arg)->size);
				bha_nga;
			default:
				FFI_ASSERT(0);
			}
		} မဟုတ်ပါက အကယ်၍ ( z == parimaa_na(ကိန်းပြည့်)) {
			*(unsigned ကိန်းပြည့် *) argp = (unsigned ကိန်းပြည့်)*(UINT32 *)(* p_argv);
		} မဟုတ်ပါက {
			memcpy(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	ပြန်ပေး FFI_ALIGN(MIN(stack - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	unsigned i, bytes = 0;

	အတွက် (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		အကယ်၍ ((*ptr)->size == 0)
			ပြန်ပေး FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		အကယ်၍ (((*ptr)->alignment - 1) & bytes)
			bytes = FFI_ALIGN(bytes, (*ptr)->alignment);

		bytes += FFI_ALIGN((*ptr)->size, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	bytes = FFI_ALIGN(bytes, 8);

	/* Make space for the return structure pointer */
	အကယ်၍ (cif->rtype->type == FFI_TYPE_STRUCT) {
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
		အကယ်၍ (cif->rtype->size <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (unsigned)FFI_TYPE_INT;
		မဟုတ်ပါက အကယ်၍ ((cif->rtype->size > 4) && (cif->rtype->size <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (unsigned)FFI_TYPE_DOUBLE;
		မဟုတ်ပါက
			/* value stored in memory */
			cif->flags = (unsigned)FFI_TYPE_STRUCT;
		bha_nga;
	default:
		cif->flags = (unsigned)FFI_TYPE_INT;
		bha_nga;
	}
	ပြန်ပေး FFI_OK;
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

	ကိန်းပြည့် small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	dviguu_na temp;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	အကယ်၍ ((rvalue == NULL ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->size);
	မဟုတ်ပါက အကယ်၍ (small_struct)
		ecif.rvalue = &temp;
	မဟုတ်ပါက
		ecif.rvalue = rvalue;

	switch (cif->abi) {
	pak_sa FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
		bha_nga;
	default:
		FFI_ASSERT(0);
		bha_nga;
	}

	အကယ်၍ (small_struct)
		memcpy (rvalue, &temp, cif->rtype->size);
}

/* private members */

static void ffi_prep_incoming_args_SYSV (var_na *, void **, void **,
	ffi_cif*, float *);

void ffi_closure_SYSV (ffi_closure *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
extern unsigned ကိန်းပြည့် ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
void ffi_init_trampoline(unsigned var_na *__tramp, unsigned ကိန်းပြည့် __fun, unsigned ကိန်းပြည့် __ctx) {
	memcpy (__tramp, ffi_metag_trampoline, parimaa_na(ffi_metag_trampoline));
	*(unsigned ကိန်းပြည့်*) &__tramp[40] = __ctx;
	*(unsigned ကိန်းပြည့်*) &__tramp[44] = __fun;
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

	အကယ်၍ (cif->abi == FFI_SYSV)
		closure_func = &ffi_closure_SYSV;
	မဟုတ်ပါက
		ပြန်ပေး FFI_BAD_ABI;

	ffi_init_trampoline(
		(unsigned var_na*)&closure->tramp[0],
		(unsigned ကိန်းပြည့်)closure_func,
		(unsigned ကိန်းပြည့်)codeloc);

	closure->cif = cif;
	closure->user_data = user_data;
	closure->fun = fun;

	ပြန်ပေး FFI_OK;
}


/* This function is jumped to by the trampoline */
unsigned ကိန်းပြည့် ffi_closure_SYSV_inner (closure, respp, args, vfp_args)
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

	ပြန်ပေး cif->flags;
}

static void ffi_prep_incoming_args_SYSV(var_na *stack, void **rvalue,
	void **avalue, ffi_cif *cif,
	float *vfp_stack)
{
	pa_njikaa unsigned ကိန်းပြည့် i;
	pa_njikaa void **p_argv;
	pa_njikaa var_na *argp;
	pa_njikaa ffi_type **p_arg;

	/* stack points to original arguments */
	argp = stack;

	/* Store return value */
	အကယ်၍ ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(void **) argp;
	}

	p_argv = avalue;

	အတွက် (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		အကယ်၍ (alignment < 4)
			alignment = 4;
		အကယ်၍ ((alignment - 1) & (unsigned)argp)
			argp = (var_na *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->size;
		*p_argv = (void*) argp;
		p_argv++;
		argp -= z;
	}
	ပြန်ပေး;
}
