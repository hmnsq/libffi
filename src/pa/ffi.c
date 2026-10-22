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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>

#define ROUND_UP(v, a)  (((माप_प्रकार)(v) + (a) - 1) & ~((a) - 1))

#define MIN_चळत_SIZE  64
#define FIRST_ARG_SLOT  9
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ अस्थिर ("fldw 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ अस्थिर ("fstw %%" #fpreg "L, 0(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ अस्थिर ("fldd 0(%0), %%" #fpreg : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ अस्थिर ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) करा { जर (lvl <= DEBUG_LEVEL) { म_लिखो(x); } } पर्यंत (0)

स्थिर ओळीत पूर्णांक ffi_struct_type(ffi_type *t)
{
  माप_प्रकार sz = t->size;

  /* Small structure results are passed in registers,
     larger ones are passed by pointer.  Note that small
     structures differ from the corresponding integer
     types in that they have different alignment requirements.  */

  जर (sz <= 8)
    परत -sz;
  अन्यथा
    परत FFI_TYPE_STRUCT; /* else, we pass it by pointer.  */
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

शून्य ffi_prep_args_pa32(UINT32 *चळत, extended_cif *ecif, अचिन्हित bytes)
{
  नोंदणी अचिन्हित पूर्णांक i;
  नोंदणी ffi_type **p_arg;
  नोंदणी शून्य **p_argv;
  अचिन्हित पूर्णांक slot = FIRST_ARG_SLOT;
  अक्षर *dest_cpy;
  माप_प्रकार len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, चळत,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  साठी (i = 0; i < ecif->cif->nargs; i++)
    {
      पूर्णांक type = (*p_arg)->type;

      निवड (type)
	{
	स्थिती FFI_TYPE_SINT8:
	  *(SINT32 *)(चळत - slot) = *(SINT8 *)(*p_argv);
	  थांबा;

	स्थिती FFI_TYPE_UINT8:
	  *(UINT32 *)(चळत - slot) = *(UINT8 *)(*p_argv);
	  थांबा;

	स्थिती FFI_TYPE_SINT16:
	  *(SINT32 *)(चळत - slot) = *(SINT16 *)(*p_argv);
	  थांबा;

	स्थिती FFI_TYPE_UINT16:
	  *(UINT32 *)(चळत - slot) = *(UINT16 *)(*p_argv);
	  थांबा;

	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_POINTER:
	  debug(3, "Storing UINT32 %u in slot %u\n", *(UINT32 *)(*p_argv),
		slot);
	  *(UINT32 *)(चळत - slot) = *(UINT32 *)(*p_argv);
	  थांबा;

	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	  /* Align slot for 64-bit type.  */
	  slot += (slot & 1) ? 1 : 2;
	  *(UINT64 *)(चळत - slot) = *(UINT64 *)(*p_argv);
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
	  /* First 4 args go in fr4L - fr7L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT32 *)(चळत - slot) = *(UINT32 *)(*p_argv);
	  निवड (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    स्थिती 0: fldw(चळत - slot, fr4); थांबा;
	    स्थिती 1: fldw(चळत - slot, fr5); थांबा;
	    स्थिती 2: fldw(चळत - slot, fr6); थांबा;
	    स्थिती 3: fldw(चळत - slot, fr7); थांबा;
	    }
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	  /* Align slot for 64-bit type.  */
	  slot += (slot & 1) ? 1 : 2;
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(चळत - slot) = *(UINT64 *)(*p_argv);
	  निवड (slot - FIRST_ARG_SLOT)
	    {
	      /* First 2 args go in fr5, fr7.  */
	      स्थिती 1: fldd(चळत - slot, fr5); थांबा;
	      स्थिती 3: fldd(चळत - slot, fr7); थांबा;
	    }
	  थांबा;

#ifdef PA_HPUX
	स्थिती FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are passed in the same manner as structures
	     larger than 8 bytes.  */
	  *(UINT32 *)(चळत - slot) = (UINT32)(*p_argv);
	  थांबा;
#endif

	स्थिती FFI_TYPE_STRUCT:

	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */

	  len = (*p_arg)->size;
	  जर (len <= 4)
	    {
	      dest_cpy = (अक्षर *)(चळत - slot) + 4 - len;
	      स_नकल(dest_cpy, (अक्षर *)*p_argv, len);
	    }
	  अन्यथा जर (len <= 8)
	    {
	      slot += (slot & 1) ? 1 : 2;
	      dest_cpy = (अक्षर *)(चळत - slot) + 8 - len;
	      स_नकल(dest_cpy, (अक्षर *)*p_argv, len);
	    }
	  अन्यथा
	    *(UINT32 *)(चळत - slot) = (UINT32)(*p_argv);
	  थांबा;

	पूर्वनिर्धारित:
	  FFI_ASSERT(0);
	}

      slot++;
      p_arg++;
      p_argv++;
    }

  /* Make sure we didn't mess up and scribble on the stack.  */
  {
    अचिन्हित पूर्णांक n;

    debug(5, "Stack setup:\n");
    साठी (n = 0; n < (bytes + 3) / 4; n++)
      {
	जर ((n%4) == 0) { debug(5, "\n%08x: ", (अचिन्हित पूर्णांक)(चळत - n)); }
	debug(5, "%08x ", *(चळत - n));
      }
    debug(5, "\n");
  }

  FFI_ASSERT(slot * 4 <= bytes);

  परत;
}

स्थिर शून्य ffi_size_चळत_pa32(ffi_cif *cif)
{
  ffi_type **ptr;
  पूर्णांक i;
  पूर्णांक z = 0; /* # stack slots */

  साठी (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      पूर्णांक type = (*ptr)->type;

      निवड (type)
	{
	स्थिती FFI_TYPE_DOUBLE:
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	  z += 2 + (z & 1); /* must start on even regs, so we may waste one */
	  थांबा;

#ifdef PA_HPUX
	स्थिती FFI_TYPE_LONGDOUBLE:
	  z += 1; /* passed by pointer, like a large struct */
	  थांबा;
#endif

	स्थिती FFI_TYPE_STRUCT:
	  /* This must mirror the slot accounting in ffi_prep_args_pa32:
	     structs of 1-4 bytes occupy one slot, structs of 5-8 bytes are
	     passed inline in two even-aligned slots (exactly like a 64-bit
	     value), and larger structs are passed by pointer in one slot.
	     z stays offset from the marshaller's slot by FIRST_ARG_SLOT (odd),
	     so (z & 1) tracks the same alignment the marshaller applies.  */
	  {
	    माप_प्रकार len = (*ptr)->size;
	    जर (len <= 4 || len > 8)
	      z += 1;
	    अन्यथा
	      z += 2 + (z & 1);
	  }
	  थांबा;

	पूर्वनिर्धारित: /* <= 32-bit values */
	  z++;
	}
    }

  /* We can fit up to 6 args in the default 64-byte stack frame,
     if we need more, we need more stack.  */
  जर (z <= 6)
    cif->bytes = MIN_चळत_SIZE; /* min stack size */
  अन्यथा
    cif->bytes = 64 + ROUND_UP((z - 6) * आकार(UINT32), MIN_चळत_SIZE);

  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag */
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
    स्थिती FFI_TYPE_FLOAT:
    स्थिती FFI_TYPE_DOUBLE:
      cif->flags = (अचिन्हित) cif->rtype->type;
      थांबा;

#ifdef PA_HPUX
    स्थिती FFI_TYPE_LONGDOUBLE:
      /* Long doubles are treated like a structure.  */
      cif->flags = FFI_TYPE_STRUCT;
      थांबा;
#endif

    स्थिती FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 4 bytes, the result is given back
	 in one register. If the size is smaller or equal 8 bytes than we
	 return the result in two registers. But if the size is bigger than
	 8 bytes, we work with pointers.  */
      cif->flags = ffi_struct_type(cif->rtype);
      थांबा;

    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64;
      थांबा;

    पूर्वनिर्धारित:
      cif->flags = FFI_TYPE_INT;
      थांबा;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  निवड (cif->abi)
    {
    स्थिती FFI_PA32:
      ffi_size_चळत_pa32(cif);
      थांबा;

    पूर्वनिर्धारित:
      FFI_ASSERT(0);
      थांबा;
    }

  परत FFI_OK;
}

बाह्य शून्य ffi_call_pa32(शून्य (*)(UINT32 *, extended_cif *, अचिन्हित),
			  extended_cif *, अचिन्हित, अचिन्हित, अचिन्हित *,
			  शून्य (*fn)(शून्य));

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;
  माप_प्रकार i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue_copy = रिक्त;

  ecif.cif = cif;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  साठी (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णांक size = at->size;
      जर (at->type == FFI_TYPE_STRUCT && size > 8)
	{
	  अक्षर *argcopy = alloca (size);
	  जर (avalue_copy == रिक्त)
	    {
	      avalue_copy = alloca (nargs * आकार (शून्य *));
	      स_नकल (avalue_copy, avalue, nargs * आकार (शून्य *));
	      avalue = avalue_copy;
	    }
	  स_नकल (argcopy, avalue[i], size);
	  avalue[i] = argcopy;
	}
    }
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  जर (rvalue == रिक्त
#ifdef PA_HPUX
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_LONGDOUBLE))
#अन्यथा
      && cif->rtype->type == FFI_TYPE_STRUCT)
#endif
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;


  निवड (cif->abi)
    {
    स्थिती FFI_PA32:
      debug(3, "Calling ffi_call_pa32: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (शून्य *)fn);
      ffi_call_pa32(ffi_prep_args_pa32, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      थांबा;

    पूर्वनिर्धारित:
      FFI_ASSERT(0);
      थांबा;
    }
}

#जर FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_status ffi_closure_inner_pa32(ffi_closure *closure, UINT32 *चळत)
{
  ffi_cif *cif;
  शून्य **avalue;
  शून्य *rvalue;
  /* Functions can return up to 64-bits in registers.  Return address
     must be double word aligned.  */
  संघ { द्विगुण rd; UINT32 ret[2]; } u;
  ffi_type **p_arg;
  अक्षर *tmp;
  पूर्णांक i, avn;
  अचिन्हित पूर्णांक slot = FIRST_ARG_SLOT;
  नोंदणी UINT32 r28 __asm__("r28");

  cif = closure->cif;

  /* If returning via structure, callee will write to our pointer.  */
  जर (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (शून्य *)r28;
  अन्यथा
    rvalue = &u;

  avalue = (शून्य **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  साठी (i = 0; i < avn; i++)
    {
      पूर्णांक type = (*p_arg)->type;

      निवड (type)
	{
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_POINTER:
	  avalue[i] = (अक्षर *)(चळत - slot) + आकार(UINT32) - (*p_arg)->size;
	  थांबा;

	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
	  slot += (slot & 1) ? 1 : 2;
	  avalue[i] = (शून्य *)(चळत - slot);
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
#ifdef PA_LINUX
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  निवड (slot - FIRST_ARG_SLOT)
	    {
	    स्थिती 0: fstw(fr4, (शून्य *)(चळत - slot)); थांबा;
	    स्थिती 1: fstw(fr5, (शून्य *)(चळत - slot)); थांबा;
	    स्थिती 2: fstw(fr6, (शून्य *)(चळत - slot)); थांबा;
	    स्थिती 3: fstw(fr7, (शून्य *)(चळत - slot)); थांबा;
	    }
#endif
	  avalue[i] = (शून्य *)(चळत - slot);
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	  slot += (slot & 1) ? 1 : 2;
#ifdef PA_LINUX
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  निवड (slot - FIRST_ARG_SLOT)
	    {
	    स्थिती 1: fstd(fr5, (शून्य *)(चळत - slot)); थांबा;
	    स्थिती 3: fstd(fr7, (शून्य *)(चळत - slot)); थांबा;
	    }
#endif
	  avalue[i] = (शून्य *)(चळत - slot);
	  थांबा;

#ifdef PA_HPUX
	स्थिती FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  avalue[i] = (शून्य *) *(चळत - slot);
	  थांबा;
#endif

	स्थिती FFI_TYPE_STRUCT:
	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */
	  जर((*p_arg)->size <= 4)
	    {
	      avalue[i] = (शून्य *)(चळत - slot) + आकार(UINT32) -
		(*p_arg)->size;
	    }
	  अन्यथा जर ((*p_arg)->size <= 8)
	    {
	      slot += (slot & 1) ? 1 : 2;
	      avalue[i] = (शून्य *)(चळत - slot) + आकार(UINT64) -
		(*p_arg)->size;
	    }
	  अन्यथा
	    avalue[i] = (शून्य *) *(चळत - slot);
	  थांबा;

	पूर्वनिर्धारित:
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
  निवड (cif->flags)
    {
    स्थिती FFI_TYPE_UINT8:
      *(चळत - FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      थांबा;
    स्थिती FFI_TYPE_SINT8:
      *(चळत - FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      थांबा;
    स्थिती FFI_TYPE_UINT16:
      *(चळत - FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      थांबा;
    स्थिती FFI_TYPE_SINT16:
      *(चळत - FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      थांबा;
    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_UINT32:
      *(चळत - FIRST_ARG_SLOT) = u.ret[0];
      थांबा;
    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
      *(चळत - FIRST_ARG_SLOT) = u.ret[0];
      *(चळत - FIRST_ARG_SLOT - 1) = u.ret[1];
      थांबा;

    स्थिती FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      थांबा;

    स्थिती FFI_TYPE_FLOAT:
      fldw(rvalue, fr4);
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      थांबा;

    स्थिती FFI_TYPE_SMALL_STRUCT1:
    स्थिती FFI_TYPE_SMALL_STRUCT2:
    स्थिती FFI_TYPE_SMALL_STRUCT3:
    स्थिती FFI_TYPE_SMALL_STRUCT4:
      tmp = (शून्य*)(चळत -  FIRST_ARG_SLOT);
      tmp += 4 - cif->rtype->size;
      स_नकल((शून्य*)tmp, &u, cif->rtype->size);
      थांबा;

    स्थिती FFI_TYPE_SMALL_STRUCT5:
    स्थिती FFI_TYPE_SMALL_STRUCT6:
    स्थिती FFI_TYPE_SMALL_STRUCT7:
    स्थिती FFI_TYPE_SMALL_STRUCT8:
      {
	अचिन्हित पूर्णांक ret2[2];
	पूर्णांक off;

	/* Right justify ret[0] and ret[1] */
	निवड (cif->flags)
	  {
	    स्थिती FFI_TYPE_SMALL_STRUCT5: off = 3; थांबा;
	    स्थिती FFI_TYPE_SMALL_STRUCT6: off = 2; थांबा;
	    स्थिती FFI_TYPE_SMALL_STRUCT7: off = 1; थांबा;
	    पूर्वनिर्धारित: off = 0; थांबा;
	  }

	स_रखो (ret2, 0, आकार (ret2));
	स_नकल ((अक्षर *)ret2 + off, &u, 8 - off);

	*(चळत - FIRST_ARG_SLOT) = ret2[0];
	*(चळत - FIRST_ARG_SLOT - 1) = ret2[1];
      }
      थांबा;

    स्थिती FFI_TYPE_POINTER:
    स्थिती FFI_TYPE_VOID:
      थांबा;

    पूर्वनिर्धारित:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      थांबा;
    }
  परत FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

बाह्य शून्य ffi_closure_pa32(शून्य);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  /* The layout of a function descriptor.  A function pointer with the PLABEL
     bit set points to a function descriptor.  */
  संरचना pa32_fd
  {
    UINT32 code_pointer;
    UINT32 gp;
  };

  संरचना ffi_pa32_trampoline_struct
  {
     UINT32 code_pointer;        /* Pointer to ffi_closure_unix.  */
     UINT32 fake_gp;             /* Pointer to closure, installed as gp.  */
     UINT32 real_gp;             /* Real gp value.  */
  };

  संरचना ffi_pa32_trampoline_struct *tramp;
  संरचना pa32_fd *fd;

  जर (cif->abi != FFI_PA32)
    परत FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa32.  */
  fd = (संरचना pa32_fd *)((UINT32)ffi_closure_pa32 & ~3);

  /* Setup trampoline.  */
  tramp = (संरचना ffi_pa32_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT32)codeloc & ~3;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  परत FFI_OK;
}
#endif
