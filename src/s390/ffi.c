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

#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdint.h>
#شامل "internal.h"
#شامل <tramp.h>

/*====================== End of Includes =============================*/

/*====================================================================*/
/*متعین_کرتا*/
/*                           -------                                  */
/*====================================================================*/

/* Maximum number of GPRs available for argument passing.  */
#تعریف_کرو MAX_GPRARGS 5

/* Maximum number of FPRs available for argument passing.  */
#اگر_تعریف_شدہ __s390x__
#تعریف_کرو MAX_FPRARGS 4
#ورنہ
#تعریف_کرو MAX_FPRARGS 2
#اگر_ختم

/* Round to multiple of 16.  */
#تعریف_کرو باری_سائز(سائز) (((سائز) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

ساخت call_چوکھٹا
{
  خالی *عقب_chain;
  خالی *eos;
  غیرعلامتی طویل gpr_دلائل[5];
  غیرعلامتی طویل gpr_save[9];
  غیرعلامتی طویل طویل fpr_دلائل[4];
};

خارجی خالی FFI_HIDDEN ffi_call_SYSV(ساخت call_چوکھٹا *, غیرعلامتی, خالی *,
			             خالی (*fn)(خالی), خالی *);

خارجی خالی ffi_احاطہ_SYSV(خالی);
خارجی خالی ffi_go_احاطہ_SYSV(خالی);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

