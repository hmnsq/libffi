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

#شامل <ffi.h>
#شامل <ffi_common.h>

خارجی خالی ffi_call_SYSV(خالی (*)(خالی*, extended_cif*), extended_cif*,
		غیرعلامتی صحیح, غیرعلامتی صحیح, غیرعلامتی صحیح*, خالی (*fn)(خالی),
		غیرعلامتی صحیح, غیرعلامتی صحیح);

خارجی خالی ffi_احاطہ_SYSV(خالی);

#تعریف_کرو لفظ_سائز			حجم(غیرعلامتی صحیح)
#تعریف_کرو دلائل_رجسٹر_سائز	(لفظ_سائز * 6)
#تعریف_کرو لفظ_FFI_ALIGN(x)		FFI_ALIGN(x, لفظ_سائز)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
خالی ffi_prep_دلائل(خالی* ڈھیر, extended_cif* ecif)
{
	غیرعلامتی صحیح i;
	ffi_type** p_arg;
	خالی** p_argv;
	خالی* ڈھیر_دلائل_p = ڈھیر;

	اگر (ecif == تہی || ecif->cif == تہی) {
		واپس; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	اگر ((ecif->cif->rtype != تہی) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		حرف* addr = ڈھیر_دلائل_p;
		حافظہ_نقل(addr, &(ecif->rvalue), لفظ_سائز);
		ڈھیر_دلائل_p += لفظ_سائز;
	}

	اگر (ecif->avalue == تہی) {
		واپس; /* no arguments to prepare */
	}

	برائے (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t سائز = (*p_arg)->سائز;
		صحیح type = (*p_arg)->type;
		خالی* قدر = p_argv[i];
		حرف* addr = ڈھیر_دلائل_p;
		صحیح aligned_سائز = لفظ_FFI_ALIGN(سائز);

		/* force word alignment on the stack */
		ڈھیر_دلائل_p += aligned_سائز;
		
		انتخاب (type)
		{
			حالت FFI_TYPE_UINT8:
				*(غیرعلامتی صحیح *)addr = (غیرعلامتی صحیح)*(UINT8*)(قدر);
				توڑو;
			حالت FFI_TYPE_SINT8:
				*(علامتی صحیح *)addr = (علامتی صحیح)*(SINT8*)(قدر);
				توڑو;
			حالت FFI_TYPE_UINT16:
				*(غیرعلامتی صحیح *)addr = (غیرعلامتی صحیح)*(UINT16*)(قدر);
				توڑو;
			حالت FFI_TYPE_SINT16:
				*(علامتی صحیح *)addr = (علامتی صحیح)*(SINT16*)(قدر);
				توڑو;
			حالت FFI_TYPE_STRUCT:
#اگر __BIG_ENDIAN__
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
				اگر (سائز < لفظ_سائز)
				{
				  حافظہ_نقل (addr + (لفظ_سائز - سائز), قدر, سائز);
				  توڑو;
				}
#اگر_ختم
			حالت FFI_TYPE_SINT32:
			حالت FFI_TYPE_UINT32:
			حالت FFI_TYPE_FLOAT:
			حالت FFI_TYPE_SINT64:
			حالت FFI_TYPE_UINT64:
			حالت FFI_TYPE_DOUBLE:
			طےشدہ:
				حافظہ_نقل(addr, قدر, aligned_سائز);
		}
	}
}

ffi_حالت ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	انتخاب (cif->abi)
	{
		حالت FFI_SYSV:
			توڑو;
		طےشدہ:
			واپس FFI_BAD_ABI;
	}
	واپس FFI_OK;
}

خالی ffi_call(ffi_cif* cif, خالی (*fn)(خالی), خالی* rvalue, خالی** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	اگر ((rvalue == تہی) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->سائز);
	} ورنہ {
		ecif.rvalue = rvalue;
	}

	انتخاب (cif->abi)
	{
	حالت FFI_SYSV:
		ffi_call_SYSV(ffi_prep_دلائل, &ecif, cif->بائٹیں, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->سائز);
		توڑو;
	طےشدہ:
		FFI_ASSERT(0);
		توڑو;
	}
}

