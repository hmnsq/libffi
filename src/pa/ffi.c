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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdio.h>

#నిర్వచించు విడత_పైకి(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#నిర్వచించు MIN_దొంతర_కొలత  64
#నిర్వచించు మొదటి_ARG_స్థానం  9
#నిర్వచించు DEBUG_LEVEL   0

#నిర్వచించు fldw(addr, fpreg) \
  __asm__ అస్థిర ("fldw 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#నిర్వచించు fstw(fpreg, addr) \
  __asm__ అస్థిర ("fstw %%" #fpreg "L, 0(%0)" : : "r"(addr))
#నిర్వచించు fldd(addr, fpreg) \
  __asm__ అస్థిర ("fldd 0(%0), %%" #fpreg : : "r"(addr) : #fpreg)
#నిర్వచించు fstd(fpreg, addr) \
  __asm__ అస్థిర ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#నిర్వచించు debug(lvl, x...) చేయి { ఒకవేళ (lvl <= DEBUG_LEVEL) { రాయి(x); } } అయితే (0)

స్థిర అంతఃవరుస పూర్ణాంకం ffi_struct_type(ffi_type *t)
{
  size_t sz = t->కొలత;

  /* Small structure results are passed in registers,
     larger ones are passed by pointer.  Note that small
     structures differ from the corresponding integer
     types in that they have different alignment requirements.  */

  ఒకవేళ (sz <= 8)
    తిరిగి -sz;
  లేకపోతే
    తిరిగి FFI_TYPE_STRUCT; /* else, we pass it by pointer.  */
}

/* PA has a downward growing stack, which looks like this:

స్థానభ్రంశం
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

శూన్య ffi_prep_వాదనలు_pa32(UINT32 *దొంతర, extended_cif *ecif, గుర్తులేని బైట్లు)
{
  నమోదు గుర్తులేని పూర్ణాంకం i;
  నమోదు ffi_type **p_arg;
  నమోదు శూన్య **p_argv;
  గుర్తులేని పూర్ణాంకం స్థానం = మొదటి_ARG_స్థానం;
  అక్షరం *dest_cpy;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, దొంతర,
	ecif, బైట్లు);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  కొరకు (i = 0; i < ecif->cif->nargs; i++)
    {
      పూర్ణాంకం type = (*p_arg)->type;

      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_SINT8:
	  *(SINT32 *)(దొంతర - స్థానం) = *(SINT8 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_UINT8:
	  *(UINT32 *)(దొంతర - స్థానం) = *(UINT8 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_SINT16:
	  *(SINT32 *)(దొంతర - స్థానం) = *(SINT16 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_UINT16:
	  *(UINT32 *)(దొంతర - స్థానం) = *(UINT16 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_సూచిక:
	  debug(3, "Storing UINT32 %u in slot %u\n", *(UINT32 *)(*p_argv),
		స్థానం);
	  *(UINT32 *)(దొంతర - స్థానం) = *(UINT32 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	  /* Align slot for 64-bit type.  */
	  స్థానం += (స్థానం & 1) ? 1 : 2;
	  *(UINT64 *)(దొంతర - స్థానం) = *(UINT64 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	  /* First 4 args go in fr4L - fr7L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", స్థానం);
	  *(UINT32 *)(దొంతర - స్థానం) = *(UINT32 *)(*p_argv);
	  ఎంపిక (స్థానం - మొదటి_ARG_స్థానం)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    స్థితి 0: fldw(దొంతర - స్థానం, fr4); ఆపు;
	    స్థితి 1: fldw(దొంతర - స్థానం, fr5); ఆపు;
	    స్థితి 2: fldw(దొంతర - స్థానం, fr6); ఆపు;
	    స్థితి 3: fldw(దొంతర - స్థానం, fr7); ఆపు;
	    }
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  /* Align slot for 64-bit type.  */
	  స్థానం += (స్థానం & 1) ? 1 : 2;
	  debug(3, "Storing UINT64(double) at slot %u\n", స్థానం);
	  *(UINT64 *)(దొంతర - స్థానం) = *(UINT64 *)(*p_argv);
	  ఎంపిక (స్థానం - మొదటి_ARG_స్థానం)
	    {
	      /* First 2 args go in fr5, fr7.  */
	      స్థితి 1: fldd(దొంతర - స్థానం, fr5); ఆపు;
	      స్థితి 3: fldd(దొంతర - స్థానం, fr7); ఆపు;
	    }
	  ఆపు;

#నిర్వచితమైతే PA_HPUX
	స్థితి FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are passed in the same manner as structures
	     larger than 8 bytes.  */
	  *(UINT32 *)(దొంతర - స్థానం) = (UINT32)(*p_argv);
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_STRUCT:

	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */

	  len = (*p_arg)->కొలత;
	  ఒకవేళ (len <= 4)
	    {
	      dest_cpy = (అక్షరం *)(దొంతర - స్థానం) + 4 - len;
	      జ్ఞాపకం_నకలు(dest_cpy, (అక్షరం *)*p_argv, len);
	    }
	  లేకపోతే ఒకవేళ (len <= 8)
	    {
	      స్థానం += (స్థానం & 1) ? 1 : 2;
	      dest_cpy = (అక్షరం *)(దొంతర - స్థానం) + 8 - len;
	      జ్ఞాపకం_నకలు(dest_cpy, (అక్షరం *)*p_argv, len);
	    }
	  లేకపోతే
	    *(UINT32 *)(దొంతర - స్థానం) = (UINT32)(*p_argv);
	  ఆపు;

	అప్రమేయ:
	  FFI_ASSERT(0);
	}

      స్థానం++;
      p_arg++;
      p_argv++;
    }

  /* Make sure we didn't mess up and scribble on the stack.  */
  {
    గుర్తులేని పూర్ణాంకం n;

    debug(5, "Stack setup:\n");
    కొరకు (n = 0; n < (బైట్లు + 3) / 4; n++)
      {
	ఒకవేళ ((n%4) == 0) { debug(5, "\n%08x: ", (గుర్తులేని పూర్ణాంకం)(దొంతర - n)); }
	debug(5, "%08x ", *(దొంతర - n));
      }
    debug(5, "\n");
  }

  FFI_ASSERT(స్థానం * 4 <= బైట్లు);

  తిరిగి;
}

