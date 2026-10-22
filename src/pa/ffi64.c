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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <stdio.h>

#تعریف_کړه پړاو_پورته(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#تعریف_کړه لومړی_ARG_خونه  0
#تعریف_کړه DEBUG_LEVEL   0

#تعریف_کړه fldw(addr, fpreg) \
  __asm__ متطاير ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#تعریف_کړه fstw(fpreg, addr) \
  __asm__ متطاير ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#تعریف_کړه fldd(addr, fpreg) \
  __asm__ متطاير ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#تعریف_کړه fstd(fpreg, addr) \
  __asm__ متطاير ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#تعریف_کړه debug(lvl, x...) افعل { إذا (lvl <= DEBUG_LEVEL) { چاپ_کړه(x); } } طالما (0)

ساكن مضمن صحيح ffi_struct_type(ffi_type *t)
{
  صحيح sz = t->اندازه;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  إذا (sz <= 16)
    أرجع -sz;
  وإلا
    أرجع FFI_TYPE_STRUCT;
}

/* PA has a downward growing stack, which looks like this.  Stack
   arguments are offset from the argument ponter (AP) in r29.

دننه_واټن
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

فراغ ffi_prep_دلیلونه_pa64(UINT64 *ډېری, extended_cif *ecif, بینښه بایټونه)
{
  سجل بینښه صحيح i;
  سجل ffi_type **p_arg;
  سجل فراغ **p_argv;
  بینښه صحيح خونه = لومړی_ARG_خونه;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ډېری,
	ecif, بایټونه);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  لكل (i = 0; i < ecif->cif->nargs; i++)
    {
      صحيح type = (*p_arg)->type;

      len = (*p_arg)->اندازه;

      بدل (type)
	{
	حالة FFI_TYPE_SINT8:
	  *(SINT64 *)(ډېری + خونه) = *(SINT8 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_UINT8:
	  *(UINT64 *)(ډېری + خونه) = *(UINT8 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_SINT16:
	  *(SINT64 *)(ډېری + خونه) = *(SINT16 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_UINT16:
	  *(UINT64 *)(ډېری + خونه) = *(UINT16 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_SINT32:
	  *(SINT64 *)(ډېری + خونه) = *(SINT32 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_UINT32:
	  *(UINT64 *)(ډېری + خونه) = *(UINT32 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_اشاره_ګر:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		خونه);
	  *(UINT64 *)(ډېری + خونه) = *(UINT64 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", خونه);
	  *(UINT64 *)(ډېری + خونه) = *(UINT32 *)(*p_argv);
	  بدل (خونه - لومړی_ARG_خونه)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    حالة 0: fldw(ډېری + خونه, fr4); اكسر;
	    حالة 1: fldw(ډېری + خونه, fr5); اكسر;
	    حالة 2: fldw(ډېری + خونه, fr6); اكسر;
	    حالة 3: fldw(ډېری + خونه, fr7); اكسر;
	    حالة 4: fldw(ډېری + خونه, fr8); اكسر;
	    حالة 5: fldw(ډېری + خونه, fr9); اكسر;
	    حالة 6: fldw(ډېری + خونه, fr10); اكسر;
	    حالة 7: fldw(ډېری + خونه, fr11); اكسر;
	    }
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", خونه);
	  *(UINT64 *)(ډېری + خونه) = *(UINT64 *)(*p_argv);
	  بدل (خونه - لومړی_ARG_خونه)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    حالة 0: fldd(ډېری + خونه, fr4); اكسر;
	    حالة 1: fldd(ډېری + خونه, fr5); اكسر;
	    حالة 2: fldd(ډېری + خونه, fr6); اكسر;
	    حالة 3: fldd(ډېری + خونه, fr7); اكسر;
	    حالة 4: fldd(ډېری + خونه, fr8); اكسر;
	    حالة 5: fldd(ډېری + خونه, fr9); اكسر;
	    حالة 6: fldd(ډېری + خونه, fr10); اكسر;
	    حالة 7: fldd(ډېری + خونه, fr11); اكسر;
	    }
	  اكسر;

#که_تعریف_شوی PA64_HPUX
	حالة FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  خونه += (خونه & 1);
	  *(UINT64 *)(ډېری + خونه) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(ډېری + خونه + 1) = *(UINT64 *)(*p_argv + 8);
	  اكسر;
#د_که_پای

	حالة FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  إذا (len > 8)
	    خونه += (خونه & 1);
	  حافظه_نقل((حرف *)(ډېری + خونه), (حرف *)*p_argv, len);
	  اكسر;

	افتراضي:
	  FFI_ASSERT(0);
	}

      خونه += پړاو_پورته (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(خونه * 8 <= بایټونه);

  أرجع;
}

ساكن فراغ ffi_اندازه_ډېری_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  صحيح i;
  صحيح z = 0; /* # stack slots */

  لكل (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      صحيح type = (*ptr)->type;
      صحيح اندازه = (*ptr)->اندازه;

      بدل (type)
	{
#که_تعریف_شوی PA64_HPUX
	حالة FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  اكسر;
#د_که_پای

	حالة FFI_TYPE_STRUCT:
	  إذا (اندازه > 8)
	    z += (z & 1);
	  z += پړاو_پورته (اندازه, 8) >> 3;
	  اكسر;

	افتراضي: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  إذا (z <= 8)
    z = 8;
  وإلا
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->بایټونه = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->بایټونه);
}

