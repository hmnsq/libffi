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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

خارجي فراغ ffi_call_SYSV(فراغ (*)(فراغ*, extended_cif*), extended_cif*,
		بنانشانی صحيح, بنانشانی صحيح, بنانشانی صحيح*, فراغ (*fn)(فراغ),
		بنانشانی صحيح, بنانشانی صحيح);

خارجي فراغ ffi_گھیرا_SYSV(فراغ);

#تعریف_کرو لفظ_ناپ			حجم(بنانشانی صحيح)
#تعریف_کرو دلیلاں_رجسٹر_ناپ	(لفظ_ناپ * 6)
#تعریف_کرو لفظ_FFI_ALIGN(x)		FFI_ALIGN(x, لفظ_ناپ)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
فراغ ffi_prep_دلیلاں(فراغ* ڈھیر, extended_cif* ecif)
{
	بنانشانی صحيح i;
	ffi_type** p_arg;
	فراغ** p_argv;
	فراغ* ڈھیر_دلیلاں_p = ڈھیر;

	إذا (ecif == عدم || ecif->cif == عدم) {
		أرجع; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	إذا ((ecif->cif->rtype != عدم) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		حرف* addr = ڈھیر_دلیلاں_p;
		حافظہ_نقل(addr, &(ecif->rvalue), لفظ_ناپ);
		ڈھیر_دلیلاں_p += لفظ_ناپ;
	}

	إذا (ecif->avalue == عدم) {
		أرجع; /* no arguments to prepare */
	}

	لكل (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t ناپ = (*p_arg)->ناپ;
		صحيح type = (*p_arg)->type;
		فراغ* قدر = p_argv[i];
		حرف* addr = ڈھیر_دلیلاں_p;
		صحيح aligned_ناپ = لفظ_FFI_ALIGN(ناپ);

		/* force word alignment on the stack */
		ڈھیر_دلیلاں_p += aligned_ناپ;
		
		بدل (type)
		{
			حالة FFI_TYPE_UINT8:
				*(بنانشانی صحيح *)addr = (بنانشانی صحيح)*(UINT8*)(قدر);
				اكسر;
			حالة FFI_TYPE_SINT8:
				*(نشانیوالا صحيح *)addr = (نشانیوالا صحيح)*(SINT8*)(قدر);
				اكسر;
			حالة FFI_TYPE_UINT16:
				*(بنانشانی صحيح *)addr = (بنانشانی صحيح)*(UINT16*)(قدر);
				اكسر;
			حالة FFI_TYPE_SINT16:
				*(نشانیوالا صحيح *)addr = (نشانیوالا صحيح)*(SINT16*)(قدر);
				اكسر;
			حالة FFI_TYPE_STRUCT:
#إذا __BIG_ENDIAN__
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
				إذا (ناپ < لفظ_ناپ)
				{
				  حافظہ_نقل (addr + (لفظ_ناپ - ناپ), قدر, ناپ);
				  اكسر;
				}
#جے_انت
			حالة FFI_TYPE_SINT32:
			حالة FFI_TYPE_UINT32:
			حالة FFI_TYPE_FLOAT:
			حالة FFI_TYPE_SINT64:
			حالة FFI_TYPE_UINT64:
			حالة FFI_TYPE_DOUBLE:
			افتراضي:
				حافظہ_نقل(addr, قدر, aligned_ناپ);
		}
	}
}

ffi_حالت ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	بدل (cif->abi)
	{
		حالة FFI_SYSV:
			اكسر;
		افتراضي:
			أرجع FFI_BAD_ABI;
	}
	أرجع FFI_OK;
}

فراغ ffi_call(ffi_cif* cif, فراغ (*fn)(فراغ), فراغ* rvalue, فراغ** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	إذا ((rvalue == عدم) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->ناپ);
	} وإلا {
		ecif.rvalue = rvalue;
	}

	بدل (cif->abi)
	{
	حالة FFI_SYSV:
		ffi_call_SYSV(ffi_prep_دلیلاں, &ecif, cif->بائٹاں, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->ناپ);
		اكسر;
	افتراضي:
		FFI_ASSERT(0);
		اكسر;
	}
}

