/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2000, 2007 Software AG
           Copyright (c) 2008 Red Hat, Inc

   S390 Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */
/*====================================================================*/
/*                          Includes                                  */
/*                          --------                                  */
/*====================================================================*/

#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <stdint.h>
#ضمن "internal.h"
#ضمن <tramp.h>

/*====================== End of Includes =============================*/

/*====================================================================*/
/*مقرر_کردا*/
/*                           -------                                  */
/*====================================================================*/

/* Maximum number of GPRs available for argument passing.  */
#تعریف_کرو MAX_GPRARGS 5

/* Maximum number of FPRs available for argument passing.  */
#جے_تعریف_شدہ __s390x__
#تعریف_کرو MAX_FPRARGS 4
#وإلا
#تعریف_کرو MAX_FPRARGS 2
#جے_انت

/* Round to multiple of 16.  */
#تعریف_کرو گیڑا_ناپ(ناپ) (((ناپ) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

بنية call_چوکھٹا
{
  فراغ *پٹھ_chain;
  فراغ *eos;
  بنانشانی لما gpr_دلیلاں[5];
  بنانشانی لما gpr_save[9];
  بنانشانی لما لما fpr_دلیلاں[4];
};

خارجي فراغ FFI_HIDDEN ffi_call_SYSV(بنية call_چوکھٹا *, بنانشانی, فراغ *,
			             فراغ (*fn)(فراغ), فراغ *);

خارجي فراغ ffi_گھیرا_SYSV(فراغ);
خارجي فراغ ffi_go_گھیرا_SYSV(فراغ);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

