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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>

#define NGREGARG 8
#define NFREGARG 12

நிலையான முழு
return_type (ffi_type *arg)
{

  எனில் (arg->type != FFI_TYPE_STRUCT)
    திரும்பு arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  எனில் (arg->size <= அளவு (UINT8))
    திரும்பு FFI_TYPE_UINT8;
  இல்லையெனில் எனில் (arg->size <= அளவு (UINT16))
    திரும்பு FFI_TYPE_UINT16;
  இல்லையெனில் எனில் (arg->size <= அளவு (UINT32))
    திரும்பு FFI_TYPE_UINT32;
  இல்லையெனில் எனில் (arg->size <= அளவு (UINT64))
    திரும்பு FFI_TYPE_UINT64;

  திரும்பு FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

வெறுமை ffi_prep_args(எழுத்து *அடுக்கு, extended_cif *ecif)
{
  பதிவு unsigned முழு i;
  பதிவு unsigned முழு avn;
  பதிவு வெறுமை **p_argv;
  பதிவு எழுத்து *argp;
  பதிவு ffi_type **p_arg;

  argp = அடுக்கு;

  எனில் (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(வெறுமை **) argp = ecif->rvalue;
      argp += அளவு (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  க்கு (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      முழு align;

      z = (*p_arg)->size;
      align = (*p_arg)->alignment;
      எனில் (z < அளவு (UINT32))
	{
	  தேர்வு ((*p_arg)->type)
	    {
	    நிலை FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      நிறுத்து;

	    இயல்பு:
	      FFI_ASSERT(0);
	    }
	  argp += அளவு (UINT64);
	}
      இல்லையெனில் எனில் (z == அளவு (UINT32) && align == அளவு (UINT32))
	{
	  தேர்வு ((*p_arg)->type)
	    {
	    நிலை FFI_TYPE_INT:
	    நிலை FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      நிறுத்து;

	    நிலை FFI_TYPE_FLOAT:
	    நிலை FFI_TYPE_POINTER:
	    நிலை FFI_TYPE_UINT32:
	    நிலை FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      நிறுத்து;

	    இயல்பு:
	      FFI_ASSERT(0);
	      நிறுத்து;
	    }
	  argp += அளவு (UINT64);
	}
      இல்லையெனில் எனில் (z == அளவு (UINT64)
	       && align == அளவு (UINT64)
	       && ((முழு) *p_argv & (அளவு (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += அளவு (UINT64);
	}
      இல்லையெனில்
	{
	  முழு n = (z + அளவு (UINT64) - 1) / அளவு (UINT64);

	  memcpy (argp, *p_argv, z);
	  argp += n * அளவு (UINT64);
	}
    }

  திரும்பு;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  முழு i, j;
  முழு size, type;
  முழு n, m;
  முழு greg;
  முழு freg;
  முழு fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  க்கு (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      தேர்வு (type)
	{
	நிலை FFI_TYPE_FLOAT:
	  greg++;
	  cif->bytes += அளவு (UINT64) - அளவு (தசம);
	  எனில் (freg >= NFREGARG - 1)
	    தொடர்;
	  எனில் (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  இல்லையெனில்
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	  எனில் (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    தொடர்;
	  எனில் ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  இல்லையெனில்
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  நிறுத்து;
	      
	இயல்பு:
	  size = (cif->arg_types)[i]->size;
	  எனில் (size < அளவு (UINT64))
	    cif->bytes += அளவு (UINT64) - size;
	  n = (size + அளவு (UINT64) - 1) / அளவு (UINT64);
	  எனில் (greg >= NGREGARG)
	    தொடர்;
	  இல்லையெனில் எனில் (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  இல்லையெனில்
	    greg += n;
	  க்கு (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  நிறுத்து;
	}
    }

  /* Set the return type flag */
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      நிறுத்து;

    நிலை FFI_TYPE_VOID:
    நிலை FFI_TYPE_FLOAT:
    நிலை FFI_TYPE_DOUBLE:
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      நிறுத்து;

    இயல்பு:
      cif->flags = FFI_TYPE_INT;
      நிறுத்து;
    }

  திரும்பு FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
வெளி வெறுமை ffi_call_SYSV(வெறுமை (*)(எழுத்து *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  unsigned, unsigned, long long,
			  /*@out@*/ unsigned *, 
			  வெறுமை (*fn)(வெறுமை));
/*@=declundef@*/
/*@=exportheader@*/

வெறுமை ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      வெறுமை (*fn)(வெறுமை), 
	      /*@out@*/ வெறுமை *rvalue, 
	      /*@dependent@*/ வெறுமை **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  எனில் (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  இல்லையெனில் எனில் ((rvalue == வெற்று) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  இல்லையெனில்
    ecif.rvalue = rvalue;

  தேர்வு (cif->abi) 
    {
    நிலை FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      நிறுத்து;
    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
    }

  எனில் (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

வெளி வெறுமை ffi_closure_SYSV (வெறுமை);
வெளி வெறுமை __ic_invalidate (வெறுமை *வரி);

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  unsigned முழு *tramp;

  எனில் (cif->abi != FFI_SYSV)
    திரும்பு FFI_BAD_ABI;

  tramp = (unsigned முழு *) &closure->tramp[0];
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
#இல்லையெனில்
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
  __asm__ நிலையற்ற ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  திரும்பு FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

முழு
ffi_closure_helper_SYSV (ffi_closure *closure, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  வெறுமை **avalue;
  ffi_type **p_arg;
  முழு i, avn;
  முழு greg, freg;
  ffi_cif *cif;
  முழு fpair = -1;

  cif = closure->cif;
  avalue = alloca (cif->nargs * அளவு (வெறுமை *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  எனில் (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  இல்லையெனில்
    greg = 0;

  freg = 0;
  cif = closure->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  க்கு (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      வெறுமை *p;

      z = (*p_arg)->size;
      எனில் (z < அளவு (UINT32))
	{
	  p = pgr + greg++;

	  தேர்வு ((*p_arg)->type)
	    {
	    நிலை FFI_TYPE_SINT8:
	    நிலை FFI_TYPE_UINT8:
	    நிலை FFI_TYPE_SINT16:
	    நிலை FFI_TYPE_UINT16:
	    நிலை FFI_TYPE_STRUCT:
#ifdef __LITTLE_ENDIAN__
	      avalue[i] = p;
#இல்லையெனில்
	      avalue[i] = ((எழுத்து *) p) + அளவு (UINT32) - z;
#endif
	      நிறுத்து;

	    இயல்பு:
	      FFI_ASSERT(0);
	    }
	}
      இல்லையெனில் எனில் (z == அளவு (UINT32))
	{
	  எனில் ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      எனில் (freg < NFREGARG - 1)
		{
		  எனில் (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  இல்லையெனில்
		    {
#ifdef __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#இல்லையெனில்
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#endif
		      freg += 2;
		    }
		}
	      இல்லையெனில்
#ifdef __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#இல்லையெனில்
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	    }
	  இல்லையெனில்
#ifdef __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#இல்லையெனில்
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	  greg++;
	}
      இல்லையெனில் எனில் ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  எனில் (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  இல்லையெனில்
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      இல்லையெனில்
	{
	  முழு n = (z + அளவு (UINT64) - 1) / அளவு (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  திரும்பு return_type (cif->rtype);
}

