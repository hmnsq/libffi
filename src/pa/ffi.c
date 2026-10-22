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

#define ROUND_UP(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#define MIN_ರಾಶಿ_SIZE  64
#define FIRST_ARG_SLOT  9
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ ಚಞ್ಚಲ ("fldw 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ ಚಞ್ಚಲ ("fstw %%" #fpreg "L, 0(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ ಚಞ್ಚಲ ("fldd 0(%0), %%" #fpreg : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ ಚಞ್ಚಲ ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) ಕುರು { ಯದಿ (lvl <= DEBUG_LEVEL) { printf(x); } } ಯಾವತ್ (0)

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ ffi_struct_type(ffi_type *t)
{
  size_t sz = t->size;

  /* Small structure results are passed in registers,
     larger ones are passed by pointer.  Note that small
     structures differ from the corresponding integer
     types in that they have different alignment requirements.  */

  ಯದಿ (sz <= 8)
    ಪ್ರತ್ಯಾಗಮ -sz;
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ FFI_TYPE_STRUCT; /* else, we pass it by pointer.  */
}

/* PA has a downward growing stack, which looks like this:

   Offset
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

ಶೂನ್ಯ ffi_prep_args_pa32(UINT32 *ರಾಶಿ, extended_cif *ecif, unsigned bytes)
{
  ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಪಞ್ಜಿಕಾ ffi_type **p_arg;
  ಪಞ್ಜಿಕಾ ಶೂನ್ಯ **p_argv;
  unsigned ಪೂರ್ಣಾಙ್ಕ slot = FIRST_ARG_SLOT;
  ವರ್ಣ *dest_cpy;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ರಾಶಿ,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = 0; i < ecif->cif->nargs; i++)
    {
      ಪೂರ್ಣಾಙ್ಕ type = (*p_arg)->type;

      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	  *(SINT32 *)(ರಾಶಿ - slot) = *(SINT8 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT8:
	  *(UINT32 *)(ರಾಶಿ - slot) = *(UINT8 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT16:
	  *(SINT32 *)(ರಾಶಿ - slot) = *(SINT16 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT16:
	  *(UINT32 *)(ರಾಶಿ - slot) = *(UINT16 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  debug(3, "Storing UINT32 %u in slot %u\n", *(UINT32 *)(*p_argv),
		slot);
	  *(UINT32 *)(ರಾಶಿ - slot) = *(UINT32 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	  /* Align slot for 64-bit type.  */
	  slot += (slot & 1) ? 1 : 2;
	  *(UINT64 *)(ರಾಶಿ - slot) = *(UINT64 *)(*p_argv);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  /* First 4 args go in fr4L - fr7L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT32 *)(ರಾಶಿ - slot) = *(UINT32 *)(*p_argv);
	  ವಿಕಲ್ಪ (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    ಪಕ್ಷ 0: fldw(ರಾಶಿ - slot, fr4); ಭಙ್ಗ;
	    ಪಕ್ಷ 1: fldw(ರಾಶಿ - slot, fr5); ಭಙ್ಗ;
	    ಪಕ್ಷ 2: fldw(ರಾಶಿ - slot, fr6); ಭಙ್ಗ;
	    ಪಕ್ಷ 3: fldw(ರಾಶಿ - slot, fr7); ಭಙ್ಗ;
	    }
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  /* Align slot for 64-bit type.  */
	  slot += (slot & 1) ? 1 : 2;
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(ರಾಶಿ - slot) = *(UINT64 *)(*p_argv);
	  ವಿಕಲ್ಪ (slot - FIRST_ARG_SLOT)
	    {
	      /* First 2 args go in fr5, fr7.  */
	      ಪಕ್ಷ 1: fldd(ರಾಶಿ - slot, fr5); ಭಙ್ಗ;
	      ಪಕ್ಷ 3: fldd(ರಾಶಿ - slot, fr7); ಭಙ್ಗ;
	    }
	  ಭಙ್ಗ;

#ifdef PA_HPUX
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are passed in the same manner as structures
	     larger than 8 bytes.  */
	  *(UINT32 *)(ರಾಶಿ - slot) = (UINT32)(*p_argv);
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_STRUCT:

	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */

	  len = (*p_arg)->size;
	  ಯದಿ (len <= 4)
	    {
	      dest_cpy = (ವರ್ಣ *)(ರಾಶಿ - slot) + 4 - len;
	      memcpy(dest_cpy, (ವರ್ಣ *)*p_argv, len);
	    }
	  ಅನ್ಯಥಾ ಯದಿ (len <= 8)
	    {
	      slot += (slot & 1) ? 1 : 2;
	      dest_cpy = (ವರ್ಣ *)(ರಾಶಿ - slot) + 8 - len;
	      memcpy(dest_cpy, (ವರ್ಣ *)*p_argv, len);
	    }
	  ಅನ್ಯಥಾ
	    *(UINT32 *)(ರಾಶಿ - slot) = (UINT32)(*p_argv);
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  FFI_ASSERT(0);
	}

      slot++;
      p_arg++;
      p_argv++;
    }

  /* Make sure we didn't mess up and scribble on the stack.  */
  {
    unsigned ಪೂರ್ಣಾಙ್ಕ n;

    debug(5, "Stack setup:\n");
    ಪ್ರತಿ (n = 0; n < (bytes + 3) / 4; n++)
      {
	ಯದಿ ((n%4) == 0) { debug(5, "\n%08x: ", (unsigned ಪೂರ್ಣಾಙ್ಕ)(ರಾಶಿ - n)); }
	debug(5, "%08x ", *(ರಾಶಿ - n));
      }
    debug(5, "\n");
  }

  FFI_ASSERT(slot * 4 <= bytes);

  ಪ್ರತ್ಯಾಗಮ;
}

