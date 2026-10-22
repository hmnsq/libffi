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

#تعریف_کرو NGREGARG 4
#اگر defined(__SH4__)
#تعریف_کرو NFREGARG 8
#اگر_ختم

#اگر defined(__HITACHI__)
#تعریف_کرو STRUCT_قدر_پتہ_WITH_ARG 1
#ورنہ
#تعریف_کرو STRUCT_قدر_پتہ_WITH_ARG 0
#اگر_ختم

/* If the structure has essentially an unique element, return its type.  */
ساکن صحیح
simple_type (ffi_type *arg)
{
  اگر (arg->type != FFI_TYPE_STRUCT)
    واپس arg->type;
  ورنہ اگر (arg->elements[1])
    واپس FFI_TYPE_STRUCT;

  واپس simple_type (arg->elements[0]);
}

ساکن صحیح
return_type (ffi_type *arg)
{
  غیرعلامتی مختصر type;

  اگر (arg->type != FFI_TYPE_STRUCT)
    واپس arg->type;

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
	  واپس FFI_TYPE_INT;

	طےشدہ:
	  واپس type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  اگر (arg->سائز <= 2 * حجم (صحیح))
    {
      صحیح i = 0;
      ffi_type *e;

      جب ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  انتخاب (type)
	    {
	    حالت FFI_TYPE_SINT32:
	    حالت FFI_TYPE_UINT32:
	    حالت FFI_TYPE_INT:
	    حالت FFI_TYPE_FLOAT:
	      واپس FFI_TYPE_UINT64;

	    طےشدہ:
	      توڑو;
	    }
	}
    }

  واپس FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

