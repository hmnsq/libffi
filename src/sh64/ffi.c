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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>

#घोषणा NGREGARG 8
#घोषणा NFREGARG 12

अटल पूर्णांक
return_type (ffi_type *arg)
{

  अगर (arg->type != FFI_TYPE_STRUCT)
    वापस arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  अगर (arg->आकार <= माप (UINT8))
    वापस FFI_TYPE_UINT8;
  अन्यथा अगर (arg->आकार <= माप (UINT16))
    वापस FFI_TYPE_UINT16;
  अन्यथा अगर (arg->आकार <= माप (UINT32))
    वापस FFI_TYPE_UINT32;
  अन्यथा अगर (arg->आकार <= माप (UINT64))
    वापस FFI_TYPE_UINT64;

  वापस FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

व्योम ffi_prep_args(अक्षर *ढेर, extended_cif *ecif)
{
  रेजिस्टर अचिन्हित पूर्णांक i;
  रेजिस्टर अचिन्हित पूर्णांक avn;
  रेजिस्टर व्योम **p_argv;
  रेजिस्टर अक्षर *argp;
  रेजिस्टर ffi_type **p_arg;

  argp = ढेर;

  अगर (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(व्योम **) argp = ecif->rvalue;
      argp += माप (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  क्रम (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      माप_प्रकार z;
      पूर्णांक align;

      z = (*p_arg)->आकार;
      align = (*p_arg)->alignment;
      अगर (z < माप (UINT32))
	{
	  चयन ((*p_arg)->type)
	    {
	    हाल FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_STRUCT:
	      स_नकल (argp, *p_argv, z);
	      अवरोध;

	    शेष:
	      FFI_ASSERT(0);
	    }
	  argp += माप (UINT64);
	}
      अन्यथा अगर (z == माप (UINT32) && align == माप (UINT32))
	{
	  चयन ((*p_arg)->type)
	    {
	    हाल FFI_TYPE_INT:
	    हाल FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      अवरोध;

	    हाल FFI_TYPE_FLOAT:
	    हाल FFI_TYPE_POINTER:
	    हाल FFI_TYPE_UINT32:
	    हाल FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      अवरोध;

	    शेष:
	      FFI_ASSERT(0);
	      अवरोध;
	    }
	  argp += माप (UINT64);
	}
      अन्यथा अगर (z == माप (UINT64)
	       && align == माप (UINT64)
	       && ((पूर्णांक) *p_argv & (माप (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += माप (UINT64);
	}
      अन्यथा
	{
	  पूर्णांक n = (z + माप (UINT64) - 1) / माप (UINT64);

	  स_नकल (argp, *p_argv, z);
	  argp += n * माप (UINT64);
	}
    }

  वापस;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  पूर्णांक i, j;
  पूर्णांक आकार, type;
  पूर्णांक n, m;
  पूर्णांक greg;
  पूर्णांक freg;
  पूर्णांक fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  क्रम (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      चयन (type)
	{
	हाल FFI_TYPE_FLOAT:
	  greg++;
	  cif->bytes += माप (UINT64) - माप (भग्न);
	  अगर (freg >= NFREGARG - 1)
	    जारी;
	  अगर (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  अन्यथा
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  अवरोध;

	हाल FFI_TYPE_DOUBLE:
	  अगर (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    जारी;
	  अगर ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  अन्यथा
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  अवरोध;
	      
	शेष:
	  आकार = (cif->arg_types)[i]->आकार;
	  अगर (आकार < माप (UINT64))
	    cif->bytes += माप (UINT64) - आकार;
	  n = (आकार + माप (UINT64) - 1) / माप (UINT64);
	  अगर (greg >= NGREGARG)
	    जारी;
	  अन्यथा अगर (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  अन्यथा
	    greg += n;
	  क्रम (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  अवरोध;
	}
    }

  /* Set the return type flag */
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      अवरोध;

    हाल FFI_TYPE_VOID:
    हाल FFI_TYPE_FLOAT:
    हाल FFI_TYPE_DOUBLE:
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      अवरोध;

    शेष:
      cif->flags = FFI_TYPE_INT;
      अवरोध;
    }

  वापस FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
बाह्य व्योम ffi_call_SYSV(व्योम (*)(अक्षर *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  अचिन्हित, अचिन्हित, दीर्घ दीर्घ,
			  /*@out@*/ अचिन्हित *, 
			  व्योम (*fn)(व्योम));
/*@=declundef@*/
/*@=exportheader@*/

व्योम ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      व्योम (*fn)(व्योम), 
	      /*@out@*/ व्योम *rvalue, 
	      /*@dependent@*/ व्योम **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  अगर (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  अन्यथा अगर ((rvalue == शून्य) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->आकार);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  चयन (cif->abi) 
    {
    हाल FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      अवरोध;
    शेष:
      FFI_ASSERT(0);
      अवरोध;
    }

  अगर (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    स_नकल (rvalue, &trvalue, cif->rtype->आकार);
}

बाह्य व्योम ffi_closure_SYSV (व्योम);
बाह्य व्योम __ic_invalidate (व्योम *पंक्ति_2);

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  अचिन्हित पूर्णांक *tramp;

  अगर (cif->abi != FFI_SYSV)
    वापस FFI_BAD_ABI;

  tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
  /* Since ffi_closure is an aligned object, the ffi trampoline is
     called as an SHcompact code.  Sigh.
     SHcompact part:
     mova @(1,pc),r0; add #1,r0; jmp @r0; nop;
     SHmedia part:
     movi fnaddr >> 16,r1; shori fnaddr,r1; ptabs/l r1,tr0
     movi cxt >> 16,r1; shori cxt,r1; blink tr0,r63  */
#अगर_घोषित __LITTLE_ENDIAN__
  tramp[0] = 0x7001c701;
  tramp[1] = 0x0009402b;
#अन्यथा
  tramp[0] = 0xc7017001;
  tramp[1] = 0x402b0009;
#पूर्ण_अगर
  tramp[2] = 0xcc000010 | (((UINT32) ffi_closure_SYSV) >> 16) << 10;
  tramp[3] = 0xc8000010 | (((UINT32) ffi_closure_SYSV) & 0xffff) << 10;
  tramp[4] = 0x6bf10600;
  tramp[5] = 0xcc000010 | (((UINT32) codeloc) >> 16) << 10;
  tramp[6] = 0xc8000010 | (((UINT32) codeloc) & 0xffff) << 10;
  tramp[7] = 0x4401fff0;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  /* Flush the icache.  */
  __asm__ अस्थिर ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  वापस FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

पूर्णांक
ffi_closure_helper_SYSV (ffi_closure *closure, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  व्योम **avalue;
  ffi_type **p_arg;
  पूर्णांक i, avn;
  पूर्णांक greg, freg;
  ffi_cif *cif;
  पूर्णांक fpair = -1;

  cif = closure->cif;
  avalue = alloca (cif->nargs * माप (व्योम *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  अगर (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  अन्यथा
    greg = 0;

  freg = 0;
  cif = closure->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  क्रम (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      माप_प्रकार z;
      व्योम *p;

      z = (*p_arg)->आकार;
      अगर (z < माप (UINT32))
	{
	  p = pgr + greg++;

	  चयन ((*p_arg)->type)
	    {
	    हाल FFI_TYPE_SINT8:
	    हाल FFI_TYPE_UINT8:
	    हाल FFI_TYPE_SINT16:
	    हाल FFI_TYPE_UINT16:
	    हाल FFI_TYPE_STRUCT:
#अगर_घोषित __LITTLE_ENDIAN__
	      avalue[i] = p;
#अन्यथा
	      avalue[i] = ((अक्षर *) p) + माप (UINT32) - z;
#पूर्ण_अगर
	      अवरोध;

	    शेष:
	      FFI_ASSERT(0);
	    }
	}
      अन्यथा अगर (z == माप (UINT32))
	{
	  अगर ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      अगर (freg < NFREGARG - 1)
		{
		  अगर (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  अन्यथा
		    {
#अगर_घोषित __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#अन्यथा
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#पूर्ण_अगर
		      freg += 2;
		    }
		}
	      अन्यथा
#अगर_घोषित __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#अन्यथा
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#पूर्ण_अगर
	    }
	  अन्यथा
#अगर_घोषित __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#अन्यथा
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#पूर्ण_अगर
	  greg++;
	}
      अन्यथा अगर ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  अगर (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  अन्यथा
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      अन्यथा
	{
	  पूर्णांक n = (z + माप (UINT64) - 1) / माप (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_तथ्य);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  वापस return_type (cif->rtype);
}