ಸ್ಥಿರ ಶೂನ್ಯ ffi_size_ರಾಶಿ_pa32(ffi_cif *cif)
{
  ffi_type **ptr;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ z = 0; /* # stack slots */

  ಪ್ರತಿ (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      ಪೂರ್ಣಾಙ್ಕ type = (*ptr)->type;

      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	  z += 2 + (z & 1); /* must start on even regs, so we may waste one */
	  ಭಙ್ಗ;

#ifdef PA_HPUX
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  z += 1; /* passed by pointer, like a large struct */
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  /* This must mirror the slot accounting in ffi_prep_args_pa32:
	     structs of 1-4 bytes occupy one slot, structs of 5-8 bytes are
	     passed inline in two even-aligned slots (exactly like a 64-bit
	     value), and larger structs are passed by pointer in one slot.
	     z stays offset from the marshaller's slot by FIRST_ARG_SLOT (odd),
	     so (z & 1) tracks the same alignment the marshaller applies.  */
	  {
	    size_t len = (*ptr)->size;
	    ಯದಿ (len <= 4 || len > 8)
	      z += 1;
	    ಅನ್ಯಥಾ
	      z += 2 + (z & 1);
	  }
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ: /* <= 32-bit values */
	  z++;
	}
    }

  /* We can fit up to 6 args in the default 64-byte stack frame,
     if we need more, we need more stack.  */
  ಯದಿ (z <= 6)
    cif->bytes = MIN_ರಾಶಿ_SIZE; /* min stack size */
  ಅನ್ಯಥಾ
    cif->bytes = 64 + ROUND_UP((z - 6) * ಪರಿಮಾಣ(UINT32), MIN_ರಾಶಿ_SIZE);

  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      cif->flags = (unsigned) cif->rtype->type;
      ಭಙ್ಗ;

#ifdef PA_HPUX
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      /* Long doubles are treated like a structure.  */
      cif->flags = FFI_TYPE_STRUCT;
      ಭಙ್ಗ;
