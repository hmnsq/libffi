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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

ബാഹ്യ ശൂന്യം ffi_call_SYSV(ശൂന്യം (*)(ശൂന്യം*, extended_cif*), extended_cif*,
		unsigned പൂർണ്ണസംഖ്യ, unsigned പൂർണ്ണസംഖ്യ, unsigned പൂർണ്ണസംഖ്യ*, ശൂന്യം (*fn)(ശൂന്യം),
		unsigned പൂർണ്ണസംഖ്യ, unsigned പൂർണ്ണസംഖ്യ);

ബാഹ്യ ശൂന്യം ffi_closure_SYSV(ശൂന്യം);

#define WORD_SIZE			വലുപ്പം(unsigned പൂർണ്ണസംഖ്യ)
#define ARGS_REGISTER_SIZE	(WORD_SIZE * 6)
#define WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_SIZE)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
ശൂന്യം ffi_prep_args(ശൂന്യം* അട്ടി, extended_cif* ecif)
{
	unsigned പൂർണ്ണസംഖ്യ i;
	ffi_type** p_arg;
	ശൂന്യം** p_argv;
	ശൂന്യം* അട്ടി_args_p = അട്ടി;

	എങ്കിൽ (ecif == ശൂന്യ || ecif->cif == ശൂന്യ) {
		തിരികെ; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	എങ്കിൽ ((ecif->cif->rtype != ശൂന്യ) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		അക്ഷരം* addr = അട്ടി_args_p;
		memcpy(addr, &(ecif->rvalue), WORD_SIZE);
		അട്ടി_args_p += WORD_SIZE;
	}

	എങ്കിൽ (ecif->avalue == ശൂന്യ) {
		തിരികെ; /* no arguments to prepare */
	}

	വേണ്ടി (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t size = (*p_arg)->size;
		പൂർണ്ണസംഖ്യ type = (*p_arg)->type;
		ശൂന്യം* മൂല്യം = p_argv[i];
		അക്ഷരം* addr = അട്ടി_args_p;
		പൂർണ്ണസംഖ്യ aligned_size = WORD_FFI_ALIGN(size);

		/* force word alignment on the stack */
		അട്ടി_args_p += aligned_size;
		
		തിരഞ്ഞെടുപ്പ് (type)
		{
			അവസ്ഥ FFI_TYPE_UINT8:
				*(unsigned പൂർണ്ണസംഖ്യ *)addr = (unsigned പൂർണ്ണസംഖ്യ)*(UINT8*)(മൂല്യം);
				നിർത്തുക;
			അവസ്ഥ FFI_TYPE_SINT8:
				*(signed പൂർണ്ണസംഖ്യ *)addr = (signed പൂർണ്ണസംഖ്യ)*(SINT8*)(മൂല്യം);
				നിർത്തുക;
			അവസ്ഥ FFI_TYPE_UINT16:
				*(unsigned പൂർണ്ണസംഖ്യ *)addr = (unsigned പൂർണ്ണസംഖ്യ)*(UINT16*)(മൂല്യം);
				നിർത്തുക;
			അവസ്ഥ FFI_TYPE_SINT16:
				*(signed പൂർണ്ണസംഖ്യ *)addr = (signed പൂർണ്ണസംഖ്യ)*(SINT16*)(മൂല്യം);
				നിർത്തുക;
			അവസ്ഥ FFI_TYPE_STRUCT:
#എങ്കിൽ __BIG_ENDIAN__
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
				എങ്കിൽ (size < WORD_SIZE)
				{
				  memcpy (addr + (WORD_SIZE - size), മൂല്യം, size);
				  നിർത്തുക;
				}
#endif
			അവസ്ഥ FFI_TYPE_SINT32:
			അവസ്ഥ FFI_TYPE_UINT32:
			അവസ്ഥ FFI_TYPE_FLOAT:
			അവസ്ഥ FFI_TYPE_SINT64:
			അവസ്ഥ FFI_TYPE_UINT64:
			അവസ്ഥ FFI_TYPE_DOUBLE:
			സ്ഥിരസ്ഥിതി:
				memcpy(addr, മൂല്യം, aligned_size);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	തിരഞ്ഞെടുപ്പ് (cif->abi)
	{
		അവസ്ഥ FFI_SYSV:
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			തിരികെ FFI_BAD_ABI;
	}
	തിരികെ FFI_OK;
}

ശൂന്യം ffi_call(ffi_cif* cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം* rvalue, ശൂന്യം** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	എങ്കിൽ ((rvalue == ശൂന്യ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->size);
	} അല്ലെങ്കിൽ {
		ecif.rvalue = rvalue;
	}

	തിരഞ്ഞെടുപ്പ് (cif->abi)
	{
	അവസ്ഥ FFI_SYSV:
		ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->size);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		FFI_ASSERT(0);
		നിർത്തുക;
	}
}

ശൂന്യം ffi_closure_call_SYSV(ശൂന്യം* register_args, ശൂന്യം* അട്ടി_args,
			ffi_closure* closure, ശൂന്യം* rvalue,
			unsigned പൂർണ്ണസംഖ്യ* rtype, unsigned പൂർണ്ണസംഖ്യ* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = closure->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	അക്ഷരം* stackclone = alloca(cif->bytes);
	ശൂന്യം** avalue = alloca(cif->nargs * വലുപ്പം(ശൂന്യം*));
	ശൂന്യം* struct_rvalue = ശൂന്യ;
	അക്ഷരം* ptr = stackclone;
	പൂർണ്ണസംഖ്യ i;

	/* copy registers into stack clone */
	പൂർണ്ണസംഖ്യ registers_used = cif->bytes;
	എങ്കിൽ (registers_used > ARGS_REGISTER_SIZE) {
		registers_used = ARGS_REGISTER_SIZE;
	}
	memcpy(stackclone, register_args, registers_used);

	/* copy stack allocated args into stack clone */
	എങ്കിൽ (cif->bytes > ARGS_REGISTER_SIZE) {
		പൂർണ്ണസംഖ്യ അട്ടി_used = cif->bytes - ARGS_REGISTER_SIZE;
		memcpy(stackclone + ARGS_REGISTER_SIZE, അട്ടി_args, അട്ടി_used);
	}

	/* preserve struct type return pointer passing */
	എങ്കിൽ ((cif->rtype != ശൂന്യ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((ശൂന്യം**)ptr);
		ptr += WORD_SIZE;
	}

	/* populate arg pointer list */
	വേണ്ടി (i = 0; i < cif->nargs; i++)
	{
		തിരഞ്ഞെടുപ്പ് (arg_types[i]->type)
		{
			അവസ്ഥ FFI_TYPE_SINT8:
			അവസ്ഥ FFI_TYPE_UINT8:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#അല്ലെങ്കിൽ
				avalue[i] = ptr;
#endif
				നിർത്തുക;
			അവസ്ഥ FFI_TYPE_SINT16:
			അവസ്ഥ FFI_TYPE_UINT16:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#അല്ലെങ്കിൽ
				avalue[i] = ptr;
#endif
				നിർത്തുക;
			അവസ്ഥ FFI_TYPE_STRUCT:
#എങ്കിൽ __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				എങ്കിൽ (arg_types[i]->size < WORD_SIZE)
				{
				  memcpy (ptr, ptr + (WORD_SIZE - arg_types[i]->size), arg_types[i]->size);
				}
#endif
				avalue[i] = (ശൂന്യം*)ptr;
				നിർത്തുക;
			അവസ്ഥ FFI_TYPE_UINT64:
			അവസ്ഥ FFI_TYPE_SINT64:
			അവസ്ഥ FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				നിർത്തുക;
			അവസ്ഥ FFI_TYPE_SINT32:
			അവസ്ഥ FFI_TYPE_UINT32:
			അവസ്ഥ FFI_TYPE_FLOAT:
			സ്ഥിരസ്ഥിതി:
				/* default 4-byte argument */
				avalue[i] = ptr;
				നിർത്തുക;
		}
		ptr += WORD_FFI_ALIGN(arg_types[i]->size);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->size;
	*rtype = cif->rtype->type;
	എങ്കിൽ (struct_rvalue != ശൂന്യ) {
		closure->fun(cif, struct_rvalue, avalue, closure->user_data);
		/* copy struct return pointer value into function return value */
		*((ശൂന്യം**)rvalue) = struct_rvalue;
	} അല്ലെങ്കിൽ {
		closure->fun(cif, rvalue, avalue, closure->user_data);
	}
}

ffi_status ffi_prep_closure_loc(
		ffi_closure* closure, ffi_cif* cif,
		ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
		ശൂന്യം* user_data, ശൂന്യം* codeloc)
{
	unsigned long* tramp = (unsigned long*)&(closure->tramp[0]);
	unsigned long cls = (unsigned long)codeloc;
	unsigned long fn = 0;
	unsigned long fn_closure_call_sysv = (unsigned long)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	തിരഞ്ഞെടുപ്പ് (cif->abi)
	{
	അവസ്ഥ FFI_SYSV:
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

		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		തിരികെ FFI_BAD_ABI;
	}
	തിരികെ FFI_OK;
}
