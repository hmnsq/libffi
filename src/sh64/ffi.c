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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NGREGARG 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NFREGARG 12

ਸਥਿਰ ਪੂਰਨ
return_type (ffi_type *arg)
{

  ਜੇ (arg->type != FFI_TYPE_STRUCT)
    ਵਾਪਸ arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  ਜੇ (arg->ਮਾਪ <= ਆਕਾਰ (UINT8))
    ਵਾਪਸ FFI_TYPE_UINT8;
  ਨਹੀਂਤਾਂ ਜੇ (arg->ਮਾਪ <= ਆਕਾਰ (UINT16))
    ਵਾਪਸ FFI_TYPE_UINT16;
  ਨਹੀਂਤਾਂ ਜੇ (arg->ਮਾਪ <= ਆਕਾਰ (UINT32))
    ਵਾਪਸ FFI_TYPE_UINT32;
  ਨਹੀਂਤਾਂ ਜੇ (arg->ਮਾਪ <= ਆਕਾਰ (UINT64))
    ਵਾਪਸ FFI_TYPE_UINT64;

  ਵਾਪਸ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ਖਾਲੀ ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, extended_cif *ecif)
{
  ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ avn;
  ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
  ਰਜਿਸਟਰ ਅੱਖਰ *argp;
  ਰਜਿਸਟਰ ffi_type **p_arg;

  argp = ਢੇਰ;

  ਜੇ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(ਖਾਲੀ **) argp = ecif->rvalue;
      argp += ਆਕਾਰ (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ਲਈ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      ਪੂਰਨ align;

      z = (*p_arg)->ਮਾਪ;
      align = (*p_arg)->alignment;
      ਜੇ (z < ਆਕਾਰ (UINT32))
	{
	  ਚੋਣ ((*p_arg)->type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_STRUCT:
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
	      ਰੋਕੋ;

	    ਮੂਲ:
	      FFI_ASSERT(0);
	    }
	  argp += ਆਕਾਰ (UINT64);
	}
      ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ (UINT32) && align == ਆਕਾਰ (UINT32))
	{
	  ਚੋਣ ((*p_arg)->type)
	    {
	    ਹਾਲਤ FFI_TYPE_INT:
	    ਹਾਲਤ FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_FLOAT:
	    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	    ਹਾਲਤ FFI_TYPE_UINT32:
	    ਹਾਲਤ FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      ਰੋਕੋ;

	    ਮੂਲ:
	      FFI_ASSERT(0);
	      ਰੋਕੋ;
	    }
	  argp += ਆਕਾਰ (UINT64);
	}
      ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ (UINT64)
	       && align == ਆਕਾਰ (UINT64)
	       && ((ਪੂਰਨ) *p_argv & (ਆਕਾਰ (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += ਆਕਾਰ (UINT64);
	}
      ਨਹੀਂਤਾਂ
	{
	  ਪੂਰਨ n = (z + ਆਕਾਰ (UINT64) - 1) / ਆਕਾਰ (UINT64);

	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
	  argp += n * ਆਕਾਰ (UINT64);
	}
    }

  ਵਾਪਸ;
}

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
  ਪੂਰਨ i, j;
  ਪੂਰਨ ਮਾਪ, type;
  ਪੂਰਨ n, m;
  ਪੂਰਨ greg;
  ਪੂਰਨ freg;
  ਪੂਰਨ fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  ਲਈ (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_FLOAT:
	  greg++;
	  cif->ਬਾਈਟਾਂ += ਆਕਾਰ (UINT64) - ਆਕਾਰ (ਦਸ਼ਮਲਵ);
	  ਜੇ (freg >= NFREGARG - 1)
	    ਜਾਰੀ;
	  ਜੇ (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  ਨਹੀਂਤਾਂ
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਜੇ (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    ਜਾਰੀ;
	  ਜੇ ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  ਨਹੀਂਤਾਂ
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ਰੋਕੋ;
	      
	ਮੂਲ:
	  ਮਾਪ = (cif->arg_types)[i]->ਮਾਪ;
	  ਜੇ (ਮਾਪ < ਆਕਾਰ (UINT64))
	    cif->ਬਾਈਟਾਂ += ਆਕਾਰ (UINT64) - ਮਾਪ;
	  n = (ਮਾਪ + ਆਕਾਰ (UINT64) - 1) / ਆਕਾਰ (UINT64);
	  ਜੇ (greg >= NGREGARG)
	    ਜਾਰੀ;
	  ਨਹੀਂਤਾਂ ਜੇ (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  ਨਹੀਂਤਾਂ
	    greg += n;
	  ਲਈ (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ਰੋਕੋ;
	}
    }

  /* Set the return type flag */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_VOID:
    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      ਰੋਕੋ;

    ਮੂਲ:
      cif->flags = FFI_TYPE_INT;
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV(ਖਾਲੀ (*)(ਅੱਖਰ *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਲੰਮਾ ਲੰਮਾ,
			  /*@out@*/ ਬਿਨਾਨਿਸ਼ਾਨੀ *, 
			  ਖਾਲੀ (*fn)(ਖਾਲੀ));
/*@=declundef@*/
/*@=exportheader@*/

ਖਾਲੀ ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      ਖਾਲੀ (*fn)(ਖਾਲੀ), 
	      /*@out@*/ ਖਾਲੀ *rvalue, 
	      /*@dependent@*/ ਖਾਲੀ **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  ਨਹੀਂਤਾਂ ਜੇ ((rvalue == ਨਿਰਾ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ਮਾਪ);
    }
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  ਚੋਣ (cif->abi) 
    {
    ਹਾਲਤ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_ਦਲੀਲਾਂ, &ecif, cif->ਬਾਈਟਾਂ, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      ਰੋਕੋ;
    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }

  ਜੇ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, &trvalue, cif->rtype->ਮਾਪ);
}

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_SYSV (ਖਾਲੀ);
ਬਾਹਰੀ ਖਾਲੀ __ic_invalidate (ਖਾਲੀ *ਸਤਰ);

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ *ਘੇਰਾ,
		      ffi_cif *cif,
		      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp;

  ਜੇ (cif->abi != FFI_SYSV)
    ਵਾਪਸ FFI_BAD_ABI;

  tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &ਘੇਰਾ->tramp[0];
  /* Since ffi_closure is an aligned object, the ffi trampoline is
     called as an SHcompact code.  Sigh.
     SHcompact part:
     mova @(1,pc),r0; add #1,r0; jmp @r0; nop;
     SHmedia part:
     movi fnaddr >> 16,r1; shori fnaddr,r1; ptabs/l r1,tr0
     movi cxt >> 16,r1; shori cxt,r1; blink tr0,r63  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
  tramp[0] = 0x7001c701;
  tramp[1] = 0x0009402b;
#ਨਹੀਂਤਾਂ
  tramp[0] = 0xc7017001;
  tramp[1] = 0x402b0009;
#ਜੇ_ਅੰਤ
  tramp[2] = 0xcc000010 | (((UINT32) ffi_ਘੇਰਾ_SYSV) >> 16) << 10;
  tramp[3] = 0xc8000010 | (((UINT32) ffi_ਘੇਰਾ_SYSV) & 0xffff) << 10;
  tramp[4] = 0x6bf10600;
  tramp[5] = 0xcc000010 | (((UINT32) codeloc) >> 16) << 10;
  tramp[6] = 0xc8000010 | (((UINT32) codeloc) & 0xffff) << 10;
  tramp[7] = 0x4401fff0;

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  /* Flush the icache.  */
  __asm__ ਅਸਥਿਰ ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  ਵਾਪਸ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

ਪੂਰਨ
ffi_ਘੇਰਾ_helper_SYSV (ffi_ਘੇਰਾ *ਘੇਰਾ, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  ਖਾਲੀ **avalue;
  ffi_type **p_arg;
  ਪੂਰਨ i, avn;
  ਪੂਰਨ greg, freg;
  ffi_cif *cif;
  ਪੂਰਨ fpair = -1;

  cif = ਘੇਰਾ->cif;
  avalue = alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ਜੇ (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  ਨਹੀਂਤਾਂ
    greg = 0;

  freg = 0;
  cif = ਘੇਰਾ->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  ਲਈ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      ਖਾਲੀ *p;

      z = (*p_arg)->ਮਾਪ;
      ਜੇ (z < ਆਕਾਰ (UINT32))
	{
	  p = pgr + greg++;

	  ਚੋਣ ((*p_arg)->type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT8:
	    ਹਾਲਤ FFI_TYPE_UINT8:
	    ਹਾਲਤ FFI_TYPE_SINT16:
	    ਹਾਲਤ FFI_TYPE_UINT16:
	    ਹਾਲਤ FFI_TYPE_STRUCT:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
	      avalue[i] = p;
#ਨਹੀਂਤਾਂ
	      avalue[i] = ((ਅੱਖਰ *) p) + ਆਕਾਰ (UINT32) - z;
#ਜੇ_ਅੰਤ
	      ਰੋਕੋ;

	    ਮੂਲ:
	      FFI_ASSERT(0);
	    }
	}
      ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ (UINT32))
	{
	  ਜੇ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ਜੇ (freg < NFREGARG - 1)
		{
		  ਜੇ (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  ਨਹੀਂਤਾਂ
		    {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#ਨਹੀਂਤਾਂ
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#ਜੇ_ਅੰਤ
		      freg += 2;
		    }
		}
	      ਨਹੀਂਤਾਂ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#ਨਹੀਂਤਾਂ
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#ਜੇ_ਅੰਤ
	    }
	  ਨਹੀਂਤਾਂ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#ਨਹੀਂਤਾਂ
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#ਜੇ_ਅੰਤ
	  greg++;
	}
      ਨਹੀਂਤਾਂ ਜੇ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ਜੇ (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  ਨਹੀਂਤਾਂ
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      ਨਹੀਂਤਾਂ
	{
	  ਪੂਰਨ n = (z + ਆਕਾਰ (UINT64) - 1) / ਆਕਾਰ (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (ਘੇਰਾ->fun) (cif, rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ਵਾਪਸ return_type (cif->rtype);
}