#endif

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 4 bytes, the result is given back
	 in one register. If the size is smaller or equal 8 bytes than we
	 return the result in two registers. But if the size is bigger than
	 8 bytes, we work with pointers.  */
      cif->flags = ffi_struct_type(cif->rtype);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64;
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      cif->flags = FFI_TYPE_INT;
      ಭಙ್ಗ;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_PA32:
      ffi_size_ರಾಶಿ_pa32(cif);
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_pa32(ಶೂನ್ಯ (*)(UINT32 *, extended_cif *, unsigned),
			  extended_cif *, unsigned, unsigned, unsigned *,
			  ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ರಿಕ್ತ;

  ecif.cif = cif;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ಪ್ರತಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಙ್ಕ size = at->size;
      ಯದಿ (at->type == FFI_TYPE_STRUCT && size > 8)
	{
	  ವರ್ಣ *argcopy = alloca (size);
	  ಯದಿ (avalue_copy == ರಿಕ್ತ)
	    {
	      avalue_copy = alloca (nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
	      memcpy (avalue_copy, avalue, nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
	      avalue = avalue_copy;
	    }
	  memcpy (argcopy, avalue[i], size);
	  avalue[i] = argcopy;
	}
    }
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ಯದಿ (rvalue == ರಿಕ್ತ
#ifdef PA_HPUX
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_LONGDOUBLE))
#ಅನ್ಯಥಾ
      && cif->rtype->type == FFI_TYPE_STRUCT)
