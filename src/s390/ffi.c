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
  unsigned long gpr_args[5];
  unsigned long gpr_save[9];
  unsigned long long fpr_args[4];
};

बाह्य शून्य FFI_HIDDEN ffi_call_SYSV(संरचना call_frame *, unsigned, शून्य *,
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

स्थिर पूर्णाङ्क
ffi_check_struct_type (ffi_type *arg)
{
  size_t size = arg->size;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  यावत् (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  विकल्प (size)
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

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t struct_size = 0;
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
#endif
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
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#अन्यथा
	cif->flags = FFI390_RET_INT32;
#endif
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
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_size += ROUND_SIZE ((*ptr)->size);
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
	      n_ov += परिमाण (द्विगुण) / परिमाण (long);
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

#ifndef __s390x__
	  पक्ष FFI_TYPE_UINT64:
	  पक्ष FFI_TYPE_SINT64:
	    यदि (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    यदि (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    अन्यथा
	      n_ov += 2;
	    भङ्ग;
#endif

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

  cif->bytes = ROUND_SIZE (n_ov * परिमाण (long)) + struct_size;

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
	     शून्य *closure)
{
  पूर्णाङ्क ret_type = cif->flags;
  size_t rsize = 0, bytes = cif->bytes;
  unsigned वर्ण *ढेर, *p_struct;
  संरचना call_frame *frame;
  unsigned long *p_ov, *p_gpr;
  unsigned long long *p_fpr;
  पूर्णाङ्क n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  यदि (rvalue == रिक्त)
    {
      यदि (ret_type & FFI390_RET_IN_MEM)
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

  ढेर = alloca (bytes + परिमाण(संरचना call_frame) + rsize);
  frame = (संरचना call_frame *)(ढेर + bytes);
  यदि (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (unsigned long *)ढेर;
  p_struct = (unsigned वर्ण *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  यदि (cif->flags & FFI390_RET_IN_MEM)
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
#ifdef __s390x__
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
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  यदि (n_fpr < MAX_FPRARGS)
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
#endif
	पक्ष FFI_TYPE_COMPLEX:
	पक्ष FFI_TYPE_SINT128:
	पक्ष FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_SIZE (ty->size);
	  memcpy (p_struct, arg, ty->size);
	  val = (uintptr_t)p_struct;
	  गच्छ do_int;

	स्वभाव:
	  FFI_ASSERT (0);
	  भङ्ग;
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
			 unsigned long *p_gpr,
			 unsigned long long *p_fpr,
			 unsigned long *p_ov)
{
  unsigned long long ret_buffer;

  शून्य *rvalue = &ret_buffer;
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
  यदि (cif->flags & FFI390_RET_IN_MEM)
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
#endif

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
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#अन्यथा
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      विकल्प (type)
	{
	  पक्ष FFI_TYPE_DOUBLE:
	    यदि (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    अन्यथा
	      *p_arg = &p_ov[n_ov],
	      n_ov += परिमाण (द्विगुण) / परिमाण (long);
	    भङ्ग;

	  पक्ष FFI_TYPE_FLOAT:
	    यदि (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    अन्यथा
	      *p_arg = (वर्ण *)&p_ov[n_ov++] + परिमाण (long) - 4;
	    भङ्ग;

	  पक्ष FFI_TYPE_UINT64:
	  पक्ष FFI_TYPE_SINT64:
#ifdef __s390x__
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
#endif
	    भङ्ग;

	  पक्ष FFI_TYPE_INT:
	  पक्ष FFI_TYPE_UINT32:
	  पक्ष FFI_TYPE_SINT32:
	    यदि (n_gpr < MAX_GPRARGS)
	      *p_arg = (वर्ण *)&p_gpr[n_gpr++] + परिमाण (long) - 4;
	    अन्यथा
	      *p_arg = (वर्ण *)&p_ov[n_ov++] + परिमाण (long) - 4;
	    भङ्ग;

	  पक्ष FFI_TYPE_UINT16:
	  पक्ष FFI_TYPE_SINT16:
	    यदि (n_gpr < MAX_GPRARGS)
	      *p_arg = (वर्ण *)&p_gpr[n_gpr++] + परिमाण (long) - 2;
	    अन्यथा
	      *p_arg = (वर्ण *)&p_ov[n_ov++] + परिमाण (long) - 2;
	    भङ्ग;

	  पक्ष FFI_TYPE_UINT8:
	  पक्ष FFI_TYPE_SINT8:
	    यदि (n_gpr < MAX_GPRARGS)
	      *p_arg = (वर्ण *)&p_gpr[n_gpr++] + परिमाण (long) - 1;
	    अन्यथा
	      *p_arg = (वर्ण *)&p_ov[n_ov++] + परिमाण (long) - 1;
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
  (fun) (cif, rvalue, avalue, user_data);

  /* Convert the return value.  */
  विकल्प (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      पक्ष FFI_TYPE_VOID:
      पक्ष FFI_TYPE_STRUCT:
      पक्ष FFI_TYPE_COMPLEX:
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      पक्ष FFI_TYPE_LONGDOUBLE:
#endif
	भङ्ग;

      /* Floating point values are returned in fpr 0.  */
      पक्ष FFI_TYPE_FLOAT:
	p_fpr[0] = (long long) *(unsigned पूर्णाङ्क *) rvalue << 32;
	भङ्ग;

      पक्ष FFI_TYPE_DOUBLE:
	p_fpr[0] = *(unsigned long long *) rvalue;
	भङ्ग;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      पक्ष FFI_TYPE_UINT64:
      पक्ष FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(unsigned long *) rvalue;
#अन्यथा
	p_gpr[0] = ((unsigned long *) rvalue)[0],
	p_gpr[1] = ((unsigned long *) rvalue)[1];
#endif
	भङ्ग;

      पक्ष FFI_TYPE_POINTER:
      पक्ष FFI_TYPE_UINT32:
      पक्ष FFI_TYPE_UINT16:
      पक्ष FFI_TYPE_UINT8:
	p_gpr[0] = *(unsigned long *) rvalue;
	भङ्ग;

      पक्ष FFI_TYPE_INT:
      पक्ष FFI_TYPE_SINT32:
      पक्ष FFI_TYPE_SINT16:
      पक्ष FFI_TYPE_SINT8:
	p_gpr[0] = *(signed long *) rvalue;
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

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  स्थिर unsigned short ध्रुव template[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#अन्यथा
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  शून्य (*dest)(शून्य);
  unsigned long *tramp = (unsigned long *)&closure->tramp;

  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;


#यदि defined(FFI_EXEC_STATIC_TRAMP)
  यदि (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      गच्छ out;
    }
#endif

  memcpy (tramp, template, परिमाण(template));
  tramp[2] = (unsigned long)codeloc;
  tramp[3] = (unsigned long)&ffi_closure_SYSV;

#यदि defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  प्रत्यागम FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*))
{
  यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  प्रत्यागम FFI_OK;
}

#यदि defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *tramp_size = FFI390_TRAMP_SIZE;
  *map_size = FFI390_TRAMP_MAP_SIZE;
  प्रत्यागम &trampoline_code_table;
}
#endif
