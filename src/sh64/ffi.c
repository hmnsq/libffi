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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>

#define NGREGARG 8
#define NFREGARG 12

ایستا صحیح
return_type (ffi_type *arg)
{

  اگر (arg->type != FFI_TYPE_STRUCT)
    بازگشت arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  اگر (arg->size <= اندازه (UINT8))
    بازگشت FFI_TYPE_UINT8;
  وگرنه اگر (arg->size <= اندازه (UINT16))
    بازگشت FFI_TYPE_UINT16;
  وگرنه اگر (arg->size <= اندازه (UINT32))
    بازگشت FFI_TYPE_UINT32;
  وگرنه اگر (arg->size <= اندازه (UINT64))
    بازگشت FFI_TYPE_UINT64;

  بازگشت FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

تهی ffi_prep_args(نویسه *پشته, extended_cif *ecif)
{
  ثبات unsigned صحیح i;
  ثبات unsigned صحیح avn;
  ثبات تهی **p_argv;
  ثبات نویسه *argp;
  ثبات ffi_type **p_arg;

  argp = پشته;

  اگر (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(تهی **) argp = ecif->rvalue;
      argp += اندازه (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  برای (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      صحیح align;

      z = (*p_arg)->size;
      align = (*p_arg)->alignment;
      اگر (z < اندازه (UINT32))
	{
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      بشکن;

	    پیشفرض:
	      FFI_ASSERT(0);
	    }
	  argp += اندازه (UINT64);
	}
      وگرنه اگر (z == اندازه (UINT32) && align == اندازه (UINT32))
	{
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_INT:
	    حالت FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      بشکن;

	    حالت FFI_TYPE_FLOAT:
	    حالت FFI_TYPE_POINTER:
	    حالت FFI_TYPE_UINT32:
	    حالت FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      بشکن;

	    پیشفرض:
	      FFI_ASSERT(0);
	      بشکن;
	    }
	  argp += اندازه (UINT64);
	}
      وگرنه اگر (z == اندازه (UINT64)
	       && align == اندازه (UINT64)
	       && ((صحیح) *p_argv & (اندازه (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += اندازه (UINT64);
	}
      وگرنه
	{
	  صحیح n = (z + اندازه (UINT64) - 1) / اندازه (UINT64);

	  memcpy (argp, *p_argv, z);
	  argp += n * اندازه (UINT64);
	}
    }

  بازگشت;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  صحیح i, j;
  صحیح size, type;
  صحیح n, m;
  صحیح greg;
  صحیح freg;
  صحیح fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  برای (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      انتخاب (type)
	{
	حالت FFI_TYPE_FLOAT:
	  greg++;
	  cif->bytes += اندازه (UINT64) - اندازه (اعشاری);
	  اگر (freg >= NFREGARG - 1)
	    ادامه;
	  اگر (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  وگرنه
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  بشکن;

	حالت FFI_TYPE_DOUBLE:
	  اگر (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    ادامه;
	  اگر ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  وگرنه
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  بشکن;
	      
	پیشفرض:
	  size = (cif->arg_types)[i]->size;
	  اگر (size < اندازه (UINT64))
	    cif->bytes += اندازه (UINT64) - size;
	  n = (size + اندازه (UINT64) - 1) / اندازه (UINT64);
	  اگر (greg >= NGREGARG)
	    ادامه;
	  وگرنه اگر (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  وگرنه
	    greg += n;
	  برای (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  بشکن;
	}
    }

  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      بشکن;

    حالت FFI_TYPE_VOID:
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      بشکن;

    پیشفرض:
      cif->flags = FFI_TYPE_INT;
      بشکن;
    }

  بازگشت FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
بیرونی تهی ffi_call_SYSV(تهی (*)(نویسه *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  unsigned, unsigned, long long,
			  /*@out@*/ unsigned *, 
			  تهی (*fn)(تهی));
/*@=declundef@*/
/*@=exportheader@*/

تهی ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      تهی (*fn)(تهی), 
	      /*@out@*/ تهی *rvalue, 
	      /*@dependent@*/ تهی **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  اگر (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  وگرنه اگر ((rvalue == پوچ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  وگرنه
    ecif.rvalue = rvalue;

  انتخاب (cif->abi) 
    {
    حالت FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      بشکن;
    پیشفرض:
      FFI_ASSERT(0);
      بشکن;
    }

  اگر (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

بیرونی تهی ffi_closure_SYSV (تهی);
بیرونی تهی __ic_invalidate (تهی *سطر);

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  unsigned صحیح *tramp;

  اگر (cif->abi != FFI_SYSV)
    بازگشت FFI_BAD_ABI;

  tramp = (unsigned صحیح *) &closure->tramp[0];
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
#وگرنه
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
  __asm__ فرار ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  بازگشت FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

صحیح
ffi_closure_helper_SYSV (ffi_closure *closure, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  تهی **avalue;
  ffi_type **p_arg;
  صحیح i, avn;
  صحیح greg, freg;
  ffi_cif *cif;
  صحیح fpair = -1;

  cif = closure->cif;
  avalue = alloca (cif->nargs * اندازه (تهی *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  اگر (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  وگرنه
    greg = 0;

  freg = 0;
  cif = closure->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  برای (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      تهی *p;

      z = (*p_arg)->size;
      اگر (z < اندازه (UINT32))
	{
	  p = pgr + greg++;

	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	    حالت FFI_TYPE_UINT8:
	    حالت FFI_TYPE_SINT16:
	    حالت FFI_TYPE_UINT16:
	    حالت FFI_TYPE_STRUCT:
#ifdef __LITTLE_ENDIAN__
	      avalue[i] = p;
#وگرنه
	      avalue[i] = ((نویسه *) p) + اندازه (UINT32) - z;
#endif
	      بشکن;

	    پیشفرض:
	      FFI_ASSERT(0);
	    }
	}
      وگرنه اگر (z == اندازه (UINT32))
	{
	  اگر ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      اگر (freg < NFREGARG - 1)
		{
		  اگر (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  وگرنه
		    {
#ifdef __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#وگرنه
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#endif
		      freg += 2;
		    }
		}
	      وگرنه
#ifdef __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#وگرنه
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	    }
	  وگرنه
#ifdef __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#وگرنه
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	  greg++;
	}
      وگرنه اگر ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  اگر (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  وگرنه
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      وگرنه
	{
	  صحیح n = (z + اندازه (UINT64) - 1) / اندازه (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  بازگشت return_type (cif->rtype);
}