ساكن صحيح
ffi_check_struct_type (ffi_type *arg)
{
  size_t ناپ = arg->ناپ;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  طالما (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  بدل (ناپ)
    {
      حالة 1:
        أرجع FFI_TYPE_UINT8;

      حالة 2:
        أرجع FFI_TYPE_UINT16;

      حالة 4:
	إذا (arg->type == FFI_TYPE_FLOAT)
          أرجع FFI_TYPE_FLOAT;
	وإلا
	  أرجع FFI_TYPE_UINT32;

      حالة 8:
	إذا (arg->type == FFI_TYPE_DOUBLE)
          أرجع FFI_TYPE_DOUBLE;
	وإلا
	  أرجع FFI_TYPE_UINT64;

      افتراضي:
	اكسر;
    }

  /* Other structs are passed via a pointer to the data.  */
  أرجع FFI_TYPE_اشارہ_گر;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_prep_cif_machdep.                                   */
/*                                                                    */
/* Function - Perform machine dependent CIF processing.               */
/*                                                                    */
/*====================================================================*/

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t struct_ناپ = 0;
  صحيح n_gpr = 0;
  صحيح n_fpr = 0;
  صحيح n_ov = 0;

  ffi_type **ptr;
  صحيح i;

  /* Determine return value handling.  */

  بدل (cif->rtype->type)
    {
      /* Void is easy.  */
      حالة FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	اكسر;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      حالة FFI_TYPE_STRUCT:
      حالة FFI_TYPE_COMPLEX:
      حالة FFI_TYPE_SINT128:
      حالة FFI_TYPE_UINT128:
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      حالة FFI_TYPE_LONGDOUBLE:
#جے_انت
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	اكسر;

      /* Floating point values are returned in fpr 0.  */
      حالة FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	اكسر;

      حالة FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	اكسر;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      حالة FFI_TYPE_UINT64:
      حالة FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	اكسر;

      حالة FFI_TYPE_اشارہ_گر:
      حالة FFI_TYPE_INT:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_UINT16:
      حالة FFI_TYPE_SINT16:
      حالة FFI_TYPE_UINT8:
      حالة FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#جے_تعریف_شدہ __s390x__
	cif->flags = FFI390_RET_INT64;
#وإلا
	cif->flags = FFI390_RET_INT32;
#جے_انت
	اكسر;

      افتراضي:
        FFI_ASSERT (0);
        اكسر;
    }

  /* Now for the arguments.  */

  لكل (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      صحيح type = (*ptr)->type;

      بدل (type)
	{
	حالة FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  إذا (type != FFI_TYPE_اشارہ_گر)
	    اكسر;
	  /* fall through */

	حالة FFI_TYPE_COMPLEX:
	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
#جے_انت
	  type = FFI_TYPE_اشارہ_گر;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_ناپ += گیڑا_ناپ ((*ptr)->ناپ);
	  اكسر;
	}

      /* Now handle all primitive int/float data types.  */
      بدل (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  حالة FFI_TYPE_DOUBLE:
	    إذا (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    وإلا
	      n_ov += حجم (مزدوج) / حجم (لما);
	    اكسر;

	  حالة FFI_TYPE_FLOAT:
	    إذا (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    وإلا
	      n_ov++;
	    اكسر;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#جے_بنا_تعریف __s390x__
	  حالة FFI_TYPE_UINT64:
	  حالة FFI_TYPE_SINT64:
	    إذا (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    إذا (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    وإلا
	      n_ov += 2;
	    اكسر;
#جے_انت

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  افتراضي:
	    إذا (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    وإلا
	      n_ov++;
	    اكسر;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->بائٹاں = گیڑا_ناپ (n_ov * حجم (لما)) + struct_ناپ;

  أرجع FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

ساكن فراغ
ffi_call_int(ffi_cif *cif,
	     فراغ (*fn)(فراغ),
	     فراغ *rvalue,
	     فراغ **avalue,
	     فراغ *گھیرا)
{
  صحيح ret_type = cif->flags;
  size_t rsize = 0, بائٹاں = cif->بائٹاں;
  بنانشانی حرف *ڈھیر, *p_struct;
  بنية call_چوکھٹا *چوکھٹا;
  بنانشانی لما *p_ov, *p_gpr;
  بنانشانی لما لما *p_fpr;
  صحيح n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  إذا (rvalue == عدم)
    {
      إذا (ret_type & FFI390_RET_وچ_MEM)
	rsize = cif->rtype->ناپ;
      وإلا
	ret_type = FFI390_RET_VOID;
    }

  /* The stack space will be filled with those areas:

	dummy structure return		    (highest addresses)
	  FPR argument register save area
	  GPR argument register save area
	stack frame for ffi_call_SYSV
	temporary struct copies
	overflow argument area              (lowest addresses)

     We set up the following pointers:

        p_fpr: bottom of the FPR area (growing upwards)
	p_gpr: bottom of the GPR area (growing upwards)
	p_ov: bottom of the overflow area (growing upwards)
	p_struct: top of the struct copy area (growing downwards)

     All areas are kept aligned to twice the word size.

     Note that we're going to create the stack frame for both
     ffi_call_SYSV _and_ the target function right here.  This
     works because we don't make any function calls with more
     than 5 arguments (indeed only memcpy and ffi_call_SYSV),
     and thus we don't have any stacked outgoing parameters.  */

  ڈھیر = alloca (بائٹاں + حجم(بنية call_چوکھٹا) + rsize);
  چوکھٹا = (بنية call_چوکھٹا *)(ڈھیر + بائٹاں);
  إذا (rsize)
    rvalue = چوکھٹا + 1;

  /* Link the new frame back to the one from this function.  */
  چوکھٹا->پٹھ_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (بنانشانی لما *)ڈھیر;
  p_struct = (بنانشانی حرف *)چوکھٹا;
  p_gpr = چوکھٹا->gpr_دلیلاں;
  p_fpr = چوکھٹا->fpr_دلیلاں;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  إذا (cif->flags & FFI390_RET_وچ_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  لكل (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      فراغ *arg = avalue[i];
      صحيح type = ty->type;
      ffi_arg val;

    restart:
      بدل (type)
	{
	حالة FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_اشارہ_گر:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  اكسر;

	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
#جے_تعریف_شدہ __s390x__
	  val = *(UINT64 *)arg;
	  اذهب do_int;
#وإلا
	  إذا (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  إذا (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  وإلا
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#جے_انت
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  إذا (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  وإلا
	    {
#جے_تعریف_شدہ __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#وإلا
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#جے_انت
	    }
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  إذا (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  وإلا
	    p_ov[n_ov++] = val;
	  اكسر;

	حالة FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  إذا (type != FFI_TYPE_اشارہ_گر)
	    اذهب restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
#جے_انت
	حالة FFI_TYPE_COMPLEX:
	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= گیڑا_ناپ (ty->ناپ);
	  حافظہ_نقل (p_struct, arg, ty->ناپ);
	  val = (uintptr_t)p_struct;
	  اذهب do_int;

	افتراضي:
	  FFI_ASSERT (0);
	  اكسر;
        }
    }

  ffi_call_SYSV (چوکھٹا, ret_type & FFI360_RET_نقاب, rvalue, fn, گھیرا);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گھیرا)
{
  ffi_call_int(cif, fn, rvalue, avalue, گھیرا);
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_closure_helper_SYSV.                                */
/*                                                                    */
/* Function - Call a FFI closure target function.                     */
/*                                                                    */
/*====================================================================*/

فراغ FFI_HIDDEN
ffi_گھیرا_helper_SYSV (ffi_cif *cif,
			 فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
			 فراغ *user_انکڑے,
			 بنانشانی لما *p_gpr,
			 بنانشانی لما لما *p_fpr,
			 بنانشانی لما *p_ov)
{
  بنانشانی لما لما ret_بفر;

  فراغ *rvalue = &ret_بفر;
  فراغ **avalue;
  فراغ **p_arg;

  صحيح n_gpr = 0;
  صحيح n_fpr = 0;
  صحيح n_ov = 0;

  ffi_type **ptr;
  صحيح i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * حجم (فراغ *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  إذا (cif->flags & FFI390_RET_وچ_MEM)
    rvalue = (فراغ *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      صحيح deref_struct_اشارہ_گر = 0;
      صحيح type = (*ptr)->type;

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      إذا (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#جے_انت

      /* Check how a structure type is passed.  */
      إذا (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  إذا (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_اشارہ_گر;
	  وإلا
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  إذا (type == FFI_TYPE_اشارہ_گر)
	    deref_struct_اشارہ_گر = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      إذا (type == FFI_TYPE_اشارہ_گر)
	{
#جے_تعریف_شدہ __s390x__
	  type = FFI_TYPE_UINT64;
#وإلا
	  type = FFI_TYPE_UINT32;
#جے_انت
	}

      /* Now handle all primitive int/float data types.  */
      بدل (type)
	{
	  حالة FFI_TYPE_DOUBLE:
	    إذا (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    وإلا
	      *p_arg = &p_ov[n_ov],
	      n_ov += حجم (مزدوج) / حجم (لما);
	    اكسر;

	  حالة FFI_TYPE_FLOAT:
	    إذا (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    وإلا
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (لما) - 4;
	    اكسر;

	  حالة FFI_TYPE_UINT64:
	  حالة FFI_TYPE_SINT64:
#جے_تعریف_شدہ __s390x__
	    إذا (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    وإلا
	      *p_arg = &p_ov[n_ov++];
#وإلا
	    إذا (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    إذا (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    وإلا
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#جے_انت
	    اكسر;

	  حالة FFI_TYPE_INT:
	  حالة FFI_TYPE_UINT32:
	  حالة FFI_TYPE_SINT32:
	    إذا (n_gpr < MAX_GPRARGS)
	      *p_arg = (حرف *)&p_gpr[n_gpr++] + حجم (لما) - 4;
	    وإلا
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (لما) - 4;
	    اكسر;

	  حالة FFI_TYPE_UINT16:
	  حالة FFI_TYPE_SINT16:
	    إذا (n_gpr < MAX_GPRARGS)
	      *p_arg = (حرف *)&p_gpr[n_gpr++] + حجم (لما) - 2;
	    وإلا
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (لما) - 2;
	    اكسر;

	  حالة FFI_TYPE_UINT8:
	  حالة FFI_TYPE_SINT8:
	    إذا (n_gpr < MAX_GPRARGS)
	      *p_arg = (حرف *)&p_gpr[n_gpr++] + حجم (لما) - 1;
	    وإلا
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (لما) - 1;
	    اكسر;

	  افتراضي:
	    FFI_ASSERT (0);
	    اكسر;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      إذا (deref_struct_اشارہ_گر)
	*p_arg = *(فراغ **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_انکڑے);

  /* Convert the return value.  */
  بدل (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      حالة FFI_TYPE_VOID:
      حالة FFI_TYPE_STRUCT:
      حالة FFI_TYPE_COMPLEX:
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      حالة FFI_TYPE_LONGDOUBLE:
#جے_انت
	اكسر;

      /* Floating point values are returned in fpr 0.  */
      حالة FFI_TYPE_FLOAT:
	p_fpr[0] = (لما لما) *(بنانشانی صحيح *) rvalue << 32;
	اكسر;

      حالة FFI_TYPE_DOUBLE:
	p_fpr[0] = *(بنانشانی لما لما *) rvalue;
	اكسر;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      حالة FFI_TYPE_UINT64:
      حالة FFI_TYPE_SINT64:
#جے_تعریف_شدہ __s390x__
	p_gpr[0] = *(بنانشانی لما *) rvalue;
#وإلا
	p_gpr[0] = ((بنانشانی لما *) rvalue)[0],
	p_gpr[1] = ((بنانشانی لما *) rvalue)[1];
#جے_انت
	اكسر;

      حالة FFI_TYPE_اشارہ_گر:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_UINT16:
      حالة FFI_TYPE_UINT8:
	p_gpr[0] = *(بنانشانی لما *) rvalue;
	اكسر;

      حالة FFI_TYPE_INT:
      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_SINT16:
      حالة FFI_TYPE_SINT8:
	p_gpr[0] = *(نشانیوالا لما *) rvalue;
	اكسر;

      افتراضي:
        FFI_ASSERT (0);
        اكسر;
    }
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_prep_closure_loc.                                   */
/*                                                                    */
/* Function - Prepare a FFI closure.                                  */
/*                                                                    */
/*====================================================================*/

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا *گھیرا,
		      ffi_cif *cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_انکڑے,
		      فراغ *codeloc)
{
  ساكن بنانشانی چھوٹا ثابت سانچا[] = {
    0x0d10,			/* basr %r1,0 */
#جے_بنا_تعریف __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#وإلا
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#جے_انت
    0x07f1			/* br %r1 */
  };
  فراغ (*dest)(فراغ);
  بنانشانی لما *tramp = (بنانشانی لما *)&گھیرا->tramp;

  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;


#إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(گھیرا))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_گھیرا_SYSV;
      ffi_tramp_سیٹ_parms (گھیرا->ftramp, dest, گھیرا);
      اذهب باہرلا;
    }
#جے_انت

  حافظہ_نقل (tramp, سانچا, حجم(سانچا));
  tramp[2] = (بنانشانی لما)codeloc;
  tramp[3] = (بنانشانی لما)&ffi_گھیرا_SYSV;

#إذا defined(FFI_EXEC_STATIC_TRAMP)
باہرلا:
#جے_انت
  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;

  أرجع FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_حالت
ffi_prep_go_گھیرا (ffi_go_گھیرا *گھیرا, ffi_cif *cif,
		     فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*))
{
  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  گھیرا->tramp = ffi_go_گھیرا_SYSV;
  گھیرا->cif = cif;
  گھیرا->fun = fun;

  أرجع FFI_OK;
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_ناپ, size_t *map_ناپ)
{
  خارجي فراغ *trampoline_کوڈ_جدول;

  *tramp_ناپ = FFI390_TRAMP_ناپ;
  *map_ناپ = FFI390_TRAMP_MAP_ناپ;
  أرجع &trampoline_کوڈ_جدول;
}
#جے_انت
