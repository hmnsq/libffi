/* -----------------------------------------------------------------------
   ffi64.c - (c) 2022 John David Anglin <dave.anglin@bell.net>
           
   HPPA Foreign Function Interface
   PA 64-Bit ABI support 

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

#تعریف_کرو پہلا_ARG_خانہ  0
#تعریف_کرو DEBUG_LEVEL   0

#تعریف_کرو fldw(addr, fpreg) \
  __asm__ متلون ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#تعریف_کرو fstw(fpreg, addr) \
  __asm__ متلون ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#تعریف_کرو fldd(addr, fpreg) \
  __asm__ متلون ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#تعریف_کرو fstd(fpreg, addr) \
  __asm__ متلون ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#تعریف_کرو debug(lvl, x...) کرو { اگر (lvl <= DEBUG_LEVEL) { لکھو(x); } } جب (0)

ساکن اندرونی صحیح ffi_struct_type(ffi_type *t)
{
  صحیح sz = t->سائز;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  اگر (sz <= 16)
    واپس -sz;
  ورنہ
    واپس FFI_TYPE_STRUCT;
}

/* PA has a downward growing stack, which looks like this.  Stack
   arguments are offset from the argument ponter (AP) in r29.

انحراف
	[ Fixed args ]
   AP-64                arg word 0 (r26, fr4)
   AP-56                arg word 1 (r25, fr5)
   AP-48                arg word 2 (r24, fr6)
   AP-40                arg word 3 (r23, fr7)
   AP-32                arg word 4 (r22, fr8)
   AP-24                arg word 5 (r21, fr9)
   AP-16                arg word 6 (r20, fr10)
   AP-8	                arg word 7 (r19, fr11)
	[ Variable args; AP = SP-16 if there are no variable args ]
   AP			stack arg 0
   AP+8			stack arg 1
   ...
	[ Frame marker ]
   SP-16                RP
   SP-8                 previous SP

   The first eight argument words on the stack are reserved for use by
   the callee.  Instead, the general and floating registers replace
   the first four argument slots.  Non FP arguments are passed solely
   in the general registers.  Single and double FP arguments are passed
   in both general and floating registers when using libffi.

   The registers are allocated in the same manner as stack slots.
   This allows the callee to save its arguments on the stack if
   necessary:

   arg word 0 -> gr26 or fr4L or fr4R
   arg word 1 -> gr25 or fr5L or fr5R
   arg word 2 -> gr24 or fr6L or fr6R
   arg word 3 -> gr23 or fr7L or fr7R
   ...

   Single Single-precision floating-point parameters, when passed in
   floating-point registers, are passed in the right halves of the
   floating point registers; the left halves are unused.

   Quad-precision floating-point parameters within the first 64 bytes of
   the parameter list are always passed in general registers.

   The rest of the arguments are passed on the stack starting at AP.

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

خالی ffi_prep_دلائل_pa64(UINT64 *ڈھیر, extended_cif *ecif, غیرعلامتی بائٹیں)
{
  اندراج غیرعلامتی صحیح i;
  اندراج ffi_type **p_arg;
  اندراج خالی **p_argv;
  غیرعلامتی صحیح خانہ = پہلا_ARG_خانہ;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ڈھیر,
	ecif, بائٹیں);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  برائے (i = 0; i < ecif->cif->nargs; i++)
    {
      صحیح type = (*p_arg)->type;

      len = (*p_arg)->سائز;

      انتخاب (type)
	{
	حالت FFI_TYPE_SINT8:
	  *(SINT64 *)(ڈھیر + خانہ) = *(SINT8 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_UINT8:
	  *(UINT64 *)(ڈھیر + خانہ) = *(UINT8 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_SINT16:
	  *(SINT64 *)(ڈھیر + خانہ) = *(SINT16 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_UINT16:
	  *(UINT64 *)(ڈھیر + خانہ) = *(UINT16 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_SINT32:
	  *(SINT64 *)(ڈھیر + خانہ) = *(SINT32 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_UINT32:
	  *(UINT64 *)(ڈھیر + خانہ) = *(UINT32 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_اشارہ_گر:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		خانہ);
	  *(UINT64 *)(ڈھیر + خانہ) = *(UINT64 *)(*p_argv);
	  توڑو;

	حالت FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", خانہ);
	  *(UINT64 *)(ڈھیر + خانہ) = *(UINT32 *)(*p_argv);
	  انتخاب (خانہ - پہلا_ARG_خانہ)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    حالت 0: fldw(ڈھیر + خانہ, fr4); توڑو;
	    حالت 1: fldw(ڈھیر + خانہ, fr5); توڑو;
	    حالت 2: fldw(ڈھیر + خانہ, fr6); توڑو;
	    حالت 3: fldw(ڈھیر + خانہ, fr7); توڑو;
	    حالت 4: fldw(ڈھیر + خانہ, fr8); توڑو;
	    حالت 5: fldw(ڈھیر + خانہ, fr9); توڑو;
	    حالت 6: fldw(ڈھیر + خانہ, fr10); توڑو;
	    حالت 7: fldw(ڈھیر + خانہ, fr11); توڑو;
	    }
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", خانہ);
	  *(UINT64 *)(ڈھیر + خانہ) = *(UINT64 *)(*p_argv);
	  انتخاب (خانہ - پہلا_ARG_خانہ)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    حالت 0: fldd(ڈھیر + خانہ, fr4); توڑو;
	    حالت 1: fldd(ڈھیر + خانہ, fr5); توڑو;
	    حالت 2: fldd(ڈھیر + خانہ, fr6); توڑو;
	    حالت 3: fldd(ڈھیر + خانہ, fr7); توڑو;
	    حالت 4: fldd(ڈھیر + خانہ, fr8); توڑو;
	    حالت 5: fldd(ڈھیر + خانہ, fr9); توڑو;
	    حالت 6: fldd(ڈھیر + خانہ, fr10); توڑو;
	    حالت 7: fldd(ڈھیر + خانہ, fr11); توڑو;
	    }
	  توڑو;

#اگر_تعریف_شدہ PA64_HPUX
	حالت FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  خانہ += (خانہ & 1);
	  *(UINT64 *)(ڈھیر + خانہ) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(ڈھیر + خانہ + 1) = *(UINT64 *)(*p_argv + 8);
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  اگر (len > 8)
	    خانہ += (خانہ & 1);
	  حافظہ_نقل((حرف *)(ڈھیر + خانہ), (حرف *)*p_argv, len);
	  توڑو;

	طےشدہ:
	  FFI_ASSERT(0);
	}

      خانہ += باری_اوپر (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(خانہ * 8 <= بائٹیں);

  واپس;
}

ساکن خالی ffi_سائز_ڈھیر_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  صحیح i;
  صحیح z = 0; /* # stack slots */

  برائے (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      صحیح type = (*ptr)->type;
      صحیح سائز = (*ptr)->سائز;

      انتخاب (type)
	{
#اگر_تعریف_شدہ PA64_HPUX
	حالت FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_STRUCT:
	  اگر (سائز > 8)
	    z += (z & 1);
	  z += باری_اوپر (سائز, 8) >> 3;
	  توڑو;

	طےشدہ: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  اگر (z <= 8)
    z = 8;
  ورنہ
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->بائٹیں = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->بائٹیں);
}

