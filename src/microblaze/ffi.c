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

بیرونی تهی ffi_call_SYSV(تهی (*)(تهی*, extended_cif*), extended_cif*,
		unsigned صحیح, unsigned صحیح, unsigned صحیح*, تهی (*fn)(تهی),
		unsigned صحیح, unsigned صحیح);

بیرونی تهی ffi_closure_SYSV(تهی);

#define WORD_SIZE			اندازه(unsigned صحیح)
#define ARGS_REGISTER_SIZE	(WORD_SIZE * 6)
#define WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_SIZE)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
تهی ffi_prep_args(تهی* پشته, extended_cif* ecif)
{
	unsigned صحیح i;
	ffi_type** p_arg;
	تهی** p_argv;
	تهی* پشته_args_p = پشته;

	اگر (ecif == پوچ || ecif->cif == پوچ) {
		بازگشت; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	اگر ((ecif->cif->rtype != پوچ) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		نویسه* addr = پشته_args_p;
		memcpy(addr, &(ecif->rvalue), WORD_SIZE);
		پشته_args_p += WORD_SIZE;
	}

	اگر (ecif->avalue == پوچ) {
		بازگشت; /* no arguments to prepare */
	}

	برای (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t size = (*p_arg)->size;
		صحیح type = (*p_arg)->type;
		تهی* مقدار = p_argv[i];
		نویسه* addr = پشته_args_p;
		صحیح aligned_size = WORD_FFI_ALIGN(size);

		/* force word alignment on the stack */
		پشته_args_p += aligned_size;
		
		انتخاب (type)
		{
			حالت FFI_TYPE_UINT8:
				*(unsigned صحیح *)addr = (unsigned صحیح)*(UINT8*)(مقدار);
				بشکن;
			حالت FFI_TYPE_SINT8:
				*(signed صحیح *)addr = (signed صحیح)*(SINT8*)(مقدار);
				بشکن;
			حالت FFI_TYPE_UINT16:
				*(unsigned صحیح *)addr = (unsigned صحیح)*(UINT16*)(مقدار);
				بشکن;
			حالت FFI_TYPE_SINT16:
				*(signed صحیح *)addr = (signed صحیح)*(SINT16*)(مقدار);
				بشکن;
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
				اگر (size < WORD_SIZE)
				{
				  memcpy (addr + (WORD_SIZE - size), مقدار, size);
				  بشکن;
				}
#endif
			حالت FFI_TYPE_SINT32:
			حالت FFI_TYPE_UINT32:
			حالت FFI_TYPE_FLOAT:
			حالت FFI_TYPE_SINT64:
			حالت FFI_TYPE_UINT64:
			حالت FFI_TYPE_DOUBLE:
			پیشفرض:
				memcpy(addr, مقدار, aligned_size);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	انتخاب (cif->abi)
	{
		حالت FFI_SYSV:
			بشکن;
		پیشفرض:
			بازگشت FFI_BAD_ABI;
	}
	بازگشت FFI_OK;
}

تهی ffi_call(ffi_cif* cif, تهی (*fn)(تهی), تهی* rvalue, تهی** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	اگر ((rvalue == پوچ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->size);
	} وگرنه {
		ecif.rvalue = rvalue;
	}

	انتخاب (cif->abi)
	{
	حالت FFI_SYSV:
		ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->size);
		بشکن;
	پیشفرض:
		FFI_ASSERT(0);
		بشکن;
	}
}

تهی ffi_closure_call_SYSV(تهی* register_args, تهی* پشته_args,
			ffi_closure* closure, تهی* rvalue,
			unsigned صحیح* rtype, unsigned صحیح* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = closure->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	نویسه* stackclone = alloca(cif->bytes);
	تهی** avalue = alloca(cif->nargs * اندازه(تهی*));
	تهی* struct_rvalue = پوچ;
	نویسه* ptr = stackclone;
	صحیح i;

	/* copy registers into stack clone */
	صحیح registers_used = cif->bytes;
	اگر (registers_used > ARGS_REGISTER_SIZE) {
		registers_used = ARGS_REGISTER_SIZE;
	}
	memcpy(stackclone, register_args, registers_used);

	/* copy stack allocated args into stack clone */
	اگر (cif->bytes > ARGS_REGISTER_SIZE) {
		صحیح پشته_used = cif->bytes - ARGS_REGISTER_SIZE;
		memcpy(stackclone + ARGS_REGISTER_SIZE, پشته_args, پشته_used);
	}

	/* preserve struct type return pointer passing */
	اگر ((cif->rtype != پوچ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((تهی**)ptr);
		ptr += WORD_SIZE;
	}

	/* populate arg pointer list */
	برای (i = 0; i < cif->nargs; i++)
	{
		انتخاب (arg_types[i]->type)
		{
			حالت FFI_TYPE_SINT8:
			حالت FFI_TYPE_UINT8:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#وگرنه
				avalue[i] = ptr;
#endif
				بشکن;
			حالت FFI_TYPE_SINT16:
			حالت FFI_TYPE_UINT16:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#وگرنه
				avalue[i] = ptr;
#endif
				بشکن;
			حالت FFI_TYPE_STRUCT:
#اگر __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				اگر (arg_types[i]->size < WORD_SIZE)
				{
				  memcpy (ptr, ptr + (WORD_SIZE - arg_types[i]->size), arg_types[i]->size);
				}
#endif
				avalue[i] = (تهی*)ptr;
				بشکن;
			حالت FFI_TYPE_UINT64:
			حالت FFI_TYPE_SINT64:
			حالت FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				بشکن;
			حالت FFI_TYPE_SINT32:
			حالت FFI_TYPE_UINT32:
			حالت FFI_TYPE_FLOAT:
			پیشفرض:
				/* default 4-byte argument */
				avalue[i] = ptr;
				بشکن;
		}
		ptr += WORD_FFI_ALIGN(arg_types[i]->size);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->size;
	*rtype = cif->rtype->type;
	اگر (struct_rvalue != پوچ) {
		closure->fun(cif, struct_rvalue, avalue, closure->user_data);
		/* copy struct return pointer value into function return value */
		*((تهی**)rvalue) = struct_rvalue;
	} وگرنه {
		closure->fun(cif, rvalue, avalue, closure->user_data);
	}
}

ffi_status ffi_prep_closure_loc(
		ffi_closure* closure, ffi_cif* cif,
		تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
		تهی* user_data, تهی* codeloc)
{
	unsigned long* tramp = (unsigned long*)&(closure->tramp[0]);
	unsigned long cls = (unsigned long)codeloc;
	unsigned long fn = 0;
	unsigned long fn_closure_call_sysv = (unsigned long)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	انتخاب (cif->abi)
	{
	حالت FFI_SYSV:
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

		بشکن;
	پیشفرض:
		بازگشت FFI_BAD_ABI;
	}
	بازگشت FFI_OK;
}
