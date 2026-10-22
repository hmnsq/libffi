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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#define NGREGARG 8
#define NFREGARG 12

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
return_type (ffi_type *arg)
{

  ଯଦି (arg->type != FFI_TYPE_STRUCT)
    ଫେରନ୍ତୁ arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  ଯଦି (arg->size <= ଆକାର (UINT8))
    ଫେରନ୍ତୁ FFI_TYPE_UINT8;
  ଅନ୍ୟଥା ଯଦି (arg->size <= ଆକାର (UINT16))
    ଫେରନ୍ତୁ FFI_TYPE_UINT16;
  ଅନ୍ୟଥା ଯଦି (arg->size <= ଆକାର (UINT32))
    ଫେରନ୍ତୁ FFI_TYPE_UINT32;
  ଅନ୍ୟଥା ଯଦି (arg->size <= ଆକାର (UINT64))
    ଫେରନ୍ତୁ FFI_TYPE_UINT64;

  ଫେରନ୍ତୁ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ଶୂନ୍ୟ ffi_prep_args(ଅକ୍ଷର *ଗଦା, extended_cif *ecif)
{
  ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା avn;
  ପଞ୍ଜିକରଣ ଶୂନ୍ୟ **p_argv;
  ପଞ୍ଜିକରଣ ଅକ୍ଷର *argp;
  ପଞ୍ଜିକରଣ ffi_type **p_arg;

  argp = ଗଦା;

  ଯଦି (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(ଶୂନ୍ୟ **) argp = ecif->rvalue;
      argp += ଆକାର (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ପାଇଁ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା align;

      z = (*p_arg)->size;
      align = (*p_arg)->alignment;
      ଯଦି (z < ଆକାର (UINT32))
	{
	  ବାଛ ((*p_arg)->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      ଭାଙ୍ଗ;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT(0);
	    }
	  argp += ଆକାର (UINT64);
	}
      ଅନ୍ୟଥା ଯଦି (z == ଆକାର (UINT32) && align == ଆକାର (UINT32))
	{
	  ବାଛ ((*p_arg)->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_INT:
	    ସ୍ଥିତି FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_FLOAT:
	    ସ୍ଥିତି FFI_TYPE_POINTER:
	    ସ୍ଥିତି FFI_TYPE_UINT32:
	    ସ୍ଥିତି FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      ଭାଙ୍ଗ;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT(0);
	      ଭାଙ୍ଗ;
	    }
	  argp += ଆକାର (UINT64);
	}
      ଅନ୍ୟଥା ଯଦି (z == ଆକାର (UINT64)
	       && align == ଆକାର (UINT64)
	       && ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *p_argv & (ଆକାର (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += ଆକାର (UINT64);
	}
      ଅନ୍ୟଥା
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (z + ଆକାର (UINT64) - 1) / ଆକାର (UINT64);

	  memcpy (argp, *p_argv, z);
	  argp += n * ଆକାର (UINT64);
	}
    }

  ଫେରନ୍ତୁ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, type;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, m;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା greg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା freg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  ପାଇଁ (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      ବାଛ (type)
	{
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  greg++;
	  cif->bytes += ଆକାର (UINT64) - ଆକାର (ଦଶମିକ);
	  ଯଦି (freg >= NFREGARG - 1)
	    ଜାରି;
	  ଯଦି (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  ଅନ୍ୟଥା
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  ଯଦି (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    ଜାରି;
	  ଯଦି ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  ଅନ୍ୟଥା
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ଭାଙ୍ଗ;
	      
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  size = (cif->arg_types)[i]->size;
	  ଯଦି (size < ଆକାର (UINT64))
	    cif->bytes += ଆକାର (UINT64) - size;
	  n = (size + ଆକାର (UINT64) - 1) / ଆକାର (UINT64);
	  ଯଦି (greg >= NGREGARG)
	    ଜାରି;
	  ଅନ୍ୟଥା ଯଦି (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  ଅନ୍ୟଥା
	    greg += n;
	  ପାଇଁ (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ଭାଙ୍ଗ;
	}
    }

  /* Set the return type flag */
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_VOID:
    ସ୍ଥିତି FFI_TYPE_FLOAT:
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      cif->flags = FFI_TYPE_INT;
      ଭାଙ୍ଗ;
    }

  ଫେରନ୍ତୁ FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV(ଶୂନ୍ୟ (*)(ଅକ୍ଷର *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  unsigned, unsigned, long long,
			  /*@out@*/ unsigned *, 
			  ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ));
/*@=declundef@*/
/*@=exportheader@*/

ଶୂନ୍ୟ ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), 
	      /*@out@*/ ଶୂନ୍ୟ *rvalue, 
	      /*@dependent@*/ ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  ଅନ୍ୟଥା ଯଦି ((rvalue == ଖାଲି) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  ବାଛ (cif->abi) 
    {
    ସ୍ଥିତି FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT(0);
      ଭାଙ୍ଗ;
    }

  ଯଦି (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_SYSV (ଶୂନ୍ୟ);
ବାହ୍ୟ ଶୂନ୍ୟ __ic_invalidate (ଶୂନ୍ୟ *ଧାଡ଼ି);

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tramp;

  ଯଦି (cif->abi != FFI_SYSV)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  tramp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &closure->tramp[0];
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
#ଅନ୍ୟଥା
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
  __asm__ ଅସ୍ଥିର ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  ଫେରନ୍ତୁ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_helper_SYSV (ffi_closure *closure, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  ଶୂନ୍ୟ **avalue;
  ffi_type **p_arg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, avn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା greg, freg;
  ffi_cif *cif;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fpair = -1;

  cif = closure->cif;
  avalue = alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ଯଦି (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  ଅନ୍ୟଥା
    greg = 0;

  freg = 0;
  cif = closure->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  ପାଇଁ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      ଶୂନ୍ୟ *p;

      z = (*p_arg)->size;
      ଯଦି (z < ଆକାର (UINT32))
	{
	  p = pgr + greg++;

	  ବାଛ ((*p_arg)->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	    ସ୍ଥିତି FFI_TYPE_UINT8:
	    ସ୍ଥିତି FFI_TYPE_SINT16:
	    ସ୍ଥିତି FFI_TYPE_UINT16:
	    ସ୍ଥିତି FFI_TYPE_STRUCT:
#ifdef __LITTLE_ENDIAN__
	      avalue[i] = p;
#ଅନ୍ୟଥା
	      avalue[i] = ((ଅକ୍ଷର *) p) + ଆକାର (UINT32) - z;
#endif
	      ଭାଙ୍ଗ;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT(0);
	    }
	}
      ଅନ୍ୟଥା ଯଦି (z == ଆକାର (UINT32))
	{
	  ଯଦି ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ଯଦି (freg < NFREGARG - 1)
		{
		  ଯଦି (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  ଅନ୍ୟଥା
		    {
#ifdef __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#ଅନ୍ୟଥା
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#endif
		      freg += 2;
		    }
		}
	      ଅନ୍ୟଥା
#ifdef __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#ଅନ୍ୟଥା
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	    }
	  ଅନ୍ୟଥା
#ifdef __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#ଅନ୍ୟଥା
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	  greg++;
	}
      ଅନ୍ୟଥା ଯଦି ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ଯଦି (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  ଅନ୍ୟଥା
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      ଅନ୍ୟଥା
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (z + ଆକାର (UINT64) - 1) / ଆକାର (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ଫେରନ୍ତୁ return_type (cif->rtype);
}

