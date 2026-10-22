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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdio.h>

#నిర్వచించు విడత_పైకి(v, a)  (((size_t)(v) + (a) - 1) & ~((a) - 1))

#నిర్వచించు మొదటి_ARG_స్థానం  0
#నిర్వచించు DEBUG_LEVEL   0

#నిర్వచించు fldw(addr, fpreg) \
  __asm__ అస్థిర ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#నిర్వచించు fstw(fpreg, addr) \
  __asm__ అస్థిర ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#నిర్వచించు fldd(addr, fpreg) \
  __asm__ అస్థిర ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#నిర్వచించు fstd(fpreg, addr) \
  __asm__ అస్థిర ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#నిర్వచించు debug(lvl, x...) చేయి { ఒకవేళ (lvl <= DEBUG_LEVEL) { రాయి(x); } } అయితే (0)

స్థిర అంతఃవరుస పూర్ణాంకం ffi_struct_type(ffi_type *t)
{
  పూర్ణాంకం sz = t->కొలత;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  ఒకవేళ (sz <= 16)
    తిరిగి -sz;
  లేకపోతే
    తిరిగి FFI_TYPE_STRUCT;
}

/* PA has a downward growing stack, which looks like this.  Stack
   arguments are offset from the argument ponter (AP) in r29.

స్థానభ్రంశం
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

శూన్య ffi_prep_వాదనలు_pa64(UINT64 *దొంతర, extended_cif *ecif, గుర్తులేని బైట్లు)
{
  నమోదు గుర్తులేని పూర్ణాంకం i;
  నమోదు ffi_type **p_arg;
  నమోదు శూన్య **p_argv;
  గుర్తులేని పూర్ణాంకం స్థానం = మొదటి_ARG_స్థానం;
  size_t len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, దొంతర,
	ecif, బైట్లు);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  కొరకు (i = 0; i < ecif->cif->nargs; i++)
    {
      పూర్ణాంకం type = (*p_arg)->type;

      len = (*p_arg)->కొలత;

      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_SINT8:
	  *(SINT64 *)(దొంతర + స్థానం) = *(SINT8 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_UINT8:
	  *(UINT64 *)(దొంతర + స్థానం) = *(UINT8 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_SINT16:
	  *(SINT64 *)(దొంతర + స్థానం) = *(SINT16 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_UINT16:
	  *(UINT64 *)(దొంతర + స్థానం) = *(UINT16 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_SINT32:
	  *(SINT64 *)(దొంతర + స్థానం) = *(SINT32 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_UINT32:
	  *(UINT64 *)(దొంతర + స్థానం) = *(UINT32 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_సూచిక:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		స్థానం);
	  *(UINT64 *)(దొంతర + స్థానం) = *(UINT64 *)(*p_argv);
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", స్థానం);
	  *(UINT64 *)(దొంతర + స్థానం) = *(UINT32 *)(*p_argv);
	  ఎంపిక (స్థానం - మొదటి_ARG_స్థానం)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    స్థితి 0: fldw(దొంతర + స్థానం, fr4); ఆపు;
	    స్థితి 1: fldw(దొంతర + స్థానం, fr5); ఆపు;
	    స్థితి 2: fldw(దొంతర + స్థానం, fr6); ఆపు;
	    స్థితి 3: fldw(దొంతర + స్థానం, fr7); ఆపు;
	    స్థితి 4: fldw(దొంతర + స్థానం, fr8); ఆపు;
	    స్థితి 5: fldw(దొంతర + స్థానం, fr9); ఆపు;
	    స్థితి 6: fldw(దొంతర + స్థానం, fr10); ఆపు;
	    స్థితి 7: fldw(దొంతర + స్థానం, fr11); ఆపు;
	    }
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", స్థానం);
	  *(UINT64 *)(దొంతర + స్థానం) = *(UINT64 *)(*p_argv);
	  ఎంపిక (స్థానం - మొదటి_ARG_స్థానం)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    స్థితి 0: fldd(దొంతర + స్థానం, fr4); ఆపు;
	    స్థితి 1: fldd(దొంతర + స్థానం, fr5); ఆపు;
	    స్థితి 2: fldd(దొంతర + స్థానం, fr6); ఆపు;
	    స్థితి 3: fldd(దొంతర + స్థానం, fr7); ఆపు;
	    స్థితి 4: fldd(దొంతర + స్థానం, fr8); ఆపు;
	    స్థితి 5: fldd(దొంతర + స్థానం, fr9); ఆపు;
	    స్థితి 6: fldd(దొంతర + స్థానం, fr10); ఆపు;
	    స్థితి 7: fldd(దొంతర + స్థానం, fr11); ఆపు;
	    }
	  ఆపు;

#నిర్వచితమైతే PA64_HPUX
	స్థితి FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  స్థానం += (స్థానం & 1);
	  *(UINT64 *)(దొంతర + స్థానం) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(దొంతర + స్థానం + 1) = *(UINT64 *)(*p_argv + 8);
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  ఒకవేళ (len > 8)
	    స్థానం += (స్థానం & 1);
	  జ్ఞాపకం_నకలు((అక్షరం *)(దొంతర + స్థానం), (అక్షరం *)*p_argv, len);
	  ఆపు;

	అప్రమేయ:
	  FFI_ASSERT(0);
	}

      స్థానం += విడత_పైకి (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(స్థానం * 8 <= బైట్లు);

  తిరిగి;
}

స్థిర శూన్య ffi_కొలత_దొంతర_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  పూర్ణాంకం i;
  పూర్ణాంకం z = 0; /* # stack slots */

  కొరకు (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      పూర్ణాంకం type = (*ptr)->type;
      పూర్ణాంకం కొలత = (*ptr)->కొలత;

      ఎంపిక (type)
	{
#నిర్వచితమైతే PA64_HPUX
	స్థితి FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_STRUCT:
	  ఒకవేళ (కొలత > 8)
	    z += (z & 1);
	  z += విడత_పైకి (కొలత, 8) >> 3;
	  ఆపు;

	అప్రమేయ: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  ఒకవేళ (z <= 8)
    z = 8;
  లేకపోతే
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->బైట్లు = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->బైట్లు);
}

