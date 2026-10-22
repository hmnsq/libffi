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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਗੇੜਾ_ਉੱਪਰ(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_ਢੇਰ_ਮਾਪ  64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਹਿਲਾ_ARG_ਥਾਂ  9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_LEVEL   0

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fldw(addr, fpreg) \
  __asm__ ਅਸਥਿਰ ("fldw 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fstw(fpreg, addr) \
  __asm__ ਅਸਥਿਰ ("fstw %%" #fpreg "L, 0(%0)" : : "r"(addr))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fldd(addr, fpreg) \
  __asm__ ਅਸਥਿਰ ("fldd 0(%0), %%" #fpreg : : "r"(addr) : #fpreg)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fstd(fpreg, addr) \
  __asm__ ਅਸਥਿਰ ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug(lvl, x...) ਕਰੋ { ਜੇ (lvl <= DEBUG_LEVEL) { ਲਿਖੋ(x); } } ਜਦੋਂ (0)

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ ffi_struct_type(ffi_type *t)
{
  size_t sz = t->ਮਾਪ;

  /* Small structure results are passed in registers,
     larger ones are passed by pointer.  Note that small
     structures differ from the corresponding integer
     types in that they have different alignment requirements.  */

  ਜੇ (sz <= 8)
    ਵਾਪਸ -sz;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ FFI_TYPE_STRUCT; /* else, we pass it by pointer.  */
}

/* PA has a downward growing stack, which looks like this:

ਵਿੱਥ
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

ਖਾਲੀ ffi_prep_ਦਲੀਲਾਂ_pa32(UINT32 *ਢੇਰ, extended_cif *ecif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ)
{
  ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਰਜਿਸਟਰ ffi_type **p_arg;
  ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਥਾਂ = ਪਹਿਲਾ_ARG_ਥਾਂ;
  ਅੱਖਰ *dest_cpy;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ਢੇਰ,
	ecif, ਬਾਈਟਾਂ);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  ਲਈ (i = 0; i < ecif->cif->nargs; i++)
    {
      ਪੂਰਨ type = (*p_arg)->type;

      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	  *(SINT32 *)(ਢੇਰ - ਥਾਂ) = *(SINT8 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT8:
	  *(UINT32 *)(ਢੇਰ - ਥਾਂ) = *(UINT8 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT16:
	  *(SINT32 *)(ਢੇਰ - ਥਾਂ) = *(SINT16 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT16:
	  *(UINT32 *)(ਢੇਰ - ਥਾਂ) = *(UINT16 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  debug(3, "Storing UINT32 %u in slot %u\n", *(UINT32 *)(*p_argv),
		ਥਾਂ);
	  *(UINT32 *)(ਢੇਰ - ਥਾਂ) = *(UINT32 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	  /* Align slot for 64-bit type.  */
	  ਥਾਂ += (ਥਾਂ & 1) ? 1 : 2;
	  *(UINT64 *)(ਢੇਰ - ਥਾਂ) = *(UINT64 *)(*p_argv);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  /* First 4 args go in fr4L - fr7L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", ਥਾਂ);
	  *(UINT32 *)(ਢੇਰ - ਥਾਂ) = *(UINT32 *)(*p_argv);
	  ਚੋਣ (ਥਾਂ - ਪਹਿਲਾ_ARG_ਥਾਂ)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    ਹਾਲਤ 0: fldw(ਢੇਰ - ਥਾਂ, fr4); ਰੋਕੋ;
	    ਹਾਲਤ 1: fldw(ਢੇਰ - ਥਾਂ, fr5); ਰੋਕੋ;
	    ਹਾਲਤ 2: fldw(ਢੇਰ - ਥਾਂ, fr6); ਰੋਕੋ;
	    ਹਾਲਤ 3: fldw(ਢੇਰ - ਥਾਂ, fr7); ਰੋਕੋ;
	    }
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  /* Align slot for 64-bit type.  */
	  ਥਾਂ += (ਥਾਂ & 1) ? 1 : 2;
	  debug(3, "Storing UINT64(double) at slot %u\n", ਥਾਂ);
	  *(UINT64 *)(ਢੇਰ - ਥਾਂ) = *(UINT64 *)(*p_argv);
	  ਚੋਣ (ਥਾਂ - ਪਹਿਲਾ_ARG_ਥਾਂ)
	    {
	      /* First 2 args go in fr5, fr7.  */
	      ਹਾਲਤ 1: fldd(ਢੇਰ - ਥਾਂ, fr5); ਰੋਕੋ;
	      ਹਾਲਤ 3: fldd(ਢੇਰ - ਥਾਂ, fr7); ਰੋਕੋ;
	    }
	  ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA_HPUX
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are passed in the same manner as structures
	     larger than 8 bytes.  */
	  *(UINT32 *)(ਢੇਰ - ਥਾਂ) = (UINT32)(*p_argv);
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_STRUCT:

	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */

	  len = (*p_arg)->ਮਾਪ;
	  ਜੇ (len <= 4)
	    {
	      dest_cpy = (ਅੱਖਰ *)(ਢੇਰ - ਥਾਂ) + 4 - len;
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest_cpy, (ਅੱਖਰ *)*p_argv, len);
	    }
	  ਨਹੀਂਤਾਂ ਜੇ (len <= 8)
	    {
	      ਥਾਂ += (ਥਾਂ & 1) ? 1 : 2;
	      dest_cpy = (ਅੱਖਰ *)(ਢੇਰ - ਥਾਂ) + 8 - len;
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest_cpy, (ਅੱਖਰ *)*p_argv, len);
	    }
	  ਨਹੀਂਤਾਂ
	    *(UINT32 *)(ਢੇਰ - ਥਾਂ) = (UINT32)(*p_argv);
	  ਰੋਕੋ;

	ਮੂਲ:
	  FFI_ASSERT(0);
	}

      ਥਾਂ++;
      p_arg++;
      p_argv++;
    }

  /* Make sure we didn't mess up and scribble on the stack.  */
  {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n;

    debug(5, "Stack setup:\n");
    ਲਈ (n = 0; n < (ਬਾਈਟਾਂ + 3) / 4; n++)
      {
	ਜੇ ((n%4) == 0) { debug(5, "\n%08x: ", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(ਢੇਰ - n)); }
	debug(5, "%08x ", *(ਢੇਰ - n));
      }
    debug(5, "\n");
  }

  FFI_ASSERT(ਥਾਂ * 4 <= ਬਾਈਟਾਂ);

  ਵਾਪਸ;
}