స్థిర శూన్య ffi_కొలత_దొంతర_pa32(ffi_cif *cif)
{
  ffi_type **ptr;
  పూర్ణాంకం i;
  పూర్ణాంకం z = 0; /* # stack slots */

  కొరకు (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      పూర్ణాంకం type = (*ptr)->type;

      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	  z += 2 + (z & 1); /* must start on even regs, so we may waste one */
	  ఆపు;

#నిర్వచితమైతే PA_HPUX
	స్థితి FFI_TYPE_LONGDOUBLE:
	  z += 1; /* passed by pointer, like a large struct */
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_STRUCT:
	  /* This must mirror the slot accounting in ffi_prep_args_pa32:
	     structs of 1-4 bytes occupy one slot, structs of 5-8 bytes are
	     passed inline in two even-aligned slots (exactly like a 64-bit
	     value), and larger structs are passed by pointer in one slot.
	     z stays offset from the marshaller's slot by FIRST_ARG_SLOT (odd),
	     so (z & 1) tracks the same alignment the marshaller applies.  */
	  {
	    size_t len = (*ptr)->కొలత;
	    ఒకవేళ (len <= 4 || len > 8)
	      z += 1;
	    లేకపోతే
	      z += 2 + (z & 1);
	  }
	  ఆపు;

	అప్రమేయ: /* <= 32-bit values */
	  z++;
	}
    }

  /* We can fit up to 6 args in the default 64-byte stack frame,
     if we need more, we need more stack.  */
  ఒకవేళ (z <= 6)
    cif->బైట్లు = MIN_దొంతర_కొలత; /* min stack size */
  లేకపోతే
    cif->బైట్లు = 64 + విడత_పైకి((z - 6) * పరిమాణం(UINT32), MIN_దొంతర_కొలత);

  debug(3, "Calculated stack size is %u bytes\n", cif->బైట్లు);
}