/* Perform machine dependent cif processing.  */
ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_COMPLEX:
    స్థితి FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      ఆపు;

    అప్రమేయ:
      cif->flags = (గుర్తులేని) cif->rtype->type;
      ఆపు;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  ఎంపిక (cif->abi)
    {
    స్థితి FFI_PA64:
      ffi_కొలత_దొంతర_pa64(cif);
      ఆపు;

    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }

  తిరిగి FFI_OK;
}

బాహ్య శూన్య ffi_call_pa64(శూన్య (*)(UINT64 *, extended_cif *, గుర్తులేని),
			  extended_cif *, గుర్తులేని, గుర్తులేని, గుర్తులేని *,
			  శూన్య (*fn)(శూన్య));

శూన్య ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ఒకవేళ (rvalue == ఖాళీ
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->కొలత > 16)
    ecif.rvalue = alloca(విడత_పైకి (cif->rtype->కొలత, 16));
  లేకపోతే
    ecif.rvalue = rvalue;


  ఎంపిక (cif->abi)
    {
    స్థితి FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->బైట్లు, cif->flags, ecif.rvalue, (శూన్య *)fn);
      ffi_call_pa64(ffi_prep_వాదనలు_pa64, &ecif, cif->బైట్లు,
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
ffi_స్థితి ffi_సంవృతం_inner_pa64(ffi_సంవృతం *సంవృతం, UINT64 *దొంతర)
{
  ffi_cif *cif;
  శూన్య **avalue;
  శూన్య *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  సంఘం { దీర్ఘ ద్విగుణ rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  అక్షరం *tmp;
  పూర్ణాంకం i, avn;
  గుర్తులేని పూర్ణాంకం స్థానం = మొదటి_ARG_స్థానం;
  నమోదు UINT64 r28 __asm__("r28");

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
	  avalue[i] = (శూన్య *)(దొంతర + స్థానం) + 7;
	  ఆపు;

	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
	  avalue[i] = (శూన్య *)(దొంతర + స్థానం) + 6;
	  ఆపు;

	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
	  avalue[i] = (శూన్య *)(దొంతర + స్థానం) + 4;
	  ఆపు;

	స్థితి FFI_TYPE_సూచిక:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	  avalue[i] = (శూన్య *)(దొంతర + స్థానం);
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  ఎంపిక (స్థానం + మొదటి_ARG_స్థానం)
	    {
	    స్థితి 0: fstw(fr4, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 1: fstw(fr5, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 2: fstw(fr6, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 3: fstw(fr7, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 4: fstw(fr8, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 5: fstw(fr9, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 6: fstw(fr10, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 7: fstw(fr11, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    }
	  avalue[i] = (శూన్య *)(దొంతర + స్థానం) + 4;
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  ఎంపిక (స్థానం + మొదటి_ARG_స్థానం)
	    {
	    స్థితి 0: fstd(fr4, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 1: fstd(fr5, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 2: fstd(fr6, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 3: fstd(fr7, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 4: fstd(fr8, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 5: fstd(fr9, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 6: fstd(fr10, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    స్థితి 7: fstd(fr11, (శూన్య *)(దొంతర + స్థానం)); ఆపు;
	    }
	  avalue[i] = (శూన్య *)(దొంతర + స్థానం);
	  ఆపు;

#నిర్వచితమైతే PA64_HPUX
	స్థితి FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  స్థానం += (స్థానం & 1);
	  avalue[i] = (శూన్య *)(దొంతర + స్థానం);
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  ఒకవేళ((*p_arg)->కొలత > 8)
	    స్థానం += (స్థానం & 1);
	  avalue[i] = (శూన్య *) (దొంతర + స్థానం);
	  ఆపు;

	అప్రమేయ:
	  FFI_ASSERT(0);
	}

      స్థానం += (విడత_పైకి ((*p_arg)->కొలత, 8) >> 3);
      p_arg++;
    }

  /* Invoke the closure.  */
  (సంవృతం->fun) (cif, rvalue, avalue, సంవృతం->user_దత్తాంశం);

  debug(3, "after calling function, ret[0] = %16lx, ret[1] = %16lx\n", u.ret[0],
	u.ret[1]);

  /* Store the result using the lower 2 bytes of the flags.  */
  ఎంపిక (cif->flags)
    {
    స్థితి FFI_TYPE_UINT8:
      *(దొంతర + మొదటి_ARG_స్థానం) = (UINT8)u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_SINT8:
      *(దొంతర + మొదటి_ARG_స్థానం) = (SINT8)u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_UINT16:
      *(దొంతర + మొదటి_ARG_స్థానం) = (UINT16)u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_SINT16:
      *(దొంతర + మొదటి_ARG_స్థానం) = (SINT16)u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
      *(దొంతర + మొదటి_ARG_స్థానం) = (SINT32)u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_UINT32:
      *(దొంతర - మొదటి_ARG_స్థానం) = (UINT32)u.ret[0];
      ఆపు;
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_సూచిక:
      *(దొంతర - మొదటి_ARG_స్థానం) = u.ret[0];
      ఆపు;

    స్థితి FFI_TYPE_LONGDOUBLE:
      *(దొంతర + మొదటి_ARG_స్థానం) = u.ret[0];
      *(దొంతర + మొదటి_ARG_స్థానం + 1) = u.ret[1];
      ఆపు;

    స్థితి FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      ఆపు;

    స్థితి FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      ఆపు;

    స్థితి -1:
    స్థితి -2:
    స్థితి -3:
    స్థితి -4:
    స్థితి -5:
    స్థితి -6:
    స్థితి -7:
    స్థితి -8:
    స్థితి -9:
    స్థితి -10:
    స్థితి -11:
    స్థితి -12:
    స్థితి -13:
    స్థితి -14:
    స్థితి -15:
    స్థితి -16:
      tmp = (శూన్య*)(దొంతర + మొదటి_ARG_స్థానం);
      జ్ఞాపకం_నకలు((శూన్య*)tmp, &u, cif->rtype->కొలత);
      ఆపు;

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

బాహ్య శూన్య ffi_సంవృతం_pa64(శూన్య);

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  /* The layout of a function descriptor.  */
  నిర్మాణం pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 సంకేతం_సూచిక;
    UINT64 gp;
  };

  నిర్మాణం ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 సంకేతం_సూచిక;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  నిర్మాణం ffi_pa64_trampoline_struct *tramp;
  నిర్మాణం pa64_fd *fd;

  ఒకవేళ (cif->abi != FFI_PA64)
    తిరిగి FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (నిర్మాణం pa64_fd *)((UINT64)ffi_సంవృతం_pa64);

  /* Setup trampoline.  */
  tramp = (నిర్మాణం ffi_pa64_trampoline_struct *)సంవృతం->tramp;
  tramp->సంకేతం_సూచిక = fd->సంకేతం_సూచిక;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  సంవృతం->cif  = cif;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  సంవృతం->fun  = fun;

  తిరిగి FFI_OK;
}
#అయితే_ముగింపు
