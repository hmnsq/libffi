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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ (*)(ಶೂನ್ಯ*, extended_cif*), extended_cif*,
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ),
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_SYSV(ಶೂನ್ಯ);

#ವ್ಯಾಖ್ಯಾನಿಸು ಶಬ್ದ_ಗಾತ್ರ			ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)
#ವ್ಯಾಖ್ಯಾನಿಸು ವಾದಗಳು_ರಿಜಿಸ್ಟರ್_ಗಾತ್ರ	(ಶಬ್ದ_ಗಾತ್ರ * 6)
#ವ್ಯಾಖ್ಯಾನಿಸು ಶಬ್ದ_FFI_ALIGN(x)		FFI_ALIGN(x, ಶಬ್ದ_ಗಾತ್ರ)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
ಶೂನ್ಯ ffi_prep_ವಾದಗಳು(ಶೂನ್ಯ* ರಾಶಿ, extended_cif* ecif)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ffi_type** p_arg;
	ಶೂನ್ಯ** p_argv;
	ಶೂನ್ಯ* ರಾಶಿ_ವಾದಗಳು_p = ರಾಶಿ;

	ಆದರೆ (ecif == ಖಾಲಿ || ecif->cif == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	ಆದರೆ ((ecif->cif->rtype != ಖಾಲಿ) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		ಅಕ್ಷರ* addr = ರಾಶಿ_ವಾದಗಳು_p;
		ಸ್ಮರಣೆ_ನಕಲು(addr, &(ecif->rvalue), ಶಬ್ದ_ಗಾತ್ರ);
		ರಾಶಿ_ವಾದಗಳು_p += ಶಬ್ದ_ಗಾತ್ರ;
	}

	ಆದರೆ (ecif->avalue == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ; /* no arguments to prepare */
	}

	ಗಾಗಿ (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t ಗಾತ್ರ_2 = (*p_arg)->ಗಾತ್ರ_2;
		ಪೂರ್ಣಾಂಕ type = (*p_arg)->type;
		ಶೂನ್ಯ* ಮೌಲ್ಯ = p_argv[i];
		ಅಕ್ಷರ* addr = ರಾಶಿ_ವಾದಗಳು_p;
		ಪೂರ್ಣಾಂಕ aligned_ಗಾತ್ರ = ಶಬ್ದ_FFI_ALIGN(ಗಾತ್ರ_2);

		/* force word alignment on the stack */
		ರಾಶಿ_ವಾದಗಳು_p += aligned_ಗಾತ್ರ;
		
		ಆಯ್ಕೆ (type)
		{
			ಸ್ಥಿತಿ FFI_TYPE_UINT8:
				*(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *)addr = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT8*)(ಮೌಲ್ಯ);
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ FFI_TYPE_SINT8:
				*(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *)addr = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT8*)(ಮೌಲ್ಯ);
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ FFI_TYPE_UINT16:
				*(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *)addr = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT16*)(ಮೌಲ್ಯ);
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ FFI_TYPE_SINT16:
				*(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *)addr = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT16*)(ಮೌಲ್ಯ);
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
#ಆದರೆ __BIG_ENDIAN__
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
				ಆದರೆ (ಗಾತ್ರ_2 < ಶಬ್ದ_ಗಾತ್ರ)
				{
				  ಸ್ಮರಣೆ_ನಕಲು (addr + (ಶಬ್ದ_ಗಾತ್ರ - ಗಾತ್ರ_2), ಮೌಲ್ಯ, ಗಾತ್ರ_2);
				  ನಿಲ್ಲಿಸು;
				}
#ಆದರೆ_ಅಂತ್ಯ
			ಸ್ಥಿತಿ FFI_TYPE_SINT32:
			ಸ್ಥಿತಿ FFI_TYPE_UINT32:
			ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
			ಸ್ಥಿತಿ FFI_TYPE_SINT64:
			ಸ್ಥಿತಿ FFI_TYPE_UINT64:
			ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
			ಪೂರ್ವನಿಯೋಜಿತ:
				ಸ್ಮರಣೆ_ನಕಲು(addr, ಮೌಲ್ಯ, aligned_ಗಾತ್ರ);
		}
	}
}

ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	ಆಯ್ಕೆ (cif->abi)
	{
		ಸ್ಥಿತಿ FFI_SYSV:
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
	}
	ಹಿಂತಿರುಗಿ FFI_OK;
}

ಶೂನ್ಯ ffi_call(ffi_cif* cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ* rvalue, ಶೂನ್ಯ** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	ಆದರೆ ((rvalue == ಖಾಲಿ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->ಗಾತ್ರ_2);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ecif.rvalue = rvalue;
	}

	ಆಯ್ಕೆ (cif->abi)
	{
	ಸ್ಥಿತಿ FFI_SYSV:
		ffi_call_SYSV(ffi_prep_ವಾದಗಳು, &ecif, cif->ಬೈಟುಗಳು, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->ಗಾತ್ರ_2);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		FFI_ASSERT(0);
		ನಿಲ್ಲಿಸು;
	}
}

