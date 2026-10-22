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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdio.h>

#define ROUND_UP(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#define MIN_স্তূপ_আকার  64
#define FIRST_ARG_SLOT  9
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ অস্থির ("fldw 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ অস্থির ("fstw %%" #fpreg "L, 0(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ অস্থির ("fldd 0(%0), %%" #fpreg : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ অস্থির ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) করো { যদি (lvl <= DEBUG_LEVEL) { printf(x); } } যখন (0)

স্থির অন্তঃরেখ পূর্ণসংখ্যা ffi_struct_type(ffi_type *t)
{
  size_t sz = t->আকার_2;

  /* Small structure results are passed in registers,
     larger ones are passed by pointer.  Note that small
     structures differ from the corresponding integer
     types in that they have different alignment requirements.  */

  যদি (sz <= 8)
    ফেরত -sz;
  অন্যথা
    ফেরত FFI_TYPE_STRUCT; /* else, we pass it by pointer.  */
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

শূন্য ffi_prep_args_pa32(UINT32 *স্তূপ, extended_cif *ecif, unsigned bytes)
{
  নিবন্ধ unsigned পূর্ণসংখ্যা i;
  নিবন্ধ ffi_type **p_arg;
  নিবন্ধ শূন্য **p_argv;
  unsigned পূর্ণসংখ্যা slot = FIRST_ARG_SLOT;
  অক্ষর *dest_cpy;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, স্তূপ,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  জন্য (i = 0; i < ecif->cif->nargs; i++)
    {
      পূর্ণসংখ্যা type = (*p_arg)->type;

      নির্বাচন (type)
	{
	ক্ষেত্র FFI_TYPE_SINT8:
	  *(SINT32 *)(স্তূপ - slot) = *(SINT8 *)(*p_argv);
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT8:
	  *(UINT32 *)(স্তূপ - slot) = *(UINT8 *)(*p_argv);
	  বিরতি;

	ক্ষেত্র FFI_TYPE_SINT16:
	  *(SINT32 *)(স্তূপ - slot) = *(SINT16 *)(*p_argv);
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT16:
	  *(UINT32 *)(স্তূপ - slot) = *(UINT16 *)(*p_argv);
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_POINTER:
	  debug(3, "Storing UINT32 %u in slot %u\n", *(UINT32 *)(*p_argv),
		slot);
	  *(UINT32 *)(স্তূপ - slot) = *(UINT32 *)(*p_argv);
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_SINT64:
	  /* Align slot for 64-bit type.  */
	  slot += (slot & 1) ? 1 : 2;
	  *(UINT64 *)(স্তূপ - slot) = *(UINT64 *)(*p_argv);
	  বিরতি;

	ক্ষেত্র FFI_TYPE_FLOAT:
	  /* First 4 args go in fr4L - fr7L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT32 *)(স্তূপ - slot) = *(UINT32 *)(*p_argv);
	  নির্বাচন (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    ক্ষেত্র 0: fldw(স্তূপ - slot, fr4); বিরতি;
	    ক্ষেত্র 1: fldw(স্তূপ - slot, fr5); বিরতি;
	    ক্ষেত্র 2: fldw(স্তূপ - slot, fr6); বিরতি;
	    ক্ষেত্র 3: fldw(স্তূপ - slot, fr7); বিরতি;
	    }
	  বিরতি;

	ক্ষেত্র FFI_TYPE_DOUBLE:
	  /* Align slot for 64-bit type.  */
	  slot += (slot & 1) ? 1 : 2;
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(স্তূপ - slot) = *(UINT64 *)(*p_argv);
	  নির্বাচন (slot - FIRST_ARG_SLOT)
	    {
	      /* First 2 args go in fr5, fr7.  */
	      ক্ষেত্র 1: fldd(স্তূপ - slot, fr5); বিরতি;
	      ক্ষেত্র 3: fldd(স্তূপ - slot, fr7); বিরতি;
	    }
	  বিরতি;

#ifdef PA_HPUX
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are passed in the same manner as structures
	     larger than 8 bytes.  */
	  *(UINT32 *)(স্তূপ - slot) = (UINT32)(*p_argv);
	  বিরতি;
#endif

	ক্ষেত্র FFI_TYPE_STRUCT:

	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */

	  len = (*p_arg)->আকার_2;
	  যদি (len <= 4)
	    {
	      dest_cpy = (অক্ষর *)(স্তূপ - slot) + 4 - len;
	      memcpy(dest_cpy, (অক্ষর *)*p_argv, len);
	    }
	  অন্যথা যদি (len <= 8)
	    {
	      slot += (slot & 1) ? 1 : 2;
	      dest_cpy = (অক্ষর *)(স্তূপ - slot) + 8 - len;
	      memcpy(dest_cpy, (অক্ষর *)*p_argv, len);
	    }
	  অন্যথা
	    *(UINT32 *)(স্তূপ - slot) = (UINT32)(*p_argv);
	  বিরতি;

	পূর্বনির্ধারিত:
	  FFI_ASSERT(0);
	}

      slot++;
      p_arg++;
      p_argv++;
    }

  /* Make sure we didn't mess up and scribble on the stack.  */
  {
    unsigned পূর্ণসংখ্যা n;

    debug(5, "Stack setup:\n");
    জন্য (n = 0; n < (bytes + 3) / 4; n++)
      {
	যদি ((n%4) == 0) { debug(5, "\n%08x: ", (unsigned পূর্ণসংখ্যা)(স্তূপ - n)); }
	debug(5, "%08x ", *(স্তূপ - n));
      }
    debug(5, "\n");
  }

  FFI_ASSERT(slot * 4 <= bytes);

  ফেরত;
}

স্থির শূন্য ffi_আকার_স্তূপ_pa32(ffi_cif *cif)
{
  ffi_type **ptr;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা z = 0; /* # stack slots */

  জন্য (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      পূর্ণসংখ্যা type = (*ptr)->type;

      নির্বাচন (type)
	{
	ক্ষেত্র FFI_TYPE_DOUBLE:
	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_SINT64:
	  z += 2 + (z & 1); /* must start on even regs, so we may waste one */
	  বিরতি;

#ifdef PA_HPUX
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	  z += 1; /* passed by pointer, like a large struct */
	  বিরতি;
#endif

	ক্ষেত্র FFI_TYPE_STRUCT:
	  /* This must mirror the slot accounting in ffi_prep_args_pa32:
	     structs of 1-4 bytes occupy one slot, structs of 5-8 bytes are
	     passed inline in two even-aligned slots (exactly like a 64-bit
	     value), and larger structs are passed by pointer in one slot.
	     z stays offset from the marshaller's slot by FIRST_ARG_SLOT (odd),
	     so (z & 1) tracks the same alignment the marshaller applies.  */
	  {
	    size_t len = (*ptr)->আকার_2;
	    যদি (len <= 4 || len > 8)
	      z += 1;
	    অন্যথা
	      z += 2 + (z & 1);
	  }
	  বিরতি;

	পূর্বনির্ধারিত: /* <= 32-bit values */
	  z++;
	}
    }

  /* We can fit up to 6 args in the default 64-byte stack frame,
     if we need more, we need more stack.  */
  যদি (z <= 6)
    cif->bytes = MIN_স্তূপ_আকার; /* min stack size */
  অন্যথা
    cif->bytes = 64 + ROUND_UP((z - 6) * আকার(UINT32), MIN_স্তূপ_আকার);

  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag */
  নির্বাচন (cif->rtype->type)
    {
    ক্ষেত্র FFI_TYPE_VOID:
    ক্ষেত্র FFI_TYPE_FLOAT:
    ক্ষেত্র FFI_TYPE_DOUBLE:
      cif->flags = (unsigned) cif->rtype->type;
      বিরতি;

#ifdef PA_HPUX
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
      /* Long doubles are treated like a structure.  */
      cif->flags = FFI_TYPE_STRUCT;
      বিরতি;
#endif

    ক্ষেত্র FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 4 bytes, the result is given back
	 in one register. If the size is smaller or equal 8 bytes than we
	 return the result in two registers. But if the size is bigger than
	 8 bytes, we work with pointers.  */
      cif->flags = ffi_struct_type(cif->rtype);
      বিরতি;

    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64;
      বিরতি;

    পূর্বনির্ধারিত:
      cif->flags = FFI_TYPE_INT;
      বিরতি;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_PA32:
      ffi_আকার_স্তূপ_pa32(cif);
      বিরতি;

    পূর্বনির্ধারিত:
      FFI_ASSERT(0);
      বিরতি;
    }

  ফেরত FFI_OK;
}

বাহ্যিক শূন্য ffi_call_pa32(শূন্য (*)(UINT32 *, extended_cif *, unsigned),
			  extended_cif *, unsigned, unsigned, unsigned *,
			  শূন্য (*fn)(শূন্য));

শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  শূন্য **avalue_copy = ফাঁকা;

  ecif.cif = cif;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  জন্য (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      পূর্ণসংখ্যা আকার_2 = at->আকার_2;
      যদি (at->type == FFI_TYPE_STRUCT && আকার_2 > 8)
	{
	  অক্ষর *argcopy = alloca (আকার_2);
	  যদি (avalue_copy == ফাঁকা)
	    {
	      avalue_copy = alloca (nargs * আকার (শূন্য *));
	      memcpy (avalue_copy, avalue, nargs * আকার (শূন্য *));
	      avalue = avalue_copy;
	    }
	  memcpy (argcopy, avalue[i], আকার_2);
	  avalue[i] = argcopy;
	}
    }
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  যদি (rvalue == ফাঁকা
#ifdef PA_HPUX
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_LONGDOUBLE))
#অন্যথা
      && cif->rtype->type == FFI_TYPE_STRUCT)
#endif
    {
      ecif.rvalue = alloca(cif->rtype->আকার_2);
    }
  অন্যথা
    ecif.rvalue = rvalue;


  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_PA32:
      debug(3, "Calling ffi_call_pa32: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (শূন্য *)fn);
      ffi_call_pa32(ffi_prep_args_pa32, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      বিরতি;

    পূর্বনির্ধারিত:
      FFI_ASSERT(0);
      বিরতি;
    }
}

#যদি FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_status ffi_closure_inner_pa32(ffi_closure *closure, UINT32 *স্তূপ)
{
  ffi_cif *cif;
  শূন্য **avalue;
  শূন্য *rvalue;
  /* Functions can return up to 64-bits in registers.  Return address
     must be double word aligned.  */
  সংঘ { দ্বিগুণ rd; UINT32 ret[2]; } u;
  ffi_type **p_arg;
  অক্ষর *tmp;
  পূর্ণসংখ্যা i, avn;
  unsigned পূর্ণসংখ্যা slot = FIRST_ARG_SLOT;
  নিবন্ধ UINT32 r28 __asm__("r28");

  cif = closure->cif;

  /* If returning via structure, callee will write to our pointer.  */
  যদি (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (শূন্য *)r28;
  অন্যথা
    rvalue = &u;

  avalue = (শূন্য **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  জন্য (i = 0; i < avn; i++)
    {
      পূর্ণসংখ্যা type = (*p_arg)->type;

      নির্বাচন (type)
	{
	ক্ষেত্র FFI_TYPE_SINT8:
	ক্ষেত্র FFI_TYPE_UINT8:
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_UINT16:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_POINTER:
	  avalue[i] = (অক্ষর *)(স্তূপ - slot) + আকার(UINT32) - (*p_arg)->আকার_2;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_SINT64:
	ক্ষেত্র FFI_TYPE_UINT64:
	  slot += (slot & 1) ? 1 : 2;
	  avalue[i] = (শূন্য *)(স্তূপ - slot);
	  বিরতি;

	ক্ষেত্র FFI_TYPE_FLOAT:
#ifdef PA_LINUX
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  নির্বাচন (slot - FIRST_ARG_SLOT)
	    {
	    ক্ষেত্র 0: fstw(fr4, (শূন্য *)(স্তূপ - slot)); বিরতি;
	    ক্ষেত্র 1: fstw(fr5, (শূন্য *)(স্তূপ - slot)); বিরতি;
	    ক্ষেত্র 2: fstw(fr6, (শূন্য *)(স্তূপ - slot)); বিরতি;
	    ক্ষেত্র 3: fstw(fr7, (শূন্য *)(স্তূপ - slot)); বিরতি;
	    }
#endif
	  avalue[i] = (শূন্য *)(স্তূপ - slot);
	  বিরতি;

	ক্ষেত্র FFI_TYPE_DOUBLE:
	  slot += (slot & 1) ? 1 : 2;
#ifdef PA_LINUX
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  নির্বাচন (slot - FIRST_ARG_SLOT)
	    {
	    ক্ষেত্র 1: fstd(fr5, (শূন্য *)(স্তূপ - slot)); বিরতি;
	    ক্ষেত্র 3: fstd(fr7, (শূন্য *)(স্তূপ - slot)); বিরতি;
	    }
#endif
	  avalue[i] = (শূন্য *)(স্তূপ - slot);
	  বিরতি;

#ifdef PA_HPUX
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  avalue[i] = (শূন্য *) *(স্তূপ - slot);
	  বিরতি;
#endif

	ক্ষেত্র FFI_TYPE_STRUCT:
	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */
	  যদি((*p_arg)->আকার_2 <= 4)
	    {
	      avalue[i] = (শূন্য *)(স্তূপ - slot) + আকার(UINT32) -
		(*p_arg)->আকার_2;
	    }
	  অন্যথা যদি ((*p_arg)->আকার_2 <= 8)
	    {
	      slot += (slot & 1) ? 1 : 2;
	      avalue[i] = (শূন্য *)(স্তূপ - slot) + আকার(UINT64) -
		(*p_arg)->আকার_2;
	    }
	  অন্যথা
	    avalue[i] = (শূন্য *) *(স্তূপ - slot);
	  বিরতি;

	পূর্বনির্ধারিত:
	  FFI_ASSERT(0);
	}

      slot++;
      p_arg++;
    }

  /* Invoke the closure.  */
  (closure->fun) (cif, rvalue, avalue, closure->user_তথ্য);

  debug(3, "after calling function, ret[0] = %08x, ret[1] = %08x\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  নির্বাচন (cif->flags)
    {
    ক্ষেত্র FFI_TYPE_UINT8:
      *(স্তূপ - FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT8:
      *(স্তূপ - FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT16:
      *(স্তূপ - FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT16:
      *(স্তূপ - FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      বিরতি;
    ক্ষেত্র FFI_TYPE_INT:
    ক্ষেত্র FFI_TYPE_SINT32:
    ক্ষেত্র FFI_TYPE_UINT32:
      *(স্তূপ - FIRST_ARG_SLOT) = u.ret[0];
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
      *(স্তূপ - FIRST_ARG_SLOT) = u.ret[0];
      *(স্তূপ - FIRST_ARG_SLOT - 1) = u.ret[1];
      বিরতি;

    ক্ষেত্র FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      বিরতি;

    ক্ষেত্র FFI_TYPE_FLOAT:
      fldw(rvalue, fr4);
      বিরতি;

    ক্ষেত্র FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      বিরতি;

    ক্ষেত্র FFI_TYPE_SMALL_STRUCT1:
    ক্ষেত্র FFI_TYPE_SMALL_STRUCT2:
    ক্ষেত্র FFI_TYPE_SMALL_STRUCT3:
    ক্ষেত্র FFI_TYPE_SMALL_STRUCT4:
      tmp = (শূন্য*)(স্তূপ -  FIRST_ARG_SLOT);
      tmp += 4 - cif->rtype->আকার_2;
      memcpy((শূন্য*)tmp, &u, cif->rtype->আকার_2);
      বিরতি;

    ক্ষেত্র FFI_TYPE_SMALL_STRUCT5:
    ক্ষেত্র FFI_TYPE_SMALL_STRUCT6:
    ক্ষেত্র FFI_TYPE_SMALL_STRUCT7:
    ক্ষেত্র FFI_TYPE_SMALL_STRUCT8:
      {
	unsigned পূর্ণসংখ্যা ret2[2];
	পূর্ণসংখ্যা off;

	/* Right justify ret[0] and ret[1] */
	নির্বাচন (cif->flags)
	  {
	    ক্ষেত্র FFI_TYPE_SMALL_STRUCT5: off = 3; বিরতি;
	    ক্ষেত্র FFI_TYPE_SMALL_STRUCT6: off = 2; বিরতি;
	    ক্ষেত্র FFI_TYPE_SMALL_STRUCT7: off = 1; বিরতি;
	    পূর্বনির্ধারিত: off = 0; বিরতি;
	  }

	memset (ret2, 0, আকার (ret2));
	memcpy ((অক্ষর *)ret2 + off, &u, 8 - off);

	*(স্তূপ - FIRST_ARG_SLOT) = ret2[0];
	*(স্তূপ - FIRST_ARG_SLOT - 1) = ret2[1];
      }
      বিরতি;

    ক্ষেত্র FFI_TYPE_POINTER:
    ক্ষেত্র FFI_TYPE_VOID:
      বিরতি;

    পূর্বনির্ধারিত:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      বিরতি;
    }
  ফেরত FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

বাহ্যিক শূন্য ffi_closure_pa32(শূন্য);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc)
{
  /* The layout of a function descriptor.  A function pointer with the PLABEL
     bit set points to a function descriptor.  */
  কাঠামো pa32_fd
  {
    UINT32 code_pointer;
    UINT32 gp;
  };

  কাঠামো ffi_pa32_trampoline_struct
  {
     UINT32 code_pointer;        /* Pointer to ffi_closure_unix.  */
     UINT32 fake_gp;             /* Pointer to closure, installed as gp.  */
     UINT32 real_gp;             /* Real gp value.  */
  };

  কাঠামো ffi_pa32_trampoline_struct *tramp;
  কাঠামো pa32_fd *fd;

  যদি (cif->abi != FFI_PA32)
    ফেরত FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa32.  */
  fd = (কাঠামো pa32_fd *)((UINT32)ffi_closure_pa32 & ~3);

  /* Setup trampoline.  */
  tramp = (কাঠামো ffi_pa32_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT32)codeloc & ~3;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_তথ্য = user_তথ্য;
  closure->fun  = fun;

  ফেরত FFI_OK;
}
#endif