ساکن صحیح
ffi_check_struct_type (ffi_type *arg)
{
  size_t سائز = arg->سائز;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  جب (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  انتخاب (سائز)
    {
      حالت 1:
        واپس FFI_TYPE_UINT8;

      حالت 2:
        واپس FFI_TYPE_UINT16;

      حالت 4:
	اگر (arg->type == FFI_TYPE_FLOAT)
          واپس FFI_TYPE_FLOAT;
	ورنہ
	  واپس FFI_TYPE_UINT32;

      حالت 8:
	اگر (arg->type == FFI_TYPE_DOUBLE)
          واپس FFI_TYPE_DOUBLE;
	ورنہ
	  واپس FFI_TYPE_UINT64;

      طےشدہ:
	توڑو;
    }

  /* Other structs are passed via a pointer to the data.  */
  واپس FFI_TYPE_اشارہ_گر;
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
  size_t struct_سائز = 0;
  صحیح n_gpr = 0;
  صحیح n_fpr = 0;
  صحیح n_ov = 0;

  ffi_type **ptr;
  صحیح i;

  /* Determine return value handling.  */

  انتخاب (cif->rtype->type)
    {
      /* Void is easy.  */
      حالت FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	توڑو;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      حالت FFI_TYPE_STRUCT:
      حالت FFI_TYPE_COMPLEX:
      حالت FFI_TYPE_SINT128:
      حالت FFI_TYPE_UINT128:
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      حالت FFI_TYPE_LONGDOUBLE:
#اگر_ختم
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	توڑو;

      /* Floating point values are returned in fpr 0.  */
      حالت FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	توڑو;

      حالت FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	توڑو;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      حالت FFI_TYPE_UINT64:
      حالت FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	توڑو;

      حالت FFI_TYPE_اشارہ_گر:
      حالت FFI_TYPE_INT:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_UINT16:
      حالت FFI_TYPE_SINT16:
      حالت FFI_TYPE_UINT8:
      حالت FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#اگر_تعریف_شدہ __s390x__
	cif->flags = FFI390_RET_INT64;
#ورنہ
	cif->flags = FFI390_RET_INT32;
#اگر_ختم
	توڑو;

      طےشدہ:
        FFI_ASSERT (0);
        توڑو;
    }

  /* Now for the arguments.  */

  برائے (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      صحیح type = (*ptr)->type;

      انتخاب (type)
	{
	حالت FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  اگر (type != FFI_TYPE_اشارہ_گر)
	    توڑو;
	  /* fall through */

	حالت FFI_TYPE_COMPLEX:
	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
#اگر_ختم
	  type = FFI_TYPE_اشارہ_گر;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_سائز += باری_سائز ((*ptr)->سائز);
	  توڑو;
	}

      /* Now handle all primitive int/float data types.  */
      انتخاب (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  حالت FFI_TYPE_DOUBLE:
	    اگر (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ورنہ
	      n_ov += حجم (دوہرا) / حجم (طویل);
	    توڑو;

	  حالت FFI_TYPE_FLOAT:
	    اگر (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ورنہ
	      n_ov++;
	    توڑو;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#اگر_غیر_تعریف_شدہ __s390x__
	  حالت FFI_TYPE_UINT64:
	  حالت FFI_TYPE_SINT64:
	    اگر (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    اگر (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    ورنہ
	      n_ov += 2;
	    توڑو;
#اگر_ختم

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  طےشدہ:
	    اگر (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    ورنہ
	      n_ov++;
	    توڑو;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->بائٹیں = باری_سائز (n_ov * حجم (طویل)) + struct_سائز;

  واپس FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

ساکن خالی
ffi_call_int(ffi_cif *cif,
	     خالی (*fn)(خالی),
	     خالی *rvalue,
	     خالی **avalue,
	     خالی *احاطہ)
{
  صحیح ret_type = cif->flags;
  size_t rsize = 0, بائٹیں = cif->بائٹیں;
  غیرعلامتی حرف *ڈھیر, *p_struct;
  ساخت call_چوکھٹا *چوکھٹا;
  غیرعلامتی طویل *p_ov, *p_gpr;
  غیرعلامتی طویل طویل *p_fpr;
  صحیح n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  اگر (rvalue == تہی)
    {
      اگر (ret_type & FFI390_RET_اندر_MEM)
	rsize = cif->rtype->سائز;
      ورنہ
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

  ڈھیر = alloca (بائٹیں + حجم(ساخت call_چوکھٹا) + rsize);
  چوکھٹا = (ساخت call_چوکھٹا *)(ڈھیر + بائٹیں);
  اگر (rsize)
    rvalue = چوکھٹا + 1;

  /* Link the new frame back to the one from this function.  */
  چوکھٹا->عقب_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (غیرعلامتی طویل *)ڈھیر;
  p_struct = (غیرعلامتی حرف *)چوکھٹا;
  p_gpr = چوکھٹا->gpr_دلائل;
  p_fpr = چوکھٹا->fpr_دلائل;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  اگر (cif->flags & FFI390_RET_اندر_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  برائے (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      خالی *arg = avalue[i];
      صحیح type = ty->type;
      ffi_arg val;

    restart:
      انتخاب (type)
	{
	حالت FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  جاؤ do_int;
	حالت FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  جاؤ do_int;
	حالت FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  جاؤ do_int;
	حالت FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  جاؤ do_int;
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  جاؤ do_int;
	حالت FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  جاؤ do_int;
	حالت FFI_TYPE_اشارہ_گر:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  توڑو;

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
#اگر_تعریف_شدہ __s390x__
	  val = *(UINT64 *)arg;
	  جاؤ do_int;
#ورنہ
	  اگر (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  اگر (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  ورنہ
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	  اگر (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  ورنہ
	    {
#اگر_تعریف_شدہ __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#ورنہ
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#اگر_ختم
	    }
	  توڑو;

	حالت FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  اگر (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  ورنہ
	    p_ov[n_ov++] = val;
	  توڑو;

	حالت FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  اگر (type != FFI_TYPE_اشارہ_گر)
	    جاؤ restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
#اگر_ختم
	حالت FFI_TYPE_COMPLEX:
	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= باری_سائز (ty->سائز);
	  حافظہ_نقل (p_struct, arg, ty->سائز);
	  val = (uintptr_t)p_struct;
	  جاؤ do_int;

	طےشدہ:
	  FFI_ASSERT (0);
	  توڑو;
        }
    }

  ffi_call_SYSV (چوکھٹا, ret_type & FFI360_RET_نقاب, rvalue, fn, احاطہ);
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, تہی);
}

خالی
ffi_call_go (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  ffi_call_int(cif, fn, rvalue, avalue, احاطہ);
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_closure_helper_SYSV.                                */
/*                                                                    */
/* Function - Call a FFI closure target function.                     */
/*                                                                    */
/*====================================================================*/

خالی FFI_HIDDEN
ffi_احاطہ_helper_SYSV (ffi_cif *cif,
			 خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
			 خالی *user_ڈیٹا,
			 غیرعلامتی طویل *p_gpr,
			 غیرعلامتی طویل طویل *p_fpr,
			 غیرعلامتی طویل *p_ov)
{
  غیرعلامتی طویل طویل ret_بفر;

  خالی *rvalue = &ret_بفر;
  خالی **avalue;
  خالی **p_arg;

  صحیح n_gpr = 0;
  صحیح n_fpr = 0;
  صحیح n_ov = 0;

  ffi_type **ptr;
  صحیح i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * حجم (خالی *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  اگر (cif->flags & FFI390_RET_اندر_MEM)
    rvalue = (خالی *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  برائے (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      صحیح deref_struct_اشارہ_گر = 0;
      صحیح type = (*ptr)->type;

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      اگر (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#اگر_ختم

      /* Check how a structure type is passed.  */
      اگر (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  اگر (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_اشارہ_گر;
	  ورنہ
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  اگر (type == FFI_TYPE_اشارہ_گر)
	    deref_struct_اشارہ_گر = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      اگر (type == FFI_TYPE_اشارہ_گر)
	{
#اگر_تعریف_شدہ __s390x__
	  type = FFI_TYPE_UINT64;
#ورنہ
	  type = FFI_TYPE_UINT32;
#اگر_ختم
	}

      /* Now handle all primitive int/float data types.  */
      انتخاب (type)
	{
	  حالت FFI_TYPE_DOUBLE:
	    اگر (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ورنہ
	      *p_arg = &p_ov[n_ov],
	      n_ov += حجم (دوہرا) / حجم (طویل);
	    توڑو;

	  حالت FFI_TYPE_FLOAT:
	    اگر (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ورنہ
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (طویل) - 4;
	    توڑو;

	  حالت FFI_TYPE_UINT64:
	  حالت FFI_TYPE_SINT64:
#اگر_تعریف_شدہ __s390x__
	    اگر (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    ورنہ
	      *p_arg = &p_ov[n_ov++];
#ورنہ
	    اگر (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    اگر (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    ورنہ
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#اگر_ختم
	    توڑو;

	  حالت FFI_TYPE_INT:
	  حالت FFI_TYPE_UINT32:
	  حالت FFI_TYPE_SINT32:
	    اگر (n_gpr < MAX_GPRARGS)
	      *p_arg = (حرف *)&p_gpr[n_gpr++] + حجم (طویل) - 4;
	    ورنہ
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (طویل) - 4;
	    توڑو;

	  حالت FFI_TYPE_UINT16:
	  حالت FFI_TYPE_SINT16:
	    اگر (n_gpr < MAX_GPRARGS)
	      *p_arg = (حرف *)&p_gpr[n_gpr++] + حجم (طویل) - 2;
	    ورنہ
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (طویل) - 2;
	    توڑو;

	  حالت FFI_TYPE_UINT8:
	  حالت FFI_TYPE_SINT8:
	    اگر (n_gpr < MAX_GPRARGS)
	      *p_arg = (حرف *)&p_gpr[n_gpr++] + حجم (طویل) - 1;
	    ورنہ
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (طویل) - 1;
	    توڑو;

	  طےشدہ:
	    FFI_ASSERT (0);
	    توڑو;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      اگر (deref_struct_اشارہ_گر)
	*p_arg = *(خالی **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_ڈیٹا);

  /* Convert the return value.  */
  انتخاب (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      حالت FFI_TYPE_VOID:
      حالت FFI_TYPE_STRUCT:
      حالت FFI_TYPE_COMPLEX:
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      حالت FFI_TYPE_LONGDOUBLE:
#اگر_ختم
	توڑو;

      /* Floating point values are returned in fpr 0.  */
      حالت FFI_TYPE_FLOAT:
	p_fpr[0] = (طویل طویل) *(غیرعلامتی صحیح *) rvalue << 32;
	توڑو;

      حالت FFI_TYPE_DOUBLE:
	p_fpr[0] = *(غیرعلامتی طویل طویل *) rvalue;
	توڑو;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      حالت FFI_TYPE_UINT64:
      حالت FFI_TYPE_SINT64:
#اگر_تعریف_شدہ __s390x__
	p_gpr[0] = *(غیرعلامتی طویل *) rvalue;
#ورنہ
	p_gpr[0] = ((غیرعلامتی طویل *) rvalue)[0],
	p_gpr[1] = ((غیرعلامتی طویل *) rvalue)[1];
#اگر_ختم
	توڑو;

      حالت FFI_TYPE_اشارہ_گر:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_UINT16:
      حالت FFI_TYPE_UINT8:
	p_gpr[0] = *(غیرعلامتی طویل *) rvalue;
	توڑو;

      حالت FFI_TYPE_INT:
      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_SINT16:
      حالت FFI_TYPE_SINT8:
	p_gpr[0] = *(علامتی طویل *) rvalue;
	توڑو;

      طےشدہ:
        FFI_ASSERT (0);
        توڑو;
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
ffi_prep_احاطہ_loc (ffi_احاطہ *احاطہ,
		      ffi_cif *cif,
		      خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  ساکن غیرعلامتی مختصر ثابت سانچہ[] = {
    0x0d10,			/* basr %r1,0 */
#اگر_غیر_تعریف_شدہ __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#ورنہ
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#اگر_ختم
    0x07f1			/* br %r1 */
  };
  خالی (*dest)(خالی);
  غیرعلامتی طویل *tramp = (غیرعلامتی طویل *)&احاطہ->tramp;

  اگر (cif->abi != FFI_SYSV)
    واپس FFI_BAD_ABI;


#اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(احاطہ))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_احاطہ_SYSV;
      ffi_tramp_سیٹ_parms (احاطہ->ftramp, dest, احاطہ);
      جاؤ برآمد;
    }
#اگر_ختم

  حافظہ_نقل (tramp, سانچہ, حجم(سانچہ));
  tramp[2] = (غیرعلامتی طویل)codeloc;
  tramp[3] = (غیرعلامتی طویل)&ffi_احاطہ_SYSV;

#اگر defined(FFI_EXEC_STATIC_TRAMP)
برآمد:
#اگر_ختم
  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  واپس FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ *احاطہ, ffi_cif *cif,
		     خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*))
{
  اگر (cif->abi != FFI_SYSV)
    واپس FFI_BAD_ABI;

  احاطہ->tramp = ffi_go_احاطہ_SYSV;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}

#اگر defined(FFI_EXEC_STATIC_TRAMP)
خالی *
ffi_tramp_arch (size_t *tramp_سائز, size_t *map_سائز)
{
  خارجی خالی *trampoline_رمز_جدول;

  *tramp_سائز = FFI390_TRAMP_سائز;
  *map_سائز = FFI390_TRAMP_MAP_سائز;
  واپس &trampoline_رمز_جدول;
}
#اگر_ختم
