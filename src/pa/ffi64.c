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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdio.h>

#define ROUND_UP(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#define FIRST_ARG_SLOT  0
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ நிலையற்ற ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ நிலையற்ற ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ நிலையற்ற ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ நிலையற்ற ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) செய் { எனில் (lvl <= DEBUG_LEVEL) { printf(x); } } போது (0)

நிலையான உள்வரி முழு ffi_struct_type(ffi_type *t)
{
  முழு sz = t->size;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  எனில் (sz <= 16)
    திரும்பு -sz;
  இல்லையெனில்
    திரும்பு FFI_TYPE_STRUCT;
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

வெறுமை ffi_prep_args_pa64(UINT64 *அடுக்கு, extended_cif *ecif, unsigned bytes)
{
  பதிவு unsigned முழு i;
  பதிவு ffi_type **p_arg;
  பதிவு வெறுமை **p_argv;
  unsigned முழு slot = FIRST_ARG_SLOT;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, அடுக்கு,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  க்கு (i = 0; i < ecif->cif->nargs; i++)
    {
      முழு type = (*p_arg)->type;

      len = (*p_arg)->size;

      தேர்வு (type)
	{
	நிலை FFI_TYPE_SINT8:
	  *(SINT64 *)(அடுக்கு + slot) = *(SINT8 *)(*p_argv);
	  நிறுத்து;

	நிலை FFI_TYPE_UINT8:
	  *(UINT64 *)(அடுக்கு + slot) = *(UINT8 *)(*p_argv);
	  நிறுத்து;

	நிலை FFI_TYPE_SINT16:
	  *(SINT64 *)(அடுக்கு + slot) = *(SINT16 *)(*p_argv);
	  நிறுத்து;

	நிலை FFI_TYPE_UINT16:
	  *(UINT64 *)(அடுக்கு + slot) = *(UINT16 *)(*p_argv);
	  நிறுத்து;

	நிலை FFI_TYPE_SINT32:
	  *(SINT64 *)(அடுக்கு + slot) = *(SINT32 *)(*p_argv);
	  நிறுத்து;

	நிலை FFI_TYPE_UINT32:
	  *(UINT64 *)(அடுக்கு + slot) = *(UINT32 *)(*p_argv);
	  நிறுத்து;

	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_POINTER:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		slot);
	  *(UINT64 *)(அடுக்கு + slot) = *(UINT64 *)(*p_argv);
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT64 *)(அடுக்கு + slot) = *(UINT32 *)(*p_argv);
	  தேர்வு (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    நிலை 0: fldw(அடுக்கு + slot, fr4); நிறுத்து;
	    நிலை 1: fldw(அடுக்கு + slot, fr5); நிறுத்து;
	    நிலை 2: fldw(அடுக்கு + slot, fr6); நிறுத்து;
	    நிலை 3: fldw(அடுக்கு + slot, fr7); நிறுத்து;
	    நிலை 4: fldw(அடுக்கு + slot, fr8); நிறுத்து;
	    நிலை 5: fldw(அடுக்கு + slot, fr9); நிறுத்து;
	    நிலை 6: fldw(அடுக்கு + slot, fr10); நிறுத்து;
	    நிலை 7: fldw(அடுக்கு + slot, fr11); நிறுத்து;
	    }
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(அடுக்கு + slot) = *(UINT64 *)(*p_argv);
	  தேர்வு (slot - FIRST_ARG_SLOT)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    நிலை 0: fldd(அடுக்கு + slot, fr4); நிறுத்து;
	    நிலை 1: fldd(அடுக்கு + slot, fr5); நிறுத்து;
	    நிலை 2: fldd(அடுக்கு + slot, fr6); நிறுத்து;
	    நிலை 3: fldd(அடுக்கு + slot, fr7); நிறுத்து;
	    நிலை 4: fldd(அடுக்கு + slot, fr8); நிறுத்து;
	    நிலை 5: fldd(அடுக்கு + slot, fr9); நிறுத்து;
	    நிலை 6: fldd(அடுக்கு + slot, fr10); நிறுத்து;
	    நிலை 7: fldd(அடுக்கு + slot, fr11); நிறுத்து;
	    }
	  நிறுத்து;

#ifdef PA64_HPUX
	நிலை FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  slot += (slot & 1);
	  *(UINT64 *)(அடுக்கு + slot) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(அடுக்கு + slot + 1) = *(UINT64 *)(*p_argv + 8);
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  எனில் (len > 8)
	    slot += (slot & 1);
	  memcpy((எழுத்து *)(அடுக்கு + slot), (எழுத்து *)*p_argv, len);
	  நிறுத்து;

	இயல்பு:
	  FFI_ASSERT(0);
	}

      slot += ROUND_UP (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(slot * 8 <= bytes);

  திரும்பு;
}

