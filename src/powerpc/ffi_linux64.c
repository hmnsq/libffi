/* -----------------------------------------------------------------------
   ffi_linux64.c - Copyright (C) 2013 IBM
                   Copyright (C) 2011, 2026 Anthony Green
                   Copyright (C) 2011 Kyle Moffett
                   Copyright (C) 2008 Red Hat, Inc
                   Copyright (C) 2007, 2008 Free Software Foundation, Inc
                   Copyright (c) 1998 Geoffrey Keating

   PowerPC Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#شامل "ffi.h"
#شامل <tramp.h>
#شامل <stdlib.h>

#اگر_تعریف_شدہ POWERPC64
#شامل "ffi_common.h"
#شامل "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
شمار {
  عدد_GPR_ARG_REGISTERS64 = 8,
  عدد_FPR_ARG_REGISTERS64 = 13,
  عدد_VEC_ARG_REGISTERS64 = 12,
};
شمار { ASM_NEEDS_REGISTERS64 = 4 };


#اگر HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
خالی FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  اگر ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.سائز = 8;
      ffi_type_longdouble.alignment = 8;
    }
  ورنہ
    {
      ffi_type_longdouble.سائز = 16;
      ffi_type_longdouble.alignment = 16;
    }
#اگر_تعریف_شدہ FFI_ہدف_HAS_COMPLEX_TYPE
  /* _Complex long double is two halves of whatever long double just became.
     Its size comes from sizeof(_Complex long double) at build time, which is
     only right for the ABI libffi itself was built for, so keep it in step.
     Leaving it behind makes the type disagree with its own element: a caller
     that asks for the other long double width would get a return value copied
     at cif->rtype->size (wrong by 16 bytes either way) and structs containing
     the type laid out to the build-time size.  */
  ffi_type_complex_longdouble.سائز = 2 * ffi_type_longdouble.سائز;
  ffi_type_complex_longdouble.alignment = ffi_type_longdouble.alignment;
#اگر_ختم
}
#اگر_ختم


ساکن غیرعلامتی صحیح
discover_homogeneous_aggregate (ffi_abi abi,
                                ثابت ffi_type *t,
                                غیرعلامتی صحیح *elnum)
{
  انتخاب (t->type)
    {
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      اگر ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          واپس FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      ورنہ اگر ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          واپس FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#اگر_ختم
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
      *elnum = 1;
      واپس (صحیح) t->type;

#اگر_تعریف_شدہ FFI_ہدف_HAS_COMPLEX_TYPE
    حالت FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	غیرعلامتی صحیح inner_elnum = 0;
	غیرعلامتی صحیح inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	اگر (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    واپس inner;
	  }
	واپس 0;
      }
#اگر_ختم

    حالت FFI_TYPE_STRUCT:;
      {
	غیرعلامتی صحیح بنیاد_elt = 0, کل_elnum = 0;
	ffi_type **el = t->elements;
	جب (*el)
	  {
	    غیرعلامتی صحیح el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    اگر (el_elt == 0
		|| (بنیاد_elt && بنیاد_elt != el_elt))
	      واپس 0;
	    بنیاد_elt = el_elt;
	    کل_elnum += el_elnum;
#اگر _CALL_ELF == 2
	    اگر (کل_elnum > 8)
	      واپس 0;
#ورنہ
	    اگر (کل_elnum > 1)
	      واپس 0;
#اگر_ختم
	    el++;
	  }
	*elnum = کل_elnum;
	واپس بنیاد_elt;
      }

    طےشدہ:
      واپس 0;
    }
}


/* Perform machine dependent cif processing */
ساکن ffi_حالت
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  غیرعلامتی بائٹیں;
  غیرعلامتی i, fparg_گنتی = 0, intarg_گنتی = 0, vecarg_گنتی = 0;
  غیرعلامتی flags = cif->flags;
  غیرعلامتی elt, elnum, rtype;

#اگر FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    واپس FFI_BAD_ABI;
#وگرنہ_اگر !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    واپس FFI_BAD_ABI;
#ورنہ
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    واپس FFI_BAD_ABI;
#اگر_ختم

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#اگر _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  بائٹیں = (4 + ASM_NEEDS_REGISTERS64) * حجم (طویل);

  /* Space for the general registers.  */
  بائٹیں += عدد_GPR_ARG_REGISTERS64 * حجم (طویل);
#ورنہ
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  بائٹیں = (6 + ASM_NEEDS_REGISTERS64) * حجم (طویل);

  /* Space for the mandatory parm save area and general registers.  */
  بائٹیں += 2 * عدد_GPR_ARG_REGISTERS64 * حجم (طویل);
#اگر_ختم

  /* Return value handling.  */
  rtype = cif->rtype->type;
#اگر _CALL_ELF == 2
homogeneous:
#اگر_ختم
  انتخاب (rtype)
    {
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          توڑو;
        }
      اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128_بٹ;
      /* Fall through.  */
#اگر_ختم
    حالت FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_بٹ;
      /* Fall through.  */
    حالت FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      توڑو;

    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_اشارہ_گر:
      flags |= FLAG_RETURNS_64_بٹ;
      توڑو;

    حالت FFI_TYPE_STRUCT:
#اگر _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      اگر (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          جاؤ homogeneous;
        }
      اگر (cif->rtype->سائز <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          توڑو;
        }
