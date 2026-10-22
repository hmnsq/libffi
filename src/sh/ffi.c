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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>

#define NGREGARG 4
#যদি defined(__SH4__)
#define NFREGARG 8
#endif

#যদি defined(__HITACHI__)
#define STRUCT_মান_ADDRESS_WITH_ARG 1
#অন্যথা
#define STRUCT_মান_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
স্থির পূর্ণসংখ্যা
simple_type (ffi_type *arg)
{
  যদি (arg->type != FFI_TYPE_STRUCT)
    ফেরত arg->type;
  অন্যথা যদি (arg->elements[1])
    ফেরত FFI_TYPE_STRUCT;

  ফেরত simple_type (arg->elements[0]);
}

স্থির পূর্ণসংখ্যা
return_type (ffi_type *arg)
{
  unsigned short type;

  যদি (arg->type != FFI_TYPE_STRUCT)
    ফেরত arg->type;

  type = simple_type (arg->elements[0]);
  যদি (! arg->elements[1])
    {
      নির্বাচন (type)
	{
	ক্ষেত্র FFI_TYPE_SINT8:
	ক্ষেত্র FFI_TYPE_UINT8:
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_UINT16:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT32:
	  ফেরত FFI_TYPE_INT;

	পূর্বনির্ধারিত:
	  ফেরত type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  যদি (arg->আকার_2 <= 2 * আকার (পূর্ণসংখ্যা))
    {
      পূর্ণসংখ্যা i = 0;
      ffi_type *e;

      যখন ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  নির্বাচন (type)
	    {
	    ক্ষেত্র FFI_TYPE_SINT32:
	    ক্ষেত্র FFI_TYPE_UINT32:
	    ক্ষেত্র FFI_TYPE_INT:
	    ক্ষেত্র FFI_TYPE_FLOAT:
	      ফেরত FFI_TYPE_UINT64;

	    পূর্বনির্ধারিত:
	      বিরতি;
	    }
	}
    }

  ফেরত FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

শূন্য ffi_prep_args(অক্ষর *স্তূপ, extended_cif *ecif)
{
  নিবন্ধ unsigned পূর্ণসংখ্যা i;
  নিবন্ধ পূর্ণসংখ্যা tmp;
  নিবন্ধ unsigned পূর্ণসংখ্যা avn;
  নিবন্ধ শূন্য **p_argv;
  নিবন্ধ অক্ষর *argp;
  নিবন্ধ ffi_type **p_arg;
  পূর্ণসংখ্যা greg, ireg;
#যদি defined(__SH4__)
  পূর্ণসংখ্যা freg = 0;
#endif

  tmp = 0;
  argp = স্তূপ;

  যদি (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(শূন্য **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_মান_ADDRESS_WITH_ARG ? 1 : 0;
    }
  অন্যথা
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  জন্য (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->আকার_2;
      যদি (z < আকার(পূর্ণসংখ্যা))
	{
	  যদি (greg++ >= NGREGARG)
	    চালু;

	  z = আকার(পূর্ণসংখ্যা);
	  নির্বাচন ((*p_arg)->type)
	    {
	    ক্ষেত্র FFI_TYPE_SINT8:
	      *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা)*(SINT8 *)(* p_argv);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_UINT8:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT8 *)(* p_argv);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_SINT16:
	      *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা)*(SINT16 *)(* p_argv);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_UINT16:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT16 *)(* p_argv);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_STRUCT:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT32 *)(* p_argv);
	      বিরতি;

	    পূর্বনির্ধারিত:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      অন্যথা যদি (z == আকার(পূর্ণসংখ্যা))
	{
#যদি defined(__SH4__)
	  যদি ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      যদি (freg++ >= NFREGARG)
		চালু;
	    }
	  অন্যথা
#endif
	    {
	      যদি (greg++ >= NGREGARG)
		চালু;
	    }
	  *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#যদি defined(__SH4__)
      অন্যথা যদি ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  যদি (freg + 1 >= NFREGARG)
	    চালু;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      অন্যথা
	{
	  পূর্ণসংখ্যা n = (z + আকার (পূর্ণসংখ্যা) - 1) / আকার (পূর্ণসংখ্যা);
#যদি defined(__SH4__)
	  যদি (greg + n - 1 >= NGREGARG)
	    চালু;
#অন্যথা
	  যদি (greg >= NGREGARG)
	    চালু;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * আকার (পূর্ণসংখ্যা);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#যদি defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  জন্য (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->আকার_2;
      যদি (z < আকার(পূর্ণসংখ্যা))
	{
	  যদি (greg++ < NGREGARG)
	    চালু;

	  z = আকার(পূর্ণসংখ্যা);
	  নির্বাচন ((*p_arg)->type)
	    {
	    ক্ষেত্র FFI_TYPE_SINT8:
	      *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা)*(SINT8 *)(* p_argv);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_UINT8:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT8 *)(* p_argv);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_SINT16:
	      *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা)*(SINT16 *)(* p_argv);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_UINT16:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT16 *)(* p_argv);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_STRUCT:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT32 *)(* p_argv);
	      বিরতি;

	    পূর্বনির্ধারিত:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      অন্যথা যদি (z == আকার(পূর্ণসংখ্যা))
	{
#যদি defined(__SH4__)
	  যদি ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      যদি (freg++ < NFREGARG)
		চালু;
	    }
	  অন্যথা
