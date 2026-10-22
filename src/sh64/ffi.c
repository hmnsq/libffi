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

#define NGREGARG 8
#define NFREGARG 12

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
return_type (ffi_type *arg)
{

  ಯದಿ (arg->type != FFI_TYPE_STRUCT)
    ಪ್ರತ್ಯಾಗಮ arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  ಯದಿ (arg->size <= ಪರಿಮಾಣ (UINT8))
    ಪ್ರತ್ಯಾಗಮ FFI_TYPE_UINT8;
  ಅನ್ಯಥಾ ಯದಿ (arg->size <= ಪರಿಮಾಣ (UINT16))
    ಪ್ರತ್ಯಾಗಮ FFI_TYPE_UINT16;
  ಅನ್ಯಥಾ ಯದಿ (arg->size <= ಪರಿಮಾಣ (UINT32))
    ಪ್ರತ್ಯಾಗಮ FFI_TYPE_UINT32;
  ಅನ್ಯಥಾ ಯದಿ (arg->size <= ಪರಿಮಾಣ (UINT64))
    ಪ್ರತ್ಯಾಗಮ FFI_TYPE_UINT64;

  ಪ್ರತ್ಯಾಗಮ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಶೂನ್ಯ ffi_prep_args(ವರ್ಣ *ರಾಶಿ, extended_cif *ecif)
{
  ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ avn;
  ಪಞ್ಜಿಕಾ ಶೂನ್ಯ **p_argv;
  ಪಞ್ಜಿಕಾ ವರ್ಣ *argp;
  ಪಞ್ಜಿಕಾ ffi_type **p_arg;

  argp = ರಾಶಿ;

  ಯದಿ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) argp = ecif->rvalue;
      argp += ಪರಿಮಾಣ (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      ಪೂರ್ಣಾಙ್ಕ align;

      z = (*p_arg)->size;
      align = (*p_arg)->alignment;
      ಯದಿ (z < ಪರಿಮಾಣ (UINT32))
	{
	  ವಿಕಲ್ಪ ((*p_arg)->type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      ಭಙ್ಗ;

	    ಸ್ವಭಾವ:
	      FFI_ASSERT(0);
	    }
	  argp += ಪರಿಮಾಣ (UINT64);
	}
      ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ (UINT32) && align == ಪರಿಮಾಣ (UINT32))
	{
	  ವಿಕಲ್ಪ ((*p_arg)->type)
	    {
	    ಪಕ್ಷ FFI_TYPE_INT:
	    ಪಕ್ಷ FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_FLOAT:
	    ಪಕ್ಷ FFI_TYPE_POINTER:
	    ಪಕ್ಷ FFI_TYPE_UINT32:
	    ಪಕ್ಷ FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      ಭಙ್ಗ;

	    ಸ್ವಭಾವ:
	      FFI_ASSERT(0);
	      ಭಙ್ಗ;
	    }
	  argp += ಪರಿಮಾಣ (UINT64);
	}
      ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ (UINT64)
	       && align == ಪರಿಮಾಣ (UINT64)
	       && ((ಪೂರ್ಣಾಙ್ಕ) *p_argv & (ಪರಿಮಾಣ (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += ಪರಿಮಾಣ (UINT64);
	}
      ಅನ್ಯಥಾ
	{
	  ಪೂರ್ಣಾಙ್ಕ n = (z + ಪರಿಮಾಣ (UINT64) - 1) / ಪರಿಮಾಣ (UINT64);

	  memcpy (argp, *p_argv, z);
	  argp += n * ಪರಿಮಾಣ (UINT64);
	}
    }

  ಪ್ರತ್ಯಾಗಮ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ಪೂರ್ಣಾಙ್ಕ size, type;
  ಪೂರ್ಣಾಙ್ಕ n, m;
  ಪೂರ್ಣಾಙ್ಕ greg;
  ಪೂರ್ಣಾಙ್ಕ freg;
  ಪೂರ್ಣಾಙ್ಕ fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  ಪ್ರತಿ (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  greg++;
	  cif->bytes += ಪರಿಮಾಣ (UINT64) - ಪರಿಮಾಣ (ಭಿನ್ನ);
	  ಯದಿ (freg >= NFREGARG - 1)
	    ಅನುವರ್ತ;
	  ಯದಿ (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  ಅನ್ಯಥಾ
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  ಯದಿ (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    ಅನುವರ್ತ;
	  ಯದಿ ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  ಅನ್ಯಥಾ
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ಭಙ್ಗ;
	      
	ಸ್ವಭಾವ:
	  size = (cif->arg_types)[i]->size;
	  ಯದಿ (size < ಪರಿಮಾಣ (UINT64))
	    cif->bytes += ಪರಿಮಾಣ (UINT64) - size;
	  n = (size + ಪರಿಮಾಣ (UINT64) - 1) / ಪರಿಮಾಣ (UINT64);
	  ಯದಿ (greg >= NGREGARG)
	    ಅನುವರ್ತ;
	  ಅನ್ಯಥಾ ಯದಿ (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  ಅನ್ಯಥಾ
	    greg += n;
	  ಪ್ರತಿ (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ಭಙ್ಗ;
	}
    }

  /* Set the return type flag */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_VOID:
    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      cif->flags = FFI_TYPE_INT;
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ (*)(ವರ್ಣ *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  unsigned, unsigned, long long,
			  /*@out@*/ unsigned *, 
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

  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  ಅನ್ಯಥಾ ಯದಿ ((rvalue == ರಿಕ್ತ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  ವಿಕಲ್ಪ (cif->abi) 
    {
    ಪಕ್ಷ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }

  ಯದಿ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_SYSV (ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ __ic_invalidate (ಶೂನ್ಯ *ಸಾಲು);

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ *tramp;

  ಯದಿ (cif->abi != FFI_SYSV)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  tramp = (unsigned ಪೂರ್ಣಾಙ್ಕ *) &closure->tramp[0];
  /* Since ffi_closure is an aligned object, the ffi trampoline is
     called as an SHcompact code.  Sigh.
     SHcompact part:
     mova @(1,pc),r0; add #1,r0; jmp @r0; nop;
     SHmedia part:
     movi fnaddr >> 16,r1; shori fnaddr,r1; ptabs/l r1,tr0
     movi cxt >> 16,r1; shori cxt,r1; blink tr0,r63  */
#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0x7001c701;
  tramp[1] = 0x0009402b;
#ಅನ್ಯಥಾ
  tramp[0] = 0xc7017001;
  tramp[1] = 0x402b0009;
#endif
  tramp[2] = 0xcc000010 | (((UINT32) ffi_closure_SYSV) >> 16) << 10;
  tramp[3] = 0xc8000010 | (((UINT32) ffi_closure_SYSV) & 0xffff) << 10;
  tramp[4] = 0x6bf10600;
  tramp[5] = 0xcc000010 | (((UINT32) codeloc) >> 16) << 10;
  tramp[6] = 0xc8000010 | (((UINT32) codeloc) & 0xffff) << 10;
  tramp[7] = 0x4401fff0;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Flush the icache.  */
  __asm__ ಚಞ್ಚಲ ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

ಪೂರ್ಣಾಙ್ಕ
ffi_closure_helper_SYSV (ffi_closure *closure, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  ಶೂನ್ಯ **avalue;
  ffi_type **p_arg;
  ಪೂರ್ಣಾಙ್ಕ i, avn;
  ಪೂರ್ಣಾಙ್ಕ greg, freg;
  ffi_cif *cif;
  ಪೂರ್ಣಾಙ್ಕ fpair = -1;

  cif = closure->cif;
  avalue = alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ಯದಿ (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  ಅನ್ಯಥಾ
    greg = 0;

  freg = 0;
  cif = closure->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಪ್ರತಿ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      ಶೂನ್ಯ *p;

      z = (*p_arg)->size;
      ಯದಿ (z < ಪರಿಮಾಣ (UINT32))
	{
	  p = pgr + greg++;

	  ವಿಕಲ್ಪ ((*p_arg)->type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	    ಪಕ್ಷ FFI_TYPE_UINT8:
	    ಪಕ್ಷ FFI_TYPE_SINT16:
	    ಪಕ್ಷ FFI_TYPE_UINT16:
	    ಪಕ್ಷ FFI_TYPE_STRUCT:
#ifdef __LITTLE_ENDIAN__
	      avalue[i] = p;
#ಅನ್ಯಥಾ
	      avalue[i] = ((ವರ್ಣ *) p) + ಪರಿಮಾಣ (UINT32) - z;
#endif
	      ಭಙ್ಗ;

	    ಸ್ವಭಾವ:
	      FFI_ASSERT(0);
	    }
	}
      ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ (UINT32))
	{
	  ಯದಿ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ಯದಿ (freg < NFREGARG - 1)
		{
		  ಯದಿ (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  ಅನ್ಯಥಾ
		    {
#ifdef __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#ಅನ್ಯಥಾ
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#endif
		      freg += 2;
		    }
		}
	      ಅನ್ಯಥಾ
#ifdef __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#ಅನ್ಯಥಾ
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	    }
	  ಅನ್ಯಥಾ
#ifdef __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#ಅನ್ಯಥಾ
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	  greg++;
	}
      ಅನ್ಯಥಾ ಯದಿ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ಯದಿ (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  ಅನ್ಯಥಾ
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      ಅನ್ಯಥಾ
	{
	  ಪೂರ್ಣಾಙ್ಕ n = (z + ಪರಿಮಾಣ (UINT64) - 1) / ಪರಿಮಾಣ (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ಪ್ರತ್ಯಾಗಮ return_type (cif->rtype);
}

