/* -----------------------------------------------------------------------
   ffi.c - (c) 2011, 2026 Anthony Green
           (c) 2008 Red Hat, Inc.
	   (c) 2006 Free Software Foundation, Inc.
           (c) 2003-2004 Randolph Chung <tausq@debian.org>
           
   HPPA Foreign Function Interface
   HP-UX PA ABI support 

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

#شامل <stdlib.h>
#شامل <stdio.h>

#تعریف_کرو باری_اوپر(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#تعریف_کرو MIN_ڈھیر_سائز  64
#تعریف_کرو پہلا_ARG_خانہ  9
#تعریف_کرو DEBUG_LEVEL   0

#تعریف_کرو fldw(addr, fpreg) \
  __asm__ متلون ("fldw 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#تعریف_کرو fstw(fpreg, addr) \
  __asm__ متلون ("fstw %%" #fpreg "L, 0(%0)" : : "r"(addr))
#تعریف_کرو fldd(addr, fpreg) \
  __asm__ متلون ("fldd 0(%0), %%" #fpreg : : "r"(addr) : #fpreg)
#تعریف_کرو fstd(fpreg, addr) \
  __asm__ متلون ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#تعریف_کرو debug(lvl, x...) کرو { اگر (lvl <= DEBUG_LEVEL) { لکھو(x); } } جب (0)

ساکن اندرونی صحیح ffi_struct_type(ffi_type *t)
{
  size_t sz = t->سائز;

  /* Small structure results are passed in registers,
     larger ones are passed by pointer.  Note that small
     structures differ from the corresponding integer
     types in that they have different alignment requirements.  */

  اگر (sz <= 8)
    واپس -sz;
  ورنہ
    واپس FFI_TYPE_STRUCT; /* else, we pass it by pointer.  */
}

/* PA has a downward growing stack, which looks like this:

انحراف
	[ Variable args ]
   SP = (4*(n+9))       arg word N
   ...
   SP-52                arg word 4
	[ Fixed args ]
   SP-48                arg word 3
   SP-44                arg word 2
   SP-40                arg word 1
   SP-36                arg word 0
	[ Frame marker ]
   ...
   SP-20                RP
   SP-4                 previous SP

   The first four argument words on the stack are reserved for use by
   the callee.  Instead, the general and floating registers replace
   the first four argument slots.  Non FP arguments are passed solely
   in the general registers.  FP arguments are passed in both general
   and floating registers when using libffi.

   Non-FP 32-bit args are passed in gr26, gr25, gr24 and gr23.
   Non-FP 64-bit args are passed in register pairs, starting
   on an odd numbered register (i.e. r25+r26 and r23+r24).
   FP 32-bit arguments are passed in fr4L, fr5L, fr6L and fr7L.
   FP 64-bit arguments are passed in fr5 and fr7.

   The registers are allocated in the same manner as stack slots.
   This allows the callee to save its arguments on the stack if
   necessary:

   arg word 3 -> gr23 or fr7L
   arg word 2 -> gr24 or fr6L or fr7R
   arg word 1 -> gr25 or fr5L
   arg word 0 -> gr26 or fr4L or fr5R

   Note that fr4R and fr6R are never used for arguments (i.e.,
   doubles are not passed in fr4 or fr6).

   The rest of the arguments are passed on the stack starting at SP-52,
   but 64-bit arguments need to be aligned to an 8-byte boundary

   This means we can have holes either in the register allocation,
   or in the stack.  */

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments

   The following code will put everything into the stack frame
   (which was allocated by the asm routine), and on return
   the asm routine will load the arguments that should be
   passed by register into the appropriate registers

   NOTE: We load floating point args in this function... that means we
   assume gcc will not mess with fp regs in here.  */