#endif
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;


  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_PA32:
      debug(3, "Calling ffi_call_pa32: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (ಶೂನ್ಯ *)fn);
      ffi_call_pa32(ffi_prep_args_pa32, &ecif, cif->bytes,
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
ffi_status ffi_closure_inner_pa32(ffi_closure *closure, UINT32 *ರಾಶಿ)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **avalue;
  ಶೂನ್ಯ *rvalue;
  /* Functions can return up to 64-bits in registers.  Return address
     must be double word aligned.  */
  ಸಙ್ಘ { ದ್ವಿಗುಣ rd; UINT32 ret[2]; } u;
  ffi_type **p_arg;
  ವರ್ಣ *tmp;
  ಪೂರ್ಣಾಙ್ಕ i, avn;
  unsigned ಪೂರ್ಣಾಙ್ಕ slot = FIRST_ARG_SLOT;
  ಪಞ್ಜಿಕಾ UINT32 r28 __asm__("r28");

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
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  avalue[i] = (ವರ್ಣ *)(ರಾಶಿ - slot) + ಪರಿಮಾಣ(UINT32) - (*p_arg)->size;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	  slot += (slot & 1) ? 1 : 2;
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ - slot);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
#ifdef PA_LINUX
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  ವಿಕಲ್ಪ (slot - FIRST_ARG_SLOT)
	    {
	    ಪಕ್ಷ 0: fstw(fr4, (ಶೂನ್ಯ *)(ರಾಶಿ - slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 1: fstw(fr5, (ಶೂನ್ಯ *)(ರಾಶಿ - slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 2: fstw(fr6, (ಶೂನ್ಯ *)(ರಾಶಿ - slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 3: fstw(fr7, (ಶೂನ್ಯ *)(ರಾಶಿ - slot)); ಭಙ್ಗ;
	    }
#endif
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ - slot);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  slot += (slot & 1) ? 1 : 2;
#ifdef PA_LINUX
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  ವಿಕಲ್ಪ (slot - FIRST_ARG_SLOT)
	    {
	    ಪಕ್ಷ 1: fstd(fr5, (ಶೂನ್ಯ *)(ರಾಶಿ - slot)); ಭಙ್ಗ;
	    ಪಕ್ಷ 3: fstd(fr7, (ಶೂನ್ಯ *)(ರಾಶಿ - slot)); ಭಙ್ಗ;
	    }
#endif
	  avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ - slot);
	  ಭಙ್ಗ;

#ifdef PA_HPUX
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  avalue[i] = (ಶೂನ್ಯ *) *(ರಾಶಿ - slot);
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */
	  ಯದಿ((*p_arg)->size <= 4)
	    {
	      avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ - slot) + ಪರಿಮಾಣ(UINT32) -
		(*p_arg)->size;
	    }
	  ಅನ್ಯಥಾ ಯದಿ ((*p_arg)->size <= 8)
	    {
	      slot += (slot & 1) ? 1 : 2;
	      avalue[i] = (ಶೂನ್ಯ *)(ರಾಶಿ - slot) + ಪರಿಮಾಣ(UINT64) -
		(*p_arg)->size;
	    }
	  ಅನ್ಯಥಾ
	    avalue[i] = (ಶೂನ್ಯ *) *(ರಾಶಿ - slot);
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  FFI_ASSERT(0);
	}

      slot++;
      p_arg++;
    }

  /* Invoke the closure.  */
  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  debug(3, "after calling function, ret[0] = %08x, ret[1] = %08x\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  ವಿಕಲ್ಪ (cif->flags)
    {
    ಪಕ್ಷ FFI_TYPE_UINT8:
      *(ರಾಶಿ - FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT8:
      *(ರಾಶಿ - FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      *(ರಾಶಿ - FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      *(ರಾಶಿ - FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_SINT32:
    ಪಕ್ಷ FFI_TYPE_UINT32:
      *(ರಾಶಿ - FIRST_ARG_SLOT) = u.ret[0];
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
      *(ರಾಶಿ - FIRST_ARG_SLOT) = u.ret[0];
      *(ರಾಶಿ - FIRST_ARG_SLOT - 1) = u.ret[1];
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_FLOAT:
      fldw(rvalue, fr4);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT1:
    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT2:
    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT3:
    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT4:
      tmp = (ಶೂನ್ಯ*)(ರಾಶಿ -  FIRST_ARG_SLOT);
      tmp += 4 - cif->rtype->size;
      memcpy((ಶೂನ್ಯ*)tmp, &u, cif->rtype->size);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT5:
    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT6:
    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT7:
    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT8:
      {
	unsigned ಪೂರ್ಣಾಙ್ಕ ret2[2];
	ಪೂರ್ಣಾಙ್ಕ off;

	/* Right justify ret[0] and ret[1] */
	ವಿಕಲ್ಪ (cif->flags)
	  {
	    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT5: off = 3; ಭಙ್ಗ;
	    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT6: off = 2; ಭಙ್ಗ;
	    ಪಕ್ಷ FFI_TYPE_SMALL_STRUCT7: off = 1; ಭಙ್ಗ;
	    ಸ್ವಭಾವ: off = 0; ಭಙ್ಗ;
	  }

	memset (ret2, 0, ಪರಿಮಾಣ (ret2));
	memcpy ((ವರ್ಣ *)ret2 + off, &u, 8 - off);

	*(ರಾಶಿ - FIRST_ARG_SLOT) = ret2[0];
	*(ರಾಶಿ - FIRST_ARG_SLOT - 1) = ret2[1];
      }
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_POINTER:
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

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_pa32(ಶೂನ್ಯ);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  /* The layout of a function descriptor.  A function pointer with the PLABEL
     bit set points to a function descriptor.  */
  ಸಂರಚನಾ pa32_fd
  {
    UINT32 code_pointer;
    UINT32 gp;
  };

  ಸಂರಚನಾ ffi_pa32_trampoline_struct
  {
     UINT32 code_pointer;        /* Pointer to ffi_closure_unix.  */
     UINT32 fake_gp;             /* Pointer to closure, installed as gp.  */
     UINT32 real_gp;             /* Real gp value.  */
  };

  ಸಂರಚನಾ ffi_pa32_trampoline_struct *tramp;
  ಸಂರಚನಾ pa32_fd *fd;

  ಯದಿ (cif->abi != FFI_PA32)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa32.  */
  fd = (ಸಂರಚನಾ pa32_fd *)((UINT32)ffi_closure_pa32 & ~3);

  /* Setup trampoline.  */
  tramp = (ಸಂರಚನಾ ffi_pa32_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT32)codeloc & ~3;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}
#endif
