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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

बाह्य व्योम ffi_call_SYSV(व्योम (*)(व्योम*, extended_cif*), extended_cif*,
		अचिन्हित पूर्णांक, अचिन्हित पूर्णांक, अचिन्हित पूर्णांक*, व्योम (*fn)(व्योम),
		अचिन्हित पूर्णांक, अचिन्हित पूर्णांक);

बाह्य व्योम ffi_closure_SYSV(व्योम);

#घोषणा WORD_आकार			माप(अचिन्हित पूर्णांक)
#घोषणा ARGS_REGISTER_आकार	(WORD_आकार * 6)
#घोषणा WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_आकार)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
व्योम ffi_prep_args(व्योम* ढेर, extended_cif* ecif)
{
	अचिन्हित पूर्णांक i;
	ffi_type** p_arg;
	व्योम** p_argv;
	व्योम* ढेर_args_p = ढेर;

	अगर (ecif == शून्य || ecif->cif == शून्य) {
		वापस; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	अगर ((ecif->cif->rtype != शून्य) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		अक्षर* addr = ढेर_args_p;
		स_नकल(addr, &(ecif->rvalue), WORD_आकार);
		ढेर_args_p += WORD_आकार;
	}

	अगर (ecif->avalue == शून्य) {
		वापस; /* no arguments to prepare */
	}

	क्रम (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		माप_प्रकार आकार = (*p_arg)->आकार;
		पूर्णांक type = (*p_arg)->type;
		व्योम* मूल्य = p_argv[i];
		अक्षर* addr = ढेर_args_p;
		पूर्णांक aligned_आकार = WORD_FFI_ALIGN(आकार);

		/* force word alignment on the stack */
		ढेर_args_p += aligned_आकार;
		
		चयन (type)
		{
			हाल FFI_TYPE_UINT8:
				*(अचिन्हित पूर्णांक *)addr = (अचिन्हित पूर्णांक)*(UINT8*)(मूल्य);
				अवरोध;
			हाल FFI_TYPE_SINT8:
				*(चिन्हित पूर्णांक *)addr = (चिन्हित पूर्णांक)*(SINT8*)(मूल्य);
				अवरोध;
			हाल FFI_TYPE_UINT16:
				*(अचिन्हित पूर्णांक *)addr = (अचिन्हित पूर्णांक)*(UINT16*)(मूल्य);
				अवरोध;
			हाल FFI_TYPE_SINT16:
				*(चिन्हित पूर्णांक *)addr = (चिन्हित पूर्णांक)*(SINT16*)(मूल्य);
				अवरोध;
			हाल FFI_TYPE_STRUCT:
#अगर __BIG_ENDIAN__
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
				अगर (आकार < WORD_आकार)
				{
				  स_नकल (addr + (WORD_आकार - आकार), मूल्य, आकार);
				  अवरोध;
				}
#पूर्ण_अगर
			हाल FFI_TYPE_SINT32:
			हाल FFI_TYPE_UINT32:
			हाल FFI_TYPE_FLOAT:
			हाल FFI_TYPE_SINT64:
			हाल FFI_TYPE_UINT64:
			हाल FFI_TYPE_DOUBLE:
			शेष:
				स_नकल(addr, मूल्य, aligned_आकार);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	चयन (cif->abi)
	{
		हाल FFI_SYSV:
			अवरोध;
		शेष:
			वापस FFI_BAD_ABI;
	}
	वापस FFI_OK;
}

व्योम ffi_call(ffi_cif* cif, व्योम (*fn)(व्योम), व्योम* rvalue, व्योम** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	अगर ((rvalue == शून्य) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->आकार);
	} अन्यथा {
		ecif.rvalue = rvalue;
	}

	चयन (cif->abi)
	{
	हाल FFI_SYSV:
		ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->आकार);
		अवरोध;
	शेष:
		FFI_ASSERT(0);
		अवरोध;
	}
}

व्योम ffi_closure_call_SYSV(व्योम* register_args, व्योम* ढेर_args,
			ffi_closure* closure, व्योम* rvalue,
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
	व्योम** avalue = alloca(cif->nargs * माप(व्योम*));
	व्योम* struct_rvalue = शून्य;
	अक्षर* ptr = stackclone;
	पूर्णांक i;

	/* copy registers into stack clone */
	पूर्णांक registers_used = cif->bytes;
	अगर (registers_used > ARGS_REGISTER_आकार) {
		registers_used = ARGS_REGISTER_आकार;
	}
	स_नकल(stackclone, register_args, registers_used);

	/* copy stack allocated args into stack clone */
	अगर (cif->bytes > ARGS_REGISTER_आकार) {
		पूर्णांक ढेर_used = cif->bytes - ARGS_REGISTER_आकार;
		स_नकल(stackclone + ARGS_REGISTER_आकार, ढेर_args, ढेर_used);
	}

	/* preserve struct type return pointer passing */
	अगर ((cif->rtype != शून्य) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((व्योम**)ptr);
		ptr += WORD_आकार;
	}

	/* populate arg pointer list */
	क्रम (i = 0; i < cif->nargs; i++)
	{
		चयन (arg_types[i]->type)
		{
			हाल FFI_TYPE_SINT8:
			हाल FFI_TYPE_UINT8:
#अगर_घोषित __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#अन्यथा
				avalue[i] = ptr;
#पूर्ण_अगर
				अवरोध;
			हाल FFI_TYPE_SINT16:
			हाल FFI_TYPE_UINT16:
#अगर_घोषित __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#अन्यथा
				avalue[i] = ptr;
#पूर्ण_अगर
				अवरोध;
			हाल FFI_TYPE_STRUCT:
#अगर __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				अगर (arg_types[i]->आकार < WORD_आकार)
				{
				  स_नकल (ptr, ptr + (WORD_आकार - arg_types[i]->आकार), arg_types[i]->आकार);
				}
#पूर्ण_अगर
				avalue[i] = (व्योम*)ptr;
				अवरोध;
			हाल FFI_TYPE_UINT64:
			हाल FFI_TYPE_SINT64:
			हाल FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				अवरोध;
			हाल FFI_TYPE_SINT32:
			हाल FFI_TYPE_UINT32:
			हाल FFI_TYPE_FLOAT:
			शेष:
				/* default 4-byte argument */
				avalue[i] = ptr;
				अवरोध;
		}
		ptr += WORD_FFI_ALIGN(arg_types[i]->आकार);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->आकार;
	*rtype = cif->rtype->type;
	अगर (struct_rvalue != शून्य) {
		closure->fun(cif, struct_rvalue, avalue, closure->user_तथ्य);
		/* copy struct return pointer value into function return value */
		*((व्योम**)rvalue) = struct_rvalue;
	} अन्यथा {
		closure->fun(cif, rvalue, avalue, closure->user_तथ्य);
	}
}

ffi_status ffi_prep_closure_loc(
		ffi_closure* closure, ffi_cif* cif,
		व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		व्योम* user_तथ्य, व्योम* codeloc)
{
	अचिन्हित दीर्घ* tramp = (अचिन्हित दीर्घ*)&(closure->tramp[0]);
	अचिन्हित दीर्घ cls = (अचिन्हित दीर्घ)codeloc;
	अचिन्हित दीर्घ fn = 0;
	अचिन्हित दीर्घ fn_closure_call_sysv = (अचिन्हित दीर्घ)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_तथ्य = user_तथ्य;

	चयन (cif->abi)
	{
	हाल FFI_SYSV:
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

		अवरोध;
	शेष:
		वापस FFI_BAD_ABI;
	}
	वापस FFI_OK;
}
