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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ "internal.h"
#అన్తర్భావ <tramp.h>

/*====================== End of Includes =============================*/

/*====================================================================*/
/*నిర్వచిస్తుంది*/
/*                           -------                                  */
/*====================================================================*/

/* Maximum number of GPRs available for argument passing.  */
#నిర్వచించు MAX_GPRARGS 5

/* Maximum number of FPRs available for argument passing.  */
#నిర్వచితమైతే __s390x__
#నిర్వచించు MAX_FPRARGS 4
#లేకపోతే
#నిర్వచించు MAX_FPRARGS 2
#అయితే_ముగింపు

/* Round to multiple of 16.  */
#నిర్వచించు విడత_కొలత(కొలత) (((కొలత) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

నిర్మాణం call_చట్రం
{
  శూన్య *వెనుక_chain;
  శూన్య *eos;
  గుర్తులేని దీర్ఘ gpr_వాదనలు[5];
  గుర్తులేని దీర్ఘ gpr_save[9];
  గుర్తులేని దీర్ఘ దీర్ఘ fpr_వాదనలు[4];
};

బాహ్య శూన్య FFI_HIDDEN ffi_call_SYSV(నిర్మాణం call_చట్రం *, గుర్తులేని, శూన్య *,
			             శూన్య (*fn)(శూన్య), శూన్య *);

బాహ్య శూన్య ffi_సంవృతం_SYSV(శూన్య);
బాహ్య శూన్య ffi_go_సంవృతం_SYSV(శూన్య);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

