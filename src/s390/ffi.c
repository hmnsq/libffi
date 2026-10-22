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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव "internal.h"
#अन्तर्भाव <tramp.h>

/*====================== End of Includes =============================*/

/*====================================================================*/
/*परिभाषते*/
/*                           -------                                  */
/*====================================================================*/

/* Maximum number of GPRs available for argument passing.  */
#परिभाषय MAX_GPRARGS 5

/* Maximum number of FPRs available for argument passing.  */
#यदि_परिभाषितम् __s390x__
#परिभाषय MAX_FPRARGS 4
#अन्यथा
#परिभाषय MAX_FPRARGS 2
#यदि_अन्तः

/* Round to multiple of 16.  */
#परिभाषय आवर्तनम्_आकार(आकार) (((आकार) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

संरचना call_frame
{
  शून्य *पृष्ठभागः_chain;
  शून्य *eos;
  अचिह्नित दीर्घ gpr_तर्काः[5];
  अचिह्नित दीर्घ gpr_save[9];
  अचिह्नित दीर्घ दीर्घ fpr_तर्काः[4];
};

बाह्य शून्य FFI_HIDDEN ffi_call_SYSV(संरचना call_frame *, अचिह्नित, शून्य *,
			             शून्य (*fn)(शून्य), शून्य *);

बाह्य शून्य ffi_संवृतिः_SYSV(शून्य);
बाह्य शून्य ffi_go_संवृतिः_SYSV(शून्य);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

