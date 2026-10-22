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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

unsigned પૂર્ણાંક ffi_prep_args(અક્ષર *ઢગલો, extended_cif *ecif)
{
	નોંધણી unsigned પૂર્ણાંક i;
	નોંધણી શૂન્ય **p_argv;
	નોંધણી અક્ષર *argp;
	નોંધણી ffi_type **p_arg;

	argp = ઢગલો;

	/* Store return value */
	જો ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(શૂન્ય **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	માટે (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->size;
		argp -= z;

		/* Align if necessary */
		argp = (અક્ષર *) FFI_ALIGN_DOWN(FFI_ALIGN_DOWN(argp, (*p_arg)->alignment), 4);

		જો (z < કદ(પૂર્ણાંક)) {
			z = કદ(પૂર્ણાંક);
			પસંદગી ((*p_arg)->type)
			{
			સ્થિતિ FFI_TYPE_SINT8:
				*(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT8 *)(* p_argv);
				તોડો;
			સ્થિતિ FFI_TYPE_UINT8:
				*(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT8 *)(* p_argv);
				તોડો;
			સ્થિતિ FFI_TYPE_SINT16:
				*(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT16 *)(* p_argv);
				તોડો;
			સ્થિતિ FFI_TYPE_UINT16:
				*(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT16 *)(* p_argv);
			સ્થિતિ FFI_TYPE_STRUCT:
				memcpy(argp, *p_argv, (*p_arg)->size);
				તોડો;
			મૂળભૂત:
				FFI_ASSERT(0);
			}
		} નહીંતર જો ( z == કદ(પૂર્ણાંક)) {
			*(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT32 *)(* p_argv);
		} નહીંતર {
			memcpy(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	પાછા FFI_ALIGN(MIN(ઢગલો - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	unsigned i, bytes = 0;

	માટે (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		જો ((*ptr)->size == 0)
			પાછા FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		જો (((*ptr)->alignment - 1) & bytes)
			bytes = FFI_ALIGN(bytes, (*ptr)->alignment);

		bytes += FFI_ALIGN((*ptr)->size, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	bytes = FFI_ALIGN(bytes, 8);

	/* Make space for the return structure pointer */
	જો (cif->rtype->type == FFI_TYPE_STRUCT) {
		bytes += કદ(શૂન્ય*);

		/* Ensure stack is aligned to an 8-byte boundary */
		bytes = FFI_ALIGN(bytes, 8);
	}

	cif->bytes = bytes;

	/* Set the return type flag */
	પસંદગી (cif->rtype->type) {
	સ્થિતિ FFI_TYPE_VOID:
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_DOUBLE:
		cif->flags = (unsigned) cif->rtype->type;
		તોડો;
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
		cif->flags = (unsigned) FFI_TYPE_SINT64;
		તોડો;
	સ્થિતિ FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		જો (cif->rtype->size <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (unsigned)FFI_TYPE_INT;
		નહીંતર જો ((cif->rtype->size > 4) && (cif->rtype->size <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (unsigned)FFI_TYPE_DOUBLE;
		નહીંતર
			/* value stored in memory */
			cif->flags = (unsigned)FFI_TYPE_STRUCT;
		તોડો;
	મૂળભૂત:
		cif->flags = (unsigned)FFI_TYPE_INT;
		તોડો;
	}
	પાછા FFI_OK;
}

બાહ્ય શૂન્ય ffi_call_SYSV(શૂન્ય (*fn)(શૂન્ય), extended_cif *, unsigned, unsigned, દ્વિગુણ *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
શૂન્ય ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
	extended_cif ecif;

	પૂર્ણાંક small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	દ્વિગુણ temp;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	જો ((rvalue == ખાલી ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->size);
	નહીંતર જો (small_struct)
		ecif.rvalue = &temp;
	નહીંતર
		ecif.rvalue = rvalue;

	પસંદગી (cif->abi) {
	સ્થિતિ FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
		તોડો;
	મૂળભૂત:
		FFI_ASSERT(0);
		તોડો;
	}

	જો (small_struct)
		memcpy (rvalue, &temp, cif->rtype->size);
}

/* private members */

સ્થિર શૂન્ય ffi_prep_incoming_args_SYSV (અક્ષર *, શૂન્ય **, શૂન્ય **,
	ffi_cif*, દશાંશ *);

શૂન્ય ffi_closure_SYSV (ffi_closure *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
બાહ્ય unsigned પૂર્ણાંક ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
શૂન્ય ffi_init_trampoline(unsigned અક્ષર *__tramp, unsigned પૂર્ણાંક __fun, unsigned પૂર્ણાંક __ctx) {
	memcpy (__tramp, ffi_metag_trampoline, કદ(ffi_metag_trampoline));
	*(unsigned પૂર્ણાંક*) &__tramp[40] = __ctx;
	*(unsigned પૂર્ણાંક*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
	ffi_cif* cif,
	શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
	શૂન્ય *user_data,
	શૂન્ય *codeloc)
{
	શૂન્ય (*closure_func)(ffi_closure*) = ખાલી;

	જો (cif->abi == FFI_SYSV)
		closure_func = &ffi_closure_SYSV;
	નહીંતર
		પાછા FFI_BAD_ABI;

	ffi_init_trampoline(
		(unsigned અક્ષર*)&closure->tramp[0],
		(unsigned પૂર્ણાંક)closure_func,
		(unsigned પૂર્ણાંક)codeloc);

	closure->cif = cif;
	closure->user_data = user_data;
	closure->fun = fun;

	પાછા FFI_OK;
}


/* This function is jumped to by the trampoline */
unsigned પૂર્ણાંક ffi_closure_SYSV_inner (closure, respp, args, vfp_args)
	ffi_closure *closure;
	શૂન્ય **respp;
	શૂન્ય *args;
	શૂન્ય *vfp_args;
{
	ffi_cif *cif;
	શૂન્ય **arg_area;

	cif = closure->cif;
	arg_area = (શૂન્ય**) alloca (cif->nargs * કદ (શૂન્ય*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif, vfp_args);

	(closure->fun) ( cif, *respp, arg_area, closure->user_data);

	પાછા cif->flags;
}

સ્થિર શૂન્ય ffi_prep_incoming_args_SYSV(અક્ષર *ઢગલો, શૂન્ય **rvalue,
	શૂન્ય **avalue, ffi_cif *cif,
	દશાંશ *vfp_ઢગલો)
{
	નોંધણી unsigned પૂર્ણાંક i;
	નોંધણી શૂન્ય **p_argv;
	નોંધણી અક્ષર *argp;
	નોંધણી ffi_type **p_arg;

	/* stack points to original arguments */
	argp = ઢગલો;

	/* Store return value */
	જો ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(શૂન્ય **) argp;
	}

	p_argv = avalue;

	માટે (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		જો (alignment < 4)
			alignment = 4;
		જો ((alignment - 1) & (unsigned)argp)
			argp = (અક્ષર *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->size;
		*p_argv = (શૂન્ય*) argp;
		p_argv++;
		argp -= z;
	}
	પાછા;
}
