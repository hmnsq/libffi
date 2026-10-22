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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV(ਖਾਲੀ (*)(ਖਾਲੀ*, extended_cif*), extended_cif*,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*, ਖਾਲੀ (*fn)(ਖਾਲੀ),
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_SYSV(ਖਾਲੀ);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸ਼ਬਦ_ਮਾਪ			ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਦਲੀਲਾਂ_ਰਜਿਸਟਰ_ਮਾਪ	(ਸ਼ਬਦ_ਮਾਪ * 6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸ਼ਬਦ_FFI_ALIGN(x)		FFI_ALIGN(x, ਸ਼ਬਦ_ਮਾਪ)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
ਖਾਲੀ ffi_prep_ਦਲੀਲਾਂ(ਖਾਲੀ* ਢੇਰ, extended_cif* ecif)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ffi_type** p_arg;
	ਖਾਲੀ** p_argv;
	ਖਾਲੀ* ਢੇਰ_ਦਲੀਲਾਂ_p = ਢੇਰ;

	ਜੇ (ecif == ਨਿਰਾ || ecif->cif == ਨਿਰਾ) {
		ਵਾਪਸ; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	ਜੇ ((ecif->cif->rtype != ਨਿਰਾ) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		ਅੱਖਰ* addr = ਢੇਰ_ਦਲੀਲਾਂ_p;
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(addr, &(ecif->rvalue), ਸ਼ਬਦ_ਮਾਪ);
		ਢੇਰ_ਦਲੀਲਾਂ_p += ਸ਼ਬਦ_ਮਾਪ;
	}

	ਜੇ (ecif->avalue == ਨਿਰਾ) {
		ਵਾਪਸ; /* no arguments to prepare */
	}

	ਲਈ (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t ਮਾਪ = (*p_arg)->ਮਾਪ;
		ਪੂਰਨ type = (*p_arg)->type;
		ਖਾਲੀ* ਮੁੱਲ = p_argv[i];
		ਅੱਖਰ* addr = ਢੇਰ_ਦਲੀਲਾਂ_p;
		ਪੂਰਨ aligned_ਮਾਪ = ਸ਼ਬਦ_FFI_ALIGN(ਮਾਪ);

		/* force word alignment on the stack */
		ਢੇਰ_ਦਲੀਲਾਂ_p += aligned_ਮਾਪ;
		
		ਚੋਣ (type)
		{
			ਹਾਲਤ FFI_TYPE_UINT8:
				*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)addr = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT8*)(ਮੁੱਲ);
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_SINT8:
				*(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *)addr = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT8*)(ਮੁੱਲ);
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_UINT16:
				*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)addr = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT16*)(ਮੁੱਲ);
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_SINT16:
				*(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *)addr = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT16*)(ਮੁੱਲ);
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_STRUCT:
#ਜੇ __BIG_ENDIAN__
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
				ਜੇ (ਮਾਪ < ਸ਼ਬਦ_ਮਾਪ)
				{
				  ਸਮ੍ਰਿਤੀ_ਨਕਲ (addr + (ਸ਼ਬਦ_ਮਾਪ - ਮਾਪ), ਮੁੱਲ, ਮਾਪ);
				  ਰੋਕੋ;
				}
#ਜੇ_ਅੰਤ
			ਹਾਲਤ FFI_TYPE_SINT32:
			ਹਾਲਤ FFI_TYPE_UINT32:
			ਹਾਲਤ FFI_TYPE_FLOAT:
			ਹਾਲਤ FFI_TYPE_SINT64:
			ਹਾਲਤ FFI_TYPE_UINT64:
			ਹਾਲਤ FFI_TYPE_DOUBLE:
			ਮੂਲ:
				ਸਮ੍ਰਿਤੀ_ਨਕਲ(addr, ਮੁੱਲ, aligned_ਮਾਪ);
		}
	}
}

ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	ਚੋਣ (cif->abi)
	{
		ਹਾਲਤ FFI_SYSV:
			ਰੋਕੋ;
		ਮੂਲ:
			ਵਾਪਸ FFI_BAD_ABI;
	}
	ਵਾਪਸ FFI_OK;
}

ਖਾਲੀ ffi_call(ffi_cif* cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ* rvalue, ਖਾਲੀ** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	ਜੇ ((rvalue == ਨਿਰਾ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->ਮਾਪ);
	} ਨਹੀਂਤਾਂ {
		ecif.rvalue = rvalue;
	}

	ਚੋਣ (cif->abi)
	{
	ਹਾਲਤ FFI_SYSV:
		ffi_call_SYSV(ffi_prep_ਦਲੀਲਾਂ, &ecif, cif->ਬਾਈਟਾਂ, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->ਮਾਪ);
		ਰੋਕੋ;
	ਮੂਲ:
		FFI_ASSERT(0);
		ਰੋਕੋ;
	}
}