स्थिर पूर्णाङ्क
ffi_check_struct_type (ffi_type *arg)
{
  size_t आकार = arg->आकार;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  यावत् (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  विकल्प (आकार)
    {
      पक्ष 1:
        प्रत्यागम FFI_TYPE_UINT8;

      पक्ष 2:
        प्रत्यागम FFI_TYPE_UINT16;

      पक्ष 4:
	यदि (arg->type == FFI_TYPE_FLOAT)
          प्रत्यागम FFI_TYPE_FLOAT;
	अन्यथा
	  प्रत्यागम FFI_TYPE_UINT32;

      पक्ष 8:
	यदि (arg->type == FFI_TYPE_DOUBLE)
          प्रत्यागम FFI_TYPE_DOUBLE;
	अन्यथा
	  प्रत्यागम FFI_TYPE_UINT64;

      स्वभाव:
	भङ्ग;
    }

  /* Other structs are passed via a pointer to the data.  */
  प्रत्यागम FFI_TYPE_POINTER;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_prep_cif_machdep.                                   */
/*                                                                    */
/* Function - Perform machine dependent CIF processing.               */
/*                                                                    */
/*====================================================================*/

ffi_स्थितिः FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t struct_आकार = 0;
  पूर्णाङ्क n_gpr = 0;
  पूर्णाङ्क n_fpr = 0;
  पूर्णाङ्क n_ov = 0;

  ffi_type **ptr;
  पूर्णाङ्क i;

  /* Determine return value handling.  */

  विकल्प (cif->rtype->type)
    {
      /* Void is easy.  */
      पक्ष FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	भङ्ग;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      पक्ष FFI_TYPE_STRUCT:
      पक्ष FFI_TYPE_COMPLEX:
      पक्ष FFI_TYPE_SINT128:
      पक्ष FFI_TYPE_UINT128:
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      पक्ष FFI_TYPE_LONGDOUBLE:
#यदि_अन्तः
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	भङ्ग;

      /* Floating point values are returned in fpr 0.  */
      पक्ष FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	भङ्ग;

      पक्ष FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	भङ्ग;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      पक्ष FFI_TYPE_UINT64:
      पक्ष FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	भङ्ग;

      पक्ष FFI_TYPE_POINTER:
      पक्ष FFI_TYPE_INT:
      पक्ष FFI_TYPE_UINT32:
      पक्ष FFI_TYPE_SINT32:
      पक्ष FFI_TYPE_UINT16:
      पक्ष FFI_TYPE_SINT16:
      पक्ष FFI_TYPE_UINT8:
      पक्ष FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#यदि_परिभाषितम् __s390x__
	cif->flags = FFI390_RET_INT64;
#अन्यथा
	cif->flags = FFI390_RET_INT32;
#यदि_अन्तः
	भङ्ग;

      स्वभाव:
        FFI_ASSERT (0);
        भङ्ग;
    }

  /* Now for the arguments.  */

  प्रति (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      पूर्णाङ्क type = (*ptr)->type;

      विकल्प (type)
	{
	पक्ष FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  यदि (type != FFI_TYPE_POINTER)
	    भङ्ग;
	  /* fall through */

	पक्ष FFI_TYPE_COMPLEX:
	पक्ष FFI_TYPE_SINT128:
	पक्ष FFI_TYPE_UINT128:
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
#यदि_अन्तः
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_आकार += आवर्तनम्_आकार ((*ptr)->आकार);
	  भङ्ग;
	}

      /* Now handle all primitive int/float data types.  */
      विकल्प (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  पक्ष FFI_TYPE_DOUBLE:
	    यदि (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    अन्यथा
	      n_ov += परिमाण (द्विगुण) / परिमाण (दीर्घ);
	    भङ्ग;

	  पक्ष FFI_TYPE_FLOAT:
	    यदि (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    अन्यथा
	      n_ov++;
	    भङ्ग;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#यदि_अपरिभाषितम् __s390x__
	  पक्ष FFI_TYPE_UINT64:
	  पक्ष FFI_TYPE_SINT64:
	    यदि (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    यदि (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    अन्यथा
	      n_ov += 2;
	    भङ्ग;
#यदि_अन्तः

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  स्वभाव:
	    यदि (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    अन्यथा
	      n_ov++;
	    भङ्ग;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->अष्टकानि = आवर्तनम्_आकार (n_ov * परिमाण (दीर्घ)) + struct_आकार;

  प्रत्यागम FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

स्थिर शून्य
ffi_call_int(ffi_cif *cif,
	     शून्य (*fn)(शून्य),
	     शून्य *rvalue,
	     शून्य **avalue,
	     शून्य *संवृतिः)
{
  पूर्णाङ्क ret_type = cif->flags;
  size_t rsize = 0, अष्टकानि = cif->अष्टकानि;
  अचिह्नित वर्ण *राशिः, *p_struct;
  संरचना call_frame *frame;
  अचिह्नित दीर्घ *p_ov, *p_gpr;
  अचिह्नित दीर्घ दीर्घ *p_fpr;
  पूर्णाङ्क n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  यदि (rvalue == रिक्त)
    {
      यदि (ret_type & FFI390_RET_अन्तर्_MEM)
	rsize = cif->rtype->आकार;
      अन्यथा
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

  राशिः = alloca (अष्टकानि + परिमाण(संरचना call_frame) + rsize);
  frame = (संरचना call_frame *)(राशिः + अष्टकानि);
  यदि (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->पृष्ठभागः_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (अचिह्नित दीर्घ *)राशिः;
  p_struct = (अचिह्नित वर्ण *)frame;
  p_gpr = frame->gpr_तर्काः;
  p_fpr = frame->fpr_तर्काः;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  यदि (cif->flags & FFI390_RET_अन्तर्_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  प्रति (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      शून्य *arg = avalue[i];
      पूर्णाङ्क type = ty->type;
      ffi_arg val;

    restart:
      विकल्प (type)
	{
	पक्ष FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  गच्छ do_int;
	पक्ष FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  गच्छ do_int;
	पक्ष FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  गच्छ do_int;
	पक्ष FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  गच्छ do_int;
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  गच्छ do_int;
	पक्ष FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  गच्छ do_int;
	पक्ष FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  भङ्ग;

	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
#यदि_परिभाषितम् __s390x__
	  val = *(UINT64 *)arg;
	  गच्छ do_int;
#अन्यथा
	  यदि (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  यदि (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  अन्यथा
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#यदि_अन्तः
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  यदि (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  अन्यथा
	    {
#यदि_परिभाषितम् __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#अन्यथा
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#यदि_अन्तः
	    }
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  यदि (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  अन्यथा
	    p_ov[n_ov++] = val;
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  यदि (type != FFI_TYPE_POINTER)
	    गच्छ restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
#यदि_अन्तः
	पक्ष FFI_TYPE_COMPLEX:
	पक्ष FFI_TYPE_SINT128:
	पक्ष FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= आवर्तनम्_आकार (ty->आकार);
	  स्मृतिं_प्रतिलिख (p_struct, arg, ty->आकार);
	  val = (uintptr_t)p_struct;
	  गच्छ do_int;

	स्वभाव:
	  FFI_ASSERT (0);
	  भङ्ग;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_आवरणम्, rvalue, fn, संवृतिः);
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *संवृतिः)
{
  ffi_call_int(cif, fn, rvalue, avalue, संवृतिः);
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_closure_helper_SYSV.                                */
/*                                                                    */
/* Function - Call a FFI closure target function.                     */
/*                                                                    */
/*====================================================================*/

शून्य FFI_HIDDEN
ffi_संवृतिः_helper_SYSV (ffi_cif *cif,
			 शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
			 शून्य *user_दत्त,
			 अचिह्नित दीर्घ *p_gpr,
			 अचिह्नित दीर्घ दीर्घ *p_fpr,
			 अचिह्नित दीर्घ *p_ov)
{
  अचिह्नित दीर्घ दीर्घ ret_बफर;

  शून्य *rvalue = &ret_बफर;
  शून्य **avalue;
  शून्य **p_arg;

  पूर्णाङ्क n_gpr = 0;
  पूर्णाङ्क n_fpr = 0;
  पूर्णाङ्क n_ov = 0;

  ffi_type **ptr;
  पूर्णाङ्क i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * परिमाण (शून्य *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  यदि (cif->flags & FFI390_RET_अन्तर्_MEM)
    rvalue = (शून्य *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  प्रति (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      पूर्णाङ्क deref_struct_pointer = 0;
      पूर्णाङ्क type = (*ptr)->type;

#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      यदि (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#यदि_अन्तः

      /* Check how a structure type is passed.  */
      यदि (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  यदि (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  अन्यथा
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  यदि (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      यदि (type == FFI_TYPE_POINTER)
	{
#यदि_परिभाषितम् __s390x__
	  type = FFI_TYPE_UINT64;
#अन्यथा
	  type = FFI_TYPE_UINT32;
#यदि_अन्तः
	}

      /* Now handle all primitive int/float data types.  */
      विकल्प (type)
	{
	  पक्ष FFI_TYPE_DOUBLE:
	    यदि (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    अन्यथा
	      *p_arg = &p_ov[n_ov],
	      n_ov += परिमाण (द्विगुण) / परिमाण (दीर्घ);
	    भङ्ग;

	  पक्ष FFI_TYPE_FLOAT:
	    यदि (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    अन्यथा
	      *p_arg = (वर्ण *)&p_ov[n_ov++] + परिमाण (दीर्घ) - 4;
	    भङ्ग;

	  पक्ष FFI_TYPE_UINT64:
	  पक्ष FFI_TYPE_SINT64:
#यदि_परिभाषितम् __s390x__
	    यदि (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    अन्यथा
	      *p_arg = &p_ov[n_ov++];
#अन्यथा
	    यदि (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    यदि (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    अन्यथा
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#यदि_अन्तः
	    भङ्ग;

	  पक्ष FFI_TYPE_INT:
	  पक्ष FFI_TYPE_UINT32:
	  पक्ष FFI_TYPE_SINT32:
	    यदि (n_gpr < MAX_GPRARGS)
	      *p_arg = (वर्ण *)&p_gpr[n_gpr++] + परिमाण (दीर्घ) - 4;
	    अन्यथा
	      *p_arg = (वर्ण *)&p_ov[n_ov++] + परिमाण (दीर्घ) - 4;
	    भङ्ग;

	  पक्ष FFI_TYPE_UINT16:
	  पक्ष FFI_TYPE_SINT16:
	    यदि (n_gpr < MAX_GPRARGS)
	      *p_arg = (वर्ण *)&p_gpr[n_gpr++] + परिमाण (दीर्घ) - 2;
	    अन्यथा
	      *p_arg = (वर्ण *)&p_ov[n_ov++] + परिमाण (दीर्घ) - 2;
	    भङ्ग;

	  पक्ष FFI_TYPE_UINT8:
	  पक्ष FFI_TYPE_SINT8:
	    यदि (n_gpr < MAX_GPRARGS)
	      *p_arg = (वर्ण *)&p_gpr[n_gpr++] + परिमाण (दीर्घ) - 1;
	    अन्यथा
	      *p_arg = (वर्ण *)&p_ov[n_ov++] + परिमाण (दीर्घ) - 1;
	    भङ्ग;

	  स्वभाव:
	    FFI_ASSERT (0);
	    भङ्ग;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      यदि (deref_struct_pointer)
	*p_arg = *(शून्य **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_दत्त);

  /* Convert the return value.  */
  विकल्प (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      पक्ष FFI_TYPE_VOID:
      पक्ष FFI_TYPE_STRUCT:
      पक्ष FFI_TYPE_COMPLEX:
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      पक्ष FFI_TYPE_LONGDOUBLE:
#यदि_अन्तः
	भङ्ग;

      /* Floating point values are returned in fpr 0.  */
      पक्ष FFI_TYPE_FLOAT:
	p_fpr[0] = (दीर्घ दीर्घ) *(अचिह्नित पूर्णाङ्क *) rvalue << 32;
	भङ्ग;

      पक्ष FFI_TYPE_DOUBLE:
	p_fpr[0] = *(अचिह्नित दीर्घ दीर्घ *) rvalue;
	भङ्ग;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      पक्ष FFI_TYPE_UINT64:
      पक्ष FFI_TYPE_SINT64:
#यदि_परिभाषितम् __s390x__
	p_gpr[0] = *(अचिह्नित दीर्घ *) rvalue;
#अन्यथा
	p_gpr[0] = ((अचिह्नित दीर्घ *) rvalue)[0],
	p_gpr[1] = ((अचिह्नित दीर्घ *) rvalue)[1];
#यदि_अन्तः
	भङ्ग;

      पक्ष FFI_TYPE_POINTER:
      पक्ष FFI_TYPE_UINT32:
      पक्ष FFI_TYPE_UINT16:
      पक्ष FFI_TYPE_UINT8:
	p_gpr[0] = *(अचिह्नित दीर्घ *) rvalue;
	भङ्ग;

      पक्ष FFI_TYPE_INT:
      पक्ष FFI_TYPE_SINT32:
      पक्ष FFI_TYPE_SINT16:
      पक्ष FFI_TYPE_SINT8:
	p_gpr[0] = *(चिह्नित दीर्घ *) rvalue;
	भङ्ग;

      स्वभाव:
        FFI_ASSERT (0);
        भङ्ग;
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

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः *संवृतिः,
		      ffi_cif *cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_दत्त,
		      शून्य *codeloc)
{
  स्थिर अचिह्नित ह्रस्व ध्रुव प्रतिरूपम्[] = {
    0x0d10,			/* basr %r1,0 */
#यदि_अपरिभाषितम् __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#अन्यथा
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#यदि_अन्तः
    0x07f1			/* br %r1 */
  };
  शून्य (*dest)(शून्य);
  अचिह्नित दीर्घ *tramp = (अचिह्नित दीर्घ *)&संवृतिः->tramp;

  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;


#यदि defined(FFI_EXEC_STATIC_TRAMP)
  यदि (ffi_tramp_is_present(संवृतिः))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_संवृतिः_SYSV;
      ffi_tramp_समुच्चयः_parms (संवृतिः->ftramp, dest, संवृतिः);
      गच्छ बहिर्गमनम्;
    }
#यदि_अन्तः

  स्मृतिं_प्रतिलिख (tramp, प्रतिरूपम्, परिमाण(प्रतिरूपम्));
  tramp[2] = (अचिह्नित दीर्घ)codeloc;
  tramp[3] = (अचिह्नित दीर्घ)&ffi_संवृतिः_SYSV;

#यदि defined(FFI_EXEC_STATIC_TRAMP)
बहिर्गमनम्:
#यदि_अन्तः
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;

  प्रत्यागम FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_स्थितिः
ffi_prep_go_संवृतिः (ffi_go_संवृतिः *संवृतिः, ffi_cif *cif,
		     शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*))
{
  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  संवृतिः->tramp = ffi_go_संवृतिः_SYSV;
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;

  प्रत्यागम FFI_OK;
}

#यदि defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (size_t *tramp_आकार, size_t *map_आकार)
{
  बाह्य शून्य *trampoline_संहिता_सारणी;

  *tramp_आकार = FFI390_TRAMP_आकार;
  *map_आकार = FFI390_TRAMP_MAP_आकार;
  प्रत्यागम &trampoline_संहिता_सारणी;
}
#यदि_अन्तः