/* Perform machine dependent cif processing.  */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_COMPLEX:
    حالت FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      توڑو;

    طےشدہ:
      cif->flags = (غیرعلامتی) cif->rtype->type;
      توڑو;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  انتخاب (cif->abi)
    {
    حالت FFI_PA64:
      ffi_سائز_ڈھیر_pa64(cif);
      توڑو;

    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }

  واپس FFI_OK;
}

خارجی خالی ffi_call_pa64(خالی (*)(UINT64 *, extended_cif *, غیرعلامتی),
			  extended_cif *, غیرعلامتی, غیرعلامتی, غیرعلامتی *,
			  خالی (*fn)(خالی));

خالی ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  اگر (rvalue == تہی
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->سائز > 16)
    ecif.rvalue = alloca(باری_اوپر (cif->rtype->سائز, 16));
  ورنہ
    ecif.rvalue = rvalue;


  انتخاب (cif->abi)
    {
    حالت FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->بائٹیں, cif->flags, ecif.rvalue, (خالی *)fn);
      ffi_call_pa64(ffi_prep_دلائل_pa64, &ecif, cif->بائٹیں,
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
ffi_حالت ffi_احاطہ_inner_pa64(ffi_احاطہ *احاطہ, UINT64 *ڈھیر)
{
  ffi_cif *cif;
  خالی **avalue;
  خالی *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  اتحاد { طویل دوہرا rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  حرف *tmp;
  صحیح i, avn;
  غیرعلامتی صحیح خانہ = پہلا_ARG_خانہ;
  اندراج UINT64 r28 __asm__("r28");

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
	  avalue[i] = (خالی *)(ڈھیر + خانہ) + 7;
	  توڑو;

	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	  avalue[i] = (خالی *)(ڈھیر + خانہ) + 6;
	  توڑو;

	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	  avalue[i] = (خالی *)(ڈھیر + خانہ) + 4;
	  توڑو;

	حالت FFI_TYPE_اشارہ_گر:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	  avalue[i] = (خالی *)(ڈھیر + خانہ);
	  توڑو;

	حالت FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  انتخاب (خانہ + پہلا_ARG_خانہ)
	    {
	    حالت 0: fstw(fr4, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 1: fstw(fr5, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 2: fstw(fr6, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 3: fstw(fr7, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 4: fstw(fr8, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 5: fstw(fr9, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 6: fstw(fr10, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 7: fstw(fr11, (خالی *)(ڈھیر + خانہ)); توڑو;
	    }
	  avalue[i] = (خالی *)(ڈھیر + خانہ) + 4;
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  انتخاب (خانہ + پہلا_ARG_خانہ)
	    {
	    حالت 0: fstd(fr4, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 1: fstd(fr5, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 2: fstd(fr6, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 3: fstd(fr7, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 4: fstd(fr8, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 5: fstd(fr9, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 6: fstd(fr10, (خالی *)(ڈھیر + خانہ)); توڑو;
	    حالت 7: fstd(fr11, (خالی *)(ڈھیر + خانہ)); توڑو;
	    }
	  avalue[i] = (خالی *)(ڈھیر + خانہ);
	  توڑو;

#اگر_تعریف_شدہ PA64_HPUX
	حالت FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  خانہ += (خانہ & 1);
	  avalue[i] = (خالی *)(ڈھیر + خانہ);
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  اگر((*p_arg)->سائز > 8)
	    خانہ += (خانہ & 1);
	  avalue[i] = (خالی *) (ڈھیر + خانہ);
	  توڑو;

	طےشدہ:
	  FFI_ASSERT(0);
	}

      خانہ += (باری_اوپر ((*p_arg)->سائز, 8) >> 3);
      p_arg++;
    }

  /* Invoke the closure.  */
  (احاطہ->fun) (cif, rvalue, avalue, احاطہ->user_ڈیٹا);

  debug(3, "after calling function, ret[0] = %16lx, ret[1] = %16lx\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  انتخاب (cif->flags)
    {
    حالت FFI_TYPE_UINT8:
      *(ڈھیر + پہلا_ARG_خانہ) = (UINT8)u.ret[0];
      توڑو;
    حالت FFI_TYPE_SINT8:
      *(ڈھیر + پہلا_ARG_خانہ) = (SINT8)u.ret[0];
      توڑو;
    حالت FFI_TYPE_UINT16:
      *(ڈھیر + پہلا_ARG_خانہ) = (UINT16)u.ret[0];
      توڑو;
    حالت FFI_TYPE_SINT16:
      *(ڈھیر + پہلا_ARG_خانہ) = (SINT16)u.ret[0];
      توڑو;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      *(ڈھیر + پہلا_ARG_خانہ) = (SINT32)u.ret[0];
      توڑو;
    حالت FFI_TYPE_UINT32:
      *(ڈھیر - پہلا_ARG_خانہ) = (UINT32)u.ret[0];
      توڑو;
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_اشارہ_گر:
      *(ڈھیر - پہلا_ARG_خانہ) = u.ret[0];
      توڑو;

    حالت FFI_TYPE_LONGDOUBLE:
      *(ڈھیر + پہلا_ARG_خانہ) = u.ret[0];
      *(ڈھیر + پہلا_ARG_خانہ + 1) = u.ret[1];
      توڑو;

    حالت FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      توڑو;

    حالت FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      توڑو;

    حالت FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      توڑو;

    حالت -1:
    حالت -2:
    حالت -3:
    حالت -4:
    حالت -5:
    حالت -6:
    حالت -7:
    حالت -8:
    حالت -9:
    حالت -10:
    حالت -11:
    حالت -12:
    حالت -13:
    حالت -14:
    حالت -15:
    حالت -16:
      tmp = (خالی*)(ڈھیر + پہلا_ARG_خانہ);
      حافظہ_نقل((خالی*)tmp, &u, cif->rtype->سائز);
      توڑو;

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

خارجی خالی ffi_احاطہ_pa64(خالی);

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  /* The layout of a function descriptor.  */
  ساخت pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 رمز_اشارہ_گر;
    UINT64 gp;
  };

  ساخت ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 رمز_اشارہ_گر;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  ساخت ffi_pa64_trampoline_struct *tramp;
  ساخت pa64_fd *fd;

  اگر (cif->abi != FFI_PA64)
    واپس FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (ساخت pa64_fd *)((UINT64)ffi_احاطہ_pa64);

  /* Setup trampoline.  */
  tramp = (ساخت ffi_pa64_trampoline_struct *)احاطہ->tramp;
  tramp->رمز_اشارہ_گر = fd->رمز_اشارہ_گر;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  احاطہ->cif  = cif;
  احاطہ->user_ڈیٹا = user_ڈیٹا;
  احاطہ->fun  = fun;

  واپس FFI_OK;
}
#اگر_ختم