ಶೂನ್ಯ ffi_ಆವರಣ_call_SYSV(ಶೂನ್ಯ* ರಿಜಿಸ್ಟರ್_ವಾದಗಳು, ಶೂನ್ಯ* ರಾಶಿ_ವಾದಗಳು,
			ffi_ಆವರಣ* ಆವರಣ, ಶೂನ್ಯ* rvalue,
			ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ* rtype, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = ಆವರಣ->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	ಅಕ್ಷರ* stackclone = alloca(cif->ಬೈಟುಗಳು);
	ಶೂನ್ಯ** avalue = alloca(cif->nargs * ಗಾತ್ರ(ಶೂನ್ಯ*));
	ಶೂನ್ಯ* struct_rvalue = ಖಾಲಿ;
	ಅಕ್ಷರ* ptr = stackclone;
	ಪೂರ್ಣಾಂಕ i;

	/* copy registers into stack clone */
	ಪೂರ್ಣಾಂಕ registers_ಬಳಸಿದ = cif->ಬೈಟುಗಳು;
	ಆದರೆ (registers_ಬಳಸಿದ > ವಾದಗಳು_ರಿಜಿಸ್ಟರ್_ಗಾತ್ರ) {
		registers_ಬಳಸಿದ = ವಾದಗಳು_ರಿಜಿಸ್ಟರ್_ಗಾತ್ರ;
	}
	ಸ್ಮರಣೆ_ನಕಲು(stackclone, ರಿಜಿಸ್ಟರ್_ವಾದಗಳು, registers_ಬಳಸಿದ);

	/* copy stack allocated args into stack clone */
	ಆದರೆ (cif->ಬೈಟುಗಳು > ವಾದಗಳು_ರಿಜಿಸ್ಟರ್_ಗಾತ್ರ) {
		ಪೂರ್ಣಾಂಕ ರಾಶಿ_ಬಳಸಿದ = cif->ಬೈಟುಗಳು - ವಾದಗಳು_ರಿಜಿಸ್ಟರ್_ಗಾತ್ರ;
		ಸ್ಮರಣೆ_ನಕಲು(stackclone + ವಾದಗಳು_ರಿಜಿಸ್ಟರ್_ಗಾತ್ರ, ರಾಶಿ_ವಾದಗಳು, ರಾಶಿ_ಬಳಸಿದ);
	}

	/* preserve struct type return pointer passing */
	ಆದರೆ ((cif->rtype != ಖಾಲಿ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((ಶೂನ್ಯ**)ptr);
		ptr += ಶಬ್ದ_ಗಾತ್ರ;
	}

	/* populate arg pointer list */
	ಗಾಗಿ (i = 0; i < cif->nargs; i++)
	{
		ಆಯ್ಕೆ (arg_types[i]->type)
		{
			ಸ್ಥಿತಿ FFI_TYPE_SINT8:
			ಸ್ಥಿತಿ FFI_TYPE_UINT8:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#ಇಲ್ಲದಿದ್ದರೆ
				avalue[i] = ptr;
#ಆದರೆ_ಅಂತ್ಯ
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ FFI_TYPE_SINT16:
			ಸ್ಥಿತಿ FFI_TYPE_UINT16:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#ಇಲ್ಲದಿದ್ದರೆ
				avalue[i] = ptr;
#ಆದರೆ_ಅಂತ್ಯ
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
#ಆದರೆ __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				ಆದರೆ (arg_types[i]->ಗಾತ್ರ_2 < ಶಬ್ದ_ಗಾತ್ರ)
				{
				  ಸ್ಮರಣೆ_ನಕಲು (ptr, ptr + (ಶಬ್ದ_ಗಾತ್ರ - arg_types[i]->ಗಾತ್ರ_2), arg_types[i]->ಗಾತ್ರ_2);
				}
#ಆದರೆ_ಅಂತ್ಯ
				avalue[i] = (ಶೂನ್ಯ*)ptr;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ FFI_TYPE_UINT64:
			ಸ್ಥಿತಿ FFI_TYPE_SINT64:
			ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ FFI_TYPE_SINT32:
			ಸ್ಥಿತಿ FFI_TYPE_UINT32:
			ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
			ಪೂರ್ವನಿಯೋಜಿತ:
				/* default 4-byte argument */
				avalue[i] = ptr;
				ನಿಲ್ಲಿಸು;
		}
		ptr += ಶಬ್ದ_FFI_ALIGN(arg_types[i]->ಗಾತ್ರ_2);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->ಗಾತ್ರ_2;
	*rtype = cif->rtype->type;
	ಆದರೆ (struct_rvalue != ಖಾಲಿ) {
		ಆವರಣ->fun(cif, struct_rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);
		/* copy struct return pointer value into function return value */
		*((ಶೂನ್ಯ**)rvalue) = struct_rvalue;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆವರಣ->fun(cif, rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);
	}
}

ffi_ಸ್ಥಿತಿ ffi_prep_ಆವರಣ_loc(
		ffi_ಆವರಣ* ಆವರಣ, ffi_cif* cif,
		ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		ಶೂನ್ಯ* user_ದತ್ತಾಂಶ, ಶೂನ್ಯ* codeloc)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ* tramp = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ*)&(ಆವರಣ->tramp[0]);
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ cls = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)codeloc;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ fn = 0;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ fn_ಆವರಣ_call_sysv = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ffi_ಆವರಣ_call_SYSV;

	ಆವರಣ->cif = cif;
	ಆವರಣ->fun = fun;
	ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

	ಆಯ್ಕೆ (cif->abi)
	{
	ಸ್ಥಿತಿ FFI_SYSV:
		fn = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ffi_ಆವರಣ_SYSV;

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
		tramp[4] = 0xb0000000 | ((fn_ಆವರಣ_call_sysv >> 16) & 0xffff);
		/* addik r3, r0, fn_closure_call_sysv(lower) */
		tramp[5] = 0x30600000 | (fn_ಆವರಣ_call_sysv & 0xffff);
		/* branch/jump to address stored in r11 (fn) */
		tramp[6] = 0x98085800; /* bra r11 */

		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
	}
	ಹಿಂತಿರುಗಿ FFI_OK;
}
