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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

வெளி வெறுமை ffi_call_SYSV(வெறுமை (*)(வெறுமை*, extended_cif*), extended_cif*,
		unsigned முழு, unsigned முழு, unsigned முழு*, வெறுமை (*fn)(வெறுமை),
		unsigned முழு, unsigned முழு);

வெளி வெறுமை ffi_closure_SYSV(வெறுமை);

#define WORD_SIZE			அளவு(unsigned முழு)
#define ARGS_REGISTER_SIZE	(WORD_SIZE * 6)
#define WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_SIZE)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
வெறுமை ffi_prep_args(வெறுமை* அடுக்கு, extended_cif* ecif)
{
	unsigned முழு i;
	ffi_type** p_arg;
	வெறுமை** p_argv;
	வெறுமை* அடுக்கு_args_p = அடுக்கு;

	எனில் (ecif == வெற்று || ecif->cif == வெற்று) {
		திரும்பு; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	எனில் ((ecif->cif->rtype != வெற்று) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		எழுத்து* addr = அடுக்கு_args_p;
		memcpy(addr, &(ecif->rvalue), WORD_SIZE);
		அடுக்கு_args_p += WORD_SIZE;
	}

	எனில் (ecif->avalue == வெற்று) {
		திரும்பு; /* no arguments to prepare */
	}

	க்கு (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t size = (*p_arg)->size;
		முழு type = (*p_arg)->type;
		வெறுமை* மதிப்பு = p_argv[i];
		எழுத்து* addr = அடுக்கு_args_p;
		முழு aligned_size = WORD_FFI_ALIGN(size);

		/* force word alignment on the stack */
		அடுக்கு_args_p += aligned_size;
		
		தேர்வு (type)
		{
			நிலை FFI_TYPE_UINT8:
				*(unsigned முழு *)addr = (unsigned முழு)*(UINT8*)(மதிப்பு);
				நிறுத்து;
			நிலை FFI_TYPE_SINT8:
				*(signed முழு *)addr = (signed முழு)*(SINT8*)(மதிப்பு);
				நிறுத்து;
			நிலை FFI_TYPE_UINT16:
				*(unsigned முழு *)addr = (unsigned முழு)*(UINT16*)(மதிப்பு);
				நிறுத்து;
			நிலை FFI_TYPE_SINT16:
				*(signed முழு *)addr = (signed முழு)*(SINT16*)(மதிப்பு);
				நிறுத்து;
			நிலை FFI_TYPE_STRUCT:
#எனில் __BIG_ENDIAN__
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
				எனில் (size < WORD_SIZE)
				{
				  memcpy (addr + (WORD_SIZE - size), மதிப்பு, size);
				  நிறுத்து;
				}
#endif
			நிலை FFI_TYPE_SINT32:
			நிலை FFI_TYPE_UINT32:
			நிலை FFI_TYPE_FLOAT:
			நிலை FFI_TYPE_SINT64:
			நிலை FFI_TYPE_UINT64:
			நிலை FFI_TYPE_DOUBLE:
			இயல்பு:
				memcpy(addr, மதிப்பு, aligned_size);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	தேர்வு (cif->abi)
	{
		நிலை FFI_SYSV:
			நிறுத்து;
		இயல்பு:
			திரும்பு FFI_BAD_ABI;
	}
	திரும்பு FFI_OK;
}

வெறுமை ffi_call(ffi_cif* cif, வெறுமை (*fn)(வெறுமை), வெறுமை* rvalue, வெறுமை** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	எனில் ((rvalue == வெற்று) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->size);
	} இல்லையெனில் {
		ecif.rvalue = rvalue;
	}

	தேர்வு (cif->abi)
	{
	நிலை FFI_SYSV:
		ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->size);
		நிறுத்து;
	இயல்பு:
		FFI_ASSERT(0);
		நிறுத்து;
	}
}

வெறுமை ffi_closure_call_SYSV(வெறுமை* register_args, வெறுமை* அடுக்கு_args,
			ffi_closure* closure, வெறுமை* rvalue,
			unsigned முழு* rtype, unsigned முழு* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = closure->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	எழுத்து* stackclone = alloca(cif->bytes);
	வெறுமை** avalue = alloca(cif->nargs * அளவு(வெறுமை*));
	வெறுமை* struct_rvalue = வெற்று;
	எழுத்து* ptr = stackclone;
	முழு i;

	/* copy registers into stack clone */
	முழு registers_used = cif->bytes;
	எனில் (registers_used > ARGS_REGISTER_SIZE) {
		registers_used = ARGS_REGISTER_SIZE;
	}
	memcpy(stackclone, register_args, registers_used);

	/* copy stack allocated args into stack clone */
	எனில் (cif->bytes > ARGS_REGISTER_SIZE) {
		முழு அடுக்கு_used = cif->bytes - ARGS_REGISTER_SIZE;
		memcpy(stackclone + ARGS_REGISTER_SIZE, அடுக்கு_args, அடுக்கு_used);
	}

	/* preserve struct type return pointer passing */
	எனில் ((cif->rtype != வெற்று) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((வெறுமை**)ptr);
		ptr += WORD_SIZE;
	}

	/* populate arg pointer list */
	க்கு (i = 0; i < cif->nargs; i++)
	{
		தேர்வு (arg_types[i]->type)
		{
			நிலை FFI_TYPE_SINT8:
			நிலை FFI_TYPE_UINT8:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#இல்லையெனில்
				avalue[i] = ptr;
#endif
				நிறுத்து;
			நிலை FFI_TYPE_SINT16:
			நிலை FFI_TYPE_UINT16:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#இல்லையெனில்
				avalue[i] = ptr;
#endif
				நிறுத்து;
			நிலை FFI_TYPE_STRUCT:
#எனில் __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				எனில் (arg_types[i]->size < WORD_SIZE)
				{
				  memcpy (ptr, ptr + (WORD_SIZE - arg_types[i]->size), arg_types[i]->size);
				}
#endif
				avalue[i] = (வெறுமை*)ptr;
				நிறுத்து;
			நிலை FFI_TYPE_UINT64:
			நிலை FFI_TYPE_SINT64:
			நிலை FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				நிறுத்து;
			நிலை FFI_TYPE_SINT32:
			நிலை FFI_TYPE_UINT32:
			நிலை FFI_TYPE_FLOAT:
			இயல்பு:
				/* default 4-byte argument */
				avalue[i] = ptr;
				நிறுத்து;
		}
		ptr += WORD_FFI_ALIGN(arg_types[i]->size);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->size;
	*rtype = cif->rtype->type;
	எனில் (struct_rvalue != வெற்று) {
		closure->fun(cif, struct_rvalue, avalue, closure->user_data);
		/* copy struct return pointer value into function return value */
		*((வெறுமை**)rvalue) = struct_rvalue;
	} இல்லையெனில் {
		closure->fun(cif, rvalue, avalue, closure->user_data);
	}
}

ffi_status ffi_prep_closure_loc(
		ffi_closure* closure, ffi_cif* cif,
		வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		வெறுமை* user_data, வெறுமை* codeloc)
{
	unsigned long* tramp = (unsigned long*)&(closure->tramp[0]);
	unsigned long cls = (unsigned long)codeloc;
	unsigned long fn = 0;
	unsigned long fn_closure_call_sysv = (unsigned long)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	தேர்வு (cif->abi)
	{
	நிலை FFI_SYSV:
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

		நிறுத்து;
	இயல்பு:
		திரும்பு FFI_BAD_ABI;
	}
	திரும்பு FFI_OK;
}
