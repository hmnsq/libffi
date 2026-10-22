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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_prep_args(ଅକ୍ଷର *ଗଦା, extended_cif *ecif)
{
	ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପଞ୍ଜିକରଣ ଶୂନ୍ୟ **p_argv;
	ପଞ୍ଜିକରଣ ଅକ୍ଷର *argp;
	ପଞ୍ଜିକରଣ ffi_type **p_arg;

	argp = ଗଦା;

	/* Store return value */
	ଯଦି ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(ଶୂନ୍ୟ **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	ପାଇଁ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->size;
		argp -= z;

		/* Align if necessary */
		argp = (ଅକ୍ଷର *) FFI_ALIGN_DOWN(FFI_ALIGN_DOWN(argp, (*p_arg)->alignment), 4);

		ଯଦି (z < ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)) {
			z = ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
			ବାଛ ((*p_arg)->type)
			{
			ସ୍ଥିତି FFI_TYPE_SINT8:
				*(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT8 *)(* p_argv);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_UINT8:
				*(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT8 *)(* p_argv);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_SINT16:
				*(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT16 *)(* p_argv);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_UINT16:
				*(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT16 *)(* p_argv);
			ସ୍ଥିତି FFI_TYPE_STRUCT:
				memcpy(argp, *p_argv, (*p_arg)->size);
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				FFI_ASSERT(0);
			}
		} ଅନ୍ୟଥା ଯଦି ( z == ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)) {
			*(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT32 *)(* p_argv);
		} ଅନ୍ୟଥା {
			memcpy(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	ଫେରନ୍ତୁ FFI_ALIGN(MIN(ଗଦା - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	unsigned i, bytes = 0;

	ପାଇଁ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		ଯଦି ((*ptr)->size == 0)
			ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		ଯଦି (((*ptr)->alignment - 1) & bytes)
			bytes = FFI_ALIGN(bytes, (*ptr)->alignment);

		bytes += FFI_ALIGN((*ptr)->size, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	bytes = FFI_ALIGN(bytes, 8);

	/* Make space for the return structure pointer */
	ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT) {
		bytes += ଆକାର(ଶୂନ୍ୟ*);

		/* Ensure stack is aligned to an 8-byte boundary */
		bytes = FFI_ALIGN(bytes, 8);
	}

	cif->bytes = bytes;

	/* Set the return type flag */
	ବାଛ (cif->rtype->type) {
	ସ୍ଥିତି FFI_TYPE_VOID:
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
		cif->flags = (unsigned) cif->rtype->type;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_UINT64:
		cif->flags = (unsigned) FFI_TYPE_SINT64;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		ଯଦି (cif->rtype->size <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (unsigned)FFI_TYPE_INT;
		ଅନ୍ୟଥା ଯଦି ((cif->rtype->size > 4) && (cif->rtype->size <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (unsigned)FFI_TYPE_DOUBLE;
		ଅନ୍ୟଥା
			/* value stored in memory */
			cif->flags = (unsigned)FFI_TYPE_STRUCT;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		cif->flags = (unsigned)FFI_TYPE_INT;
		ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV(ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), extended_cif *, unsigned, unsigned, ଦ୍ୱିଗୁଣ *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
ଶୂନ୍ୟ ffi_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
	extended_cif ecif;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	ଦ୍ୱିଗୁଣ temp;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	ଯଦି ((rvalue == ଖାଲି ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->size);
	ଅନ୍ୟଥା ଯଦି (small_struct)
		ecif.rvalue = &temp;
	ଅନ୍ୟଥା
		ecif.rvalue = rvalue;

	ବାଛ (cif->abi) {
	ସ୍ଥିତି FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		FFI_ASSERT(0);
		ଭାଙ୍ଗ;
	}

	ଯଦି (small_struct)
		memcpy (rvalue, &temp, cif->rtype->size);
}

/* private members */

ସ୍ଥିର ଶୂନ୍ୟ ffi_prep_incoming_args_SYSV (ଅକ୍ଷର *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ **,
	ffi_cif*, ଦଶମିକ *);

ଶୂନ୍ୟ ffi_closure_SYSV (ffi_closure *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
ବାହ୍ୟ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
ଶୂନ୍ୟ ffi_init_trampoline(unsigned ଅକ୍ଷର *__tramp, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା __fun, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା __ctx) {
	memcpy (__tramp, ffi_metag_trampoline, ଆକାର(ffi_metag_trampoline));
	*(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[40] = __ctx;
	*(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
	ffi_cif* cif,
	ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
	ଶୂନ୍ୟ *user_data,
	ଶୂନ୍ୟ *codeloc)
{
	ଶୂନ୍ୟ (*closure_func)(ffi_closure*) = ଖାଲି;

	ଯଦି (cif->abi == FFI_SYSV)
		closure_func = &ffi_closure_SYSV;
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ FFI_BAD_ABI;

	ffi_init_trampoline(
		(unsigned ଅକ୍ଷର*)&closure->tramp[0],
		(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)closure_func,
		(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)codeloc);

	closure->cif = cif;
	closure->user_data = user_data;
	closure->fun = fun;

	ଫେରନ୍ତୁ FFI_OK;
}


/* This function is jumped to by the trampoline */
unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_closure_SYSV_inner (closure, respp, args, vfp_args)
	ffi_closure *closure;
	ଶୂନ୍ୟ **respp;
	ଶୂନ୍ୟ *args;
	ଶୂନ୍ୟ *vfp_args;
{
	ffi_cif *cif;
	ଶୂନ୍ୟ **arg_area;

	cif = closure->cif;
	arg_area = (ଶୂନ୍ୟ**) alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif, vfp_args);

	(closure->fun) ( cif, *respp, arg_area, closure->user_data);

	ଫେରନ୍ତୁ cif->flags;
}

ସ୍ଥିର ଶୂନ୍ୟ ffi_prep_incoming_args_SYSV(ଅକ୍ଷର *ଗଦା, ଶୂନ୍ୟ **rvalue,
	ଶୂନ୍ୟ **avalue, ffi_cif *cif,
	ଦଶମିକ *vfp_ଗଦା)
{
	ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପଞ୍ଜିକରଣ ଶୂନ୍ୟ **p_argv;
	ପଞ୍ଜିକରଣ ଅକ୍ଷର *argp;
	ପଞ୍ଜିକରଣ ffi_type **p_arg;

	/* stack points to original arguments */
	argp = ଗଦା;

	/* Store return value */
	ଯଦି ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(ଶୂନ୍ୟ **) argp;
	}

	p_argv = avalue;

	ପାଇଁ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		ଯଦି (alignment < 4)
			alignment = 4;
		ଯଦି ((alignment - 1) & (unsigned)argp)
			argp = (ଅକ୍ଷର *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->size;
		*p_argv = (ଶୂନ୍ୟ*) argp;
		p_argv++;
		argp -= z;
	}
	ଫେରନ୍ତୁ;
}
