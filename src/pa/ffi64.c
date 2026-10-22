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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ವ್ಯಾಖ್ಯಾನಿಸು ಸುತ್ತು_ಮೇಲೆ(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#ವ್ಯಾಖ್ಯಾನಿಸು ಮೊದಲ_ARG_ಸ್ಥಾನ  0
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_LEVEL   0

#ವ್ಯಾಖ್ಯಾನಿಸು fldw(addr, fpreg) \
  __asm__ ಅಸ್ಥಿರ ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#ವ್ಯಾಖ್ಯಾನಿಸು fstw(fpreg, addr) \
  __asm__ ಅಸ್ಥಿರ ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#ವ್ಯಾಖ್ಯಾನಿಸು fldd(addr, fpreg) \
  __asm__ ಅಸ್ಥಿರ ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#ವ್ಯಾಖ್ಯಾನಿಸು fstd(fpreg, addr) \
  __asm__ ಅಸ್ಥಿರ ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#ವ್ಯಾಖ್ಯಾನಿಸು debug(lvl, x...) ಮಾಡು { ಆದರೆ (lvl <= DEBUG_LEVEL) { ಬರೆ(x); } } ಆಗ (0)

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ffi_struct_type(ffi_type *t)
{
  ಪೂರ್ಣಾಂಕ sz = t->ಗಾತ್ರ_2;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  ಆದರೆ (sz <= 16)
    ಹಿಂತಿರುಗಿ -sz;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ FFI_TYPE_STRUCT;
}

/* PA has a downward growing stack, which looks like this.  Stack
   arguments are offset from the argument ponter (AP) in r29.

ಸ್ಥಾನಾಂತರ
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

ಶೂನ್ಯ ffi_prep_ವಾದಗಳು_pa64(UINT64 *ರಾಶಿ, extended_cif *ecif, ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು)
{
  ನೋಂದಣಿ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ನೋಂದಣಿ ffi_type **p_arg;
  ನೋಂದಣಿ ಶೂನ್ಯ **p_argv;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸ್ಥಾನ = ಮೊದಲ_ARG_ಸ್ಥಾನ;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ರಾಶಿ,
	ecif, ಬೈಟುಗಳು);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  ಗಾಗಿ (i = 0; i < ecif->cif->nargs; i++)
    {
      ಪೂರ್ಣಾಂಕ type = (*p_arg)->type;

      len = (*p_arg)->ಗಾತ್ರ_2;

      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  *(SINT64 *)(ರಾಶಿ + ಸ್ಥಾನ) = *(SINT8 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  *(UINT64 *)(ರಾಶಿ + ಸ್ಥಾನ) = *(UINT8 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  *(SINT64 *)(ರಾಶಿ + ಸ್ಥಾನ) = *(SINT16 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  *(UINT64 *)(ರಾಶಿ + ಸ್ಥಾನ) = *(UINT16 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	  *(SINT64 *)(ರಾಶಿ + ಸ್ಥಾನ) = *(SINT32 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  *(UINT64 *)(ರಾಶಿ + ಸ್ಥಾನ) = *(UINT32 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		ಸ್ಥಾನ);
	  *(UINT64 *)(ರಾಶಿ + ಸ್ಥಾನ) = *(UINT64 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", ಸ್ಥಾನ);
	  *(UINT64 *)(ರಾಶಿ + ಸ್ಥಾನ) = *(UINT32 *)(*p_argv);
	  ಆಯ್ಕೆ (ಸ್ಥಾನ - ಮೊದಲ_ARG_ಸ್ಥಾನ)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    ಸ್ಥಿತಿ 0: fldw(ರಾಶಿ + ಸ್ಥಾನ, fr4); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 1: fldw(ರಾಶಿ + ಸ್ಥಾನ, fr5); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 2: fldw(ರಾಶಿ + ಸ್ಥಾನ, fr6); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 3: fldw(ರಾಶಿ + ಸ್ಥಾನ, fr7); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 4: fldw(ರಾಶಿ + ಸ್ಥಾನ, fr8); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 5: fldw(ರಾಶಿ + ಸ್ಥಾನ, fr9); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 6: fldw(ರಾಶಿ + ಸ್ಥಾನ, fr10); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 7: fldw(ರಾಶಿ + ಸ್ಥಾನ, fr11); ನಿಲ್ಲಿಸು;
	    }
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", ಸ್ಥಾನ);
	  *(UINT64 *)(ರಾಶಿ + ಸ್ಥಾನ) = *(UINT64 *)(*p_argv);
	  ಆಯ್ಕೆ (ಸ್ಥಾನ - ಮೊದಲ_ARG_ಸ್ಥಾನ)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    ಸ್ಥಿತಿ 0: fldd(ರಾಶಿ + ಸ್ಥಾನ, fr4); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 1: fldd(ರಾಶಿ + ಸ್ಥಾನ, fr5); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 2: fldd(ರಾಶಿ + ಸ್ಥಾನ, fr6); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 3: fldd(ರಾಶಿ + ಸ್ಥಾನ, fr7); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 4: fldd(ರಾಶಿ + ಸ್ಥಾನ, fr8); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 5: fldd(ರಾಶಿ + ಸ್ಥಾನ, fr9); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 6: fldd(ರಾಶಿ + ಸ್ಥಾನ, fr10); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 7: fldd(ರಾಶಿ + ಸ್ಥಾನ, fr11); ನಿಲ್ಲಿಸು;
	    }
	  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA64_HPUX
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  ಸ್ಥಾನ += (ಸ್ಥಾನ & 1);
	  *(UINT64 *)(ರಾಶಿ + ಸ್ಥಾನ) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(ರಾಶಿ + ಸ್ಥಾನ + 1) = *(UINT64 *)(*p_argv + 8);
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  ಆದರೆ (len > 8)
	    ಸ್ಥಾನ += (ಸ್ಥಾನ & 1);
	  ಸ್ಮರಣೆ_ನಕಲು((ಅಕ್ಷರ *)(ರಾಶಿ + ಸ್ಥಾನ), (ಅಕ್ಷರ *)*p_argv, len);
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  FFI_ASSERT(0);
	}

      ಸ್ಥಾನ += ಸುತ್ತು_ಮೇಲೆ (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(ಸ್ಥಾನ * 8 <= ಬೈಟುಗಳು);

  ಹಿಂತಿರುಗಿ;
}

ಸ್ಥಿರ ಶೂನ್ಯ ffi_ಗಾತ್ರ_ರಾಶಿ_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ z = 0; /* # stack slots */

  ಗಾಗಿ (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      ಪೂರ್ಣಾಂಕ type = (*ptr)->type;
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = (*ptr)->ಗಾತ್ರ_2;

      ಆಯ್ಕೆ (type)
	{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA64_HPUX
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಆದರೆ (ಗಾತ್ರ_2 > 8)
	    z += (z & 1);
	  z += ಸುತ್ತು_ಮೇಲೆ (ಗಾತ್ರ_2, 8) >> 3;
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  ಆದರೆ (z <= 8)
    z = 8;
  ಇಲ್ಲದಿದ್ದರೆ
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->ಬೈಟುಗಳು = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->ಬೈಟುಗಳು);
}

/* Perform machine dependent cif processing.  */
ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      cif->flags = (ಚಿಹ್ನೆರಹಿತ) cif->rtype->type;
      ನಿಲ್ಲಿಸು;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_PA64:
      ffi_ಗಾತ್ರ_ರಾಶಿ_pa64(cif);
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_pa64(ಶೂನ್ಯ (*)(UINT64 *, extended_cif *, ಚಿಹ್ನೆರಹಿತ),
			  extended_cif *, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ *,
			  ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ಆದರೆ (rvalue == ಖಾಲಿ
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->ಗಾತ್ರ_2 > 16)
    ecif.rvalue = alloca(ಸುತ್ತು_ಮೇಲೆ (cif->rtype->ಗಾತ್ರ_2, 16));
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;


  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->ಬೈಟುಗಳು, cif->flags, ecif.rvalue, (ಶೂನ್ಯ *)fn);
      ffi_call_pa64(ffi_prep_ವಾದಗಳು_pa64, &ecif, cif->ಬೈಟುಗಳು,
		     cif->flags, ecif.rvalue, fn);
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }
}

