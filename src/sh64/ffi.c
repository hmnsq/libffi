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

#define NGREGARG 8
#define NFREGARG 12

ساكن صحيح
return_type (ffi_type *arg)
{

  ܐܢ (arg->type != FFI_TYPE_STRUCT)
    ܦܢܐ arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  ܐܢ (arg->size <= حجم (UINT8))
    ܦܢܐ FFI_TYPE_UINT8;
  ܐܚܪܢܐ ܐܢ (arg->size <= حجم (UINT16))
    ܦܢܐ FFI_TYPE_UINT16;
  ܐܚܪܢܐ ܐܢ (arg->size <= حجم (UINT32))
    ܦܢܐ FFI_TYPE_UINT32;
  ܐܚܪܢܐ ܐܢ (arg->size <= حجم (UINT64))
    ܦܢܐ FFI_TYPE_UINT64;

  ܦܢܐ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

فراغ ffi_prep_args(حرف *stack, extended_cif *ecif)
{
  سجل unsigned صحيح i;
  سجل unsigned صحيح avn;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;

  argp = stack;

  ܐܢ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
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

      z = (*p_arg)->size;
      align = (*p_arg)->alignment;
      ܐܢ (z < حجم (UINT32))
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
	      memcpy (argp, *p_argv, z);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	  argp += حجم (UINT64);
	}
      ܐܚܪܢܐ ܐܢ (z == حجم (UINT32) && align == حجم (UINT32))
	{
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_INT:
	    حالة FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      اكسر;

	    حالة FFI_TYPE_FLOAT:
	    حالة FFI_TYPE_POINTER:
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
      ܐܚܪܢܐ ܐܢ (z == حجم (UINT64)
	       && align == حجم (UINT64)
	       && ((صحيح) *p_argv & (حجم (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += حجم (UINT64);
	}
      ܐܚܪܢܐ
	{
	  صحيح n = (z + حجم (UINT64) - 1) / حجم (UINT64);

	  memcpy (argp, *p_argv, z);
	  argp += n * حجم (UINT64);
	}
    }

  ܦܢܐ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  صحيح i, j;
  صحيح size, type;
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
	  cif->bytes += حجم (UINT64) - حجم (عائم);
	  ܐܢ (freg >= NFREGARG - 1)
	    استمر;
	  ܐܢ (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  ܐܚܪܢܐ
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  ܐܢ (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    استمر;
	  ܐܢ ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  ܐܚܪܢܐ
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  اكسر;
	      
	افتراضي:
	  size = (cif->arg_types)[i]->size;
	  ܐܢ (size < حجم (UINT64))
	    cif->bytes += حجم (UINT64) - size;
	  n = (size + حجم (UINT64) - 1) / حجم (UINT64);
	  ܐܢ (greg >= NGREGARG)
	    استمر;
	  ܐܚܪܢܐ ܐܢ (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  ܐܚܪܢܐ
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

  ܦܢܐ FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
خارجي فراغ ffi_call_SYSV(فراغ (*)(حرف *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  unsigned, unsigned, long long,
			  /*@out@*/ unsigned *, 
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

  ܐܢ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  ܐܚܪܢܐ ܐܢ ((rvalue == عدم) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ܐܚܪܢܐ
    ecif.rvalue = rvalue;

  بدل (cif->abi) 
    {
    حالة FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }

  ܐܢ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

خارجي فراغ ffi_closure_SYSV (فراغ);
خارجي فراغ __ic_invalidate (فراغ *line);

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_data,
		      فراغ *codeloc)
{
  unsigned صحيح *tramp;

  ܐܢ (cif->abi != FFI_SYSV)
    ܦܢܐ FFI_BAD_ABI;

  tramp = (unsigned صحيح *) &closure->tramp[0];
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
#ܐܚܪܢܐ
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
  __asm__ متطاير ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  ܦܢܐ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

صحيح
ffi_closure_helper_SYSV (ffi_closure *closure, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  فراغ **avalue;
  ffi_type **p_arg;
  صحيح i, avn;
  صحيح greg, freg;
  ffi_cif *cif;
  صحيح fpair = -1;

  cif = closure->cif;
  avalue = alloca (cif->nargs * حجم (فراغ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ܐܢ (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  ܐܚܪܢܐ
    greg = 0;

  freg = 0;
  cif = closure->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  لكل (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      فراغ *p;

      z = (*p_arg)->size;
      ܐܢ (z < حجم (UINT32))
	{
	  p = pgr + greg++;

	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	    حالة FFI_TYPE_UINT8:
	    حالة FFI_TYPE_SINT16:
	    حالة FFI_TYPE_UINT16:
	    حالة FFI_TYPE_STRUCT:
#ifdef __LITTLE_ENDIAN__
	      avalue[i] = p;
#ܐܚܪܢܐ
	      avalue[i] = ((حرف *) p) + حجم (UINT32) - z;
#endif
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	}
      ܐܚܪܢܐ ܐܢ (z == حجم (UINT32))
	{
	  ܐܢ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ܐܢ (freg < NFREGARG - 1)
		{
		  ܐܢ (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  ܐܚܪܢܐ
		    {
#ifdef __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#ܐܚܪܢܐ
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#endif
		      freg += 2;
		    }
		}
	      ܐܚܪܢܐ
#ifdef __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#ܐܚܪܢܐ
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	    }
	  ܐܚܪܢܐ
#ifdef __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#ܐܚܪܢܐ
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	  greg++;
	}
      ܐܚܪܢܐ ܐܢ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ܐܢ (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  ܐܚܪܢܐ
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      ܐܚܪܢܐ
	{
	  صحيح n = (z + حجم (UINT64) - 1) / حجم (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ܦܢܐ return_type (cif->rtype);
}

