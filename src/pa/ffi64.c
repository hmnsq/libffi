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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਗੇੜਾ_ਉੱਪਰ(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਹਿਲਾ_ARG_ਥਾਂ  0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_LEVEL   0

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fldw(addr, fpreg) \
  __asm__ ਅਸਥਿਰ ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fstw(fpreg, addr) \
  __asm__ ਅਸਥਿਰ ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fldd(addr, fpreg) \
  __asm__ ਅਸਥਿਰ ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fstd(fpreg, addr) \
  __asm__ ਅਸਥਿਰ ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug(lvl, x...) ਕਰੋ { ਜੇ (lvl <= DEBUG_LEVEL) { ਲਿਖੋ(x); } } ਜਦੋਂ (0)

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ ffi_struct_type(ffi_type *t)
{
  ਪੂਰਨ sz = t->ਮਾਪ;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  ਜੇ (sz <= 16)
    ਵਾਪਸ -sz;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ FFI_TYPE_STRUCT;
}

/* PA has a downward growing stack, which looks like this.  Stack
   arguments are offset from the argument ponter (AP) in r29.

ਵਿੱਥ
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

ਖਾਲੀ ffi_prep_ਦਲੀਲਾਂ_pa64(UINT64 *ਢੇਰ, extended_cif *ecif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ)
{
  ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਰਜਿਸਟਰ ffi_type **p_arg;
  ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਥਾਂ = ਪਹਿਲਾ_ARG_ਥਾਂ;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ਢੇਰ,
	ecif, ਬਾਈਟਾਂ);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  ਲਈ (i = 0; i < ecif->cif->nargs; i++)
    {
      ਪੂਰਨ type = (*p_arg)->type;

      len = (*p_arg)->ਮਾਪ;

      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	  *(SINT64 *)(ਢੇਰ + ਥਾਂ) = *(SINT8 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT8:
	  *(UINT64 *)(ਢੇਰ + ਥਾਂ) = *(UINT8 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT16:
	  *(SINT64 *)(ਢੇਰ + ਥਾਂ) = *(SINT16 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT16:
	  *(UINT64 *)(ਢੇਰ + ਥਾਂ) = *(UINT16 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT32:
	  *(SINT64 *)(ਢੇਰ + ਥਾਂ) = *(SINT32 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT32:
	  *(UINT64 *)(ਢੇਰ + ਥਾਂ) = *(UINT32 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		ਥਾਂ);
	  *(UINT64 *)(ਢੇਰ + ਥਾਂ) = *(UINT64 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", ਥਾਂ);
	  *(UINT64 *)(ਢੇਰ + ਥਾਂ) = *(UINT32 *)(*p_argv);
	  ਚੋਣ (ਥਾਂ - ਪਹਿਲਾ_ARG_ਥਾਂ)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    ਹਾਲਤ 0: fldw(ਢੇਰ + ਥਾਂ, fr4); ਰੋਕੋ;
	    ਹਾਲਤ 1: fldw(ਢੇਰ + ਥਾਂ, fr5); ਰੋਕੋ;
	    ਹਾਲਤ 2: fldw(ਢੇਰ + ਥਾਂ, fr6); ਰੋਕੋ;
	    ਹਾਲਤ 3: fldw(ਢੇਰ + ਥਾਂ, fr7); ਰੋਕੋ;
	    ਹਾਲਤ 4: fldw(ਢੇਰ + ਥਾਂ, fr8); ਰੋਕੋ;
	    ਹਾਲਤ 5: fldw(ਢੇਰ + ਥਾਂ, fr9); ਰੋਕੋ;
	    ਹਾਲਤ 6: fldw(ਢੇਰ + ਥਾਂ, fr10); ਰੋਕੋ;
	    ਹਾਲਤ 7: fldw(ਢੇਰ + ਥਾਂ, fr11); ਰੋਕੋ;
	    }
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", ਥਾਂ);
	  *(UINT64 *)(ਢੇਰ + ਥਾਂ) = *(UINT64 *)(*p_argv);
	  ਚੋਣ (ਥਾਂ - ਪਹਿਲਾ_ARG_ਥਾਂ)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    ਹਾਲਤ 0: fldd(ਢੇਰ + ਥਾਂ, fr4); ਰੋਕੋ;
	    ਹਾਲਤ 1: fldd(ਢੇਰ + ਥਾਂ, fr5); ਰੋਕੋ;
	    ਹਾਲਤ 2: fldd(ਢੇਰ + ਥਾਂ, fr6); ਰੋਕੋ;
	    ਹਾਲਤ 3: fldd(ਢੇਰ + ਥਾਂ, fr7); ਰੋਕੋ;
	    ਹਾਲਤ 4: fldd(ਢੇਰ + ਥਾਂ, fr8); ਰੋਕੋ;
	    ਹਾਲਤ 5: fldd(ਢੇਰ + ਥਾਂ, fr9); ਰੋਕੋ;
	    ਹਾਲਤ 6: fldd(ਢੇਰ + ਥਾਂ, fr10); ਰੋਕੋ;
	    ਹਾਲਤ 7: fldd(ਢੇਰ + ਥਾਂ, fr11); ਰੋਕੋ;
	    }
	  ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA64_HPUX
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  ਥਾਂ += (ਥਾਂ & 1);
	  *(UINT64 *)(ਢੇਰ + ਥਾਂ) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(ਢੇਰ + ਥਾਂ + 1) = *(UINT64 *)(*p_argv + 8);
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  ਜੇ (len > 8)
	    ਥਾਂ += (ਥਾਂ & 1);
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਅੱਖਰ *)(ਢੇਰ + ਥਾਂ), (ਅੱਖਰ *)*p_argv, len);
	  ਰੋਕੋ;

	ਮੂਲ:
	  FFI_ASSERT(0);
	}

      ਥਾਂ += ਗੇੜਾ_ਉੱਪਰ (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(ਥਾਂ * 8 <= ਬਾਈਟਾਂ);

  ਵਾਪਸ;
}

ਸਥਿਰ ਖਾਲੀ ffi_ਮਾਪ_ਢੇਰ_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  ਪੂਰਨ i;
  ਪੂਰਨ z = 0; /* # stack slots */

  ਲਈ (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      ਪੂਰਨ type = (*ptr)->type;
      ਪੂਰਨ ਮਾਪ = (*ptr)->ਮਾਪ;

      ਚੋਣ (type)
	{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA64_HPUX
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਜੇ (ਮਾਪ > 8)
	    z += (z & 1);
	  z += ਗੇੜਾ_ਉੱਪਰ (ਮਾਪ, 8) >> 3;
	  ਰੋਕੋ;

	ਮੂਲ: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  ਜੇ (z <= 8)
    z = 8;
  ਨਹੀਂਤਾਂ
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->ਬਾਈਟਾਂ = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->ਬਾਈਟਾਂ);
}

/* Perform machine dependent cif processing.  */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_COMPLEX:
    ਹਾਲਤ FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      ਰੋਕੋ;

    ਮੂਲ:
      cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ) cif->rtype->type;
      ਰੋਕੋ;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_PA64:
      ffi_ਮਾਪ_ਢੇਰ_pa64(cif);
      ਰੋਕੋ;

    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_pa64(ਖਾਲੀ (*)(UINT64 *, extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ),
			  extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *,
			  ਖਾਲੀ (*fn)(ਖਾਲੀ));

