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
		अचिह्नित पूर्णाङ्क, अचिह्नित पूर्णाङ्क, अचिह्नित पूर्णाङ्क*, शून्य (*fn)(शून्य),
		अचिह्नित पूर्णाङ्क, अचिह्नित पूर्णाङ्क);

बाह्य शून्य ffi_संवृतिः_SYSV(शून्य);

#परिभाषय WORD_आकार			परिमाण(अचिह्नित पूर्णाङ्क)
#परिभाषय तर्काः_पञ्जिकाकोशः_आकार	(WORD_आकार * 6)
#परिभाषय WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_आकार)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
शून्य ffi_prep_तर्काः(शून्य* राशिः, extended_cif* ecif)
{
	अचिह्नित पूर्णाङ्क i;
	ffi_type** p_arg;
	शून्य** p_argv;
	शून्य* राशिः_तर्काः_p = राशिः;

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
		वर्ण* addr = राशिः_तर्काः_p;
		स्मृतिं_प्रतिलिख(addr, &(ecif->rvalue), WORD_आकार);
		राशिः_तर्काः_p += WORD_आकार;
	}

	यदि (ecif->avalue == रिक्त) {
		प्रत्यागम; /* no arguments to prepare */
	}

	प्रति (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t आकार = (*p_arg)->आकार;
		पूर्णाङ्क type = (*p_arg)->type;
		शून्य* मानम् = p_argv[i];
		वर्ण* addr = राशिः_तर्काः_p;
		पूर्णाङ्क aligned_आकार = WORD_FFI_ALIGN(आकार);

		/* force word alignment on the stack */
		राशिः_तर्काः_p += aligned_आकार;
		
		विकल्प (type)
		{
			पक्ष FFI_TYPE_UINT8:
				*(अचिह्नित पूर्णाङ्क *)addr = (अचिह्नित पूर्णाङ्क)*(UINT8*)(मानम्);
				भङ्ग;
			पक्ष FFI_TYPE_SINT8:
				*(चिह्नित पूर्णाङ्क *)addr = (चिह्नित पूर्णाङ्क)*(SINT8*)(मानम्);
				भङ्ग;
			पक्ष FFI_TYPE_UINT16:
				*(अचिह्नित पूर्णाङ्क *)addr = (अचिह्नित पूर्णाङ्क)*(UINT16*)(मानम्);
				भङ्ग;
			पक्ष FFI_TYPE_SINT16:
				*(चिह्नित पूर्णाङ्क *)addr = (चिह्नित पूर्णाङ्क)*(SINT16*)(मानम्);
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
				यदि (आकार < WORD_आकार)
				{
				  स्मृतिं_प्रतिलिख (addr + (WORD_आकार - आकार), मानम्, आकार);
				  भङ्ग;
				}
#यदि_अन्तः
			पक्ष FFI_TYPE_SINT32:
			पक्ष FFI_TYPE_UINT32:
			पक्ष FFI_TYPE_FLOAT:
			पक्ष FFI_TYPE_SINT64:
			पक्ष FFI_TYPE_UINT64:
			पक्ष FFI_TYPE_DOUBLE:
			स्वभाव:
				स्मृतिं_प्रतिलिख(addr, मानम्, aligned_आकार);
		}
	}
}

ffi_स्थितिः ffi_prep_cif_machdep(ffi_cif* cif)
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
		ecif.rvalue = alloca(cif->rtype->आकार);
	} अन्यथा {
		ecif.rvalue = rvalue;
	}

	विकल्प (cif->abi)
	{
	पक्ष FFI_SYSV:
		ffi_call_SYSV(ffi_prep_तर्काः, &ecif, cif->अष्टकानि, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->आकार);
		भङ्ग;
	स्वभाव:
		FFI_ASSERT(0);
		भङ्ग;
	}
}