#ಆದರೆ FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_ಸ್ಥಿತಿ ffi_ಆವರಣ_inner_pa64(ffi_ಆವರಣ *ಆವರಣ, UINT64 *ರಾಶಿ)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **avalue;
  ಶೂನ್ಯ *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  ಒಕ್ಕೂಟ { ದೀರ್ಘ ದ್ವಿಗುಣ rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  ಅಕ್ಷರ *tmp;
  ಪೂರ್ಣಾಂಕ i, avn;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸ್ಥಾನ = ಮೊದಲ_ARG_ಸ್ಥಾನ;
  ನೋಂದಣಿ UINT64 r28 __asm__("r28");

  cif = ಆವರಣ->cif;

  /* If returning via structure, callee will write to our pointer.  */
  ಆದರೆ (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (ಶೂನ್ಯ *)r28;
  ಇಲ್ಲದಿದ್ದರೆ
    rvalue = &u;

  avalue = (ಶೂನ್ಯ **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  ಗಾಗಿ (i = 0; i < avn; i++)
    {
      ಪೂರ್ಣಾಂಕ type = (*p_arg)->type;

      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ) + 7;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ) + 6;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ) + 4;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  ಆಯ್ಕೆ (ಸ್ಥಾನ + ಮೊದಲ_ARG_ಸ್ಥಾನ)
	    {
	    ಸ್ಥಿತಿ 0: fstw(fr4, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 1: fstw(fr5, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 2: fstw(fr6, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 3: fstw(fr7, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 4: fstw(fr8, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 5: fstw(fr9, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 6: fstw(fr10, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 7: fstw(fr11, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    }
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ) + 4;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  ಆಯ್ಕೆ (ಸ್ಥಾನ + ಮೊದಲ_ARG_ಸ್ಥಾನ)
	    {
	    ಸ್ಥಿತಿ 0: fstd(fr4, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 1: fstd(fr5, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 2: fstd(fr6, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 3: fstd(fr7, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 4: fstd(fr8, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 5: fstd(fr9, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 6: fstd(fr10, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 7: fstd(fr11, (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    }
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ);
	  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA64_HPUX
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  ಸ್ಥಾನ += (ಸ್ಥಾನ & 1);
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + ಸ್ಥಾನ);
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  ಆದರೆ((*p_arg)->ಗಾತ್ರ_2 > 8)
	    ಸ್ಥಾನ += (ಸ್ಥಾನ & 1);
	  avalue[i] = (ಶೂನ್ಯ *) (ರಾಶಿ + ಸ್ಥಾನ);
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  FFI_ASSERT(0);
	}

      ಸ್ಥಾನ += (ಸುತ್ತು_ಮೇಲೆ ((*p_arg)->ಗಾತ್ರ_2, 8) >> 3);
      p_arg++;
    }

  /* Invoke the closure.  */
  (ಆವರಣ->fun) (cif, rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);

  debug(3, "after calling function, ret[0] = %16lx, ret[1] = %16lx\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  ಆಯ್ಕೆ (cif->flags)
    {
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      *(ರಾಶಿ + ಮೊದಲ_ARG_ಸ್ಥಾನ) = (UINT8)u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      *(ರಾಶಿ + ಮೊದಲ_ARG_ಸ್ಥಾನ) = (SINT8)u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      *(ರಾಶಿ + ಮೊದಲ_ARG_ಸ್ಥಾನ) = (UINT16)u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      *(ರಾಶಿ + ಮೊದಲ_ARG_ಸ್ಥಾನ) = (SINT16)u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      *(ರಾಶಿ + ಮೊದಲ_ARG_ಸ್ಥಾನ) = (SINT32)u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      *(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ) = (UINT32)u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      *(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ) = u.ret[0];
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      *(ರಾಶಿ + ಮೊದಲ_ARG_ಸ್ಥಾನ) = u.ret[0];
      *(ರಾಶಿ + ಮೊದಲ_ARG_ಸ್ಥಾನ + 1) = u.ret[1];
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ -1:
    ಸ್ಥಿತಿ -2:
    ಸ್ಥಿತಿ -3:
    ಸ್ಥಿತಿ -4:
    ಸ್ಥಿತಿ -5:
    ಸ್ಥಿತಿ -6:
    ಸ್ಥಿತಿ -7:
    ಸ್ಥಿತಿ -8:
    ಸ್ಥಿತಿ -9:
    ಸ್ಥಿತಿ -10:
    ಸ್ಥಿತಿ -11:
    ಸ್ಥಿತಿ -12:
    ಸ್ಥಿತಿ -13:
    ಸ್ಥಿತಿ -14:
    ಸ್ಥಿತಿ -15:
    ಸ್ಥಿತಿ -16:
      tmp = (ಶೂನ್ಯ*)(ರಾಶಿ + ಮೊದಲ_ARG_ಸ್ಥಾನ);
      ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ*)tmp, &u, cif->rtype->ಗಾತ್ರ_2);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }
  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_pa64(ಶೂನ್ಯ);

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  /* The layout of a function descriptor.  */
  ರಚನೆ pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 ಸಂಕೇತ_ಸೂಚಕ;
    UINT64 gp;
  };

  ರಚನೆ ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 ಸಂಕೇತ_ಸೂಚಕ;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  ರಚನೆ ffi_pa64_trampoline_struct *tramp;
  ರಚನೆ pa64_fd *fd;

  ಆದರೆ (cif->abi != FFI_PA64)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (ರಚನೆ pa64_fd *)((UINT64)ffi_ಆವರಣ_pa64);

  /* Setup trampoline.  */
  tramp = (ರಚನೆ ffi_pa64_trampoline_struct *)ಆವರಣ->tramp;
  tramp->ಸಂಕೇತ_ಸೂಚಕ = fd->ಸಂಕೇತ_ಸೂಚಕ;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  ಆವರಣ->cif  = cif;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
  ಆವರಣ->fun  = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}
#ಆದರೆ_ಅಂತ್ಯ
