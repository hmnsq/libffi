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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ "internal.h"
#ଅନ୍ତର୍ଭାଵ <tramp.h>

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
#ଅନ୍ୟଥା
#define MAX_FPRARGS 2
#endif

/* Round to multiple of 16.  */
#define ROUND_SIZE(size) (((size) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

ସଂରଚନା call_frame
{
  ଶୂନ୍ୟ *back_chain;
  ଶୂନ୍ୟ *eos;
  unsigned long gpr_args[5];
  unsigned long gpr_save[9];
  unsigned long long fpr_args[4];
};

ବାହ୍ୟ ଶୂନ୍ୟ FFI_HIDDEN ffi_call_SYSV(ସଂରଚନା call_frame *, unsigned, ଶୂନ୍ୟ *,
			             ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *);

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_SYSV(ଶୂନ୍ୟ);
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_SYSV(ଶୂନ୍ୟ);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_check_struct_type (ffi_type *arg)
{
  size_t size = arg->size;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  ଯେତେବେଳେ (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  ବାଛ (size)
    {
      ସ୍ଥିତି 1:
        ଫେରନ୍ତୁ FFI_TYPE_UINT8;

      ସ୍ଥିତି 2:
        ଫେରନ୍ତୁ FFI_TYPE_UINT16;

      ସ୍ଥିତି 4:
	ଯଦି (arg->type == FFI_TYPE_FLOAT)
          ଫେରନ୍ତୁ FFI_TYPE_FLOAT;
	ଅନ୍ୟଥା
	  ଫେରନ୍ତୁ FFI_TYPE_UINT32;

      ସ୍ଥିତି 8:
	ଯଦି (arg->type == FFI_TYPE_DOUBLE)
          ଫେରନ୍ତୁ FFI_TYPE_DOUBLE;
	ଅନ୍ୟଥା
	  ଫେରନ୍ତୁ FFI_TYPE_UINT64;

      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	ଭାଙ୍ଗ;
    }

  /* Other structs are passed via a pointer to the data.  */
  ଫେରନ୍ତୁ FFI_TYPE_POINTER;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_gpr = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_fpr = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ov = 0;

  ffi_type **ptr;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* Determine return value handling.  */

  ବାଛ (cif->rtype->type)
    {
      /* Void is easy.  */
      ସ୍ଥିତି FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	ଭାଙ୍ଗ;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      ସ୍ଥିତି FFI_TYPE_STRUCT:
      ସ୍ଥିତି FFI_TYPE_COMPLEX:
      ସ୍ଥିତି FFI_TYPE_SINT128:
      ସ୍ଥିତି FFI_TYPE_UINT128:
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#endif
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	ଭାଙ୍ଗ;

      /* Floating point values are returned in fpr 0.  */
      ସ୍ଥିତି FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	ଭାଙ୍ଗ;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      ସ୍ଥିତି FFI_TYPE_UINT64:
      ସ୍ଥିତି FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_POINTER:
      ସ୍ଥିତି FFI_TYPE_INT:
      ସ୍ଥିତି FFI_TYPE_UINT32:
      ସ୍ଥିତି FFI_TYPE_SINT32:
      ସ୍ଥିତି FFI_TYPE_UINT16:
      ସ୍ଥିତି FFI_TYPE_SINT16:
      ସ୍ଥିତି FFI_TYPE_UINT8:
      ସ୍ଥିତି FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#ଅନ୍ୟଥା
	cif->flags = FFI390_RET_INT32;
#endif
	ଭାଙ୍ଗ;

      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        FFI_ASSERT (0);
        ଭାଙ୍ଗ;
    }

  /* Now for the arguments.  */

  ପାଇଁ (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା type = (*ptr)->type;

      ବାଛ (type)
	{
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  ଯଦି (type != FFI_TYPE_POINTER)
	    ଭାଙ୍ଗ;
	  /* fall through */

	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	ସ୍ଥିତି FFI_TYPE_SINT128:
	ସ୍ଥିତି FFI_TYPE_UINT128:
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_size += ROUND_SIZE ((*ptr)->size);
	  ଭାଙ୍ଗ;
	}

      /* Now handle all primitive int/float data types.  */
      ବାଛ (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  ସ୍ଥିତି FFI_TYPE_DOUBLE:
	    ଯଦି (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ଅନ୍ୟଥା
	      n_ov += ଆକାର (ଦ୍ୱିଗୁଣ) / ଆକାର (long);
	    ଭାଙ୍ଗ;

	  ସ୍ଥିତି FFI_TYPE_FLOAT:
	    ଯଦି (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ଅନ୍ୟଥା
	      n_ov++;
	    ଭାଙ୍ଗ;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ifndef __s390x__
	  ସ୍ଥିତି FFI_TYPE_UINT64:
	  ସ୍ଥିତି FFI_TYPE_SINT64:
	    ଯଦି (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ଯଦି (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    ଅନ୍ୟଥା
	      n_ov += 2;
	    ଭାଙ୍ଗ;
#endif

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	    ଯଦି (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    ଅନ୍ୟଥା
	      n_ov++;
	    ଭାଙ୍ଗ;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_SIZE (n_ov * ଆକାର (long)) + struct_size;

  ଫେରନ୍ତୁ FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int(ffi_cif *cif,
	     ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ),
	     ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue,
	     ଶୂନ୍ୟ *closure)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret_type = cif->flags;
  size_t rsize = 0, bytes = cif->bytes;
  unsigned ଅକ୍ଷର *ଗଦା, *p_struct;
  ସଂରଚନା call_frame *frame;
  unsigned long *p_ov, *p_gpr;
  unsigned long long *p_fpr;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  ଯଦି (rvalue == ଖାଲି)
    {
      ଯଦି (ret_type & FFI390_RET_IN_MEM)
	rsize = cif->rtype->size;
      ଅନ୍ୟଥା
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

  ଗଦା = alloca (bytes + ଆକାର(ସଂରଚନା call_frame) + rsize);
  frame = (ସଂରଚନା call_frame *)(ଗଦା + bytes);
  ଯଦି (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (unsigned long *)ଗଦା;
  p_struct = (unsigned ଅକ୍ଷର *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  ଯଦି (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  ପାଇଁ (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      ଶୂନ୍ୟ *arg = avalue[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା type = ty->type;
      ffi_arg val;

    restart:
      ବାଛ (type)
	{
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  ଯାଅ do_int;
	ସ୍ଥିତି FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  ଯାଅ do_int;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  ଯାଅ do_int;
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  ଯାଅ do_int;
	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  ଯାଅ do_int;
	ସ୍ଥିତି FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  ଯାଅ do_int;
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
#ifdef __s390x__
	  val = *(UINT64 *)arg;
	  ଯାଅ do_int;
#ଅନ୍ୟଥା
	  ଯଦି (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  ଯଦି (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  ଅନ୍ୟଥା
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  ଯଦି (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  ଅନ୍ୟଥା
	    {
#ifdef __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#ଅନ୍ୟଥା
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	    }
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  ଯଦି (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  ଅନ୍ୟଥା
	    p_ov[n_ov++] = val;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  ଯଦି (type != FFI_TYPE_POINTER)
	    ଯାଅ restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#endif
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	ସ୍ଥିତି FFI_TYPE_SINT128:
	ସ୍ଥିତି FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_SIZE (ty->size);
	  memcpy (p_struct, arg, ty->size);
	  val = (uintptr_t)p_struct;
	  ଯାଅ do_int;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  FFI_ASSERT (0);
	  ଭାଙ୍ଗ;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ଖାଲି);
}

ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
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

ଶୂନ୍ୟ FFI_HIDDEN
ffi_closure_helper_SYSV (ffi_cif *cif,
			 ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
			 ଶୂନ୍ୟ *user_data,
			 unsigned long *p_gpr,
			 unsigned long long *p_fpr,
			 unsigned long *p_ov)
{
  unsigned long long ret_buffer;

  ଶୂନ୍ୟ *rvalue = &ret_buffer;
  ଶୂନ୍ୟ **avalue;
  ଶୂନ୍ୟ **p_arg;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_gpr = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_fpr = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ov = 0;

  ffi_type **ptr;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  ଯଦି (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (ଶୂନ୍ୟ *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  ପାଇଁ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା deref_struct_pointer = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା type = (*ptr)->type;

#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      ଯଦି (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif

      /* Check how a structure type is passed.  */
      ଯଦି (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  ଯଦି (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  ଅନ୍ୟଥା
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  ଯଦି (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      ଯଦି (type == FFI_TYPE_POINTER)
	{
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#ଅନ୍ୟଥା
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      ବାଛ (type)
	{
	  ସ୍ଥିତି FFI_TYPE_DOUBLE:
	    ଯଦି (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ଅନ୍ୟଥା
	      *p_arg = &p_ov[n_ov],
	      n_ov += ଆକାର (ଦ୍ୱିଗୁଣ) / ଆକାର (long);
	    ଭାଙ୍ଗ;

	  ସ୍ଥିତି FFI_TYPE_FLOAT:
	    ଯଦି (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ଅନ୍ୟଥା
	      *p_arg = (ଅକ୍ଷର *)&p_ov[n_ov++] + ଆକାର (long) - 4;
	    ଭାଙ୍ଗ;

	  ସ୍ଥିତି FFI_TYPE_UINT64:
	  ସ୍ଥିତି FFI_TYPE_SINT64:
#ifdef __s390x__
	    ଯଦି (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    ଅନ୍ୟଥା
	      *p_arg = &p_ov[n_ov++];
#ଅନ୍ୟଥା
	    ଯଦି (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ଯଦି (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    ଅନ୍ୟଥା
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#endif
	    ଭାଙ୍ଗ;

	  ସ୍ଥିତି FFI_TYPE_INT:
	  ସ୍ଥିତି FFI_TYPE_UINT32:
	  ସ୍ଥିତି FFI_TYPE_SINT32:
	    ଯଦି (n_gpr < MAX_GPRARGS)
	      *p_arg = (ଅକ୍ଷର *)&p_gpr[n_gpr++] + ଆକାର (long) - 4;
	    ଅନ୍ୟଥା
	      *p_arg = (ଅକ୍ଷର *)&p_ov[n_ov++] + ଆକାର (long) - 4;
	    ଭାଙ୍ଗ;

	  ସ୍ଥିତି FFI_TYPE_UINT16:
	  ସ୍ଥିତି FFI_TYPE_SINT16:
	    ଯଦି (n_gpr < MAX_GPRARGS)
	      *p_arg = (ଅକ୍ଷର *)&p_gpr[n_gpr++] + ଆକାର (long) - 2;
	    ଅନ୍ୟଥା
	      *p_arg = (ଅକ୍ଷର *)&p_ov[n_ov++] + ଆକାର (long) - 2;
	    ଭାଙ୍ଗ;

	  ସ୍ଥିତି FFI_TYPE_UINT8:
	  ସ୍ଥିତି FFI_TYPE_SINT8:
	    ଯଦି (n_gpr < MAX_GPRARGS)
	      *p_arg = (ଅକ୍ଷର *)&p_gpr[n_gpr++] + ଆକାର (long) - 1;
	    ଅନ୍ୟଥା
	      *p_arg = (ଅକ୍ଷର *)&p_ov[n_ov++] + ଆକାର (long) - 1;
	    ଭାଙ୍ଗ;

	  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	    FFI_ASSERT (0);
	    ଭାଙ୍ଗ;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      ଯଦି (deref_struct_pointer)
	*p_arg = *(ଶୂନ୍ୟ **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_data);

  /* Convert the return value.  */
  ବାଛ (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      ସ୍ଥିତି FFI_TYPE_VOID:
      ସ୍ଥିତି FFI_TYPE_STRUCT:
      ସ୍ଥିତି FFI_TYPE_COMPLEX:
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#endif
	ଭାଙ୍ଗ;

      /* Floating point values are returned in fpr 0.  */
      ସ୍ଥିତି FFI_TYPE_FLOAT:
	p_fpr[0] = (long long) *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) rvalue << 32;
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_DOUBLE:
	p_fpr[0] = *(unsigned long long *) rvalue;
	ଭାଙ୍ଗ;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      ସ୍ଥିତି FFI_TYPE_UINT64:
      ସ୍ଥିତି FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(unsigned long *) rvalue;
#ଅନ୍ୟଥା
	p_gpr[0] = ((unsigned long *) rvalue)[0],
	p_gpr[1] = ((unsigned long *) rvalue)[1];
#endif
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_POINTER:
      ସ୍ଥିତି FFI_TYPE_UINT32:
      ସ୍ଥିତି FFI_TYPE_UINT16:
      ସ୍ଥିତି FFI_TYPE_UINT8:
	p_gpr[0] = *(unsigned long *) rvalue;
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_INT:
      ସ୍ଥିତି FFI_TYPE_SINT32:
      ସ୍ଥିତି FFI_TYPE_SINT16:
      ସ୍ଥିତି FFI_TYPE_SINT8:
	p_gpr[0] = *(signed long *) rvalue;
	ଭାଙ୍ଗ;

      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        FFI_ASSERT (0);
        ଭାଙ୍ଗ;
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
		      ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  ସ୍ଥିର unsigned short ଅଚଳ template[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#ଅନ୍ୟଥା
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  ଶୂନ୍ୟ (*dest)(ଶୂନ୍ୟ);
  unsigned long *tramp = (unsigned long *)&closure->tramp;

  ଯଦି (cif->abi != FFI_SYSV)
    ଫେରନ୍ତୁ FFI_BAD_ABI;


#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
  ଯଦି (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      ଯାଅ out;
    }
#endif

  memcpy (tramp, template, ଆକାର(template));
  tramp[2] = (unsigned long)codeloc;
  tramp[3] = (unsigned long)&ffi_closure_SYSV;

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ଫେରନ୍ତୁ FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*))
{
  ଯଦି (cif->abi != FFI_SYSV)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ଶୂନ୍ୟ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ବାହ୍ୟ ଶୂନ୍ୟ *trampoline_code_table;

  *tramp_size = FFI390_TRAMP_SIZE;
  *map_size = FFI390_TRAMP_MAP_SIZE;
  ଫେରନ୍ତୁ &trampoline_code_table;
}
#endif
