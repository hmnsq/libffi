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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>

#অন্তর্ভাৱ <stdlib.h>

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
স্থির পূর্ণাঙ্ক
simple_type (ffi_type *arg)
{
  যদি (arg->type != FFI_TYPE_STRUCT)
    ঘূৰাই arg->type;
  অন্যথা যদি (arg->elements[1])
    ঘূৰাই FFI_TYPE_STRUCT;

  ঘূৰাই simple_type (arg->elements[0]);
}

স্থির পূর্ণাঙ্ক
return_type (ffi_type *arg)
{
  unsigned short type;

  যদি (arg->type != FFI_TYPE_STRUCT)
    ঘূৰাই arg->type;

  type = simple_type (arg->elements[0]);
  যদি (! arg->elements[1])
    {
      ৱিকল্প (type)
	{
	পক্ষ FFI_TYPE_SINT8:
	পক্ষ FFI_TYPE_UINT8:
	পক্ষ FFI_TYPE_SINT16:
	পক্ষ FFI_TYPE_UINT16:
	পক্ষ FFI_TYPE_SINT32:
	পক্ষ FFI_TYPE_UINT32:
	  ঘূৰাই FFI_TYPE_INT;

	স্ৱভাৱ:
	  ঘূৰাই type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  যদি (arg->size <= 2 * পরিমাণ (পূর্ণাঙ্ক))
    {
      পূর্ণাঙ্ক i = 0;
      ffi_type *e;

      যেতিয়ালৈ ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  ৱিকল্প (type)
	    {
	    পক্ষ FFI_TYPE_SINT32:
	    পক্ষ FFI_TYPE_UINT32:
	    পক্ষ FFI_TYPE_INT:
	    পক্ষ FFI_TYPE_FLOAT:
	      ঘূৰাই FFI_TYPE_UINT64;

	    স্ৱভাৱ:
	      ভঙ্গ;
	    }
	}
    }

  ঘূৰাই FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

শূন্য ffi_prep_args(ৱর্ণ *স্তূপ, extended_cif *ecif)
{
  পঞ্জিকা unsigned পূর্ণাঙ্ক i;
  পঞ্জিকা পূর্ণাঙ্ক tmp;
  পঞ্জিকা unsigned পূর্ণাঙ্ক avn;
  পঞ্জিকা শূন্য **p_argv;
  পঞ্জিকা ৱর্ণ *argp;
  পঞ্জিকা ffi_type **p_arg;
  পূর্ণাঙ্ক greg, ireg;
#যদি defined(__SH4__)
  পূর্ণাঙ্ক freg = 0;
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

  বাবে (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      যদি (z < পরিমাণ(পূর্ণাঙ্ক))
	{
	  যদি (greg++ >= NGREGARG)
	    অনুৱর্ত;

	  z = পরিমাণ(পূর্ণাঙ্ক);
	  ৱিকল্প ((*p_arg)->type)
	    {
	    পক্ষ FFI_TYPE_SINT8:
	      *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক)*(SINT8 *)(* p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_UINT8:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT8 *)(* p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_SINT16:
	      *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক)*(SINT16 *)(* p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_UINT16:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT16 *)(* p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_STRUCT:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT32 *)(* p_argv);
	      ভঙ্গ;

	    স্ৱভাৱ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      অন্যথা যদি (z == পরিমাণ(পূর্ণাঙ্ক))
	{
#যদি defined(__SH4__)
	  যদি ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      যদি (freg++ >= NFREGARG)
		অনুৱর্ত;
	    }
	  অন্যথা
#endif
	    {
	      যদি (greg++ >= NGREGARG)
		অনুৱর্ত;
	    }
	  *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#যদি defined(__SH4__)
      অন্যথা যদি ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  যদি (freg + 1 >= NFREGARG)
	    অনুৱর্ত;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      অন্যথা
	{
	  পূর্ণাঙ্ক n = (z + পরিমাণ (পূর্ণাঙ্ক) - 1) / পরিমাণ (পূর্ণাঙ্ক);
#যদি defined(__SH4__)
	  যদি (greg + n - 1 >= NGREGARG)
	    অনুৱর্ত;
#অন্যথা
	  যদি (greg >= NGREGARG)
	    অনুৱর্ত;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * পরিমাণ (পূর্ণাঙ্ক);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#যদি defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  বাবে (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      যদি (z < পরিমাণ(পূর্ণাঙ্ক))
	{
	  যদি (greg++ < NGREGARG)
	    অনুৱর্ত;

	  z = পরিমাণ(পূর্ণাঙ্ক);
	  ৱিকল্প ((*p_arg)->type)
	    {
	    পক্ষ FFI_TYPE_SINT8:
	      *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক)*(SINT8 *)(* p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_UINT8:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT8 *)(* p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_SINT16:
	      *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক)*(SINT16 *)(* p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_UINT16:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT16 *)(* p_argv);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_STRUCT:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT32 *)(* p_argv);
	      ভঙ্গ;

	    স্ৱভাৱ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      অন্যথা যদি (z == পরিমাণ(পূর্ণাঙ্ক))
	{
#যদি defined(__SH4__)
	  যদি ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      যদি (freg++ < NFREGARG)
		অনুৱর্ত;
	    }
	  অন্যথা
#endif
	    {
	      যদি (greg++ < NGREGARG)
		অনুৱর্ত;
	    }
	  *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#যদি defined(__SH4__)
      অন্যথা যদি ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  যদি (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      অনুৱর্ত;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      অন্যথা
	{
	  পূর্ণাঙ্ক n = (z + পরিমাণ (পূর্ণাঙ্ক) - 1) / পরিমাণ (পূর্ণাঙ্ক);
	  যদি (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      অনুৱর্ত;
	    }
#যদি (! defined(__SH4__))
	  অন্যথা যদি (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      অনুৱর্ত;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * পরিমাণ (পূর্ণাঙ্ক);
	}
    }

  ঘূৰাই;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  পূর্ণাঙ্ক i, j;
  পূর্ণাঙ্ক size, type;
  পূর্ণাঙ্ক n, m;
  পূর্ণাঙ্ক greg;
#যদি defined(__SH4__)
  পূর্ণাঙ্ক freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_মান_ADDRESS_WITH_ARG) ? 1 : 0;

#যদি defined(__SH4__)
  বাবে (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      ৱিকল্প (type)
	{
	পক্ষ FFI_TYPE_FLOAT:
	  যদি (freg >= NFREGARG)
	    অনুৱর্ত;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_DOUBLE:
	  যদি ((freg + 1) >= NFREGARG)
	    অনুৱর্ত;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ভঙ্গ;
	      
	স্ৱভাৱ:
	  size = (cif->arg_types)[i]->size;
	  n = (size + পরিমাণ (পূর্ণাঙ্ক) - 1) / পরিমাণ (পূর্ণাঙ্ক);
	  যদি (greg + n - 1 >= NGREGARG)
		অনুৱর্ত;
	  greg += n;
	  বাবে (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  ভঙ্গ;
	}
    }
#অন্যথা
  বাবে (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + পরিমাণ (পূর্ণাঙ্ক) - 1) / পরিমাণ (পূর্ণাঙ্ক);
      যদি (greg >= NGREGARG)
	অনুৱর্ত;
      অন্যথা যদি (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      বাবে (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  ৱিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_STRUCT:
      cif->flags += (unsigned) (return_type (cif->rtype)) << 24;
      ভঙ্গ;

    পক্ষ FFI_TYPE_VOID:
    পক্ষ FFI_TYPE_FLOAT:
    পক্ষ FFI_TYPE_DOUBLE:
    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
      cif->flags += (unsigned) cif->rtype->type << 24;
      ভঙ্গ;

    স্ৱভাৱ:
      cif->flags += FFI_TYPE_INT << 24;
      ভঙ্গ;
    }

  ঘূৰাই FFI_OK;
}

বাহ্য শূন্য ffi_call_SYSV(শূন্য (*)(ৱর্ণ *, extended_cif *), extended_cif *,
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
  অন্যথা যদি ((rvalue == রিক্ত) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  অন্যথা
    ecif.rvalue = rvalue;

  ৱিকল্প (cif->abi) 
    {
    পক্ষ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      ভঙ্গ;
    স্ৱভাৱ:
      FFI_ASSERT(0);
      ভঙ্গ;
    }

  যদি (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

বাহ্য শূন্য ffi_closure_SYSV (শূন্য);
#যদি defined(__SH4__)
বাহ্য শূন্য __ic_invalidate (শূন্য *সাৰি);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_data,
		      শূন্য *codeloc)
{
  unsigned পূর্ণাঙ্ক *tramp;
  unsigned পূর্ণাঙ্ক insn;

  যদি (cif->abi != FFI_SYSV)
    ঘূৰাই FFI_BAD_ABI;

  tramp = (unsigned পূর্ণাঙ্ক *) &closure->tramp[0];
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
  closure->user_data = user_data;

#যদি defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  ঘূৰাই FFI_OK;
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

পূর্ণাঙ্ক
ffi_closure_helper_SYSV (ffi_closure *closure, শূন্য *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  শূন্য **avalue;
  ffi_type **p_arg;
  পূর্ণাঙ্ক i, avn;
  পূর্ণাঙ্ক ireg, greg = 0;
#যদি defined(__SH4__)
  পূর্ণাঙ্ক freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * পরিমাণ(শূন্য *));

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
  বাবে (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      যদি (z < পরিমাণ(পূর্ণাঙ্ক))
	{
	  যদি (greg++ >= NGREGARG)
	    অনুৱর্ত;

	  z = পরিমাণ(পূর্ণাঙ্ক);
	  ৱিকল্প ((*p_arg)->type)
	    {
	    পক্ষ FFI_TYPE_SINT8:
	    পক্ষ FFI_TYPE_UINT8:
	      avalue[i] = (((ৱর্ণ *)pgr) + OFS_INT8);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_SINT16:
	    পক্ষ FFI_TYPE_UINT16:
	      avalue[i] = (((ৱর্ণ *)pgr) + OFS_INT16);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      ভঙ্গ;

	    স্ৱভাৱ:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      অন্যথা যদি (z == পরিমাণ(পূর্ণাঙ্ক))
	{
#যদি defined(__SH4__)
	  যদি ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      যদি (freg++ >= NFREGARG)
		অনুৱর্ত;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  অন্যথা
#endif
	    {
	      যদি (greg++ >= NGREGARG)
		অনুৱর্ত;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#যদি defined(__SH4__)
      অন্যথা যদি ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  যদি (freg + 1 >= NFREGARG)
	    অনুৱর্ত;
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
	  পূর্ণাঙ্ক n = (z + পরিমাণ (পূর্ণাঙ্ক) - 1) / পরিমাণ (পূর্ণাঙ্ক);
#যদি defined(__SH4__)
	  যদি (greg + n - 1 >= NGREGARG)
	    অনুৱর্ত;
#অন্যথা
	  যদি (greg >= NGREGARG)
	    অনুৱর্ত;
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

  বাবে (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      যদি (z < পরিমাণ(পূর্ণাঙ্ক))
	{
	  যদি (greg++ < NGREGARG)
	    অনুৱর্ত;

	  z = পরিমাণ(পূর্ণাঙ্ক);
	  ৱিকল্প ((*p_arg)->type)
	    {
	    পক্ষ FFI_TYPE_SINT8:
	    পক্ষ FFI_TYPE_UINT8:
	      avalue[i] = (((ৱর্ণ *)pst) + OFS_INT8);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_SINT16:
	    পক্ষ FFI_TYPE_UINT16:
	      avalue[i] = (((ৱর্ণ *)pst) + OFS_INT16);
	      ভঙ্গ;
  
	    পক্ষ FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      ভঙ্গ;

	    স্ৱভাৱ:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      অন্যথা যদি (z == পরিমাণ(পূর্ণাঙ্ক))
	{
#যদি defined(__SH4__)
	  যদি ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      যদি (freg++ < NFREGARG)
		অনুৱর্ত;
	    }
	  অন্যথা
#endif
	    {
	      যদি (greg++ < NGREGARG)
		অনুৱর্ত;
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
	      অনুৱর্ত;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      অন্যথা
	{
	  পূর্ণাঙ্ক n = (z + পরিমাণ (পূর্ণাঙ্ক) - 1) / পরিমাণ (পূর্ণাঙ্ক);
	  যদি (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      অনুৱর্ত;
	    }
#যদি (! defined(__SH4__))
	  অন্যথা যদি (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      অনুৱর্ত;
	    }
#endif
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ঘূৰাই return_type (cif->rtype);
}
