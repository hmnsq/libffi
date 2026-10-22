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
		unsigned ಪೂರ್ಣಾಙ್ಕ, unsigned ಪೂರ್ಣಾಙ್ಕ, unsigned ಪೂರ್ಣಾಙ್ಕ*, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ),
		unsigned ಪೂರ್ಣಾಙ್ಕ, unsigned ಪೂರ್ಣಾಙ್ಕ);

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_SYSV(ಶೂನ್ಯ);

#define WORD_SIZE			ಪರಿಮಾಣ(unsigned ಪೂರ್ಣಾಙ್ಕ)
#define ARGS_REGISTER_SIZE	(WORD_SIZE * 6)
#define WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_SIZE)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
ಶೂನ್ಯ ffi_prep_args(ಶೂನ್ಯ* ರಾಶಿ, extended_cif* ecif)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ffi_type** p_arg;
	ಶೂನ್ಯ** p_argv;
	ಶೂನ್ಯ* ರಾಶಿ_args_p = ರಾಶಿ;

	ಯದಿ (ecif == ರಿಕ್ತ || ecif->cif == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	ಯದಿ ((ecif->cif->rtype != ರಿಕ್ತ) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		ವರ್ಣ* addr = ರಾಶಿ_args_p;
		memcpy(addr, &(ecif->rvalue), WORD_SIZE);
		ರಾಶಿ_args_p += WORD_SIZE;
	}

	ಯದಿ (ecif->avalue == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ; /* no arguments to prepare */
	}

	ಪ್ರತಿ (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t size = (*p_arg)->size;
		ಪೂರ್ಣಾಙ್ಕ type = (*p_arg)->type;
		ಶೂನ್ಯ* ಮೌಲ್ಯ = p_argv[i];
		ವರ್ಣ* addr = ರಾಶಿ_args_p;
		ಪೂರ್ಣಾಙ್ಕ aligned_size = WORD_FFI_ALIGN(size);

		/* force word alignment on the stack */
		ರಾಶಿ_args_p += aligned_size;
		
		ವಿಕಲ್ಪ (type)
		{
			ಪಕ್ಷ FFI_TYPE_UINT8:
				*(unsigned ಪೂರ್ಣಾಙ್ಕ *)addr = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT8*)(ಮೌಲ್ಯ);
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_SINT8:
				*(signed ಪೂರ್ಣಾಙ್ಕ *)addr = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT8*)(ಮೌಲ್ಯ);
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_UINT16:
				*(unsigned ಪೂರ್ಣಾಙ್ಕ *)addr = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT16*)(ಮೌಲ್ಯ);
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_SINT16:
				*(signed ಪೂರ್ಣಾಙ್ಕ *)addr = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT16*)(ಮೌಲ್ಯ);
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_STRUCT:
#ಯದಿ __BIG_ENDIAN__
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
				ಯದಿ (size < WORD_SIZE)
				{
				  memcpy (addr + (WORD_SIZE - size), ಮೌಲ್ಯ, size);
				  ಭಙ್ಗ;
				}
#endif
			ಪಕ್ಷ FFI_TYPE_SINT32:
			ಪಕ್ಷ FFI_TYPE_UINT32:
			ಪಕ್ಷ FFI_TYPE_FLOAT:
			ಪಕ್ಷ FFI_TYPE_SINT64:
			ಪಕ್ಷ FFI_TYPE_UINT64:
			ಪಕ್ಷ FFI_TYPE_DOUBLE:
			ಸ್ವಭಾವ:
				memcpy(addr, ಮೌಲ್ಯ, aligned_size);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	ವಿಕಲ್ಪ (cif->abi)
	{
		ಪಕ್ಷ FFI_SYSV:
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
	}
	ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಶೂನ್ಯ ffi_call(ffi_cif* cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ* rvalue, ಶೂನ್ಯ** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	ಯದಿ ((rvalue == ರಿಕ್ತ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->size);
	} ಅನ್ಯಥಾ {
		ecif.rvalue = rvalue;
	}

	ವಿಕಲ್ಪ (cif->abi)
	{
	ಪಕ್ಷ FFI_SYSV:
		ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->size);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		FFI_ASSERT(0);
		ಭಙ್ಗ;
	}
}

