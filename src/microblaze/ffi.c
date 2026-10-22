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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

બાહ્ય શૂન્ય ffi_call_SYSV(શૂન્ય (*)(શૂન્ય*, extended_cif*), extended_cif*,
		unsigned પૂર્ણાંક, unsigned પૂર્ણાંક, unsigned પૂર્ણાંક*, શૂન્ય (*fn)(શૂન્ય),
		unsigned પૂર્ણાંક, unsigned પૂર્ણાંક);

બાહ્ય શૂન્ય ffi_closure_SYSV(શૂન્ય);

#define WORD_SIZE			કદ(unsigned પૂર્ણાંક)
#define ARGS_REGISTER_SIZE	(WORD_SIZE * 6)
#define WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_SIZE)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
શૂન્ય ffi_prep_args(શૂન્ય* ઢગલો, extended_cif* ecif)
{
	unsigned પૂર્ણાંક i;
	ffi_type** p_arg;
	શૂન્ય** p_argv;
	શૂન્ય* ઢગલો_args_p = ઢગલો;

	જો (ecif == ખાલી || ecif->cif == ખાલી) {
		પાછા; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	જો ((ecif->cif->rtype != ખાલી) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		અક્ષર* addr = ઢગલો_args_p;
		memcpy(addr, &(ecif->rvalue), WORD_SIZE);
		ઢગલો_args_p += WORD_SIZE;
	}

	જો (ecif->avalue == ખાલી) {
		પાછા; /* no arguments to prepare */
	}

	માટે (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t size = (*p_arg)->size;
		પૂર્ણાંક type = (*p_arg)->type;
		શૂન્ય* કિંમત = p_argv[i];
		અક્ષર* addr = ઢગલો_args_p;
		પૂર્ણાંક aligned_size = WORD_FFI_ALIGN(size);

		/* force word alignment on the stack */
		ઢગલો_args_p += aligned_size;
		
		પસંદગી (type)
		{
			સ્થિતિ FFI_TYPE_UINT8:
				*(unsigned પૂર્ણાંક *)addr = (unsigned પૂર્ણાંક)*(UINT8*)(કિંમત);
				તોડો;
			સ્થિતિ FFI_TYPE_SINT8:
				*(signed પૂર્ણાંક *)addr = (signed પૂર્ણાંક)*(SINT8*)(કિંમત);
				તોડો;
			સ્થિતિ FFI_TYPE_UINT16:
				*(unsigned પૂર્ણાંક *)addr = (unsigned પૂર્ણાંક)*(UINT16*)(કિંમત);
				તોડો;
			સ્થિતિ FFI_TYPE_SINT16:
				*(signed પૂર્ણાંક *)addr = (signed પૂર્ણાંક)*(SINT16*)(કિંમત);
				તોડો;
			સ્થિતિ FFI_TYPE_STRUCT:
#જો __BIG_ENDIAN__
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
				જો (size < WORD_SIZE)
				{
				  memcpy (addr + (WORD_SIZE - size), કિંમત, size);
				  તોડો;
				}
#endif
			સ્થિતિ FFI_TYPE_SINT32:
			સ્થિતિ FFI_TYPE_UINT32:
			સ્થિતિ FFI_TYPE_FLOAT:
			સ્થિતિ FFI_TYPE_SINT64:
			સ્થિતિ FFI_TYPE_UINT64:
			સ્થિતિ FFI_TYPE_DOUBLE:
			મૂળભૂત:
				memcpy(addr, કિંમત, aligned_size);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	પસંદગી (cif->abi)
	{
		સ્થિતિ FFI_SYSV:
			તોડો;
		મૂળભૂત:
			પાછા FFI_BAD_ABI;
	}
	પાછા FFI_OK;
}

શૂન્ય ffi_call(ffi_cif* cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય* rvalue, શૂન્ય** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	જો ((rvalue == ખાલી) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->size);
	} નહીંતર {
		ecif.rvalue = rvalue;
	}

	પસંદગી (cif->abi)
	{
	સ્થિતિ FFI_SYSV:
		ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->size);
		તોડો;
	મૂળભૂત:
		FFI_ASSERT(0);
		તોડો;
	}
}

શૂન્ય ffi_closure_call_SYSV(શૂન્ય* register_args, શૂન્ય* ઢગલો_args,
			ffi_closure* closure, શૂન્ય* rvalue,
			unsigned પૂર્ણાંક* rtype, unsigned પૂર્ણાંક* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = closure->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	અક્ષર* stackclone = alloca(cif->bytes);
	શૂન્ય** avalue = alloca(cif->nargs * કદ(શૂન્ય*));
	શૂન્ય* struct_rvalue = ખાલી;
	અક્ષર* ptr = stackclone;
	પૂર્ણાંક i;

	/* copy registers into stack clone */
	પૂર્ણાંક registers_used = cif->bytes;
	જો (registers_used > ARGS_REGISTER_SIZE) {
		registers_used = ARGS_REGISTER_SIZE;
	}
	memcpy(stackclone, register_args, registers_used);

	/* copy stack allocated args into stack clone */
	જો (cif->bytes > ARGS_REGISTER_SIZE) {
		પૂર્ણાંક ઢગલો_used = cif->bytes - ARGS_REGISTER_SIZE;
		memcpy(stackclone + ARGS_REGISTER_SIZE, ઢગલો_args, ઢગલો_used);
	}

	/* preserve struct type return pointer passing */
	જો ((cif->rtype != ખાલી) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((શૂન્ય**)ptr);
		ptr += WORD_SIZE;
	}

	/* populate arg pointer list */
	માટે (i = 0; i < cif->nargs; i++)
	{
		પસંદગી (arg_types[i]->type)
		{
			સ્થિતિ FFI_TYPE_SINT8:
			સ્થિતિ FFI_TYPE_UINT8:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#નહીંતર
				avalue[i] = ptr;
#endif
				તોડો;
			સ્થિતિ FFI_TYPE_SINT16:
			સ્થિતિ FFI_TYPE_UINT16:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#નહીંતર
				avalue[i] = ptr;
#endif
				તોડો;
			સ્થિતિ FFI_TYPE_STRUCT:
#જો __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				જો (arg_types[i]->size < WORD_SIZE)
				{
				  memcpy (ptr, ptr + (WORD_SIZE - arg_types[i]->size), arg_types[i]->size);
				}
#endif
				avalue[i] = (શૂન્ય*)ptr;
				તોડો;
			સ્થિતિ FFI_TYPE_UINT64:
			સ્થિતિ FFI_TYPE_SINT64:
			સ્થિતિ FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				તોડો;
			સ્થિતિ FFI_TYPE_SINT32:
			સ્થિતિ FFI_TYPE_UINT32:
			સ્થિતિ FFI_TYPE_FLOAT:
			મૂળભૂત:
				/* default 4-byte argument */
				avalue[i] = ptr;
				તોડો;
		}
		ptr += WORD_FFI_ALIGN(arg_types[i]->size);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->size;
	*rtype = cif->rtype->type;
	જો (struct_rvalue != ખાલી) {
		closure->fun(cif, struct_rvalue, avalue, closure->user_data);
		/* copy struct return pointer value into function return value */
		*((શૂન્ય**)rvalue) = struct_rvalue;
	} નહીંતર {
		closure->fun(cif, rvalue, avalue, closure->user_data);
	}
}

ffi_status ffi_prep_closure_loc(
		ffi_closure* closure, ffi_cif* cif,
		શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		શૂન્ય* user_data, શૂન્ય* codeloc)
{
	unsigned long* tramp = (unsigned long*)&(closure->tramp[0]);
	unsigned long cls = (unsigned long)codeloc;
	unsigned long fn = 0;
	unsigned long fn_closure_call_sysv = (unsigned long)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	પસંદગી (cif->abi)
	{
	સ્થિતિ FFI_SYSV:
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

		તોડો;
	મૂળભૂત:
		પાછા FFI_BAD_ABI;
	}
	પાછા FFI_OK;
}
