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

अचिन्हित पूर्णांक ffi_prep_args(अक्षर *चळत, extended_cif *ecif)
{
	नोंदणी अचिन्हित पूर्णांक i;
	नोंदणी शून्य **p_argv;
	नोंदणी अक्षर *argp;
	नोंदणी ffi_type **p_arg;

	argp = चळत;

	/* Store return value */
	जर ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(शून्य **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	साठी (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		माप_प्रकार z;

		/* Move argp to address of argument */
		z = (*p_arg)->size;
		argp -= z;

		/* Align if necessary */
		argp = (अक्षर *) FFI_ALIGN_DOWN(FFI_ALIGN_DOWN(argp, (*p_arg)->alignment), 4);

		जर (z < आकार(पूर्णांक)) {
			z = आकार(पूर्णांक);
			निवड ((*p_arg)->type)
			{
			स्थिती FFI_TYPE_SINT8:
				*(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
				थांबा;
			स्थिती FFI_TYPE_UINT8:
				*(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
				थांबा;
			स्थिती FFI_TYPE_SINT16:
				*(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
				थांबा;
			स्थिती FFI_TYPE_UINT16:
				*(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
			स्थिती FFI_TYPE_STRUCT:
				स_नकल(argp, *p_argv, (*p_arg)->size);
				थांबा;
			पूर्वनिर्धारित:
				FFI_ASSERT(0);
			}
		} अन्यथा जर ( z == आकार(पूर्णांक)) {
			*(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
		} अन्यथा {
			स_नकल(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	परत FFI_ALIGN(MIN(चळत - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	अचिन्हित i, bytes = 0;

	साठी (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		जर ((*ptr)->size == 0)
			परत FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		जर (((*ptr)->alignment - 1) & bytes)
			bytes = FFI_ALIGN(bytes, (*ptr)->alignment);

		bytes += FFI_ALIGN((*ptr)->size, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	bytes = FFI_ALIGN(bytes, 8);

	/* Make space for the return structure pointer */
	जर (cif->rtype->type == FFI_TYPE_STRUCT) {
		bytes += आकार(शून्य*);

		/* Ensure stack is aligned to an 8-byte boundary */
		bytes = FFI_ALIGN(bytes, 8);
	}

	cif->bytes = bytes;

	/* Set the return type flag */
	निवड (cif->rtype->type) {
	स्थिती FFI_TYPE_VOID:
	स्थिती FFI_TYPE_FLOAT:
	स्थिती FFI_TYPE_DOUBLE:
		cif->flags = (अचिन्हित) cif->rtype->type;
		थांबा;
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
		cif->flags = (अचिन्हित) FFI_TYPE_SINT64;
		थांबा;
	स्थिती FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		जर (cif->rtype->size <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (अचिन्हित)FFI_TYPE_INT;
		अन्यथा जर ((cif->rtype->size > 4) && (cif->rtype->size <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (अचिन्हित)FFI_TYPE_DOUBLE;
		अन्यथा
			/* value stored in memory */
			cif->flags = (अचिन्हित)FFI_TYPE_STRUCT;
		थांबा;
	पूर्वनिर्धारित:
		cif->flags = (अचिन्हित)FFI_TYPE_INT;
		थांबा;
	}
	परत FFI_OK;
}

बाह्य शून्य ffi_call_SYSV(शून्य (*fn)(शून्य), extended_cif *, अचिन्हित, अचिन्हित, द्विगुण *);

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

	पूर्णांक small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	द्विगुण temp;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	जर ((rvalue == रिक्त ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->size);
	अन्यथा जर (small_struct)
		ecif.rvalue = &temp;
	अन्यथा
		ecif.rvalue = rvalue;

	निवड (cif->abi) {
	स्थिती FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
		थांबा;
	पूर्वनिर्धारित:
		FFI_ASSERT(0);
		थांबा;
	}

	जर (small_struct)
		स_नकल (rvalue, &temp, cif->rtype->size);
}

/* private members */

स्थिर शून्य ffi_prep_incoming_args_SYSV (अक्षर *, शून्य **, शून्य **,
	ffi_cif*, दशांश *);

शून्य ffi_closure_SYSV (ffi_closure *);

/* Do NOT change that without changing the FFI_TRAMPOLINE_SIZE */
बाह्य अचिन्हित पूर्णांक ffi_metag_trampoline[10]; /* 10 instructions */

/* end of private members */

/*
 * __tramp: trampoline memory location
 * __fun: assembly routine
 * __ctx: memory location for wrapper
 *
 * At this point, tramp[0] == __ctx !
 */
शून्य ffi_init_trampoline(अचिन्हित अक्षर *__tramp, अचिन्हित पूर्णांक __fun, अचिन्हित पूर्णांक __ctx) {
	स_नकल (__tramp, ffi_metag_trampoline, आकार(ffi_metag_trampoline));
	*(अचिन्हित पूर्णांक*) &__tramp[40] = __ctx;
	*(अचिन्हित पूर्णांक*) &__tramp[44] = __fun;
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

	जर (cif->abi == FFI_SYSV)
		closure_func = &ffi_closure_SYSV;
	अन्यथा
		परत FFI_BAD_ABI;

	ffi_init_trampoline(
		(अचिन्हित अक्षर*)&closure->tramp[0],
		(अचिन्हित पूर्णांक)closure_func,
		(अचिन्हित पूर्णांक)codeloc);

	closure->cif = cif;
	closure->user_data = user_data;
	closure->fun = fun;

	परत FFI_OK;
}


/* This function is jumped to by the trampoline */
अचिन्हित पूर्णांक ffi_closure_SYSV_inner (closure, respp, args, vfp_args)
	ffi_closure *closure;
	शून्य **respp;
	शून्य *args;
	शून्य *vfp_args;
{
	ffi_cif *cif;
	शून्य **arg_area;

	cif = closure->cif;
	arg_area = (शून्य**) alloca (cif->nargs * आकार (शून्य*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif, vfp_args);

	(closure->fun) ( cif, *respp, arg_area, closure->user_data);

	परत cif->flags;
}

स्थिर शून्य ffi_prep_incoming_args_SYSV(अक्षर *चळत, शून्य **rvalue,
	शून्य **avalue, ffi_cif *cif,
	दशांश *vfp_चळत)
{
	नोंदणी अचिन्हित पूर्णांक i;
	नोंदणी शून्य **p_argv;
	नोंदणी अक्षर *argp;
	नोंदणी ffi_type **p_arg;

	/* stack points to original arguments */
	argp = चळत;

	/* Store return value */
	जर ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(शून्य **) argp;
	}

	p_argv = avalue;

	साठी (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		माप_प्रकार z;
		माप_प्रकार alignment;

		alignment = (*p_arg)->alignment;
		जर (alignment < 4)
			alignment = 4;
		जर ((alignment - 1) & (अचिन्हित)argp)
			argp = (अक्षर *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->size;
		*p_argv = (शून्य*) argp;
		p_argv++;
		argp -= z;
	}
	परत;
}
