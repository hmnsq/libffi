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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdio.h>

#define ROUND_UP(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#define FIRST_ARG_SLOT  0
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ চঞ্চল ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ চঞ্চল ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ চঞ্চল ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ চঞ্চল ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) কৰা { যদি (lvl <= DEBUG_LEVEL) { printf(x); } } যেতিয়ালৈ (0)

স্থির অন্তঃস্থ পূর্ণাঙ্ক ffi_struct_type(ffi_type *t)
{
  পূর্ণাঙ্ক sz = t->size;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  যদি (sz <= 16)
    ঘূৰাই -sz;
  অন্যথা
    ঘূৰাই FFI_TYPE_STRUCT;
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

শূন্য ffi_prep_args_pa64(UINT64 *স্তূপ, extended_cif *ecif, unsigned bytes)
{
  পঞ্জিকা unsigned পূর্ণাঙ্ক i;
  পঞ্জিকা ffi_type **p_arg;
  পঞ্জিকা শূন্য **p_argv;
  unsigned পূর্ণাঙ্ক slot = FIRST_ARG_SLOT;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, স্তূপ,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  বাবে (i = 0; i < ecif->cif->nargs; i++)
    {
      পূর্ণাঙ্ক type = (*p_arg)->type;

      len = (*p_arg)->size;

      ৱিকল্প (type)
	{
	পক্ষ FFI_TYPE_SINT8:
	  *(SINT64 *)(স্তূপ + slot) = *(SINT8 *)(*p_argv);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT8:
	  *(UINT64 *)(স্তূপ + slot) = *(UINT8 *)(*p_argv);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT16:
	  *(SINT64 *)(স্তূপ + slot) = *(SINT16 *)(*p_argv);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT16:
	  *(UINT64 *)(স্তূপ + slot) = *(UINT16 *)(*p_argv);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT32:
	  *(SINT64 *)(স্তূপ + slot) = *(SINT32 *)(*p_argv);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT32:
	  *(UINT64 *)(স্তূপ + slot) = *(UINT32 *)(*p_argv);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_POINTER:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		slot);
	  *(UINT64 *)(স্তূপ + slot) = *(UINT64 *)(*p_argv);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT64 *)(স্তূপ + slot) = *(UINT32 *)(*p_argv);
	  ৱিকল্প (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    পক্ষ 0: fldw(স্তূপ + slot, fr4); ভঙ্গ;
	    পক্ষ 1: fldw(স্তূপ + slot, fr5); ভঙ্গ;
	    পক্ষ 2: fldw(স্তূপ + slot, fr6); ভঙ্গ;
	    পক্ষ 3: fldw(স্তূপ + slot, fr7); ভঙ্গ;
	    পক্ষ 4: fldw(স্তূপ + slot, fr8); ভঙ্গ;
	    পক্ষ 5: fldw(স্তূপ + slot, fr9); ভঙ্গ;
	    পক্ষ 6: fldw(স্তূপ + slot, fr10); ভঙ্গ;
	    পক্ষ 7: fldw(স্তূপ + slot, fr11); ভঙ্গ;
	    }
	  ভঙ্গ;

	পক্ষ FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(স্তূপ + slot) = *(UINT64 *)(*p_argv);
	  ৱিকল্প (slot - FIRST_ARG_SLOT)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    পক্ষ 0: fldd(স্তূপ + slot, fr4); ভঙ্গ;
	    পক্ষ 1: fldd(স্তূপ + slot, fr5); ভঙ্গ;
	    পক্ষ 2: fldd(স্তূপ + slot, fr6); ভঙ্গ;
	    পক্ষ 3: fldd(স্তূপ + slot, fr7); ভঙ্গ;
	    পক্ষ 4: fldd(স্তূপ + slot, fr8); ভঙ্গ;
	    পক্ষ 5: fldd(স্তূপ + slot, fr9); ভঙ্গ;
	    পক্ষ 6: fldd(স্তূপ + slot, fr10); ভঙ্গ;
	    পক্ষ 7: fldd(স্তূপ + slot, fr11); ভঙ্গ;
	    }
	  ভঙ্গ;

#ifdef PA64_HPUX
	পক্ষ FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  slot += (slot & 1);
	  *(UINT64 *)(স্তূপ + slot) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(স্তূপ + slot + 1) = *(UINT64 *)(*p_argv + 8);
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  যদি (len > 8)
	    slot += (slot & 1);
	  memcpy((ৱর্ণ *)(স্তূপ + slot), (ৱর্ণ *)*p_argv, len);
	  ভঙ্গ;

	স্ৱভাৱ:
	  FFI_ASSERT(0);
	}

      slot += ROUND_UP (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(slot * 8 <= bytes);

  ঘূৰাই;
}