#اگر_ختم
      intarg_گنتی++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    حالت FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      توڑو;

#اگر_تعریف_شدہ FFI_ہدف_HAS_COMPLEX_TYPE
    حالت FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      انتخاب (rtype)
        {
        حالت FFI_TYPE_FLOAT:
        حالت FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          جاؤ homogeneous;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        حالت FFI_TYPE_LONGDOUBLE:
          اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              توڑو;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          جاؤ homogeneous;
#اگر_ختم
        حالت FFI_TYPE_INT:
        حالت FFI_TYPE_SINT8:  حالت FFI_TYPE_UINT8:
        حالت FFI_TYPE_SINT16: حالت FFI_TYPE_UINT16:
        حالت FFI_TYPE_SINT32: حالت FFI_TYPE_UINT32:
        حالت FFI_TYPE_SINT64: حالت FFI_TYPE_UINT64:
        حالت FFI_TYPE_اشارہ_گر:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          توڑو;
        طےشدہ:
          واپس FFI_BAD_TYPEDEF;
        }
      توڑو;
#اگر_ختم

    طےشدہ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      توڑو;
    }

  برائے (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      غیرعلامتی صحیح align;

      انتخاب ((*ptr)->type)
	{
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
          اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_گنتی++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_گنتی = (intarg_گنتی + 3) & ~0x1;
              اگر (vecarg_گنتی > عدد_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              توڑو;
            }
	  اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_گنتی++;
	      intarg_گنتی++;
	    }
	  /* Fall through.  */
#اگر_ختم
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_FLOAT:
	  fparg_گنتی++;
	  intarg_گنتی++;
	  اگر (fparg_گنتی > عدد_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  توڑو;

	حالت FFI_TYPE_STRUCT:
	  اگر ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      اگر (align > 16)
		align = 16;
	      align = align / 8;
	      اگر (align > 1)
		intarg_گنتی = FFI_ALIGN (intarg_گنتی, align);
	    }
	  intarg_گنتی += ((*ptr)->سائز + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          اگر (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_گنتی += elnum;
              اگر (vecarg_گنتی > عدد_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              توڑو;
            }
	  ورنہ
#اگر_ختم
	  اگر (elt)
	    {
	      fparg_گنتی += elnum;
	      اگر (fparg_گنتی > عدد_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ورنہ
	    {
	      اگر (intarg_گنتی > عدد_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  توڑو;

	حالت FFI_TYPE_اشارہ_گر:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_گنتی++;
	  اگر (intarg_گنتی > عدد_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  توڑو;

#اگر_تعریف_شدہ FFI_ہدف_HAS_COMPLEX_TYPE
	حالت FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  اگر (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#اگر_ختم
	  انتخاب (elt)
	    {
	    حالت FFI_TYPE_FLOAT:
	    حالت FFI_TYPE_DOUBLE:
	      fparg_گنتی += 2;
	      intarg_گنتی += 2;
	      اگر (fparg_گنتی > عدد_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      توڑو;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    حالت FFI_TYPE_LONGDOUBLE:
	      اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_گنتی += 2;
		  intarg_گنتی = (intarg_گنتی + 1) & ~0x1;
		  intarg_گنتی += 4;
		  اگر (vecarg_گنتی > عدد_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  توڑو;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_گنتی += 4;
	      intarg_گنتی += 4;
	      اگر (fparg_گنتی > عدد_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      توڑو;
#اگر_ختم
	    حالت FFI_TYPE_INT:
	    حالت FFI_TYPE_SINT8:  حالت FFI_TYPE_UINT8:
	    حالت FFI_TYPE_SINT16: حالت FFI_TYPE_UINT16:
	    حالت FFI_TYPE_SINT32: حالت FFI_TYPE_UINT32:
	    حالت FFI_TYPE_SINT64: حالت FFI_TYPE_UINT64:
	    حالت FFI_TYPE_اشارہ_گر:
	      intarg_گنتی += 2;
	      توڑو;
	    طےشدہ:
	      واپس FFI_BAD_TYPEDEF;
	    }
	  اگر (intarg_گنتی > عدد_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  توڑو;
#اگر_ختم

	طےشدہ:
	  FFI_ASSERT (0);
	}
    }

  اگر (fparg_گنتی != 0)
    flags |= FLAG_FP_ARGUMENTS;
  اگر (intarg_گنتی > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  اگر (vecarg_گنتی != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  اگر (fparg_گنتی != 0)
    بائٹیں += عدد_FPR_ARG_REGISTERS64 * حجم (دوہرا);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  اگر (vecarg_گنتی != 0) {
    بائٹیں = (بائٹیں + 15) & ~0xF;
    بائٹیں += عدد_VEC_ARG_REGISTERS64 * حجم (float128);
  }

  /* Stack space.  */
#اگر _CALL_ELF == 2
  اگر ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    بائٹیں += intarg_گنتی * حجم (طویل);
#ورنہ
  اگر (intarg_گنتی > عدد_GPR_ARG_REGISTERS64)
    بائٹیں += (intarg_گنتی - عدد_GPR_ARG_REGISTERS64) * حجم (طویل);
#اگر_ختم

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  بائٹیں = (بائٹیں + 15) & ~0xF;

  cif->flags = flags;
  cif->بائٹیں = بائٹیں;

  واپس FFI_OK;
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  اگر ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#اگر _CALL_ELF != 2
  ورنہ اگر (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#اگر_ختم
  ورنہ
    واپس FFI_BAD_ABI;
  واپس ffi_prep_cif_linux64_core (cif);
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  غیرعلامتی صحیح nfixedargs,
			  غیرعلامتی صحیح ntotalargs MAYBE_غیر_مستعمل)
{
  اگر ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#اگر _CALL_ELF != 2
  ورنہ اگر (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#اگر_ختم
  ورنہ
    واپس FFI_BAD_ABI;
#اگر _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#اگر_ختم
  واپس ffi_prep_cif_linux64_core (cif);
}


/* ffi_prep_args64 is called by the assembly routine once stack space
   has been allocated for the function's arguments.

   The stack layout we want looks like this:

   |   Ret addr from ffi_call_LINUX64	8bytes	|	higher addresses
   |--------------------------------------------|
   |   CR save area			8bytes	|
   |--------------------------------------------|
   |   Previous backchain pointer	8	|	stack pointer here
   |--------------------------------------------|<+ <<<	on entry to
   |   Saved r28-r31			4*8	| |	ffi_call_LINUX64
   |--------------------------------------------| |
   |   GPR registers r3-r10		8*8	| |
   |--------------------------------------------| |
   |   FPR registers f1-f13 (optional)	13*8	| |
   |--------------------------------------------| |
   |   VEC registers v2-v13 (optional)  12*16   | |
   |--------------------------------------------| |
   |   Parameter save area		        | |
   |--------------------------------------------| |
   |   TOC save area			8	| |
   |--------------------------------------------| |	stack	|
   |   Linker doubleword		8	| |	grows	|
   |--------------------------------------------| |	down	V
   |   Compiler doubleword		8	| |
   |--------------------------------------------| |	lower addresses
   |   Space for callee's LR		8	| |
   |--------------------------------------------| |
   |   CR save area			8	| |
   |--------------------------------------------| |	stack pointer here
   |   Current backchain pointer	8	|-/	during
   |--------------------------------------------|   <<<	ffi_call_LINUX64

*/

خالی FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, غیرعلامتی طویل *ثابت ڈھیر)
{
  ثابت غیرعلامتی طویل بائٹیں = ecif->cif->بائٹیں;
  ثابت غیرعلامتی طویل flags = ecif->cif->flags;

  قسمتعریف اتحاد
  {
    حرف *c;
    غیرعلامتی طویل *ul;
    اعشاری *f;
    دوہرا *d;
    float128 *f128;
    size_t p;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'next_arg' points at the space for gpr3, and grows upwards as
     we use GPR registers, then continues at rest.  */
  valp gpr_بنیاد;
  valp gpr_انتہا;
  valp rest;
  valp اگلا_arg;

  /* 'fpr_base' points at the space for f1, and grows upwards as
     we use FPR registers.  */
  valp fpr_بنیاد;
  غیرعلامتی صحیح fparg_گنتی;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_بنیاد;
  غیرعلامتی صحیح vecarg_گنتی;

  غیرعلامتی صحیح i, الفاظ, nargs, nfixedargs;
  ffi_type **ptr;
  دوہرا double_tmp;
  اتحاد
  {
    خالی **v;
    حرف **c;
    علامتی حرف **sc;
    غیرعلامتی حرف **uc;
    علامتی مختصر **ss;
    غیرعلامتی مختصر **us;
    علامتی صحیح **si;
    غیرعلامتی صحیح **ui;
    غیرعلامتی طویل **ul;
    اعشاری **f;
    دوہرا **d;
    float128 **f128;
  } p_argv;
  غیرعلامتی طویل gprvalue;
  غیرعلامتی طویل align;

  stacktop.c = (حرف *) ڈھیر + بائٹیں;
  gpr_بنیاد.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - عدد_GPR_ARG_REGISTERS64;
  gpr_انتہا.ul = gpr_بنیاد.ul + عدد_GPR_ARG_REGISTERS64;
#اگر _CALL_ELF == 2
  rest.ul = ڈھیر + 4 + عدد_GPR_ARG_REGISTERS64;
#ورنہ
  rest.ul = ڈھیر + 6 + عدد_GPR_ARG_REGISTERS64;
#اگر_ختم
  fpr_بنیاد.d = gpr_بنیاد.d - عدد_FPR_ARG_REGISTERS64;
  fparg_گنتی = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  اگر (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_بنیاد.p = fpr_بنیاد.p & ~0xF;
  ورنہ
    vec_بنیاد.p = gpr_بنیاد.p;
  vec_بنیاد.f128 -= عدد_VEC_ARG_REGISTERS64;
  vecarg_گنتی = 0;
  اگلا_arg.ul = gpr_بنیاد.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((غیرعلامتی طویل) (حرف *) ڈھیر & 0xF) == 0);
  FFI_ASSERT (((غیرعلامتی طویل) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((غیرعلامتی طویل) gpr_بنیاد.c & 0xF) == 0);
  FFI_ASSERT (((غیرعلامتی طویل) gpr_انتہا.c  & 0xF) == 0);
  FFI_ASSERT (((غیرعلامتی طویل) vec_بنیاد.c & 0xF) == 0);
  FFI_ASSERT ((بائٹیں & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  اگر (flags & FLAG_RETVAL_REFERENCE)
    *اگلا_arg.ul++ = (غیرعلامتی طویل) (حرف *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#اگر _CALL_ELF != 2
  nfixedargs = (غیرعلامتی) -1;
  اگر ((flags & FLAG_COMPAT) == 0)
#اگر_ختم
    nfixedargs = ecif->cif->nfixedargs;
  برائے (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      غیرعلامتی صحیح elt, elnum;

      انتخاب ((*ptr)->type)
	{
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
          اگر ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              اگلا_arg.p = FFI_ALIGN (اگلا_arg.p, 16);
              اگر (اگلا_arg.ul == gpr_انتہا.ul)
                اگلا_arg.ul = rest.ul;
              اگر (vecarg_گنتی < عدد_VEC_ARG_REGISTERS64 && i < nfixedargs)
		حافظہ_نقل (vec_بنیاد.f128++, *p_argv.f128, حجم (float128));
              ورنہ
		حافظہ_نقل (اگلا_arg.f128, *p_argv.f128, حجم (float128));
              اگر (++اگلا_arg.f128 == gpr_انتہا.f128)
                اگلا_arg.f128 = rest.f128;
              vecarg_گنتی++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              توڑو;
            }
	  اگر ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_بنیاد.d++ = double_tmp;
# اگر _CALL_ELF != 2
		  اگر ((flags & FLAG_COMPAT) != 0)
		    *اگلا_arg.d = double_tmp;
# endif
		}
	      ورنہ
		*اگلا_arg.d = double_tmp;
	      اگر (++اگلا_arg.ul == gpr_انتہا.ul)
		اگلا_arg.ul = rest.ul;
	      fparg_گنتی++;
	      double_tmp = (*p_argv.d)[1];
	      اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_بنیاد.d++ = double_tmp;
# اگر _CALL_ELF != 2
		  اگر ((flags & FLAG_COMPAT) != 0)
		    *اگلا_arg.d = double_tmp;
# endif
		}
	      ورنہ
		*اگلا_arg.d = double_tmp;
	      اگر (++اگلا_arg.ul == gpr_انتہا.ul)
		اگلا_arg.ul = rest.ul;
	      fparg_گنتی++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      توڑو;
	    }
	  /* Fall through.  */
#اگر_ختم
	حالت FFI_TYPE_DOUBLE:
#اگر _CALL_ELF != 2
	do_double:
#اگر_ختم
	  double_tmp = **p_argv.d;
	  اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_بنیاد.d++ = double_tmp;
#اگر _CALL_ELF != 2
	      اگر ((flags & FLAG_COMPAT) != 0)
		*اگلا_arg.d = double_tmp;
#اگر_ختم
	    }
	  ورنہ
	    *اگلا_arg.d = double_tmp;
	  اگر (++اگلا_arg.ul == gpr_انتہا.ul)
	    اگلا_arg.ul = rest.ul;
	  fparg_گنتی++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  توڑو;

	حالت FFI_TYPE_FLOAT:
#اگر _CALL_ELF != 2
	do_float:
#اگر_ختم
	  double_tmp = **p_argv.f;
	  اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_بنیاد.d++ = double_tmp;
#اگر _CALL_ELF != 2
	      اگر ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  اگلا_arg.f[1] = (اعشاری) double_tmp;
# ورنہ
		  اگلا_arg.f[0] = (اعشاری) double_tmp;
# endif
		}
#اگر_ختم
	    }
	  ورنہ
	    {
# ifndef __LITTLE_ENDIAN__
	      اگلا_arg.f[1] = (اعشاری) double_tmp;
# ورنہ
	      اگلا_arg.f[0] = (اعشاری) double_tmp;
# endif
	    }
	  اگر (++اگلا_arg.ul == gpr_انتہا.ul)
	    اگلا_arg.ul = rest.ul;
	  fparg_گنتی++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  توڑو;

#اگر_تعریف_شدہ FFI_ہدف_HAS_COMPLEX_TYPE
	حالت FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  اگر (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  اگر (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      غیرعلامتی صحیح j;
	      برائے (j = 0; j < 2; j++)
		{
		  اگلا_arg.p = FFI_ALIGN (اگلا_arg.p, 16);
		  اگر (اگلا_arg.ul == gpr_انتہا.ul)
		    اگلا_arg.ul = rest.ul;
		  اگر (vecarg_گنتی < عدد_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    حافظہ_نقل (vec_بنیاد.f128++, cval + j, حجم (float128));
		  ورنہ
		    حافظہ_نقل (اگلا_arg.f128, cval + j, حجم (float128));
		  اگر (++اگلا_arg.f128 == gpr_انتہا.f128)
		    اگلا_arg.f128 = rest.f128;
		  vecarg_گنتی++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      توڑو;
	    }
	  اگر (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      دوہرا *cval = (دوہرا *) *p_argv.v;
	      غیرعلامتی صحیح j;
	      برائے (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_بنیاد.d++ = double_tmp;
		  ورنہ
		    *اگلا_arg.d = double_tmp;
		  اگر (++اگلا_arg.ul == gpr_انتہا.ul)
		    اگلا_arg.ul = rest.ul;
		  fparg_گنتی++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      توڑو;
	    }
#اگر_ختم
	  اگر (elt == FFI_TYPE_FLOAT)
	    {
	      اعشاری *cval = (اعشاری *) *p_argv.v;
	      غیرعلامتی صحیح j;
	      برائے (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_بنیاد.d++ = double_tmp;
		  ورنہ
		    {
# ifndef __LITTLE_ENDIAN__
		      اگلا_arg.f[1] = (اعشاری) double_tmp;
# ورنہ
		      اگلا_arg.f[0] = (اعشاری) double_tmp;
# endif
		    }
		  اگر (++اگلا_arg.ul == gpr_انتہا.ul)
		    اگلا_arg.ul = rest.ul;
		  fparg_گنتی++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  ورنہ اگر (elt == FFI_TYPE_DOUBLE)
	    {
	      دوہرا *cval = (دوہرا *) *p_argv.v;
	      غیرعلامتی صحیح j;
	      برائے (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_بنیاد.d++ = double_tmp;
		  ورنہ
		    *اگلا_arg.d = double_tmp;
		  اگر (++اگلا_arg.ul == gpr_انتہا.ul)
		    اگلا_arg.ul = rest.ul;
		  fparg_گنتی++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  ورنہ
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      حرف *cval = (حرف *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->سائز;
	      غیرعلامتی صحیح j;
	      برائے (j = 0; j < 2; j++)
		{
		  حرف *half = cval + j * hsize;
		  غیرعلامتی طویل gprvalue;
		  انتخاب (elt)
		    {
		    حالت FFI_TYPE_UINT8:
		      gprvalue = *(غیرعلامتی حرف *) half; توڑو;
		    حالت FFI_TYPE_SINT8:
		      gprvalue = (غیرعلامتی طویل) (طویل) *(علامتی حرف *) half;
		      توڑو;
		    حالت FFI_TYPE_UINT16:
		      gprvalue = *(غیرعلامتی مختصر *) half; توڑو;
		    حالت FFI_TYPE_SINT16:
		      gprvalue = (غیرعلامتی طویل) (طویل) *(علامتی مختصر *) half;
		      توڑو;
		    حالت FFI_TYPE_UINT32:
		      gprvalue = *(غیرعلامتی صحیح *) half; توڑو;
		    حالت FFI_TYPE_INT:
		    حالت FFI_TYPE_SINT32:
		      gprvalue = (غیرعلامتی طویل) (طویل) *(علامتی صحیح *) half;
		      توڑو;
		    حالت FFI_TYPE_SINT64:
		    حالت FFI_TYPE_UINT64:
		    حالت FFI_TYPE_اشارہ_گر:
		    طےشدہ:
		      gprvalue = *(غیرعلامتی طویل *) half; توڑو;
		    }
		  *اگلا_arg.ul++ = gprvalue;
		  اگر (اگلا_arg.ul == gpr_انتہا.ul)
		    اگلا_arg.ul = rest.ul;
		}
	    }
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_STRUCT:
	  اگر ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      اگر (align > 16)
		align = 16;
	      اگر (align > 1)
                {
                  اگلا_arg.p = FFI_ALIGN (اگلا_arg.p, align);
                  اگر (اگلا_arg.ul == gpr_انتہا.ul)
                    اگلا_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  اگر (elt)
	    {
#اگر _CALL_ELF == 2
	      اتحاد {
		خالی *v;
		اعشاری *f;
		دوہرا *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              اگر (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  کرو
                    {
                      اگر (vecarg_گنتی < عدد_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        حافظہ_نقل (vec_بنیاد.f128++, arg.f128++, حجم (float128));
                      ورنہ
		        حافظہ_نقل (اگلا_arg.f128, arg.f128++, حجم (float128));
                      اگر (++اگلا_arg.f128 == gpr_انتہا.f128)
                        اگلا_arg.f128 = rest.f128;
                      vecarg_گنتی++;
                    }
                  جب (--elnum != 0);
                }
              ورنہ
#اگر_ختم
	      اگر (elt == FFI_TYPE_FLOAT)
		{
		  کرو
		    {
		      double_tmp = *arg.f++;
		      اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_بنیاد.d++ = double_tmp;
		      ورنہ
			*اگلا_arg.f = (اعشاری) double_tmp;
		      اگر (++اگلا_arg.f == gpr_انتہا.f)
			اگلا_arg.f = rest.f;
		      fparg_گنتی++;
		    }
		  جب (--elnum != 0);
		  اگر ((اگلا_arg.p & 7) != 0)
                    اگر (++اگلا_arg.f == gpr_انتہا.f)
                      اگلا_arg.f = rest.f;
		}
	      ورنہ
		کرو
		  {
		    double_tmp = *arg.d++;
		    اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_بنیاد.d++ = double_tmp;
		    ورنہ
		      *اگلا_arg.d = double_tmp;
		    اگر (++اگلا_arg.d == gpr_انتہا.d)
		      اگلا_arg.d = rest.d;
		    fparg_گنتی++;
		  }
		جب (--elnum != 0);
#ورنہ
	      اگر (elt == FFI_TYPE_FLOAT)
		جاؤ do_float;
	      ورنہ
		جاؤ do_double;
#اگر_ختم
	    }
	  ورنہ
	    {
	      الفاظ = ((*ptr)->سائز + 7) / 8;
	      اگر (اگلا_arg.ul >= gpr_بنیاد.ul && اگلا_arg.ul + الفاظ > gpr_انتہا.ul)
		{
		  size_t پہلا = gpr_انتہا.c - اگلا_arg.c;
		  حافظہ_نقل (اگلا_arg.c, *p_argv.c, پہلا);
		  حافظہ_نقل (rest.c, *p_argv.c + پہلا, (*ptr)->سائز - پہلا);
		  اگلا_arg.c = rest.c + الفاظ * 8 - پہلا;
		}
	      ورنہ
		{
		  حرف *where = اگلا_arg.c;

#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  اگر ((*ptr)->سائز < 8)
		    where += 8 - (*ptr)->سائز;
#اگر_ختم
		  حافظہ_نقل (where, *p_argv.c, (*ptr)->سائز);
		  اگلا_arg.ul += الفاظ;
		  اگر (اگلا_arg.ul == gpr_انتہا.ul)
		    اگلا_arg.ul = rest.ul;
		}
	    }
	  توڑو;

	حالت FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  جاؤ putgpr;
	حالت FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  جاؤ putgpr;
	حالت FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  جاؤ putgpr;
	حالت FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  جاؤ putgpr;
	حالت FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  جاؤ putgpr;
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  جاؤ putgpr;

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_اشارہ_گر:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *اگلا_arg.ul++ = gprvalue;
	  اگر (اگلا_arg.ul == gpr_انتہا.ul)
	    اگلا_arg.ul = rest.ul;
	  توڑو;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (اگلا_arg.ul >= gpr_بنیاد.ul
		  && اگلا_arg.ul <= gpr_بنیاد.ul + 4));
}


#اگر _CALL_ELF == 2
#تعریف_کرو MIN_CACHE_سطر_سائز 8

ساکن خالی
flush_icache (حرف *wraddr, حرف *xaddr, صحیح سائز)
{
  صحیح i;
  برائے (i = 0; i < سائز; i += MIN_CACHE_سطر_سائز)
    __asm__ متلون ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ متلون ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + سائز - 1), "r"(wraddr + سائز - 1)
		    : "memory");
}
#اگر_ختم


ffi_حالت FFI_HIDDEN
ffi_prep_احاطہ_loc_linux64 (ffi_احاطہ *احاطہ,
			      ffi_cif *cif,
			      خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
			      خالی *user_ڈیٹا,
			      خالی *codeloc)
{
  اگر (cif->abi < FFI_LINUX || cif->abi >= FFI_آخری_ABI)
    واپس FFI_BAD_ABI;

#اگر_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
  اگر (ffi_tramp_is_present(احاطہ))
    {
      /* Initialize the static trampoline's parameters. */
      خالی (*dest)(خالی) = ffi_احاطہ_LINUX64;
      ffi_tramp_سیٹ_parms (احاطہ->ftramp, dest, احاطہ);
    }
  ورنہ
#اگر_ختم
    {
#اگر _CALL_ELF == 2
      غیرعلامتی صحیح *tramp = (غیرعلامتی صحیح *) &احاطہ->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(خالی **) &tramp[4] = (خالی *) ffi_احاطہ_LINUX64;
      *(خالی **) &tramp[6] = codeloc;
      flush_icache ((حرف *) tramp, (حرف *) codeloc, 4 * 4);
#ورنہ
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      خالی **tramp = (خالی **) &احاطہ->tramp[0];
      حافظہ_نقل (&tramp[0], (خالی **) ffi_احاطہ_LINUX64, حجم (خالی *));
      tramp[1] = codeloc;
      حافظہ_نقل (&tramp[2], (خالی **) ffi_احاطہ_LINUX64 + 1, حجم (خالی *));
#اگر_ختم
    }

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  واپس FFI_OK;
}


صحیح FFI_HIDDEN
ffi_احاطہ_helper_LINUX64 (ffi_cif *cif,
			    خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
			    خالی *user_ڈیٹا,
			    خالی *rvalue,
			    غیرعلامتی طویل *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  خالی **avalue;
  ffi_type **arg_types;
  غیرعلامتی طویل i, avn, nfixedargs;
  ffi_dblfl *انتہا_pfr = pfr + عدد_FPR_ARG_REGISTERS64;
  float128 *انتہا_pvec = pvec + عدد_VEC_ARG_REGISTERS64;
  غیرعلامتی طویل align;

  avalue = alloca (cif->nargs * حجم (خالی *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  اگر (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (خالی *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#اگر _CALL_ELF != 2
  nfixedargs = (غیرعلامتی) -1;
  اگر ((cif->flags & FLAG_COMPAT) == 0)
#اگر_ختم
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  جب (i < avn)
    {
      غیرعلامتی صحیح elt, elnum;

      انتخاب (arg_types[i]->type)
	{
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
	  avalue[i] = (حرف *) pst + 7;
	  pst++;
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
	  avalue[i] = (حرف *) pst + 6;
	  pst++;
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
	  avalue[i] = (حرف *) pst + 4;
	  pst++;
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_اشارہ_گر:
	  avalue[i] = pst;
	  pst++;
	  توڑو;

	حالت FFI_TYPE_STRUCT:
	  اگر ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      اگر (align > 16)
		align = 16;
	      اگر (align > 1)
		pst = (غیرعلامتی طویل *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  اگر (elt)
	    {
#اگر _CALL_ELF == 2
	      اتحاد {
		خالی *v;
		غیرعلامتی طویل *ul;
		اعشاری *f;
		دوہرا *d;
		float128 *f128;
		size_t p;
	      } مقصد, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              اگر (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  اگر (pvec + elnum <= انتہا_pvec)
                    مقصد.v = pvec;
                  ورنہ
                    مقصد.v = pst;
                }
              ورنہ
#اگر_ختم
	      اگر (pfr + elnum <= انتہا_pfr)
		مقصد.v = pfr;
	      ورنہ
		مقصد.v = pst;

	      avalue[i] = مقصد.v;
	      from.ul = pst;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              اگر (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  کرو
                    {
                      اگر (pvec < انتہا_pvec && i < nfixedargs)
		        حافظہ_نقل (مقصد.f128, pvec++, حجم (float128));
                      ورنہ
		        حافظہ_نقل (مقصد.f128, from.f128, حجم (float128));
                      مقصد.f128++;
                      from.f128++;
                    }
                  جب (--elnum != 0);
                }
              ورنہ
#اگر_ختم
	      اگر (elt == FFI_TYPE_FLOAT)
		{
		  کرو
		    {
		      اگر (pfr < انتہا_pfr && i < nfixedargs)
			{
			  *مقصد.f = (اعشاری) pfr->d;
			  pfr++;
			}
		      ورنہ
			*مقصد.f = *from.f;
		      مقصد.f++;
		      from.f++;
		    }
		  جب (--elnum != 0);
		}
	      ورنہ
		{
		  کرو
		    {
		      اگر (pfr < انتہا_pfr && i < nfixedargs)
			{
			  *مقصد.d = pfr->d;
			  pfr++;
			}
		      ورنہ
			*مقصد.d = *from.d;
		      مقصد.d++;
		      from.d++;
		    }
		  جب (--elnum != 0);
		}
#ورنہ
	      اگر (elt == FFI_TYPE_FLOAT)
		جاؤ do_float;
	      ورنہ
		جاؤ do_double;
#اگر_ختم
	    }
	  ورنہ
	    {
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      اگر (arg_types[i]->سائز < 8)
		avalue[i] = (حرف *) pst + 8 - arg_types[i]->سائز;
	      ورنہ
#اگر_ختم
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->سائز + 7) / 8;
	  توڑو;

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
          اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              اگر (((غیرعلامتی طویل) pst & 0xF) != 0)
                ++pst;
              اگر (pvec < انتہا_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              ورنہ
                avalue[i] = pst;
              pst += 2;
              توڑو;
            }
          ورنہ اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      اگر (pfr + 1 < انتہا_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      ورنہ
		{
		  اگر (pfr < انتہا_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(غیرعلامتی طویل *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      توڑو;
	    }
	  /* Fall through.  */
#اگر_ختم
	حالت FFI_TYPE_DOUBLE:
#اگر _CALL_ELF != 2
	do_double:
#اگر_ختم
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  اگر (pfr < انتہا_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ورنہ
	    avalue[i] = pst;
	  pst++;
	  توڑو;

	حالت FFI_TYPE_FLOAT:
#اگر _CALL_ELF != 2
	do_float:
#اگر_ختم
	  اگر (pfr < انتہا_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (اعشاری) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ورنہ
	    {
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
	      avalue[i] = (حرف *) pst + 4;
#ورنہ
	      avalue[i] = pst;
#اگر_ختم
	    }
	  pst++;
	  توڑو;

#اگر_تعریف_شدہ FFI_ہدف_HAS_COMPLEX_TYPE
	حالت FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    غیرعلامتی صحیح j;
	    elt = arg_types[i]->elements[0]->type;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    اگر (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    اگر (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * حجم (float128));
		اگر (((غیرعلامتی طویل) pst & 0xF) != 0)
		  ++pst;
		برائے (j = 0; j < 2; j++)
		  {
		    اگر (pvec < انتہا_pvec && i < nfixedargs)
		      حافظہ_نقل (&cval[j], pvec++, حجم (float128));
		    ورنہ
		      حافظہ_نقل (&cval[j], pst, حجم (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		توڑو;
	      }
	    اگر (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		دوہرا *cval = alloca (4 * حجم (دوہرا));
		برائے (j = 0; j < 4; j++)
		  {
		    اگر (pfr < انتہا_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    ورنہ
		      cval[j] = *(دوہرا *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		توڑو;
	      }
#اگر_ختم
	    اگر (elt == FFI_TYPE_FLOAT)
	      {
		اعشاری *cval = alloca (2 * حجم (اعشاری));
		برائے (j = 0; j < 2; j++)
		  {
		    اگر (pfr < انتہا_pfr && i < nfixedargs)
		      {
			cval[j] = (اعشاری) pfr->d;
			pfr++;
		      }
		    ورنہ
		      {
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
			cval[j] = ((اعشاری *) pst)[1];
#ورنہ
			cval[j] = ((اعشاری *) pst)[0];
#اگر_ختم
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    ورنہ اگر (elt == FFI_TYPE_DOUBLE)
	      {
		دوہرا *cval = alloca (2 * حجم (دوہرا));
		برائے (j = 0; j < 2; j++)
		  {
		    اگر (pfr < انتہا_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    ورنہ
		      cval[j] = *(دوہرا *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    ورنہ
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->سائز;
		حرف *cval = alloca (2 * hsize);
		برائے (j = 0; j < 2; j++)
		  {
		    حرف *src;
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
		    src = (حرف *) pst + (8 - hsize);
#ورنہ
		    src = (حرف *) pst;
#اگر_ختم
		    حافظہ_نقل (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  توڑو;
#اگر_ختم

	طےشدہ:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_ڈیٹا);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      واپس PPC_LD_NONE;
    حالت FFI_TYPE_FLOAT:
      واپس PPC_LD_F32;
    حالت FFI_TYPE_DOUBLE:
      واپس PPC_LD_F64;
#اگر FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      اگر ((cif->flags & FLAG_RETURNS_VEC) != 0)
	واپس PPC64_LD_سمتیہ;
      واپس PPC_LD_F128;
#اگر_ختم
    حالت FFI_TYPE_UINT8:
      واپس PPC_LD_U8;
    حالت FFI_TYPE_SINT8:
      واپس PPC_LD_S8;
    حالت FFI_TYPE_UINT16:
      واپس PPC_LD_U16;
    حالت FFI_TYPE_SINT16:
      واپس PPC_LD_S16;
    حالت FFI_TYPE_UINT32:
      واپس PPC_LD_U32;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      واپس PPC_LD_S32;
    حالت FFI_TYPE_اشارہ_گر:
      واپس PPC_LD_PTR;
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      واپس PPC_LD_I64;
#اگر_تعریف_شدہ FFI_ہدف_HAS_COMPLEX_TYPE
    حالت FFI_TYPE_COMPLEX:
      {
	صحیح inner = cif->rtype->elements[0]->type;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	اگر (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      واپس PPC64_LD_سمتیہ_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#اگر_ختم
	اگر (inner == FFI_TYPE_FLOAT)
	  واپس PPC64_LD_FLOAT_HOMOG;
	اگر (inner == FFI_TYPE_DOUBLE)
	  واپس PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  حرف *rv = rvalue;
	  غیرعلامتی طویل re, im;
	  انتخاب (inner)
	    {
	    حالت FFI_TYPE_UINT8:
	      re = ((غیرعلامتی حرف *) rv)[0];
	      im = ((غیرعلامتی حرف *) rv)[1];
	      توڑو;
	    حالت FFI_TYPE_SINT8:
	      re = (غیرعلامتی طویل) (طویل) ((علامتی حرف *) rv)[0];
	      im = (غیرعلامتی طویل) (طویل) ((علامتی حرف *) rv)[1];
	      توڑو;
	    حالت FFI_TYPE_UINT16:
	      re = ((غیرعلامتی مختصر *) rv)[0];
	      im = ((غیرعلامتی مختصر *) rv)[1];
	      توڑو;
	    حالت FFI_TYPE_SINT16:
	      re = (غیرعلامتی طویل) (طویل) ((علامتی مختصر *) rv)[0];
	      im = (غیرعلامتی طویل) (طویل) ((علامتی مختصر *) rv)[1];
	      توڑو;
	    حالت FFI_TYPE_UINT32:
	      re = ((غیرعلامتی صحیح *) rv)[0];
	      im = ((غیرعلامتی صحیح *) rv)[1];
	      توڑو;
	    حالت FFI_TYPE_INT:
	    حالت FFI_TYPE_SINT32:
	      re = (غیرعلامتی طویل) (طویل) ((علامتی صحیح *) rv)[0];
	      im = (غیرعلامتی طویل) (طویل) ((علامتی صحیح *) rv)[1];
	      توڑو;
	    حالت FFI_TYPE_SINT64:
	    حالت FFI_TYPE_UINT64:
	    حالت FFI_TYPE_اشارہ_گر:
	    طےشدہ:
	      re = ((غیرعلامتی طویل *) rv)[0];
	      im = ((غیرعلامتی طویل *) rv)[1];
	      توڑو;
	    }
	  ((غیرعلامتی طویل *) rv)[0] = re;
	  ((غیرعلامتی طویل *) rv)[1] = im;
	  واپس PPC_LD_R3R4;
	}
      }
#اگر_ختم
    حالت FFI_TYPE_STRUCT:
      اگر ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  اگر ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      انتخاب (cif->rtype->سائز)
		{
		حالت 0:
		  واپس PPC_LD_NONE;
		حالت 1:
		  واپس PPC_LD_U8;
		حالت 2:
		  واپس PPC_LD_U16;
		حالت 3:
		  واپس PPC64_LD_STRUCT_3;
		حالت 4:
		  واپس PPC_LD_U32;
		حالت 5:
		  واپس PPC64_LD_STRUCT_5;
		حالت 6:
		  واپس PPC64_LD_STRUCT_6;
		حالت 7:
		  واپس PPC64_LD_STRUCT_7;
		حالت 8 ... 16:
		  واپس PPC_LD_R3R4;
		}
	      توڑو;
	    }
	  اگر ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    واپس PPC64_LD_سمتیہ_HOMOG;
	  اگر ((cif->flags & FLAG_RETURNS_64_بٹ) != 0)
	    واپس PPC64_LD_DOUBLE_HOMOG;
	  واپس PPC64_LD_FLOAT_HOMOG;
        }
      واپس PPC_LD_NONE;
    }
  abort();
}
#اگر_ختم