فراغ ffi_گھیرا_call_SYSV(فراغ* رجسٹر_دلیلاں, فراغ* ڈھیر_دلیلاں,
			ffi_گھیرا* گھیرا, فراغ* rvalue,
			بنانشانی صحيح* rtype, بنانشانی صحيح* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = گھیرا->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	حرف* stackclone = alloca(cif->بائٹاں);
	فراغ** avalue = alloca(cif->nargs * حجم(فراغ*));
	فراغ* struct_rvalue = عدم;
	حرف* ptr = stackclone;
	صحيح i;

	/* copy registers into stack clone */
	صحيح registers_ورتیا = cif->بائٹاں;
	إذا (registers_ورتیا > دلیلاں_رجسٹر_ناپ) {
		registers_ورتیا = دلیلاں_رجسٹر_ناپ;
	}
	حافظہ_نقل(stackclone, رجسٹر_دلیلاں, registers_ورتیا);

	/* copy stack allocated args into stack clone */
	إذا (cif->بائٹاں > دلیلاں_رجسٹر_ناپ) {
		صحيح ڈھیر_ورتیا = cif->بائٹاں - دلیلاں_رجسٹر_ناپ;
		حافظہ_نقل(stackclone + دلیلاں_رجسٹر_ناپ, ڈھیر_دلیلاں, ڈھیر_ورتیا);
	}

	/* preserve struct type return pointer passing */
	إذا ((cif->rtype != عدم) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((فراغ**)ptr);
		ptr += لفظ_ناپ;
	}

	/* populate arg pointer list */
	لكل (i = 0; i < cif->nargs; i++)
	{
		بدل (arg_types[i]->type)
		{
			حالة FFI_TYPE_SINT8:
			حالة FFI_TYPE_UINT8:
#جے_تعریف_شدہ __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#وإلا
				avalue[i] = ptr;
#جے_انت
				اكسر;
			حالة FFI_TYPE_SINT16:
			حالة FFI_TYPE_UINT16:
#جے_تعریف_شدہ __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#وإلا
				avalue[i] = ptr;
#جے_انت
				اكسر;
			حالة FFI_TYPE_STRUCT:
#إذا __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				إذا (arg_types[i]->ناپ < لفظ_ناپ)
				{
				  حافظہ_نقل (ptr, ptr + (لفظ_ناپ - arg_types[i]->ناپ), arg_types[i]->ناپ);
				}
#جے_انت
				avalue[i] = (فراغ*)ptr;
				اكسر;
			حالة FFI_TYPE_UINT64:
			حالة FFI_TYPE_SINT64:
			حالة FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				اكسر;
			حالة FFI_TYPE_SINT32:
			حالة FFI_TYPE_UINT32:
			حالة FFI_TYPE_FLOAT:
			افتراضي:
				/* default 4-byte argument */
				avalue[i] = ptr;
				اكسر;
		}
		ptr += لفظ_FFI_ALIGN(arg_types[i]->ناپ);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->ناپ;
	*rtype = cif->rtype->type;
	إذا (struct_rvalue != عدم) {
		گھیرا->fun(cif, struct_rvalue, avalue, گھیرا->user_انکڑے);
		/* copy struct return pointer value into function return value */
		*((فراغ**)rvalue) = struct_rvalue;
	} وإلا {
		گھیرا->fun(cif, rvalue, avalue, گھیرا->user_انکڑے);
	}
}

ffi_حالت ffi_prep_گھیرا_loc(
		ffi_گھیرا* گھیرا, ffi_cif* cif,
		فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		فراغ* user_انکڑے, فراغ* codeloc)
{
	بنانشانی لما* tramp = (بنانشانی لما*)&(گھیرا->tramp[0]);
	بنانشانی لما cls = (بنانشانی لما)codeloc;
	بنانشانی لما fn = 0;
	بنانشانی لما fn_گھیرا_call_sysv = (بنانشانی لما)ffi_گھیرا_call_SYSV;

	گھیرا->cif = cif;
	گھیرا->fun = fun;
	گھیرا->user_انکڑے = user_انکڑے;

	بدل (cif->abi)
	{
	حالة FFI_SYSV:
		fn = (بنانشانی لما)ffi_گھیرا_SYSV;

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
		tramp[4] = 0xb0000000 | ((fn_گھیرا_call_sysv >> 16) & 0xffff);
		/* addik r3, r0, fn_closure_call_sysv(lower) */
		tramp[5] = 0x30600000 | (fn_گھیرا_call_sysv & 0xffff);
		/* branch/jump to address stored in r11 (fn) */
		tramp[6] = 0x98085800; /* bra r11 */

		اكسر;
	افتراضي:
		أرجع FFI_BAD_ABI;
	}
	أرجع FFI_OK;
}
