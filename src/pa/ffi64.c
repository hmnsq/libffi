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

#define ROUND_UP(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#define FIRST_ARG_SLOT  0
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ ಚಞ್ಚಲ ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ ಚಞ್ಚಲ ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ ಚಞ್ಚಲ ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ ಚಞ್ಚಲ ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) ಕುರು { ಯದಿ (lvl <= DEBUG_LEVEL) { printf(x); } } ಯಾವತ್ (0)

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ ffi_struct_type(ffi_type *t)
{
  ಪೂರ್ಣಾಙ್ಕ sz = t->size;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  ಯದಿ (sz <= 16)
    ಪ್ರತ್ಯಾಗಮ -sz;
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ FFI_TYPE_STRUCT;
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

ಶೂನ್ಯ ffi_prep_args_pa64(UINT64 *ರಾಶಿ, extended_cif *ecif, unsigned bytes)
{
  ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಪಞ್ಜಿಕಾ ffi_type **p_arg;
  ಪಞ್ಜಿಕಾ ಶೂನ್ಯ **p_argv;
  unsigned ಪೂರ್ಣಾಙ್ಕ slot = FIRST_ARG_SLOT;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ರಾಶಿ,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = 0; i < ecif->cif->nargs; i++)
    {
      ಪೂರ್ಣಾಙ್ಕ type = (*p_arg)->type;

      len = (*p_arg)->size;

      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	  *(SINT64 *)(ರಾಶಿ + slot) = *(SINT8 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT8:
	  *(UINT64 *)(ರಾಶಿ + slot) = *(UINT8 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT16:
	  *(SINT64 *)(ರಾಶಿ + slot) = *(SINT16 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT16:
	  *(UINT64 *)(ರಾಶಿ + slot) = *(UINT16 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT32:
	  *(SINT64 *)(ರಾಶಿ + slot) = *(SINT32 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT32:
	  *(UINT64 *)(ರಾಶಿ + slot) = *(UINT32 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		slot);
	  *(UINT64 *)(ರಾಶಿ + slot) = *(UINT64 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT64 *)(ರಾಶಿ + slot) = *(UINT32 *)(*p_argv);
	  ವಿಕಲ್ಪ (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    ಪಕ್ಷ 0: fldw(ರಾಶಿ + slot, fr4); ಭಙ್ಗ;
	    ಪಕ್ಷ 1: fldw(ರಾಶಿ + slot, fr5); ಭಙ್ಗ;
	    ಪಕ್ಷ 2: fldw(ರಾಶಿ + slot, fr6); ಭಙ್ಗ;
	    ಪಕ್ಷ 3: fldw(ರಾಶಿ + slot, fr7); ಭಙ್ಗ;
	    ಪಕ್ಷ 4: fldw(ರಾಶಿ + slot, fr8); ಭಙ್ಗ;
	    ಪಕ್ಷ 5: fldw(ರಾಶಿ + slot, fr9); ಭಙ್ಗ;
	    ಪಕ್ಷ 6: fldw(ರಾಶಿ + slot, fr10); ಭಙ್ಗ;
	    ಪಕ್ಷ 7: fldw(ರಾಶಿ + slot, fr11); ಭಙ್ಗ;
	    }
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(ರಾಶಿ + slot) = *(UINT64 *)(*p_argv);
	  ವಿಕಲ್ಪ (slot - FIRST_ARG_SLOT)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    ಪಕ್ಷ 0: fldd(ರಾಶಿ + slot, fr4); ಭಙ್ಗ;
	    ಪಕ್ಷ 1: fldd(ರಾಶಿ + slot, fr5); ಭಙ್ಗ;
	    ಪಕ್ಷ 2: fldd(ರಾಶಿ + slot, fr6); ಭಙ್ಗ;
	    ಪಕ್ಷ 3: fldd(ರಾಶಿ + slot, fr7); ಭಙ್ಗ;
	    ಪಕ್ಷ 4: fldd(ರಾಶಿ + slot, fr8); ಭಙ್ಗ;
	    ಪಕ್ಷ 5: fldd(ರಾಶಿ + slot, fr9); ಭಙ್ಗ;
	    ಪಕ್ಷ 6: fldd(ರಾಶಿ + slot, fr10); ಭಙ್ಗ;
	    ಪಕ್ಷ 7: fldd(ರಾಶಿ + slot, fr11); ಭಙ್ಗ;
	    }
	  ಭಙ್ಗ;

#ifdef PA64_HPUX
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  slot += (slot & 1);
	  *(UINT64 *)(ರಾಶಿ + slot) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(ರಾಶಿ + slot + 1) = *(UINT64 *)(*p_argv + 8);
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  ಯದಿ (len > 8)
	    slot += (slot & 1);
	  memcpy((ವರ್ಣ *)(ರಾಶಿ + slot), (ವರ್ಣ *)*p_argv, len);
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  FFI_ASSERT(0);
	}

      slot += ROUND_UP (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(slot * 8 <= bytes);

  ಪ್ರತ್ಯಾಗಮ;
}

ಸ್ಥಿರ ಶೂನ್ಯ ffi_size_ರಾಶಿ_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ z = 0; /* # stack slots */

  ಪ್ರತಿ (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      ಪೂರ್ಣಾಙ್ಕ type = (*ptr)->type;
      ಪೂರ್ಣಾಙ್ಕ size = (*ptr)->size;

      ವಿಕಲ್ಪ (type)
	{
#ifdef PA64_HPUX
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  ಯದಿ (size > 8)
	    z += (z & 1);
	  z += ROUND_UP (size, 8) >> 3;
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  ಯದಿ (z <= 8)
    z = 8;
  ಅನ್ಯಥಾ
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->bytes = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
    ಪಕ್ಷ FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      cif->flags = (unsigned) cif->rtype->type;
      ಭಙ್ಗ;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_PA64:
      ffi_size_ರಾಶಿ_pa64(cif);
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_pa64(ಶೂನ್ಯ (*)(UINT64 *, extended_cif *, unsigned),
			  extended_cif *, unsigned, unsigned, unsigned *,
			  ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ಯದಿ (rvalue == ರಿಕ್ತ
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->size > 16)
    ecif.rvalue = alloca(ROUND_UP (cif->rtype->size, 16));
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;


  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (ಶೂನ್ಯ *)fn);
      ffi_call_pa64(ffi_prep_args_pa64, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }
}

#ಯದಿ FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_status ffi_closure_inner_pa64(ffi_closure *closure, UINT64 *ರಾಶಿ)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **avalue;
  ಶೂನ್ಯ *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  ಸಙ್ಘ { long ದ್ವಿಗುಣ rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  ವರ್ಣ *tmp;
  ಪೂರ್ಣಾಙ್ಕ i, avn;
  unsigned ಪೂರ್ಣಾಙ್ಕ slot = FIRST_ARG_SLOT;
  ಪಞ್ಜಿಕಾ UINT64 r28 __asm__("r28");

  cif = closure->cif;

  /* If returning via structure, callee will write to our pointer.  */
  ಯದಿ (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (ಶೂನ್ಯ *)r28;
  ಅನ್ಯಥಾ
    rvalue = &u;

  avalue = (ಶೂನ್ಯ **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  ಪ್ರತಿ (i = 0; i < avn; i++)
    {
      ಪೂರ್ಣಾಙ್ಕ type = (*p_arg)->type;

      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + slot) + 7;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + slot) + 6;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + slot) + 4;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_POINTER:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + slot);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  ವಿಕಲ್ಪ (slot + FIRST_ARG_SLOT)
	    {
	    ಪಕ್ಷ 0: fstw(fr4, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 1: fstw(fr5, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 2: fstw(fr6, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 3: fstw(fr7, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 4: fstw(fr8, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 5: fstw(fr9, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 6: fstw(fr10, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 7: fstw(fr11, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    }
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + slot) + 4;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  ವಿಕಲ್ಪ (slot + FIRST_ARG_SLOT)
	    {
	    ಪಕ್ಷ 0: fstd(fr4, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 1: fstd(fr5, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 2: fstd(fr6, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 3: fstd(fr7, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 4: fstd(fr8, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 5: fstd(fr9, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 6: fstd(fr10, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 7: fstd(fr11, (ಶೂನ್ಯ *)(ರಾಶಿ + slot)); ಭಙ್ಗ;
	    }
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + slot);
	  ಭಙ್ಗ;

#ifdef PA64_HPUX
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  slot += (slot & 1);
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ + slot);
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  ಯದಿ((*p_arg)->size > 8)
	    slot += (slot & 1);
	  avalue[i] = (ಶೂನ್ಯ *) (ರಾಶಿ + slot);
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
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
  ವಿಕಲ್ಪ (cif->flags)
    {
    ಪಕ್ಷ FFI_TYPE_UINT8:
      *(ರಾಶಿ + FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT8:
      *(ರಾಶಿ + FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      *(ರಾಶಿ + FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      *(ರಾಶಿ + FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_SINT32:
      *(ರಾಶಿ + FIRST_ARG_SLOT) = (SINT32)u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT32:
      *(ರಾಶಿ - FIRST_ARG_SLOT) = (UINT32)u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_POINTER:
      *(ರಾಶಿ - FIRST_ARG_SLOT) = u.ret[0];
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      *(ರಾಶಿ + FIRST_ARG_SLOT) = u.ret[0];
      *(ರಾಶಿ + FIRST_ARG_SLOT + 1) = u.ret[1];
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      ಭಙ್ಗ;

    ಪಕ್ಷ -1:
    ಪಕ್ಷ -2:
    ಪಕ್ಷ -3:
    ಪಕ್ಷ -4:
    ಪಕ್ಷ -5:
    ಪಕ್ಷ -6:
    ಪಕ್ಷ -7:
    ಪಕ್ಷ -8:
    ಪಕ್ಷ -9:
    ಪಕ್ಷ -10:
    ಪಕ್ಷ -11:
    ಪಕ್ಷ -12:
    ಪಕ್ಷ -13:
    ಪಕ್ಷ -14:
    ಪಕ್ಷ -15:
    ಪಕ್ಷ -16:
      tmp = (ಶೂನ್ಯ*)(ರಾಶಿ + FIRST_ARG_SLOT);
      memcpy((ಶೂನ್ಯ*)tmp, &u, cif->rtype->size);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_VOID:
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_pa64(ಶೂನ್ಯ);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  /* The layout of a function descriptor.  */
  ಸಂರಚನಾ pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 code_pointer;
    UINT64 gp;
  };

  ಸಂರಚನಾ ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 code_pointer;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  ಸಂರಚನಾ ffi_pa64_trampoline_struct *tramp;
  ಸಂರಚನಾ pa64_fd *fd;

  ಯದಿ (cif->abi != FFI_PA64)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (ಸಂರಚನಾ pa64_fd *)((UINT64)ffi_closure_pa64);

  /* Setup trampoline.  */
  tramp = (ಸಂರಚನಾ ffi_pa64_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}
#endif