স্থির শূন্য ffi_size_স্তূপ_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক z = 0; /* # stack slots */

  বাবে (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      পূর্ণাঙ্ক type = (*ptr)->type;
      পূর্ণাঙ্ক size = (*ptr)->size;

      ৱিকল্প (type)
	{
#ifdef PA64_HPUX
	পক্ষ FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_STRUCT:
	  যদি (size > 8)
	    z += (z & 1);
	  z += ROUND_UP (size, 8) >> 3;
	  ভঙ্গ;

	স্ৱভাৱ: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  যদি (z <= 8)
    z = 8;
  অন্যথা
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->bytes = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  ৱিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_COMPLEX:
    পক্ষ FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      ভঙ্গ;

    স্ৱভাৱ:
      cif->flags = (unsigned) cif->rtype->type;
      ভঙ্গ;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  ৱিকল্প (cif->abi)
    {
    পক্ষ FFI_PA64:
      ffi_size_স্তূপ_pa64(cif);
      ভঙ্গ;

    স্ৱভাৱ:
      FFI_ASSERT(0);
      ভঙ্গ;
    }

  ঘূৰাই FFI_OK;
}

বাহ্য শূন্য ffi_call_pa64(শূন্য (*)(UINT64 *, extended_cif *, unsigned),
			  extended_cif *, unsigned, unsigned, unsigned *,
			  শূন্য (*fn)(শূন্য));

শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  যদি (rvalue == রিক্ত
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->size > 16)
    ecif.rvalue = alloca(ROUND_UP (cif->rtype->size, 16));
  অন্যথা
    ecif.rvalue = rvalue;


  ৱিকল্প (cif->abi)
    {
    পক্ষ FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (শূন্য *)fn);
      ffi_call_pa64(ffi_prep_args_pa64, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      ভঙ্গ;

    স্ৱভাৱ:
      FFI_ASSERT(0);
      ভঙ্গ;
    }
}

