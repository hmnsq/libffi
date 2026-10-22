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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdio.h>

#define ROUND_UP(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#define FIRST_ARG_SLOT  0
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ અસ્થિર ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ અસ્થિર ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ અસ્થિર ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ અસ્થિર ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) કરો { જો (lvl <= DEBUG_LEVEL) { printf(x); } } જ્યારે (0)

સ્થિર અંતઃરેખ પૂર્ણાંક ffi_struct_type(ffi_type *t)
{
  પૂર્ણાંક sz = t->size;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  જો (sz <= 16)
    પાછા -sz;
  નહીંતર
    પાછા FFI_TYPE_STRUCT;
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

શૂન્ય ffi_prep_args_pa64(UINT64 *ઢગલો, extended_cif *ecif, unsigned bytes)
{
  નોંધણી unsigned પૂર્ણાંક i;
  નોંધણી ffi_type **p_arg;
  નોંધણી શૂન્ય **p_argv;
  unsigned પૂર્ણાંક slot = FIRST_ARG_SLOT;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ઢગલો,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  માટે (i = 0; i < ecif->cif->nargs; i++)
    {
      પૂર્ણાંક type = (*p_arg)->type;

      len = (*p_arg)->size;

      પસંદગી (type)
	{
	સ્થિતિ FFI_TYPE_SINT8:
	  *(SINT64 *)(ઢગલો + slot) = *(SINT8 *)(*p_argv);
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT8:
	  *(UINT64 *)(ઢગલો + slot) = *(UINT8 *)(*p_argv);
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT16:
	  *(SINT64 *)(ઢગલો + slot) = *(SINT16 *)(*p_argv);
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT16:
	  *(UINT64 *)(ઢગલો + slot) = *(UINT16 *)(*p_argv);
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT32:
	  *(SINT64 *)(ઢગલો + slot) = *(SINT32 *)(*p_argv);
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT32:
	  *(UINT64 *)(ઢગલો + slot) = *(UINT32 *)(*p_argv);
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_POINTER:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		slot);
	  *(UINT64 *)(ઢગલો + slot) = *(UINT64 *)(*p_argv);
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT64 *)(ઢગલો + slot) = *(UINT32 *)(*p_argv);
	  પસંદગી (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    સ્થિતિ 0: fldw(ઢગલો + slot, fr4); તોડો;
	    સ્થિતિ 1: fldw(ઢગલો + slot, fr5); તોડો;
	    સ્થિતિ 2: fldw(ઢગલો + slot, fr6); તોડો;
	    સ્થિતિ 3: fldw(ઢગલો + slot, fr7); તોડો;
	    સ્થિતિ 4: fldw(ઢગલો + slot, fr8); તોડો;
	    સ્થિતિ 5: fldw(ઢગલો + slot, fr9); તોડો;
	    સ્થિતિ 6: fldw(ઢગલો + slot, fr10); તોડો;
	    સ્થિતિ 7: fldw(ઢગલો + slot, fr11); તોડો;
	    }
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(ઢગલો + slot) = *(UINT64 *)(*p_argv);
	  પસંદગી (slot - FIRST_ARG_SLOT)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    સ્થિતિ 0: fldd(ઢગલો + slot, fr4); તોડો;
	    સ્થિતિ 1: fldd(ઢગલો + slot, fr5); તોડો;
	    સ્થિતિ 2: fldd(ઢગલો + slot, fr6); તોડો;
	    સ્થિતિ 3: fldd(ઢગલો + slot, fr7); તોડો;
	    સ્થિતિ 4: fldd(ઢગલો + slot, fr8); તોડો;
	    સ્થિતિ 5: fldd(ઢગલો + slot, fr9); તોડો;
	    સ્થિતિ 6: fldd(ઢગલો + slot, fr10); તોડો;
	    સ્થિતિ 7: fldd(ઢગલો + slot, fr11); તોડો;
	    }
	  તોડો;

#ifdef PA64_HPUX
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  slot += (slot & 1);
	  *(UINT64 *)(ઢગલો + slot) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(ઢગલો + slot + 1) = *(UINT64 *)(*p_argv + 8);
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  જો (len > 8)
	    slot += (slot & 1);
	  memcpy((અક્ષર *)(ઢગલો + slot), (અક્ષર *)*p_argv, len);
	  તોડો;

	મૂળભૂત:
	  FFI_ASSERT(0);
	}

      slot += ROUND_UP (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(slot * 8 <= bytes);

  પાછા;
}

સ્થિર શૂન્ય ffi_size_ઢગલો_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  પૂર્ણાંક i;
  પૂર્ણાંક z = 0; /* # stack slots */

  માટે (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      પૂર્ણાંક type = (*ptr)->type;
      પૂર્ણાંક size = (*ptr)->size;

      પસંદગી (type)
	{
#ifdef PA64_HPUX
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_STRUCT:
	  જો (size > 8)
	    z += (z & 1);
	  z += ROUND_UP (size, 8) >> 3;
	  તોડો;

	મૂળભૂત: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  જો (z <= 8)
    z = 8;
  નહીંતર
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->bytes = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_COMPLEX:
    સ્થિતિ FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      તોડો;

    મૂળભૂત:
      cif->flags = (unsigned) cif->rtype->type;
      તોડો;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_PA64:
      ffi_size_ઢગલો_pa64(cif);
      તોડો;

    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
    }

  પાછા FFI_OK;
}

બાહ્ય શૂન્ય ffi_call_pa64(શૂન્ય (*)(UINT64 *, extended_cif *, unsigned),
			  extended_cif *, unsigned, unsigned, unsigned *,
			  શૂન્ય (*fn)(શૂન્ય));

