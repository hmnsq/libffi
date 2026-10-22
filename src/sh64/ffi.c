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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>

#تعریف_کرو NGREGARG 8
#تعریف_کرو NFREGARG 12

ساكن صحيح
return_type (ffi_type *arg)
{

  إذا (arg->type != FFI_TYPE_STRUCT)
    أرجع arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  إذا (arg->ناپ <= حجم (UINT8))
    أرجع FFI_TYPE_UINT8;
  وإلا إذا (arg->ناپ <= حجم (UINT16))
    أرجع FFI_TYPE_UINT16;
  وإلا إذا (arg->ناپ <= حجم (UINT32))
    أرجع FFI_TYPE_UINT32;
  وإلا إذا (arg->ناپ <= حجم (UINT64))
    أرجع FFI_TYPE_UINT64;

  أرجع FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

فراغ ffi_prep_دلیلاں(حرف *ڈھیر, extended_cif *ecif)
{
  سجل بنانشانی صحيح i;
  سجل بنانشانی صحيح avn;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;

  argp = ڈھیر;

  إذا (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(فراغ **) argp = ecif->rvalue;
      argp += حجم (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  لكل (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      صحيح align;

      z = (*p_arg)->ناپ;
      align = (*p_arg)->alignment;
      إذا (z < حجم (UINT32))
	{
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_STRUCT:
	      حافظہ_نقل (argp, *p_argv, z);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	  argp += حجم (UINT64);
	}
      وإلا إذا (z == حجم (UINT32) && align == حجم (UINT32))
	{
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_INT:
	    حالة FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      اكسر;

	    حالة FFI_TYPE_FLOAT:
	    حالة FFI_TYPE_اشارہ_گر:
	    حالة FFI_TYPE_UINT32:
	    حالة FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	      اكسر;
	    }
	  argp += حجم (UINT64);
	}
      وإلا إذا (z == حجم (UINT64)
	       && align == حجم (UINT64)
	       && ((صحيح) *p_argv & (حجم (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += حجم (UINT64);
	}
      وإلا
	{
	  صحيح n = (z + حجم (UINT64) - 1) / حجم (UINT64);

	  حافظہ_نقل (argp, *p_argv, z);
	  argp += n * حجم (UINT64);
	}
    }

  أرجع;
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  صحيح i, j;
  صحيح ناپ, type;
  صحيح n, m;
  صحيح greg;
  صحيح freg;
  صحيح fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  لكل (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      بدل (type)
	{
	حالة FFI_TYPE_FLOAT:
	  greg++;
	  cif->بائٹاں += حجم (UINT64) - حجم (عائم);
	  إذا (freg >= NFREGARG - 1)
	    استمر;
	  إذا (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  وإلا
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  إذا (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    استمر;
	  إذا ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  وإلا
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  اكسر;
	      
	افتراضي:
	  ناپ = (cif->arg_types)[i]->ناپ;
	  إذا (ناپ < حجم (UINT64))
	    cif->بائٹاں += حجم (UINT64) - ناپ;
	  n = (ناپ + حجم (UINT64) - 1) / حجم (UINT64);
	  إذا (greg >= NGREGARG)
	    استمر;
	  وإلا إذا (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  وإلا
	    greg += n;
	  لكل (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  اكسر;
	}
    }

  /* Set the return type flag */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      اكسر;

    حالة FFI_TYPE_VOID:
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      اكسر;

    افتراضي:
      cif->flags = FFI_TYPE_INT;
      اكسر;
    }

  أرجع FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
خارجي فراغ ffi_call_SYSV(فراغ (*)(حرف *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  بنانشانی, بنانشانی, لما لما,
			  /*@out@*/ بنانشانی *, 
			  فراغ (*fn)(فراغ));
/*@=declundef@*/
/*@=exportheader@*/

فراغ ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      فراغ (*fn)(فراغ), 
	      /*@out@*/ فراغ *rvalue, 
	      /*@dependent@*/ فراغ **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  إذا (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  وإلا إذا ((rvalue == عدم) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ناپ);
    }
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi) 
    {
    حالة FFI_SYSV:
      ffi_call_SYSV(ffi_prep_دلیلاں, &ecif, cif->بائٹاں, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }

  إذا (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    حافظہ_نقل (rvalue, &trvalue, cif->rtype->ناپ);
}

خارجي فراغ ffi_گھیرا_SYSV (فراغ);
خارجي فراغ __ic_invalidate (فراغ *سطر);

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا *گھیرا,
		      ffi_cif *cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_انکڑے,
		      فراغ *codeloc)
{
  بنانشانی صحيح *tramp;

  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  tramp = (بنانشانی صحيح *) &گھیرا->tramp[0];
  /* Since ffi_closure is an aligned object, the ffi trampoline is
     called as an SHcompact code.  Sigh.
     SHcompact part:
     mova @(1,pc),r0; add #1,r0; jmp @r0; nop;
     SHmedia part:
     movi fnaddr >> 16,r1; shori fnaddr,r1; ptabs/l r1,tr0
     movi cxt >> 16,r1; shori cxt,r1; blink tr0,r63  */
#جے_تعریف_شدہ __LITTLE_ENDIAN__
  tramp[0] = 0x7001c701;
  tramp[1] = 0x0009402b;
#وإلا
  tramp[0] = 0xc7017001;
  tramp[1] = 0x402b0009;
#جے_انت
  tramp[2] = 0xcc000010 | (((UINT32) ffi_گھیرا_SYSV) >> 16) << 10;
  tramp[3] = 0xc8000010 | (((UINT32) ffi_گھیرا_SYSV) & 0xffff) << 10;
  tramp[4] = 0x6bf10600;
  tramp[5] = 0xcc000010 | (((UINT32) codeloc) >> 16) << 10;
  tramp[6] = 0xc8000010 | (((UINT32) codeloc) & 0xffff) << 10;
  tramp[7] = 0x4401fff0;

  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;

  /* Flush the icache.  */
  __asm__ متطاير ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  أرجع FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

صحيح
ffi_گھیرا_helper_SYSV (ffi_گھیرا *گھیرا, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  فراغ **avalue;
  ffi_type **p_arg;
  صحيح i, avn;
  صحيح greg, freg;
  ffi_cif *cif;
  صحيح fpair = -1;

  cif = گھیرا->cif;
  avalue = alloca (cif->nargs * حجم (فراغ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  إذا (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  وإلا
    greg = 0;

  freg = 0;
  cif = گھیرا->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  لكل (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      فراغ *p;

      z = (*p_arg)->ناپ;
      إذا (z < حجم (UINT32))
	{
	  p = pgr + greg++;

	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	    حالة FFI_TYPE_UINT8:
	    حالة FFI_TYPE_SINT16:
	    حالة FFI_TYPE_UINT16:
	    حالة FFI_TYPE_STRUCT:
#جے_تعریف_شدہ __LITTLE_ENDIAN__
	      avalue[i] = p;
#وإلا
	      avalue[i] = ((حرف *) p) + حجم (UINT32) - z;
#جے_انت
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	}
      وإلا إذا (z == حجم (UINT32))
	{
	  إذا ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      إذا (freg < NFREGARG - 1)
		{
		  إذا (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  وإلا
		    {
#جے_تعریف_شدہ __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#وإلا
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#جے_انت
		      freg += 2;
		    }
		}
	      وإلا
#جے_تعریف_شدہ __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#وإلا
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#جے_انت
	    }
	  وإلا
#جے_تعریف_شدہ __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#وإلا
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#جے_انت
	  greg++;
	}
      وإلا إذا ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  إذا (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  وإلا
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      وإلا
	{
	  صحيح n = (z + حجم (UINT64) - 1) / حجم (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (گھیرا->fun) (cif, rvalue, avalue, گھیرا->user_انکڑے);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  أرجع return_type (cif->rtype);
}