நிலையான வெறுமை ffi_size_அடுக்கு_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  முழு i;
  முழு z = 0; /* # stack slots */

  க்கு (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      முழு type = (*ptr)->type;
      முழு size = (*ptr)->size;

      தேர்வு (type)
	{
#ifdef PA64_HPUX
	நிலை FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_STRUCT:
	  எனில் (size > 8)
	    z += (z & 1);
	  z += ROUND_UP (size, 8) >> 3;
	  நிறுத்து;

	இயல்பு: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  எனில் (z <= 8)
    z = 8;
  இல்லையெனில்
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->bytes = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_COMPLEX:
    நிலை FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      நிறுத்து;

    இயல்பு:
      cif->flags = (unsigned) cif->rtype->type;
      நிறுத்து;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  தேர்வு (cif->abi)
    {
    நிலை FFI_PA64:
      ffi_size_அடுக்கு_pa64(cif);
      நிறுத்து;

    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
    }

  திரும்பு FFI_OK;
}

வெளி வெறுமை ffi_call_pa64(வெறுமை (*)(UINT64 *, extended_cif *, unsigned),
			  extended_cif *, unsigned, unsigned, unsigned *,
			  வெறுமை (*fn)(வெறுமை));

வெறுமை ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  எனில் (rvalue == வெற்று
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->size > 16)
    ecif.rvalue = alloca(ROUND_UP (cif->rtype->size, 16));
  இல்லையெனில்
    ecif.rvalue = rvalue;


  தேர்வு (cif->abi)
    {
    நிலை FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (வெறுமை *)fn);
      ffi_call_pa64(ffi_prep_args_pa64, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      நிறுத்து;

    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
    }
}

