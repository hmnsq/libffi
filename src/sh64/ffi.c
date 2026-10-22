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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>

#define NGREGARG 8
#define NFREGARG 12

স্থির পূর্ণাঙ্ক
return_type (ffi_type *arg)
{

  যদি (arg->type != FFI_TYPE_STRUCT)
    প্রত্যাগম arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  যদি (arg->size <= পরিমাণ (UINT8))
    প্রত্যাগম FFI_TYPE_UINT8;
  অন্যথা যদি (arg->size <= পরিমাণ (UINT16))
    প্রত্যাগম FFI_TYPE_UINT16;
  অন্যথা যদি (arg->size <= পরিমাণ (UINT32))
    প্রত্যাগম FFI_TYPE_UINT32;
  অন্যথা যদি (arg->size <= পরিমাণ (UINT64))
    প্রত্যাগম FFI_TYPE_UINT64;

  প্রত্যাগম FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

শূন্য ffi_prep_args(বর্ণ *স্তূপ, extended_cif *ecif)
{
  পঞ্জিকা unsigned পূর্ণাঙ্ক i;
  পঞ্জিকা unsigned পূর্ণাঙ্ক avn;
  পঞ্জিকা শূন্য **p_argv;
  পঞ্জিকা বর্ণ *argp;
  পঞ্জিকা ffi_type **p_arg;

  argp = স্তূপ;

  যদি (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(শূন্য **) argp = ecif->rvalue;
      argp += পরিমাণ (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  প্রতি (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      পূর্ণাঙ্ক align;

      z = (*p_arg)->size;
      align = (*p_arg)->alignment;
      যদি (z < পরিমাণ (UINT32))
	{
	  বিকল্প ((*p_arg)->type)
	    {
	    পক্ষ FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      ভঙ্গ;

	    স্বভাব:
	      FFI_ASSERT(0);
	    }
	  argp += পরিমাণ (UINT64);
	}
      অন্যথা যদি (z == পরিমাণ (UINT32) && align == পরিমাণ (UINT32))
	{
	  বিকল্প ((*p_arg)->type)
	    {
	    পক্ষ FFI_TYPE_INT:
	    পক্ষ FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      ভঙ্গ;

	    পক্ষ FFI_TYPE_FLOAT:
	    পক্ষ FFI_TYPE_POINTER:
	    পক্ষ FFI_TYPE_UINT32:
	    পক্ষ FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      ভঙ্গ;

	    স্বভাব:
	      FFI_ASSERT(0);
	      ভঙ্গ;
	    }
	  argp += পরিমাণ (UINT64);
	}
      অন্যথা যদি (z == পরিমাণ (UINT64)
	       && align == পরিমাণ (UINT64)
	       && ((পূর্ণাঙ্ক) *p_argv & (পরিমাণ (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += পরিমাণ (UINT64);
	}
      অন্যথা
	{
	  পূর্ণাঙ্ক n = (z + পরিমাণ (UINT64) - 1) / পরিমাণ (UINT64);

	  memcpy (argp, *p_argv, z);
	  argp += n * পরিমাণ (UINT64);
	}
    }

  প্রত্যাগম;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  পূর্ণাঙ্ক i, j;
  পূর্ণাঙ্ক size, type;
  পূর্ণাঙ্ক n, m;
  পূর্ণাঙ্ক greg;
  পূর্ণাঙ্ক freg;
  পূর্ণাঙ্ক fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  প্রতি (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      বিকল্প (type)
	{
	পক্ষ FFI_TYPE_FLOAT:
	  greg++;
	  cif->bytes += পরিমাণ (UINT64) - পরিমাণ (ভিন্ন);
	  যদি (freg >= NFREGARG - 1)
	    অনুবর্ত;
	  যদি (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  অন্যথা
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_DOUBLE:
	  যদি (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    অনুবর্ত;
	  যদি ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  অন্যথা
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ভঙ্গ;
	      
	স্বভাব:
	  size = (cif->arg_types)[i]->size;
	  যদি (size < পরিমাণ (UINT64))
	    cif->bytes += পরিমাণ (UINT64) - size;
	  n = (size + পরিমাণ (UINT64) - 1) / পরিমাণ (UINT64);
	  যদি (greg >= NGREGARG)
	    অনুবর্ত;
	  অন্যথা যদি (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  অন্যথা
	    greg += n;
	  প্রতি (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ভঙ্গ;
	}
    }

  /* Set the return type flag */
  বিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      ভঙ্গ;

    পক্ষ FFI_TYPE_VOID:
    পক্ষ FFI_TYPE_FLOAT:
    পক্ষ FFI_TYPE_DOUBLE:
    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      ভঙ্গ;

    স্বভাব:
      cif->flags = FFI_TYPE_INT;
      ভঙ্গ;
    }

  প্রত্যাগম FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
বাহ্য শূন্য ffi_call_SYSV(শূন্য (*)(বর্ণ *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  unsigned, unsigned, long long,
			  /*@out@*/ unsigned *, 
			  শূন্য (*fn)(শূন্য));
/*@=declundef@*/
/*@=exportheader@*/

শূন্য ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      শূন্য (*fn)(শূন্য), 
	      /*@out@*/ শূন্য *rvalue, 
	      /*@dependent@*/ শূন্য **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  যদি (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  অন্যথা যদি ((rvalue == রিক্ত) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  অন্যথা
    ecif.rvalue = rvalue;

  বিকল্প (cif->abi) 
    {
    পক্ষ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      ভঙ্গ;
    স্বভাব:
      FFI_ASSERT(0);
      ভঙ্গ;
    }

  যদি (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

বাহ্য শূন্য ffi_closure_SYSV (শূন্য);
বাহ্য শূন্য __ic_invalidate (শূন্য *সারি);

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_data,
		      শূন্য *codeloc)
{
  unsigned পূর্ণাঙ্ক *tramp;

  যদি (cif->abi != FFI_SYSV)
    প্রত্যাগম FFI_BAD_ABI;

  tramp = (unsigned পূর্ণাঙ্ক *) &closure->tramp[0];
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
#অন্যথা
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
  __asm__ চঞ্চল ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  প্রত্যাগম FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

পূর্ণাঙ্ক
ffi_closure_helper_SYSV (ffi_closure *closure, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  শূন্য **avalue;
  ffi_type **p_arg;
  পূর্ণাঙ্ক i, avn;
  পূর্ণাঙ্ক greg, freg;
  ffi_cif *cif;
  পূর্ণাঙ্ক fpair = -1;

  cif = closure->cif;
  avalue = alloca (cif->nargs * পরিমাণ (শূন্য *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  যদি (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  অন্যথা
    greg = 0;

  freg = 0;
  cif = closure->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  প্রতি (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      শূন্য *p;

      z = (*p_arg)->size;
      যদি (z < পরিমাণ (UINT32))
	{
	  p = pgr + greg++;

	  বিকল্প ((*p_arg)->type)
	    {
	    পক্ষ FFI_TYPE_SINT8:
	    পক্ষ FFI_TYPE_UINT8:
	    পক্ষ FFI_TYPE_SINT16:
	    পক্ষ FFI_TYPE_UINT16:
	    পক্ষ FFI_TYPE_STRUCT:
#ifdef __LITTLE_ENDIAN__
	      avalue[i] = p;
#অন্যথা
	      avalue[i] = ((বর্ণ *) p) + পরিমাণ (UINT32) - z;
#endif
	      ভঙ্গ;

	    স্বভাব:
	      FFI_ASSERT(0);
	    }
	}
      অন্যথা যদি (z == পরিমাণ (UINT32))
	{
	  যদি ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      যদি (freg < NFREGARG - 1)
		{
		  যদি (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  অন্যথা
		    {
#ifdef __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#অন্যথা
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#endif
		      freg += 2;
		    }
		}
	      অন্যথা
#ifdef __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#অন্যথা
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	    }
	  অন্যথা
#ifdef __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#অন্যথা
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	  greg++;
	}
      অন্যথা যদি ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  যদি (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  অন্যথা
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      অন্যথা
	{
	  পূর্ণাঙ্ক n = (z + পরিমাণ (UINT64) - 1) / পরিমাণ (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  প্রত্যাগম return_type (cif->rtype);
}