/* Perform machine dependent cif processing.  */
ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag */
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
    స్థితి FFI_TYPE_FLOAT:
    స్థితి FFI_TYPE_DOUBLE:
      cif->flags = (గుర్తులేని) cif->rtype->type;
      ఆపు;

#నిర్వచితమైతే PA_HPUX
    స్థితి FFI_TYPE_LONGDOUBLE:
      /* Long doubles are treated like a structure.  */
      cif->flags = FFI_TYPE_STRUCT;
      ఆపు;
#అయితే_ముగింపు

    స్థితి FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 4 bytes, the result is given back
	 in one register. If the size is smaller or equal 8 bytes than we
	 return the result in two registers. But if the size is bigger than
	 8 bytes, we work with pointers.  */
      cif->flags = ffi_struct_type(cif->rtype);
      ఆపు;

    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
      cif->flags = FFI_TYPE_UINT64;
      ఆపు;

    అప్రమేయ:
      cif->flags = FFI_TYPE_INT;
      ఆపు;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  ఎంపిక (cif->abi)
    {
    స్థితి FFI_PA32:
      ffi_కొలత_దొంతర_pa32(cif);
      ఆపు;

    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }

  తిరిగి FFI_OK;
}

బాహ్య శూన్య ffi_call_pa32(శూన్య (*)(UINT32 *, extended_cif *, గుర్తులేని),
			  extended_cif *, గుర్తులేని, గుర్తులేని, గుర్తులేని *,
			  శూన్య (*fn)(శూన్య));

శూన్య ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  extended_cif ecif;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  శూన్య **avalue_copy = ఖాళీ;

  ecif.cif = cif;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  కొరకు (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      పూర్ణాంకం కొలత = at->కొలత;
      ఒకవేళ (at->type == FFI_TYPE_STRUCT && కొలత > 8)
	{
	  అక్షరం *argcopy = alloca (కొలత);
	  ఒకవేళ (avalue_copy == ఖాళీ)
	    {
	      avalue_copy = alloca (nargs * పరిమాణం (శూన్య *));
	      జ్ఞాపకం_నకలు (avalue_copy, avalue, nargs * పరిమాణం (శూన్య *));
	      avalue = avalue_copy;
	    }
	  జ్ఞాపకం_నకలు (argcopy, avalue[i], కొలత);
	  avalue[i] = argcopy;
	}
    }
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ఒకవేళ (rvalue == ఖాళీ
#నిర్వచితమైతే PA_HPUX
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_LONGDOUBLE))
#లేకపోతే
      && cif->rtype->type == FFI_TYPE_STRUCT)
#అయితే_ముగింపు
    {
      ecif.rvalue = alloca(cif->rtype->కొలత);
    }
  లేకపోతే
    ecif.rvalue = rvalue;


  ఎంపిక (cif->abi)
    {
    స్థితి FFI_PA32:
      debug(3, "Calling ffi_call_pa32: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->బైట్లు, cif->flags, ecif.rvalue, (శూన్య *)fn);
      ffi_call_pa32(ffi_prep_వాదనలు_pa32, &ecif, cif->బైట్లు,
		     cif->flags, ecif.rvalue, fn);
      ఆపు;

    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }
}