ಶೂನ್ಯ ffi_closure_call_SYSV(ಶೂನ್ಯ* register_args, ಶೂನ್ಯ* ರಾಶಿ_args,
			ffi_closure* closure, ಶೂನ್ಯ* rvalue,
			unsigned ಪೂರ್ಣಾಙ್ಕ* rtype, unsigned ಪೂರ್ಣಾಙ್ಕ* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = closure->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	ವರ್ಣ* stackclone = alloca(cif->bytes);
	ಶೂನ್ಯ** avalue = alloca(cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ*));
	ಶೂನ್ಯ* struct_rvalue = ರಿಕ್ತ;
	ವರ್ಣ* ptr = stackclone;
	ಪೂರ್ಣಾಙ್ಕ i;

	/* copy registers into stack clone */
	ಪೂರ್ಣಾಙ್ಕ registers_used = cif->bytes;
	ಯದಿ (registers_used > ARGS_REGISTER_SIZE) {
		registers_used = ARGS_REGISTER_SIZE;
	}
	memcpy(stackclone, register_args, registers_used);

	/* copy stack allocated args into stack clone */
	ಯದಿ (cif->bytes > ARGS_REGISTER_SIZE) {
		ಪೂರ್ಣಾಙ್ಕ ರಾಶಿ_used = cif->bytes - ARGS_REGISTER_SIZE;
		memcpy(stackclone + ARGS_REGISTER_SIZE, ರಾಶಿ_args, ರಾಶಿ_used);
	}

	/* preserve struct type return pointer passing */
	ಯದಿ ((cif->rtype != ರಿಕ್ತ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((ಶೂನ್ಯ**)ptr);
		ptr += WORD_SIZE;
	}

	/* populate arg pointer list */
	ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
	{
		ವಿಕಲ್ಪ (arg_types[i]->type)
		{
			ಪಕ್ಷ FFI_TYPE_SINT8:
			ಪಕ್ಷ FFI_TYPE_UINT8:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#ಅನ್ಯಥಾ
				avalue[i] = ptr;
#endif
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_SINT16:
			ಪಕ್ಷ FFI_TYPE_UINT16:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#ಅನ್ಯಥಾ
				avalue[i] = ptr;
#endif
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_STRUCT:
#ಯದಿ __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				ಯದಿ (arg_types[i]->size < WORD_SIZE)
				{
				  memcpy (ptr, ptr + (WORD_SIZE - arg_types[i]->size), arg_types[i]->size);
				}
#endif
				avalue[i] = (ಶೂನ್ಯ*)ptr;
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_UINT64:
			ಪಕ್ಷ FFI_TYPE_SINT64:
			ಪಕ್ಷ FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				ಭಙ್ಗ;
			ಪಕ್ಷ FFI_TYPE_SINT32:
			ಪಕ್ಷ FFI_TYPE_UINT32:
			ಪಕ್ಷ FFI_TYPE_FLOAT:
			ಸ್ವಭಾವ:
				/* default 4-byte argument */
				avalue[i] = ptr;
				ಭಙ್ಗ;
		}
		ptr += WORD_FFI_ALIGN(arg_types[i]->size);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->size;
	*rtype = cif->rtype->type;
	ಯದಿ (struct_rvalue != ರಿಕ್ತ) {
		closure->fun(cif, struct_rvalue, avalue, closure->user_data);
		/* copy struct return pointer value into function return value */
		*((ಶೂನ್ಯ**)rvalue) = struct_rvalue;
	} ಅನ್ಯಥಾ {
		closure->fun(cif, rvalue, avalue, closure->user_data);
	}
}

ffi_status ffi_prep_closure_loc(
		ffi_closure* closure, ffi_cif* cif,
		ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		ಶೂನ್ಯ* user_data, ಶೂನ್ಯ* codeloc)
{
	unsigned long* tramp = (unsigned long*)&(closure->tramp[0]);
	unsigned long cls = (unsigned long)codeloc;
	unsigned long fn = 0;
	unsigned long fn_closure_call_sysv = (unsigned long)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	ವಿಕಲ್ಪ (cif->abi)
	{
	ಪಕ್ಷ FFI_SYSV:
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

		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
	}
	ಪ್ರತ್ಯಾಗಮ FFI_OK;
}