స్థిర పూర్ణాంకం
ffi_check_struct_type (ffi_type *arg)
{
  size_t కొలత = arg->కొలత;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  అయితే (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  ఎంపిక (కొలత)
    {
      స్థితి 1:
        తిరిగి FFI_TYPE_UINT8;

      స్థితి 2:
        తిరిగి FFI_TYPE_UINT16;

      స్థితి 4:
	ఒకవేళ (arg->type == FFI_TYPE_FLOAT)
          తిరిగి FFI_TYPE_FLOAT;
	లేకపోతే
	  తిరిగి FFI_TYPE_UINT32;

      స్థితి 8:
	ఒకవేళ (arg->type == FFI_TYPE_DOUBLE)
          తిరిగి FFI_TYPE_DOUBLE;
	లేకపోతే
	  తిరిగి FFI_TYPE_UINT64;

      అప్రమేయ:
	ఆపు;
    }

  /* Other structs are passed via a pointer to the data.  */
  తిరిగి FFI_TYPE_సూచిక;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_prep_cif_machdep.                                   */
/*                                                                    */
/* Function - Perform machine dependent CIF processing.               */
/*                                                                    */
/*====================================================================*/

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t struct_కొలత = 0;
  పూర్ణాంకం n_gpr = 0;
  పూర్ణాంకం n_fpr = 0;
  పూర్ణాంకం n_ov = 0;

  ffi_type **ptr;
  పూర్ణాంకం i;

  /* Determine return value handling.  */

  ఎంపిక (cif->rtype->type)
    {
      /* Void is easy.  */
      స్థితి FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	ఆపు;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      స్థితి FFI_TYPE_STRUCT:
      స్థితి FFI_TYPE_COMPLEX:
      స్థితి FFI_TYPE_SINT128:
      స్థితి FFI_TYPE_UINT128:
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      స్థితి FFI_TYPE_LONGDOUBLE:
#అయితే_ముగింపు
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	ఆపు;

      /* Floating point values are returned in fpr 0.  */
      స్థితి FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	ఆపు;

      స్థితి FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	ఆపు;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      స్థితి FFI_TYPE_UINT64:
      స్థితి FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	ఆపు;

      స్థితి FFI_TYPE_సూచిక:
      స్థితి FFI_TYPE_INT:
      స్థితి FFI_TYPE_UINT32:
      స్థితి FFI_TYPE_SINT32:
      స్థితి FFI_TYPE_UINT16:
      స్థితి FFI_TYPE_SINT16:
      స్థితి FFI_TYPE_UINT8:
      స్థితి FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#నిర్వచితమైతే __s390x__
	cif->flags = FFI390_RET_INT64;
#లేకపోతే
	cif->flags = FFI390_RET_INT32;
#అయితే_ముగింపు
	ఆపు;

      అప్రమేయ:
        FFI_ASSERT (0);
        ఆపు;
    }

  /* Now for the arguments.  */

  కొరకు (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      పూర్ణాంకం type = (*ptr)->type;

      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  ఒకవేళ (type != FFI_TYPE_సూచిక)
	    ఆపు;
	  /* fall through */

	స్థితి FFI_TYPE_COMPLEX:
	స్థితి FFI_TYPE_SINT128:
	స్థితి FFI_TYPE_UINT128:
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	స్థితి FFI_TYPE_LONGDOUBLE:
#అయితే_ముగింపు
	  type = FFI_TYPE_సూచిక;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_కొలత += విడత_కొలత ((*ptr)->కొలత);
	  ఆపు;
	}

      /* Now handle all primitive int/float data types.  */
      ఎంపిక (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  స్థితి FFI_TYPE_DOUBLE:
	    ఒకవేళ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    లేకపోతే
	      n_ov += పరిమాణం (ద్విగుణ) / పరిమాణం (దీర్ఘ);
	    ఆపు;

	  స్థితి FFI_TYPE_FLOAT:
	    ఒకవేళ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    లేకపోతే
	      n_ov++;
	    ఆపు;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#నిర్వచితం_కాకపోతే __s390x__
	  స్థితి FFI_TYPE_UINT64:
	  స్థితి FFI_TYPE_SINT64:
	    ఒకవేళ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ఒకవేళ (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    లేకపోతే
	      n_ov += 2;
	    ఆపు;
#అయితే_ముగింపు

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  అప్రమేయ:
	    ఒకవేళ (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    లేకపోతే
	      n_ov++;
	    ఆపు;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->బైట్లు = విడత_కొలత (n_ov * పరిమాణం (దీర్ఘ)) + struct_కొలత;

  తిరిగి FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

స్థిర శూన్య
ffi_call_int(ffi_cif *cif,
	     శూన్య (*fn)(శూన్య),
	     శూన్య *rvalue,
	     శూన్య **avalue,
	     శూన్య *సంవృతం)
{
  పూర్ణాంకం ret_type = cif->flags;
  size_t rsize = 0, బైట్లు = cif->బైట్లు;
  గుర్తులేని అక్షరం *దొంతర, *p_struct;
  నిర్మాణం call_చట్రం *చట్రం;
  గుర్తులేని దీర్ఘ *p_ov, *p_gpr;
  గుర్తులేని దీర్ఘ దీర్ఘ *p_fpr;
  పూర్ణాంకం n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  ఒకవేళ (rvalue == ఖాళీ)
    {
      ఒకవేళ (ret_type & FFI390_RET_లోపల_MEM)
	rsize = cif->rtype->కొలత;
      లేకపోతే
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

  దొంతర = alloca (బైట్లు + పరిమాణం(నిర్మాణం call_చట్రం) + rsize);
  చట్రం = (నిర్మాణం call_చట్రం *)(దొంతర + బైట్లు);
  ఒకవేళ (rsize)
    rvalue = చట్రం + 1;

  /* Link the new frame back to the one from this function.  */
  చట్రం->వెనుక_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (గుర్తులేని దీర్ఘ *)దొంతర;
  p_struct = (గుర్తులేని అక్షరం *)చట్రం;
  p_gpr = చట్రం->gpr_వాదనలు;
  p_fpr = చట్రం->fpr_వాదనలు;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  ఒకవేళ (cif->flags & FFI390_RET_లోపల_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  కొరకు (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      శూన్య *arg = avalue[i];
      పూర్ణాంకం type = ty->type;
      ffi_arg val;

    restart:
      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  వెళ్ళు do_int;
	స్థితి FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  వెళ్ళు do_int;
	స్థితి FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  వెళ్ళు do_int;
	స్థితి FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  వెళ్ళు do_int;
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  వెళ్ళు do_int;
	స్థితి FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  వెళ్ళు do_int;
	స్థితి FFI_TYPE_సూచిక:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  ఆపు;

	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
#నిర్వచితమైతే __s390x__
	  val = *(UINT64 *)arg;
	  వెళ్ళు do_int;
#లేకపోతే
	  ఒకవేళ (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  ఒకవేళ (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  లేకపోతే
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  ఒకవేళ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  లేకపోతే
	    {
#నిర్వచితమైతే __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#లేకపోతే
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#అయితే_ముగింపు
	    }
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  ఒకవేళ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  లేకపోతే
	    p_ov[n_ov++] = val;
	  ఆపు;

	స్థితి FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  ఒకవేళ (type != FFI_TYPE_సూచిక)
	    వెళ్ళు restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	స్థితి FFI_TYPE_LONGDOUBLE:
#అయితే_ముగింపు
	స్థితి FFI_TYPE_COMPLEX:
	స్థితి FFI_TYPE_SINT128:
	స్థితి FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= విడత_కొలత (ty->కొలత);
	  జ్ఞాపకం_నకలు (p_struct, arg, ty->కొలత);
	  val = (uintptr_t)p_struct;
	  వెళ్ళు do_int;

	అప్రమేయ:
	  FFI_ASSERT (0);
	  ఆపు;
        }
    }

  ffi_call_SYSV (చట్రం, ret_type & FFI360_RET_ముసుగు, rvalue, fn, సంవృతం);
}

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ఖాళీ);
}

శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
  ffi_call_int(cif, fn, rvalue, avalue, సంవృతం);
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_closure_helper_SYSV.                                */
/*                                                                    */
/* Function - Call a FFI closure target function.                     */
/*                                                                    */
/*====================================================================*/

శూన్య FFI_HIDDEN
ffi_సంవృతం_helper_SYSV (ffi_cif *cif,
			 శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
			 శూన్య *user_దత్తాంశం,
			 గుర్తులేని దీర్ఘ *p_gpr,
			 గుర్తులేని దీర్ఘ దీర్ఘ *p_fpr,
			 గుర్తులేని దీర్ఘ *p_ov)
{
  గుర్తులేని దీర్ఘ దీర్ఘ ret_బఫరు;

  శూన్య *rvalue = &ret_బఫరు;
  శూన్య **avalue;
  శూన్య **p_arg;

  పూర్ణాంకం n_gpr = 0;
  పూర్ణాంకం n_fpr = 0;
  పూర్ణాంకం n_ov = 0;

  ffi_type **ptr;
  పూర్ణాంకం i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * పరిమాణం (శూన్య *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  ఒకవేళ (cif->flags & FFI390_RET_లోపల_MEM)
    rvalue = (శూన్య *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  కొరకు (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      పూర్ణాంకం deref_struct_సూచిక = 0;
      పూర్ణాంకం type = (*ptr)->type;

#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      ఒకవేళ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#అయితే_ముగింపు

      /* Check how a structure type is passed.  */
      ఒకవేళ (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  ఒకవేళ (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_సూచిక;
	  లేకపోతే
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  ఒకవేళ (type == FFI_TYPE_సూచిక)
	    deref_struct_సూచిక = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      ఒకవేళ (type == FFI_TYPE_సూచిక)
	{
#నిర్వచితమైతే __s390x__
	  type = FFI_TYPE_UINT64;
#లేకపోతే
	  type = FFI_TYPE_UINT32;
#అయితే_ముగింపు
	}

      /* Now handle all primitive int/float data types.  */
      ఎంపిక (type)
	{
	  స్థితి FFI_TYPE_DOUBLE:
	    ఒకవేళ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    లేకపోతే
	      *p_arg = &p_ov[n_ov],
	      n_ov += పరిమాణం (ద్విగుణ) / పరిమాణం (దీర్ఘ);
	    ఆపు;

	  స్థితి FFI_TYPE_FLOAT:
	    ఒకవేళ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    లేకపోతే
	      *p_arg = (అక్షరం *)&p_ov[n_ov++] + పరిమాణం (దీర్ఘ) - 4;
	    ఆపు;

	  స్థితి FFI_TYPE_UINT64:
	  స్థితి FFI_TYPE_SINT64:
#నిర్వచితమైతే __s390x__
	    ఒకవేళ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    లేకపోతే
	      *p_arg = &p_ov[n_ov++];
#లేకపోతే
	    ఒకవేళ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ఒకవేళ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    లేకపోతే
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#అయితే_ముగింపు
	    ఆపు;

	  స్థితి FFI_TYPE_INT:
	  స్థితి FFI_TYPE_UINT32:
	  స్థితి FFI_TYPE_SINT32:
	    ఒకవేళ (n_gpr < MAX_GPRARGS)
	      *p_arg = (అక్షరం *)&p_gpr[n_gpr++] + పరిమాణం (దీర్ఘ) - 4;
	    లేకపోతే
	      *p_arg = (అక్షరం *)&p_ov[n_ov++] + పరిమాణం (దీర్ఘ) - 4;
	    ఆపు;

	  స్థితి FFI_TYPE_UINT16:
	  స్థితి FFI_TYPE_SINT16:
	    ఒకవేళ (n_gpr < MAX_GPRARGS)
	      *p_arg = (అక్షరం *)&p_gpr[n_gpr++] + పరిమాణం (దీర్ఘ) - 2;
	    లేకపోతే
	      *p_arg = (అక్షరం *)&p_ov[n_ov++] + పరిమాణం (దీర్ఘ) - 2;
	    ఆపు;

	  స్థితి FFI_TYPE_UINT8:
	  స్థితి FFI_TYPE_SINT8:
	    ఒకవేళ (n_gpr < MAX_GPRARGS)
	      *p_arg = (అక్షరం *)&p_gpr[n_gpr++] + పరిమాణం (దీర్ఘ) - 1;
	    లేకపోతే
	      *p_arg = (అక్షరం *)&p_ov[n_ov++] + పరిమాణం (దీర్ఘ) - 1;
	    ఆపు;

	  అప్రమేయ:
	    FFI_ASSERT (0);
	    ఆపు;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      ఒకవేళ (deref_struct_సూచిక)
	*p_arg = *(శూన్య **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_దత్తాంశం);

  /* Convert the return value.  */
  ఎంపిక (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      స్థితి FFI_TYPE_VOID:
      స్థితి FFI_TYPE_STRUCT:
      స్థితి FFI_TYPE_COMPLEX:
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      స్థితి FFI_TYPE_LONGDOUBLE:
#అయితే_ముగింపు
	ఆపు;

      /* Floating point values are returned in fpr 0.  */
      స్థితి FFI_TYPE_FLOAT:
	p_fpr[0] = (దీర్ఘ దీర్ఘ) *(గుర్తులేని పూర్ణాంకం *) rvalue << 32;
	ఆపు;

      స్థితి FFI_TYPE_DOUBLE:
	p_fpr[0] = *(గుర్తులేని దీర్ఘ దీర్ఘ *) rvalue;
	ఆపు;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      స్థితి FFI_TYPE_UINT64:
      స్థితి FFI_TYPE_SINT64:
#నిర్వచితమైతే __s390x__
	p_gpr[0] = *(గుర్తులేని దీర్ఘ *) rvalue;
#లేకపోతే
	p_gpr[0] = ((గుర్తులేని దీర్ఘ *) rvalue)[0],
	p_gpr[1] = ((గుర్తులేని దీర్ఘ *) rvalue)[1];
#అయితే_ముగింపు
	ఆపు;

      స్థితి FFI_TYPE_సూచిక:
      స్థితి FFI_TYPE_UINT32:
      స్థితి FFI_TYPE_UINT16:
      స్థితి FFI_TYPE_UINT8:
	p_gpr[0] = *(గుర్తులేని దీర్ఘ *) rvalue;
	ఆపు;

      స్థితి FFI_TYPE_INT:
      స్థితి FFI_TYPE_SINT32:
      స్థితి FFI_TYPE_SINT16:
      స్థితి FFI_TYPE_SINT8:
	p_gpr[0] = *(గుర్తుగల దీర్ఘ *) rvalue;
	ఆపు;

      అప్రమేయ:
        FFI_ASSERT (0);
        ఆపు;
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

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం *సంవృతం,
		      ffi_cif *cif,
		      శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  స్థిర గుర్తులేని పొట్టి స్థిరరాశి మూస[] = {
    0x0d10,			/* basr %r1,0 */
#నిర్వచితం_కాకపోతే __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#లేకపోతే
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#అయితే_ముగింపు
    0x07f1			/* br %r1 */
  };
  శూన్య (*dest)(శూన్య);
  గుర్తులేని దీర్ఘ *tramp = (గుర్తులేని దీర్ఘ *)&సంవృతం->tramp;

  ఒకవేళ (cif->abi != FFI_SYSV)
    తిరిగి FFI_BAD_ABI;


#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
  ఒకవేళ (ffi_tramp_is_present(సంవృతం))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_సంవృతం_SYSV;
      ffi_tramp_సమితి_parms (సంవృతం->ftramp, dest, సంవృతం);
      వెళ్ళు బహిర్గతం;
    }
#అయితే_ముగింపు

  జ్ఞాపకం_నకలు (tramp, మూస, పరిమాణం(మూస));
  tramp[2] = (గుర్తులేని దీర్ఘ)codeloc;
  tramp[3] = (గుర్తులేని దీర్ఘ)&ffi_సంవృతం_SYSV;

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
బహిర్గతం:
#అయితే_ముగింపు
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం *సంవృతం, ffi_cif *cif,
		     శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*))
{
  ఒకవేళ (cif->abi != FFI_SYSV)
    తిరిగి FFI_BAD_ABI;

  సంవృతం->tramp = ffi_go_సంవృతం_SYSV;
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
శూన్య *
ffi_tramp_arch (size_t *tramp_కొలత, size_t *map_కొలత)
{
  బాహ్య శూన్య *trampoline_సంకేతం_పట్టిక;

  *tramp_కొలత = FFI390_TRAMP_కొలత;
  *map_కొలత = FFI390_TRAMP_MAP_కొలత;
  తిరిగి &trampoline_సంకేతం_పట్టిక;
}
#అయితే_ముగింపు
