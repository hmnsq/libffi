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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdio.h>

#define ROUND_UP(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#define FIRST_ARG_SLOT  0
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ അസ്ഥിര ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ അസ്ഥിര ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ അസ്ഥിര ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ അസ്ഥിര ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) ചെയ്യുക { എങ്കിൽ (lvl <= DEBUG_LEVEL) { printf(x); } } ആകുമ്പോൾ (0)

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ ffi_struct_type(ffi_type *t)
{
  പൂർണ്ണസംഖ്യ sz = t->size;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  എങ്കിൽ (sz <= 16)
    തിരികെ -sz;
  അല്ലെങ്കിൽ
    തിരികെ FFI_TYPE_STRUCT;
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

ശൂന്യം ffi_prep_args_pa64(UINT64 *അട്ടി, extended_cif *ecif, unsigned bytes)
{
  രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ i;
  രജിസ്റ്റർ ffi_type **p_arg;
  രജിസ്റ്റർ ശൂന്യം **p_argv;
  unsigned പൂർണ്ണസംഖ്യ slot = FIRST_ARG_SLOT;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, അട്ടി,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  വേണ്ടി (i = 0; i < ecif->cif->nargs; i++)
    {
      പൂർണ്ണസംഖ്യ type = (*p_arg)->type;

      len = (*p_arg)->size;

      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_SINT8:
	  *(SINT64 *)(അട്ടി + slot) = *(SINT8 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT8:
	  *(UINT64 *)(അട്ടി + slot) = *(UINT8 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT16:
	  *(SINT64 *)(അട്ടി + slot) = *(SINT16 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT16:
	  *(UINT64 *)(അട്ടി + slot) = *(UINT16 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT32:
	  *(SINT64 *)(അട്ടി + slot) = *(SINT32 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT32:
	  *(UINT64 *)(അട്ടി + slot) = *(UINT32 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_POINTER:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		slot);
	  *(UINT64 *)(അട്ടി + slot) = *(UINT64 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT64 *)(അട്ടി + slot) = *(UINT32 *)(*p_argv);
	  തിരഞ്ഞെടുപ്പ് (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    അവസ്ഥ 0: fldw(അട്ടി + slot, fr4); നിർത്തുക;
	    അവസ്ഥ 1: fldw(അട്ടി + slot, fr5); നിർത്തുക;
	    അവസ്ഥ 2: fldw(അട്ടി + slot, fr6); നിർത്തുക;
	    അവസ്ഥ 3: fldw(അട്ടി + slot, fr7); നിർത്തുക;
	    അവസ്ഥ 4: fldw(അട്ടി + slot, fr8); നിർത്തുക;
	    അവസ്ഥ 5: fldw(അട്ടി + slot, fr9); നിർത്തുക;
	    അവസ്ഥ 6: fldw(അട്ടി + slot, fr10); നിർത്തുക;
	    അവസ്ഥ 7: fldw(അട്ടി + slot, fr11); നിർത്തുക;
	    }
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(അട്ടി + slot) = *(UINT64 *)(*p_argv);
	  തിരഞ്ഞെടുപ്പ് (slot - FIRST_ARG_SLOT)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    അവസ്ഥ 0: fldd(അട്ടി + slot, fr4); നിർത്തുക;
	    അവസ്ഥ 1: fldd(അട്ടി + slot, fr5); നിർത്തുക;
	    അവസ്ഥ 2: fldd(അട്ടി + slot, fr6); നിർത്തുക;
	    അവസ്ഥ 3: fldd(അട്ടി + slot, fr7); നിർത്തുക;
	    അവസ്ഥ 4: fldd(അട്ടി + slot, fr8); നിർത്തുക;
	    അവസ്ഥ 5: fldd(അട്ടി + slot, fr9); നിർത്തുക;
	    അവസ്ഥ 6: fldd(അട്ടി + slot, fr10); നിർത്തുക;
	    അവസ്ഥ 7: fldd(അട്ടി + slot, fr11); നിർത്തുക;
	    }
	  നിർത്തുക;

#ifdef PA64_HPUX
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  slot += (slot & 1);
	  *(UINT64 *)(അട്ടി + slot) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(അട്ടി + slot + 1) = *(UINT64 *)(*p_argv + 8);
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  എങ്കിൽ (len > 8)
	    slot += (slot & 1);
	  memcpy((അക്ഷരം *)(അട്ടി + slot), (അക്ഷരം *)*p_argv, len);
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  FFI_ASSERT(0);
	}

      slot += ROUND_UP (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(slot * 8 <= bytes);

  തിരികെ;
}

സ്ഥിതം ശൂന്യം ffi_size_അട്ടി_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ z = 0; /* # stack slots */

  വേണ്ടി (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      പൂർണ്ണസംഖ്യ type = (*ptr)->type;
      പൂർണ്ണസംഖ്യ size = (*ptr)->size;

      തിരഞ്ഞെടുപ്പ് (type)
	{
#ifdef PA64_HPUX
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_STRUCT:
	  എങ്കിൽ (size > 8)
	    z += (z & 1);
	  z += ROUND_UP (size, 8) >> 3;
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  എങ്കിൽ (z <= 8)
    z = 8;
  അല്ലെങ്കിൽ
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->bytes = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_COMPLEX:
    അവസ്ഥ FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      cif->flags = (unsigned) cif->rtype->type;
      നിർത്തുക;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_PA64:
      ffi_size_അട്ടി_pa64(cif);
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

ബാഹ്യ ശൂന്യം ffi_call_pa64(ശൂന്യം (*)(UINT64 *, extended_cif *, unsigned),
			  extended_cif *, unsigned, unsigned, unsigned *,
			  ശൂന്യം (*fn)(ശൂന്യം));

ശൂന്യം ffi_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  എങ്കിൽ (rvalue == ശൂന്യ
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->size > 16)
    ecif.rvalue = alloca(ROUND_UP (cif->rtype->size, 16));
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;


  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (ശൂന്യം *)fn);
      ffi_call_pa64(ffi_prep_args_pa64, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }
}

#എങ്കിൽ FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_status ffi_closure_inner_pa64(ffi_closure *closure, UINT64 *അട്ടി)
{
  ffi_cif *cif;
  ശൂന്യം **avalue;
  ശൂന്യം *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  സംഘം { long ഇരട്ട rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  അക്ഷരം *tmp;
  പൂർണ്ണസംഖ്യ i, avn;
  unsigned പൂർണ്ണസംഖ്യ slot = FIRST_ARG_SLOT;
  രജിസ്റ്റർ UINT64 r28 __asm__("r28");

  cif = closure->cif;

  /* If returning via structure, callee will write to our pointer.  */
  എങ്കിൽ (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (ശൂന്യം *)r28;
  അല്ലെങ്കിൽ
    rvalue = &u;

  avalue = (ശൂന്യം **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  വേണ്ടി (i = 0; i < avn; i++)
    {
      പൂർണ്ണസംഖ്യ type = (*p_arg)->type;

      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT8:
	  avalue[i] = (ശൂന്യം *)(അട്ടി + slot) + 7;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT16:
	  avalue[i] = (ശൂന്യം *)(അട്ടി + slot) + 6;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT32:
	  avalue[i] = (ശൂന്യം *)(അട്ടി + slot) + 4;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_POINTER:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_UINT64:
	  avalue[i] = (ശൂന്യം *)(അട്ടി + slot);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  തിരഞ്ഞെടുപ്പ് (slot + FIRST_ARG_SLOT)
	    {
	    അവസ്ഥ 0: fstw(fr4, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 1: fstw(fr5, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 2: fstw(fr6, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 3: fstw(fr7, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 4: fstw(fr8, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 5: fstw(fr9, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 6: fstw(fr10, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 7: fstw(fr11, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    }
	  avalue[i] = (ശൂന്യം *)(അട്ടി + slot) + 4;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  തിരഞ്ഞെടുപ്പ് (slot + FIRST_ARG_SLOT)
	    {
	    അവസ്ഥ 0: fstd(fr4, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 1: fstd(fr5, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 2: fstd(fr6, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 3: fstd(fr7, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 4: fstd(fr8, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 5: fstd(fr9, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 6: fstd(fr10, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    അവസ്ഥ 7: fstd(fr11, (ശൂന്യം *)(അട്ടി + slot)); നിർത്തുക;
	    }
	  avalue[i] = (ശൂന്യം *)(അട്ടി + slot);
	  നിർത്തുക;

#ifdef PA64_HPUX
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  slot += (slot & 1);
	  avalue[i] = (ശൂന്യം *)(അട്ടി + slot);
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  എങ്കിൽ((*p_arg)->size > 8)
	    slot += (slot & 1);
	  avalue[i] = (ശൂന്യം *) (അട്ടി + slot);
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
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
  തിരഞ്ഞെടുപ്പ് (cif->flags)
    {
    അവസ്ഥ FFI_TYPE_UINT8:
      *(അട്ടി + FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT8:
      *(അട്ടി + FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT16:
      *(അട്ടി + FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16:
      *(അട്ടി + FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
      *(അട്ടി + FIRST_ARG_SLOT) = (SINT32)u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT32:
      *(അട്ടി - FIRST_ARG_SLOT) = (UINT32)u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_POINTER:
      *(അട്ടി - FIRST_ARG_SLOT) = u.ret[0];
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      *(അട്ടി + FIRST_ARG_SLOT) = u.ret[0];
      *(അട്ടി + FIRST_ARG_SLOT + 1) = u.ret[1];
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      നിർത്തുക;

    അവസ്ഥ -1:
    അവസ്ഥ -2:
    അവസ്ഥ -3:
    അവസ്ഥ -4:
    അവസ്ഥ -5:
    അവസ്ഥ -6:
    അവസ്ഥ -7:
    അവസ്ഥ -8:
    അവസ്ഥ -9:
    അവസ്ഥ -10:
    അവസ്ഥ -11:
    അവസ്ഥ -12:
    അവസ്ഥ -13:
    അവസ്ഥ -14:
    അവസ്ഥ -15:
    അവസ്ഥ -16:
      tmp = (ശൂന്യം*)(അട്ടി + FIRST_ARG_SLOT);
      memcpy((ശൂന്യം*)tmp, &u, cif->rtype->size);
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_VOID:
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      നിർത്തുക;
    }
  തിരികെ FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

ബാഹ്യ ശൂന്യം ffi_closure_pa64(ശൂന്യം);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  /* The layout of a function descriptor.  */
  ഘടന pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 code_pointer;
    UINT64 gp;
  };

  ഘടന ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 code_pointer;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  ഘടന ffi_pa64_trampoline_struct *tramp;
  ഘടന pa64_fd *fd;

  എങ്കിൽ (cif->abi != FFI_PA64)
    തിരികെ FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (ഘടന pa64_fd *)((UINT64)ffi_closure_pa64);

  /* Setup trampoline.  */
  tramp = (ഘടന ffi_pa64_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  തിരികെ FFI_OK;
}
#endif