#ఒకవేళ FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_స్థితి ffi_సంవృతం_inner_pa32(ffi_సంవృతం *సంవృతం, UINT32 *దొంతర)
{
  ffi_cif *cif;
  శూన్య **avalue;
  శూన్య *rvalue;
  /* Functions can return up to 64-bits in registers.  Return address
     must be double word aligned.  */
  సంఘం { ద్విగుణ rd; UINT32 ret[2]; } u;
  ffi_type **p_arg;
  అక్షరం *tmp;
  పూర్ణాంకం i, avn;
  గుర్తులేని పూర్ణాంకం స్థానం = మొదటి_ARG_స్థానం;
  నమోదు UINT32 r28 __asm__("r28");

  cif = సంవృతం->cif;

  /* If returning via structure, callee will write to our pointer.  */
  ఒకవేళ (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (శూన్య *)r28;
  లేకపోతే
    rvalue = &u;

  avalue = (శూన్య **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  కొరకు (i = 0; i < avn; i++)
    {
      పూర్ణాంకం type = (*p_arg)->type;

      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_సూచిక:
	  avalue[i] = (అక్షరం *)(దొంతర - స్థానం) + పరిమాణం(UINT32) - (*p_arg)->కొలత;
	  ఆపు;

	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	  స్థానం += (స్థానం & 1) ? 1 : 2;
	  avalue[i] = (శూన్య *)(దొంతర - స్థానం);
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
#నిర్వచితమైతే PA_LINUX
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  ఎంపిక (స్థానం - మొదటి_ARG_స్థానం)
	    {
	    స్థితి 0: fstw(fr4, (శూన్య *)(దొంతర - స్థానం)); ఆపు;
	    స్థితి 1: fstw(fr5, (శూన్య *)(దొంతర - స్థానం)); ఆపు;
	    స్థితి 2: fstw(fr6, (శూన్య *)(దొంతర - స్థానం)); ఆపు;
	    స్థితి 3: fstw(fr7, (శూన్య *)(దొంతర - స్థానం)); ఆపు;
	    }
#అయితే_ముగింపు
	  avalue[i] = (శూన్య *)(దొంతర - స్థానం);
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  స్థానం += (స్థానం & 1) ? 1 : 2;
#నిర్వచితమైతే PA_LINUX
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  ఎంపిక (స్థానం - మొదటి_ARG_స్థానం)
	    {
	    స్థితి 1: fstd(fr5, (శూన్య *)(దొంతర - స్థానం)); ఆపు;
	    స్థితి 3: fstd(fr7, (శూన్య *)(దొంతర - స్థానం)); ఆపు;
	    }
#అయితే_ముగింపు
	  avalue[i] = (శూన్య *)(దొంతర - స్థానం);
	  ఆపు;

#నిర్వచితమైతే PA_HPUX
	స్థితి FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  avalue[i] = (శూన్య *) *(దొంతర - స్థానం);
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_STRUCT:
	  /* Structs smaller or equal than 4 bytes are passed in one
	     register. Structs smaller or equal 8 bytes are passed in two
	     registers. Larger structures are passed by pointer.  */
	  ఒకవేళ((*p_arg)->కొలత <= 4)
	    {
	      avalue[i] = (శూన్య *)(దొంతర - స్థానం) + పరిమాణం(UINT32) -
		(*p_arg)->కొలత;
	    }
	  లేకపోతే ఒకవేళ ((*p_arg)->కొలత <= 8)
	    {
	      స్థానం += (స్థానం & 1) ? 1 : 2;
	      avalue[i] = (శూన్య *)(దొంతర - స్థానం) + పరిమాణం(UINT64) -
		(*p_arg)->కొలత;
	    }
	  లేకపోతే
	    avalue[i] = (శూన్య *) *(దొంతర - స్థానం);
	  ఆపు;

	అప్రమేయ:
	  FFI_ASSERT(0);
	}

      స్థానం++;
      p_arg++;
    }

  /* Invoke the closure.  */
  (సంవృతం->fun) (cif, rvalue, avalue, సంవృతం->user_దత్తాంశం);

  debug(3, "after calling function, ret[0] = %08x, ret[1] = %08x\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  ఎంపిక (cif->flags)
    {
    స్థితి FFI_TYPE_UINT8:
      *(దొంతర - మొదటి_ARG_స్థానం) = (UINT8)u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_SINT8:
      *(దొంతర - మొదటి_ARG_స్థానం) = (SINT8)u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_UINT16:
      *(దొంతర - మొదటి_ARG_స్థానం) = (UINT16)u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_SINT16:
      *(దొంతర - మొదటి_ARG_స్థానం) = (SINT16)u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_UINT32:
      *(దొంతర - మొదటి_ARG_స్థానం) = u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      *(దొంతర - మొదటి_ARG_స్థానం) = u.ret[0];
      *(దొంతర - మొదటి_ARG_స్థానం - 1) = u.ret[1];
      ఆపు;

    స్థితి FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      ఆపు;

    స్థితి FFI_TYPE_FLOAT:
      fldw(rvalue, fr4);
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      ఆపు;

    స్థితి FFI_TYPE_SMALL_STRUCT1:
    స్థితి FFI_TYPE_SMALL_STRUCT2:
    స్థితి FFI_TYPE_SMALL_STRUCT3:
    స్థితి FFI_TYPE_SMALL_STRUCT4:
      tmp = (శూన్య*)(దొంతర -  మొదటి_ARG_స్థానం);
      tmp += 4 - cif->rtype->కొలత;
      జ్ఞాపకం_నకలు((శూన్య*)tmp, &u, cif->rtype->కొలత);
      ఆపు;

    స్థితి FFI_TYPE_SMALL_STRUCT5:
    స్థితి FFI_TYPE_SMALL_STRUCT6:
    స్థితి FFI_TYPE_SMALL_STRUCT7:
    స్థితి FFI_TYPE_SMALL_STRUCT8:
      {
	గుర్తులేని పూర్ణాంకం ret2[2];
	పూర్ణాంకం off;

	/* Right justify ret[0] and ret[1] */
	ఎంపిక (cif->flags)
	  {
	    స్థితి FFI_TYPE_SMALL_STRUCT5: off = 3; ఆపు;
	    స్థితి FFI_TYPE_SMALL_STRUCT6: off = 2; ఆపు;
	    స్థితి FFI_TYPE_SMALL_STRUCT7: off = 1; ఆపు;
	    అప్రమేయ: off = 0; ఆపు;
	  }

	memset (ret2, 0, పరిమాణం (ret2));
	జ్ఞాపకం_నకలు ((అక్షరం *)ret2 + off, &u, 8 - off);

	*(దొంతర - మొదటి_ARG_స్థానం) = ret2[0];
	*(దొంతర - మొదటి_ARG_స్థానం - 1) = ret2[1];
      }
      ఆపు;

    స్థితి FFI_TYPE_సూచిక:
    స్థితి FFI_TYPE_VOID:
      ఆపు;

    అప్రమేయ:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      ఆపు;
    }
  తిరిగి FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

బాహ్య శూన్య ffi_సంవృతం_pa32(శూన్య);

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  /* The layout of a function descriptor.  A function pointer with the PLABEL
     bit set points to a function descriptor.  */
  నిర్మాణం pa32_fd
  {
    UINT32 సంకేతం_సూచిక;
    UINT32 gp;
  };

  నిర్మాణం ffi_pa32_trampoline_struct
  {
     UINT32 సంకేతం_సూచిక;        /* Pointer to ffi_closure_unix.  */
     UINT32 fake_gp;             /* Pointer to closure, installed as gp.  */
     UINT32 real_gp;             /* Real gp value.  */
  };

  నిర్మాణం ffi_pa32_trampoline_struct *tramp;
  నిర్మాణం pa32_fd *fd;

  ఒకవేళ (cif->abi != FFI_PA32)
    తిరిగి FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa32.  */
  fd = (నిర్మాణం pa32_fd *)((UINT32)ffi_సంవృతం_pa32 & ~3);

  /* Setup trampoline.  */
  tramp = (నిర్మాణం ffi_pa32_trampoline_struct *)సంవృతం->tramp;
  tramp->సంకేతం_సూచిక = fd->సంకేతం_సూచిక;
  tramp->fake_gp = (UINT32)codeloc & ~3;
  tramp->real_gp = fd->gp;

  సంవృతం->cif  = cif;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  సంవృతం->fun  = fun;

  తిరిగి FFI_OK;
}
#అయితే_ముగింపు
