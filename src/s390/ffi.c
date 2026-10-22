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

#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश <stdint.h>
#समावेश "internal.h"
#समावेश <tramp.h>

/*====================== End of Includes =============================*/

/*====================================================================*/
/*                           Defines                                  */
/*                           -------                                  */
/*====================================================================*/

/* Maximum number of GPRs available for argument passing.  */
#घोषणा MAX_GPRARGS 5

/* Maximum number of FPRs available for argument passing.  */
#अगर_घोषित __s390x__
#घोषणा MAX_FPRARGS 4
#अन्यथा
#घोषणा MAX_FPRARGS 2
#पूर्ण_अगर

/* Round to multiple of 16.  */
#घोषणा ROUND_आकार(आकार) (((आकार) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

काष्ठा call_frame
{
  व्योम *back_chain;
  व्योम *eos;
  अचिन्हित दीर्घ gpr_args[5];
  अचिन्हित दीर्घ gpr_save[9];
  अचिन्हित दीर्घ दीर्घ fpr_args[4];
};

बाह्य व्योम FFI_HIDDEN ffi_call_SYSV(काष्ठा call_frame *, अचिन्हित, व्योम *,
			             व्योम (*fn)(व्योम), व्योम *);

बाह्य व्योम ffi_closure_SYSV(व्योम);
बाह्य व्योम ffi_go_closure_SYSV(व्योम);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

