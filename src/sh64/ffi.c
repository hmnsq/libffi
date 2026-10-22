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

#تعریف_کرو NGREGARG 8
#تعریف_کرو NFREGARG 12

ساکن صحیح
return_type (ffi_type *arg)
{

  اگر (arg->type != FFI_TYPE_STRUCT)
    واپس arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  اگر (arg->سائز <= حجم (UINT8))
    واپس FFI_TYPE_UINT8;
  ورنہ اگر (arg->سائز <= حجم (UINT16))
    واپس FFI_TYPE_UINT16;
  ورنہ اگر (arg->سائز <= حجم (UINT32))
    واپس FFI_TYPE_UINT32;
  ورنہ اگر (arg->سائز <= حجم (UINT64))
    واپس FFI_TYPE_UINT64;

  واپس FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

خالی ffi_prep_دلائل(حرف *ڈھیر, extended_cif *ecif)
{
  اندراج غیرعلامتی صحیح i;
  اندراج غیرعلامتی صحیح avn;
  اندراج خالی **p_argv;
  اندراج حرف *argp;
  اندراج ffi_type **p_arg;

  argp = ڈھیر;

  اگر (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(خالی **) argp = ecif->rvalue;
      argp += حجم (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  برائے (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      صحیح align;

      z = (*p_arg)->سائز;
      align = (*p_arg)->alignment;
      اگر (z < حجم (UINT32))
	{
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_STRUCT:
	      حافظہ_نقل (argp, *p_argv, z);
	      توڑو;

	    طےشدہ:
	      FFI_ASSERT(0);
	    }
	  argp += حجم (UINT64);
	}
      ورنہ اگر (z == حجم (UINT32) && align == حجم (UINT32))
	{
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_INT:
	    حالت FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      توڑو;

	    حالت FFI_TYPE_FLOAT:
	    حالت FFI_TYPE_اشارہ_گر:
	    حالت FFI_TYPE_UINT32:
	    حالت FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      توڑو;

	    طےشدہ:
	      FFI_ASSERT(0);
	      توڑو;
	    }
	  argp += حجم (UINT64);
	}
      ورنہ اگر (z == حجم (UINT64)
	       && align == حجم (UINT64)
	       && ((صحیح) *p_argv & (حجم (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += حجم (UINT64);
	}
      ورنہ
	{
	  صحیح n = (z + حجم (UINT64) - 1) / حجم (UINT64);

	  حافظہ_نقل (argp, *p_argv, z);
	  argp += n * حجم (UINT64);
	}
    }

  واپس;
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  صحیح i, j;
  صحیح سائز, type;
  صحیح n, m;
  صحیح greg;
  صحیح freg;
  صحیح fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  برائے (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      انتخاب (type)
	{
	حالت FFI_TYPE_FLOAT:
	  greg++;
	  cif->بائٹیں += حجم (UINT64) - حجم (اعشاری);
	  اگر (freg >= NFREGARG - 1)
	    جاری;
	  اگر (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  ورنہ
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	  اگر (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    جاری;
	  اگر ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  ورنہ
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  توڑو;
	      
	طےشدہ:
	  سائز = (cif->arg_types)[i]->سائز;
	  اگر (سائز < حجم (UINT64))
	    cif->بائٹیں += حجم (UINT64) - سائز;
	  n = (سائز + حجم (UINT64) - 1) / حجم (UINT64);
	  اگر (greg >= NGREGARG)
	    جاری;
	  ورنہ اگر (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  ورنہ
	    greg += n;
	  برائے (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  توڑو;
	}
    }

  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      توڑو;

    حالت FFI_TYPE_VOID:
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      توڑو;

    طےشدہ:
      cif->flags = FFI_TYPE_INT;
      توڑو;
    }

  واپس FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
خارجی خالی ffi_call_SYSV(خالی (*)(حرف *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  غیرعلامتی, غیرعلامتی, طویل طویل,
			  /*@out@*/ غیرعلامتی *, 
			  خالی (*fn)(خالی));
/*@=declundef@*/
/*@=exportheader@*/

خالی ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      خالی (*fn)(خالی), 
	      /*@out@*/ خالی *rvalue, 
	      /*@dependent@*/ خالی **avalue)
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
  ورنہ اگر ((rvalue == تہی) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->سائز);
    }
  ورنہ
    ecif.rvalue = rvalue;

  انتخاب (cif->abi) 
    {
    حالت FFI_SYSV:
      ffi_call_SYSV(ffi_prep_دلائل, &ecif, cif->بائٹیں, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      توڑو;
    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }

  اگر (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    حافظہ_نقل (rvalue, &trvalue, cif->rtype->سائز);
}

خارجی خالی ffi_احاطہ_SYSV (خالی);
خارجی خالی __ic_invalidate (خالی *سطر);

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ *احاطہ,
		      ffi_cif *cif,
		      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  غیرعلامتی صحیح *tramp;

  اگر (cif->abi != FFI_SYSV)
    واپس FFI_BAD_ABI;

  tramp = (غیرعلامتی صحیح *) &احاطہ->tramp[0];
  /* Since ffi_closure is an aligned object, the ffi trampoline is
     called as an SHcompact code.  Sigh.
     SHcompact part:
     mova @(1,pc),r0; add #1,r0; jmp @r0; nop;
     SHmedia part:
     movi fnaddr >> 16,r1; shori fnaddr,r1; ptabs/l r1,tr0
     movi cxt >> 16,r1; shori cxt,r1; blink tr0,r63  */
#اگر_تعریف_شدہ __LITTLE_ENDIAN__
  tramp[0] = 0x7001c701;
  tramp[1] = 0x0009402b;
#ورنہ
  tramp[0] = 0xc7017001;
  tramp[1] = 0x402b0009;
#اگر_ختم
  tramp[2] = 0xcc000010 | (((UINT32) ffi_احاطہ_SYSV) >> 16) << 10;
  tramp[3] = 0xc8000010 | (((UINT32) ffi_احاطہ_SYSV) & 0xffff) << 10;
  tramp[4] = 0x6bf10600;
  tramp[5] = 0xcc000010 | (((UINT32) codeloc) >> 16) << 10;
  tramp[6] = 0xc8000010 | (((UINT32) codeloc) & 0xffff) << 10;
  tramp[7] = 0x4401fff0;

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  /* Flush the icache.  */
  __asm__ متلون ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  واپس FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

صحیح
ffi_احاطہ_helper_SYSV (ffi_احاطہ *احاطہ, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  خالی **avalue;
  ffi_type **p_arg;
  صحیح i, avn;
  صحیح greg, freg;
  ffi_cif *cif;
  صحیح fpair = -1;

  cif = احاطہ->cif;
  avalue = alloca (cif->nargs * حجم (خالی *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  اگر (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  ورنہ
    greg = 0;

  freg = 0;
  cif = احاطہ->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  برائے (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      خالی *p;

      z = (*p_arg)->سائز;
      اگر (z < حجم (UINT32))
	{
	  p = pgr + greg++;

	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	    حالت FFI_TYPE_UINT8:
	    حالت FFI_TYPE_SINT16:
	    حالت FFI_TYPE_UINT16:
	    حالت FFI_TYPE_STRUCT:
#اگر_تعریف_شدہ __LITTLE_ENDIAN__
	      avalue[i] = p;
#ورنہ
	      avalue[i] = ((حرف *) p) + حجم (UINT32) - z;
#اگر_ختم
	      توڑو;

	    طےشدہ:
	      FFI_ASSERT(0);
	    }
	}
      ورنہ اگر (z == حجم (UINT32))
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
		  ورنہ
		    {
#اگر_تعریف_شدہ __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#ورنہ
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#اگر_ختم
		      freg += 2;
		    }
		}
	      ورنہ
#اگر_تعریف_شدہ __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#ورنہ
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#اگر_ختم
	    }
	  ورنہ
#اگر_تعریف_شدہ __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#ورنہ
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#اگر_ختم
	  greg++;
	}
      ورنہ اگر ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  اگر (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  ورنہ
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      ورنہ
	{
	  صحیح n = (z + حجم (UINT64) - 1) / حجم (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (احاطہ->fun) (cif, rvalue, avalue, احاطہ->user_ڈیٹا);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  واپس return_type (cif->rtype);
}

