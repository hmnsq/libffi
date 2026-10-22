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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>

#నిర్వచించు NGREGARG 8
#నిర్వచించు NFREGARG 12

స్థిర పూర్ణాంకం
return_type (ffi_type *arg)
{

  ఒకవేళ (arg->type != FFI_TYPE_STRUCT)
    తిరిగి arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  ఒకవేళ (arg->కొలత <= పరిమాణం (UINT8))
    తిరిగి FFI_TYPE_UINT8;
  లేకపోతే ఒకవేళ (arg->కొలత <= పరిమాణం (UINT16))
    తిరిగి FFI_TYPE_UINT16;
  లేకపోతే ఒకవేళ (arg->కొలత <= పరిమాణం (UINT32))
    తిరిగి FFI_TYPE_UINT32;
  లేకపోతే ఒకవేళ (arg->కొలత <= పరిమాణం (UINT64))
    తిరిగి FFI_TYPE_UINT64;

  తిరిగి FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

శూన్య ffi_prep_వాదనలు(అక్షరం *దొంతర, extended_cif *ecif)
{
  నమోదు గుర్తులేని పూర్ణాంకం i;
  నమోదు గుర్తులేని పూర్ణాంకం avn;
  నమోదు శూన్య **p_argv;
  నమోదు అక్షరం *argp;
  నమోదు ffi_type **p_arg;

  argp = దొంతర;

  ఒకవేళ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(శూన్య **) argp = ecif->rvalue;
      argp += పరిమాణం (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  కొరకు (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      పూర్ణాంకం align;

      z = (*p_arg)->కొలత;
      align = (*p_arg)->alignment;
      ఒకవేళ (z < పరిమాణం (UINT32))
	{
	  ఎంపిక ((*p_arg)->type)
	    {
	    స్థితి FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_STRUCT:
	      జ్ఞాపకం_నకలు (argp, *p_argv, z);
	      ఆపు;

	    అప్రమేయ:
	      FFI_ASSERT(0);
	    }
	  argp += పరిమాణం (UINT64);
	}
      లేకపోతే ఒకవేళ (z == పరిమాణం (UINT32) && align == పరిమాణం (UINT32))
	{
	  ఎంపిక ((*p_arg)->type)
	    {
	    స్థితి FFI_TYPE_INT:
	    స్థితి FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      ఆపు;

	    స్థితి FFI_TYPE_FLOAT:
	    స్థితి FFI_TYPE_సూచిక:
	    స్థితి FFI_TYPE_UINT32:
	    స్థితి FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      ఆపు;

	    అప్రమేయ:
	      FFI_ASSERT(0);
	      ఆపు;
	    }
	  argp += పరిమాణం (UINT64);
	}
      లేకపోతే ఒకవేళ (z == పరిమాణం (UINT64)
	       && align == పరిమాణం (UINT64)
	       && ((పూర్ణాంకం) *p_argv & (పరిమాణం (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += పరిమాణం (UINT64);
	}
      లేకపోతే
	{
	  పూర్ణాంకం n = (z + పరిమాణం (UINT64) - 1) / పరిమాణం (UINT64);

	  జ్ఞాపకం_నకలు (argp, *p_argv, z);
	  argp += n * పరిమాణం (UINT64);
	}
    }

  తిరిగి;
}

/* Perform machine dependent cif processing */
ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif)
{
  పూర్ణాంకం i, j;
  పూర్ణాంకం కొలత, type;
  పూర్ణాంకం n, m;
  పూర్ణాంకం greg;
  పూర్ణాంకం freg;
  పూర్ణాంకం fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  కొరకు (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_FLOAT:
	  greg++;
	  cif->బైట్లు += పరిమాణం (UINT64) - పరిమాణం (దశాంశం);
	  ఒకవేళ (freg >= NFREGARG - 1)
	    కొనసాగు;
	  ఒకవేళ (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  లేకపోతే
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  ఒకవేళ (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    కొనసాగు;
	  ఒకవేళ ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  లేకపోతే
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ఆపు;
	      
	అప్రమేయ:
	  కొలత = (cif->arg_types)[i]->కొలత;
	  ఒకవేళ (కొలత < పరిమాణం (UINT64))
	    cif->బైట్లు += పరిమాణం (UINT64) - కొలత;
	  n = (కొలత + పరిమాణం (UINT64) - 1) / పరిమాణం (UINT64);
	  ఒకవేళ (greg >= NGREGARG)
	    కొనసాగు;
	  లేకపోతే ఒకవేళ (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  లేకపోతే
	    greg += n;
	  కొరకు (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  ఆపు;
	}
    }

  /* Set the return type flag */
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      ఆపు;

    స్థితి FFI_TYPE_VOID:
    స్థితి FFI_TYPE_FLOAT:
    స్థితి FFI_TYPE_DOUBLE:
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      ఆపు;

    అప్రమేయ:
      cif->flags = FFI_TYPE_INT;
      ఆపు;
    }

  తిరిగి FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
బాహ్య శూన్య ffi_call_SYSV(శూన్య (*)(అక్షరం *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  గుర్తులేని, గుర్తులేని, దీర్ఘ దీర్ఘ,
			  /*@out@*/ గుర్తులేని *, 
			  శూన్య (*fn)(శూన్య));
/*@=declundef@*/
/*@=exportheader@*/

శూన్య ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      శూన్య (*fn)(శూన్య), 
	      /*@out@*/ శూన్య *rvalue, 
	      /*@dependent@*/ శూన్య **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  లేకపోతే ఒకవేళ ((rvalue == ఖాళీ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->కొలత);
    }
  లేకపోతే
    ecif.rvalue = rvalue;

  ఎంపిక (cif->abi) 
    {
    స్థితి FFI_SYSV:
      ffi_call_SYSV(ffi_prep_వాదనలు, &ecif, cif->బైట్లు, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      ఆపు;
    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }

  ఒకవేళ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    జ్ఞాపకం_నకలు (rvalue, &trvalue, cif->rtype->కొలత);
}

బాహ్య శూన్య ffi_సంవృతం_SYSV (శూన్య);
బాహ్య శూన్య __ic_invalidate (శూన్య *పంక్తి);

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం *సంవృతం,
		      ffi_cif *cif,
		      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  గుర్తులేని పూర్ణాంకం *tramp;

  ఒకవేళ (cif->abi != FFI_SYSV)
    తిరిగి FFI_BAD_ABI;

  tramp = (గుర్తులేని పూర్ణాంకం *) &సంవృతం->tramp[0];
  /* Since ffi_closure is an aligned object, the ffi trampoline is
     called as an SHcompact code.  Sigh.
     SHcompact part:
     mova @(1,pc),r0; add #1,r0; jmp @r0; nop;
     SHmedia part:
     movi fnaddr >> 16,r1; shori fnaddr,r1; ptabs/l r1,tr0
     movi cxt >> 16,r1; shori cxt,r1; blink tr0,r63  */
#నిర్వచితమైతే __LITTLE_ENDIAN__
  tramp[0] = 0x7001c701;
  tramp[1] = 0x0009402b;
#లేకపోతే
  tramp[0] = 0xc7017001;
  tramp[1] = 0x402b0009;
#అయితే_ముగింపు
  tramp[2] = 0xcc000010 | (((UINT32) ffi_సంవృతం_SYSV) >> 16) << 10;
  tramp[3] = 0xc8000010 | (((UINT32) ffi_సంవృతం_SYSV) & 0xffff) << 10;
  tramp[4] = 0x6bf10600;
  tramp[5] = 0xcc000010 | (((UINT32) codeloc) >> 16) << 10;
  tramp[6] = 0xc8000010 | (((UINT32) codeloc) & 0xffff) << 10;
  tramp[7] = 0x4401fff0;

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  /* Flush the icache.  */
  __asm__ అస్థిర ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  తిరిగి FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

పూర్ణాంకం
ffi_సంవృతం_helper_SYSV (ffi_సంవృతం *సంవృతం, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  శూన్య **avalue;
  ffi_type **p_arg;
  పూర్ణాంకం i, avn;
  పూర్ణాంకం greg, freg;
  ffi_cif *cif;
  పూర్ణాంకం fpair = -1;

  cif = సంవృతం->cif;
  avalue = alloca (cif->nargs * పరిమాణం (శూన్య *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ఒకవేళ (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  లేకపోతే
    greg = 0;

  freg = 0;
  cif = సంవృతం->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  కొరకు (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      శూన్య *p;

      z = (*p_arg)->కొలత;
      ఒకవేళ (z < పరిమాణం (UINT32))
	{
	  p = pgr + greg++;

	  ఎంపిక ((*p_arg)->type)
	    {
	    స్థితి FFI_TYPE_SINT8:
	    స్థితి FFI_TYPE_UINT8:
	    స్థితి FFI_TYPE_SINT16:
	    స్థితి FFI_TYPE_UINT16:
	    స్థితి FFI_TYPE_STRUCT:
#నిర్వచితమైతే __LITTLE_ENDIAN__
	      avalue[i] = p;
#లేకపోతే
	      avalue[i] = ((అక్షరం *) p) + పరిమాణం (UINT32) - z;
#అయితే_ముగింపు
	      ఆపు;

	    అప్రమేయ:
	      FFI_ASSERT(0);
	    }
	}
      లేకపోతే ఒకవేళ (z == పరిమాణం (UINT32))
	{
	  ఒకవేళ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ఒకవేళ (freg < NFREGARG - 1)
		{
		  ఒకవేళ (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  లేకపోతే
		    {
#నిర్వచితమైతే __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#లేకపోతే
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#అయితే_ముగింపు
		      freg += 2;
		    }
		}
	      లేకపోతే
#నిర్వచితమైతే __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#లేకపోతే
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#అయితే_ముగింపు
	    }
	  లేకపోతే
#నిర్వచితమైతే __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#లేకపోతే
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#అయితే_ముగింపు
	  greg++;
	}
      లేకపోతే ఒకవేళ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ఒకవేళ (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  లేకపోతే
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      లేకపోతే
	{
	  పూర్ణాంకం n = (z + పరిమాణం (UINT64) - 1) / పరిమాణం (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (సంవృతం->fun) (cif, rvalue, avalue, సంవృతం->user_దత్తాంశం);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  తిరిగి return_type (cif->rtype);
}