#எனில் FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_status ffi_closure_inner_pa64(ffi_closure *closure, UINT64 *அடுக்கு)
{
  ffi_cif *cif;
  வெறுமை **avalue;
  வெறுமை *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  ஒன்றிப்பு { long இரட்டை rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  எழுத்து *tmp;
  முழு i, avn;
  unsigned முழு slot = FIRST_ARG_SLOT;
  பதிவு UINT64 r28 __asm__("r28");

  cif = closure->cif;

  /* If returning via structure, callee will write to our pointer.  */
  எனில் (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (வெறுமை *)r28;
  இல்லையெனில்
    rvalue = &u;

  avalue = (வெறுமை **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  க்கு (i = 0; i < avn; i++)
    {
      முழு type = (*p_arg)->type;

      தேர்வு (type)
	{
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT8:
	  avalue[i] = (வெறுமை *)(அடுக்கு + slot) + 7;
	  நிறுத்து;

	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT16:
	  avalue[i] = (வெறுமை *)(அடுக்கு + slot) + 6;
	  நிறுத்து;

	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT32:
	  avalue[i] = (வெறுமை *)(அடுக்கு + slot) + 4;
	  நிறுத்து;

	நிலை FFI_TYPE_POINTER:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
	  avalue[i] = (வெறுமை *)(அடுக்கு + slot);
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  தேர்வு (slot + FIRST_ARG_SLOT)
	    {
	    நிலை 0: fstw(fr4, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 1: fstw(fr5, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 2: fstw(fr6, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 3: fstw(fr7, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 4: fstw(fr8, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 5: fstw(fr9, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 6: fstw(fr10, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 7: fstw(fr11, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    }
	  avalue[i] = (வெறுமை *)(அடுக்கு + slot) + 4;
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  தேர்வு (slot + FIRST_ARG_SLOT)
	    {
	    நிலை 0: fstd(fr4, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 1: fstd(fr5, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 2: fstd(fr6, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 3: fstd(fr7, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 4: fstd(fr8, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 5: fstd(fr9, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 6: fstd(fr10, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    நிலை 7: fstd(fr11, (வெறுமை *)(அடுக்கு + slot)); நிறுத்து;
	    }
	  avalue[i] = (வெறுமை *)(அடுக்கு + slot);
	  நிறுத்து;

#ifdef PA64_HPUX
	நிலை FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  slot += (slot & 1);
	  avalue[i] = (வெறுமை *)(அடுக்கு + slot);
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  எனில்((*p_arg)->size > 8)
	    slot += (slot & 1);
	  avalue[i] = (வெறுமை *) (அடுக்கு + slot);
	  நிறுத்து;

	இயல்பு:
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
  தேர்வு (cif->flags)
    {
    நிலை FFI_TYPE_UINT8:
      *(அடுக்கு + FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      நிறுத்து;
    நிலை FFI_TYPE_SINT8:
      *(அடுக்கு + FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      நிறுத்து;
    நிலை FFI_TYPE_UINT16:
      *(அடுக்கு + FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      நிறுத்து;
    நிலை FFI_TYPE_SINT16:
      *(அடுக்கு + FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      நிறுத்து;
    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_SINT32:
      *(அடுக்கு + FIRST_ARG_SLOT) = (SINT32)u.ret[0];
      நிறுத்து;
    நிலை FFI_TYPE_UINT32:
      *(அடுக்கு - FIRST_ARG_SLOT) = (UINT32)u.ret[0];
      நிறுத்து;
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_POINTER:
      *(அடுக்கு - FIRST_ARG_SLOT) = u.ret[0];
      நிறுத்து;

    நிலை FFI_TYPE_LONGDOUBLE:
      *(அடுக்கு + FIRST_ARG_SLOT) = u.ret[0];
      *(அடுக்கு + FIRST_ARG_SLOT + 1) = u.ret[1];
      நிறுத்து;

    நிலை FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      நிறுத்து;

    நிலை FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      நிறுத்து;

    நிலை -1:
    நிலை -2:
    நிலை -3:
    நிலை -4:
    நிலை -5:
    நிலை -6:
    நிலை -7:
    நிலை -8:
    நிலை -9:
    நிலை -10:
    நிலை -11:
    நிலை -12:
    நிலை -13:
    நிலை -14:
    நிலை -15:
    நிலை -16:
      tmp = (வெறுமை*)(அடுக்கு + FIRST_ARG_SLOT);
      memcpy((வெறுமை*)tmp, &u, cif->rtype->size);
      நிறுத்து;

    நிலை FFI_TYPE_VOID:
      நிறுத்து;

    இயல்பு:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      நிறுத்து;
    }
  திரும்பு FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

வெளி வெறுமை ffi_closure_pa64(வெறுமை);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  /* The layout of a function descriptor.  */
  கட்டமைப்பு pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 code_pointer;
    UINT64 gp;
  };

  கட்டமைப்பு ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 code_pointer;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  கட்டமைப்பு ffi_pa64_trampoline_struct *tramp;
  கட்டமைப்பு pa64_fd *fd;

  எனில் (cif->abi != FFI_PA64)
    திரும்பு FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (கட்டமைப்பு pa64_fd *)((UINT64)ffi_closure_pa64);

  /* Setup trampoline.  */
  tramp = (கட்டமைப்பு ffi_pa64_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  திரும்பு FFI_OK;
}
#endif
