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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>

#define NGREGARG 8
#define NFREGARG 12

സ്ഥിതം പൂർണ്ണസംഖ്യ
return_type (ffi_type *arg)
{

  എങ്കിൽ (arg->type != FFI_TYPE_STRUCT)
    തിരികെ arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  എങ്കിൽ (arg->size <= വലുപ്പം (UINT8))
    തിരികെ FFI_TYPE_UINT8;
  അല്ലെങ്കിൽ എങ്കിൽ (arg->size <= വലുപ്പം (UINT16))
    തിരികെ FFI_TYPE_UINT16;
  അല്ലെങ്കിൽ എങ്കിൽ (arg->size <= വലുപ്പം (UINT32))
    തിരികെ FFI_TYPE_UINT32;
  അല്ലെങ്കിൽ എങ്കിൽ (arg->size <= വലുപ്പം (UINT64))
    തിരികെ FFI_TYPE_UINT64;

  തിരികെ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ശൂന്യം ffi_prep_args(അക്ഷരം *അട്ടി, extended_cif *ecif)
{
  രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ i;
  രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ avn;
  രജിസ്റ്റർ ശൂന്യം **p_argv;
  രജിസ്റ്റർ അക്ഷരം *argp;
  രജിസ്റ്റർ ffi_type **p_arg;

  argp = അട്ടി;

  എങ്കിൽ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(ശൂന്യം **) argp = ecif->rvalue;
      argp += വലുപ്പം (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  വേണ്ടി (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      പൂർണ്ണസംഖ്യ align;

      z = (*p_arg)->size;
      align = (*p_arg)->alignment;
      എങ്കിൽ (z < വലുപ്പം (UINT32))
	{
	  തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      നിർത്തുക;

	    സ്ഥിരസ്ഥിതി:
	      FFI_ASSERT(0);
	    }
	  argp += വലുപ്പം (UINT64);
	}
      അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം (UINT32) && align == വലുപ്പം (UINT32))
	{
	  തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
	    {
	    അവസ്ഥ FFI_TYPE_INT:
	    അവസ്ഥ FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_FLOAT:
	    അവസ്ഥ FFI_TYPE_POINTER:
	    അവസ്ഥ FFI_TYPE_UINT32:
	    അവസ്ഥ FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      നിർത്തുക;

	    സ്ഥിരസ്ഥിതി:
	      FFI_ASSERT(0);
	      നിർത്തുക;
	    }
	  argp += വലുപ്പം (UINT64);
	}
      അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം (UINT64)
	       && align == വലുപ്പം (UINT64)
	       && ((പൂർണ്ണസംഖ്യ) *p_argv & (വലുപ്പം (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += വലുപ്പം (UINT64);
	}
      അല്ലെങ്കിൽ
	{
	  പൂർണ്ണസംഖ്യ n = (z + വലുപ്പം (UINT64) - 1) / വലുപ്പം (UINT64);

	  memcpy (argp, *p_argv, z);
	  argp += n * വലുപ്പം (UINT64);
	}
    }

  തിരികെ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  പൂർണ്ണസംഖ്യ i, j;
  പൂർണ്ണസംഖ്യ size, type;
  പൂർണ്ണസംഖ്യ n, m;
  പൂർണ്ണസംഖ്യ greg;
  പൂർണ്ണസംഖ്യ freg;
  പൂർണ്ണസംഖ്യ fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  വേണ്ടി (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_FLOAT:
	  greg++;
	  cif->bytes += വലുപ്പം (UINT64) - വലുപ്പം (ദശാംശം);
	  എങ്കിൽ (freg >= NFREGARG - 1)
	    തുടരുക;
	  എങ്കിൽ (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  അല്ലെങ്കിൽ
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  എങ്കിൽ (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    തുടരുക;
	  എങ്കിൽ ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  അല്ലെങ്കിൽ
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  നിർത്തുക;
	      
	സ്ഥിരസ്ഥിതി:
	  size = (cif->arg_types)[i]->size;
	  എങ്കിൽ (size < വലുപ്പം (UINT64))
	    cif->bytes += വലുപ്പം (UINT64) - size;
	  n = (size + വലുപ്പം (UINT64) - 1) / വലുപ്പം (UINT64);
	  എങ്കിൽ (greg >= NGREGARG)
	    തുടരുക;
	  അല്ലെങ്കിൽ എങ്കിൽ (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  അല്ലെങ്കിൽ
	    greg += n;
	  വേണ്ടി (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  നിർത്തുക;
	}
    }

  /* Set the return type flag */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_VOID:
    അവസ്ഥ FFI_TYPE_FLOAT:
    അവസ്ഥ FFI_TYPE_DOUBLE:
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      cif->flags = FFI_TYPE_INT;
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
ബാഹ്യ ശൂന്യം ffi_call_SYSV(ശൂന്യം (*)(അക്ഷരം *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  unsigned, unsigned, long long,
			  /*@out@*/ unsigned *, 
			  ശൂന്യം (*fn)(ശൂന്യം));
/*@=declundef@*/
/*@=exportheader@*/

ശൂന്യം ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      ശൂന്യം (*fn)(ശൂന്യം), 
	      /*@out@*/ ശൂന്യം *rvalue, 
	      /*@dependent@*/ ശൂന്യം **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  അല്ലെങ്കിൽ എങ്കിൽ ((rvalue == ശൂന്യ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi) 
    {
    അവസ്ഥ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }

  എങ്കിൽ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

ബാഹ്യ ശൂന്യം ffi_closure_SYSV (ശൂന്യം);
ബാഹ്യ ശൂന്യം __ic_invalidate (ശൂന്യം *വരി);

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  unsigned പൂർണ്ണസംഖ്യ *tramp;

  എങ്കിൽ (cif->abi != FFI_SYSV)
    തിരികെ FFI_BAD_ABI;

  tramp = (unsigned പൂർണ്ണസംഖ്യ *) &closure->tramp[0];
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
#അല്ലെങ്കിൽ
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
  __asm__ അസ്ഥിര ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  തിരികെ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

പൂർണ്ണസംഖ്യ
ffi_closure_helper_SYSV (ffi_closure *closure, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  ശൂന്യം **avalue;
  ffi_type **p_arg;
  പൂർണ്ണസംഖ്യ i, avn;
  പൂർണ്ണസംഖ്യ greg, freg;
  ffi_cif *cif;
  പൂർണ്ണസംഖ്യ fpair = -1;

  cif = closure->cif;
  avalue = alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  എങ്കിൽ (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  അല്ലെങ്കിൽ
    greg = 0;

  freg = 0;
  cif = closure->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  വേണ്ടി (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      ശൂന്യം *p;

      z = (*p_arg)->size;
      എങ്കിൽ (z < വലുപ്പം (UINT32))
	{
	  p = pgr + greg++;

	  തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT8:
	    അവസ്ഥ FFI_TYPE_UINT8:
	    അവസ്ഥ FFI_TYPE_SINT16:
	    അവസ്ഥ FFI_TYPE_UINT16:
	    അവസ്ഥ FFI_TYPE_STRUCT:
#ifdef __LITTLE_ENDIAN__
	      avalue[i] = p;
#അല്ലെങ്കിൽ
	      avalue[i] = ((അക്ഷരം *) p) + വലുപ്പം (UINT32) - z;
#endif
	      നിർത്തുക;

	    സ്ഥിരസ്ഥിതി:
	      FFI_ASSERT(0);
	    }
	}
      അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം (UINT32))
	{
	  എങ്കിൽ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      എങ്കിൽ (freg < NFREGARG - 1)
		{
		  എങ്കിൽ (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  അല്ലെങ്കിൽ
		    {
#ifdef __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#അല്ലെങ്കിൽ
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#endif
		      freg += 2;
		    }
		}
	      അല്ലെങ്കിൽ
#ifdef __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#അല്ലെങ്കിൽ
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	    }
	  അല്ലെങ്കിൽ
#ifdef __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#അല്ലെങ്കിൽ
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	  greg++;
	}
      അല്ലെങ്കിൽ എങ്കിൽ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  എങ്കിൽ (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  അല്ലെങ്കിൽ
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      അല്ലെങ്കിൽ
	{
	  പൂർണ്ണസംഖ്യ n = (z + വലുപ്പം (UINT64) - 1) / വലുപ്പം (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  തിരികെ return_type (cif->rtype);
}

