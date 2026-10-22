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
/*                           Defines                                  */
/*                           -------                                  */
/*====================================================================*/

/* Maximum number of GPRs available for argument passing.  */
#define MAX_GPRARGS 5

/* Maximum number of FPRs available for argument passing.  */
#ifdef __s390x__
#define MAX_FPRARGS 4
#अन्यथा
#define MAX_FPRARGS 2
#endif

/* Round to multiple of 16.  */
#define ROUND_SIZE(size) (((size) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

संरचना call_frame
{
  शून्य *back_chain;
  शून्य *eos;
  अचिन्हित दीर्घ gpr_args[5];
  अचिन्हित दीर्घ gpr_save[9];
  अचिन्हित दीर्घ दीर्घ fpr_args[4];
};

बाह्य शून्य FFI_HIDDEN ffi_call_SYSV(संरचना call_frame *, अचिन्हित, शून्य *,
			             शून्य (*fn)(शून्य), शून्य *);

बाह्य शून्य ffi_closure_SYSV(शून्य);
बाह्य शून्य ffi_go_closure_SYSV(शून्य);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

स्थिर पूर्णांक
ffi_check_struct_type (ffi_type *arg)
{
  माप_प्रकार size = arg->size;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  पर्यंत (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  निवड (size)
    {
      स्थिती 1:
        परत FFI_TYPE_UINT8;

      स्थिती 2:
        परत FFI_TYPE_UINT16;

      स्थिती 4:
	जर (arg->type == FFI_TYPE_FLOAT)
          परत FFI_TYPE_FLOAT;
	अन्यथा
	  परत FFI_TYPE_UINT32;

      स्थिती 8:
	जर (arg->type == FFI_TYPE_DOUBLE)
          परत FFI_TYPE_DOUBLE;
	अन्यथा
	  परत FFI_TYPE_UINT64;

      पूर्वनिर्धारित:
	थांबा;
    }

  /* Other structs are passed via a pointer to the data.  */
  परत FFI_TYPE_POINTER;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_prep_cif_machdep.                                   */
/*                                                                    */
/* Function - Perform machine dependent CIF processing.               */
/*                                                                    */
/*====================================================================*/

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  माप_प्रकार struct_size = 0;
  पूर्णांक n_gpr = 0;
  पूर्णांक n_fpr = 0;
  पूर्णांक n_ov = 0;

  ffi_type **ptr;
  पूर्णांक i;

  /* Determine return value handling.  */

  निवड (cif->rtype->type)
    {
      /* Void is easy.  */
      स्थिती FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	थांबा;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      स्थिती FFI_TYPE_STRUCT:
      स्थिती FFI_TYPE_COMPLEX:
      स्थिती FFI_TYPE_SINT128:
      स्थिती FFI_TYPE_UINT128:
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      स्थिती FFI_TYPE_LONGDOUBLE:
#endif
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	थांबा;

      /* Floating point values are returned in fpr 0.  */
      स्थिती FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	थांबा;

      स्थिती FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	थांबा;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      स्थिती FFI_TYPE_UINT64:
      स्थिती FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	थांबा;

      स्थिती FFI_TYPE_POINTER:
      स्थिती FFI_TYPE_INT:
      स्थिती FFI_TYPE_UINT32:
      स्थिती FFI_TYPE_SINT32:
      स्थिती FFI_TYPE_UINT16:
      स्थिती FFI_TYPE_SINT16:
      स्थिती FFI_TYPE_UINT8:
      स्थिती FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#अन्यथा
	cif->flags = FFI390_RET_INT32;
#endif
	थांबा;

      पूर्वनिर्धारित:
        FFI_ASSERT (0);
        थांबा;
    }

  /* Now for the arguments.  */

  साठी (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      पूर्णांक type = (*ptr)->type;

      निवड (type)
	{
	स्थिती FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  जर (type != FFI_TYPE_POINTER)
	    थांबा;
	  /* fall through */

	स्थिती FFI_TYPE_COMPLEX:
	स्थिती FFI_TYPE_SINT128:
	स्थिती FFI_TYPE_UINT128:
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	स्थिती FFI_TYPE_LONGDOUBLE:
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_size += ROUND_SIZE ((*ptr)->size);
	  थांबा;
	}

      /* Now handle all primitive int/float data types.  */
      निवड (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  स्थिती FFI_TYPE_DOUBLE:
	    जर (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    अन्यथा
	      n_ov += आकार (द्विगुण) / आकार (दीर्घ);
	    थांबा;

	  स्थिती FFI_TYPE_FLOAT:
	    जर (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    अन्यथा
	      n_ov++;
	    थांबा;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ifndef __s390x__
	  स्थिती FFI_TYPE_UINT64:
	  स्थिती FFI_TYPE_SINT64:
	    जर (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    जर (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    अन्यथा
	      n_ov += 2;
	    थांबा;
#endif

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  पूर्वनिर्धारित:
	    जर (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    अन्यथा
	      n_ov++;
	    थांबा;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_SIZE (n_ov * आकार (दीर्घ)) + struct_size;

  परत FFI_OK;
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
	     शून्य *closure)
{
  पूर्णांक ret_type = cif->flags;
  माप_प्रकार rsize = 0, bytes = cif->bytes;
  अचिन्हित अक्षर *चळत, *p_struct;
  संरचना call_frame *frame;
  अचिन्हित दीर्घ *p_ov, *p_gpr;
  अचिन्हित दीर्घ दीर्घ *p_fpr;
  पूर्णांक n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  जर (rvalue == रिक्त)
    {
      जर (ret_type & FFI390_RET_IN_MEM)
	rsize = cif->rtype->size;
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

  चळत = alloca (bytes + आकार(संरचना call_frame) + rsize);
  frame = (संरचना call_frame *)(चळत + bytes);
  जर (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (अचिन्हित दीर्घ *)चळत;
  p_struct = (अचिन्हित अक्षर *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  जर (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  साठी (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      शून्य *arg = avalue[i];
      पूर्णांक type = ty->type;
      ffi_arg val;

    restart:
      निवड (type)
	{
	स्थिती FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  जा do_int;
	स्थिती FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  जा do_int;
	स्थिती FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  जा do_int;
	स्थिती FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  जा do_int;
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  जा do_int;
	स्थिती FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  जा do_int;
	स्थिती FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  थांबा;

	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
#ifdef __s390x__
	  val = *(UINT64 *)arg;
	  जा do_int;
#अन्यथा
	  जर (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  जर (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  अन्यथा
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	  जर (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  अन्यथा
	    {
#ifdef __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#अन्यथा
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	    }
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  जर (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  अन्यथा
	    p_ov[n_ov++] = val;
	  थांबा;

	स्थिती FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  जर (type != FFI_TYPE_POINTER)
	    जा restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	स्थिती FFI_TYPE_LONGDOUBLE:
#endif
	स्थिती FFI_TYPE_COMPLEX:
	स्थिती FFI_TYPE_SINT128:
	स्थिती FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_SIZE (ty->size);
	  स_नकल (p_struct, arg, ty->size);
	  val = (uintptr_t)p_struct;
	  जा do_int;

	पूर्वनिर्धारित:
	  FFI_ASSERT (0);
	  थांबा;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
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
ffi_closure_helper_SYSV (ffi_cif *cif,
			 शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
			 शून्य *user_data,
			 अचिन्हित दीर्घ *p_gpr,
			 अचिन्हित दीर्घ दीर्घ *p_fpr,
			 अचिन्हित दीर्घ *p_ov)
{
  अचिन्हित दीर्घ दीर्घ ret_buffer;

  शून्य *rvalue = &ret_buffer;
  शून्य **avalue;
  शून्य **p_arg;

  पूर्णांक n_gpr = 0;
  पूर्णांक n_fpr = 0;
  पूर्णांक n_ov = 0;

  ffi_type **ptr;
  पूर्णांक i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * आकार (शून्य *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  जर (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (शून्य *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  साठी (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      पूर्णांक deref_struct_pointer = 0;
      पूर्णांक type = (*ptr)->type;

#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      जर (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif

      /* Check how a structure type is passed.  */
      जर (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  जर (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  अन्यथा
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  जर (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      जर (type == FFI_TYPE_POINTER)
	{
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#अन्यथा
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      निवड (type)
	{
	  स्थिती FFI_TYPE_DOUBLE:
	    जर (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    अन्यथा
	      *p_arg = &p_ov[n_ov],
	      n_ov += आकार (द्विगुण) / आकार (दीर्घ);
	    थांबा;

	  स्थिती FFI_TYPE_FLOAT:
	    जर (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    अन्यथा
	      *p_arg = (अक्षर *)&p_ov[n_ov++] + आकार (दीर्घ) - 4;
	    थांबा;

	  स्थिती FFI_TYPE_UINT64:
	  स्थिती FFI_TYPE_SINT64:
#ifdef __s390x__
	    जर (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    अन्यथा
	      *p_arg = &p_ov[n_ov++];
#अन्यथा
	    जर (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    जर (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    अन्यथा
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#endif
	    थांबा;

	  स्थिती FFI_TYPE_INT:
	  स्थिती FFI_TYPE_UINT32:
	  स्थिती FFI_TYPE_SINT32:
	    जर (n_gpr < MAX_GPRARGS)
	      *p_arg = (अक्षर *)&p_gpr[n_gpr++] + आकार (दीर्घ) - 4;
	    अन्यथा
	      *p_arg = (अक्षर *)&p_ov[n_ov++] + आकार (दीर्घ) - 4;
	    थांबा;

	  स्थिती FFI_TYPE_UINT16:
	  स्थिती FFI_TYPE_SINT16:
	    जर (n_gpr < MAX_GPRARGS)
	      *p_arg = (अक्षर *)&p_gpr[n_gpr++] + आकार (दीर्घ) - 2;
	    अन्यथा
	      *p_arg = (अक्षर *)&p_ov[n_ov++] + आकार (दीर्घ) - 2;
	    थांबा;

	  स्थिती FFI_TYPE_UINT8:
	  स्थिती FFI_TYPE_SINT8:
	    जर (n_gpr < MAX_GPRARGS)
	      *p_arg = (अक्षर *)&p_gpr[n_gpr++] + आकार (दीर्घ) - 1;
	    अन्यथा
	      *p_arg = (अक्षर *)&p_ov[n_ov++] + आकार (दीर्घ) - 1;
	    थांबा;

	  पूर्वनिर्धारित:
	    FFI_ASSERT (0);
	    थांबा;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      जर (deref_struct_pointer)
	*p_arg = *(शून्य **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_data);

  /* Convert the return value.  */
  निवड (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      स्थिती FFI_TYPE_VOID:
      स्थिती FFI_TYPE_STRUCT:
      स्थिती FFI_TYPE_COMPLEX:
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      स्थिती FFI_TYPE_LONGDOUBLE:
#endif
	थांबा;

      /* Floating point values are returned in fpr 0.  */
      स्थिती FFI_TYPE_FLOAT:
	p_fpr[0] = (दीर्घ दीर्घ) *(अचिन्हित पूर्णांक *) rvalue << 32;
	थांबा;

      स्थिती FFI_TYPE_DOUBLE:
	p_fpr[0] = *(अचिन्हित दीर्घ दीर्घ *) rvalue;
	थांबा;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      स्थिती FFI_TYPE_UINT64:
      स्थिती FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(अचिन्हित दीर्घ *) rvalue;
#अन्यथा
	p_gpr[0] = ((अचिन्हित दीर्घ *) rvalue)[0],
	p_gpr[1] = ((अचिन्हित दीर्घ *) rvalue)[1];
#endif
	थांबा;

      स्थिती FFI_TYPE_POINTER:
      स्थिती FFI_TYPE_UINT32:
      स्थिती FFI_TYPE_UINT16:
      स्थिती FFI_TYPE_UINT8:
	p_gpr[0] = *(अचिन्हित दीर्घ *) rvalue;
	थांबा;

      स्थिती FFI_TYPE_INT:
      स्थिती FFI_TYPE_SINT32:
      स्थिती FFI_TYPE_SINT16:
      स्थिती FFI_TYPE_SINT8:
	p_gpr[0] = *(चिन्हित दीर्घ *) rvalue;
	थांबा;

      पूर्वनिर्धारित:
        FFI_ASSERT (0);
        थांबा;
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

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  स्थिर अचिन्हित लघु स्थिरांक ढाँचा[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#अन्यथा
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  शून्य (*dest)(शून्य);
  अचिन्हित दीर्घ *tramp = (अचिन्हित दीर्घ *)&closure->tramp;

  जर (cif->abi != FFI_SYSV)
    परत FFI_BAD_ABI;


#जर defined(FFI_EXEC_STATIC_TRAMP)
  जर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      जा out;
    }
#endif

  स_नकल (tramp, ढाँचा, आकार(ढाँचा));
  tramp[2] = (अचिन्हित दीर्घ)codeloc;
  tramp[3] = (अचिन्हित दीर्घ)&ffi_closure_SYSV;

#जर defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  परत FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*))
{
  जर (cif->abi != FFI_SYSV)
    परत FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  परत FFI_OK;
}

#जर defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (माप_प्रकार *tramp_size, माप_प्रकार *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *tramp_size = FFI390_TRAMP_SIZE;
  *map_size = FFI390_TRAMP_MAP_SIZE;
  परत &trampoline_code_table;
}
#endif