ਸਥਿਰ ਖਾਲੀ ffi_ਮਾਪ_ਢੇਰ_pa32(ffi_cif *cif)
{
  ffi_type **ptr;
  ਪੂਰਨ i;
  ਪੂਰਨ z = 0; /* # stack slots */

  ਲਈ (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      ਪੂਰਨ type = (*ptr)->type;

      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	  z += 2 + (z & 1); /* must start on even regs, so we may waste one */
	  ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA_HPUX
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  z += 1; /* passed by pointer, like a large struct */
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  /* This must mirror the slot accounting in ffi_prep_args_pa32:
	     structs of 1-4 bytes occupy one slot, structs of 5-8 bytes are
	     passed inline in two even-aligned slots (exactly like a 64-bit
	     value), and larger structs are passed by pointer in one slot.
	     z stays offset from the marshaller's slot by FIRST_ARG_SLOT (odd),
	     so (z & 1) tracks the same alignment the marshaller applies.  */
	  {
	    size_t len = (*ptr)->ਮਾਪ;
	    ਜੇ (len <= 4 || len > 8)
	      z += 1;
	    ਨਹੀਂਤਾਂ
	      z += 2 + (z & 1);
	  }
	  ਰੋਕੋ;

	ਮੂਲ: /* <= 32-bit values */
	  z++;
	}
    }

  /* We can fit up to 6 args in the default 64-byte stack frame,
     if we need more, we need more stack.  */
  ਜੇ (z <= 6)
    cif->ਬਾਈਟਾਂ = MIN_ਢੇਰ_ਮਾਪ; /* min stack size */
  ਨਹੀਂਤਾਂ
    cif->ਬਾਈਟਾਂ = 64 + ਗੇੜਾ_ਉੱਪਰ((z - 6) * ਆਕਾਰ(UINT32), MIN_ਢੇਰ_ਮਾਪ);

  debug(3, "Calculated stack size is %u bytes\n", cif->ਬਾਈਟਾਂ);
}

