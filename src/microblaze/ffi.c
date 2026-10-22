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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>

বাহ্য শূন্য ffi_call_SYSV(শূন্য (*)(শূন্য*, extended_cif*), extended_cif*,
		unsigned পূর্ণাঙ্ক, unsigned পূর্ণাঙ্ক, unsigned পূর্ণাঙ্ক*, শূন্য (*fn)(শূন্য),
		unsigned পূর্ণাঙ্ক, unsigned পূর্ণাঙ্ক);

বাহ্য শূন্য ffi_closure_SYSV(শূন্য);

#define WORD_SIZE			পরিমাণ(unsigned পূর্ণাঙ্ক)
#define ARGS_REGISTER_SIZE	(WORD_SIZE * 6)
#define WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_SIZE)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
শূন্য ffi_prep_args(শূন্য* স্তূপ, extended_cif* ecif)
{
	unsigned পূর্ণাঙ্ক i;
	ffi_type** p_arg;
	শূন্য** p_argv;
	শূন্য* স্তূপ_args_p = স্তূপ;

	যদি (ecif == রিক্ত || ecif->cif == রিক্ত) {
		ঘূৰাই; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	যদি ((ecif->cif->rtype != রিক্ত) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		ৱর্ণ* addr = স্তূপ_args_p;
		memcpy(addr, &(ecif->rvalue), WORD_SIZE);
		স্তূপ_args_p += WORD_SIZE;
	}

	যদি (ecif->avalue == রিক্ত) {
		ঘূৰাই; /* no arguments to prepare */
	}

	বাবে (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t size = (*p_arg)->size;
		পূর্ণাঙ্ক type = (*p_arg)->type;
		শূন্য* মান = p_argv[i];
		ৱর্ণ* addr = স্তূপ_args_p;
		পূর্ণাঙ্ক aligned_size = WORD_FFI_ALIGN(size);

		/* force word alignment on the stack */
		স্তূপ_args_p += aligned_size;
		
		ৱিকল্প (type)
		{
			পক্ষ FFI_TYPE_UINT8:
				*(unsigned পূর্ণাঙ্ক *)addr = (unsigned পূর্ণাঙ্ক)*(UINT8*)(মান);
				ভঙ্গ;
			পক্ষ FFI_TYPE_SINT8:
				*(signed পূর্ণাঙ্ক *)addr = (signed পূর্ণাঙ্ক)*(SINT8*)(মান);
				ভঙ্গ;
			পক্ষ FFI_TYPE_UINT16:
				*(unsigned পূর্ণাঙ্ক *)addr = (unsigned পূর্ণাঙ্ক)*(UINT16*)(মান);
				ভঙ্গ;
			পক্ষ FFI_TYPE_SINT16:
				*(signed পূর্ণাঙ্ক *)addr = (signed পূর্ণাঙ্ক)*(SINT16*)(মান);
				ভঙ্গ;
			পক্ষ FFI_TYPE_STRUCT:
#যদি __BIG_ENDIAN__
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
				যদি (size < WORD_SIZE)
				{
				  memcpy (addr + (WORD_SIZE - size), মান, size);
				  ভঙ্গ;
				}
#endif
			পক্ষ FFI_TYPE_SINT32:
			পক্ষ FFI_TYPE_UINT32:
			পক্ষ FFI_TYPE_FLOAT:
			পক্ষ FFI_TYPE_SINT64:
			পক্ষ FFI_TYPE_UINT64:
			পক্ষ FFI_TYPE_DOUBLE:
			স্ৱভাৱ:
				memcpy(addr, মান, aligned_size);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	ৱিকল্প (cif->abi)
	{
		পক্ষ FFI_SYSV:
			ভঙ্গ;
		স্ৱভাৱ:
			ঘূৰাই FFI_BAD_ABI;
	}
	ঘূৰাই FFI_OK;
}

শূন্য ffi_call(ffi_cif* cif, শূন্য (*fn)(শূন্য), শূন্য* rvalue, শূন্য** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	যদি ((rvalue == রিক্ত) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->size);
	} অন্যথা {
		ecif.rvalue = rvalue;
	}

	ৱিকল্প (cif->abi)
	{
	পক্ষ FFI_SYSV:
		ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->size);
		ভঙ্গ;
	স্ৱভাৱ:
		FFI_ASSERT(0);
		ভঙ্গ;
	}
}

শূন্য ffi_closure_call_SYSV(শূন্য* register_args, শূন্য* স্তূপ_args,
			ffi_closure* closure, শূন্য* rvalue,
			unsigned পূর্ণাঙ্ক* rtype, unsigned পূর্ণাঙ্ক* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = closure->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	ৱর্ণ* stackclone = alloca(cif->bytes);
	শূন্য** avalue = alloca(cif->nargs * পরিমাণ(শূন্য*));
	শূন্য* struct_rvalue = রিক্ত;
	ৱর্ণ* ptr = stackclone;
	পূর্ণাঙ্ক i;

	/* copy registers into stack clone */
	পূর্ণাঙ্ক registers_used = cif->bytes;
	যদি (registers_used > ARGS_REGISTER_SIZE) {
		registers_used = ARGS_REGISTER_SIZE;
	}
	memcpy(stackclone, register_args, registers_used);

	/* copy stack allocated args into stack clone */
	যদি (cif->bytes > ARGS_REGISTER_SIZE) {
		পূর্ণাঙ্ক স্তূপ_used = cif->bytes - ARGS_REGISTER_SIZE;
		memcpy(stackclone + ARGS_REGISTER_SIZE, স্তূপ_args, স্তূপ_used);
	}

	/* preserve struct type return pointer passing */
	যদি ((cif->rtype != রিক্ত) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((শূন্য**)ptr);
		ptr += WORD_SIZE;
	}

	/* populate arg pointer list */
	বাবে (i = 0; i < cif->nargs; i++)
	{
		ৱিকল্প (arg_types[i]->type)
		{
			পক্ষ FFI_TYPE_SINT8:
			পক্ষ FFI_TYPE_UINT8:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#অন্যথা
				avalue[i] = ptr;
#endif
				ভঙ্গ;
			পক্ষ FFI_TYPE_SINT16:
			পক্ষ FFI_TYPE_UINT16:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#অন্যথা
				avalue[i] = ptr;
#endif
				ভঙ্গ;
			পক্ষ FFI_TYPE_STRUCT:
#যদি __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				যদি (arg_types[i]->size < WORD_SIZE)
				{
				  memcpy (ptr, ptr + (WORD_SIZE - arg_types[i]->size), arg_types[i]->size);
				}
#endif
				avalue[i] = (শূন্য*)ptr;
				ভঙ্গ;
			পক্ষ FFI_TYPE_UINT64:
			পক্ষ FFI_TYPE_SINT64:
			পক্ষ FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				ভঙ্গ;
			পক্ষ FFI_TYPE_SINT32:
			পক্ষ FFI_TYPE_UINT32:
			পক্ষ FFI_TYPE_FLOAT:
			স্ৱভাৱ:
				/* default 4-byte argument */
				avalue[i] = ptr;
				ভঙ্গ;
		}
		ptr += WORD_FFI_ALIGN(arg_types[i]->size);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->size;
	*rtype = cif->rtype->type;
	যদি (struct_rvalue != রিক্ত) {
		closure->fun(cif, struct_rvalue, avalue, closure->user_data);
		/* copy struct return pointer value into function return value */
		*((শূন্য**)rvalue) = struct_rvalue;
	} অন্যথা {
		closure->fun(cif, rvalue, avalue, closure->user_data);
	}
}

ffi_status ffi_prep_closure_loc(
		ffi_closure* closure, ffi_cif* cif,
		শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		শূন্য* user_data, শূন্য* codeloc)
{
	unsigned long* tramp = (unsigned long*)&(closure->tramp[0]);
	unsigned long cls = (unsigned long)codeloc;
	unsigned long fn = 0;
	unsigned long fn_closure_call_sysv = (unsigned long)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	ৱিকল্প (cif->abi)
	{
	পক্ষ FFI_SYSV:
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

		ভঙ্গ;
	স্ৱভাৱ:
		ঘূৰাই FFI_BAD_ABI;
	}
	ঘূৰাই FFI_OK;
}
