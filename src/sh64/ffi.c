/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2003, 2004, 2006, 2007, 2012 Kaz Kojima
           Copyright (c) 2008, 2026 Anthony Green
   
   SuperH SHmedia Foreign Function Interface 

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

#ವ್ಯಾಖ್ಯಾನಿಸು NGREGARG 8
#ವ್ಯಾಖ್ಯಾನಿಸು NFREGARG 12

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
return_type (ffi_type *arg)
{

  ಆದರೆ (arg->type != FFI_TYPE_STRUCT)
    ಹಿಂತಿರುಗಿ arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  ಆದರೆ (arg->ಗಾತ್ರ_2 <= ಗಾತ್ರ (UINT8))
    ಹಿಂತಿರುಗಿ FFI_TYPE_UINT8;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg->ಗಾತ್ರ_2 <= ಗಾತ್ರ (UINT16))
    ಹಿಂತಿರುಗಿ FFI_TYPE_UINT16;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg->ಗಾತ್ರ_2 <= ಗಾತ್ರ (UINT32))
    ಹಿಂತಿರುಗಿ FFI_TYPE_UINT32;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg->ಗಾತ್ರ_2 <= ಗಾತ್ರ (UINT64))
    ಹಿಂತಿರುಗಿ FFI_TYPE_UINT64;

  ಹಿಂತಿರುಗಿ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಶೂನ್ಯ ffi_prep_ವಾದಗಳು(ಅಕ್ಷರ *ರಾಶಿ, extended_cif *ecif)
{
  ನೋಂದಣಿ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ನೋಂದಣಿ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ avn;
  ನೋಂದಣಿ ಶೂನ್ಯ **p_argv;
  ನೋಂದಣಿ ಅಕ್ಷರ *argp;
  ನೋಂದಣಿ ffi_type **p_arg;

  argp = ರಾಶಿ;

  ಆದರೆ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) argp = ecif->rvalue;
      argp += ಗಾತ್ರ (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ಗಾಗಿ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      ಪೂರ್ಣಾಂಕ align;

      z = (*p_arg)->ಗಾತ್ರ_2;
      align = (*p_arg)->alignment;
      ಆದರೆ (z < ಗಾತ್ರ (UINT32))
	{
	  ಆಯ್ಕೆ ((*p_arg)->type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	      ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
	      ನಿಲ್ಲಿಸು;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      FFI_ASSERT(0);
	    }
	  argp += ಗಾತ್ರ (UINT64);
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z == ಗಾತ್ರ (UINT32) && align == ಗಾತ್ರ (UINT32))
	{
	  ಆಯ್ಕೆ ((*p_arg)->type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_INT:
	    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      ನಿಲ್ಲಿಸು;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      FFI_ASSERT(0);
	      ನಿಲ್ಲಿಸು;
	    }
	  argp += ಗಾತ್ರ (UINT64);
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z == ಗಾತ್ರ (UINT64)
	       && align == ಗಾತ್ರ (UINT64)
	       && ((ಪೂರ್ಣಾಂಕ) *p_argv & (ಗಾತ್ರ (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += ಗಾತ್ರ (UINT64);
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಪೂರ್ಣಾಂಕ n = (z + ಗಾತ್ರ (UINT64) - 1) / ಗಾತ್ರ (UINT64);

	  ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
	  argp += n * ಗಾತ್ರ (UINT64);
	}
    }

  ಹಿಂತಿರುಗಿ;
}

/* Perform machine dependent cif processing */
ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep(ffi_cif *cif)
{
  ಪೂರ್ಣಾಂಕ i, j;
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, type;
  ಪೂರ್ಣಾಂಕ n, m;
  ಪೂರ್ಣಾಂಕ greg;
  ಪೂರ್ಣಾಂಕ freg;
  ಪೂರ್ಣಾಂಕ fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  ಗಾಗಿ (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  greg++;
	  cif->ಬೈಟುಗಳು += ಗಾತ್ರ (UINT64) - ಗಾತ್ರ (ದಶಮಾಂಶ);
	  ಆದರೆ (freg >= NFREGARG - 1)
	    ಮುಂದುವರಿ;
	  ಆದರೆ (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಆದರೆ (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    ಮುಂದುವರಿ;
	  ಆದರೆ ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ನಿಲ್ಲಿಸು;
	      
	ಪೂರ್ವನಿಯೋಜಿತ:
	  ಗಾತ್ರ_2 = (cif->arg_types)[i]->ಗಾತ್ರ_2;
	  ಆದರೆ (ಗಾತ್ರ_2 < ಗಾತ್ರ (UINT64))
	    cif->ಬೈಟುಗಳು += ಗಾತ್ರ (UINT64) - ಗಾತ್ರ_2;
	  n = (ಗಾತ್ರ_2 + ಗಾತ್ರ (UINT64) - 1) / ಗಾತ್ರ (UINT64);
	  ಆದರೆ (greg >= NGREGARG)
	    ಮುಂದುವರಿ;
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  ಇಲ್ಲದಿದ್ದರೆ
	    greg += n;
	  ಗಾಗಿ (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ನಿಲ್ಲಿಸು;
	}
    }

  /* Set the return type flag */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_VOID:
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      cif->flags = FFI_TYPE_INT;
      ನಿಲ್ಲಿಸು;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ (*)(ಅಕ್ಷರ *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ, ದೀರ್ಘ ದೀರ್ಘ,
			  /*@out@*/ ಚಿಹ್ನೆರಹಿತ *, 
			  ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));
/*@=declundef@*/
/*@=exportheader@*/

ಶೂನ್ಯ ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), 
	      /*@out@*/ ಶೂನ್ಯ *rvalue, 
	      /*@dependent@*/ ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((rvalue == ಖಾಲಿ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ಗಾತ್ರ_2);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;

  ಆಯ್ಕೆ (cif->abi) 
    {
    ಸ್ಥಿತಿ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_ವಾದಗಳು, &ecif, cif->ಬೈಟುಗಳು, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }

  ಆದರೆ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ಸ್ಮರಣೆ_ನಕಲು (rvalue, &trvalue, cif->rtype->ಗಾತ್ರ_2);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_SYSV (ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ __ic_invalidate (ಶೂನ್ಯ *ಸಾಲು);

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ *ಆವರಣ,
		      ffi_cif *cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp;

  ಆದರೆ (cif->abi != FFI_SYSV)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &ಆವರಣ->tramp[0];
  /* Since ffi_closure is an aligned object, the ffi trampoline is
     called as an SHcompact code.  Sigh.
     SHcompact part:
     mova @(1,pc),r0; add #1,r0; jmp @r0; nop;
     SHmedia part:
     movi fnaddr >> 16,r1; shori fnaddr,r1; ptabs/l r1,tr0
     movi cxt >> 16,r1; shori cxt,r1; blink tr0,r63  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN__
  tramp[0] = 0x7001c701;
  tramp[1] = 0x0009402b;
#ಇಲ್ಲದಿದ್ದರೆ
  tramp[0] = 0xc7017001;
  tramp[1] = 0x402b0009;
#ಆದರೆ_ಅಂತ್ಯ
  tramp[2] = 0xcc000010 | (((UINT32) ffi_ಆವರಣ_SYSV) >> 16) << 10;
  tramp[3] = 0xc8000010 | (((UINT32) ffi_ಆವರಣ_SYSV) & 0xffff) << 10;
  tramp[4] = 0x6bf10600;
  tramp[5] = 0xcc000010 | (((UINT32) codeloc) >> 16) << 10;
  tramp[6] = 0xc8000010 | (((UINT32) codeloc) & 0xffff) << 10;
  tramp[7] = 0x4401fff0;

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  /* Flush the icache.  */
  __asm__ ಅಸ್ಥಿರ ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_helper_SYSV (ffi_ಆವರಣ *ಆವರಣ, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  ಶೂನ್ಯ **avalue;
  ffi_type **p_arg;
  ಪೂರ್ಣಾಂಕ i, avn;
  ಪೂರ್ಣಾಂಕ greg, freg;
  ffi_cif *cif;
  ಪೂರ್ಣಾಂಕ fpair = -1;

  cif = ಆವರಣ->cif;
  avalue = alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ಆದರೆ (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    greg = 0;

  freg = 0;
  cif = ಆವರಣ->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಗಾಗಿ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      ಶೂನ್ಯ *p;

      z = (*p_arg)->ಗಾತ್ರ_2;
      ಆದರೆ (z < ಗಾತ್ರ (UINT32))
	{
	  p = pgr + greg++;

	  ಆಯ್ಕೆ ((*p_arg)->type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN__
	      avalue[i] = p;
#ಇಲ್ಲದಿದ್ದರೆ
	      avalue[i] = ((ಅಕ್ಷರ *) p) + ಗಾತ್ರ (UINT32) - z;
#ಆದರೆ_ಅಂತ್ಯ
	      ನಿಲ್ಲಿಸು;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      FFI_ASSERT(0);
	    }
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z == ಗಾತ್ರ (UINT32))
	{
	  ಆದರೆ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ಆದರೆ (freg < NFREGARG - 1)
		{
		  ಆದರೆ (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  ಇಲ್ಲದಿದ್ದರೆ
		    {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#ಇಲ್ಲದಿದ್ದರೆ
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#ಆದರೆ_ಅಂತ್ಯ
		      freg += 2;
		    }
		}
	      ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#ಇಲ್ಲದಿದ್ದರೆ
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#ಆದರೆ_ಅಂತ್ಯ
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#ಇಲ್ಲದಿದ್ದರೆ
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#ಆದರೆ_ಅಂತ್ಯ
	  greg++;
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ಆದರೆ (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಪೂರ್ಣಾಂಕ n = (z + ಗಾತ್ರ (UINT64) - 1) / ಗಾತ್ರ (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (ಆವರಣ->fun) (cif, rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ಹಿಂತಿರುಗಿ return_type (cif->rtype);
}

