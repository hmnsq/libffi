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
		unsigned पूर्णाङ्क, unsigned पूर्णाङ्क, unsigned पूर्णाङ्क*, शून्य (*fn)(शून्य),
		unsigned पूर्णाङ्क, unsigned पूर्णाङ्क);

बाह्य शून्य ffi_closure_SYSV(शून्य);

#define WORD_SIZE			परिमाण(unsigned पूर्णाङ्क)
#define ARGS_REGISTER_SIZE	(WORD_SIZE * 6)
#define WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_SIZE)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
शून्य ffi_prep_args(शून्य* ढेर, extended_cif* ecif)
{
	unsigned पूर्णाङ्क i;
	ffi_type** p_arg;
	शून्य** p_argv;
	शून्य* ढेर_args_p = ढेर;

	यदि (ecif == रिक्त || ecif->cif == रिक्त) {
		प्रत्यागम; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	यदि ((ecif->cif->rtype != रिक्त) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		वर्ण* addr = ढेर_args_p;
		memcpy(addr, &(ecif->rvalue), WORD_SIZE);
		ढेर_args_p += WORD_SIZE;
	}

	यदि (ecif->avalue == रिक्त) {
		प्रत्यागम; /* no arguments to prepare */
	}

	प्रति (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t size = (*p_arg)->size;
		पूर्णाङ्क type = (*p_arg)->type;
		शून्य* मूल्य = p_argv[i];
		वर्ण* addr = ढेर_args_p;
		पूर्णाङ्क aligned_size = WORD_FFI_ALIGN(size);

		/* force word alignment on the stack */
		ढेर_args_p += aligned_size;
		
		विकल्प (type)
		{
			पक्ष FFI_TYPE_UINT8:
				*(unsigned पूर्णाङ्क *)addr = (unsigned पूर्णाङ्क)*(UINT8*)(मूल्य);
				भङ्ग;
			पक्ष FFI_TYPE_SINT8:
				*(signed पूर्णाङ्क *)addr = (signed पूर्णाङ्क)*(SINT8*)(मूल्य);
				भङ्ग;
			पक्ष FFI_TYPE_UINT16:
				*(unsigned पूर्णाङ्क *)addr = (unsigned पूर्णाङ्क)*(UINT16*)(मूल्य);
				भङ्ग;
			पक्ष FFI_TYPE_SINT16:
				*(signed पूर्णाङ्क *)addr = (signed पूर्णाङ्क)*(SINT16*)(मूल्य);
				भङ्ग;
			पक्ष FFI_TYPE_STRUCT:
#यदि __BIG_ENDIAN__
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
				यदि (size < WORD_SIZE)
				{
				  memcpy (addr + (WORD_SIZE - size), मूल्य, size);
				  भङ्ग;
				}
#endif
			पक्ष FFI_TYPE_SINT32:
			पक्ष FFI_TYPE_UINT32:
			पक्ष FFI_TYPE_FLOAT:
			पक्ष FFI_TYPE_SINT64:
			पक्ष FFI_TYPE_UINT64:
			पक्ष FFI_TYPE_DOUBLE:
			स्वभाव:
				memcpy(addr, मूल्य, aligned_size);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	विकल्प (cif->abi)
	{
		पक्ष FFI_SYSV:
			भङ्ग;
		स्वभाव:
			प्रत्यागम FFI_BAD_ABI;
	}
	प्रत्यागम FFI_OK;
}

शून्य ffi_call(ffi_cif* cif, शून्य (*fn)(शून्य), शून्य* rvalue, शून्य** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	यदि ((rvalue == रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->size);
	} अन्यथा {
		ecif.rvalue = rvalue;
	}

	विकल्प (cif->abi)
	{
	पक्ष FFI_SYSV:
		ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->size);
		भङ्ग;
	स्वभाव:
		FFI_ASSERT(0);
		भङ्ग;
	}
}

शून्य ffi_closure_call_SYSV(शून्य* register_args, शून्य* ढेर_args,
			ffi_closure* closure, शून्य* rvalue,
			unsigned पूर्णाङ्क* rtype, unsigned पूर्णाङ्क* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = closure->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	वर्ण* stackclone = alloca(cif->bytes);
	शून्य** avalue = alloca(cif->nargs * परिमाण(शून्य*));
	शून्य* struct_rvalue = रिक्त;
	वर्ण* ptr = stackclone;
	पूर्णाङ्क i;

	/* copy registers into stack clone */
	पूर्णाङ्क registers_used = cif->bytes;
	यदि (registers_used > ARGS_REGISTER_SIZE) {
		registers_used = ARGS_REGISTER_SIZE;
	}
	memcpy(stackclone, register_args, registers_used);

	/* copy stack allocated args into stack clone */
	यदि (cif->bytes > ARGS_REGISTER_SIZE) {
		पूर्णाङ्क ढेर_used = cif->bytes - ARGS_REGISTER_SIZE;
		memcpy(stackclone + ARGS_REGISTER_SIZE, ढेर_args, ढेर_used);
	}

	/* preserve struct type return pointer passing */
	यदि ((cif->rtype != रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((शून्य**)ptr);
		ptr += WORD_SIZE;
	}

	/* populate arg pointer list */
	प्रति (i = 0; i < cif->nargs; i++)
	{
		विकल्प (arg_types[i]->type)
		{
			पक्ष FFI_TYPE_SINT8:
			पक्ष FFI_TYPE_UINT8:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#अन्यथा
				avalue[i] = ptr;
#endif
				भङ्ग;
			पक्ष FFI_TYPE_SINT16:
			पक्ष FFI_TYPE_UINT16:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#अन्यथा
				avalue[i] = ptr;
#endif
				भङ्ग;
			पक्ष FFI_TYPE_STRUCT:
#यदि __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				यदि (arg_types[i]->size < WORD_SIZE)
				{
				  memcpy (ptr, ptr + (WORD_SIZE - arg_types[i]->size), arg_types[i]->size);
				}
#endif
				avalue[i] = (शून्य*)ptr;
				भङ्ग;
			पक्ष FFI_TYPE_UINT64:
			पक्ष FFI_TYPE_SINT64:
			पक्ष FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				भङ्ग;
			पक्ष FFI_TYPE_SINT32:
			पक्ष FFI_TYPE_UINT32:
			पक्ष FFI_TYPE_FLOAT:
			स्वभाव:
				/* default 4-byte argument */
				avalue[i] = ptr;
				भङ्ग;
		}
		ptr += WORD_FFI_ALIGN(arg_types[i]->size);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->size;
	*rtype = cif->rtype->type;
	यदि (struct_rvalue != रिक्त) {
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
	unsigned long* tramp = (unsigned long*)&(closure->tramp[0]);
	unsigned long cls = (unsigned long)codeloc;
	unsigned long fn = 0;
	unsigned long fn_closure_call_sysv = (unsigned long)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	विकल्प (cif->abi)
	{
	पक्ष FFI_SYSV:
		fn = (unsigned long)ffi_closure_SYSV;

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

		भङ्ग;
	स्वभाव:
		प्रत्यागम FFI_BAD_ABI;
	}
	प्रत्यागम FFI_OK;
}
