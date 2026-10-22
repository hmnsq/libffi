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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>

#परिभाषय NGREGARG 8
#परिभाषय NFREGARG 12

स्थिर पूर्णाङ्क
return_type (ffi_type *arg)
{

  यदि (arg->type != FFI_TYPE_STRUCT)
    प्रत्यागम arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  यदि (arg->आकार <= परिमाण (UINT8))
    प्रत्यागम FFI_TYPE_UINT8;
  अन्यथा यदि (arg->आकार <= परिमाण (UINT16))
    प्रत्यागम FFI_TYPE_UINT16;
  अन्यथा यदि (arg->आकार <= परिमाण (UINT32))
    प्रत्यागम FFI_TYPE_UINT32;
  अन्यथा यदि (arg->आकार <= परिमाण (UINT64))
    प्रत्यागम FFI_TYPE_UINT64;

  प्रत्यागम FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

शून्य ffi_prep_तर्काः(वर्ण *राशिः, extended_cif *ecif)
{
  पञ्जिका अचिह्नित पूर्णाङ्क i;
  पञ्जिका अचिह्नित पूर्णाङ्क avn;
  पञ्जिका शून्य **p_argv;
  पञ्जिका वर्ण *argp;
  पञ्जिका ffi_type **p_arg;

  argp = राशिः;

  यदि (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(शून्य **) argp = ecif->rvalue;
      argp += परिमाण (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  प्रति (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      पूर्णाङ्क align;

      z = (*p_arg)->आकार;
      align = (*p_arg)->alignment;
      यदि (z < परिमाण (UINT32))
	{
	  विकल्प ((*p_arg)->type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_STRUCT:
	      स्मृतिं_प्रतिलिख (argp, *p_argv, z);
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT(0);
	    }
	  argp += परिमाण (UINT64);
	}
      अन्यथा यदि (z == परिमाण (UINT32) && align == परिमाण (UINT32))
	{
	  विकल्प ((*p_arg)->type)
	    {
	    पक्ष FFI_TYPE_INT:
	    पक्ष FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      भङ्ग;

	    पक्ष FFI_TYPE_FLOAT:
	    पक्ष FFI_TYPE_POINTER:
	    पक्ष FFI_TYPE_UINT32:
	    पक्ष FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT(0);
	      भङ्ग;
	    }
	  argp += परिमाण (UINT64);
	}
      अन्यथा यदि (z == परिमाण (UINT64)
	       && align == परिमाण (UINT64)
	       && ((पूर्णाङ्क) *p_argv & (परिमाण (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += परिमाण (UINT64);
	}
      अन्यथा
	{
	  पूर्णाङ्क n = (z + परिमाण (UINT64) - 1) / परिमाण (UINT64);

	  स्मृतिं_प्रतिलिख (argp, *p_argv, z);
	  argp += n * परिमाण (UINT64);
	}
    }

  प्रत्यागम;
}

/* Perform machine dependent cif processing */
ffi_स्थितिः ffi_prep_cif_machdep(ffi_cif *cif)
{
  पूर्णाङ्क i, j;
  पूर्णाङ्क आकार, type;
  पूर्णाङ्क n, m;
  पूर्णाङ्क greg;
  पूर्णाङ्क freg;
  पूर्णाङ्क fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  प्रति (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      विकल्प (type)
	{
	पक्ष FFI_TYPE_FLOAT:
	  greg++;
	  cif->अष्टकानि += परिमाण (UINT64) - परिमाण (भिन्न);
	  यदि (freg >= NFREGARG - 1)
	    अनुवर्त;
	  यदि (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  अन्यथा
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  यदि (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    अनुवर्त;
	  यदि ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  अन्यथा
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  भङ्ग;
	      
	स्वभाव:
	  आकार = (cif->arg_types)[i]->आकार;
	  यदि (आकार < परिमाण (UINT64))
	    cif->अष्टकानि += परिमाण (UINT64) - आकार;
	  n = (आकार + परिमाण (UINT64) - 1) / परिमाण (UINT64);
	  यदि (greg >= NGREGARG)
	    अनुवर्त;
	  अन्यथा यदि (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  अन्यथा
	    greg += n;
	  प्रति (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  भङ्ग;
	}
    }

  /* Set the return type flag */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      भङ्ग;

    पक्ष FFI_TYPE_VOID:
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      भङ्ग;

    स्वभाव:
      cif->flags = FFI_TYPE_INT;
      भङ्ग;
    }

  प्रत्यागम FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
बाह्य शून्य ffi_call_SYSV(शून्य (*)(वर्ण *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  अचिह्नित, अचिह्नित, दीर्घ दीर्घ,
			  /*@out@*/ अचिह्नित *, 
			  शून्य (*fn)(शून्य));
/*@=declundef@*/
/*@=exportheader@*/

शून्य ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      शून्य (*fn)(शून्य), 
	      /*@out@*/ शून्य *rvalue, 
	      /*@dependent@*/ शून्य **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  यदि (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  अन्यथा यदि ((rvalue == रिक्त) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->आकार);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi) 
    {
    पक्ष FFI_SYSV:
      ffi_call_SYSV(ffi_prep_तर्काः, &ecif, cif->अष्टकानि, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      भङ्ग;
    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }

  यदि (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    स्मृतिं_प्रतिलिख (rvalue, &trvalue, cif->rtype->आकार);
}

बाह्य शून्य ffi_संवृतिः_SYSV (शून्य);
बाह्य शून्य __ic_invalidate (शून्य *पङ्क्तिः);

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः *संवृतिः,
		      ffi_cif *cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_दत्त,
		      शून्य *codeloc)
{
  अचिह्नित पूर्णाङ्क *tramp;

  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  tramp = (अचिह्नित पूर्णाङ्क *) &संवृतिः->tramp[0];
  /* Since ffi_closure is an aligned object, the ffi trampoline is
     called as an SHcompact code.  Sigh.
     SHcompact part:
     mova @(1,pc),r0; add #1,r0; jmp @r0; nop;
     SHmedia part:
     movi fnaddr >> 16,r1; shori fnaddr,r1; ptabs/l r1,tr0
     movi cxt >> 16,r1; shori cxt,r1; blink tr0,r63  */
#यदि_परिभाषितम् __LITTLE_ENDIAN__
  tramp[0] = 0x7001c701;
  tramp[1] = 0x0009402b;
#अन्यथा
  tramp[0] = 0xc7017001;
  tramp[1] = 0x402b0009;
#यदि_अन्तः
  tramp[2] = 0xcc000010 | (((UINT32) ffi_संवृतिः_SYSV) >> 16) << 10;
  tramp[3] = 0xc8000010 | (((UINT32) ffi_संवृतिः_SYSV) & 0xffff) << 10;
  tramp[4] = 0x6bf10600;
  tramp[5] = 0xcc000010 | (((UINT32) codeloc) >> 16) << 10;
  tramp[6] = 0xc8000010 | (((UINT32) codeloc) & 0xffff) << 10;
  tramp[7] = 0x4401fff0;

  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;

  /* Flush the icache.  */
  __asm__ चञ्चल ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  प्रत्यागम FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

पूर्णाङ्क
ffi_संवृतिः_helper_SYSV (ffi_संवृतिः *संवृतिः, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  शून्य **avalue;
  ffi_type **p_arg;
  पूर्णाङ्क i, avn;
  पूर्णाङ्क greg, freg;
  ffi_cif *cif;
  पूर्णाङ्क fpair = -1;

  cif = संवृतिः->cif;
  avalue = alloca (cif->nargs * परिमाण (शून्य *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  यदि (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  अन्यथा
    greg = 0;

  freg = 0;
  cif = संवृतिः->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  प्रति (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      शून्य *p;

      z = (*p_arg)->आकार;
      यदि (z < परिमाण (UINT32))
	{
	  p = pgr + greg++;

	  विकल्प ((*p_arg)->type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	    पक्ष FFI_TYPE_UINT8:
	    पक्ष FFI_TYPE_SINT16:
	    पक्ष FFI_TYPE_UINT16:
	    पक्ष FFI_TYPE_STRUCT:
#यदि_परिभाषितम् __LITTLE_ENDIAN__
	      avalue[i] = p;
#अन्यथा
	      avalue[i] = ((वर्ण *) p) + परिमाण (UINT32) - z;
#यदि_अन्तः
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT(0);
	    }
	}
      अन्यथा यदि (z == परिमाण (UINT32))
	{
	  यदि ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      यदि (freg < NFREGARG - 1)
		{
		  यदि (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  अन्यथा
		    {
#यदि_परिभाषितम् __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#अन्यथा
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#यदि_अन्तः
		      freg += 2;
		    }
		}
	      अन्यथा
#यदि_परिभाषितम् __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#अन्यथा
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#यदि_अन्तः
	    }
	  अन्यथा
#यदि_परिभाषितम् __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#अन्यथा
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#यदि_अन्तः
	  greg++;
	}
      अन्यथा यदि ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  यदि (freg + 1 >= NFREGARG)
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
	  पूर्णाङ्क n = (z + परिमाण (UINT64) - 1) / परिमाण (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (संवृतिः->fun) (cif, rvalue, avalue, संवृतिः->user_दत्त);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  प्रत्यागम return_type (cif->rtype);
}

