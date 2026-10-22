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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

బాహ్య శూన్య ffi_call_SYSV(శూన్య (*)(శూన్య*, extended_cif*), extended_cif*,
		గుర్తులేని పూర్ణాంకం, గుర్తులేని పూర్ణాంకం, గుర్తులేని పూర్ణాంకం*, శూన్య (*fn)(శూన్య),
		గుర్తులేని పూర్ణాంకం, గుర్తులేని పూర్ణాంకం);

బాహ్య శూన్య ffi_సంవృతం_SYSV(శూన్య);

#నిర్వచించు మాట_కొలత			పరిమాణం(గుర్తులేని పూర్ణాంకం)
#నిర్వచించు వాదనలు_రిజిస్టరు_కొలత	(మాట_కొలత * 6)
#నిర్వచించు మాట_FFI_ALIGN(x)		FFI_ALIGN(x, మాట_కొలత)

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */
శూన్య ffi_prep_వాదనలు(శూన్య* దొంతర, extended_cif* ecif)
{
	గుర్తులేని పూర్ణాంకం i;
	ffi_type** p_arg;
	శూన్య** p_argv;
	శూన్య* దొంతర_వాదనలు_p = దొంతర;

	ఒకవేళ (ecif == ఖాళీ || ecif->cif == ఖాళీ) {
		తిరిగి; /* no description to prepare */
	}

	p_argv = ecif->avalue;

	ఒకవేళ ((ecif->cif->rtype != ఖాళీ) &&
			(ecif->cif->rtype->type == FFI_TYPE_STRUCT))
	{
		/* if return type is a struct which is referenced on the stack/reg5,
		 * by a pointer. Stored the return value pointer in r5.
		 */
		అక్షరం* addr = దొంతర_వాదనలు_p;
		జ్ఞాపకం_నకలు(addr, &(ecif->rvalue), మాట_కొలత);
		దొంతర_వాదనలు_p += మాట_కొలత;
	}

	ఒకవేళ (ecif->avalue == ఖాళీ) {
		తిరిగి; /* no arguments to prepare */
	}

	కొరకు (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
			i++, p_arg++)
	{
		size_t కొలత = (*p_arg)->కొలత;
		పూర్ణాంకం type = (*p_arg)->type;
		శూన్య* విలువ = p_argv[i];
		అక్షరం* addr = దొంతర_వాదనలు_p;
		పూర్ణాంకం aligned_కొలత = మాట_FFI_ALIGN(కొలత);

		/* force word alignment on the stack */
		దొంతర_వాదనలు_p += aligned_కొలత;
		
		ఎంపిక (type)
		{
			స్థితి FFI_TYPE_UINT8:
				*(గుర్తులేని పూర్ణాంకం *)addr = (గుర్తులేని పూర్ణాంకం)*(UINT8*)(విలువ);
				ఆపు;
			స్థితి FFI_TYPE_SINT8:
				*(గుర్తుగల పూర్ణాంకం *)addr = (గుర్తుగల పూర్ణాంకం)*(SINT8*)(విలువ);
				ఆపు;
			స్థితి FFI_TYPE_UINT16:
				*(గుర్తులేని పూర్ణాంకం *)addr = (గుర్తులేని పూర్ణాంకం)*(UINT16*)(విలువ);
				ఆపు;
			స్థితి FFI_TYPE_SINT16:
				*(గుర్తుగల పూర్ణాంకం *)addr = (గుర్తుగల పూర్ణాంకం)*(SINT16*)(విలువ);
				ఆపు;
			స్థితి FFI_TYPE_STRUCT:
#ఒకవేళ __BIG_ENDIAN__
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
				ఒకవేళ (కొలత < మాట_కొలత)
				{
				  జ్ఞాపకం_నకలు (addr + (మాట_కొలత - కొలత), విలువ, కొలత);
				  ఆపు;
				}
#అయితే_ముగింపు
			స్థితి FFI_TYPE_SINT32:
			స్థితి FFI_TYPE_UINT32:
			స్థితి FFI_TYPE_FLOAT:
			స్థితి FFI_TYPE_SINT64:
			స్థితి FFI_TYPE_UINT64:
			స్థితి FFI_TYPE_DOUBLE:
			అప్రమేయ:
				జ్ఞాపకం_నకలు(addr, విలువ, aligned_కొలత);
		}
	}
}