#endif
	    {
	      যদি (greg++ < NGREGARG)
		চালু;
	    }
	  *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#যদি defined(__SH4__)
      অন্যথা যদি ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  যদি (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      চালু;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      অন্যথা
	{
	  পূর্ণসংখ্যা n = (z + আকার (পূর্ণসংখ্যা) - 1) / আকার (পূর্ণসংখ্যা);
	  যদি (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      চালু;
	    }
#যদি (! defined(__SH4__))
	  অন্যথা যদি (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      চালু;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * আকার (পূর্ণসংখ্যা);
	}
    }

  ফেরত;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  পূর্ণসংখ্যা i, j;
  পূর্ণসংখ্যা আকার_2, type;
  পূর্ণসংখ্যা n, m;
  পূর্ণসংখ্যা greg;
#যদি defined(__SH4__)
  পূর্ণসংখ্যা freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_মান_ADDRESS_WITH_ARG) ? 1 : 0;

#যদি defined(__SH4__)
  জন্য (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      নির্বাচন (type)
	{
	ক্ষেত্র FFI_TYPE_FLOAT:
	  যদি (freg >= NFREGARG)
	    চালু;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_DOUBLE:
	  যদি ((freg + 1) >= NFREGARG)
	    চালু;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  বিরতি;
	      
	পূর্বনির্ধারিত:
	  আকার_2 = (cif->arg_types)[i]->আকার_2;
	  n = (আকার_2 + আকার (পূর্ণসংখ্যা) - 1) / আকার (পূর্ণসংখ্যা);
	  যদি (greg + n - 1 >= NGREGARG)
		চালু;
	  greg += n;
	  জন্য (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  বিরতি;
	}
    }
#অন্যথা
  জন্য (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      আকার_2 = (cif->arg_types)[i]->আকার_2;
      n = (আকার_2 + আকার (পূর্ণসংখ্যা) - 1) / আকার (পূর্ণসংখ্যা);
      যদি (greg >= NGREGARG)
	চালু;
      অন্যথা যদি (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      জন্য (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  নির্বাচন (cif->rtype->type)
    {
    ক্ষেত্র FFI_TYPE_STRUCT:
      cif->flags += (unsigned) (return_type (cif->rtype)) << 24;
      বিরতি;

    ক্ষেত্র FFI_TYPE_VOID:
    ক্ষেত্র FFI_TYPE_FLOAT:
    ক্ষেত্র FFI_TYPE_DOUBLE:
    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
      cif->flags += (unsigned) cif->rtype->type << 24;
      বিরতি;

    পূর্বনির্ধারিত:
      cif->flags += FFI_TYPE_INT << 24;
      বিরতি;
    }

  ফেরত FFI_OK;
}

বাহ্যিক শূন্য ffi_call_SYSV(শূন্য (*)(অক্ষর *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, শূন্য (*fn)(শূন্য));

শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  যদি (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  অন্যথা যদি ((rvalue == ফাঁকা) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->আকার_2);
    }
  অন্যথা
    ecif.rvalue = rvalue;

  নির্বাচন (cif->abi) 
    {
    ক্ষেত্র FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      বিরতি;
    পূর্বনির্ধারিত:
      FFI_ASSERT(0);
      বিরতি;
    }

  যদি (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->আকার_2);
}

বাহ্যিক শূন্য ffi_closure_SYSV (শূন্য);
#যদি defined(__SH4__)
বাহ্যিক শূন্য __ic_invalidate (শূন্য *সারি);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc)
{
  unsigned পূর্ণসংখ্যা *tramp;
  unsigned পূর্ণসংখ্যা insn;

  যদি (cif->abi != FFI_SYSV)
    ফেরত FFI_BAD_ABI;

  tramp = (unsigned পূর্ণসংখ্যা *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#অন্যথা
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(শূন্য **) &tramp[2] = (শূন্য *)codeloc;          /* ctx */
  *(শূন্য **) &tramp[3] = (শূন্য *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

#যদি defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  ফেরত FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

#ifdef __LITTLE_ENDIAN__
#define OFS_INT8	0
#define OFS_INT16	0
#অন্যথা
#define OFS_INT8	3
#define OFS_INT16	2
#endif

পূর্ণসংখ্যা
ffi_closure_helper_SYSV (ffi_closure *closure, শূন্য *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  শূন্য **avalue;
  ffi_type **p_arg;
  পূর্ণসংখ্যা i, avn;
  পূর্ণসংখ্যা ireg, greg = 0;
#যদি defined(__SH4__)
  পূর্ণসংখ্যা freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * আকার(শূন্য *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  যদি (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_মান_ADDRESS_WITH_ARG)
    {
      rvalue = (শূন্য *) *pgr++;
      ireg = 1;
    }
  অন্যথা
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  জন্য (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->আকার_2;
      যদি (z < আকার(পূর্ণসংখ্যা))
	{
	  যদি (greg++ >= NGREGARG)
	    চালু;

	  z = আকার(পূর্ণসংখ্যা);
	  নির্বাচন ((*p_arg)->type)
	    {
	    ক্ষেত্র FFI_TYPE_SINT8:
	    ক্ষেত্র FFI_TYPE_UINT8:
	      avalue[i] = (((অক্ষর *)pgr) + OFS_INT8);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_SINT16:
	    ক্ষেত্র FFI_TYPE_UINT16:
	      avalue[i] = (((অক্ষর *)pgr) + OFS_INT16);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      বিরতি;

	    পূর্বনির্ধারিত:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      অন্যথা যদি (z == আকার(পূর্ণসংখ্যা))
	{
#যদি defined(__SH4__)
	  যদি ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      যদি (freg++ >= NFREGARG)
		চালু;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  অন্যথা
#endif
	    {
	      যদি (greg++ >= NGREGARG)
		চালু;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#যদি defined(__SH4__)
      অন্যথা যদি ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  যদি (freg + 1 >= NFREGARG)
	    চালু;
	  যদি (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      অন্যথা
	{
	  পূর্ণসংখ্যা n = (z + আকার (পূর্ণসংখ্যা) - 1) / আকার (পূর্ণসংখ্যা);
#যদি defined(__SH4__)
	  যদি (greg + n - 1 >= NGREGARG)
	    চালু;
#অন্যথা
	  যদি (greg >= NGREGARG)
	    চালু;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#যদি defined(__SH4__)
  freg = 0;
#endif

  জন্য (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->আকার_2;
      যদি (z < আকার(পূর্ণসংখ্যা))
	{
	  যদি (greg++ < NGREGARG)
	    চালু;

	  z = আকার(পূর্ণসংখ্যা);
	  নির্বাচন ((*p_arg)->type)
	    {
	    ক্ষেত্র FFI_TYPE_SINT8:
	    ক্ষেত্র FFI_TYPE_UINT8:
	      avalue[i] = (((অক্ষর *)pst) + OFS_INT8);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_SINT16:
	    ক্ষেত্র FFI_TYPE_UINT16:
	      avalue[i] = (((অক্ষর *)pst) + OFS_INT16);
	      বিরতি;
  
	    ক্ষেত্র FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      বিরতি;

	    পূর্বনির্ধারিত:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      অন্যথা যদি (z == আকার(পূর্ণসংখ্যা))
	{
#যদি defined(__SH4__)
	  যদি ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      যদি (freg++ < NFREGARG)
		চালু;
	    }
	  অন্যথা
#endif
	    {
	      যদি (greg++ < NGREGARG)
		চালু;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#যদি defined(__SH4__)
      অন্যথা যদি ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  যদি (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      চালু;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      অন্যথা
	{
	  পূর্ণসংখ্যা n = (z + আকার (পূর্ণসংখ্যা) - 1) / আকার (পূর্ণসংখ্যা);
	  যদি (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      চালু;
	    }
#যদি (! defined(__SH4__))
	  অন্যথা যদি (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      চালু;
	    }
#endif
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_তথ্য);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ফেরত return_type (cif->rtype);
}