خالی ffi_prep_دلائل(حرف *ڈھیر, extended_cif *ecif)
{
  اندراج غیرعلامتی صحیح i;
  اندراج صحیح tmp;
  اندراج غیرعلامتی صحیح avn;
  اندراج خالی **p_argv;
  اندراج حرف *argp;
  اندراج ffi_type **p_arg;
  صحیح greg, ireg;
#اگر defined(__SH4__)
  صحیح freg = 0;
#اگر_ختم

  tmp = 0;
  argp = ڈھیر;

  اگر (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(خالی **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_قدر_پتہ_WITH_ARG ? 1 : 0;
    }
  ورنہ
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  برائے (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->سائز;
      اگر (z < حجم(صحیح))
	{
	  اگر (greg++ >= NGREGARG)
	    جاری;

	  z = حجم(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(علامتی صحیح *) argp = (علامتی صحیح)*(SINT8 *)(* p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_UINT8:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT8 *)(* p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_SINT16:
	      *(علامتی صحیح *) argp = (علامتی صحیح)*(SINT16 *)(* p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_UINT16:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT16 *)(* p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_STRUCT:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT32 *)(* p_argv);
	      توڑو;

	    طےشدہ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      ورنہ اگر (z == حجم(صحیح))
	{
#اگر defined(__SH4__)
	  اگر ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      اگر (freg++ >= NFREGARG)
		جاری;
	    }
	  ورنہ
#اگر_ختم
	    {
	      اگر (greg++ >= NGREGARG)
		جاری;
	    }
	  *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#اگر defined(__SH4__)
      ورنہ اگر ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  اگر (freg + 1 >= NFREGARG)
	    جاری;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  حافظہ_نقل (argp, *p_argv, z);
	  argp += z;
	}
#اگر_ختم
      ورنہ
	{
	  صحیح n = (z + حجم (صحیح) - 1) / حجم (صحیح);
#اگر defined(__SH4__)
	  اگر (greg + n - 1 >= NGREGARG)
	    جاری;
#ورنہ
	  اگر (greg >= NGREGARG)
	    جاری;
#اگر_ختم
	  greg += n;
	  حافظہ_نقل (argp, *p_argv, z);
	  argp += n * حجم (صحیح);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#اگر defined(__SH4__)
  freg = 0;
#اگر_ختم
  p_argv = ecif->avalue;

  برائے (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->سائز;
      اگر (z < حجم(صحیح))
	{
	  اگر (greg++ < NGREGARG)
	    جاری;

	  z = حجم(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(علامتی صحیح *) argp = (علامتی صحیح)*(SINT8 *)(* p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_UINT8:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT8 *)(* p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_SINT16:
	      *(علامتی صحیح *) argp = (علامتی صحیح)*(SINT16 *)(* p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_UINT16:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT16 *)(* p_argv);
	      توڑو;
  
	    حالت FFI_TYPE_STRUCT:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT32 *)(* p_argv);
	      توڑو;

	    طےشدہ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      ورنہ اگر (z == حجم(صحیح))
	{
#اگر defined(__SH4__)
	  اگر ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      اگر (freg++ < NFREGARG)
		جاری;
	    }
	  ورنہ
#اگر_ختم
	    {
	      اگر (greg++ < NGREGARG)
		جاری;
	    }
	  *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#اگر defined(__SH4__)
      ورنہ اگر ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  اگر (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      جاری;
	    }
	  حافظہ_نقل (argp, *p_argv, z);
	  argp += z;
	}
#اگر_ختم
      ورنہ
	{
	  صحیح n = (z + حجم (صحیح) - 1) / حجم (صحیح);
	  اگر (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      جاری;
	    }
#اگر (! defined(__SH4__))
	  ورنہ اگر (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      جاری;
	    }
#اگر_ختم
	  حافظہ_نقل (argp, *p_argv, z);
	  argp += n * حجم (صحیح);
	}
    }

  واپس;
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  صحیح i, j;
  صحیح سائز, type;
  صحیح n, m;
  صحیح greg;
#اگر defined(__SH4__)
  صحیح freg = 0;
#اگر_ختم

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_قدر_پتہ_WITH_ARG) ? 1 : 0;

#اگر defined(__SH4__)
  برائے (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      انتخاب (type)
	{
	حالت FFI_TYPE_FLOAT:
	  اگر (freg >= NFREGARG)
	    جاری;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	  اگر ((freg + 1) >= NFREGARG)
	    جاری;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  توڑو;
	      
	طےشدہ:
	  سائز = (cif->arg_types)[i]->سائز;
	  n = (سائز + حجم (صحیح) - 1) / حجم (صحیح);
	  اگر (greg + n - 1 >= NGREGARG)
		جاری;
	  greg += n;
	  برائے (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  توڑو;
	}
    }
#ورنہ
  برائے (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      سائز = (cif->arg_types)[i]->سائز;
      n = (سائز + حجم (صحیح) - 1) / حجم (صحیح);
      اگر (greg >= NGREGARG)
	جاری;
      ورنہ اگر (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      برائے (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#اگر_ختم

  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_STRUCT:
      cif->flags += (غیرعلامتی) (return_type (cif->rtype)) << 24;
      توڑو;

    حالت FFI_TYPE_VOID:
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags += (غیرعلامتی) cif->rtype->type << 24;
      توڑو;

    طےشدہ:
      cif->flags += FFI_TYPE_INT << 24;
      توڑو;
    }

  واپس FFI_OK;
}

خارجی خالی ffi_call_SYSV(خالی (*)(حرف *, extended_cif *), extended_cif *,
			  غیرعلامتی, غیرعلامتی, غیرعلامتی *, خالی (*fn)(خالی));

خالی ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
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
  ورنہ اگر ((rvalue == تہی) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->سائز);
    }
  ورنہ
    ecif.rvalue = rvalue;

  انتخاب (cif->abi) 
    {
    حالت FFI_SYSV:
      ffi_call_SYSV(ffi_prep_دلائل, &ecif, cif->بائٹیں, cif->flags, ecif.rvalue,
		    fn);
      توڑو;
    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }

  اگر (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    حافظہ_نقل (rvalue, &trvalue, cif->rtype->سائز);
}

خارجی خالی ffi_احاطہ_SYSV (خالی);
#اگر defined(__SH4__)
خارجی خالی __ic_invalidate (خالی *سطر);
#اگر_ختم

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  غیرعلامتی صحیح *tramp;
  غیرعلامتی صحیح insn;

  اگر (cif->abi != FFI_SYSV)
    واپس FFI_BAD_ABI;

  tramp = (غیرعلامتی صحیح *) &احاطہ->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#اگر_تعریف_شدہ __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#ورنہ
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#اگر_ختم
  *(خالی **) &tramp[2] = (خالی *)codeloc;          /* ctx */
  *(خالی **) &tramp[3] = (خالی *)ffi_احاطہ_SYSV; /* funaddr */

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

#اگر defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#اگر_ختم

  واپس FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

#اگر_تعریف_شدہ __LITTLE_ENDIAN__
#تعریف_کرو OFS_INT8	0
#تعریف_کرو OFS_INT16	0
#ورنہ
#تعریف_کرو OFS_INT8	3
#تعریف_کرو OFS_INT16	2
#اگر_ختم

صحیح
ffi_احاطہ_helper_SYSV (ffi_احاطہ *احاطہ, خالی *rvalue, 
			 غیرعلامتی طویل *pgr, غیرعلامتی طویل *pfr, 
			 غیرعلامتی طویل *pst)
{
  خالی **avalue;
  ffi_type **p_arg;
  صحیح i, avn;
  صحیح ireg, greg = 0;
#اگر defined(__SH4__)
  صحیح freg = 0;
#اگر_ختم
  ffi_cif *cif; 

  cif = احاطہ->cif;
  avalue = alloca(cif->nargs * حجم(خالی *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  اگر (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_قدر_پتہ_WITH_ARG)
    {
      rvalue = (خالی *) *pgr++;
      ireg = 1;
    }
  ورنہ
    ireg = 0;

  cif = احاطہ->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  برائے (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->سائز;
      اگر (z < حجم(صحیح))
	{
	  اگر (greg++ >= NGREGARG)
	    جاری;

	  z = حجم(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	    حالت FFI_TYPE_UINT8:
	      avalue[i] = (((حرف *)pgr) + OFS_INT8);
	      توڑو;
  
	    حالت FFI_TYPE_SINT16:
	    حالت FFI_TYPE_UINT16:
	      avalue[i] = (((حرف *)pgr) + OFS_INT16);
	      توڑو;
  
	    حالت FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      توڑو;

	    طےشدہ:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      ورنہ اگر (z == حجم(صحیح))
	{
#اگر defined(__SH4__)
	  اگر ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      اگر (freg++ >= NFREGARG)
		جاری;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ورنہ
#اگر_ختم
	    {
	      اگر (greg++ >= NGREGARG)
		جاری;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#اگر defined(__SH4__)
      ورنہ اگر ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  اگر (freg + 1 >= NFREGARG)
	    جاری;
	  اگر (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#اگر_ختم
      ورنہ
	{
	  صحیح n = (z + حجم (صحیح) - 1) / حجم (صحیح);
#اگر defined(__SH4__)
	  اگر (greg + n - 1 >= NGREGARG)
	    جاری;
#ورنہ
	  اگر (greg >= NGREGARG)
	    جاری;
#اگر_ختم
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#اگر defined(__SH4__)
  freg = 0;
#اگر_ختم

  برائے (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->سائز;
      اگر (z < حجم(صحیح))
	{
	  اگر (greg++ < NGREGARG)
	    جاری;

	  z = حجم(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	    حالت FFI_TYPE_UINT8:
	      avalue[i] = (((حرف *)pst) + OFS_INT8);
	      توڑو;
  
	    حالت FFI_TYPE_SINT16:
	    حالت FFI_TYPE_UINT16:
	      avalue[i] = (((حرف *)pst) + OFS_INT16);
	      توڑو;
  
	    حالت FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      توڑو;

	    طےشدہ:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      ورنہ اگر (z == حجم(صحیح))
	{
#اگر defined(__SH4__)
	  اگر ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      اگر (freg++ < NFREGARG)
		جاری;
	    }
	  ورنہ
#اگر_ختم
	    {
	      اگر (greg++ < NGREGARG)
		جاری;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#اگر defined(__SH4__)
      ورنہ اگر ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  اگر (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      جاری;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#اگر_ختم
      ورنہ
	{
	  صحیح n = (z + حجم (صحیح) - 1) / حجم (صحیح);
	  اگر (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      جاری;
	    }
#اگر (! defined(__SH4__))
	  ورنہ اگر (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      جاری;
	    }
#اگر_ختم
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (احاطہ->fun) (cif, rvalue, avalue, احاطہ->user_ڈیٹا);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  واپس return_type (cif->rtype);
}
