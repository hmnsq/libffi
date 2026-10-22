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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>

#define NGREGARG 4
#اگر defined(__SH4__)
#define NFREGARG 8
#endif

#اگر defined(__HITACHI__)
#define STRUCT_مقدار_ADDRESS_WITH_ARG 1
#وگرنه
#define STRUCT_مقدار_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
ایستا صحیح
simple_type (ffi_type *arg)
{
  اگر (arg->type != FFI_TYPE_STRUCT)
    بازگشت arg->type;
  وگرنه اگر (arg->elements[1])
    بازگشت FFI_TYPE_STRUCT;

  بازگشت simple_type (arg->elements[0]);
}

ایستا صحیح
return_type (ffi_type *arg)
{
  unsigned short type;

  اگر (arg->type != FFI_TYPE_STRUCT)
    بازگشت arg->type;

  type = simple_type (arg->elements[0]);
  اگر (! arg->elements[1])
    {
      انتخاب (type)
	{
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	  بازگشت FFI_TYPE_INT;

	پیشفرض:
	  بازگشت type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  اگر (arg->size <= 2 * اندازه (صحیح))
    {
      صحیح i = 0;
      ffi_type *e;

      وقتی ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  انتخاب (type)
	    {
	    حالت FFI_TYPE_SINT32:
	    حالت FFI_TYPE_UINT32:
	    حالت FFI_TYPE_INT:
	    حالت FFI_TYPE_FLOAT:
	      بازگشت FFI_TYPE_UINT64;

	    پیشفرض:
	      بشکن;
	    }
	}
    }

  بازگشت FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

تهی ffi_prep_args(نویسه *پشته, extended_cif *ecif)
{
  ثبات unsigned صحیح i;
  ثبات صحیح tmp;
  ثبات unsigned صحیح avn;
  ثبات تهی **p_argv;
  ثبات نویسه *argp;
  ثبات ffi_type **p_arg;
  صحیح greg, ireg;
#اگر defined(__SH4__)
  صحیح freg = 0;
#endif

  tmp = 0;
  argp = پشته;

  اگر (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(تهی **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_مقدار_ADDRESS_WITH_ARG ? 1 : 0;
    }
  وگرنه
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  برای (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      اگر (z < اندازه(صحیح))
	{
	  اگر (greg++ >= NGREGARG)
	    ادامه;

	  z = اندازه(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(signed صحیح *) argp = (signed صحیح)*(SINT8 *)(* p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_UINT8:
	      *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT8 *)(* p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_SINT16:
	      *(signed صحیح *) argp = (signed صحیح)*(SINT16 *)(* p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_UINT16:
	      *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT16 *)(* p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_STRUCT:
	      *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT32 *)(* p_argv);
	      بشکن;

	    پیشفرض:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      وگرنه اگر (z == اندازه(صحیح))
	{
#اگر defined(__SH4__)
	  اگر ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      اگر (freg++ >= NFREGARG)
		ادامه;
	    }
	  وگرنه
#endif
	    {
	      اگر (greg++ >= NGREGARG)
		ادامه;
	    }
	  *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#اگر defined(__SH4__)
      وگرنه اگر ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  اگر (freg + 1 >= NFREGARG)
	    ادامه;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      وگرنه
	{
	  صحیح n = (z + اندازه (صحیح) - 1) / اندازه (صحیح);
#اگر defined(__SH4__)
	  اگر (greg + n - 1 >= NGREGARG)
	    ادامه;
#وگرنه
	  اگر (greg >= NGREGARG)
	    ادامه;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * اندازه (صحیح);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#اگر defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  برای (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      اگر (z < اندازه(صحیح))
	{
	  اگر (greg++ < NGREGARG)
	    ادامه;

	  z = اندازه(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(signed صحیح *) argp = (signed صحیح)*(SINT8 *)(* p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_UINT8:
	      *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT8 *)(* p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_SINT16:
	      *(signed صحیح *) argp = (signed صحیح)*(SINT16 *)(* p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_UINT16:
	      *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT16 *)(* p_argv);
	      بشکن;
  
	    حالت FFI_TYPE_STRUCT:
	      *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT32 *)(* p_argv);
	      بشکن;

	    پیشفرض:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      وگرنه اگر (z == اندازه(صحیح))
	{
#اگر defined(__SH4__)
	  اگر ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      اگر (freg++ < NFREGARG)
		ادامه;
	    }
	  وگرنه
#endif
	    {
	      اگر (greg++ < NGREGARG)
		ادامه;
	    }
	  *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#اگر defined(__SH4__)
      وگرنه اگر ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  اگر (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ادامه;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      وگرنه
	{
	  صحیح n = (z + اندازه (صحیح) - 1) / اندازه (صحیح);
	  اگر (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ادامه;
	    }
#اگر (! defined(__SH4__))
	  وگرنه اگر (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      ادامه;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * اندازه (صحیح);
	}
    }

  بازگشت;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  صحیح i, j;
  صحیح size, type;
  صحیح n, m;
  صحیح greg;
#اگر defined(__SH4__)
  صحیح freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_مقدار_ADDRESS_WITH_ARG) ? 1 : 0;

#اگر defined(__SH4__)
  برای (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      انتخاب (type)
	{
	حالت FFI_TYPE_FLOAT:
	  اگر (freg >= NFREGARG)
	    ادامه;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  بشکن;

	حالت FFI_TYPE_DOUBLE:
	  اگر ((freg + 1) >= NFREGARG)
	    ادامه;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  بشکن;
	      
	پیشفرض:
	  size = (cif->arg_types)[i]->size;
	  n = (size + اندازه (صحیح) - 1) / اندازه (صحیح);
	  اگر (greg + n - 1 >= NGREGARG)
		ادامه;
	  greg += n;
	  برای (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  بشکن;
	}
    }
#وگرنه
  برای (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + اندازه (صحیح) - 1) / اندازه (صحیح);
      اگر (greg >= NGREGARG)
	ادامه;
      وگرنه اگر (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      برای (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_STRUCT:
      cif->flags += (unsigned) (return_type (cif->rtype)) << 24;
      بشکن;

    حالت FFI_TYPE_VOID:
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags += (unsigned) cif->rtype->type << 24;
      بشکن;

    پیشفرض:
      cif->flags += FFI_TYPE_INT << 24;
      بشکن;
    }

  بازگشت FFI_OK;
}

بیرونی تهی ffi_call_SYSV(تهی (*)(نویسه *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, تهی (*fn)(تهی));

تهی ffi_call(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
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
  وگرنه اگر ((rvalue == پوچ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  وگرنه
    ecif.rvalue = rvalue;

  انتخاب (cif->abi) 
    {
    حالت FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      بشکن;
    پیشفرض:
      FFI_ASSERT(0);
      بشکن;
    }

  اگر (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

بیرونی تهی ffi_closure_SYSV (تهی);
#اگر defined(__SH4__)
بیرونی تهی __ic_invalidate (تهی *سطر);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  unsigned صحیح *tramp;
  unsigned صحیح insn;

  اگر (cif->abi != FFI_SYSV)
    بازگشت FFI_BAD_ABI;

  tramp = (unsigned صحیح *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#وگرنه
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(تهی **) &tramp[2] = (تهی *)codeloc;          /* ctx */
  *(تهی **) &tramp[3] = (تهی *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#اگر defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  بازگشت FFI_OK;
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
#وگرنه
#define OFS_INT8	3
#define OFS_INT16	2
#endif

صحیح
ffi_closure_helper_SYSV (ffi_closure *closure, تهی *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  تهی **avalue;
  ffi_type **p_arg;
  صحیح i, avn;
  صحیح ireg, greg = 0;
#اگر defined(__SH4__)
  صحیح freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * اندازه(تهی *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  اگر (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_مقدار_ADDRESS_WITH_ARG)
    {
      rvalue = (تهی *) *pgr++;
      ireg = 1;
    }
  وگرنه
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  برای (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      اگر (z < اندازه(صحیح))
	{
	  اگر (greg++ >= NGREGARG)
	    ادامه;

	  z = اندازه(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	    حالت FFI_TYPE_UINT8:
	      avalue[i] = (((نویسه *)pgr) + OFS_INT8);
	      بشکن;
  
	    حالت FFI_TYPE_SINT16:
	    حالت FFI_TYPE_UINT16:
	      avalue[i] = (((نویسه *)pgr) + OFS_INT16);
	      بشکن;
  
	    حالت FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      بشکن;

	    پیشفرض:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      وگرنه اگر (z == اندازه(صحیح))
	{
#اگر defined(__SH4__)
	  اگر ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      اگر (freg++ >= NFREGARG)
		ادامه;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وگرنه
#endif
	    {
	      اگر (greg++ >= NGREGARG)
		ادامه;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#اگر defined(__SH4__)
      وگرنه اگر ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  اگر (freg + 1 >= NFREGARG)
	    ادامه;
	  اگر (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      وگرنه
	{
	  صحیح n = (z + اندازه (صحیح) - 1) / اندازه (صحیح);
#اگر defined(__SH4__)
	  اگر (greg + n - 1 >= NGREGARG)
	    ادامه;
#وگرنه
	  اگر (greg >= NGREGARG)
	    ادامه;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#اگر defined(__SH4__)
  freg = 0;
#endif

  برای (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      اگر (z < اندازه(صحیح))
	{
	  اگر (greg++ < NGREGARG)
	    ادامه;

	  z = اندازه(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	    حالت FFI_TYPE_UINT8:
	      avalue[i] = (((نویسه *)pst) + OFS_INT8);
	      بشکن;
  
	    حالت FFI_TYPE_SINT16:
	    حالت FFI_TYPE_UINT16:
	      avalue[i] = (((نویسه *)pst) + OFS_INT16);
	      بشکن;
  
	    حالت FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      بشکن;

	    پیشفرض:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      وگرنه اگر (z == اندازه(صحیح))
	{
#اگر defined(__SH4__)
	  اگر ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      اگر (freg++ < NFREGARG)
		ادامه;
	    }
	  وگرنه
#endif
	    {
	      اگر (greg++ < NGREGARG)
		ادامه;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#اگر defined(__SH4__)
      وگرنه اگر ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  اگر (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ادامه;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      وگرنه
	{
	  صحیح n = (z + اندازه (صحیح) - 1) / اندازه (صحیح);
	  اگر (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ادامه;
	    }
#اگر (! defined(__SH4__))
	  وگرنه اگر (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      ادامه;
	    }
#endif
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  بازگشت return_type (cif->rtype);
}