/* Perform machine dependent cif processing.  */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ) cif->rtype->type;
      ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA_HPUX
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      /* Long doubles are treated like a structure.  */
      cif->flags = FFI_TYPE_STRUCT;
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ

    ਹਾਲਤ FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 4 bytes, the result is given back
	 in one register. If the size is smaller or equal 8 bytes than we
	 return the result in two registers. But if the size is bigger than
	 8 bytes, we work with pointers.  */
      cif->flags = ffi_struct_type(cif->rtype);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64;
      ਰੋਕੋ;

    ਮੂਲ:
      cif->flags = FFI_TYPE_INT;
      ਰੋਕੋ;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_PA32:
      ffi_ਮਾਪ_ਢੇਰ_pa32(cif);
      ਰੋਕੋ;

    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_pa32(ਖਾਲੀ (*)(UINT32 *, extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ),
			  extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *,
			  ਖਾਲੀ (*fn)(ਖਾਲੀ));

ਖਾਲੀ ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ਖਾਲੀ **avalue_copy = ਨਿਰਾ;

  ecif.cif = cif;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ਲਈ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ਪੂਰਨ ਮਾਪ = at->ਮਾਪ;
      ਜੇ (at->type == FFI_TYPE_STRUCT && ਮਾਪ > 8)
	{
	  ਅੱਖਰ *argcopy = alloca (ਮਾਪ);
	  ਜੇ (avalue_copy == ਨਿਰਾ)
	    {
	      avalue_copy = alloca (nargs * ਆਕਾਰ (ਖਾਲੀ *));
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ (avalue_copy, avalue, nargs * ਆਕਾਰ (ਖਾਲੀ *));
	      avalue = avalue_copy;
	    }
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (argcopy, avalue[i], ਮਾਪ);
	  avalue[i] = argcopy;
	}
    }
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ਜੇ (rvalue == ਨਿਰਾ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA_HPUX
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_LONGDOUBLE))
#ਨਹੀਂਤਾਂ
      && cif->rtype->type == FFI_TYPE_STRUCT)
