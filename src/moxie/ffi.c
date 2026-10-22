/* -----------------------------------------------------------------------
   ffi.c - Copyright (C) 2012, 2013, 2018, 2021, 2022, 2026  Anthony Green

   Moxie Foreign Function Interface

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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಶೂನ್ಯ *ffi_prep_ವಾದಗಳು(ಅಕ್ಷರ *ರಾಶಿ, extended_cif *ecif)
{
  ನೋಂದಣಿ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ನೋಂದಣಿ ಶೂನ್ಯ **p_argv;
  ನೋಂದಣಿ ಅಕ್ಷರ *argp;
  ನೋಂದಣಿ ffi_type **p_arg;
  ನೋಂದಣಿ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;

  p_argv = ecif->avalue;
  argp = ರಾಶಿ;

  ಆದರೆ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) argp = ecif->rvalue;
      argp += 4;
    }

  ಗಾಗಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ಗಾತ್ರ_2;

      ಆದರೆ ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = ಗಾತ್ರ(ಶೂನ್ಯ*);
	  *(ಶೂನ್ಯ **) argp = *p_argv;
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z < ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
	{
	  z = ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ);
	  ಆಯ್ಕೆ ((*p_arg)->type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	      *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT8 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT8 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	      *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT16 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT16 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      FFI_ASSERT(0);
	    }
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z == ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
	{
	  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT32 *)(* p_argv);
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಸ್ಮರಣೆ_ನಕಲು(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      ಎಣಿಕೆ_2 += z;
    }

  ಹಿಂತಿರುಗಿ (ರಾಶಿ + ((ಎಣಿಕೆ_2 > 24) ? 24 : FFI_ALIGN_ಕೆಳಕ್ಕೆ(ಎಣಿಕೆ_2, 8)));
}

/* Perform machine dependent cif processing */
ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep(ffi_cif *cif)
{
  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  ಇಲ್ಲದಿದ್ದರೆ
    cif->flags = cif->rtype->ಗಾತ್ರ_2;

  cif->ಬೈಟುಗಳು = FFI_ALIGN (cif->ಬೈಟುಗಳು, 8);

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_EABI(ಶೂನ್ಯ *(*)(ಅಕ್ಷರ *, extended_cif *),
			  extended_cif *,
			  ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ,
			  ಚಿಹ್ನೆರಹಿತ *,
			  ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));