#যদি FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_status ffi_closure_inner_pa64(ffi_closure *closure, UINT64 *স্তূপ)
{
  ffi_cif *cif;
  শূন্য **avalue;
  শূন্য *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  সঙ্ঘ { long দ্ৱিগুণ rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  ৱর্ণ *tmp;
  পূর্ণাঙ্ক i, avn;
  unsigned পূর্ণাঙ্ক slot = FIRST_ARG_SLOT;
  পঞ্জিকা UINT64 r28 __asm__("r28");

  cif = closure->cif;

  /* If returning via structure, callee will write to our pointer.  */
  যদি (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (শূন্য *)r28;
  অন্যথা
    rvalue = &u;

  avalue = (শূন্য **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  বাবে (i = 0; i < avn; i++)
    {
      পূর্ণাঙ্ক type = (*p_arg)->type;

      ৱিকল্প (type)
	{
	পক্ষ FFI_TYPE_SINT8:
	পক্ষ FFI_TYPE_UINT8:
	  avalue[i] = (শূন্য *)(স্তূপ + slot) + 7;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT16:
	পক্ষ FFI_TYPE_UINT16:
	  avalue[i] = (শূন্য *)(স্তূপ + slot) + 6;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT32:
	পক্ষ FFI_TYPE_UINT32:
	  avalue[i] = (শূন্য *)(স্তূপ + slot) + 4;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_POINTER:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_UINT64:
	  avalue[i] = (শূন্য *)(স্তূপ + slot);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  ৱিকল্প (slot + FIRST_ARG_SLOT)
	    {
	    পক্ষ 0: fstw(fr4, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 1: fstw(fr5, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 2: fstw(fr6, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 3: fstw(fr7, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 4: fstw(fr8, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 5: fstw(fr9, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 6: fstw(fr10, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 7: fstw(fr11, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    }
	  avalue[i] = (শূন্য *)(স্তূপ + slot) + 4;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  ৱিকল্প (slot + FIRST_ARG_SLOT)
	    {
	    পক্ষ 0: fstd(fr4, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 1: fstd(fr5, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 2: fstd(fr6, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 3: fstd(fr7, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 4: fstd(fr8, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 5: fstd(fr9, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 6: fstd(fr10, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    পক্ষ 7: fstd(fr11, (শূন্য *)(স্তূপ + slot)); ভঙ্গ;
	    }
	  avalue[i] = (শূন্য *)(স্তূপ + slot);
	  ভঙ্গ;

#ifdef PA64_HPUX
	পক্ষ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  slot += (slot & 1);
	  avalue[i] = (শূন্য *)(স্তূপ + slot);
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  যদি((*p_arg)->size > 8)
	    slot += (slot & 1);
	  avalue[i] = (শূন্য *) (স্তূপ + slot);
	  ভঙ্গ;

	স্ৱভাৱ:
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
  ৱিকল্প (cif->flags)
    {
    পক্ষ FFI_TYPE_UINT8:
      *(স্তূপ + FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT8:
      *(স্তূপ + FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT16:
      *(স্তূপ + FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT16:
      *(স্তূপ + FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      ভঙ্গ;
    পক্ষ FFI_TYPE_INT:
    পক্ষ FFI_TYPE_SINT32:
      *(স্তূপ + FIRST_ARG_SLOT) = (SINT32)u.ret[0];
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT32:
      *(স্তূপ - FIRST_ARG_SLOT) = (UINT32)u.ret[0];
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_POINTER:
      *(স্তূপ - FIRST_ARG_SLOT) = u.ret[0];
      ভঙ্গ;

    পক্ষ FFI_TYPE_LONGDOUBLE:
      *(স্তূপ + FIRST_ARG_SLOT) = u.ret[0];
      *(স্তূপ + FIRST_ARG_SLOT + 1) = u.ret[1];
      ভঙ্গ;

    পক্ষ FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      ভঙ্গ;

    পক্ষ FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      ভঙ্গ;

    পক্ষ FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      ভঙ্গ;

    পক্ষ -1:
    পক্ষ -2:
    পক্ষ -3:
    পক্ষ -4:
    পক্ষ -5:
    পক্ষ -6:
    পক্ষ -7:
    পক্ষ -8:
    পক্ষ -9:
    পক্ষ -10:
    পক্ষ -11:
    পক্ষ -12:
    পক্ষ -13:
    পক্ষ -14:
    পক্ষ -15:
    পক্ষ -16:
      tmp = (শূন্য*)(স্তূপ + FIRST_ARG_SLOT);
      memcpy((শূন্য*)tmp, &u, cif->rtype->size);
      ভঙ্গ;

    পক্ষ FFI_TYPE_VOID:
      ভঙ্গ;

    স্ৱভাৱ:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      ভঙ্গ;
    }
  ঘূৰাই FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

বাহ্য শূন্য ffi_closure_pa64(শূন্য);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
		      শূন্য *user_data,
		      শূন্য *codeloc)
{
  /* The layout of a function descriptor.  */
  সংরচনা pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 code_pointer;
    UINT64 gp;
  };

  সংরচনা ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 code_pointer;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  সংরচনা ffi_pa64_trampoline_struct *tramp;
  সংরচনা pa64_fd *fd;

  যদি (cif->abi != FFI_PA64)
    ঘূৰাই FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (সংরচনা pa64_fd *)((UINT64)ffi_closure_pa64);

  /* Setup trampoline.  */
  tramp = (সংরচনা ffi_pa64_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ঘূৰাই FFI_OK;
}
#endif