ffi_స్థితి ffi_prep_cif_machdep(ffi_cif* cif)
{
	/* check ABI */
	ఎంపిక (cif->abi)
	{
		స్థితి FFI_SYSV:
			ఆపు;
		అప్రమేయ:
			తిరిగి FFI_BAD_ABI;
	}
	తిరిగి FFI_OK;
}

శూన్య ffi_call(ffi_cif* cif, శూన్య (*fn)(శూన్య), శూన్య* rvalue, శూన్య** avalue)
{
	extended_cif ecif;
	ecif.cif = cif;
	ecif.avalue = avalue;

	/* If the return value is a struct and we don't have a return */
	/* value address then we need to make one */
	ఒకవేళ ((rvalue == ఖాళీ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		ecif.rvalue = alloca(cif->rtype->కొలత);
	} లేకపోతే {
		ecif.rvalue = rvalue;
	}

	ఎంపిక (cif->abi)
	{
	స్థితి FFI_SYSV:
		ffi_call_SYSV(ffi_prep_వాదనలు, &ecif, cif->బైట్లు, cif->flags,
				ecif.rvalue, fn, cif->rtype->type, cif->rtype->కొలత);
		ఆపు;
	అప్రమేయ:
		FFI_ASSERT(0);
		ఆపు;
	}
}

శూన్య ffi_సంవృతం_call_SYSV(శూన్య* రిజిస్టరు_వాదనలు, శూన్య* దొంతర_వాదనలు,
			ffi_సంవృతం* సంవృతం, శూన్య* rvalue,
			గుర్తులేని పూర్ణాంకం* rtype, గుర్తులేని పూర్ణాంకం* rsize)
{
	/* prepare arguments for closure call */
	ffi_cif* cif = సంవృతం->cif;
	ffi_type** arg_types = cif->arg_types;

	/* re-allocate data for the args. This needs to be done in order to keep
	 * multi-word objects (e.g. structs) in contiguous memory. Callers are not
	 * required to store the value of args in the lower 6 words in the stack
	 * (although they are allocated in the stack).
	 */
	అక్షరం* stackclone = alloca(cif->బైట్లు);
	శూన్య** avalue = alloca(cif->nargs * పరిమాణం(శూన్య*));
	శూన్య* struct_rvalue = ఖాళీ;
	అక్షరం* ptr = stackclone;
	పూర్ణాంకం i;

	/* copy registers into stack clone */
	పూర్ణాంకం registers_వాడిన = cif->బైట్లు;
	ఒకవేళ (registers_వాడిన > వాదనలు_రిజిస్టరు_కొలత) {
		registers_వాడిన = వాదనలు_రిజిస్టరు_కొలత;
	}
	జ్ఞాపకం_నకలు(stackclone, రిజిస్టరు_వాదనలు, registers_వాడిన);

	/* copy stack allocated args into stack clone */
	ఒకవేళ (cif->బైట్లు > వాదనలు_రిజిస్టరు_కొలత) {
		పూర్ణాంకం దొంతర_వాడిన = cif->బైట్లు - వాదనలు_రిజిస్టరు_కొలత;
		జ్ఞాపకం_నకలు(stackclone + వాదనలు_రిజిస్టరు_కొలత, దొంతర_వాదనలు, దొంతర_వాడిన);
	}

	/* preserve struct type return pointer passing */
	ఒకవేళ ((cif->rtype != ఖాళీ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
		struct_rvalue = *((శూన్య**)ptr);
		ptr += మాట_కొలత;
	}

	/* populate arg pointer list */
	కొరకు (i = 0; i < cif->nargs; i++)
	{
		ఎంపిక (arg_types[i]->type)
		{
			స్థితి FFI_TYPE_SINT8:
			స్థితి FFI_TYPE_UINT8:
#నిర్వచితమైతే __BIG_ENDIAN__
				avalue[i] = ptr + 3;
#లేకపోతే
				avalue[i] = ptr;
#అయితే_ముగింపు
				ఆపు;
			స్థితి FFI_TYPE_SINT16:
			స్థితి FFI_TYPE_UINT16:
#నిర్వచితమైతే __BIG_ENDIAN__
				avalue[i] = ptr + 2;
#లేకపోతే
				avalue[i] = ptr;
#అయితే_ముగింపు
				ఆపు;
			స్థితి FFI_TYPE_STRUCT:
#ఒకవేళ __BIG_ENDIAN__
				/*
				 * Work around strange ABI behaviour.
				 * (see info in ffi_prep_args)
				 */
				ఒకవేళ (arg_types[i]->కొలత < మాట_కొలత)
				{
				  జ్ఞాపకం_నకలు (ptr, ptr + (మాట_కొలత - arg_types[i]->కొలత), arg_types[i]->కొలత);
				}
#అయితే_ముగింపు
				avalue[i] = (శూన్య*)ptr;
				ఆపు;
			స్థితి FFI_TYPE_UINT64:
			స్థితి FFI_TYPE_SINT64:
			స్థితి FFI_TYPE_DOUBLE:
				avalue[i] = ptr;
				ఆపు;
			స్థితి FFI_TYPE_SINT32:
			స్థితి FFI_TYPE_UINT32:
			స్థితి FFI_TYPE_FLOAT:
			అప్రమేయ:
				/* default 4-byte argument */
				avalue[i] = ptr;
				ఆపు;
		}
		ptr += మాట_FFI_ALIGN(arg_types[i]->కొలత);
	}

	/* set the return type info passed back to the wrapper */
	*rsize = cif->rtype->కొలత;
	*rtype = cif->rtype->type;
	ఒకవేళ (struct_rvalue != ఖాళీ) {
		సంవృతం->fun(cif, struct_rvalue, avalue, సంవృతం->user_దత్తాంశం);
		/* copy struct return pointer value into function return value */
		*((శూన్య**)rvalue) = struct_rvalue;
	} లేకపోతే {
		సంవృతం->fun(cif, rvalue, avalue, సంవృతం->user_దత్తాంశం);
	}
}

ffi_స్థితి ffi_prep_సంవృతం_loc(
		ffi_సంవృతం* సంవృతం, ffi_cif* cif,
		శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		శూన్య* user_దత్తాంశం, శూన్య* codeloc)
{
	గుర్తులేని దీర్ఘ* tramp = (గుర్తులేని దీర్ఘ*)&(సంవృతం->tramp[0]);
	గుర్తులేని దీర్ఘ cls = (గుర్తులేని దీర్ఘ)codeloc;
	గుర్తులేని దీర్ఘ fn = 0;
	గుర్తులేని దీర్ఘ fn_సంవృతం_call_sysv = (గుర్తులేని దీర్ఘ)ffi_సంవృతం_call_SYSV;

	సంవృతం->cif = cif;
	సంవృతం->fun = fun;
	సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

	ఎంపిక (cif->abi)
	{
	స్థితి FFI_SYSV:
		fn = (గుర్తులేని దీర్ఘ)ffi_సంవృతం_SYSV;

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
		tramp[4] = 0xb0000000 | ((fn_సంవృతం_call_sysv >> 16) & 0xffff);
		/* addik r3, r0, fn_closure_call_sysv(lower) */
		tramp[5] = 0x30600000 | (fn_సంవృతం_call_sysv & 0xffff);
		/* branch/jump to address stored in r11 (fn) */
		tramp[6] = 0x98085800; /* bra r11 */

		ఆపు;
	అప్రమేయ:
		తిరిగి FFI_BAD_ABI;
	}
	తిరిగి FFI_OK;
}
