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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdio.h>

#define ROUND_UP(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#define MIN_അട്ടി_SIZE  64
#define FIRST_ARG_SLOT  9
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ അസ്ഥിര ("fldw 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ അസ്ഥിര ("fstw %%" #fpreg "L, 0(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ അസ്ഥിര ("fldd 0(%0), %%" #fpreg : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ അസ്ഥിര ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) ചെയ്യുക { എങ്കിൽ (lvl <= DEBUG_LEVEL) { printf(x); } } ആകുമ്പോൾ (0)

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ ffi_struct_type(ffi_type *t)
{
  size_t sz = t->size;

  /* Small structure results are passed in registers,
     larger ones are passed by pointer.  Note that small
     structures differ from the corresponding integer
     types in that they have different alignment requirements.  */

  എങ്കിൽ (sz <= 8)
    തിരികെ -sz;
  അല്ലെങ്കിൽ
    തിരികെ FFI_TYPE_STRUCT; /* else, we pass it by pointer.  */
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

ശൂന്യം ffi_prep_args_pa32(UINT32 *അട്ടി, extended_cif *ecif, unsigned bytes)
{
  രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ i;
  രജിസ്റ്റർ ffi_type **p_arg;
  രജിസ്റ്റർ ശൂന്യം **p_argv;
  unsigned പൂർണ്ണസംഖ്യ slot = FIRST_ARG_SLOT;
  അക്ഷരം *dest_cpy;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, അട്ടി,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  വേണ്ടി (i = 0; i < ecif->cif->nargs; i++)
    {
      പൂർണ്ണസംഖ്യ type = (*p_arg)->type;

      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_SINT8:
	  *(SINT32 *)(അട്ടി - slot) = *(SINT8 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT8:
	  *(UINT32 *)(അട്ടി - slot) = *(UINT8 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT16:
	  *(SINT32 *)(അട്ടി - slot) = *(SINT16 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT16:
	  *(UINT32 *)(അട്ടി - slot) = *(UINT16 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_POINTER:
	  debug(3, "Storing UINT32 %u in slot %u\n", *(UINT32 *)(*p_argv),
		slot);
	  *(UINT32 *)(അട്ടി - slot) = *(UINT32 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	  /* Align slot for 64-bit type.  */
	  slot += (slot & 1) ? 1 : 2;
	  *(UINT64 *)(അട്ടി - slot) = *(UINT64 *)(*p_argv);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
	  /* First 4 args go in fr4L - fr7L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT32 *)(അട്ടി - slot) = *(UINT32 *)(*p_argv);
	  തിരഞ്ഞെടുപ്പ് (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    അവസ്ഥ 0: fldw(അട്ടി - slot, fr4); നിർത്തുക;
	    അവസ്ഥ 1: fldw(അട്ടി - slot, fr5); നിർത്തുക;
	    അവസ്ഥ 2: fldw(അട്ടി - slot, fr6); നിർത്തുക;
	    അവസ്ഥ 3: fldw(അട്ടി - slot, fr7); നിർത്തുക;
	    }
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  /* Align slot for 64-bit type.  */
	  slot += (slot & 1) ? 1 : 2;
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(അട്ടി - slot) = *(UINT64 *)(*p_argv);
	  തിരഞ്ഞെടുപ്പ് (slot - FIRST_ARG_SLOT)
	    {
	      /* First 2 args go in fr5, fr7.  */
	      അവസ്ഥ 1: fldd(അട്ടി - slot, fr5); നിർത്തുക;
	      അവസ്ഥ 3: fldd(അട്ടി - slot, fr7); നിർത്തുക;
	    }
	  നിർത്തുക;

#ifdef PA_HPUX
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are passed in the same manner as structures
	     larger than 8 bytes.  */
	  *(UINT32 *)(അട്ടി - slot) = (UINT32)(*p_argv);
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_STRUCT:

	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */

	  len = (*p_arg)->size;
	  എങ്കിൽ (len <= 4)
	    {
	      dest_cpy = (അക്ഷരം *)(അട്ടി - slot) + 4 - len;
	      memcpy(dest_cpy, (അക്ഷരം *)*p_argv, len);
	    }
	  അല്ലെങ്കിൽ എങ്കിൽ (len <= 8)
	    {
	      slot += (slot & 1) ? 1 : 2;
	      dest_cpy = (അക്ഷരം *)(അട്ടി - slot) + 8 - len;
	      memcpy(dest_cpy, (അക്ഷരം *)*p_argv, len);
	    }
	  അല്ലെങ്കിൽ
	    *(UINT32 *)(അട്ടി - slot) = (UINT32)(*p_argv);
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  FFI_ASSERT(0);
	}

      slot++;
      p_arg++;
      p_argv++;
    }

  /* Make sure we didn't mess up and scribble on the stack.  */
  {
    unsigned പൂർണ്ണസംഖ്യ n;

    debug(5, "Stack setup:\n");
    വേണ്ടി (n = 0; n < (bytes + 3) / 4; n++)
      {
	എങ്കിൽ ((n%4) == 0) { debug(5, "\n%08x: ", (unsigned പൂർണ്ണസംഖ്യ)(അട്ടി - n)); }
	debug(5, "%08x ", *(അട്ടി - n));
      }
    debug(5, "\n");
  }

  FFI_ASSERT(slot * 4 <= bytes);

  തിരികെ;
}

സ്ഥിതം ശൂന്യം ffi_size_അട്ടി_pa32(ffi_cif *cif)
{
  ffi_type **ptr;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ z = 0; /* # stack slots */

  വേണ്ടി (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      പൂർണ്ണസംഖ്യ type = (*ptr)->type;

      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_DOUBLE:
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	  z += 2 + (z & 1); /* must start on even regs, so we may waste one */
	  നിർത്തുക;

#ifdef PA_HPUX
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  z += 1; /* passed by pointer, like a large struct */
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_STRUCT:
	  /* This must mirror the slot accounting in ffi_prep_args_pa32:
	     structs of 1-4 bytes occupy one slot, structs of 5-8 bytes are
	     passed inline in two even-aligned slots (exactly like a 64-bit
	     value), and larger structs are passed by pointer in one slot.
	     z stays offset from the marshaller's slot by FIRST_ARG_SLOT (odd),
	     so (z & 1) tracks the same alignment the marshaller applies.  */
	  {
	    size_t len = (*ptr)->size;
	    എങ്കിൽ (len <= 4 || len > 8)
	      z += 1;
	    അല്ലെങ്കിൽ
	      z += 2 + (z & 1);
	  }
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി: /* <= 32-bit values */
	  z++;
	}
    }

  /* We can fit up to 6 args in the default 64-byte stack frame,
     if we need more, we need more stack.  */
  എങ്കിൽ (z <= 6)
    cif->bytes = MIN_അട്ടി_SIZE; /* min stack size */
  അല്ലെങ്കിൽ
    cif->bytes = 64 + ROUND_UP((z - 6) * വലുപ്പം(UINT32), MIN_അട്ടി_SIZE);

  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
    അവസ്ഥ FFI_TYPE_FLOAT:
    അവസ്ഥ FFI_TYPE_DOUBLE:
      cif->flags = (unsigned) cif->rtype->type;
      നിർത്തുക;

#ifdef PA_HPUX
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      /* Long doubles are treated like a structure.  */
      cif->flags = FFI_TYPE_STRUCT;
      നിർത്തുക;
#endif

    അവസ്ഥ FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 4 bytes, the result is given back
	 in one register. If the size is smaller or equal 8 bytes than we
	 return the result in two registers. But if the size is bigger than
	 8 bytes, we work with pointers.  */
      cif->flags = ffi_struct_type(cif->rtype);
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      cif->flags = FFI_TYPE_INT;
      നിർത്തുക;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_PA32:
      ffi_size_അട്ടി_pa32(cif);
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

ബാഹ്യ ശൂന്യം ffi_call_pa32(ശൂന്യം (*)(UINT32 *, extended_cif *, unsigned),
			  extended_cif *, unsigned, unsigned, unsigned *,
			  ശൂന്യം (*fn)(ശൂന്യം));

ശൂന്യം ffi_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ശൂന്യം **avalue_copy = ശൂന്യ;

  ecif.cif = cif;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  വേണ്ടി (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      പൂർണ്ണസംഖ്യ size = at->size;
      എങ്കിൽ (at->type == FFI_TYPE_STRUCT && size > 8)
	{
	  അക്ഷരം *argcopy = alloca (size);
	  എങ്കിൽ (avalue_copy == ശൂന്യ)
	    {
	      avalue_copy = alloca (nargs * വലുപ്പം (ശൂന്യം *));
	      memcpy (avalue_copy, avalue, nargs * വലുപ്പം (ശൂന്യം *));
	      avalue = avalue_copy;
	    }
	  memcpy (argcopy, avalue[i], size);
	  avalue[i] = argcopy;
	}
    }
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  എങ്കിൽ (rvalue == ശൂന്യ
#ifdef PA_HPUX
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_LONGDOUBLE))
#അല്ലെങ്കിൽ
      && cif->rtype->type == FFI_TYPE_STRUCT)
#endif
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;


  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_PA32:
      debug(3, "Calling ffi_call_pa32: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (ശൂന്യം *)fn);
      ffi_call_pa32(ffi_prep_args_pa32, &ecif, cif->bytes,
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
ffi_status ffi_closure_inner_pa32(ffi_closure *closure, UINT32 *അട്ടി)
{
  ffi_cif *cif;
  ശൂന്യം **avalue;
  ശൂന്യം *rvalue;
  /* Functions can return up to 64-bits in registers.  Return address
     must be double word aligned.  */
  സംഘം { ഇരട്ട rd; UINT32 ret[2]; } u;
  ffi_type **p_arg;
  അക്ഷരം *tmp;
  പൂർണ്ണസംഖ്യ i, avn;
  unsigned പൂർണ്ണസംഖ്യ slot = FIRST_ARG_SLOT;
  രജിസ്റ്റർ UINT32 r28 __asm__("r28");

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
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_POINTER:
	  avalue[i] = (അക്ഷരം *)(അട്ടി - slot) + വലുപ്പം(UINT32) - (*p_arg)->size;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_UINT64:
	  slot += (slot & 1) ? 1 : 2;
	  avalue[i] = (ശൂന്യം *)(അട്ടി - slot);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
#ifdef PA_LINUX
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  തിരഞ്ഞെടുപ്പ് (slot - FIRST_ARG_SLOT)
	    {
	    അവസ്ഥ 0: fstw(fr4, (ശൂന്യം *)(അട്ടി - slot)); നിർത്തുക;
	    അവസ്ഥ 1: fstw(fr5, (ശൂന്യം *)(അട്ടി - slot)); നിർത്തുക;
	    അവസ്ഥ 2: fstw(fr6, (ശൂന്യം *)(അട്ടി - slot)); നിർത്തുക;
	    അവസ്ഥ 3: fstw(fr7, (ശൂന്യം *)(അട്ടി - slot)); നിർത്തുക;
	    }
#endif
	  avalue[i] = (ശൂന്യം *)(അട്ടി - slot);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  slot += (slot & 1) ? 1 : 2;
#ifdef PA_LINUX
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  തിരഞ്ഞെടുപ്പ് (slot - FIRST_ARG_SLOT)
	    {
	    അവസ്ഥ 1: fstd(fr5, (ശൂന്യം *)(അട്ടി - slot)); നിർത്തുക;
	    അവസ്ഥ 3: fstd(fr7, (ശൂന്യം *)(അട്ടി - slot)); നിർത്തുക;
	    }
#endif
	  avalue[i] = (ശൂന്യം *)(അട്ടി - slot);
	  നിർത്തുക;

#ifdef PA_HPUX
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  avalue[i] = (ശൂന്യം *) *(അട്ടി - slot);
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_STRUCT:
	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */
	  എങ്കിൽ((*p_arg)->size <= 4)
	    {
	      avalue[i] = (ശൂന്യം *)(അട്ടി - slot) + വലുപ്പം(UINT32) -
		(*p_arg)->size;
	    }
	  അല്ലെങ്കിൽ എങ്കിൽ ((*p_arg)->size <= 8)
	    {
	      slot += (slot & 1) ? 1 : 2;
	      avalue[i] = (ശൂന്യം *)(അട്ടി - slot) + വലുപ്പം(UINT64) -
		(*p_arg)->size;
	    }
	  അല്ലെങ്കിൽ
	    avalue[i] = (ശൂന്യം *) *(അട്ടി - slot);
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
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
  തിരഞ്ഞെടുപ്പ് (cif->flags)
    {
    അവസ്ഥ FFI_TYPE_UINT8:
      *(അട്ടി - FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT8:
      *(അട്ടി - FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT16:
      *(അട്ടി - FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16:
      *(അട്ടി - FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
    അവസ്ഥ FFI_TYPE_UINT32:
      *(അട്ടി - FIRST_ARG_SLOT) = u.ret[0];
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
      *(അട്ടി - FIRST_ARG_SLOT) = u.ret[0];
      *(അട്ടി - FIRST_ARG_SLOT - 1) = u.ret[1];
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_FLOAT:
      fldw(rvalue, fr4);
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SMALL_STRUCT1:
    അവസ്ഥ FFI_TYPE_SMALL_STRUCT2:
    അവസ്ഥ FFI_TYPE_SMALL_STRUCT3:
    അവസ്ഥ FFI_TYPE_SMALL_STRUCT4:
      tmp = (ശൂന്യം*)(അട്ടി -  FIRST_ARG_SLOT);
      tmp += 4 - cif->rtype->size;
      memcpy((ശൂന്യം*)tmp, &u, cif->rtype->size);
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SMALL_STRUCT5:
    അവസ്ഥ FFI_TYPE_SMALL_STRUCT6:
    അവസ്ഥ FFI_TYPE_SMALL_STRUCT7:
    അവസ്ഥ FFI_TYPE_SMALL_STRUCT8:
      {
	unsigned പൂർണ്ണസംഖ്യ ret2[2];
	പൂർണ്ണസംഖ്യ off;

	/* Right justify ret[0] and ret[1] */
	തിരഞ്ഞെടുപ്പ് (cif->flags)
	  {
	    അവസ്ഥ FFI_TYPE_SMALL_STRUCT5: off = 3; നിർത്തുക;
	    അവസ്ഥ FFI_TYPE_SMALL_STRUCT6: off = 2; നിർത്തുക;
	    അവസ്ഥ FFI_TYPE_SMALL_STRUCT7: off = 1; നിർത്തുക;
	    സ്ഥിരസ്ഥിതി: off = 0; നിർത്തുക;
	  }

	memset (ret2, 0, വലുപ്പം (ret2));
	memcpy ((അക്ഷരം *)ret2 + off, &u, 8 - off);

	*(അട്ടി - FIRST_ARG_SLOT) = ret2[0];
	*(അട്ടി - FIRST_ARG_SLOT - 1) = ret2[1];
      }
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_POINTER:
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

ബാഹ്യ ശൂന്യം ffi_closure_pa32(ശൂന്യം);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  /* The layout of a function descriptor.  A function pointer with the PLABEL
     bit set points to a function descriptor.  */
  ഘടന pa32_fd
  {
    UINT32 code_pointer;
    UINT32 gp;
  };

  ഘടന ffi_pa32_trampoline_struct
  {
     UINT32 code_pointer;        /* Pointer to ffi_closure_unix.  */
     UINT32 fake_gp;             /* Pointer to closure, installed as gp.  */
     UINT32 real_gp;             /* Real gp value.  */
  };

  ഘടന ffi_pa32_trampoline_struct *tramp;
  ഘടന pa32_fd *fd;

  എങ്കിൽ (cif->abi != FFI_PA32)
    തിരികെ FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa32.  */
  fd = (ഘടന pa32_fd *)((UINT32)ffi_closure_pa32 & ~3);

  /* Setup trampoline.  */
  tramp = (ഘടന ffi_pa32_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT32)codeloc & ~3;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  തിരികെ FFI_OK;
}
#endif
