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
		unsigned صحيح, unsigned صحيح, unsigned صحيح*, فراغ (*fn)(فراغ),
		unsigned صحيح, unsigned صحيح);

خارجي فراغ ffi_closure_SYSV(فراغ);

#define WORD_SIZE			حجم(unsigned صحيح)
#define وسائط_REGISTER_SIZE	(WORD_SIZE * 6)
#define WORD_FFI_ALIGN(x)		FFI_ALIGN(x, WORD_SIZE)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
فراغ ffi_prep_وسائط(فراغ* مكدس, extended_cif* ecif)
{
	unsigned صحيح i;
	ffi_type** p_arg;
	فراغ** p_argv;
	فراغ* مكدس_وسائط_p = مكدس;

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
		حرف* addr = مكدس_وسائط_p;
		memcpy(addr, &(ecif->rvalue), WORD_SIZE);
		مكدس_وسائط_p += WORD_SIZE;
	}

	إذا (ecif->avalue == عدم) {
		أرجع; /* no arguments to prepare */
	}

	لكل (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t size = (*p_arg)->size;
		صحيح type = (*p_arg)->type;
		فراغ* قيمة = p_argv[i];
		حرف* addr = مكدس_وسائط_p;
		صحيح aligned_size = WORD_FFI_ALIGN(size);

		/* force word alignment on the stack */
		مكدس_وسائط_p += aligned_size;
		
		بدل (type)
		{
			حالة FFI_TYPE_UINT8:
				*(unsigned صحيح *)addr = (unsigned صحيح)*(UINT8*)(قيمة);
				اكسر;
			حالة FFI_TYPE_SINT8:
				*(signed صحيح *)addr = (signed صحيح)*(SINT8*)(قيمة);
				اكسر;
			حالة FFI_TYPE_UINT16:
				*(unsigned صحيح *)addr = (unsigned صحيح)*(UINT16*)(قيمة);
				اكسر;
			حالة FFI_TYPE_SINT16:
				*(signed صحيح *)addr = (signed صحيح)*(SINT16*)(قيمة);
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
				إذا (size < WORD_SIZE)
				{
				  memcpy (addr + (WORD_SIZE - size), قيمة, size);
				  اكسر;
				}
#endif
			حالة FFI_TYPE_SINT32:
			حالة FFI_TYPE_UINT32:
			حالة FFI_TYPE_FLOAT:
			حالة FFI_TYPE_SINT64:
			حالة FFI_TYPE_UINT64:
			حالة FFI_TYPE_DOUBLE:
			افتراضي:
				memcpy(addr, قيمة, aligned_size);
		}
	}
}

ffi_status ffi_prep_cif_machdep(ffi_cif* cif)
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
		ecif.rvalue = alloca(cif->rtype->size);
	} وإلا {
		ecif.rvalue = rvalue;
	}

	بدل (cif->abi)
	{
	حالة FFI_SYSV:
		ffi_call_SYSV(ffi_prep_وسائط, &ecif, cif->bytes, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->size);
		اكسر;
	افتراضي:
		FFI_ASSERT(0);
		اكسر;
	}
}

فراغ ffi_closure_call_SYSV(فراغ* register_وسائط, فراغ* مكدس_وسائط,
			ffi_closure* closure, فراغ* rvalue,
			unsigned صحيح* rtype, unsigned صحيح* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = closure->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	حرف* stackclone = alloca(cif->bytes);
	فراغ** avalue = alloca(cif->nargs * حجم(فراغ*));
	فراغ* struct_rvalue = عدم;
	حرف* ptr = stackclone;
	صحيح i;

	/* copy registers into stack clone */
	صحيح registers_used = cif->bytes;
	إذا (registers_used > وسائط_REGISTER_SIZE) {
		registers_used = وسائط_REGISTER_SIZE;
	}
	memcpy(stackclone, register_وسائط, registers_used);

	/* copy stack allocated args into stack clone */
	إذا (cif->bytes > وسائط_REGISTER_SIZE) {
		صحيح مكدس_used = cif->bytes - وسائط_REGISTER_SIZE;
		memcpy(stackclone + وسائط_REGISTER_SIZE, مكدس_وسائط, مكدس_used);
	}

	/* preserve struct type return pointer passing */
	إذا ((cif->rtype != عدم) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((فراغ**)ptr);
		ptr += WORD_SIZE;
	}

	/* populate arg pointer list */
	لكل (i = 0; i < cif->nargs; i++)
	{
		بدل (arg_types[i]->type)
		{
			حالة FFI_TYPE_SINT8:
			حالة FFI_TYPE_UINT8:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#وإلا
				avalue[i] = ptr;
#endif
				اكسر;
			حالة FFI_TYPE_SINT16:
			حالة FFI_TYPE_UINT16:
#ifdef __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#وإلا
				avalue[i] = ptr;
#endif
				اكسر;
			حالة FFI_TYPE_STRUCT:
#إذا __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				إذا (arg_types[i]->size < WORD_SIZE)
				{
				  memcpy (ptr, ptr + (WORD_SIZE - arg_types[i]->size), arg_types[i]->size);
				}
#endif
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
		ptr += WORD_FFI_ALIGN(arg_types[i]->size);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->size;
	*rtype = cif->rtype->type;
	إذا (struct_rvalue != عدم) {
		closure->fun(cif, struct_rvalue, avalue, closure->user_data);
		/* copy struct return pointer value into function return value */
		*((فراغ**)rvalue) = struct_rvalue;
	} وإلا {
		closure->fun(cif, rvalue, avalue, closure->user_data);
	}
}

ffi_status ffi_prep_closure_loc(
		ffi_closure* closure, ffi_cif* cif,
		فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		فراغ* user_data, فراغ* codeloc)
{
	unsigned long* tramp = (unsigned long*)&(closure->tramp[0]);
	unsigned long cls = (unsigned long)codeloc;
	unsigned long fn = 0;
	unsigned long fn_closure_call_sysv = (unsigned long)ffi_closure_call_SYSV;

	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	بدل (cif->abi)
	{
	حالة FFI_SYSV:
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

		اكسر;
	افتراضي:
		أرجع FFI_BAD_ABI;
	}
	أرجع FFI_OK;
}