/* Perform machine dependent cif processing.  */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_COMPLEX:
    حالة FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      اكسر;

    افتراضي:
      cif->flags = (بینښه) cif->rtype->type;
      اكسر;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  بدل (cif->abi)
    {
    حالة FFI_PA64:
      ffi_اندازه_ډېری_pa64(cif);
      اكسر;

    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_pa64(فراغ (*)(UINT64 *, extended_cif *, بینښه),
			  extended_cif *, بینښه, بینښه, بینښه *,
			  فراغ (*fn)(فراغ));

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  إذا (rvalue == عدم
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->اندازه > 16)
    ecif.rvalue = alloca(پړاو_پورته (cif->rtype->اندازه, 16));
  وإلا
    ecif.rvalue = rvalue;


  بدل (cif->abi)
    {
    حالة FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->بایټونه, cif->flags, ecif.rvalue, (فراغ *)fn);
      ffi_call_pa64(ffi_prep_دلیلونه_pa64, &ecif, cif->بایټونه,
		     cif->flags, ecif.rvalue, fn);
      اكسر;

    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}

#إذا FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_حالت ffi_تړون_inner_pa64(ffi_تړون *تړون, UINT64 *ډېری)
{
  ffi_cif *cif;
  فراغ **avalue;
  فراغ *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  اتحاد { اوږد مزدوج rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  حرف *tmp;
  صحيح i, avn;
  بینښه صحيح خونه = لومړی_ARG_خونه;
  سجل UINT64 r28 __asm__("r28");

  cif = تړون->cif;

  /* If returning via structure, callee will write to our pointer.  */
  إذا (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (فراغ *)r28;
  وإلا
    rvalue = &u;

  avalue = (فراغ **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  لكل (i = 0; i < avn; i++)
    {
      صحيح type = (*p_arg)->type;

      بدل (type)
	{
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
	  avalue[i] = (فراغ *)(ډېری + خونه) + 7;
	  اكسر;

	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	  avalue[i] = (فراغ *)(ډېری + خونه) + 6;
	  اكسر;

	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	  avalue[i] = (فراغ *)(ډېری + خونه) + 4;
	  اكسر;

	حالة FFI_TYPE_اشاره_ګر:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	  avalue[i] = (فراغ *)(ډېری + خونه);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  بدل (خونه + لومړی_ARG_خونه)
	    {
	    حالة 0: fstw(fr4, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 1: fstw(fr5, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 2: fstw(fr6, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 3: fstw(fr7, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 4: fstw(fr8, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 5: fstw(fr9, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 6: fstw(fr10, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 7: fstw(fr11, (فراغ *)(ډېری + خونه)); اكسر;
	    }
	  avalue[i] = (فراغ *)(ډېری + خونه) + 4;
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  بدل (خونه + لومړی_ARG_خونه)
	    {
	    حالة 0: fstd(fr4, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 1: fstd(fr5, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 2: fstd(fr6, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 3: fstd(fr7, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 4: fstd(fr8, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 5: fstd(fr9, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 6: fstd(fr10, (فراغ *)(ډېری + خونه)); اكسر;
	    حالة 7: fstd(fr11, (فراغ *)(ډېری + خونه)); اكسر;
	    }
	  avalue[i] = (فراغ *)(ډېری + خونه);
	  اكسر;

#که_تعریف_شوی PA64_HPUX
	حالة FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  خونه += (خونه & 1);
	  avalue[i] = (فراغ *)(ډېری + خونه);
	  اكسر;
#د_که_پای

	حالة FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  إذا((*p_arg)->اندازه > 8)
	    خونه += (خونه & 1);
	  avalue[i] = (فراغ *) (ډېری + خونه);
	  اكسر;

	افتراضي:
	  FFI_ASSERT(0);
	}

      خونه += (پړاو_پورته ((*p_arg)->اندازه, 8) >> 3);
      p_arg++;
    }

  /* Invoke the closure.  */
  (تړون->fun) (cif, rvalue, avalue, تړون->user_معلومات);

  debug(3, "after calling function, ret[0] = %16lx, ret[1] = %16lx\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  بدل (cif->flags)
    {
    حالة FFI_TYPE_UINT8:
      *(ډېری + لومړی_ARG_خونه) = (UINT8)u.ret[0];
      اكسر;
    حالة FFI_TYPE_SINT8:
      *(ډېری + لومړی_ARG_خونه) = (SINT8)u.ret[0];
      اكسر;
    حالة FFI_TYPE_UINT16:
      *(ډېری + لومړی_ARG_خونه) = (UINT16)u.ret[0];
      اكسر;
    حالة FFI_TYPE_SINT16:
      *(ډېری + لومړی_ARG_خونه) = (SINT16)u.ret[0];
      اكسر;
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
      *(ډېری + لومړی_ARG_خونه) = (SINT32)u.ret[0];
      اكسر;
    حالة FFI_TYPE_UINT32:
      *(ډېری - لومړی_ARG_خونه) = (UINT32)u.ret[0];
      اكسر;
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_اشاره_ګر:
      *(ډېری - لومړی_ARG_خونه) = u.ret[0];
      اكسر;

    حالة FFI_TYPE_LONGDOUBLE:
      *(ډېری + لومړی_ARG_خونه) = u.ret[0];
      *(ډېری + لومړی_ARG_خونه + 1) = u.ret[1];
      اكسر;

    حالة FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      اكسر;

    حالة FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      اكسر;

    حالة FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      اكسر;

    حالة -1:
    حالة -2:
    حالة -3:
    حالة -4:
    حالة -5:
    حالة -6:
    حالة -7:
    حالة -8:
    حالة -9:
    حالة -10:
    حالة -11:
    حالة -12:
    حالة -13:
    حالة -14:
    حالة -15:
    حالة -16:
      tmp = (فراغ*)(ډېری + لومړی_ARG_خونه);
      حافظه_نقل((فراغ*)tmp, &u, cif->rtype->اندازه);
      اكسر;

    حالة FFI_TYPE_VOID:
      اكسر;

    افتراضي:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      اكسر;
    }
  أرجع FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

خارجي فراغ ffi_تړون_pa64(فراغ);

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون* تړون,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  /* The layout of a function descriptor.  */
  بنية pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 کوډ_اشاره_ګر;
    UINT64 gp;
  };

  بنية ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 کوډ_اشاره_ګر;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  بنية ffi_pa64_trampoline_struct *tramp;
  بنية pa64_fd *fd;

  إذا (cif->abi != FFI_PA64)
    أرجع FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (بنية pa64_fd *)((UINT64)ffi_تړون_pa64);

  /* Setup trampoline.  */
  tramp = (بنية ffi_pa64_trampoline_struct *)تړون->tramp;
  tramp->کوډ_اشاره_ګر = fd->کوډ_اشاره_ګر;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  تړون->cif  = cif;
  تړون->user_معلومات = user_معلومات;
  تړون->fun  = fun;

  أرجع FFI_OK;
}
#د_که_پای
