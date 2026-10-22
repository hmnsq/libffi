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

#تعریف_کرو NGREGARG 4
#إذا defined(__SH4__)
#تعریف_کرو NFREGARG 8
#جے_انت

#إذا defined(__HITACHI__)
#تعریف_کرو STRUCT_قدر_پتہ_WITH_ARG 1
#وإلا
#تعریف_کرو STRUCT_قدر_پتہ_WITH_ARG 0
#جے_انت

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
  بنانشانی چھوٹا type;

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
  إذا (arg->ناپ <= 2 * حجم (صحيح))
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

فراغ ffi_prep_دلیلاں(حرف *ڈھیر, extended_cif *ecif)
{
  سجل بنانشانی صحيح i;
  سجل صحيح tmp;
  سجل بنانشانی صحيح avn;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;
  صحيح greg, ireg;
#إذا defined(__SH4__)
  صحيح freg = 0;
#جے_انت

  tmp = 0;
  argp = ڈھیر;

  إذا (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(فراغ **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_قدر_پتہ_WITH_ARG ? 1 : 0;
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

      z = (*p_arg)->ناپ;
      إذا (z < حجم(صحيح))
	{
	  إذا (greg++ >= NGREGARG)
	    استمر;

	  z = حجم(صحيح);
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح)*(SINT8 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_UINT8:
	      *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT8 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_SINT16:
	      *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح)*(SINT16 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_UINT16:
	      *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT16 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_STRUCT:
	      *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT32 *)(* p_argv);
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
#جے_انت
	    {
	      إذا (greg++ >= NGREGARG)
		استمر;
	    }
	  *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#إذا defined(__SH4__)
      وإلا إذا ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  إذا (freg + 1 >= NFREGARG)
	    استمر;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  حافظہ_نقل (argp, *p_argv, z);
	  argp += z;
	}
#جے_انت
      وإلا
	{
	  صحيح n = (z + حجم (صحيح) - 1) / حجم (صحيح);
#إذا defined(__SH4__)
	  إذا (greg + n - 1 >= NGREGARG)
	    استمر;
#وإلا
	  إذا (greg >= NGREGARG)
	    استمر;
#جے_انت
	  greg += n;
	  حافظہ_نقل (argp, *p_argv, z);
	  argp += n * حجم (صحيح);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#إذا defined(__SH4__)
  freg = 0;
#جے_انت
  p_argv = ecif->avalue;

  لكل (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->ناپ;
      إذا (z < حجم(صحيح))
	{
	  إذا (greg++ < NGREGARG)
	    استمر;

	  z = حجم(صحيح);
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح)*(SINT8 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_UINT8:
	      *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT8 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_SINT16:
	      *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح)*(SINT16 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_UINT16:
	      *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT16 *)(* p_argv);
	      اكسر;
  
	    حالة FFI_TYPE_STRUCT:
	      *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT32 *)(* p_argv);
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
#جے_انت
	    {
	      إذا (greg++ < NGREGARG)
		استمر;
	    }
	  *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT32 *)(* p_argv);
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
	  حافظہ_نقل (argp, *p_argv, z);
	  argp += z;
	}
#جے_انت
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
#جے_انت
	  حافظہ_نقل (argp, *p_argv, z);
	  argp += n * حجم (صحيح);
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
#إذا defined(__SH4__)
  صحيح freg = 0;
#جے_انت

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_قدر_پتہ_WITH_ARG) ? 1 : 0;

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
	  ناپ = (cif->arg_types)[i]->ناپ;
	  n = (ناپ + حجم (صحيح) - 1) / حجم (صحيح);
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
      ناپ = (cif->arg_types)[i]->ناپ;
      n = (ناپ + حجم (صحيح) - 1) / حجم (صحيح);
      إذا (greg >= NGREGARG)
	استمر;
      وإلا إذا (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      لكل (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#جے_انت

  /* Set the return type flag */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_STRUCT:
      cif->flags += (بنانشانی) (return_type (cif->rtype)) << 24;
      اكسر;

    حالة FFI_TYPE_VOID:
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags += (بنانشانی) cif->rtype->type << 24;
      اكسر;

    افتراضي:
      cif->flags += FFI_TYPE_INT << 24;
      اكسر;
    }

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_SYSV(فراغ (*)(حرف *, extended_cif *), extended_cif *,
			  بنانشانی, بنانشانی, بنانشانی *, فراغ (*fn)(فراغ));

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
      ecif.rvalue = alloca(cif->rtype->ناپ);
    }
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi) 
    {
    حالة FFI_SYSV:
      ffi_call_SYSV(ffi_prep_دلیلاں, &ecif, cif->بائٹاں, cif->flags, ecif.rvalue,
		    fn);
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
#إذا defined(__SH4__)
خارجي فراغ __ic_invalidate (فراغ *سطر);
#جے_انت

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا* گھیرا,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_انکڑے,
		      فراغ *codeloc)
{
  بنانشانی صحيح *tramp;
  بنانشانی صحيح insn;

  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  tramp = (بنانشانی صحيح *) &گھیرا->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#جے_تعریف_شدہ __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#وإلا
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#جے_انت
  *(فراغ **) &tramp[2] = (فراغ *)codeloc;          /* ctx */
  *(فراغ **) &tramp[3] = (فراغ *)ffi_گھیرا_SYSV; /* funaddr */

  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;

#إذا defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#جے_انت

  أرجع FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

#جے_تعریف_شدہ __LITTLE_ENDIAN__
#تعریف_کرو OFS_INT8	0
#تعریف_کرو OFS_INT16	0
#وإلا
#تعریف_کرو OFS_INT8	3
#تعریف_کرو OFS_INT16	2
#جے_انت

صحيح
ffi_گھیرا_helper_SYSV (ffi_گھیرا *گھیرا, فراغ *rvalue, 
			 بنانشانی لما *pgr, بنانشانی لما *pfr, 
			 بنانشانی لما *pst)
{
  فراغ **avalue;
  ffi_type **p_arg;
  صحيح i, avn;
  صحيح ireg, greg = 0;
#إذا defined(__SH4__)
  صحيح freg = 0;
#جے_انت
  ffi_cif *cif; 

  cif = گھیرا->cif;
  avalue = alloca(cif->nargs * حجم(فراغ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  إذا (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_قدر_پتہ_WITH_ARG)
    {
      rvalue = (فراغ *) *pgr++;
      ireg = 1;
    }
  وإلا
    ireg = 0;

  cif = گھیرا->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  لكل (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ناپ;
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
#جے_انت
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
#جے_انت
      وإلا
	{
	  صحيح n = (z + حجم (صحيح) - 1) / حجم (صحيح);
#إذا defined(__SH4__)
	  إذا (greg + n - 1 >= NGREGARG)
	    استمر;
#وإلا
	  إذا (greg >= NGREGARG)
	    استمر;
#جے_انت
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#إذا defined(__SH4__)
  freg = 0;
#جے_انت

  لكل (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ناپ;
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
#جے_انت
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
#جے_انت
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
#جے_انت
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (گھیرا->fun) (cif, rvalue, avalue, گھیرا->user_انکڑے);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  أرجع return_type (cif->rtype);
}