अटल पूर्णांक
ffi_check_struct_type (ffi_type *arg)
{
  माप_प्रकार आकार = arg->आकार;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  जबतक (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  चयन (आकार)
    {
      हाल 1:
        वापस FFI_TYPE_UINT8;

      हाल 2:
        वापस FFI_TYPE_UINT16;

      हाल 4:
	अगर (arg->type == FFI_TYPE_FLOAT)
          वापस FFI_TYPE_FLOAT;
	अन्यथा
	  वापस FFI_TYPE_UINT32;

      हाल 8:
	अगर (arg->type == FFI_TYPE_DOUBLE)
          वापस FFI_TYPE_DOUBLE;
	अन्यथा
	  वापस FFI_TYPE_UINT64;

      शेष:
	अवरोध;
    }

  /* Other structs are passed via a pointer to the data.  */
  वापस FFI_TYPE_POINTER;
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
  माप_प्रकार struct_आकार = 0;
  पूर्णांक n_gpr = 0;
  पूर्णांक n_fpr = 0;
  पूर्णांक n_ov = 0;

  ffi_type **ptr;
  पूर्णांक i;

  /* Determine return value handling.  */

  चयन (cif->rtype->type)
    {
      /* Void is easy.  */
      हाल FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	अवरोध;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      हाल FFI_TYPE_STRUCT:
      हाल FFI_TYPE_COMPLEX:
      हाल FFI_TYPE_SINT128:
      हाल FFI_TYPE_UINT128:
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      हाल FFI_TYPE_LONGDOUBLE:
#पूर्ण_अगर
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	अवरोध;

      /* Floating point values are returned in fpr 0.  */
      हाल FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	अवरोध;

      हाल FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	अवरोध;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      हाल FFI_TYPE_UINT64:
      हाल FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	अवरोध;

      हाल FFI_TYPE_POINTER:
      हाल FFI_TYPE_INT:
      हाल FFI_TYPE_UINT32:
      हाल FFI_TYPE_SINT32:
      हाल FFI_TYPE_UINT16:
      हाल FFI_TYPE_SINT16:
      हाल FFI_TYPE_UINT8:
      हाल FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#अगर_घोषित __s390x__
	cif->flags = FFI390_RET_INT64;
#अन्यथा
	cif->flags = FFI390_RET_INT32;
#पूर्ण_अगर
	अवरोध;

      शेष:
        FFI_ASSERT (0);
        अवरोध;
    }

  /* Now for the arguments.  */

  क्रम (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      पूर्णांक type = (*ptr)->type;

      चयन (type)
	{
	हाल FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  अगर (type != FFI_TYPE_POINTER)
	    अवरोध;
	  /* fall through */

	हाल FFI_TYPE_COMPLEX:
	हाल FFI_TYPE_SINT128:
	हाल FFI_TYPE_UINT128:
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	हाल FFI_TYPE_LONGDOUBLE:
#पूर्ण_अगर
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_आकार += ROUND_आकार ((*ptr)->आकार);
	  अवरोध;
	}

      /* Now handle all primitive int/float data types.  */
      चयन (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  हाल FFI_TYPE_DOUBLE:
	    अगर (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    अन्यथा
	      n_ov += माप (द्विगुन) / माप (दीर्घ);
	    अवरोध;

	  हाल FFI_TYPE_FLOAT:
	    अगर (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    अन्यथा
	      n_ov++;
	    अवरोध;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#अगर_अघोषित __s390x__
	  हाल FFI_TYPE_UINT64:
	  हाल FFI_TYPE_SINT64:
	    अगर (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    अगर (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    अन्यथा
	      n_ov += 2;
	    अवरोध;
#पूर्ण_अगर

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  शेष:
	    अगर (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    अन्यथा
	      n_ov++;
	    अवरोध;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_आकार (n_ov * माप (दीर्घ)) + struct_आकार;

  वापस FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

अटल व्योम
ffi_call_int(ffi_cif *cif,
	     व्योम (*fn)(व्योम),
	     व्योम *rvalue,
	     व्योम **avalue,
	     व्योम *closure)
{
  पूर्णांक ret_type = cif->flags;
  माप_प्रकार rsize = 0, bytes = cif->bytes;
  अचिन्हित अक्षर *ढेर, *p_struct;
  काष्ठा call_frame *frame;
  अचिन्हित दीर्घ *p_ov, *p_gpr;
  अचिन्हित दीर्घ दीर्घ *p_fpr;
  पूर्णांक n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  अगर (rvalue == शून्य)
    {
      अगर (ret_type & FFI390_RET_IN_MEM)
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

  ढेर = alloca (bytes + माप(काष्ठा call_frame) + rsize);
  frame = (काष्ठा call_frame *)(ढेर + bytes);
  अगर (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (अचिन्हित दीर्घ *)ढेर;
  p_struct = (अचिन्हित अक्षर *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  अगर (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  क्रम (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      व्योम *arg = avalue[i];
      पूर्णांक type = ty->type;
      ffi_arg val;

    restart:
      चयन (type)
	{
	हाल FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  जाओ do_int;
	हाल FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  जाओ do_int;
	हाल FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  जाओ do_int;
	हाल FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  जाओ do_int;
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  जाओ do_int;
	हाल FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  जाओ do_int;
	हाल FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  अवरोध;

	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
#अगर_घोषित __s390x__
	  val = *(UINT64 *)arg;
	  जाओ do_int;
#अन्यथा
	  अगर (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  अगर (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  अन्यथा
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_DOUBLE:
	  अगर (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  अन्यथा
	    {
#अगर_घोषित __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#अन्यथा
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#पूर्ण_अगर
	    }
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  अगर (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  अन्यथा
	    p_ov[n_ov++] = val;
	  अवरोध;

	हाल FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  अगर (type != FFI_TYPE_POINTER)
	    जाओ restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	हाल FFI_TYPE_LONGDOUBLE:
#पूर्ण_अगर
	हाल FFI_TYPE_COMPLEX:
	हाल FFI_TYPE_SINT128:
	हाल FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_आकार (ty->आकार);
	  स_नकल (p_struct, arg, ty->आकार);
	  val = (uintptr_t)p_struct;
	  जाओ do_int;

	शेष:
	  FFI_ASSERT (0);
	  अवरोध;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

व्योम
ffi_call (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, शून्य);
}

व्योम
ffi_call_go (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
	     व्योम **avalue, व्योम *closure)
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

व्योम FFI_HIDDEN
ffi_closure_helper_SYSV (ffi_cif *cif,
			 व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
			 व्योम *user_तथ्य,
			 अचिन्हित दीर्घ *p_gpr,
			 अचिन्हित दीर्घ दीर्घ *p_fpr,
			 अचिन्हित दीर्घ *p_ov)
{
  अचिन्हित दीर्घ दीर्घ ret_बफर;

  व्योम *rvalue = &ret_बफर;
  व्योम **avalue;
  व्योम **p_arg;

  पूर्णांक n_gpr = 0;
  पूर्णांक n_fpr = 0;
  पूर्णांक n_ov = 0;

  ffi_type **ptr;
  पूर्णांक i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * माप (व्योम *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  अगर (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (व्योम *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  क्रम (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      पूर्णांक deref_struct_pointer = 0;
      पूर्णांक type = (*ptr)->type;

#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      अगर (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#पूर्ण_अगर

      /* Check how a structure type is passed.  */
      अगर (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  अगर (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  अन्यथा
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  अगर (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      अगर (type == FFI_TYPE_POINTER)
	{
#अगर_घोषित __s390x__
	  type = FFI_TYPE_UINT64;
#अन्यथा
	  type = FFI_TYPE_UINT32;
#पूर्ण_अगर
	}

      /* Now handle all primitive int/float data types.  */
      चयन (type)
	{
	  हाल FFI_TYPE_DOUBLE:
	    अगर (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    अन्यथा
	      *p_arg = &p_ov[n_ov],
	      n_ov += माप (द्विगुन) / माप (दीर्घ);
	    अवरोध;

	  हाल FFI_TYPE_FLOAT:
	    अगर (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    अन्यथा
	      *p_arg = (अक्षर *)&p_ov[n_ov++] + माप (दीर्घ) - 4;
	    अवरोध;

	  हाल FFI_TYPE_UINT64:
	  हाल FFI_TYPE_SINT64:
#अगर_घोषित __s390x__
	    अगर (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    अन्यथा
	      *p_arg = &p_ov[n_ov++];
#अन्यथा
	    अगर (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    अगर (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    अन्यथा
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#पूर्ण_अगर
	    अवरोध;

	  हाल FFI_TYPE_INT:
	  हाल FFI_TYPE_UINT32:
	  हाल FFI_TYPE_SINT32:
	    अगर (n_gpr < MAX_GPRARGS)
	      *p_arg = (अक्षर *)&p_gpr[n_gpr++] + माप (दीर्घ) - 4;
	    अन्यथा
	      *p_arg = (अक्षर *)&p_ov[n_ov++] + माप (दीर्घ) - 4;
	    अवरोध;

	  हाल FFI_TYPE_UINT16:
	  हाल FFI_TYPE_SINT16:
	    अगर (n_gpr < MAX_GPRARGS)
	      *p_arg = (अक्षर *)&p_gpr[n_gpr++] + माप (दीर्घ) - 2;
	    अन्यथा
	      *p_arg = (अक्षर *)&p_ov[n_ov++] + माप (दीर्घ) - 2;
	    अवरोध;

	  हाल FFI_TYPE_UINT8:
	  हाल FFI_TYPE_SINT8:
	    अगर (n_gpr < MAX_GPRARGS)
	      *p_arg = (अक्षर *)&p_gpr[n_gpr++] + माप (दीर्घ) - 1;
	    अन्यथा
	      *p_arg = (अक्षर *)&p_ov[n_ov++] + माप (दीर्घ) - 1;
	    अवरोध;

	  शेष:
	    FFI_ASSERT (0);
	    अवरोध;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      अगर (deref_struct_pointer)
	*p_arg = *(व्योम **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_तथ्य);

  /* Convert the return value.  */
  चयन (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      हाल FFI_TYPE_VOID:
      हाल FFI_TYPE_STRUCT:
      हाल FFI_TYPE_COMPLEX:
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      हाल FFI_TYPE_LONGDOUBLE:
#पूर्ण_अगर
	अवरोध;

      /* Floating point values are returned in fpr 0.  */
      हाल FFI_TYPE_FLOAT:
	p_fpr[0] = (दीर्घ दीर्घ) *(अचिन्हित पूर्णांक *) rvalue << 32;
	अवरोध;

      हाल FFI_TYPE_DOUBLE:
	p_fpr[0] = *(अचिन्हित दीर्घ दीर्घ *) rvalue;
	अवरोध;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      हाल FFI_TYPE_UINT64:
      हाल FFI_TYPE_SINT64:
#अगर_घोषित __s390x__
	p_gpr[0] = *(अचिन्हित दीर्घ *) rvalue;
#अन्यथा
	p_gpr[0] = ((अचिन्हित दीर्घ *) rvalue)[0],
	p_gpr[1] = ((अचिन्हित दीर्घ *) rvalue)[1];
#पूर्ण_अगर
	अवरोध;

      हाल FFI_TYPE_POINTER:
      हाल FFI_TYPE_UINT32:
      हाल FFI_TYPE_UINT16:
      हाल FFI_TYPE_UINT8:
	p_gpr[0] = *(अचिन्हित दीर्घ *) rvalue;
	अवरोध;

      हाल FFI_TYPE_INT:
      हाल FFI_TYPE_SINT32:
      हाल FFI_TYPE_SINT16:
      हाल FFI_TYPE_SINT8:
	p_gpr[0] = *(चिन्हित दीर्घ *) rvalue;
	अवरोध;

      शेष:
        FFI_ASSERT (0);
        अवरोध;
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
		      व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  अटल अचिन्हित लघु स्थिर ढाँचा[] = {
    0x0d10,			/* basr %r1,0 */
#अगर_अघोषित __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#अन्यथा
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#पूर्ण_अगर
    0x07f1			/* br %r1 */
  };
  व्योम (*dest)(व्योम);
  अचिन्हित दीर्घ *tramp = (अचिन्हित दीर्घ *)&closure->tramp;

  अगर (cif->abi != FFI_SYSV)
    वापस FFI_BAD_ABI;


#अगर defined(FFI_EXEC_STATIC_TRAMP)
  अगर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      जाओ out;
    }
#पूर्ण_अगर

  स_नकल (tramp, ढाँचा, माप(ढाँचा));
  tramp[2] = (अचिन्हित दीर्घ)codeloc;
  tramp[3] = (अचिन्हित दीर्घ)&ffi_closure_SYSV;

#अगर defined(FFI_EXEC_STATIC_TRAMP)
out:
#पूर्ण_अगर
  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  वापस FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*))
{
  अगर (cif->abi != FFI_SYSV)
    वापस FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  वापस FFI_OK;
}

#अगर defined(FFI_EXEC_STATIC_TRAMP)
व्योम *
ffi_tramp_arch (माप_प्रकार *tramp_आकार, माप_प्रकार *map_आकार)
{
  बाह्य व्योम *trampoline_code_table;

  *tramp_आकार = FFI390_TRAMP_आकार;
  *map_आकार = FFI390_TRAMP_MAP_आकार;
  वापस &trampoline_code_table;
}
#पूर्ण_अगर