خالی ffi_احاطہ_call_SYSV(خالی* رجسٹر_دلائل, خالی* ڈھیر_دلائل,
			ffi_احاطہ* احاطہ, خالی* rvalue,
			غیرعلامتی صحیح* rtype, غیرعلامتی صحیح* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = احاطہ->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	حرف* stackclone = alloca(cif->بائٹیں);
	خالی** avalue = alloca(cif->nargs * حجم(خالی*));
	خالی* struct_rvalue = تہی;
	حرف* ptr = stackclone;
	صحیح i;

	/* copy registers into stack clone */
	صحیح registers_مستعمل = cif->بائٹیں;
	اگر (registers_مستعمل > دلائل_رجسٹر_سائز) {
		registers_مستعمل = دلائل_رجسٹر_سائز;
	}
	حافظہ_نقل(stackclone, رجسٹر_دلائل, registers_مستعمل);

	/* copy stack allocated args into stack clone */
	اگر (cif->بائٹیں > دلائل_رجسٹر_سائز) {
		صحیح ڈھیر_مستعمل = cif->بائٹیں - دلائل_رجسٹر_سائز;
		حافظہ_نقل(stackclone + دلائل_رجسٹر_سائز, ڈھیر_دلائل, ڈھیر_مستعمل);
	}

	/* preserve struct type return pointer passing */
	اگر ((cif->rtype != تہی) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((خالی**)ptr);
		ptr += لفظ_سائز;
	}

	/* populate arg pointer list */
	برائے (i = 0; i < cif->nargs; i++)
	{
		انتخاب (arg_types[i]->type)
		{
			حالت FFI_TYPE_SINT8:
			حالت FFI_TYPE_UINT8:
#اگر_تعریف_شدہ __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#ورنہ
				avalue[i] = ptr;
#اگر_ختم
				توڑو;
			حالت FFI_TYPE_SINT16:
			حالت FFI_TYPE_UINT16:
#اگر_تعریف_شدہ __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#ورنہ
				avalue[i] = ptr;
#اگر_ختم
				توڑو;
			حالت FFI_TYPE_STRUCT:
#اگر __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				اگر (arg_types[i]->سائز < لفظ_سائز)
				{
				  حافظہ_نقل (ptr, ptr + (لفظ_سائز - arg_types[i]->سائز), arg_types[i]->سائز);
				}
#اگر_ختم
				avalue[i] = (خالی*)ptr;
				توڑو;
			حالت FFI_TYPE_UINT64:
			حالت FFI_TYPE_SINT64:
			حالت FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				توڑو;
			حالت FFI_TYPE_SINT32:
			حالت FFI_TYPE_UINT32:
			حالت FFI_TYPE_FLOAT:
			طےشدہ:
				/* default 4-byte argument */
				avalue[i] = ptr;
				توڑو;
		}
		ptr += لفظ_FFI_ALIGN(arg_types[i]->سائز);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->سائز;
	*rtype = cif->rtype->type;
	اگر (struct_rvalue != تہی) {
		احاطہ->fun(cif, struct_rvalue, avalue, احاطہ->user_ڈیٹا);
		/* copy struct return pointer value into function return value */
		*((خالی**)rvalue) = struct_rvalue;
	} ورنہ {
		احاطہ->fun(cif, rvalue, avalue, احاطہ->user_ڈیٹا);
	}
}

ffi_حالت ffi_prep_احاطہ_loc(
		ffi_احاطہ* احاطہ, ffi_cif* cif,
		خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		خالی* user_ڈیٹا, خالی* codeloc)
{
	غیرعلامتی طویل* tramp = (غیرعلامتی طویل*)&(احاطہ->tramp[0]);
	غیرعلامتی طویل cls = (غیرعلامتی طویل)codeloc;
	غیرعلامتی طویل fn = 0;
	غیرعلامتی طویل fn_احاطہ_call_sysv = (غیرعلامتی طویل)ffi_احاطہ_call_SYSV;

	احاطہ->cif = cif;
	احاطہ->fun = fun;
	احاطہ->user_ڈیٹا = user_ڈیٹا;

	انتخاب (cif->abi)
	{
	حالت FFI_SYSV:
		fn = (غیرعلامتی طویل)ffi_احاطہ_SYSV;

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
		tramp[4] = 0xb0000000 | ((fn_احاطہ_call_sysv >> 16) & 0xffff);
		/* addik r3, r0, fn_closure_call_sysv(lower) */
		tramp[5] = 0x30600000 | (fn_احاطہ_call_sysv & 0xffff);
		/* branch/jump to address stored in r11 (fn) */
		tramp[6] = 0x98085800; /* bra r11 */

		توڑو;
	طےشدہ:
		واپس FFI_BAD_ABI;
	}
	واپس FFI_OK;
}
