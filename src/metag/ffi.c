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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

unsigned पूर्णाङ्क ffi_prep_args(वर्ण *ढेर, extended_cif *ecif)
{
	पञ्जिका unsigned पूर्णाङ्क i;
	पञ्जिका शून्य **p_argv;
	पञ्जिका वर्ण *argp;
	पञ्जिका ffi_type **p_arg;

	argp = ढेर;

	/* Store return value */
	यदि ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(शून्य **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	प्रति (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		size_t z;

		/* Move argp to address of argument */
		z = (*p_arg)->size;
		argp -= z;

		/* Align if necessary */
		argp = (वर्ण *) FFI_ALIGN_DOWN(FFI_ALIGN_DOWN(argp, (*p_arg)->alignment), 4);

		यदि (z < परिमाण(पूर्णाङ्क)) {
			z = परिमाण(पूर्णाङ्क);
			विकल्प ((*p_arg)->type)
			{
			पक्ष FFI_TYPE_SINT8:
				*(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क)*(SINT8 *)(* p_argv);
				भङ्ग;
			पक्ष FFI_TYPE_UINT8:
				*(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT8 *)(* p_argv);
				भङ्ग;
			पक्ष FFI_TYPE_SINT16:
				*(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क)*(SINT16 *)(* p_argv);
				भङ्ग;
			पक्ष FFI_TYPE_UINT16:
				*(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT16 *)(* p_argv);
			पक्ष FFI_TYPE_STRUCT:
				memcpy(argp, *p_argv, (*p_arg)->size);
				भङ्ग;
			स्वभाव:
				FFI_ASSERT(0);
			}
		} अन्यथा यदि ( z == परिमाण(पूर्णाङ्क)) {
			*(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT32 *)(* p_argv);
		} अन्यथा {
			memcpy(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	प्रत्यागम FFI_ALIGN(MIN(ढेर - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	unsigned i, bytes = 0;

	प्रति (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		यदि ((*ptr)->size == 0)
			प्रत्यागम FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		यदि (((*ptr)->alignment - 1) & bytes)
			bytes = FFI_ALIGN(bytes, (*ptr)->alignment);

		bytes += FFI_ALIGN((*ptr)->size, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	bytes = FFI_ALIGN(bytes, 8);

	/* Make space for the return structure pointer */
	यदि (cif->rtype->type == FFI_TYPE_STRUCT) {
		bytes += परिमाण(शून्य*);

		/* Ensure stack is aligned to an 8-byte boundary */
		bytes = FFI_ALIGN(bytes, 8);
	}

	cif->bytes = bytes;

	/* Set the return type flag */
	विकल्प (cif->rtype->type) {
	पक्ष FFI_TYPE_VOID:
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
		cif->flags = (unsigned) cif->rtype->type;
		भङ्ग;
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
		cif->flags = (unsigned) FFI_TYPE_SINT64;
		भङ्ग;
	पक्ष FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		यदि (cif->rtype->size <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (unsigned)FFI_TYPE_INT;
		अन्यथा यदि ((cif->rtype->size > 4) && (cif->rtype->size <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (unsigned)FFI_TYPE_DOUBLE;
		अन्यथा
			/* value stored in memory */
			cif->flags = (unsigned)FFI_TYPE_STRUCT;
		भङ्ग;
	स्वभाव:
		cif->flags = (unsigned)FFI_TYPE_INT;
		भङ्ग;
	}
	प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_call_SYSV(शून्य (*fn)(शून्य), extended_cif *, unsigned, unsigned, द्विगुण *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
	extended_cif ecif;

	पूर्णाङ्क small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	द्विगुण temp;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	यदि ((rvalue == रिक्त ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->size);
	अन्यथा यदि (small_struct)
		ecif.rvalue = &temp;
	अन्यथा
		ecif.rvalue = rvalue;

	विकल्प (cif->abi) {
	पक्ष FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
		भङ्ग;
	स्वभाव:
		FFI_ASSERT(0);
		भङ्ग;
	}

	यदि (small_struct)
		memcpy (rvalue, &temp, cif->rtype->size);
}

/* private members */

स्थिर शून्य ffi_prep_incoming_args_SYSV (वर्ण *, शून्य **, शून्य **,
	ffi_cif*, भिन्न *);

शून्य ffi_closure_SYSV (ffi_closure *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
बाह्य unsigned पूर्णाङ्क ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
शून्य ffi_init_trampoline(unsigned वर्ण *__tramp, unsigned पूर्णाङ्क __fun, unsigned पूर्णाङ्क __ctx) {
	memcpy (__tramp, ffi_metag_trampoline, परिमाण(ffi_metag_trampoline));
	*(unsigned पूर्णाङ्क*) &__tramp[40] = __ctx;
	*(unsigned पूर्णाङ्क*) &__tramp[44] = __fun;
	/* This will flush the instruction cache */
	__builtin_meta2_cachewd(&__tramp[0], 1);
	__builtin_meta2_cachewd(&__tramp[47], 1);
}



/* the cif must already be prepared */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
	ffi_cif* cif,
	शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
	शून्य *user_data,
	शून्य *codeloc)
{
	शून्य (*closure_func)(ffi_closure*) = रिक्त;

	यदि (cif->abi == FFI_SYSV)
		closure_func = &ffi_closure_SYSV;
	अन्यथा
		प्रत्यागम FFI_BAD_ABI;

	ffi_init_trampoline(
		(unsigned वर्ण*)&closure->tramp[0],
		(unsigned पूर्णाङ्क)closure_func,
		(unsigned पूर्णाङ्क)codeloc);

	closure->cif = cif;
	closure->user_data = user_data;
	closure->fun = fun;

	प्रत्यागम FFI_OK;
}


/* This function is jumped to by the trampoline */
unsigned पूर्णाङ्क ffi_closure_SYSV_inner (closure, respp, args, vfp_args)
	ffi_closure *closure;
	शून्य **respp;
	शून्य *args;
	शून्य *vfp_args;
{
	ffi_cif *cif;
	शून्य **arg_area;

	cif = closure->cif;
	arg_area = (शून्य**) alloca (cif->nargs * परिमाण (शून्य*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif, vfp_args);

	(closure->fun) ( cif, *respp, arg_area, closure->user_data);

	प्रत्यागम cif->flags;
}

स्थिर शून्य ffi_prep_incoming_args_SYSV(वर्ण *ढेर, शून्य **rvalue,
	शून्य **avalue, ffi_cif *cif,
	भिन्न *vfp_ढेर)
{
	पञ्जिका unsigned पूर्णाङ्क i;
	पञ्जिका शून्य **p_argv;
	पञ्जिका वर्ण *argp;
	पञ्जिका ffi_type **p_arg;

	/* stack points to original arguments */
	argp = ढेर;

	/* Store return value */
	यदि ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(शून्य **) argp;
	}

	p_argv = avalue;

	प्रति (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		size_t z;
		size_t alignment;

		alignment = (*p_arg)->alignment;
		यदि (alignment < 4)
			alignment = 4;
		यदि ((alignment - 1) & (unsigned)argp)
			argp = (वर्ण *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->size;
		*p_argv = (शून्य*) argp;
		p_argv++;
		argp -= z;
	}
	प्रत्यागम;
}
