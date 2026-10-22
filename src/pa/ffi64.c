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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>

#define ROUND_UP(v, a)  (((माप_प्रकार)(v) + (a) - 1) & ~((a) - 1))

#define FIRST_ARG_SLOT  0
#define DEBUG_LEVEL   0

#define fldw(addr, fpreg) \
  __asm__ चञ्चल ("fldw 4(%0), %%" #fpreg "R" : : "r"(addr) : #fpreg)
#define fstw(fpreg, addr) \
  __asm__ चञ्चल ("fstw %%" #fpreg "R, 4(%0)" : : "r"(addr))
#define fldd(addr, fpreg) \
  __asm__ चञ्चल ("fldd 0(%0), %%" #fpreg "L" : : "r"(addr) : #fpreg)
#define fstd(fpreg, addr) \
  __asm__ चञ्चल ("fstd %%" #fpreg "L, 0(%0)" : : "r"(addr))

#define debug(lvl, x...) कुरु { यदि (lvl <= DEBUG_LEVEL) { म_लिखो(x); } } यावत् (0)

स्थिर अन्तःस्थ पूर्णाङ्क ffi_struct_type(ffi_type *t)
{
  पूर्णाङ्क sz = t->size;

  /* Small structure results are returned in registers 28 and 29,
     larger ones are in a buffer allocated by the callee.  The
     address of the buffer is passed in r28.  The buffer is supposed
     to be aligned on a 16-byte boundary.   Register return values are
     padded on the right.  The pad bits on the right are undefined.  */

  यदि (sz <= 16)
    प्रत्यागम -sz;
  अन्यथा
    प्रत्यागम FFI_TYPE_STRUCT;
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

शून्य ffi_prep_args_pa64(UINT64 *ढेर, extended_cif *ecif, अचिन्हित bytes)
{
  पञ्जिका अचिन्हित पूर्णाङ्क i;
  पञ्जिका ffi_type **p_arg;
  पञ्जिका शून्य **p_argv;
  अचिन्हित पूर्णाङ्क slot = FIRST_ARG_SLOT;
  माप_प्रकार len;

  debug(1, "%s: stack = %p, ecif = %p, bytes = %u\n", __FUNCTION__, ढेर,
	ecif, bytes);

  p_arg = ecif->cif->arg_types;
  p_argv = ecif->avalue;

  प्रति (i = 0; i < ecif->cif->nargs; i++)
    {
      पूर्णाङ्क type = (*p_arg)->type;

      len = (*p_arg)->size;

      विकल्प (type)
	{
	पक्ष FFI_TYPE_SINT8:
	  *(SINT64 *)(ढेर + slot) = *(SINT8 *)(*p_argv);
	  भङ्ग;

	पक्ष FFI_TYPE_UINT8:
	  *(UINT64 *)(ढेर + slot) = *(UINT8 *)(*p_argv);
	  भङ्ग;

	पक्ष FFI_TYPE_SINT16:
	  *(SINT64 *)(ढेर + slot) = *(SINT16 *)(*p_argv);
	  भङ्ग;

	पक्ष FFI_TYPE_UINT16:
	  *(UINT64 *)(ढेर + slot) = *(UINT16 *)(*p_argv);
	  भङ्ग;

	पक्ष FFI_TYPE_SINT32:
	  *(SINT64 *)(ढेर + slot) = *(SINT32 *)(*p_argv);
	  भङ्ग;

	पक्ष FFI_TYPE_UINT32:
	  *(UINT64 *)(ढेर + slot) = *(UINT32 *)(*p_argv);
	  भङ्ग;

	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_POINTER:
	  debug(3, "Storing UINT64 %lu in slot %u\n", *(UINT64 *)(*p_argv),
		slot);
	  *(UINT64 *)(ढेर + slot) = *(UINT64 *)(*p_argv);
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  /* First 8 args go in fr4L - fr11L.  */
	  debug(3, "Storing UINT32(float) in slot %u\n", slot);
	  *(UINT64 *)(ढेर + slot) = *(UINT32 *)(*p_argv);
	  विकल्प (slot - FIRST_ARG_SLOT)
	    {
	    /* First 4 args go in fr4L - fr7L.  */
	    पक्ष 0: fldw(ढेर + slot, fr4); भङ्ग;
	    पक्ष 1: fldw(ढेर + slot, fr5); भङ्ग;
	    पक्ष 2: fldw(ढेर + slot, fr6); भङ्ग;
	    पक्ष 3: fldw(ढेर + slot, fr7); भङ्ग;
	    पक्ष 4: fldw(ढेर + slot, fr8); भङ्ग;
	    पक्ष 5: fldw(ढेर + slot, fr9); भङ्ग;
	    पक्ष 6: fldw(ढेर + slot, fr10); भङ्ग;
	    पक्ष 7: fldw(ढेर + slot, fr11); भङ्ग;
	    }
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  debug(3, "Storing UINT64(double) at slot %u\n", slot);
	  *(UINT64 *)(ढेर + slot) = *(UINT64 *)(*p_argv);
	  विकल्प (slot - FIRST_ARG_SLOT)
	    {
	    /* First 8 args go in fr4 to fr11.  */
	    पक्ष 0: fldd(ढेर + slot, fr4); भङ्ग;
	    पक्ष 1: fldd(ढेर + slot, fr5); भङ्ग;
	    पक्ष 2: fldd(ढेर + slot, fr6); भङ्ग;
	    पक्ष 3: fldd(ढेर + slot, fr7); भङ्ग;
	    पक्ष 4: fldd(ढेर + slot, fr8); भङ्ग;
	    पक्ष 5: fldd(ढेर + slot, fr9); भङ्ग;
	    पक्ष 6: fldd(ढेर + slot, fr10); भङ्ग;
	    पक्ष 7: fldd(ढेर + slot, fr11); भङ्ग;
	    }
	  भङ्ग;

#ifdef PA64_HPUX
	पक्ष FFI_TYPE_LONGDOUBLE:
	  /* Align slot to a 16-byte boundary.  */
	  slot += (slot & 1);
	  *(UINT64 *)(ढेर + slot) = *(UINT64 *)(*p_argv);
	  *(UINT64 *)(ढेर + slot + 1) = *(UINT64 *)(*p_argv + 8);
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_STRUCT:
	  /* Structs larger than 8 bytes are aligned on a 16-byte boundary. */
	  यदि (len > 8)
	    slot += (slot & 1);
	  स_नकल((वर्ण *)(ढेर + slot), (वर्ण *)*p_argv, len);
	  भङ्ग;

	स्वभाव:
	  FFI_ASSERT(0);
	}

      slot += ROUND_UP (len, 8) >> 3;
      p_arg++;
      p_argv++;
    }

  FFI_ASSERT(slot * 8 <= bytes);

  प्रत्यागम;
}

स्थिर शून्य ffi_size_ढेर_pa64(ffi_cif *cif)
{
  ffi_type **ptr;
  पूर्णाङ्क i;
  पूर्णाङ्क z = 0; /* # stack slots */

  प्रति (ptr = cif->arg_types, i = 0; i < cif->nargs; ptr++, i++)
    {
      पूर्णाङ्क type = (*ptr)->type;
      पूर्णाङ्क size = (*ptr)->size;

      विकल्प (type)
	{
#ifdef PA64_HPUX
	पक्ष FFI_TYPE_LONGDOUBLE:
	  z += 2 + (z & 1);
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_STRUCT:
	  यदि (size > 8)
	    z += (z & 1);
	  z += ROUND_UP (size, 8) >> 3;
	  भङ्ग;

	स्वभाव: /* 64-bit values */
	  z++;
	}
    }

  /* We need a minimum of 8 argument slots.  Stack must be 16-byte
     aligned.  */
  यदि (z <= 8)
    z = 8;
  अन्यथा
    z += (z & 1);

  /* Add 16 bytes for frame marker.  */
  cif->bytes = z * 8 + 64;
  debug(3, "Calculated stack size is %u bytes\n", cif->bytes);
}

/* Perform machine dependent cif processing.  */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag for jump table.  */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_COMPLEX:
    पक्ष FFI_TYPE_STRUCT:
      /* For the return type we have to check the size of the structures.
	 If the size is smaller or equal 8 bytes, the result is given back
	 in one register. If the size is smaller or equal 16 bytes than we
	 return the result in two registers. If the size is bigger than
	 16 bytes, the return is in a buffer allocated by the caller.  */
      cif->flags = ffi_struct_type(cif->rtype);
      भङ्ग;

    स्वभाव:
      cif->flags = (अचिन्हित) cif->rtype->type;
      भङ्ग;
    }

  /* Lucky us, because of the unique PA ABI we get to do our
     own stack sizing.  */
  विकल्प (cif->abi)
    {
    पक्ष FFI_PA64:
      ffi_size_ढेर_pa64(cif);
      भङ्ग;

    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }

  प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_call_pa64(शून्य (*)(UINT64 *, extended_cif *, अचिन्हित),
			  extended_cif *, अचिन्हित, अचिन्हित, अचिन्हित *,
			  शून्य (*fn)(शून्य));

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  यदि (rvalue == रिक्त
      && (cif->rtype->type == FFI_TYPE_STRUCT
	  || cif->rtype->type == FFI_TYPE_COMPLEX)
      && cif->rtype->size > 16)
    ecif.rvalue = alloca(ROUND_UP (cif->rtype->size, 16));
  अन्यथा
    ecif.rvalue = rvalue;


  विकल्प (cif->abi)
    {
    पक्ष FFI_PA64:
      debug(3, "Calling ffi_call_pa64: ecif=%p, bytes=%u, flags=%u, rvalue=%p, fn=%p\n", &ecif, cif->bytes, cif->flags, ecif.rvalue, (शून्य *)fn);
      ffi_call_pa64(ffi_prep_args_pa64, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      भङ्ग;

    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }
}

#यदि FFI_CLOSURES
/* This is more-or-less an inverse of ffi_call -- we have arguments on
   the stack, and we need to fill them into a cif structure and invoke
   the user function. This really ought to be in asm to make sure
   the compiler doesn't do things we don't expect.  */
ffi_status ffi_closure_inner_pa64(ffi_closure *closure, UINT64 *ढेर)
{
  ffi_cif *cif;
  शून्य **avalue;
  शून्य *rvalue;
  /* Functions can return up to 128-bits in registers.  Return address
     must be double word aligned.  */
  सङ्घ { दीर्घ द्विगुण rld; UINT64 ret[2]; } u;
  ffi_type **p_arg;
  वर्ण *tmp;
  पूर्णाङ्क i, avn;
  अचिन्हित पूर्णाङ्क slot = FIRST_ARG_SLOT;
  पञ्जिका UINT64 r28 __asm__("r28");

  cif = closure->cif;

  /* If returning via structure, callee will write to our pointer.  */
  यदि (cif->flags == FFI_TYPE_STRUCT)
    rvalue = (शून्य *)r28;
  अन्यथा
    rvalue = &u;

  avalue = (शून्य **)alloca(cif->nargs * FFI_SIZEOF_ARG);
  avn = cif->nargs;
  p_arg = cif->arg_types;

  प्रति (i = 0; i < avn; i++)
    {
      पूर्णाङ्क type = (*p_arg)->type;

      विकल्प (type)
	{
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
	  avalue[i] = (शून्य *)(ढेर + slot) + 7;
	  भङ्ग;

	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
	  avalue[i] = (शून्य *)(ढेर + slot) + 6;
	  भङ्ग;

	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
	  avalue[i] = (शून्य *)(ढेर + slot) + 4;
	  भङ्ग;

	पक्ष FFI_TYPE_POINTER:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
	  avalue[i] = (शून्य *)(ढेर + slot);
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  /* The closure call is indirect.  In Linux, floating point
	     arguments in indirect calls with a prototype are passed
	     in the floating point registers instead of the general
	     registers.  So, we need to replace what was previously
	     stored in the current slot with the value in the
	     corresponding floating point register.  */
	  विकल्प (slot + FIRST_ARG_SLOT)
	    {
	    पक्ष 0: fstw(fr4, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 1: fstw(fr5, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 2: fstw(fr6, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 3: fstw(fr7, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 4: fstw(fr8, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 5: fstw(fr9, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 6: fstw(fr10, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 7: fstw(fr11, (शून्य *)(ढेर + slot)); भङ्ग;
	    }
	  avalue[i] = (शून्य *)(ढेर + slot) + 4;
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  /* See previous comment for FFI_TYPE_FLOAT.  */
	  विकल्प (slot + FIRST_ARG_SLOT)
	    {
	    पक्ष 0: fstd(fr4, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 1: fstd(fr5, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 2: fstd(fr6, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 3: fstd(fr7, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 4: fstd(fr8, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 5: fstd(fr9, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 6: fstd(fr10, (शून्य *)(ढेर + slot)); भङ्ग;
	    पक्ष 7: fstd(fr11, (शून्य *)(ढेर + slot)); भङ्ग;
	    }
	  avalue[i] = (शून्य *)(ढेर + slot);
	  भङ्ग;

#ifdef PA64_HPUX
	पक्ष FFI_TYPE_LONGDOUBLE:
	  /* Long doubles are treated like a big structure.  */
	  slot += (slot & 1);
	  avalue[i] = (शून्य *)(ढेर + slot);
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_STRUCT:
	  /* All structs are passed in registers.  Structs larger
	     than 8 bytes are aligned on a 16-byte boundary.  */
	  यदि((*p_arg)->size > 8)
	    slot += (slot & 1);
	  avalue[i] = (शून्य *) (ढेर + slot);
	  भङ्ग;

	स्वभाव:
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
  विकल्प (cif->flags)
    {
    पक्ष FFI_TYPE_UINT8:
      *(ढेर + FIRST_ARG_SLOT) = (UINT8)u.ret[0];
      भङ्ग;
    पक्ष FFI_TYPE_SINT8:
      *(ढेर + FIRST_ARG_SLOT) = (SINT8)u.ret[0];
      भङ्ग;
    पक्ष FFI_TYPE_UINT16:
      *(ढेर + FIRST_ARG_SLOT) = (UINT16)u.ret[0];
      भङ्ग;
    पक्ष FFI_TYPE_SINT16:
      *(ढेर + FIRST_ARG_SLOT) = (SINT16)u.ret[0];
      भङ्ग;
    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
      *(ढेर + FIRST_ARG_SLOT) = (SINT32)u.ret[0];
      भङ्ग;
    पक्ष FFI_TYPE_UINT32:
      *(ढेर - FIRST_ARG_SLOT) = (UINT32)u.ret[0];
      भङ्ग;
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_POINTER:
      *(ढेर - FIRST_ARG_SLOT) = u.ret[0];
      भङ्ग;

    पक्ष FFI_TYPE_LONGDOUBLE:
      *(ढेर + FIRST_ARG_SLOT) = u.ret[0];
      *(ढेर + FIRST_ARG_SLOT + 1) = u.ret[1];
      भङ्ग;

    पक्ष FFI_TYPE_DOUBLE:
      fldd(rvalue, fr4);
      भङ्ग;

    पक्ष FFI_TYPE_FLOAT:
      /* Adjust for address adjustment in fldw macro.  */
      fldw(rvalue - 4, fr4);
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      /* Don't need a return value, done by caller.  */
      भङ्ग;

    पक्ष -1:
    पक्ष -2:
    पक्ष -3:
    पक्ष -4:
    पक्ष -5:
    पक्ष -6:
    पक्ष -7:
    पक्ष -8:
    पक्ष -9:
    पक्ष -10:
    पक्ष -11:
    पक्ष -12:
    पक्ष -13:
    पक्ष -14:
    पक्ष -15:
    पक्ष -16:
      tmp = (शून्य*)(ढेर + FIRST_ARG_SLOT);
      स_नकल((शून्य*)tmp, &u, cif->rtype->size);
      भङ्ग;

    पक्ष FFI_TYPE_VOID:
      भङ्ग;

    स्वभाव:
      debug(0, "assert with cif->flags: %d\n",cif->flags);
      FFI_ASSERT(0);
      भङ्ग;
    }
  प्रत्यागम FFI_OK;
}

/* Fill in a closure to refer to the specified fun and user_data.
   cif specifies the argument and result types for fun.
   The cif must already be prep'ed.  */

बाह्य शून्य ffi_closure_pa64(शून्य);

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  /* The layout of a function descriptor.  */
  संरचना pa64_fd
  {
    UINT64 tmp1;
    UINT64 tmp2;
    UINT64 code_pointer;
    UINT64 gp;
  };

  संरचना ffi_pa64_trampoline_struct
  {
    UINT64 real_gp;             /* Real gp value.  */
    UINT64 tmp2;
    UINT64 code_pointer;        /* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;             /* Pointer to closure, installed as gp.  */
  };

  संरचना ffi_pa64_trampoline_struct *tramp;
  संरचना pa64_fd *fd;

  यदि (cif->abi != FFI_PA64)
    प्रत्यागम FFI_BAD_ABI;

  /* Get function descriptor address for ffi_closure_pa64.  */
  fd = (संरचना pa64_fd *)((UINT64)ffi_closure_pa64);

  /* Setup trampoline.  */
  tramp = (संरचना ffi_pa64_trampoline_struct *)closure->tramp;
  tramp->code_pointer = fd->code_pointer;
  tramp->fake_gp = (UINT64)codeloc;
  tramp->real_gp = fd->gp;

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  प्रत्यागम FFI_OK;
}
#endif