ਖਾਲੀ ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ਜੇ (rvalue == ਨਿਰਾ
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->ਮਾਪ > 16)
    ecif.rvalue = alloca(ਗੇੜਾ_ਉੱਪਰ (cif->rtype->ਮਾਪ, 16));
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;


  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->ਬਾਈਟਾਂ, cif->flags, ecif.rvalue, (ਖਾਲੀ *)fn);
      ffi_call_pa64(ffi_prep_ਦਲੀਲਾਂ_pa64, &ecif, cif->ਬਾਈਟਾਂ,
		     cif->flags, ecif.rvalue, fn);
      ਰੋਕੋ;

    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }
}

#ਜੇ FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_ਸਥਿਤੀ ffi_ਘੇਰਾ_inner_pa64(ffi_ਘੇਰਾ *ਘੇਰਾ, UINT64 *ਢੇਰ)
{
  ffi_cif *cif;
  ਖਾਲੀ **avalue;
  ਖਾਲੀ *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  ਸੰਘ { ਲੰਮਾ ਦੁੱਗਣਾ rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  ਅੱਖਰ *tmp;
  ਪੂਰਨ i, avn;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਥਾਂ = ਪਹਿਲਾ_ARG_ਥਾਂ;
  ਰਜਿਸਟਰ UINT64 r28 __asm__("r28");

  cif = ਘੇਰਾ->cif;

  /* If returning via structure, callee will write to our pointer.  */
  ਜੇ (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (ਖਾਲੀ *)r28;
  ਨਹੀਂਤਾਂ
    rvalue = &u;

  avalue = (ਖਾਲੀ **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  ਲਈ (i = 0; i < avn; i++)
    {
      ਪੂਰਨ type = (*p_arg)->type;

      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
	  avalue[i] = (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ) + 7;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	  avalue[i] = (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ) + 6;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	  avalue[i] = (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ) + 4;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	  avalue[i] = (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  ਚੋਣ (ਥਾਂ + ਪਹਿਲਾ_ARG_ਥਾਂ)
	    {
	    ਹਾਲਤ 0: fstw(fr4, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 1: fstw(fr5, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 2: fstw(fr6, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 3: fstw(fr7, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 4: fstw(fr8, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 5: fstw(fr9, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 6: fstw(fr10, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 7: fstw(fr11, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    }
	  avalue[i] = (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ) + 4;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  ਚੋਣ (ਥਾਂ + ਪਹਿਲਾ_ARG_ਥਾਂ)
	    {
	    ਹਾਲਤ 0: fstd(fr4, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 1: fstd(fr5, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 2: fstd(fr6, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 3: fstd(fr7, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 4: fstd(fr8, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 5: fstd(fr9, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 6: fstd(fr10, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 7: fstd(fr11, (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ)); ਰੋਕੋ;
	    }
	  avalue[i] = (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ);
	  ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA64_HPUX
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  ਥਾਂ += (ਥਾਂ & 1);
	  avalue[i] = (ਖਾਲੀ *)(ਢੇਰ + ਥਾਂ);
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  ਜੇ((*p_arg)->ਮਾਪ > 8)
	    ਥਾਂ += (ਥਾਂ & 1);
	  avalue[i] = (ਖਾਲੀ *) (ਢੇਰ + ਥਾਂ);
	  ਰੋਕੋ;

	ਮੂਲ:
	  FFI_ASSERT(0);
	}

      ਥਾਂ += (ਗੇੜਾ_ਉੱਪਰ ((*p_arg)->ਮਾਪ, 8) >> 3);
      p_arg++;
    }

  /* Invoke the closure.  */
  (ਘੇਰਾ->fun) (cif, rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);

  debug(3, "after calling function, ret[0] = %16lx, ret[1] = %16lx\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  ਚੋਣ (cif->flags)
    {
    ਹਾਲਤ FFI_TYPE_UINT8:
      *(ਢੇਰ + ਪਹਿਲਾ_ARG_ਥਾਂ) = (UINT8)u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT8:
      *(ਢੇਰ + ਪਹਿਲਾ_ARG_ਥਾਂ) = (SINT8)u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16:
      *(ਢੇਰ + ਪਹਿਲਾ_ARG_ਥਾਂ) = (UINT16)u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
      *(ਢੇਰ + ਪਹਿਲਾ_ARG_ਥਾਂ) = (SINT16)u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
      *(ਢੇਰ + ਪਹਿਲਾ_ARG_ਥਾਂ) = (SINT32)u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT32:
      *(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ) = (UINT32)u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      *(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ) = u.ret[0];
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      *(ਢੇਰ + ਪਹਿਲਾ_ARG_ਥਾਂ) = u.ret[0];
      *(ਢੇਰ + ਪਹਿਲਾ_ARG_ਥਾਂ + 1) = u.ret[1];
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      ਰੋਕੋ;

    ਹਾਲਤ -1:
    ਹਾਲਤ -2:
    ਹਾਲਤ -3:
    ਹਾਲਤ -4:
    ਹਾਲਤ -5:
    ਹਾਲਤ -6:
    ਹਾਲਤ -7:
    ਹਾਲਤ -8:
    ਹਾਲਤ -9:
    ਹਾਲਤ -10:
    ਹਾਲਤ -11:
    ਹਾਲਤ -12:
    ਹਾਲਤ -13:
    ਹਾਲਤ -14:
    ਹਾਲਤ -15:
    ਹਾਲਤ -16:
      tmp = (ਖਾਲੀ*)(ਢੇਰ + ਪਹਿਲਾ_ARG_ਥਾਂ);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ*)tmp, &u, cif->rtype->ਮਾਪ);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_VOID:
      ਰੋਕੋ;

    ਮੂਲ:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }
  ਵਾਪਸ FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_pa64(ਖਾਲੀ);

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  /* The layout of a function descriptor.  */
  ਬਣਤਰ pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 ਕੋਡ_ਸੂਚਕ;
    UINT64 gp;
  };

  ਬਣਤਰ ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 ਕੋਡ_ਸੂਚਕ;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  ਬਣਤਰ ffi_pa64_trampoline_struct *tramp;
  ਬਣਤਰ pa64_fd *fd;

  ਜੇ (cif->abi != FFI_PA64)
    ਵਾਪਸ FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (ਬਣਤਰ pa64_fd *)((UINT64)ffi_ਘੇਰਾ_pa64);

  /* Setup trampoline.  */
  tramp = (ਬਣਤਰ ffi_pa64_trampoline_struct *)ਘੇਰਾ->tramp;
  tramp->ਕੋਡ_ਸੂਚਕ = fd->ਕੋਡ_ਸੂਚਕ;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  ਘੇਰਾ->cif  = cif;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  ਘੇਰਾ->fun  = fun;

  ਵਾਪਸ FFI_OK;
}
#ਜੇ_ਅੰਤ