શૂન્ય ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  જો (rvalue == ખાલી
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->size > 16)
    ecif.rvalue = alloca(ROUND_UP (cif->rtype->size, 16));
  નહીંતર
    ecif.rvalue = rvalue;


  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (શૂન્ય *)fn);
      ffi_call_pa64(ffi_prep_args_pa64, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      તોડો;

    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
    }
}

#જો FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_status ffi_closure_inner_pa64(ffi_closure *closure, UINT64 *ઢગલો)
{
  ffi_cif *cif;
  શૂન્ય **avalue;
  શૂન્ય *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  સંઘ { long દ્વિગુણ rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  અક્ષર *tmp;
  પૂર્ણાંક i, avn;
  unsigned પૂર્ણાંક slot = FIRST_ARG_SLOT;
  નોંધણી UINT64 r28 __asm__("r28");

  cif = closure->cif;

  /* If returning via structure, callee will write to our pointer.  */
  જો (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (શૂન્ય *)r28;
  નહીંતર
    rvalue = &u;

  avalue = (શૂન્ય **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  માટે (i = 0; i < avn; i++)
    {
      પૂર્ણાંક type = (*p_arg)->type;

      પસંદગી (type)
	{
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT8:
	  avalue[i] = (શૂન્ય *)(ઢગલો + slot) + 7;
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT16:
	  avalue[i] = (શૂન્ય *)(ઢગલો + slot) + 6;
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT32:
	  avalue[i] = (શૂન્ય *)(ઢગલો + slot) + 4;
	  તોડો;

	સ્થિતિ FFI_TYPE_POINTER:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	  avalue[i] = (શૂન્ય *)(ઢગલો + slot);
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  પસંદગી (slot + FIRST_ARG_SLOT)
	    {
	    સ્થિતિ 0: fstw(fr4, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 1: fstw(fr5, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 2: fstw(fr6, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 3: fstw(fr7, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 4: fstw(fr8, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 5: fstw(fr9, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 6: fstw(fr10, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 7: fstw(fr11, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    }
	  avalue[i] = (શૂન્ય *)(ઢગલો + slot) + 4;
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  પસંદગી (slot + FIRST_ARG_SLOT)
	    {
	    સ્થિતિ 0: fstd(fr4, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 1: fstd(fr5, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 2: fstd(fr6, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 3: fstd(fr7, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 4: fstd(fr8, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 5: fstd(fr9, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 6: fstd(fr10, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    સ્થિતિ 7: fstd(fr11, (શૂન્ય *)(ઢગલો + slot)); તોડો;
	    }
	  avalue[i] = (શૂન્ય *)(ઢગલો + slot);
	  તોડો;

#ifdef PA64_HPUX
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  slot += (slot & 1);
	  avalue[i] = (શૂન્ય *)(ઢગલો + slot);
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  જો((*p_arg)->size > 8)
	    slot += (slot & 1);
	  avalue[i] = (શૂન્ય *) (ઢગલો + slot);
	  તોડો;

	મૂળભૂત:
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
  પસંદગી (cif->flags)
    {
    સ્થિતિ FFI_TYPE_UINT8:
      *(ઢગલો + FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      તોડો;
    સ્થિતિ FFI_TYPE_SINT8:
      *(ઢગલો + FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      તોડો;
    સ્થિતિ FFI_TYPE_UINT16:
      *(ઢગલો + FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      તોડો;
    સ્થિતિ FFI_TYPE_SINT16:
      *(ઢગલો + FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      તોડો;
    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_SINT32:
      *(ઢગલો + FIRST_ARG_SLOT) = (SINT32)u.ret[0];
      તોડો;
    સ્થિતિ FFI_TYPE_UINT32:
      *(ઢગલો - FIRST_ARG_SLOT) = (UINT32)u.ret[0];
      તોડો;
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_POINTER:
      *(ઢગલો - FIRST_ARG_SLOT) = u.ret[0];
      તોડો;

    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      *(ઢગલો + FIRST_ARG_SLOT) = u.ret[0];
      *(ઢગલો + FIRST_ARG_SLOT + 1) = u.ret[1];
      તોડો;

    સ્થિતિ FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      તોડો;

    સ્થિતિ FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      તોડો;

    સ્થિતિ -1:
    સ્થિતિ -2:
    સ્થિતિ -3:
    સ્થિતિ -4:
    સ્થિતિ -5:
    સ્થિતિ -6:
    સ્થિતિ -7:
    સ્થિતિ -8:
    સ્થિતિ -9:
    સ્થિતિ -10:
    સ્થિતિ -11:
    સ્થિતિ -12:
    સ્થિતિ -13:
    સ્થિતિ -14:
    સ્થિતિ -15:
    સ્થિતિ -16:
      tmp = (શૂન્ય*)(ઢગલો + FIRST_ARG_SLOT);
      memcpy((શૂન્ય*)tmp, &u, cif->rtype->size);
      તોડો;

    સ્થિતિ FFI_TYPE_VOID:
      તોડો;

    મૂળભૂત:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      તોડો;
    }
  પાછા FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

બાહ્ય શૂન્ય ffi_closure_pa64(શૂન્ય);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  /* The layout of a function descriptor.  */
  સંરચના pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 code_pointer;
    UINT64 gp;
  };

  સંરચના ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 code_pointer;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  સંરચના ffi_pa64_trampoline_struct *tramp;
  સંરચના pa64_fd *fd;

  જો (cif->abi != FFI_PA64)
    પાછા FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (સંરચના pa64_fd *)((UINT64)ffi_closure_pa64);

  /* Setup trampoline.  */
  tramp = (સંરચના ffi_pa64_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  પાછા FFI_OK;
}
#endif
