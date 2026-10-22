/* -----------------------------------------------------------------------
   ffi.c - Copyright (C) 2004, 2026  Anthony Green
   Copyright (C) 2007  Free Software Foundation, Inc.
	   Copyright (C) 2008  Red Hat, Inc.
   
   FR-V Foreign Function Interface 

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
      /*      if ((*p_arg)->type == FFI_TYPE_FLOAT)
	{
	  if (count > 24)
	    {
	      // This is going on the stack.  Turn it into a double.  
	      *(double *) argp = (double) *(float*)(* p_argv);
	      z = sizeof(double);
	    }
	  else
	    *(void **) argp = *(void **)(* p_argv);
	}  */
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

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ಆದರೆ ((rvalue == ಖಾಲಿ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ಗಾತ್ರ_2);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;
    
  
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
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  ನೋಂದಣಿ ffi_ಆವರಣ *creg __asm__ ("gr7");
  ffi_ಆವರಣ *ಆವರಣ = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  ನೋಂದಣಿ ಅಕ್ಷರ *ಚೌಕಟ್ಟು_ಸೂಚಕ __asm__ ("fp");
  ಅಕ್ಷರ *ರಾಶಿ_ವಾದಗಳು = ಚೌಕಟ್ಟು_ಸೂಚಕ + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  ಚಿಹ್ನೆರಹಿತ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = ಆವರಣ->cif;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue = alloca (cif->nargs * ಗಾತ್ರ(ಶೂನ್ಯ *));
  ಅಕ್ಷರ *ptr = (ಅಕ್ಷರ *) ರಿಜಿಸ್ಟರ್_ವಾದಗಳು;
  ಪೂರ್ಣಾಂಕ i;

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
	  avalue[i] = ptr;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  avalue[i] = *(ಶೂನ್ಯ**)ptr;
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  ನಿಲ್ಲಿಸು;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ಆದರೆ (ptr == ((ಅಕ್ಷರ *)ರಿಜಿಸ್ಟರ್_ವಾದಗಳು + (6*4)))
	ptr = ರಾಶಿ_ವಾದಗಳು;
    }

  /* Invoke the closure.  */
  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      ನೋಂದಣಿ ಶೂನ್ಯ *return_struct_ptr __asm__("gr3");
      (ಆವರಣ->fun) (cif, return_struct_ptr, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      /* Allocate space for the return value and call the function.  */
      ದೀರ್ಘ ದೀರ್ಘ rvalue;
      (ಆವರಣ->fun) (cif, &rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((ಪೂರ್ಣಾಂಕ *) &rvalue)[1]));
    }
}

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &ಆವರಣ->tramp[0];
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ fn = (ದೀರ್ಘ) ffi_ಆವರಣ_eabi;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ cls = (ದೀರ್ಘ) codeloc;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __FRV_FDPIC__
  ನೋಂದಣಿ ಶೂನ್ಯ *got __asm__("gr15");
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ i;

  fn = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ffi_ಆವರಣ_eabi;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __FRV_FDPIC__
  tramp[0] = &((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#ಇಲ್ಲದಿದ್ದರೆ
  tramp[0] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[1] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[2] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[3] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[4] = 0x80300006;                  /* jmpl @(gr0, gr6)      */
#ಆದರೆ_ಅಂತ್ಯ

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  /* Cache flushing.  */
  ಗಾಗಿ (i = 0; i < FFI_TRAMPOLINE_ಗಾತ್ರ; i++)
    __asm__ ಅಸ್ಥಿರ ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  ಹಿಂತಿರುಗಿ FFI_OK;
}