#ਜੇ_ਅੰਤ
    {
      ecif.rvalue = alloca(cif->rtype->ਮਾਪ);
    }
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;


  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_PA32:
      debug(3, "Calling ffi_call_pa32: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->ਬਾਈਟਾਂ, cif->flags, ecif.rvalue, (ਖਾਲੀ *)fn);
      ffi_call_pa32(ffi_prep_ਦਲੀਲਾਂ_pa32, &ecif, cif->ਬਾਈਟਾਂ,
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
ffi_ਸਥਿਤੀ ffi_ਘੇਰਾ_inner_pa32(ffi_ਘੇਰਾ *ਘੇਰਾ, UINT32 *ਢੇਰ)
{
  ffi_cif *cif;
  ਖਾਲੀ **avalue;
  ਖਾਲੀ *rvalue;
  /* Functions can return up to 64-bits in registers.  Return address
     must be double word aligned.  */
  ਸੰਘ { ਦੁੱਗਣਾ rd; UINT32 ret[2]; } u;
  ffi_type **p_arg;
  ਅੱਖਰ *tmp;
  ਪੂਰਨ i, avn;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਥਾਂ = ਪਹਿਲਾ_ARG_ਥਾਂ;
  ਰਜਿਸਟਰ UINT32 r28 __asm__("r28");

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
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  avalue[i] = (ਅੱਖਰ *)(ਢੇਰ - ਥਾਂ) + ਆਕਾਰ(UINT32) - (*p_arg)->ਮਾਪ;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	  ਥਾਂ += (ਥਾਂ & 1) ? 1 : 2;
	  avalue[i] = (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA_LINUX
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  ਚੋਣ (ਥਾਂ - ਪਹਿਲਾ_ARG_ਥਾਂ)
	    {
	    ਹਾਲਤ 0: fstw(fr4, (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 1: fstw(fr5, (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 2: fstw(fr6, (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 3: fstw(fr7, (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ)); ਰੋਕੋ;
	    }
#ਜੇ_ਅੰਤ
	  avalue[i] = (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਥਾਂ += (ਥਾਂ & 1) ? 1 : 2;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA_LINUX
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  ਚੋਣ (ਥਾਂ - ਪਹਿਲਾ_ARG_ਥਾਂ)
	    {
	    ਹਾਲਤ 1: fstd(fr5, (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ)); ਰੋਕੋ;
	    ਹਾਲਤ 3: fstd(fr7, (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ)); ਰੋਕੋ;
	    }
#ਜੇ_ਅੰਤ
	  avalue[i] = (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ);
	  ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA_HPUX
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  avalue[i] = (ਖਾਲੀ *) *(ਢੇਰ - ਥਾਂ);
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */
	  ਜੇ((*p_arg)->ਮਾਪ <= 4)
	    {
	      avalue[i] = (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ) + ਆਕਾਰ(UINT32) -
		(*p_arg)->ਮਾਪ;
	    }
	  ਨਹੀਂਤਾਂ ਜੇ ((*p_arg)->ਮਾਪ <= 8)
	    {
	      ਥਾਂ += (ਥਾਂ & 1) ? 1 : 2;
	      avalue[i] = (ਖਾਲੀ *)(ਢੇਰ - ਥਾਂ) + ਆਕਾਰ(UINT64) -
		(*p_arg)->ਮਾਪ;
	    }
	  ਨਹੀਂਤਾਂ
	    avalue[i] = (ਖਾਲੀ *) *(ਢੇਰ - ਥਾਂ);
	  ਰੋਕੋ;

	ਮੂਲ:
	  FFI_ASSERT(0);
	}

      ਥਾਂ++;
      p_arg++;
    }

  /* Invoke the closure.  */
  (ਘੇਰਾ->fun) (cif, rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);

  debug(3, "after calling function, ret[0] = %08x, ret[1] = %08x\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  ਚੋਣ (cif->flags)
    {
    ਹਾਲਤ FFI_TYPE_UINT8:
      *(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ) = (UINT8)u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT8:
      *(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ) = (SINT8)u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16:
      *(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ) = (UINT16)u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
      *(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ) = (SINT16)u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_UINT32:
      *(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ) = u.ret[0];
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      *(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ) = u.ret[0];
      *(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ - 1) = u.ret[1];
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_FLOAT:
      fldw(rvalue, fr4);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT1:
    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT2:
    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT3:
    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT4:
      tmp = (ਖਾਲੀ*)(ਢੇਰ -  ਪਹਿਲਾ_ARG_ਥਾਂ);
      tmp += 4 - cif->rtype->ਮਾਪ;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ*)tmp, &u, cif->rtype->ਮਾਪ);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT5:
    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT6:
    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT7:
    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT8:
      {
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ret2[2];
	ਪੂਰਨ off;

	/* Right justify ret[0] and ret[1] */
	ਚੋਣ (cif->flags)
	  {
	    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT5: off = 3; ਰੋਕੋ;
	    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT6: off = 2; ਰੋਕੋ;
	    ਹਾਲਤ FFI_TYPE_SMALL_STRUCT7: off = 1; ਰੋਕੋ;
	    ਮੂਲ: off = 0; ਰੋਕੋ;
	  }

	memset (ret2, 0, ਆਕਾਰ (ret2));
	ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਅੱਖਰ *)ret2 + off, &u, 8 - off);

	*(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ) = ret2[0];
	*(ਢੇਰ - ਪਹਿਲਾ_ARG_ਥਾਂ - 1) = ret2[1];
      }
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
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

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_pa32(ਖਾਲੀ);

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  /* The layout of a function descriptor.  A function pointer with the PLABEL
     bit set points to a function descriptor.  */
  ਬਣਤਰ pa32_fd
  {
    UINT32 ਕੋਡ_ਸੂਚਕ;
    UINT32 gp;
  };

  ਬਣਤਰ ffi_pa32_trampoline_struct
  {
     UINT32 ਕੋਡ_ਸੂਚਕ;        /* Pointer to ffi_closure_unix.  */
     UINT32 fake_gp;             /* Pointer to closure, installed as gp.  */
     UINT32 real_gp;             /* Real gp value.  */
  };

  ਬਣਤਰ ffi_pa32_trampoline_struct *tramp;
  ਬਣਤਰ pa32_fd *fd;

  ਜੇ (cif->abi != FFI_PA32)
    ਵਾਪਸ FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa32.  */
  fd = (ਬਣਤਰ pa32_fd *)((UINT32)ffi_ਘੇਰਾ_pa32 & ~3);

  /* Setup trampoline.  */
  tramp = (ਬਣਤਰ ffi_pa32_trampoline_struct *)ਘੇਰਾ->tramp;
  tramp->ਕੋਡ_ਸੂਚਕ = fd->ਕੋਡ_ਸੂਚਕ;
  tramp->fake_gp = (UINT32)codeloc & ~3;
  tramp->real_gp = fd->gp;

  ਘੇਰਾ->cif  = cif;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  ਘੇਰਾ->fun  = fun;

  ਵਾਪਸ FFI_OK;
}
#ਜੇ_ਅੰਤ
