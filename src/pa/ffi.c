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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ವ್ಯಾಖ್ಯಾನಿಸು ಸುತ್ತು_ಮೇಲೆ(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#ವ್ಯಾಖ್ಯಾನಿಸು MIN_ರಾಶಿ_ಗಾತ್ರ  64
#ವ್ಯಾಖ್ಯಾನಿಸು ಮೊದಲ_ARG_ಸ್ಥಾನ  9
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_LEVEL   0

#ವ್ಯಾಖ್ಯಾನಿಸು fldw(addr, fpreg) \
  __asm__ ಅಸ್ಥಿರ ("fldw 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#ವ್ಯಾಖ್ಯಾನಿಸು fstw(fpreg, addr) \
  __asm__ ಅಸ್ಥಿರ ("fstw %%" #fpreg "L, 0(%0)" : : "r"(addr))
#ವ್ಯಾಖ್ಯಾನಿಸು fldd(addr, fpreg) \
  __asm__ ಅಸ್ಥಿರ ("fldd 0(%0), %%" #fpreg : : "r"(addr) : #fpreg)
#ವ್ಯಾಖ್ಯಾನಿಸು fstd(fpreg, addr) \
  __asm__ ಅಸ್ಥಿರ ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#ವ್ಯಾಖ್ಯಾನಿಸು debug(lvl, x...) ಮಾಡು { ಆದರೆ (lvl <= DEBUG_LEVEL) { ಬರೆ(x); } } ಆಗ (0)

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ffi_struct_type(ffi_type *t)
{
  size_t sz = t->ಗಾತ್ರ_2;

  /* Small structure results are passed in registers,
     larger ones are passed by pointer.  Note that small
     structures differ from the corresponding integer
     types in that they have different alignment requirements.  */

  ಆದರೆ (sz <= 8)
    ಹಿಂತಿರುಗಿ -sz;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ FFI_TYPE_STRUCT; /* else, we pass it by pointer.  */
}

/* PA has a downward growing stack, which looks like this:

ಸ್ಥಾನಾಂತರ
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

ಶೂನ್ಯ ffi_prep_ವಾದಗಳು_pa32(UINT32 *ರಾಶಿ, extended_cif *ecif, ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು)
{
  ನೋಂದಣಿ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ನೋಂದಣಿ ffi_type **p_arg;
  ನೋಂದಣಿ ಶೂನ್ಯ **p_argv;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸ್ಥಾನ = ಮೊದಲ_ARG_ಸ್ಥಾನ;
  ಅಕ್ಷರ *dest_cpy;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ರಾಶಿ,
	ecif, ಬೈಟುಗಳು);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  ಗಾಗಿ (i = 0; i < ecif->cif->nargs; i++)
    {
      ಪೂರ್ಣಾಂಕ type = (*p_arg)->type;

      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  *(SINT32 *)(ರಾಶಿ - ಸ್ಥಾನ) = *(SINT8 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  *(UINT32 *)(ರಾಶಿ - ಸ್ಥಾನ) = *(UINT8 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  *(SINT32 *)(ರಾಶಿ - ಸ್ಥಾನ) = *(SINT16 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  *(UINT32 *)(ರಾಶಿ - ಸ್ಥಾನ) = *(UINT16 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  debug(3, "Storing UINT32 %u in slot %u\n", *(UINT32 *)(*p_argv),
		ಸ್ಥಾನ);
	  *(UINT32 *)(ರಾಶಿ - ಸ್ಥಾನ) = *(UINT32 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	  /* Align slot for 64-bit type.  */
	  ಸ್ಥಾನ += (ಸ್ಥಾನ & 1) ? 1 : 2;
	  *(UINT64 *)(ರಾಶಿ - ಸ್ಥಾನ) = *(UINT64 *)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  /* First 4 args go in fr4L - fr7L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", ಸ್ಥಾನ);
	  *(UINT32 *)(ರಾಶಿ - ಸ್ಥಾನ) = *(UINT32 *)(*p_argv);
	  ಆಯ್ಕೆ (ಸ್ಥಾನ - ಮೊದಲ_ARG_ಸ್ಥಾನ)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    ಸ್ಥಿತಿ 0: fldw(ರಾಶಿ - ಸ್ಥಾನ, fr4); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 1: fldw(ರಾಶಿ - ಸ್ಥಾನ, fr5); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 2: fldw(ರಾಶಿ - ಸ್ಥಾನ, fr6); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 3: fldw(ರಾಶಿ - ಸ್ಥಾನ, fr7); ನಿಲ್ಲಿಸು;
	    }
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  /* Align slot for 64-bit type.  */
	  ಸ್ಥಾನ += (ಸ್ಥಾನ & 1) ? 1 : 2;
	  debug(3, "Storing UINT64(double) at slot %u\n", ಸ್ಥಾನ);
	  *(UINT64 *)(ರಾಶಿ - ಸ್ಥಾನ) = *(UINT64 *)(*p_argv);
	  ಆಯ್ಕೆ (ಸ್ಥಾನ - ಮೊದಲ_ARG_ಸ್ಥಾನ)
	    {
	      /* First 2 args go in fr5, fr7.  */
	      ಸ್ಥಿತಿ 1: fldd(ರಾಶಿ - ಸ್ಥಾನ, fr5); ನಿಲ್ಲಿಸು;
	      ಸ್ಥಿತಿ 3: fldd(ರಾಶಿ - ಸ್ಥಾನ, fr7); ನಿಲ್ಲಿಸು;
	    }
	  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA_HPUX
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are passed in the same manner as structures
	     larger than 8 bytes.  */
	  *(UINT32 *)(ರಾಶಿ - ಸ್ಥಾನ) = (UINT32)(*p_argv);
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:

	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */

	  len = (*p_arg)->ಗಾತ್ರ_2;
	  ಆದರೆ (len <= 4)
	    {
	      dest_cpy = (ಅಕ್ಷರ *)(ರಾಶಿ - ಸ್ಥಾನ) + 4 - len;
	      ಸ್ಮರಣೆ_ನಕಲು(dest_cpy, (ಅಕ್ಷರ *)*p_argv, len);
	    }
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (len <= 8)
	    {
	      ಸ್ಥಾನ += (ಸ್ಥಾನ & 1) ? 1 : 2;
	      dest_cpy = (ಅಕ್ಷರ *)(ರಾಶಿ - ಸ್ಥಾನ) + 8 - len;
	      ಸ್ಮರಣೆ_ನಕಲು(dest_cpy, (ಅಕ್ಷರ *)*p_argv, len);
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    *(UINT32 *)(ರಾಶಿ - ಸ್ಥಾನ) = (UINT32)(*p_argv);
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  FFI_ASSERT(0);
	}

      ಸ್ಥಾನ++;
      p_arg++;
      p_argv++;
    }

  /* Make sure we didn't mess up and scribble on the stack.  */
  {
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n;

    debug(5, "Stack setup:\n");
    ಗಾಗಿ (n = 0; n < (ಬೈಟುಗಳು + 3) / 4; n++)
      {
	ಆದರೆ ((n%4) == 0) { debug(5, "\n%08x: ", (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(ರಾಶಿ - n)); }
	debug(5, "%08x ", *(ರಾಶಿ - n));
      }
    debug(5, "\n");
  }

  FFI_ASSERT(ಸ್ಥಾನ * 4 <= ಬೈಟುಗಳು);

  ಹಿಂತಿರುಗಿ;
}

ಸ್ಥಿರ ಶೂನ್ಯ ffi_ಗಾತ್ರ_ರಾಶಿ_pa32(ffi_cif *cif)
{
  ffi_type **ptr;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ z = 0; /* # stack slots */

  ಗಾಗಿ (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      ಪೂರ್ಣಾಂಕ type = (*ptr)->type;

      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	  z += 2 + (z & 1); /* must start on even regs, so we may waste one */
	  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA_HPUX
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  z += 1; /* passed by pointer, like a large struct */
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  /* This must mirror the slot accounting in ffi_prep_args_pa32:
	     structs of 1-4 bytes occupy one slot, structs of 5-8 bytes are
	     passed inline in two even-aligned slots (exactly like a 64-bit
	     value), and larger structs are passed by pointer in one slot.
	     z stays offset from the marshaller's slot by FIRST_ARG_SLOT (odd),
	     so (z & 1) tracks the same alignment the marshaller applies.  */
	  {
	    size_t len = (*ptr)->ಗಾತ್ರ_2;
	    ಆದರೆ (len <= 4 || len > 8)
	      z += 1;
	    ಇಲ್ಲದಿದ್ದರೆ
	      z += 2 + (z & 1);
	  }
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ: /* <= 32-bit values */
	  z++;
	}
    }

  /* We can fit up to 6 args in the default 64-byte stack frame,
     if we need more, we need more stack.  */
  ಆದರೆ (z <= 6)
    cif->ಬೈಟುಗಳು = MIN_ರಾಶಿ_ಗಾತ್ರ; /* min stack size */
  ಇಲ್ಲದಿದ್ದರೆ
    cif->ಬೈಟುಗಳು = 64 + ಸುತ್ತು_ಮೇಲೆ((z - 6) * ಗಾತ್ರ(UINT32), MIN_ರಾಶಿ_ಗಾತ್ರ);

  debug(3, "Calculated stack size is %u bytes\n", cif->ಬೈಟುಗಳು);
}

/* Perform machine dependent cif processing.  */
ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      cif->flags = (ಚಿಹ್ನೆರಹಿತ) cif->rtype->type;
      ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA_HPUX
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      /* Long doubles are treated like a structure.  */
      cif->flags = FFI_TYPE_STRUCT;
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 4 bytes, the result is given back
	 in one register. If the size is smaller or equal 8 bytes than we
	 return the result in two registers. But if the size is bigger than
	 8 bytes, we work with pointers.  */
      cif->flags = ffi_struct_type(cif->rtype);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      cif->flags = FFI_TYPE_INT;
      ನಿಲ್ಲಿಸು;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_PA32:
      ffi_ಗಾತ್ರ_ರಾಶಿ_pa32(cif);
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_pa32(ಶೂನ್ಯ (*)(UINT32 *, extended_cif *, ಚಿಹ್ನೆರಹಿತ),
			  extended_cif *, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ *,
			  ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ಖಾಲಿ;

  ecif.cif = cif;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ಗಾಗಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = at->ಗಾತ್ರ_2;
      ಆದರೆ (at->type == FFI_TYPE_STRUCT && ಗಾತ್ರ_2 > 8)
	{
	  ಅಕ್ಷರ *argcopy = alloca (ಗಾತ್ರ_2);
	  ಆದರೆ (avalue_copy == ಖಾಲಿ)
	    {
	      avalue_copy = alloca (nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
	      ಸ್ಮರಣೆ_ನಕಲು (avalue_copy, avalue, nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
	      avalue = avalue_copy;
	    }
	  ಸ್ಮರಣೆ_ನಕಲು (argcopy, avalue[i], ಗಾತ್ರ_2);
	  avalue[i] = argcopy;
	}
    }
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ಆದರೆ (rvalue == ಖಾಲಿ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA_HPUX
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_LONGDOUBLE))
#ಇಲ್ಲದಿದ್ದರೆ
      && cif->rtype->type == FFI_TYPE_STRUCT)
#ಆದರೆ_ಅಂತ್ಯ
    {
      ecif.rvalue = alloca(cif->rtype->ಗಾತ್ರ_2);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;


  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_PA32:
      debug(3, "Calling ffi_call_pa32: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->ಬೈಟುಗಳು, cif->flags, ecif.rvalue, (ಶೂನ್ಯ *)fn);
      ffi_call_pa32(ffi_prep_ವಾದಗಳು_pa32, &ecif, cif->ಬೈಟುಗಳು,
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
ffi_ಸ್ಥಿತಿ ffi_ಆವರಣ_inner_pa32(ffi_ಆವರಣ *ಆವರಣ, UINT32 *ರಾಶಿ)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **avalue;
  ಶೂನ್ಯ *rvalue;
  /* Functions can return up to 64-bits in registers.  Return address
     must be double word aligned.  */
  ಒಕ್ಕೂಟ { ದ್ವಿಗುಣ rd; UINT32 ret[2]; } u;
  ffi_type **p_arg;
  ಅಕ್ಷರ *tmp;
  ಪೂರ್ಣಾಂಕ i, avn;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸ್ಥಾನ = ಮೊದಲ_ARG_ಸ್ಥಾನ;
  ನೋಂದಣಿ UINT32 r28 __asm__("r28");

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
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  avalue[i] = (ಅಕ್ಷರ *)(ರಾಶಿ - ಸ್ಥಾನ) + ಗಾತ್ರ(UINT32) - (*p_arg)->ಗಾತ್ರ_2;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	  ಸ್ಥಾನ += (ಸ್ಥಾನ & 1) ? 1 : 2;
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA_LINUX
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  ಆಯ್ಕೆ (ಸ್ಥಾನ - ಮೊದಲ_ARG_ಸ್ಥಾನ)
	    {
	    ಸ್ಥಿತಿ 0: fstw(fr4, (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 1: fstw(fr5, (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 2: fstw(fr6, (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 3: fstw(fr7, (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    }
#ಆದರೆ_ಅಂತ್ಯ
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಸ್ಥಾನ += (ಸ್ಥಾನ & 1) ? 1 : 2;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA_LINUX
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  ಆಯ್ಕೆ (ಸ್ಥಾನ - ಮೊದಲ_ARG_ಸ್ಥಾನ)
	    {
	    ಸ್ಥಿತಿ 1: fstd(fr5, (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ 3: fstd(fr7, (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ)); ನಿಲ್ಲಿಸು;
	    }
#ಆದರೆ_ಅಂತ್ಯ
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ);
	  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA_HPUX
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  avalue[i] = (ಶೂನ್ಯ *) *(ರಾಶಿ - ಸ್ಥಾನ);
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */
	  ಆದರೆ((*p_arg)->ಗಾತ್ರ_2 <= 4)
	    {
	      avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ) + ಗಾತ್ರ(UINT32) -
		(*p_arg)->ಗಾತ್ರ_2;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((*p_arg)->ಗಾತ್ರ_2 <= 8)
	    {
	      ಸ್ಥಾನ += (ಸ್ಥಾನ & 1) ? 1 : 2;
	      avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ - ಸ್ಥಾನ) + ಗಾತ್ರ(UINT64) -
		(*p_arg)->ಗಾತ್ರ_2;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    avalue[i] = (ಶೂನ್ಯ *) *(ರಾಶಿ - ಸ್ಥಾನ);
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  FFI_ASSERT(0);
	}

      ಸ್ಥಾನ++;
      p_arg++;
    }

  /* Invoke the closure.  */
  (ಆವರಣ->fun) (cif, rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);

  debug(3, "after calling function, ret[0] = %08x, ret[1] = %08x\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  ಆಯ್ಕೆ (cif->flags)
    {
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      *(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ) = (UINT8)u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      *(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ) = (SINT8)u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      *(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ) = (UINT16)u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      *(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ) = (SINT16)u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      *(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ) = u.ret[0];
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      *(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ) = u.ret[0];
      *(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ - 1) = u.ret[1];
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      fldw(rvalue, fr4);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT1:
    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT2:
    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT3:
    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT4:
      tmp = (ಶೂನ್ಯ*)(ರಾಶಿ -  ಮೊದಲ_ARG_ಸ್ಥಾನ);
      tmp += 4 - cif->rtype->ಗಾತ್ರ_2;
      ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ*)tmp, &u, cif->rtype->ಗಾತ್ರ_2);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT5:
    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT6:
    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT7:
    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT8:
      {
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ret2[2];
	ಪೂರ್ಣಾಂಕ off;

	/* Right justify ret[0] and ret[1] */
	ಆಯ್ಕೆ (cif->flags)
	  {
	    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT5: off = 3; ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT6: off = 2; ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ FFI_TYPE_SMALL_STRUCT7: off = 1; ನಿಲ್ಲಿಸು;
	    ಪೂರ್ವನಿಯೋಜಿತ: off = 0; ನಿಲ್ಲಿಸು;
	  }

	memset (ret2, 0, ಗಾತ್ರ (ret2));
	ಸ್ಮರಣೆ_ನಕಲು ((ಅಕ್ಷರ *)ret2 + off, &u, 8 - off);

	*(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ) = ret2[0];
	*(ರಾಶಿ - ಮೊದಲ_ARG_ಸ್ಥಾನ - 1) = ret2[1];
      }
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
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

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_pa32(ಶೂನ್ಯ);

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  /* The layout of a function descriptor.  A function pointer with the PLABEL
     bit set points to a function descriptor.  */
  ರಚನೆ pa32_fd
  {
    UINT32 ಸಂಕೇತ_ಸೂಚಕ;
    UINT32 gp;
  };

  ರಚನೆ ffi_pa32_trampoline_struct
  {
     UINT32 ಸಂಕೇತ_ಸೂಚಕ;        /* Pointer to ffi_closure_unix.  */
     UINT32 fake_gp;             /* Pointer to closure, installed as gp.  */
     UINT32 real_gp;             /* Real gp value.  */
  };

  ರಚನೆ ffi_pa32_trampoline_struct *tramp;
  ರಚನೆ pa32_fd *fd;

  ಆದರೆ (cif->abi != FFI_PA32)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa32.  */
  fd = (ರಚನೆ pa32_fd *)((UINT32)ffi_ಆವರಣ_pa32 & ~3);

  /* Setup trampoline.  */
  tramp = (ರಚನೆ ffi_pa32_trampoline_struct *)ಆವರಣ->tramp;
  tramp->ಸಂಕೇತ_ಸೂಚಕ = fd->ಸಂಕೇತ_ಸೂಚಕ;
  tramp->fake_gp = (UINT32)codeloc & ~3;
  tramp->real_gp = fd->gp;

  ಆವರಣ->cif  = cif;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
  ಆವರಣ->fun  = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}
#ಆದರೆ_ಅಂತ್ಯ
