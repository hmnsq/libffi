/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2002-2008, 2012 Kaz Kojima
           Copyright (c) 2008 Red Hat, Inc.
   
   SuperH Foreign Function Interface 

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

#परिभाषय NGREGARG 4
#यदि defined(__SH4__)
#परिभाषय NFREGARG 8
#यदि_अन्तः

#यदि defined(__HITACHI__)
#परिभाषय STRUCT_मानम्_स्थानाङ्कः_WITH_ARG 1
#अन्यथा
#परिभाषय STRUCT_मानम्_स्थानाङ्कः_WITH_ARG 0
#यदि_अन्तः

/* If the structure has essentially an unique element, return its type.  */
स्थिर पूर्णाङ्क
simple_type (ffi_type *arg)
{
  यदि (arg->type != FFI_TYPE_STRUCT)
    प्रत्यागम arg->type;
  अन्यथा यदि (arg->elements[1])
    प्रत्यागम FFI_TYPE_STRUCT;

  प्रत्यागम simple_type (arg->elements[0]);
}

स्थिर पूर्णाङ्क
return_type (ffi_type *arg)
{
  अचिह्नित ह्रस्व type;

  यदि (arg->type != FFI_TYPE_STRUCT)
    प्रत्यागम arg->type;

  type = simple_type (arg->elements[0]);
  यदि (! arg->elements[1])
    {
      विकल्प (type)
	{
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
	  प्रत्यागम FFI_TYPE_INT;

	स्वभाव:
	  प्रत्यागम type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  यदि (arg->आकार <= 2 * परिमाण (पूर्णाङ्क))
    {
      पूर्णाङ्क i = 0;
      ffi_type *e;

      यावत् ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  विकल्प (type)
	    {
	    पक्ष FFI_TYPE_SINT32:
	    पक्ष FFI_TYPE_UINT32:
	    पक्ष FFI_TYPE_INT:
	    पक्ष FFI_TYPE_FLOAT:
	      प्रत्यागम FFI_TYPE_UINT64;

	    स्वभाव:
	      भङ्ग;
	    }
	}
    }

  प्रत्यागम FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

शून्य ffi_prep_तर्काः(वर्ण *राशिः, extended_cif *ecif)
{
  पञ्जिका अचिह्नित पूर्णाङ्क i;
  पञ्जिका पूर्णाङ्क tmp;
  पञ्जिका अचिह्नित पूर्णाङ्क avn;
  पञ्जिका शून्य **p_argv;
  पञ्जिका वर्ण *argp;
  पञ्जिका ffi_type **p_arg;
  पूर्णाङ्क greg, ireg;
#यदि defined(__SH4__)
  पूर्णाङ्क freg = 0;
#यदि_अन्तः

  tmp = 0;
  argp = राशिः;

  यदि (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(शून्य **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_मानम्_स्थानाङ्कः_WITH_ARG ? 1 : 0;
    }
  अन्यथा
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  प्रति (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->आकार;
      यदि (z < परिमाण(पूर्णाङ्क))
	{
	  यदि (greg++ >= NGREGARG)
	    अनुवर्त;

	  z = परिमाण(पूर्णाङ्क);
	  विकल्प ((*p_arg)->type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	      *(चिह्नित पूर्णाङ्क *) argp = (चिह्नित पूर्णाङ्क)*(SINT8 *)(* p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_UINT8:
	      *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT8 *)(* p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_SINT16:
	      *(चिह्नित पूर्णाङ्क *) argp = (चिह्नित पूर्णाङ्क)*(SINT16 *)(* p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_UINT16:
	      *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT16 *)(* p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_STRUCT:
	      *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT32 *)(* p_argv);
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      अन्यथा यदि (z == परिमाण(पूर्णाङ्क))
	{
#यदि defined(__SH4__)
	  यदि ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      यदि (freg++ >= NFREGARG)
		अनुवर्त;
	    }
	  अन्यथा
#यदि_अन्तः
	    {
	      यदि (greg++ >= NGREGARG)
		अनुवर्त;
	    }
	  *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#यदि defined(__SH4__)
      अन्यथा यदि ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  यदि (freg + 1 >= NFREGARG)
	    अनुवर्त;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  स्मृतिं_प्रतिलिख (argp, *p_argv, z);
	  argp += z;
	}
#यदि_अन्तः
      अन्यथा
	{
	  पूर्णाङ्क n = (z + परिमाण (पूर्णाङ्क) - 1) / परिमाण (पूर्णाङ्क);
#यदि defined(__SH4__)
	  यदि (greg + n - 1 >= NGREGARG)
	    अनुवर्त;
#अन्यथा
	  यदि (greg >= NGREGARG)
	    अनुवर्त;
#यदि_अन्तः
	  greg += n;
	  स्मृतिं_प्रतिलिख (argp, *p_argv, z);
	  argp += n * परिमाण (पूर्णाङ्क);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#यदि defined(__SH4__)
  freg = 0;
#यदि_अन्तः
  p_argv = ecif->avalue;

  प्रति (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->आकार;
      यदि (z < परिमाण(पूर्णाङ्क))
	{
	  यदि (greg++ < NGREGARG)
	    अनुवर्त;

	  z = परिमाण(पूर्णाङ्क);
	  विकल्प ((*p_arg)->type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	      *(चिह्नित पूर्णाङ्क *) argp = (चिह्नित पूर्णाङ्क)*(SINT8 *)(* p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_UINT8:
	      *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT8 *)(* p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_SINT16:
	      *(चिह्नित पूर्णाङ्क *) argp = (चिह्नित पूर्णाङ्क)*(SINT16 *)(* p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_UINT16:
	      *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT16 *)(* p_argv);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_STRUCT:
	      *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT32 *)(* p_argv);
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      अन्यथा यदि (z == परिमाण(पूर्णाङ्क))
	{
#यदि defined(__SH4__)
	  यदि ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      यदि (freg++ < NFREGARG)
		अनुवर्त;
	    }
	  अन्यथा
#यदि_अन्तः
	    {
	      यदि (greg++ < NGREGARG)
		अनुवर्त;
	    }
	  *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#यदि defined(__SH4__)
      अन्यथा यदि ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  यदि (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      अनुवर्त;
	    }
	  स्मृतिं_प्रतिलिख (argp, *p_argv, z);
	  argp += z;
	}
#यदि_अन्तः
      अन्यथा
	{
	  पूर्णाङ्क n = (z + परिमाण (पूर्णाङ्क) - 1) / परिमाण (पूर्णाङ्क);
	  यदि (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      अनुवर्त;
	    }
#यदि (! defined(__SH4__))
	  अन्यथा यदि (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      अनुवर्त;
	    }
#यदि_अन्तः
	  स्मृतिं_प्रतिलिख (argp, *p_argv, z);
	  argp += n * परिमाण (पूर्णाङ्क);
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
#यदि defined(__SH4__)
  पूर्णाङ्क freg = 0;
#यदि_अन्तः

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_मानम्_स्थानाङ्कः_WITH_ARG) ? 1 : 0;

#यदि defined(__SH4__)
  प्रति (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      विकल्प (type)
	{
	पक्ष FFI_TYPE_FLOAT:
	  यदि (freg >= NFREGARG)
	    अनुवर्त;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  यदि ((freg + 1) >= NFREGARG)
	    अनुवर्त;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  भङ्ग;
	      
	स्वभाव:
	  आकार = (cif->arg_types)[i]->आकार;
	  n = (आकार + परिमाण (पूर्णाङ्क) - 1) / परिमाण (पूर्णाङ्क);
	  यदि (greg + n - 1 >= NGREGARG)
		अनुवर्त;
	  greg += n;
	  प्रति (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  भङ्ग;
	}
    }
#अन्यथा
  प्रति (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      आकार = (cif->arg_types)[i]->आकार;
      n = (आकार + परिमाण (पूर्णाङ्क) - 1) / परिमाण (पूर्णाङ्क);
      यदि (greg >= NGREGARG)
	अनुवर्त;
      अन्यथा यदि (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      प्रति (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#यदि_अन्तः

  /* Set the return type flag */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_STRUCT:
      cif->flags += (अचिह्नित) (return_type (cif->rtype)) << 24;
      भङ्ग;

    पक्ष FFI_TYPE_VOID:
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
      cif->flags += (अचिह्नित) cif->rtype->type << 24;
      भङ्ग;

    स्वभाव:
      cif->flags += FFI_TYPE_INT << 24;
      भङ्ग;
    }

  प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_call_SYSV(शून्य (*)(वर्ण *, extended_cif *), extended_cif *,
			  अचिह्नित, अचिह्नित, अचिह्नित *, शून्य (*fn)(शून्य));

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
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
      ffi_call_SYSV(ffi_prep_तर्काः, &ecif, cif->अष्टकानि, cif->flags, ecif.rvalue,
		    fn);
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
#यदि defined(__SH4__)
बाह्य शून्य __ic_invalidate (शून्य *पङ्क्तिः);
#यदि_अन्तः

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः* संवृतिः,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_दत्त,
		      शून्य *codeloc)
{
  अचिह्नित पूर्णाङ्क *tramp;
  अचिह्नित पूर्णाङ्क insn;

  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  tramp = (अचिह्नित पूर्णाङ्क *) &संवृतिः->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#यदि_परिभाषितम् __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#अन्यथा
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#यदि_अन्तः
  *(शून्य **) &tramp[2] = (शून्य *)codeloc;          /* ctx */
  *(शून्य **) &tramp[3] = (शून्य *)ffi_संवृतिः_SYSV; /* funaddr */

  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;

#यदि defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#यदि_अन्तः

  प्रत्यागम FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

#यदि_परिभाषितम् __LITTLE_ENDIAN__
#परिभाषय OFS_INT8	0
#परिभाषय OFS_INT16	0
#अन्यथा
#परिभाषय OFS_INT8	3
#परिभाषय OFS_INT16	2
#यदि_अन्तः

पूर्णाङ्क
ffi_संवृतिः_helper_SYSV (ffi_संवृतिः *संवृतिः, शून्य *rvalue, 
			 अचिह्नित दीर्घ *pgr, अचिह्नित दीर्घ *pfr, 
			 अचिह्नित दीर्घ *pst)
{
  शून्य **avalue;
  ffi_type **p_arg;
  पूर्णाङ्क i, avn;
  पूर्णाङ्क ireg, greg = 0;
#यदि defined(__SH4__)
  पूर्णाङ्क freg = 0;
#यदि_अन्तः
  ffi_cif *cif; 

  cif = संवृतिः->cif;
  avalue = alloca(cif->nargs * परिमाण(शून्य *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  यदि (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_मानम्_स्थानाङ्कः_WITH_ARG)
    {
      rvalue = (शून्य *) *pgr++;
      ireg = 1;
    }
  अन्यथा
    ireg = 0;

  cif = संवृतिः->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  प्रति (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->आकार;
      यदि (z < परिमाण(पूर्णाङ्क))
	{
	  यदि (greg++ >= NGREGARG)
	    अनुवर्त;

	  z = परिमाण(पूर्णाङ्क);
	  विकल्प ((*p_arg)->type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	    पक्ष FFI_TYPE_UINT8:
	      avalue[i] = (((वर्ण *)pgr) + OFS_INT8);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_SINT16:
	    पक्ष FFI_TYPE_UINT16:
	      avalue[i] = (((वर्ण *)pgr) + OFS_INT16);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      अन्यथा यदि (z == परिमाण(पूर्णाङ्क))
	{
#यदि defined(__SH4__)
	  यदि ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      यदि (freg++ >= NFREGARG)
		अनुवर्त;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
#यदि_अन्तः
	    {
	      यदि (greg++ >= NGREGARG)
		अनुवर्त;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#यदि defined(__SH4__)
      अन्यथा यदि ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  यदि (freg + 1 >= NFREGARG)
	    अनुवर्त;
	  यदि (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#यदि_अन्तः
      अन्यथा
	{
	  पूर्णाङ्क n = (z + परिमाण (पूर्णाङ्क) - 1) / परिमाण (पूर्णाङ्क);
#यदि defined(__SH4__)
	  यदि (greg + n - 1 >= NGREGARG)
	    अनुवर्त;
#अन्यथा
	  यदि (greg >= NGREGARG)
	    अनुवर्त;
#यदि_अन्तः
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#यदि defined(__SH4__)
  freg = 0;
#यदि_अन्तः

  प्रति (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->आकार;
      यदि (z < परिमाण(पूर्णाङ्क))
	{
	  यदि (greg++ < NGREGARG)
	    अनुवर्त;

	  z = परिमाण(पूर्णाङ्क);
	  विकल्प ((*p_arg)->type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	    पक्ष FFI_TYPE_UINT8:
	      avalue[i] = (((वर्ण *)pst) + OFS_INT8);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_SINT16:
	    पक्ष FFI_TYPE_UINT16:
	      avalue[i] = (((वर्ण *)pst) + OFS_INT16);
	      भङ्ग;
  
	    पक्ष FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      अन्यथा यदि (z == परिमाण(पूर्णाङ्क))
	{
#यदि defined(__SH4__)
	  यदि ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      यदि (freg++ < NFREGARG)
		अनुवर्त;
	    }
	  अन्यथा
#यदि_अन्तः
	    {
	      यदि (greg++ < NGREGARG)
		अनुवर्त;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#यदि defined(__SH4__)
      अन्यथा यदि ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  यदि (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      अनुवर्त;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#यदि_अन्तः
      अन्यथा
	{
	  पूर्णाङ्क n = (z + परिमाण (पूर्णाङ्क) - 1) / परिमाण (पूर्णाङ्क);
	  यदि (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      अनुवर्त;
	    }
#यदि (! defined(__SH4__))
	  अन्यथा यदि (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      अनुवर्त;
	    }
#यदि_अन्तः
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (संवृतिः->fun) (cif, rvalue, avalue, संवृतिः->user_दत्त);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  प्रत्यागम return_type (cif->rtype);
}
