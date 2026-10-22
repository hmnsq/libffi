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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>

#define NGREGARG 8
#define NFREGARG 12

સ્થિર પૂર્ણાંક
return_type (ffi_type *arg)
{

  જો (arg->type != FFI_TYPE_STRUCT)
    પાછા arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  જો (arg->size <= કદ (UINT8))
    પાછા FFI_TYPE_UINT8;
  નહીંતર જો (arg->size <= કદ (UINT16))
    પાછા FFI_TYPE_UINT16;
  નહીંતર જો (arg->size <= કદ (UINT32))
    પાછા FFI_TYPE_UINT32;
  નહીંતર જો (arg->size <= કદ (UINT64))
    પાછા FFI_TYPE_UINT64;

  પાછા FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

શૂન્ય ffi_prep_args(અક્ષર *ઢગલો, extended_cif *ecif)
{
  નોંધણી unsigned પૂર્ણાંક i;
  નોંધણી unsigned પૂર્ણાંક avn;
  નોંધણી શૂન્ય **p_argv;
  નોંધણી અક્ષર *argp;
  નોંધણી ffi_type **p_arg;

  argp = ઢગલો;

  જો (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(શૂન્ય **) argp = ecif->rvalue;
      argp += કદ (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  માટે (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      પૂર્ણાંક align;

      z = (*p_arg)->size;
      align = (*p_arg)->alignment;
      જો (z < કદ (UINT32))
	{
	  પસંદગી ((*p_arg)->type)
	    {
	    સ્થિતિ FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      તોડો;

	    મૂળભૂત:
	      FFI_ASSERT(0);
	    }
	  argp += કદ (UINT64);
	}
      નહીંતર જો (z == કદ (UINT32) && align == કદ (UINT32))
	{
	  પસંદગી ((*p_arg)->type)
	    {
	    સ્થિતિ FFI_TYPE_INT:
	    સ્થિતિ FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      તોડો;

	    સ્થિતિ FFI_TYPE_FLOAT:
	    સ્થિતિ FFI_TYPE_POINTER:
	    સ્થિતિ FFI_TYPE_UINT32:
	    સ્થિતિ FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      તોડો;

	    મૂળભૂત:
	      FFI_ASSERT(0);
	      તોડો;
	    }
	  argp += કદ (UINT64);
	}
      નહીંતર જો (z == કદ (UINT64)
	       && align == કદ (UINT64)
	       && ((પૂર્ણાંક) *p_argv & (કદ (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += કદ (UINT64);
	}
      નહીંતર
	{
	  પૂર્ણાંક n = (z + કદ (UINT64) - 1) / કદ (UINT64);

	  memcpy (argp, *p_argv, z);
	  argp += n * કદ (UINT64);
	}
    }

  પાછા;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  પૂર્ણાંક i, j;
  પૂર્ણાંક size, type;
  પૂર્ણાંક n, m;
  પૂર્ણાંક greg;
  પૂર્ણાંક freg;
  પૂર્ણાંક fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  માટે (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      પસંદગી (type)
	{
	સ્થિતિ FFI_TYPE_FLOAT:
	  greg++;
	  cif->bytes += કદ (UINT64) - કદ (દશાંશ);
	  જો (freg >= NFREGARG - 1)
	    ચાલુ;
	  જો (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  નહીંતર
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	  જો (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    ચાલુ;
	  જો ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  નહીંતર
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  તોડો;
	      
	મૂળભૂત:
	  size = (cif->arg_types)[i]->size;
	  જો (size < કદ (UINT64))
	    cif->bytes += કદ (UINT64) - size;
	  n = (size + કદ (UINT64) - 1) / કદ (UINT64);
	  જો (greg >= NGREGARG)
	    ચાલુ;
	  નહીંતર જો (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  નહીંતર
	    greg += n;
	  માટે (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  તોડો;
	}
    }

  /* Set the return type flag */
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      તોડો;

    સ્થિતિ FFI_TYPE_VOID:
    સ્થિતિ FFI_TYPE_FLOAT:
    સ્થિતિ FFI_TYPE_DOUBLE:
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      તોડો;

    મૂળભૂત:
      cif->flags = FFI_TYPE_INT;
      તોડો;
    }

  પાછા FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
બાહ્ય શૂન્ય ffi_call_SYSV(શૂન્ય (*)(અક્ષર *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  unsigned, unsigned, long long,
			  /*@out@*/ unsigned *, 
			  શૂન્ય (*fn)(શૂન્ય));
/*@=declundef@*/
/*@=exportheader@*/

શૂન્ય ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      શૂન્ય (*fn)(શૂન્ય), 
	      /*@out@*/ શૂન્ય *rvalue, 
	      /*@dependent@*/ શૂન્ય **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  જો (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  નહીંતર જો ((rvalue == ખાલી) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  નહીંતર
    ecif.rvalue = rvalue;

  પસંદગી (cif->abi) 
    {
    સ્થિતિ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      તોડો;
    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
    }

  જો (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

બાહ્ય શૂન્ય ffi_closure_SYSV (શૂન્ય);
બાહ્ય શૂન્ય __ic_invalidate (શૂન્ય *લીટી);

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  unsigned પૂર્ણાંક *tramp;

  જો (cif->abi != FFI_SYSV)
    પાછા FFI_BAD_ABI;

  tramp = (unsigned પૂર્ણાંક *) &closure->tramp[0];
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
#નહીંતર
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
  __asm__ અસ્થિર ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  પાછા FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

પૂર્ણાંક
ffi_closure_helper_SYSV (ffi_closure *closure, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  શૂન્ય **avalue;
  ffi_type **p_arg;
  પૂર્ણાંક i, avn;
  પૂર્ણાંક greg, freg;
  ffi_cif *cif;
  પૂર્ણાંક fpair = -1;

  cif = closure->cif;
  avalue = alloca (cif->nargs * કદ (શૂન્ય *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  જો (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  નહીંતર
    greg = 0;

  freg = 0;
  cif = closure->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  માટે (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      શૂન્ય *p;

      z = (*p_arg)->size;
      જો (z < કદ (UINT32))
	{
	  p = pgr + greg++;

	  પસંદગી ((*p_arg)->type)
	    {
	    સ્થિતિ FFI_TYPE_SINT8:
	    સ્થિતિ FFI_TYPE_UINT8:
	    સ્થિતિ FFI_TYPE_SINT16:
	    સ્થિતિ FFI_TYPE_UINT16:
	    સ્થિતિ FFI_TYPE_STRUCT:
#ifdef __LITTLE_ENDIAN__
	      avalue[i] = p;
#નહીંતર
	      avalue[i] = ((અક્ષર *) p) + કદ (UINT32) - z;
#endif
	      તોડો;

	    મૂળભૂત:
	      FFI_ASSERT(0);
	    }
	}
      નહીંતર જો (z == કદ (UINT32))
	{
	  જો ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      જો (freg < NFREGARG - 1)
		{
		  જો (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  નહીંતર
		    {
#ifdef __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#નહીંતર
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#endif
		      freg += 2;
		    }
		}
	      નહીંતર
#ifdef __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#નહીંતર
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	    }
	  નહીંતર
#ifdef __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#નહીંતર
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	  greg++;
	}
      નહીંતર જો ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  જો (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  નહીંતર
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      નહીંતર
	{
	  પૂર્ણાંક n = (z + કદ (UINT64) - 1) / કદ (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  પાછા return_type (cif->rtype);
}

