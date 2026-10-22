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

#define ROUND_UP(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#define FIRST_ARG_SLOT  0
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ متطاير ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ متطاير ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ متطاير ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ متطاير ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) افعل { إذا (lvl <= DEBUG_LEVEL) { printf(x); } } طالما (0)

ساكن مضمن صحيح ffi_struct_type(ffi_type *t)
{
  صحيح sz = t->size;

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

   Offset
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

فراغ ffi_prep_وسائط_pa64(UINT64 *مكدس, extended_cif *ecif, unsigned bytes)
{
  سجل unsigned صحيح i;
  سجل ffi_type **p_arg;
  سجل فراغ **p_argv;
  unsigned صحيح slot = FIRST_ARG_SLOT;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, مكدس,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  لكل (i = 0; i < ecif->cif->nargs; i++)
    {
      صحيح type = (*p_arg)->type;

      len = (*p_arg)->size;

      بدل (type)
	{
	حالة FFI_TYPE_SINT8:
	  *(SINT64 *)(مكدس + slot) = *(SINT8 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_UINT8:
	  *(UINT64 *)(مكدس + slot) = *(UINT8 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_SINT16:
	  *(SINT64 *)(مكدس + slot) = *(SINT16 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_UINT16:
	  *(UINT64 *)(مكدس + slot) = *(UINT16 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_SINT32:
	  *(SINT64 *)(مكدس + slot) = *(SINT32 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_UINT32:
	  *(UINT64 *)(مكدس + slot) = *(UINT32 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_POINTER:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		slot);
	  *(UINT64 *)(مكدس + slot) = *(UINT64 *)(*p_argv);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT64 *)(مكدس + slot) = *(UINT32 *)(*p_argv);
	  بدل (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    حالة 0: fldw(مكدس + slot, fr4); اكسر;
	    حالة 1: fldw(مكدس + slot, fr5); اكسر;
	    حالة 2: fldw(مكدس + slot, fr6); اكسر;
	    حالة 3: fldw(مكدس + slot, fr7); اكسر;
	    حالة 4: fldw(مكدس + slot, fr8); اكسر;
	    حالة 5: fldw(مكدس + slot, fr9); اكسر;
	    حالة 6: fldw(مكدس + slot, fr10); اكسر;
	    حالة 7: fldw(مكدس + slot, fr11); اكسر;
	    }
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(مكدس + slot) = *(UINT64 *)(*p_argv);
	  بدل (slot - FIRST_ARG_SLOT)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    حالة 0: fldd(مكدس + slot, fr4); اكسر;
	    حالة 1: fldd(مكدس + slot, fr5); اكسر;
	    حالة 2: fldd(مكدس + slot, fr6); اكسر;
	    حالة 3: fldd(مكدس + slot, fr7); اكسر;
	    حالة 4: fldd(مكدس + slot, fr8); اكسر;
	    حالة 5: fldd(مكدس + slot, fr9); اكسر;
	    حالة 6: fldd(مكدس + slot, fr10); اكسر;
	    حالة 7: fldd(مكدس + slot, fr11); اكسر;
	    }
	  اكسر;

#ifdef PA64_HPUX
	حالة FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  slot += (slot & 1);
	  *(UINT64 *)(مكدس + slot) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(مكدس + slot + 1) = *(UINT64 *)(*p_argv + 8);
	  اكسر;
#endif

	حالة FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  إذا (len > 8)
	    slot += (slot & 1);
	  memcpy((حرف *)(مكدس + slot), (حرف *)*p_argv, len);
	  اكسر;

	افتراضي:
	  FFI_ASSERT(0);
	}

      slot += ROUND_UP (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(slot * 8 <= bytes);

  أرجع;
}

ساكن فراغ ffi_size_مكدس_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  صحيح i;
  صحيح z = 0; /* # stack slots */

  لكل (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      صحيح type = (*ptr)->type;
      صحيح size = (*ptr)->size;

      بدل (type)
	{
#ifdef PA64_HPUX
	حالة FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  اكسر;
#endif

	حالة FFI_TYPE_STRUCT:
	  إذا (size > 8)
	    z += (z & 1);
	  z += ROUND_UP (size, 8) >> 3;
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
  cif->bytes = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
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
      cif->flags = (unsigned) cif->rtype->type;
      اكسر;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  بدل (cif->abi)
    {
    حالة FFI_PA64:
      ffi_size_مكدس_pa64(cif);
      اكسر;

    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_pa64(فراغ (*)(UINT64 *, extended_cif *, unsigned),
			  extended_cif *, unsigned, unsigned, unsigned *,
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
      && cif->rtype->size > 16)
    ecif.rvalue = alloca(ROUND_UP (cif->rtype->size, 16));
  وإلا
    ecif.rvalue = rvalue;


  بدل (cif->abi)
    {
    حالة FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (فراغ *)fn);
      ffi_call_pa64(ffi_prep_وسائط_pa64, &ecif, cif->bytes,
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
ffi_status ffi_closure_inner_pa64(ffi_closure *closure, UINT64 *مكدس)
{
  ffi_cif *cif;
  فراغ **avalue;
  فراغ *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  اتحاد { long مزدوج rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  حرف *tmp;
  صحيح i, avn;
  unsigned صحيح slot = FIRST_ARG_SLOT;
  سجل UINT64 r28 __asm__("r28");

  cif = closure->cif;

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
	  avalue[i] = (فراغ *)(مكدس + slot) + 7;
	  اكسر;

	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	  avalue[i] = (فراغ *)(مكدس + slot) + 6;
	  اكسر;

	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	  avalue[i] = (فراغ *)(مكدس + slot) + 4;
	  اكسر;

	حالة FFI_TYPE_POINTER:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	  avalue[i] = (فراغ *)(مكدس + slot);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  بدل (slot + FIRST_ARG_SLOT)
	    {
	    حالة 0: fstw(fr4, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 1: fstw(fr5, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 2: fstw(fr6, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 3: fstw(fr7, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 4: fstw(fr8, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 5: fstw(fr9, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 6: fstw(fr10, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 7: fstw(fr11, (فراغ *)(مكدس + slot)); اكسر;
	    }
	  avalue[i] = (فراغ *)(مكدس + slot) + 4;
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  بدل (slot + FIRST_ARG_SLOT)
	    {
	    حالة 0: fstd(fr4, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 1: fstd(fr5, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 2: fstd(fr6, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 3: fstd(fr7, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 4: fstd(fr8, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 5: fstd(fr9, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 6: fstd(fr10, (فراغ *)(مكدس + slot)); اكسر;
	    حالة 7: fstd(fr11, (فراغ *)(مكدس + slot)); اكسر;
	    }
	  avalue[i] = (فراغ *)(مكدس + slot);
	  اكسر;

#ifdef PA64_HPUX
	حالة FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  slot += (slot & 1);
	  avalue[i] = (فراغ *)(مكدس + slot);
	  اكسر;
#endif

	حالة FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  إذا((*p_arg)->size > 8)
	    slot += (slot & 1);
	  avalue[i] = (فراغ *) (مكدس + slot);
	  اكسر;

	افتراضي:
	  FFI_ASSERT(0);
	}

      slot += (ROUND_UP ((*p_arg)->size, 8) >> 3);
      p_arg++;
    }

  /* Invoke the closure.  */
  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  debug(3, "after calling function, ret[0] = %16lx, ret[1] = %16lx\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  بدل (cif->flags)
    {
    حالة FFI_TYPE_UINT8:
      *(مكدس + FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      اكسر;
    حالة FFI_TYPE_SINT8:
      *(مكدس + FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      اكسر;
    حالة FFI_TYPE_UINT16:
      *(مكدس + FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      اكسر;
    حالة FFI_TYPE_SINT16:
      *(مكدس + FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      اكسر;
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
      *(مكدس + FIRST_ARG_SLOT) = (SINT32)u.ret[0];
      اكسر;
    حالة FFI_TYPE_UINT32:
      *(مكدس - FIRST_ARG_SLOT) = (UINT32)u.ret[0];
      اكسر;
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_POINTER:
      *(مكدس - FIRST_ARG_SLOT) = u.ret[0];
      اكسر;

    حالة FFI_TYPE_LONGDOUBLE:
      *(مكدس + FIRST_ARG_SLOT) = u.ret[0];
      *(مكدس + FIRST_ARG_SLOT + 1) = u.ret[1];
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
      tmp = (فراغ*)(مكدس + FIRST_ARG_SLOT);
      memcpy((فراغ*)tmp, &u, cif->rtype->size);
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

خارجي فراغ ffi_closure_pa64(فراغ);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		      فراغ *user_data,
		      فراغ *codeloc)
{
  /* The layout of a function descriptor.  */
  بنية pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 code_pointer;
    UINT64 gp;
  };

  بنية ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 code_pointer;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  بنية ffi_pa64_trampoline_struct *tramp;
  بنية pa64_fd *fd;

  إذا (cif->abi != FFI_PA64)
    أرجع FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (بنية pa64_fd *)((UINT64)ffi_closure_pa64);

  /* Setup trampoline.  */
  tramp = (بنية ffi_pa64_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  أرجع FFI_OK;
}
#endif
