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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV(ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ*, extended_cif*), extended_cif*,
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ),
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_SYSV(ଶୂନ୍ୟ);

#define WORD_SIZE			ଆକାର(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)
#define ARGS_REGISTER_SIZE	(WORD_SIZE * 6)
#define WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_SIZE)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
ଶୂନ୍ୟ ffi_prep_args(ଶୂନ୍ୟ* ଗଦା, extended_cif* ecif)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ffi_type** p_arg;
	ଶୂନ୍ୟ** p_argv;
	ଶୂନ୍ୟ* ଗଦା_args_p = ଗଦା;

	ଯଦି (ecif == ଖାଲି || ecif->cif == ଖାଲି) {
		ଫେରନ୍ତୁ; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	ଯଦି ((ecif->cif->rtype != ଖାଲି) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		ଅକ୍ଷର* addr = ଗଦା_args_p;
		memcpy(addr, &(ecif->rvalue), WORD_SIZE);
		ଗଦା_args_p += WORD_SIZE;
	}

	ଯଦି (ecif->avalue == ଖାଲି) {
		ଫେରନ୍ତୁ; /* no arguments to prepare */
	}

	ପାଇଁ (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t size = (*p_arg)->size;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା type = (*p_arg)->type;
		ଶୂନ୍ୟ* ମୂଲ୍ୟ = p_argv[i];
		ଅକ୍ଷର* addr = ଗଦା_args_p;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା aligned_size = WORD_FFI_ALIGN(size);

		/* force word alignment on the stack */
		ଗଦା_args_p += aligned_size;
		
		ବାଛ (type)
		{
			ସ୍ଥିତି FFI_TYPE_UINT8:
				*(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)addr = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT8*)(ମୂଲ୍ୟ);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_SINT8:
				*(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)addr = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT8*)(ମୂଲ୍ୟ);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_UINT16:
				*(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)addr = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT16*)(ମୂଲ୍ୟ);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_SINT16:
				*(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)addr = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT16*)(ମୂଲ୍ୟ);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_STRUCT:
#ଯଦି __BIG_ENDIAN__
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
				ଯଦି (size < WORD_SIZE)
				{
				  memcpy (addr + (WORD_SIZE - size), ମୂଲ୍ୟ, size);
				  ଭାଙ୍ଗ;
				}
#endif
			ସ୍ଥିତି FFI_TYPE_SINT32:
			ସ୍ଥିତି FFI_TYPE_UINT32:
			ସ୍ଥିତି FFI_TYPE_FLOAT:
			ସ୍ଥିତି FFI_TYPE_SINT64:
			ସ୍ଥିତି FFI_TYPE_UINT64:
			ସ୍ଥିତି FFI_TYPE_DOUBLE:
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				memcpy(addr, ମୂଲ୍ୟ, aligned_size);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	ବାଛ (cif->abi)
	{
		ସ୍ଥିତି FFI_SYSV:
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଫେରନ୍ତୁ FFI_BAD_ABI;
	}
	ଫେରନ୍ତୁ FFI_OK;
}

ଶୂନ୍ୟ ffi_call(ffi_cif* cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ* rvalue, ଶୂନ୍ୟ** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	ଯଦି ((rvalue == ଖାଲି) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->size);
	} ଅନ୍ୟଥା {
		ecif.rvalue = rvalue;
	}

	ବାଛ (cif->abi)
	{
	ସ୍ଥିତି FFI_SYSV:
		ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->size);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		FFI_ASSERT(0);
		ଭାଙ୍ଗ;
	}
}

ଶୂନ୍ୟ ffi_closure_call_SYSV(ଶୂନ୍ୟ* register_args, ଶୂନ୍ୟ* ଗଦା_args,
			ffi_closure* closure, ଶୂନ୍ୟ* rvalue,
			unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା* rtype, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = closure->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	ଅକ୍ଷର* stackclone = alloca(cif->bytes);
	ଶୂନ୍ୟ** avalue = alloca(cif->nargs * ଆକାର(ଶୂନ୍ୟ*));
	ଶୂନ୍ୟ* struct_rvalue = ଖାଲି;
	ଅକ୍ଷର* ptr = stackclone;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	/* copy registers into stack clone */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା registers_used = cif->bytes;
	ଯଦି (registers_used > ARGS_REGISTER_SIZE) {
		registers_used = ARGS_REGISTER_SIZE;
	}
	memcpy(stackclone, register_args, registers_used);

	/* copy stack allocated args into stack clone */
	ଯଦି (cif->bytes > ARGS_REGISTER_SIZE) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଦା_used = cif->bytes - ARGS_REGISTER_SIZE;
		memcpy(stackclone + ARGS_REGISTER_SIZE, ଗଦା_args, ଗଦା_used);
	}

	/* preserve struct type return pointer passing */
	ଯଦି ((cif->rtype != ଖାଲି) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((ଶୂନ୍ୟ**)ptr);
		ptr += WORD_SIZE;
	}

	/* populate arg pointer list */
	ପାଇଁ (i = 0; i < cif->nargs; i++)
	{
		ବାଛ (arg_types[i]->type)
		{
			ସ୍ଥିତି FFI_TYPE_SINT8:
			ସ୍ଥିତି FFI_TYPE_UINT8:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#ଅନ୍ୟଥା
				avalue[i] = ptr;
#endif
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_SINT16:
			ସ୍ଥିତି FFI_TYPE_UINT16:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#ଅନ୍ୟଥା
				avalue[i] = ptr;
#endif
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_STRUCT:
#ଯଦି __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				ଯଦି (arg_types[i]->size < WORD_SIZE)
				{
				  memcpy (ptr, ptr + (WORD_SIZE - arg_types[i]->size), arg_types[i]->size);
				}
#endif
				avalue[i] = (ଶୂନ୍ୟ*)ptr;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_UINT64:
			ସ୍ଥିତି FFI_TYPE_SINT64:
			ସ୍ଥିତି FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି FFI_TYPE_SINT32:
			ସ୍ଥିତି FFI_TYPE_UINT32:
			ସ୍ଥିତି FFI_TYPE_FLOAT:
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				/* default 4-byte argument */
				avalue[i] = ptr;
				ଭାଙ୍ଗ;
		}
		ptr += WORD_FFI_ALIGN(arg_types[i]->size);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->size;
	*rtype = cif->rtype->type;
	ଯଦି (struct_rvalue != ଖାଲି) {
		closure->fun(cif, struct_rvalue, avalue, closure->user_data);
		/* copy struct return pointer value into function return value */
		*((ଶୂନ୍ୟ**)rvalue) = struct_rvalue;
	} ଅନ୍ୟଥା {
		closure->fun(cif, rvalue, avalue, closure->user_data);
	}
}

ffi_status ffi_prep_closure_loc(
		ffi_closure* closure, ffi_cif* cif,
		ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
		ଶୂନ୍ୟ* user_data, ଶୂନ୍ୟ* codeloc)
{
	unsigned long* tramp = (unsigned long*)&(closure->tramp[0]);
	unsigned long cls = (unsigned long)codeloc;
	unsigned long fn = 0;
	unsigned long fn_closure_call_sysv = (unsigned long)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	ବାଛ (cif->abi)
	{
	ସ୍ଥିତି FFI_SYSV:
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

		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ FFI_BAD_ABI;
	}
	ଫେରନ୍ତୁ FFI_OK;
}
