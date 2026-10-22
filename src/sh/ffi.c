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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>

#تعریف_کړه NGREGARG 4
#إذا defined(__SH4__)
#تعریف_کړه NFREGARG 8
#د_که_پای

#إذا defined(__HITACHI__)
#تعریف_کړه STRUCT_ارزښت_پته_WITH_ARG 1
#وإلا
#تعریف_کړه STRUCT_ارزښت_پته_WITH_ARG 0
#د_که_پای

/* If the structure has essentially an unique element, return its type.  */
ساكن صحيح
simple_type (ffi_type *arg)
{
  إذا (arg->type != FFI_TYPE_STRUCT)
    أرجع arg->type;
  وإلا إذا (arg->elements[1])
    أرجع FFI_TYPE_STRUCT;

  أرجع simple_type (arg->elements[0]);
}

ساكن صحيح
return_type (ffi_type *arg)
{
  بینښه لنډ type;

  إذا (arg->type != FFI_TYPE_STRUCT)
    أرجع arg->type;

  type = simple_type (arg->elements[0]);
  إذا (! arg->elements[1])
    {
      بدل (type)
	{
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	  أرجع FFI_TYPE_INT;

	افتراضي:
	  أرجع type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  إذا (arg->اندازه <= 2 * حجم (صحيح))
    {
      صحيح i = 0;
      ffi_type *e;

      طالما ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  بدل (type)
	    {
	    حالة FFI_TYPE_SINT32:
	    حالة FFI_TYPE_UINT32:
	    حالة FFI_TYPE_INT:
	    حالة FFI_TYPE_FLOAT:
	      أرجع FFI_TYPE_UINT64;

	    افتراضي:
	      اكسر;
	    }
	}
    }

  أرجع FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

فراغ ffi_prep_دلیلونه(حرف *ډېری, extended_cif *ecif)
{
  سجل بینښه صحيح i;
  سجل صحيح tmp;
  سجل بینښه صحيح avn;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;
  صحيح greg, ireg;
#إذا defined(__SH4__)
  صحيح freg = 0;
#د_که_پای

  tmp = 0;
  argp = ډېری;

  إذا (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(فراغ **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_ارزښت_پته_WITH_ARG ? 1 : 0;
    }
  وإلا
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  لكل (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->اندازه;
      إذا (z < حجم(صحيح))
	{
	  إذا (greg++ >= NGREGARG)
	    استمر;

	  z = حجم(صحيح);
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(نښهدار صحيح *) argp = (نښهدار صحيح)*(SINT8 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_UINT8:
	      *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT8 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_SINT16:
	      *(نښهدار صحيح *) argp = (نښهدار صحيح)*(SINT16 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_UINT16:
	      *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT16 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_STRUCT:
	      *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT32 *)(* p_argv);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      وإلا إذا (z == حجم(صحيح))
	{
#إذا defined(__SH4__)
	  إذا ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      إذا (freg++ >= NFREGARG)
		استمر;
	    }
	  وإلا
#د_که_پای
	    {
	      إذا (greg++ >= NGREGARG)
		استمر;
	    }
	  *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#إذا defined(__SH4__)
      وإلا إذا ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  إذا (freg + 1 >= NFREGARG)
	    استمر;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  حافظه_نقل (argp, *p_argv, z);
	  argp += z;
	}
#د_که_پای
      وإلا
	{
	  صحيح n = (z + حجم (صحيح) - 1) / حجم (صحيح);
#إذا defined(__SH4__)
	  إذا (greg + n - 1 >= NGREGARG)
	    استمر;
#وإلا
	  إذا (greg >= NGREGARG)
	    استمر;
#د_که_پای
	  greg += n;
	  حافظه_نقل (argp, *p_argv, z);
	  argp += n * حجم (صحيح);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#إذا defined(__SH4__)
  freg = 0;
#د_که_پای
  p_argv = ecif->avalue;

  لكل (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->اندازه;
      إذا (z < حجم(صحيح))
	{
	  إذا (greg++ < NGREGARG)
	    استمر;

	  z = حجم(صحيح);
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(نښهدار صحيح *) argp = (نښهدار صحيح)*(SINT8 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_UINT8:
	      *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT8 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_SINT16:
	      *(نښهدار صحيح *) argp = (نښهدار صحيح)*(SINT16 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_UINT16:
	      *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT16 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_STRUCT:
	      *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT32 *)(* p_argv);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      وإلا إذا (z == حجم(صحيح))
	{
#إذا defined(__SH4__)
	  إذا ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      إذا (freg++ < NFREGARG)
		استمر;
	    }
	  وإلا
#د_که_پای
	    {
	      إذا (greg++ < NGREGARG)
		استمر;
	    }
	  *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#إذا defined(__SH4__)
      وإلا إذا ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  إذا (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      استمر;
	    }
	  حافظه_نقل (argp, *p_argv, z);
	  argp += z;
	}
#د_که_پای
      وإلا
	{
	  صحيح n = (z + حجم (صحيح) - 1) / حجم (صحيح);
	  إذا (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      استمر;
	    }
#إذا (! defined(__SH4__))
	  وإلا إذا (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      استمر;
	    }
#د_که_پای
	  حافظه_نقل (argp, *p_argv, z);
	  argp += n * حجم (صحيح);
	}
    }

  أرجع;
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  صحيح i, j;
  صحيح اندازه, type;
  صحيح n, m;
  صحيح greg;
#إذا defined(__SH4__)
  صحيح freg = 0;
#د_که_پای

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_ارزښت_پته_WITH_ARG) ? 1 : 0;

#إذا defined(__SH4__)
  لكل (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      بدل (type)
	{
	حالة FFI_TYPE_FLOAT:
	  إذا (freg >= NFREGARG)
	    استمر;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  إذا ((freg + 1) >= NFREGARG)
	    استمر;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  اكسر;
	      
	افتراضي:
	  اندازه = (cif->arg_types)[i]->اندازه;
	  n = (اندازه + حجم (صحيح) - 1) / حجم (صحيح);
	  إذا (greg + n - 1 >= NGREGARG)
		استمر;
	  greg += n;
	  لكل (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  اكسر;
	}
    }
#وإلا
  لكل (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      اندازه = (cif->arg_types)[i]->اندازه;
      n = (اندازه + حجم (صحيح) - 1) / حجم (صحيح);
      إذا (greg >= NGREGARG)
	استمر;
      وإلا إذا (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      لكل (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#د_که_پای

  /* Set the return type flag */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_STRUCT:
      cif->flags += (بینښه) (return_type (cif->rtype)) << 24;
      اكسر;

    حالة FFI_TYPE_VOID:
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags += (بینښه) cif->rtype->type << 24;
      اكسر;

    افتراضي:
      cif->flags += FFI_TYPE_INT << 24;
      اكسر;
    }

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_SYSV(فراغ (*)(حرف *, extended_cif *), extended_cif *,
			  بینښه, بینښه, بینښه *, فراغ (*fn)(فراغ));

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
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
      ecif.rvalue = alloca(cif->rtype->اندازه);
    }
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi) 
    {
    حالة FFI_SYSV:
      ffi_call_SYSV(ffi_prep_دلیلونه, &ecif, cif->بایټونه, cif->flags, ecif.rvalue,
		    fn);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }

  إذا (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    حافظه_نقل (rvalue, &trvalue, cif->rtype->اندازه);
}

خارجي فراغ ffi_تړون_SYSV (فراغ);
#إذا defined(__SH4__)
خارجي فراغ __ic_invalidate (فراغ *کرښه);
#د_که_پای

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون* تړون,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  بینښه صحيح *tramp;
  بینښه صحيح insn;

  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  tramp = (بینښه صحيح *) &تړون->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#که_تعریف_شوی __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#وإلا
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#د_که_پای
  *(فراغ **) &tramp[2] = (فراغ *)codeloc;          /* ctx */
  *(فراغ **) &tramp[3] = (فراغ *)ffi_تړون_SYSV; /* funaddr */

  تړون->cif = cif;
  تړون->fun = fun;
  تړون->user_معلومات = user_معلومات;

#إذا defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#د_که_پای

  أرجع FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

#که_تعریف_شوی __LITTLE_ENDIAN__
#تعریف_کړه OFS_INT8	0
#تعریف_کړه OFS_INT16	0
#وإلا
#تعریف_کړه OFS_INT8	3
#تعریف_کړه OFS_INT16	2
#د_که_پای

صحيح
ffi_تړون_helper_SYSV (ffi_تړون *تړون, فراغ *rvalue, 
			 بینښه اوږد *pgr, بینښه اوږد *pfr, 
			 بینښه اوږد *pst)
{
  فراغ **avalue;
  ffi_type **p_arg;
  صحيح i, avn;
  صحيح ireg, greg = 0;
#إذا defined(__SH4__)
  صحيح freg = 0;
#د_که_پای
  ffi_cif *cif; 

  cif = تړون->cif;
  avalue = alloca(cif->nargs * حجم(فراغ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  إذا (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_ارزښت_پته_WITH_ARG)
    {
      rvalue = (فراغ *) *pgr++;
      ireg = 1;
    }
  وإلا
    ireg = 0;

  cif = تړون->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  لكل (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->اندازه;
      إذا (z < حجم(صحيح))
	{
	  إذا (greg++ >= NGREGARG)
	    استمر;

	  z = حجم(صحيح);
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	    حالة FFI_TYPE_UINT8:
	      avalue[i] = (((حرف *)pgr) + OFS_INT8);
	      اكسر;
  
	    حالة FFI_TYPE_SINT16:
	    حالة FFI_TYPE_UINT16:
	      avalue[i] = (((حرف *)pgr) + OFS_INT16);
	      اكسر;
  
	    حالة FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      وإلا إذا (z == حجم(صحيح))
	{
#إذا defined(__SH4__)
	  إذا ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      إذا (freg++ >= NFREGARG)
		استمر;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وإلا
#د_که_پای
	    {
	      إذا (greg++ >= NGREGARG)
		استمر;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#إذا defined(__SH4__)
      وإلا إذا ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  إذا (freg + 1 >= NFREGARG)
	    استمر;
	  إذا (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#د_که_پای
      وإلا
	{
	  صحيح n = (z + حجم (صحيح) - 1) / حجم (صحيح);
#إذا defined(__SH4__)
	  إذا (greg + n - 1 >= NGREGARG)
	    استمر;
#وإلا
	  إذا (greg >= NGREGARG)
	    استمر;
#د_که_پای
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#إذا defined(__SH4__)
  freg = 0;
#د_که_پای

  لكل (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->اندازه;
      إذا (z < حجم(صحيح))
	{
	  إذا (greg++ < NGREGARG)
	    استمر;

	  z = حجم(صحيح);
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	    حالة FFI_TYPE_UINT8:
	      avalue[i] = (((حرف *)pst) + OFS_INT8);
	      اكسر;
  
	    حالة FFI_TYPE_SINT16:
	    حالة FFI_TYPE_UINT16:
	      avalue[i] = (((حرف *)pst) + OFS_INT16);
	      اكسر;
  
	    حالة FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      وإلا إذا (z == حجم(صحيح))
	{
#إذا defined(__SH4__)
	  إذا ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      إذا (freg++ < NFREGARG)
		استمر;
	    }
	  وإلا
#د_که_پای
	    {
	      إذا (greg++ < NGREGARG)
		استمر;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#إذا defined(__SH4__)
      وإلا إذا ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  إذا (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      استمر;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#د_که_پای
      وإلا
	{
	  صحيح n = (z + حجم (صحيح) - 1) / حجم (صحيح);
	  إذا (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      استمر;
	    }
#إذا (! defined(__SH4__))
	  وإلا إذا (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      استمر;
	    }
#د_که_پای
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (تړون->fun) (cif, rvalue, avalue, تړون->user_معلومات);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  أرجع return_type (cif->rtype);
}
