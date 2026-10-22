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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>

#घोषणा MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
 * ffi_prep_args is called by the assembly routine once stack space has been
 * allocated for the function's arguments
 */

अचिन्हित पूर्णांक ffi_prep_args(अक्षर *ढेर, extended_cif *ecif)
{
	रेजिस्टर अचिन्हित पूर्णांक i;
	रेजिस्टर व्योम **p_argv;
	रेजिस्टर अक्षर *argp;
	रेजिस्टर ffi_type **p_arg;

	argp = ढेर;

	/* Store return value */
	अगर ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*(व्योम **) argp = ecif->rvalue;
	}

	p_argv = ecif->avalue;

	/* point to next location */
	क्रम (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; (i != 0); i--, p_arg++, p_argv++)
	{
		माप_प्रकार z;

		/* Move argp to address of argument */
		z = (*p_arg)->आकार;
		argp -= z;

		/* Align if necessary */
		argp = (अक्षर *) FFI_ALIGN_DOWN(FFI_ALIGN_DOWN(argp, (*p_arg)->alignment), 4);

		अगर (z < माप(पूर्णांक)) {
			z = माप(पूर्णांक);
			चयन ((*p_arg)->type)
			{
			हाल FFI_TYPE_SINT8:
				*(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
				अवरोध;
			हाल FFI_TYPE_UINT8:
				*(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
				अवरोध;
			हाल FFI_TYPE_SINT16:
				*(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
				अवरोध;
			हाल FFI_TYPE_UINT16:
				*(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
			हाल FFI_TYPE_STRUCT:
				स_नकल(argp, *p_argv, (*p_arg)->आकार);
				अवरोध;
			शेष:
				FFI_ASSERT(0);
			}
		} अन्यथा अगर ( z == माप(पूर्णांक)) {
			*(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
		} अन्यथा {
			स_नकल(argp, *p_argv, z);
		}
	}

	/* return the size of the arguments to be passed in registers,
	   padded to an 8 byte boundary to preserve stack alignment */
	वापस FFI_ALIGN(MIN(ढेर - argp, 6*4), 8);
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
	ffi_type **ptr;
	अचिन्हित i, bytes = 0;

	क्रम (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++) {
		अगर ((*ptr)->आकार == 0)
			वापस FFI_BAD_TYPEDEF;

		/* Perform a sanity check on the argument type, do this
		   check after the initialization.  */
		FFI_ASSERT_VALID_TYPE(*ptr);

		/* Add any padding if necessary */
		अगर (((*ptr)->alignment - 1) & bytes)
			bytes = FFI_ALIGN(bytes, (*ptr)->alignment);

		bytes += FFI_ALIGN((*ptr)->आकार, 4);
	}

	/* Ensure arg space is aligned to an 8-byte boundary */
	bytes = FFI_ALIGN(bytes, 8);

	/* Make space for the return structure pointer */
	अगर (cif->rtype->type == FFI_TYPE_STRUCT) {
		bytes += माप(व्योम*);

		/* Ensure stack is aligned to an 8-byte boundary */
		bytes = FFI_ALIGN(bytes, 8);
	}

	cif->bytes = bytes;

	/* Set the return type flag */
	चयन (cif->rtype->type) {
	हाल FFI_TYPE_VOID:
	हाल FFI_TYPE_FLOAT:
	हाल FFI_TYPE_DOUBLE:
		cif->flags = (अचिन्हित) cif->rtype->type;
		अवरोध;
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_UINT64:
		cif->flags = (अचिन्हित) FFI_TYPE_SINT64;
		अवरोध;
	हाल FFI_TYPE_STRUCT:
		/* Meta can store return values which are <= 64 bits */
		अगर (cif->rtype->आकार <= 4)
			/* Returned to D0Re0 as 32-bit value */
			cif->flags = (अचिन्हित)FFI_TYPE_INT;
		अन्यथा अगर ((cif->rtype->आकार > 4) && (cif->rtype->आकार <= 8))
			/* Returned valued is stored to D1Re0|R0Re0 */
			cif->flags = (अचिन्हित)FFI_TYPE_DOUBLE;
		अन्यथा
			/* value stored in memory */
			cif->flags = (अचिन्हित)FFI_TYPE_STRUCT;
		अवरोध;
	शेष:
		cif->flags = (अचिन्हित)FFI_TYPE_INT;
		अवरोध;
	}
	वापस FFI_OK;
}

बाह्य व्योम ffi_call_SYSV(व्योम (*fn)(व्योम), extended_cif *, अचिन्हित, अचिन्हित, द्विगुन *);

/*
 * Exported in API. Entry point
 * cif -> ffi_cif object
 * fn -> function pointer
 * rvalue -> pointer to return value
 * avalue -> vector of void * pointers pointing to memory locations holding the
 * arguments
 */
व्योम ffi_call(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
	extended_cif ecif;

	पूर्णांक small_struct = (((cif->flags == FFI_TYPE_INT) || (cif->flags == FFI_TYPE_DOUBLE)) && (cif->rtype->type == FFI_TYPE_STRUCT));
	ecif.cif = cif;
	ecif.avalue = avalue;

	द्विगुन अस्थायी;

	/*
	 * If the return value is a struct and we don't have a return value address
	 * then we need to make one
	 */

	अगर ((rvalue == शून्य ) && (cif->flags == FFI_TYPE_STRUCT))
		ecif.rvalue = alloca(cif->rtype->आकार);
	अन्यथा अगर (small_struct)
		ecif.rvalue = &अस्थायी;
	अन्यथा
		ecif.rvalue = rvalue;

	चयन (cif->abi) {
	हाल FFI_SYSV:
		ffi_call_SYSV(fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
		अवरोध;
	शेष:
		FFI_ASSERT(0);
		अवरोध;
	}

	अगर (small_struct)
		स_नकल (rvalue, &अस्थायी, cif->rtype->आकार);
}

/* private members */

अटल व्योम ffi_prep_incoming_args_SYSV (अक्षर *, व्योम **, व्योम **,
	ffi_cif*, भग्न *);

व्योम ffi_closure_SYSV (ffi_closure *);

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
व्योम ffi_init_trampoline(अचिन्हित अक्षर *__tramp, अचिन्हित पूर्णांक __fun, अचिन्हित पूर्णांक __ctx) {
	स_नकल (__tramp, ffi_metag_trampoline, माप(ffi_metag_trampoline));
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
	व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
	व्योम *user_तथ्य,
	व्योम *codeloc)
{
	व्योम (*closure_func)(ffi_closure*) = शून्य;

	अगर (cif->abi == FFI_SYSV)
		closure_func = &ffi_closure_SYSV;
	अन्यथा
		वापस FFI_BAD_ABI;

	ffi_init_trampoline(
		(अचिन्हित अक्षर*)&closure->tramp[0],
		(अचिन्हित पूर्णांक)closure_func,
		(अचिन्हित पूर्णांक)codeloc);

	closure->cif = cif;
	closure->user_तथ्य = user_तथ्य;
	closure->fun = fun;

	वापस FFI_OK;
}


/* This function is jumped to by the trampoline */
अचिन्हित पूर्णांक ffi_closure_SYSV_inner (closure, respp, args, vfp_args)
	ffi_closure *closure;
	व्योम **respp;
	व्योम *args;
	व्योम *vfp_args;
{
	ffi_cif *cif;
	व्योम **arg_area;

	cif = closure->cif;
	arg_area = (व्योम**) alloca (cif->nargs * माप (व्योम*));

	/*
	 * This call will initialize ARG_AREA, such that each
	 * element in that array points to the corresponding
	 * value on the stack; and if the function returns
	 * a structure, it will re-set RESP to point to the
	 * structure return address.
	 */
	ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif, vfp_args);

	(closure->fun) ( cif, *respp, arg_area, closure->user_तथ्य);

	वापस cif->flags;
}

अटल व्योम ffi_prep_incoming_args_SYSV(अक्षर *ढेर, व्योम **rvalue,
	व्योम **avalue, ffi_cif *cif,
	भग्न *vfp_ढेर)
{
	रेजिस्टर अचिन्हित पूर्णांक i;
	रेजिस्टर व्योम **p_argv;
	रेजिस्टर अक्षर *argp;
	रेजिस्टर ffi_type **p_arg;

	/* stack points to original arguments */
	argp = ढेर;

	/* Store return value */
	अगर ( cif->flags == FFI_TYPE_STRUCT ) {
		argp -= 4;
		*rvalue = *(व्योम **) argp;
	}

	p_argv = avalue;

	क्रम (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++) {
		माप_प्रकार z;
		माप_प्रकार alignment;

		alignment = (*p_arg)->alignment;
		अगर (alignment < 4)
			alignment = 4;
		अगर ((alignment - 1) & (अचिन्हित)argp)
			argp = (अक्षर *) FFI_ALIGN(argp, alignment);

		z = (*p_arg)->आकार;
		*p_argv = (व्योम*) argp;
		p_argv++;
		argp -= z;
	}
	वापस;
}