خالی ffi_prep_دلائل_pa32(UINT32 *ڈھیر, extended_cif *ecif, غیرعلامتی بائٹیں)
{
  اندراج غیرعلامتی صحیح i;
  اندراج ffi_type **p_arg;
  اندراج خالی **p_argv;
  غیرعلامتی صحیح خانہ = پہلا_ARG_خانہ;
  حرف *dest_cpy;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ڈھیر,
	ecif, بائٹیں);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  برائے (i = 0; i < ecif->cif->nargs; i++)
    {
      صحیح type = (*p_arg)->type;

      انتخاب (type)
	{
	حالت FFI_TYPE_SINT8:
	  *(SINT32 *)(ڈھیر - خانہ) = *(SINT8 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_UINT8:
	  *(UINT32 *)(ڈھیر - خانہ) = *(UINT8 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_SINT16:
	  *(SINT32 *)(ڈھیر - خانہ) = *(SINT16 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_UINT16:
	  *(UINT32 *)(ڈھیر - خانہ) = *(UINT16 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_اشارہ_گر:
	  debug(3, "Storing UINT32 %u in slot %u\n", *(UINT32 *)(*p_argv),
		خانہ);
	  *(UINT32 *)(ڈھیر - خانہ) = *(UINT32 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	  /* Align slot for 64-bit type.  */
	  خانہ += (خانہ & 1) ? 1 : 2;
	  *(UINT64 *)(ڈھیر - خانہ) = *(UINT64 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_FLOAT:
	  /* First 4 args go in fr4L - fr7L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", خانہ);
	  *(UINT32 *)(ڈھیر - خانہ) = *(UINT32 *)(*p_argv);
	  انتخاب (خانہ - پہلا_ARG_خانہ)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    حالت 0: fldw(ڈھیر - خانہ, fr4); توڑو;
	    حالت 1: fldw(ڈھیر - خانہ, fr5); توڑو;
	    حالت 2: fldw(ڈھیر - خانہ, fr6); توڑو;
	    حالت 3: fldw(ڈھیر - خانہ, fr7); توڑو;
	    }
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	  /* Align slot for 64-bit type.  */
	  خانہ += (خانہ & 1) ? 1 : 2;
	  debug(3, "Storing UINT64(double) at slot %u\n", خانہ);
	  *(UINT64 *)(ڈھیر - خانہ) = *(UINT64 *)(*p_argv);
	  انتخاب (خانہ - پہلا_ARG_خانہ)
	    {
	      /* First 2 args go in fr5, fr7.  */
	      حالت 1: fldd(ڈھیر - خانہ, fr5); توڑو;
	      حالت 3: fldd(ڈھیر - خانہ, fr7); توڑو;
	    }
	  توڑو;

#اگر_تعریف_شدہ PA_HPUX
	حالت FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are passed in the same manner as structures
	     larger than 8 bytes.  */
	  *(UINT32 *)(ڈھیر - خانہ) = (UINT32)(*p_argv);
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_STRUCT:

	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */

	  len = (*p_arg)->سائز;
	  اگر (len <= 4)
	    {
	      dest_cpy = (حرف *)(ڈھیر - خانہ) + 4 - len;
	      حافظہ_نقل(dest_cpy, (حرف *)*p_argv, len);
	    }
	  ورنہ اگر (len <= 8)
	    {
	      خانہ += (خانہ & 1) ? 1 : 2;
	      dest_cpy = (حرف *)(ڈھیر - خانہ) + 8 - len;
	      حافظہ_نقل(dest_cpy, (حرف *)*p_argv, len);
	    }
	  ورنہ
	    *(UINT32 *)(ڈھیر - خانہ) = (UINT32)(*p_argv);
	  توڑو;

	طےشدہ:
	  FFI_ASSERT(0);
	}

      خانہ++;
      p_arg++;
      p_argv++;
    }

  /* Make sure we didn't mess up and scribble on the stack.  */
  {
    غیرعلامتی صحیح n;

    debug(5, "Stack setup:\n");
    برائے (n = 0; n < (بائٹیں + 3) / 4; n++)
      {
	اگر ((n%4) == 0) { debug(5, "\n%08x: ", (غیرعلامتی صحیح)(ڈھیر - n)); }
	debug(5, "%08x ", *(ڈھیر - n));
      }
    debug(5, "\n");
  }

  FFI_ASSERT(خانہ * 4 <= بائٹیں);

  واپس;
}

ساکن خالی ffi_سائز_ڈھیر_pa32(ffi_cif *cif)
{
  ffi_type **ptr;
  صحیح i;
  صحیح z = 0; /* # stack slots */

  برائے (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      صحیح type = (*ptr)->type;

      انتخاب (type)
	{
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	  z += 2 + (z & 1); /* must start on even regs, so we may waste one */
	  توڑو;

#اگر_تعریف_شدہ PA_HPUX
	حالت FFI_TYPE_LONGDOUBLE:
	  z += 1; /* passed by pointer, like a large struct */
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_STRUCT:
	  /* This must mirror the slot accounting in ffi_prep_args_pa32:
	     structs of 1-4 bytes occupy one slot, structs of 5-8 bytes are
	     passed inline in two even-aligned slots (exactly like a 64-bit
	     value), and larger structs are passed by pointer in one slot.
	     z stays offset from the marshaller's slot by FIRST_ARG_SLOT (odd),
	     so (z & 1) tracks the same alignment the marshaller applies.  */
	  {
	    size_t len = (*ptr)->سائز;
	    اگر (len <= 4 || len > 8)
	      z += 1;
	    ورنہ
	      z += 2 + (z & 1);
	  }
	  توڑو;

	طےشدہ: /* <= 32-bit values */
	  z++;
	}
    }

  /* We can fit up to 6 args in the default 64-byte stack frame,
     if we need more, we need more stack.  */
  اگر (z <= 6)
    cif->بائٹیں = MIN_ڈھیر_سائز; /* min stack size */
  ورنہ
    cif->بائٹیں = 64 + باری_اوپر((z - 6) * حجم(UINT32), MIN_ڈھیر_سائز);

  debug(3, "Calculated stack size is %u bytes\n", cif->بائٹیں);
}

/* Perform machine dependent cif processing.  */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
      cif->flags = (غیرعلامتی) cif->rtype->type;
      توڑو;

#اگر_تعریف_شدہ PA_HPUX
    حالت FFI_TYPE_LONGDOUBLE:
      /* Long doubles are treated like a structure.  */
      cif->flags = FFI_TYPE_STRUCT;
      توڑو;
#اگر_ختم

    حالت FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 4 bytes, the result is given back
	 in one register. If the size is smaller or equal 8 bytes than we
	 return the result in two registers. But if the size is bigger than
	 8 bytes, we work with pointers.  */
      cif->flags = ffi_struct_type(cif->rtype);
      توڑو;

    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64;
      توڑو;

    طےشدہ:
      cif->flags = FFI_TYPE_INT;
      توڑو;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  انتخاب (cif->abi)
    {
    حالت FFI_PA32:
      ffi_سائز_ڈھیر_pa32(cif);
      توڑو;

    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }

  واپس FFI_OK;
}

خارجی خالی ffi_call_pa32(خالی (*)(UINT32 *, extended_cif *, غیرعلامتی),
			  extended_cif *, غیرعلامتی, غیرعلامتی, غیرعلامتی *,
			  خالی (*fn)(خالی));

خالی ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  extended_cif ecif;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  خالی **avalue_copy = تہی;

  ecif.cif = cif;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  برائے (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحیح سائز = at->سائز;
      اگر (at->type == FFI_TYPE_STRUCT && سائز > 8)
	{
	  حرف *argcopy = alloca (سائز);
	  اگر (avalue_copy == تہی)
	    {
	      avalue_copy = alloca (nargs * حجم (خالی *));
	      حافظہ_نقل (avalue_copy, avalue, nargs * حجم (خالی *));
	      avalue = avalue_copy;
	    }
	  حافظہ_نقل (argcopy, avalue[i], سائز);
	  avalue[i] = argcopy;
	}
    }
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  اگر (rvalue == تہی
#اگر_تعریف_شدہ PA_HPUX
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_LONGDOUBLE))
#ورنہ
      && cif->rtype->type == FFI_TYPE_STRUCT)
#اگر_ختم
    {
      ecif.rvalue = alloca(cif->rtype->سائز);
    }
  ورنہ
    ecif.rvalue = rvalue;


  انتخاب (cif->abi)
    {
    حالت FFI_PA32:
      debug(3, "Calling ffi_call_pa32: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->بائٹیں, cif->flags, ecif.rvalue, (خالی *)fn);
      ffi_call_pa32(ffi_prep_دلائل_pa32, &ecif, cif->بائٹیں,
		     cif->flags, ecif.rvalue, fn);
      توڑو;

    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }
}

#اگر FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_حالت ffi_احاطہ_inner_pa32(ffi_احاطہ *احاطہ, UINT32 *ڈھیر)
{
  ffi_cif *cif;
  خالی **avalue;
  خالی *rvalue;
  /* Functions can return up to 64-bits in registers.  Return address
     must be double word aligned.  */
  اتحاد { دوہرا rd; UINT32 ret[2]; } u;
  ffi_type **p_arg;
  حرف *tmp;
  صحیح i, avn;
  غیرعلامتی صحیح خانہ = پہلا_ARG_خانہ;
  اندراج UINT32 r28 __asm__("r28");

  cif = احاطہ->cif;

  /* If returning via structure, callee will write to our pointer.  */
  اگر (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (خالی *)r28;
  ورنہ
    rvalue = &u;

  avalue = (خالی **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  برائے (i = 0; i < avn; i++)
    {
      صحیح type = (*p_arg)->type;

      انتخاب (type)
	{
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_اشارہ_گر:
	  avalue[i] = (حرف *)(ڈھیر - خانہ) + حجم(UINT32) - (*p_arg)->سائز;
	  توڑو;

	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	  خانہ += (خانہ & 1) ? 1 : 2;
	  avalue[i] = (خالی *)(ڈھیر - خانہ);
	  توڑو;

	حالت FFI_TYPE_FLOAT:
#اگر_تعریف_شدہ PA_LINUX
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  انتخاب (خانہ - پہلا_ARG_خانہ)
	    {
	    حالت 0: fstw(fr4, (خالی *)(ڈھیر - خانہ)); توڑو;
	    حالت 1: fstw(fr5, (خالی *)(ڈھیر - خانہ)); توڑو;
	    حالت 2: fstw(fr6, (خالی *)(ڈھیر - خانہ)); توڑو;
	    حالت 3: fstw(fr7, (خالی *)(ڈھیر - خانہ)); توڑو;
	    }
#اگر_ختم
	  avalue[i] = (خالی *)(ڈھیر - خانہ);
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	  خانہ += (خانہ & 1) ? 1 : 2;
#اگر_تعریف_شدہ PA_LINUX
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  انتخاب (خانہ - پہلا_ARG_خانہ)
	    {
	    حالت 1: fstd(fr5, (خالی *)(ڈھیر - خانہ)); توڑو;
	    حالت 3: fstd(fr7, (خالی *)(ڈھیر - خانہ)); توڑو;
	    }
#اگر_ختم
	  avalue[i] = (خالی *)(ڈھیر - خانہ);
	  توڑو;

#اگر_تعریف_شدہ PA_HPUX
	حالت FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  avalue[i] = (خالی *) *(ڈھیر - خانہ);
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_STRUCT:
	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */
	  اگر((*p_arg)->سائز <= 4)
	    {
	      avalue[i] = (خالی *)(ڈھیر - خانہ) + حجم(UINT32) -
		(*p_arg)->سائز;
	    }
	  ورنہ اگر ((*p_arg)->سائز <= 8)
	    {
	      خانہ += (خانہ & 1) ? 1 : 2;
	      avalue[i] = (خالی *)(ڈھیر - خانہ) + حجم(UINT64) -
		(*p_arg)->سائز;
	    }
	  ورنہ
	    avalue[i] = (خالی *) *(ڈھیر - خانہ);
	  توڑو;

	طےشدہ:
	  FFI_ASSERT(0);
	}

      خانہ++;
      p_arg++;
    }

  /* Invoke the closure.  */
  (احاطہ->fun) (cif, rvalue, avalue, احاطہ->user_ڈیٹا);

  debug(3, "after calling function, ret[0] = %08x, ret[1] = %08x\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  انتخاب (cif->flags)
    {
    حالت FFI_TYPE_UINT8:
      *(ڈھیر - پہلا_ARG_خانہ) = (UINT8)u.ret[0];
      توڑو;
    حالت FFI_TYPE_SINT8:
      *(ڈھیر - پہلا_ARG_خانہ) = (SINT8)u.ret[0];
      توڑو;
    حالت FFI_TYPE_UINT16:
      *(ڈھیر - پہلا_ARG_خانہ) = (UINT16)u.ret[0];
      توڑو;
    حالت FFI_TYPE_SINT16:
      *(ڈھیر - پہلا_ARG_خانہ) = (SINT16)u.ret[0];
      توڑو;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
      *(ڈھیر - پہلا_ARG_خانہ) = u.ret[0];
      توڑو;
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      *(ڈھیر - پہلا_ARG_خانہ) = u.ret[0];
      *(ڈھیر - پہلا_ARG_خانہ - 1) = u.ret[1];
      توڑو;

    حالت FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      توڑو;

    حالت FFI_TYPE_FLOAT:
      fldw(rvalue, fr4);
      توڑو;

    حالت FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      توڑو;

    حالت FFI_TYPE_SMALL_STRUCT1:
    حالت FFI_TYPE_SMALL_STRUCT2:
    حالت FFI_TYPE_SMALL_STRUCT3:
    حالت FFI_TYPE_SMALL_STRUCT4:
      tmp = (خالی*)(ڈھیر -  پہلا_ARG_خانہ);
      tmp += 4 - cif->rtype->سائز;
      حافظہ_نقل((خالی*)tmp, &u, cif->rtype->سائز);
      توڑو;

    حالت FFI_TYPE_SMALL_STRUCT5:
    حالت FFI_TYPE_SMALL_STRUCT6:
    حالت FFI_TYPE_SMALL_STRUCT7:
    حالت FFI_TYPE_SMALL_STRUCT8:
      {
	غیرعلامتی صحیح ret2[2];
	صحیح off;

	/* Right justify ret[0] and ret[1] */
	انتخاب (cif->flags)
	  {
	    حالت FFI_TYPE_SMALL_STRUCT5: off = 3; توڑو;
	    حالت FFI_TYPE_SMALL_STRUCT6: off = 2; توڑو;
	    حالت FFI_TYPE_SMALL_STRUCT7: off = 1; توڑو;
	    طےشدہ: off = 0; توڑو;
	  }

	memset (ret2, 0, حجم (ret2));
	حافظہ_نقل ((حرف *)ret2 + off, &u, 8 - off);

	*(ڈھیر - پہلا_ARG_خانہ) = ret2[0];
	*(ڈھیر - پہلا_ARG_خانہ - 1) = ret2[1];
      }
      توڑو;

    حالت FFI_TYPE_اشارہ_گر:
    حالت FFI_TYPE_VOID:
      توڑو;

    طےشدہ:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      توڑو;
    }
  واپس FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

خارجی خالی ffi_احاطہ_pa32(خالی);

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  /* The layout of a function descriptor.  A function pointer with the PLABEL
     bit set points to a function descriptor.  */
  ساخت pa32_fd
  {
    UINT32 رمز_اشارہ_گر;
    UINT32 gp;
  };

  ساخت ffi_pa32_trampoline_struct
  {
     UINT32 رمز_اشارہ_گر;        /* Pointer to ffi_closure_unix.  */
     UINT32 fake_gp;             /* Pointer to closure, installed as gp.  */
     UINT32 real_gp;             /* Real gp value.  */
  };

  ساخت ffi_pa32_trampoline_struct *tramp;
  ساخت pa32_fd *fd;

  اگر (cif->abi != FFI_PA32)
    واپس FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa32.  */
  fd = (ساخت pa32_fd *)((UINT32)ffi_احاطہ_pa32 & ~3);

  /* Setup trampoline.  */
  tramp = (ساخت ffi_pa32_trampoline_struct *)احاطہ->tramp;
  tramp->رمز_اشارہ_گر = fd->رمز_اشارہ_گر;
  tramp->fake_gp = (UINT32)codeloc & ~3;
  tramp->real_gp = fd->gp;

  احاطہ->cif  = cif;
  احاطہ->user_ڈیٹا = user_ڈیٹا;
  احاطہ->fun  = fun;

  واپس FFI_OK;
}
#اگر_ختم
