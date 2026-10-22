/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2012, 2013 Xilinx, Inc

   MicroBlaze Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

बाह्य शून्य ffi_call_SYSV(शून्य (*)(शून्य*, extended_cif*), extended_cif*,
		अचिन्हित पूर्णांक, अचिन्हित पूर्णांक, अचिन्हित पूर्णांक*, शून्य (*fn)(शून्य),
		अचिन्हित पूर्णांक, अचिन्हित पूर्णांक);

बाह्य शून्य ffi_closure_SYSV(शून्य);

#define WORD_SIZE			आकार(अचिन्हित पूर्णांक)
#define ARGS_REGISTER_SIZE	(WORD_SIZE * 6)
#define WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_SIZE)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
शून्य ffi_prep_args(शून्य* चळत, extended_cif* ecif)
{
	अचिन्हित पूर्णांक i;
	ffi_type** p_arg;
	शून्य** p_argv;
	शून्य* चळत_args_p = चळत;

	जर (ecif == रिक्त || ecif->cif == रिक्त) {
		परत; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	जर ((ecif->cif->rtype != रिक्त) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		अक्षर* addr = चळत_args_p;
		स_नकल(addr, &(ecif->rvalue), WORD_SIZE);
		चळत_args_p += WORD_SIZE;
	}

	जर (ecif->avalue == रिक्त) {
		परत; /* no arguments to prepare */
	}

	साठी (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		माप_प्रकार size = (*p_arg)->size;
		पूर्णांक type = (*p_arg)->type;
		शून्य* मूल्य = p_argv[i];
		अक्षर* addr = चळत_args_p;
		पूर्णांक aligned_size = WORD_FFI_ALIGN(size);

		/* force word alignment on the stack */
		चळत_args_p += aligned_size;
		
		निवड (type)
		{
			स्थिती FFI_TYPE_UINT8:
				*(अचिन्हित पूर्णांक *)addr = (अचिन्हित पूर्णांक)*(UINT8*)(मूल्य);
				थांबा;
			स्थिती FFI_TYPE_SINT8:
				*(चिन्हित पूर्णांक *)addr = (चिन्हित पूर्णांक)*(SINT8*)(मूल्य);
				थांबा;
			स्थिती FFI_TYPE_UINT16:
				*(अचिन्हित पूर्णांक *)addr = (अचिन्हित पूर्णांक)*(UINT16*)(मूल्य);
				थांबा;
			स्थिती FFI_TYPE_SINT16:
				*(चिन्हित पूर्णांक *)addr = (चिन्हित पूर्णांक)*(SINT16*)(मूल्य);
				थांबा;
			स्थिती FFI_TYPE_STRUCT:
#जर __BIG_ENDIAN__
				/*
				 * MicroBlaze toolchain appears to emit:
				 * bsrli r5, r5, 8 (caller)
				 * ...
				 * <branch to callee>
				 * ...
				 * bslli r5, r5, 8 (callee)
				 * 
				 * For structs like "struct a { uint8_t a[3]; };", when passed
				 * by value.
				 *
				 * Structs like "struct b { uint16_t a; };" are also expected
				 * to be packed strangely in registers.
				 *
				 * This appears to be because the microblaze toolchain expects
				 * "struct b == uint16_t", which is only any issue for big
				 * endian.
				 *
				 * The following is a work around for big-endian only, for the
				 * above mentioned case, it will re-align the contents of a
				 * <= 3-byte struct value.
				 */
				जर (size < WORD_SIZE)
				{
				  स_नकल (addr + (WORD_SIZE - size), मूल्य, size);
				  थांबा;
				}
#endif
			स्थिती FFI_TYPE_SINT32:
			स्थिती FFI_TYPE_UINT32:
			स्थिती FFI_TYPE_FLOAT:
			स्थिती FFI_TYPE_SINT64:
			स्थिती FFI_TYPE_UINT64:
			स्थिती FFI_TYPE_DOUBLE:
			पूर्वनिर्धारित:
				स_नकल(addr, मूल्य, aligned_size);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	निवड (cif->abi)
	{
		स्थिती FFI_SYSV:
			थांबा;
		पूर्वनिर्धारित:
			परत FFI_BAD_ABI;
	}
	परत FFI_OK;
}

शून्य ffi_call(ffi_cif* cif, शून्य (*fn)(शून्य), शून्य* rvalue, शून्य** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	जर ((rvalue == रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->size);
	} अन्यथा {
		ecif.rvalue = rvalue;
	}

	निवड (cif->abi)
	{
	स्थिती FFI_SYSV:
		ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->size);
		थांबा;
	पूर्वनिर्धारित:
		FFI_ASSERT(0);
		थांबा;
	}
}

शून्य ffi_closure_call_SYSV(शून्य* register_args, शून्य* चळत_args,
			ffi_closure* closure, शून्य* rvalue,
			अचिन्हित पूर्णांक* rtype, अचिन्हित पूर्णांक* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = closure->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	अक्षर* stackclone = alloca(cif->bytes);
	शून्य** avalue = alloca(cif->nargs * आकार(शून्य*));
	शून्य* struct_rvalue = रिक्त;
	अक्षर* ptr = stackclone;
	पूर्णांक i;

	/* copy registers into stack clone */
	पूर्णांक registers_used = cif->bytes;
	जर (registers_used > ARGS_REGISTER_SIZE) {
		registers_used = ARGS_REGISTER_SIZE;
	}
	स_नकल(stackclone, register_args, registers_used);

	/* copy stack allocated args into stack clone */
	जर (cif->bytes > ARGS_REGISTER_SIZE) {
		पूर्णांक चळत_used = cif->bytes - ARGS_REGISTER_SIZE;
		स_नकल(stackclone + ARGS_REGISTER_SIZE, चळत_args, चळत_used);
	}

	/* preserve struct type return pointer passing */
	जर ((cif->rtype != रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((शून्य**)ptr);
		ptr += WORD_SIZE;
	}

	/* populate arg pointer list */
	साठी (i = 0; i < cif->nargs; i++)
	{
		निवड (arg_types[i]->type)
		{
			स्थिती FFI_TYPE_SINT8:
			स्थिती FFI_TYPE_UINT8:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#अन्यथा
				avalue[i] = ptr;
#endif
				थांबा;
			स्थिती FFI_TYPE_SINT16:
			स्थिती FFI_TYPE_UINT16:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#अन्यथा
				avalue[i] = ptr;
#endif
				थांबा;
			स्थिती FFI_TYPE_STRUCT:
#जर __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				जर (arg_types[i]->size < WORD_SIZE)
				{
				  स_नकल (ptr, ptr + (WORD_SIZE - arg_types[i]->size), arg_types[i]->size);
				}
#endif
				avalue[i] = (शून्य*)ptr;
				थांबा;
			स्थिती FFI_TYPE_UINT64:
			स्थिती FFI_TYPE_SINT64:
			स्थिती FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				थांबा;
			स्थिती FFI_TYPE_SINT32:
			स्थिती FFI_TYPE_UINT32:
			स्थिती FFI_TYPE_FLOAT:
			पूर्वनिर्धारित:
				/* default 4-byte argument */
				avalue[i] = ptr;
				थांबा;
		}
		ptr += WORD_FFI_ALIGN(arg_types[i]->size);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->size;
	*rtype = cif->rtype->type;
	जर (struct_rvalue != रिक्त) {
		closure->fun(cif, struct_rvalue, avalue, closure->user_data);
		/* copy struct return pointer value into function return value */
		*((शून्य**)rvalue) = struct_rvalue;
	} अन्यथा {
		closure->fun(cif, rvalue, avalue, closure->user_data);
	}
}

ffi_status ffi_prep_closure_loc(
		ffi_closure* closure, ffi_cif* cif,
		शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		शून्य* user_data, शून्य* codeloc)
{
	अचिन्हित दीर्घ* tramp = (अचिन्हित दीर्घ*)&(closure->tramp[0]);
	अचिन्हित दीर्घ cls = (अचिन्हित दीर्घ)codeloc;
	अचिन्हित दीर्घ fn = 0;
	अचिन्हित दीर्घ fn_closure_call_sysv = (अचिन्हित दीर्घ)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	निवड (cif->abi)
	{
	स्थिती FFI_SYSV:
		fn = (अचिन्हित दीर्घ)ffi_closure_SYSV;

		/* load r11 (temp) with fn */
		/* imm fn(upper) */
		tramp[0] = 0xb0000000 | ((fn >> 16) & 0xffff);
		/* addik r11, r0, fn(lower) */
		tramp[1] = 0x31600000 | (fn & 0xffff);

		/* load r12 (temp) with cls */
		/* imm cls(upper) */
		tramp[2] = 0xb0000000 | ((cls >> 16) & 0xffff);
		/* addik r12, r0, cls(lower) */
		tramp[3] = 0x31800000 | (cls & 0xffff);

		/* load r3 (temp) with ffi_closure_call_SYSV */
		/* imm fn_closure_call_sysv(upper) */
		tramp[4] = 0xb0000000 | ((fn_closure_call_sysv >> 16) & 0xffff);
		/* addik r3, r0, fn_closure_call_sysv(lower) */
		tramp[5] = 0x30600000 | (fn_closure_call_sysv & 0xffff);
		/* branch/jump to address stored in r11 (fn) */
		tramp[6] = 0x98085800; /* bra r11 */

		थांबा;
	पूर्वनिर्धारित:
		परत FFI_BAD_ABI;
	}
	परत FFI_OK;
}