शून्य ffi_संवृतिः_call_SYSV(शून्य* पञ्जिकाकोशः_तर्काः, शून्य* राशिः_तर्काः,
			ffi_संवृतिः* संवृतिः, शून्य* rvalue,
			अचिह्नित पूर्णाङ्क* rtype, अचिह्नित पूर्णाङ्क* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = संवृतिः->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	वर्ण* stackclone = alloca(cif->अष्टकानि);
	शून्य** avalue = alloca(cif->nargs * परिमाण(शून्य*));
	शून्य* struct_rvalue = रिक्त;
	वर्ण* ptr = stackclone;
	पूर्णाङ्क i;

	/* copy registers into stack clone */
	पूर्णाङ्क registers_प्रयुक्तम् = cif->अष्टकानि;
	यदि (registers_प्रयुक्तम् > तर्काः_पञ्जिकाकोशः_आकार) {
		registers_प्रयुक्तम् = तर्काः_पञ्जिकाकोशः_आकार;
	}
	स्मृतिं_प्रतिलिख(stackclone, पञ्जिकाकोशः_तर्काः, registers_प्रयुक्तम्);

	/* copy stack allocated args into stack clone */
	यदि (cif->अष्टकानि > तर्काः_पञ्जिकाकोशः_आकार) {
		पूर्णाङ्क राशिः_प्रयुक्तम् = cif->अष्टकानि - तर्काः_पञ्जिकाकोशः_आकार;
		स्मृतिं_प्रतिलिख(stackclone + तर्काः_पञ्जिकाकोशः_आकार, राशिः_तर्काः, राशिः_प्रयुक्तम्);
	}

	/* preserve struct type return pointer passing */
	यदि ((cif->rtype != रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((शून्य**)ptr);
		ptr += WORD_आकार;
	}

	/* populate arg pointer list */
	प्रति (i = 0; i < cif->nargs; i++)
	{
		विकल्प (arg_types[i]->type)
		{
			पक्ष FFI_TYPE_SINT8:
			पक्ष FFI_TYPE_UINT8:
#यदि_परिभाषितम् __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#अन्यथा
				avalue[i] = ptr;
#यदि_अन्तः
				भङ्ग;
			पक्ष FFI_TYPE_SINT16:
			पक्ष FFI_TYPE_UINT16:
#यदि_परिभाषितम् __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#अन्यथा
				avalue[i] = ptr;
#यदि_अन्तः
				भङ्ग;
			पक्ष FFI_TYPE_STRUCT:
#यदि __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				यदि (arg_types[i]->आकार < WORD_आकार)
				{
				  स्मृतिं_प्रतिलिख (ptr, ptr + (WORD_आकार - arg_types[i]->आकार), arg_types[i]->आकार);
				}
#यदि_अन्तः
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
		ptr += WORD_FFI_ALIGN(arg_types[i]->आकार);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->आकार;
	*rtype = cif->rtype->type;
	यदि (struct_rvalue != रिक्त) {
		संवृतिः->fun(cif, struct_rvalue, avalue, संवृतिः->user_दत्त);
		/* copy struct return pointer value into function return value */
		*((शून्य**)rvalue) = struct_rvalue;
	} अन्यथा {
		संवृतिः->fun(cif, rvalue, avalue, संवृतिः->user_दत्त);
	}
}

ffi_स्थितिः ffi_prep_संवृतिः_loc(
		ffi_संवृतिः* संवृतिः, ffi_cif* cif,
		शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		शून्य* user_दत्त, शून्य* codeloc)
{
	अचिह्नित दीर्घ* tramp = (अचिह्नित दीर्घ*)&(संवृतिः->tramp[0]);
	अचिह्नित दीर्घ cls = (अचिह्नित दीर्घ)codeloc;
	अचिह्नित दीर्घ fn = 0;
	अचिह्नित दीर्घ fn_संवृतिः_call_sysv = (अचिह्नित दीर्घ)ffi_संवृतिः_call_SYSV;

	संवृतिः->cif = cif;
	संवृतिः->fun = fun;
	संवृतिः->user_दत्त = user_दत्त;

	विकल्प (cif->abi)
	{
	पक्ष FFI_SYSV:
		fn = (अचिह्नित दीर्घ)ffi_संवृतिः_SYSV;

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
		tramp[4] = 0xb0000000 | ((fn_संवृतिः_call_sysv >> 16) & 0xffff);
		/* addik r3, r0, fn_closure_call_sysv(lower) */
		tramp[5] = 0x30600000 | (fn_संवृतिः_call_sysv & 0xffff);
		/* branch/jump to address stored in r11 (fn) */
		tramp[6] = 0x98085800; /* bra r11 */

		भङ्ग;
	स्वभाव:
		प्रत्यागम FFI_BAD_ABI;
	}
	प्रत्यागम FFI_OK;
}