ಶೂನ್ಯ ffi_call(ffi_cif *cif,
	      ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ),
	      ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ಖಾಲಿ;
  ಪೂರ್ಣಾಂಕ i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ಆದರೆ ((rvalue == ಖಾಲಿ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ಗಾತ್ರ_2);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ಗಾಗಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = at->ಗಾತ್ರ_2;
      ಆದರೆ (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          ಅಕ್ಷರ *argcopy = alloca (ಗಾತ್ರ_2);
          ಆದರೆ (avalue_copy == ಖಾಲಿ)
            {
              avalue_copy = alloca (nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
              ಸ್ಮರಣೆ_ನಕಲು (avalue_copy, avalue, nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
              avalue = avalue_copy;
            }
          ಸ್ಮರಣೆ_ನಕಲು (argcopy, avalue[i], ಗಾತ್ರ_2);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_EABI:
      ffi_call_EABI(ffi_prep_ವಾದಗಳು, &ecif, cif->ಬೈಟುಗಳು,
		    cif->flags, ecif.rvalue, fn);
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }
}

ಶೂನ್ಯ ffi_ಆವರಣ_eabi (ಚಿಹ್ನೆರಹಿತ arg1, ಚಿಹ್ನೆರಹಿತ arg2, ಚಿಹ್ನೆರಹಿತ arg3,
		       ಚಿಹ್ನೆರಹಿತ arg4, ಚಿಹ್ನೆರಹಿತ arg5, ಚಿಹ್ನೆರಹಿತ arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  ನೋಂದಣಿ ffi_ಆವರಣ *creg __asm__ ("$r12");
  ffi_ಆವರಣ *ಆವರಣ = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  ನೋಂದಣಿ ಅಕ್ಷರ *ಚೌಕಟ್ಟು_ಸೂಚಕ __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  ಶೂನ್ಯ *struct_rvalue = (ಶೂನ್ಯ *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  ಅಕ್ಷರ *ರಾಶಿ_ವಾದಗಳು = ಚೌಕಟ್ಟು_ಸೂಚಕ + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  ಚಿಹ್ನೆರಹಿತ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  ಅಕ್ಷರ *ರಿಜಿಸ್ಟರ್_ವಾದಗಳು_ptr = (ಅಕ್ಷರ *) ರಿಜಿಸ್ಟರ್_ವಾದಗಳು;

  ffi_cif *cif = ಆವರಣ->cif;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue = alloca (cif->nargs * ಗಾತ್ರ(ಶೂನ್ಯ *));
  ಅಕ್ಷರ *ptr = (ಅಕ್ಷರ *) ರಿಜಿಸ್ಟರ್_ವಾದಗಳು;
  ಪೂರ್ಣಾಂಕ i;

  /* preserve struct type return pointer passing */
  ಆದರೆ ((cif->rtype != ಖಾಲಿ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    ರಿಜಿಸ್ಟರ್_ವಾದಗಳು_ptr = (ಅಕ್ಷರ *)&ರಿಜಿಸ್ಟರ್_ವಾದಗಳು[1];
  }

  /* Find the address of each argument.  */
  ಗಾಗಿ (i = 0; i < cif->nargs; i++)
    {
      ಆಯ್ಕೆ (arg_types[i]->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  avalue[i] = ptr;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
          {
            ಆದರೆ (arg_types[i]->ಗಾತ್ರ_2 > 4)
              {
                ಶೂನ್ಯ *copy = alloca(arg_types[i]->ಗಾತ್ರ_2);
                ಸ್ಮರಣೆ_ನಕಲು(copy, *(ಶೂನ್ಯ**)ptr, arg_types[i]->ಗಾತ್ರ_2);
                avalue[i] = copy;
              }
            ಇಲ್ಲದಿದ್ದರೆ
              avalue[i] = *(ಶೂನ್ಯ**)ptr;
          }
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  /* This is an 8-byte value.  */
	  ಆದರೆ (ptr == (ಅಕ್ಷರ *) &ರಿಜಿಸ್ಟರ್_ವಾದಗಳು[5])
	    {
	      /* The value is split across two locations */
	      ಚಿಹ್ನೆರಹಿತ *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(ಚಿಹ್ನೆರಹಿತ *) ptr;
	      ip[1] = *(ಚಿಹ್ನೆರಹಿತ *) ರಾಶಿ_ವಾದಗಳು;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  ನಿಲ್ಲಿಸು;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ಆದರೆ (ptr == (ಅಕ್ಷರ *) &ರಿಜಿಸ್ಟರ್_ವಾದಗಳು[6])
	ptr = ರಾಶಿ_ವಾದಗಳು;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ptr == (ಅಕ್ಷರ *) &ರಿಜಿಸ್ಟರ್_ವಾದಗಳು[7])
	ptr = ರಾಶಿ_ವಾದಗಳು + 4;
    }

  /* Invoke the closure.  */
  ಆದರೆ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (ಆವರಣ->fun) (cif, struct_rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      /* Allocate space for the return value and call the function.  */
      ದೀರ್ಘ ದೀರ್ಘ rvalue;
      (ಆವರಣ->fun) (cif, &rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *tramp = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *) &ಆವರಣ->tramp[0];
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ fn = (ದೀರ್ಘ) ffi_ಆವರಣ_eabi;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ cls = (ದೀರ್ಘ) codeloc;

  ಆದರೆ (cif->abi != FFI_EABI)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  fn = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ffi_ಆವರಣ_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}