ਖਾਲੀ ffi_ਘੇਰਾ_call_SYSV(ਖਾਲੀ* ਰਜਿਸਟਰ_ਦਲੀਲਾਂ, ਖਾਲੀ* ਢੇਰ_ਦਲੀਲਾਂ,
			ffi_ਘੇਰਾ* ਘੇਰਾ, ਖਾਲੀ* rvalue,
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ* rtype, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = ਘੇਰਾ->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	ਅੱਖਰ* stackclone = alloca(cif->ਬਾਈਟਾਂ);
	ਖਾਲੀ** avalue = alloca(cif->nargs * ਆਕਾਰ(ਖਾਲੀ*));
	ਖਾਲੀ* struct_rvalue = ਨਿਰਾ;
	ਅੱਖਰ* ptr = stackclone;
	ਪੂਰਨ i;

	/* copy registers into stack clone */
	ਪੂਰਨ registers_ਵਰਤਿਆ = cif->ਬਾਈਟਾਂ;
	ਜੇ (registers_ਵਰਤਿਆ > ਦਲੀਲਾਂ_ਰਜਿਸਟਰ_ਮਾਪ) {
		registers_ਵਰਤਿਆ = ਦਲੀਲਾਂ_ਰਜਿਸਟਰ_ਮਾਪ;
	}
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(stackclone, ਰਜਿਸਟਰ_ਦਲੀਲਾਂ, registers_ਵਰਤਿਆ);

	/* copy stack allocated args into stack clone */
	ਜੇ (cif->ਬਾਈਟਾਂ > ਦਲੀਲਾਂ_ਰਜਿਸਟਰ_ਮਾਪ) {
		ਪੂਰਨ ਢੇਰ_ਵਰਤਿਆ = cif->ਬਾਈਟਾਂ - ਦਲੀਲਾਂ_ਰਜਿਸਟਰ_ਮਾਪ;
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(stackclone + ਦਲੀਲਾਂ_ਰਜਿਸਟਰ_ਮਾਪ, ਢੇਰ_ਦਲੀਲਾਂ, ਢੇਰ_ਵਰਤਿਆ);
	}

	/* preserve struct type return pointer passing */
	ਜੇ ((cif->rtype != ਨਿਰਾ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((ਖਾਲੀ**)ptr);
		ptr += ਸ਼ਬਦ_ਮਾਪ;
	}

	/* populate arg pointer list */
	ਲਈ (i = 0; i < cif->nargs; i++)
	{
		ਚੋਣ (arg_types[i]->type)
		{
			ਹਾਲਤ FFI_TYPE_SINT8:
			ਹਾਲਤ FFI_TYPE_UINT8:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#ਨਹੀਂਤਾਂ
				avalue[i] = ptr;
#ਜੇ_ਅੰਤ
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_SINT16:
			ਹਾਲਤ FFI_TYPE_UINT16:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#ਨਹੀਂਤਾਂ
				avalue[i] = ptr;
#ਜੇ_ਅੰਤ
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_STRUCT:
#ਜੇ __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				ਜੇ (arg_types[i]->ਮਾਪ < ਸ਼ਬਦ_ਮਾਪ)
				{
				  ਸਮ੍ਰਿਤੀ_ਨਕਲ (ptr, ptr + (ਸ਼ਬਦ_ਮਾਪ - arg_types[i]->ਮਾਪ), arg_types[i]->ਮਾਪ);
				}
#ਜੇ_ਅੰਤ
				avalue[i] = (ਖਾਲੀ*)ptr;
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_UINT64:
			ਹਾਲਤ FFI_TYPE_SINT64:
			ਹਾਲਤ FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				ਰੋਕੋ;
			ਹਾਲਤ FFI_TYPE_SINT32:
			ਹਾਲਤ FFI_TYPE_UINT32:
			ਹਾਲਤ FFI_TYPE_FLOAT:
			ਮੂਲ:
				/* default 4-byte argument */
				avalue[i] = ptr;
				ਰੋਕੋ;
		}
		ptr += ਸ਼ਬਦ_FFI_ALIGN(arg_types[i]->ਮਾਪ);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->ਮਾਪ;
	*rtype = cif->rtype->type;
	ਜੇ (struct_rvalue != ਨਿਰਾ) {
		ਘੇਰਾ->fun(cif, struct_rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);
		/* copy struct return pointer value into function return value */
		*((ਖਾਲੀ**)rvalue) = struct_rvalue;
	} ਨਹੀਂਤਾਂ {
		ਘੇਰਾ->fun(cif, rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);
	}
}

ffi_ਸਥਿਤੀ ffi_prep_ਘੇਰਾ_loc(
		ffi_ਘੇਰਾ* ਘੇਰਾ, ffi_cif* cif,
		ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		ਖਾਲੀ* user_ਅੰਕੜੇ, ਖਾਲੀ* codeloc)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ* tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ*)&(ਘੇਰਾ->tramp[0]);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ cls = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)codeloc;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ fn = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ fn_ਘੇਰਾ_call_sysv = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ffi_ਘੇਰਾ_call_SYSV;

	ਘੇਰਾ->cif = cif;
	ਘੇਰਾ->fun = fun;
	ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

	ਚੋਣ (cif->abi)
	{
	ਹਾਲਤ FFI_SYSV:
		fn = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ffi_ਘੇਰਾ_SYSV;

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
		tramp[4] = 0xb0000000 | ((fn_ਘੇਰਾ_call_sysv >> 16) & 0xffff);
		/* addik r3, r0, fn_closure_call_sysv(lower) */
		tramp[5] = 0x30600000 | (fn_ਘੇਰਾ_call_sysv & 0xffff);
		/* branch/jump to address stored in r11 (fn) */
		tramp[6] = 0x98085800; /* bra r11 */

		ਰੋਕੋ;
	ਮੂਲ:
		ਵਾਪਸ FFI_BAD_ABI;
	}
	ਵਾਪਸ FFI_OK;
}
