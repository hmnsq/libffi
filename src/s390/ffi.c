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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ "internal.h"
#അന്തര്ഭാവ <tramp.h>

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
#അല്ലെങ്കിൽ
#define MAX_FPRARGS 2
#endif

/* Round to multiple of 16.  */
#define ROUND_SIZE(size) (((size) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

ഘടന call_frame
{
  ശൂന്യം *back_chain;
  ശൂന്യം *eos;
  unsigned long gpr_args[5];
  unsigned long gpr_save[9];
  unsigned long long fpr_args[4];
};

ബാഹ്യ ശൂന്യം FFI_HIDDEN ffi_call_SYSV(ഘടന call_frame *, unsigned, ശൂന്യം *,
			             ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *);

ബാഹ്യ ശൂന്യം ffi_closure_SYSV(ശൂന്യം);
ബാഹ്യ ശൂന്യം ffi_go_closure_SYSV(ശൂന്യം);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

സ്ഥിതം പൂർണ്ണസംഖ്യ
ffi_check_struct_type (ffi_type *arg)
{
  size_t size = arg->size;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  ആകുമ്പോൾ (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  തിരഞ്ഞെടുപ്പ് (size)
    {
      അവസ്ഥ 1:
        തിരികെ FFI_TYPE_UINT8;

      അവസ്ഥ 2:
        തിരികെ FFI_TYPE_UINT16;

      അവസ്ഥ 4:
	എങ്കിൽ (arg->type == FFI_TYPE_FLOAT)
          തിരികെ FFI_TYPE_FLOAT;
	അല്ലെങ്കിൽ
	  തിരികെ FFI_TYPE_UINT32;

      അവസ്ഥ 8:
	എങ്കിൽ (arg->type == FFI_TYPE_DOUBLE)
          തിരികെ FFI_TYPE_DOUBLE;
	അല്ലെങ്കിൽ
	  തിരികെ FFI_TYPE_UINT64;

      സ്ഥിരസ്ഥിതി:
	നിർത്തുക;
    }

  /* Other structs are passed via a pointer to the data.  */
  തിരികെ FFI_TYPE_POINTER;
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
  പൂർണ്ണസംഖ്യ n_gpr = 0;
  പൂർണ്ണസംഖ്യ n_fpr = 0;
  പൂർണ്ണസംഖ്യ n_ov = 0;

  ffi_type **ptr;
  പൂർണ്ണസംഖ്യ i;

  /* Determine return value handling.  */

  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
      /* Void is easy.  */
      അവസ്ഥ FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	നിർത്തുക;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      അവസ്ഥ FFI_TYPE_STRUCT:
      അവസ്ഥ FFI_TYPE_COMPLEX:
      അവസ്ഥ FFI_TYPE_SINT128:
      അവസ്ഥ FFI_TYPE_UINT128:
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      അവസ്ഥ FFI_TYPE_LONGDOUBLE:
#endif
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	നിർത്തുക;

      /* Floating point values are returned in fpr 0.  */
      അവസ്ഥ FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	നിർത്തുക;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      അവസ്ഥ FFI_TYPE_UINT64:
      അവസ്ഥ FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_POINTER:
      അവസ്ഥ FFI_TYPE_INT:
      അവസ്ഥ FFI_TYPE_UINT32:
      അവസ്ഥ FFI_TYPE_SINT32:
      അവസ്ഥ FFI_TYPE_UINT16:
      അവസ്ഥ FFI_TYPE_SINT16:
      അവസ്ഥ FFI_TYPE_UINT8:
      അവസ്ഥ FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#അല്ലെങ്കിൽ
	cif->flags = FFI390_RET_INT32;
#endif
	നിർത്തുക;

      സ്ഥിരസ്ഥിതി:
        FFI_ASSERT (0);
        നിർത്തുക;
    }

  /* Now for the arguments.  */

  വേണ്ടി (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      പൂർണ്ണസംഖ്യ type = (*ptr)->type;

      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  എങ്കിൽ (type != FFI_TYPE_POINTER)
	    നിർത്തുക;
	  /* fall through */

	അവസ്ഥ FFI_TYPE_COMPLEX:
	അവസ്ഥ FFI_TYPE_SINT128:
	അവസ്ഥ FFI_TYPE_UINT128:
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_size += ROUND_SIZE ((*ptr)->size);
	  നിർത്തുക;
	}

      /* Now handle all primitive int/float data types.  */
      തിരഞ്ഞെടുപ്പ് (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  അവസ്ഥ FFI_TYPE_DOUBLE:
	    എങ്കിൽ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    അല്ലെങ്കിൽ
	      n_ov += വലുപ്പം (ഇരട്ട) / വലുപ്പം (long);
	    നിർത്തുക;

	  അവസ്ഥ FFI_TYPE_FLOAT:
	    എങ്കിൽ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    അല്ലെങ്കിൽ
	      n_ov++;
	    നിർത്തുക;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ifndef __s390x__
	  അവസ്ഥ FFI_TYPE_UINT64:
	  അവസ്ഥ FFI_TYPE_SINT64:
	    എങ്കിൽ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    എങ്കിൽ (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    അല്ലെങ്കിൽ
	      n_ov += 2;
	    നിർത്തുക;
#endif

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  സ്ഥിരസ്ഥിതി:
	    എങ്കിൽ (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    അല്ലെങ്കിൽ
	      n_ov++;
	    നിർത്തുക;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_SIZE (n_ov * വലുപ്പം (long)) + struct_size;

  തിരികെ FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

സ്ഥിതം ശൂന്യം
ffi_call_int(ffi_cif *cif,
	     ശൂന്യം (*fn)(ശൂന്യം),
	     ശൂന്യം *rvalue,
	     ശൂന്യം **avalue,
	     ശൂന്യം *closure)
{
  പൂർണ്ണസംഖ്യ ret_type = cif->flags;
  size_t rsize = 0, bytes = cif->bytes;
  unsigned അക്ഷരം *അട്ടി, *p_struct;
  ഘടന call_frame *frame;
  unsigned long *p_ov, *p_gpr;
  unsigned long long *p_fpr;
  പൂർണ്ണസംഖ്യ n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  എങ്കിൽ (rvalue == ശൂന്യ)
    {
      എങ്കിൽ (ret_type & FFI390_RET_IN_MEM)
	rsize = cif->rtype->size;
      അല്ലെങ്കിൽ
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

  അട്ടി = alloca (bytes + വലുപ്പം(ഘടന call_frame) + rsize);
  frame = (ഘടന call_frame *)(അട്ടി + bytes);
  എങ്കിൽ (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (unsigned long *)അട്ടി;
  p_struct = (unsigned അക്ഷരം *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  എങ്കിൽ (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  വേണ്ടി (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      ശൂന്യം *arg = avalue[i];
      പൂർണ്ണസംഖ്യ type = ty->type;
      ffi_arg val;

    restart:
      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  പോകുക do_int;
	അവസ്ഥ FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  പോകുക do_int;
	അവസ്ഥ FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  പോകുക do_int;
	അവസ്ഥ FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  പോകുക do_int;
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  പോകുക do_int;
	അവസ്ഥ FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  പോകുക do_int;
	അവസ്ഥ FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
#ifdef __s390x__
	  val = *(UINT64 *)arg;
	  പോകുക do_int;
#അല്ലെങ്കിൽ
	  എങ്കിൽ (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  എങ്കിൽ (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  അല്ലെങ്കിൽ
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  എങ്കിൽ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  അല്ലെങ്കിൽ
	    {
#ifdef __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#അല്ലെങ്കിൽ
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	    }
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  എങ്കിൽ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  അല്ലെങ്കിൽ
	    p_ov[n_ov++] = val;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  എങ്കിൽ (type != FFI_TYPE_POINTER)
	    പോകുക restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
#endif
	അവസ്ഥ FFI_TYPE_COMPLEX:
	അവസ്ഥ FFI_TYPE_SINT128:
	അവസ്ഥ FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_SIZE (ty->size);
	  memcpy (p_struct, arg, ty->size);
	  val = (uintptr_t)p_struct;
	  പോകുക do_int;

	സ്ഥിരസ്ഥിതി:
	  FFI_ASSERT (0);
	  നിർത്തുക;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ശൂന്യ);
}

ശൂന്യം
ffi_call_go (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
	     ശൂന്യം **avalue, ശൂന്യം *closure)
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

ശൂന്യം FFI_HIDDEN
ffi_closure_helper_SYSV (ffi_cif *cif,
			 ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
			 ശൂന്യം *user_data,
			 unsigned long *p_gpr,
			 unsigned long long *p_fpr,
			 unsigned long *p_ov)
{
  unsigned long long ret_buffer;

  ശൂന്യം *rvalue = &ret_buffer;
  ശൂന്യം **avalue;
  ശൂന്യം **p_arg;

  പൂർണ്ണസംഖ്യ n_gpr = 0;
  പൂർണ്ണസംഖ്യ n_fpr = 0;
  പൂർണ്ണസംഖ്യ n_ov = 0;

  ffi_type **ptr;
  പൂർണ്ണസംഖ്യ i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  എങ്കിൽ (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (ശൂന്യം *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  വേണ്ടി (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      പൂർണ്ണസംഖ്യ deref_struct_pointer = 0;
      പൂർണ്ണസംഖ്യ type = (*ptr)->type;

#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      എങ്കിൽ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif

      /* Check how a structure type is passed.  */
      എങ്കിൽ (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  എങ്കിൽ (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  അല്ലെങ്കിൽ
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  എങ്കിൽ (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      എങ്കിൽ (type == FFI_TYPE_POINTER)
	{
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#അല്ലെങ്കിൽ
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      തിരഞ്ഞെടുപ്പ് (type)
	{
	  അവസ്ഥ FFI_TYPE_DOUBLE:
	    എങ്കിൽ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    അല്ലെങ്കിൽ
	      *p_arg = &p_ov[n_ov],
	      n_ov += വലുപ്പം (ഇരട്ട) / വലുപ്പം (long);
	    നിർത്തുക;

	  അവസ്ഥ FFI_TYPE_FLOAT:
	    എങ്കിൽ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    അല്ലെങ്കിൽ
	      *p_arg = (അക്ഷരം *)&p_ov[n_ov++] + വലുപ്പം (long) - 4;
	    നിർത്തുക;

	  അവസ്ഥ FFI_TYPE_UINT64:
	  അവസ്ഥ FFI_TYPE_SINT64:
#ifdef __s390x__
	    എങ്കിൽ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    അല്ലെങ്കിൽ
	      *p_arg = &p_ov[n_ov++];
#അല്ലെങ്കിൽ
	    എങ്കിൽ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    എങ്കിൽ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    അല്ലെങ്കിൽ
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#endif
	    നിർത്തുക;

	  അവസ്ഥ FFI_TYPE_INT:
	  അവസ്ഥ FFI_TYPE_UINT32:
	  അവസ്ഥ FFI_TYPE_SINT32:
	    എങ്കിൽ (n_gpr < MAX_GPRARGS)
	      *p_arg = (അക്ഷരം *)&p_gpr[n_gpr++] + വലുപ്പം (long) - 4;
	    അല്ലെങ്കിൽ
	      *p_arg = (അക്ഷരം *)&p_ov[n_ov++] + വലുപ്പം (long) - 4;
	    നിർത്തുക;

	  അവസ്ഥ FFI_TYPE_UINT16:
	  അവസ്ഥ FFI_TYPE_SINT16:
	    എങ്കിൽ (n_gpr < MAX_GPRARGS)
	      *p_arg = (അക്ഷരം *)&p_gpr[n_gpr++] + വലുപ്പം (long) - 2;
	    അല്ലെങ്കിൽ
	      *p_arg = (അക്ഷരം *)&p_ov[n_ov++] + വലുപ്പം (long) - 2;
	    നിർത്തുക;

	  അവസ്ഥ FFI_TYPE_UINT8:
	  അവസ്ഥ FFI_TYPE_SINT8:
	    എങ്കിൽ (n_gpr < MAX_GPRARGS)
	      *p_arg = (അക്ഷരം *)&p_gpr[n_gpr++] + വലുപ്പം (long) - 1;
	    അല്ലെങ്കിൽ
	      *p_arg = (അക്ഷരം *)&p_ov[n_ov++] + വലുപ്പം (long) - 1;
	    നിർത്തുക;

	  സ്ഥിരസ്ഥിതി:
	    FFI_ASSERT (0);
	    നിർത്തുക;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      എങ്കിൽ (deref_struct_pointer)
	*p_arg = *(ശൂന്യം **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_data);

  /* Convert the return value.  */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      അവസ്ഥ FFI_TYPE_VOID:
      അവസ്ഥ FFI_TYPE_STRUCT:
      അവസ്ഥ FFI_TYPE_COMPLEX:
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      അവസ്ഥ FFI_TYPE_LONGDOUBLE:
#endif
	നിർത്തുക;

      /* Floating point values are returned in fpr 0.  */
      അവസ്ഥ FFI_TYPE_FLOAT:
	p_fpr[0] = (long long) *(unsigned പൂർണ്ണസംഖ്യ *) rvalue << 32;
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_DOUBLE:
	p_fpr[0] = *(unsigned long long *) rvalue;
	നിർത്തുക;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      അവസ്ഥ FFI_TYPE_UINT64:
      അവസ്ഥ FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(unsigned long *) rvalue;
#അല്ലെങ്കിൽ
	p_gpr[0] = ((unsigned long *) rvalue)[0],
	p_gpr[1] = ((unsigned long *) rvalue)[1];
#endif
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_POINTER:
      അവസ്ഥ FFI_TYPE_UINT32:
      അവസ്ഥ FFI_TYPE_UINT16:
      അവസ്ഥ FFI_TYPE_UINT8:
	p_gpr[0] = *(unsigned long *) rvalue;
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_INT:
      അവസ്ഥ FFI_TYPE_SINT32:
      അവസ്ഥ FFI_TYPE_SINT16:
      അവസ്ഥ FFI_TYPE_SINT8:
	p_gpr[0] = *(signed long *) rvalue;
	നിർത്തുക;

      സ്ഥിരസ്ഥിതി:
        FFI_ASSERT (0);
        നിർത്തുക;
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
		      ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  സ്ഥിതം unsigned short സ്ഥിരം template[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#അല്ലെങ്കിൽ
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  ശൂന്യം (*dest)(ശൂന്യം);
  unsigned long *tramp = (unsigned long *)&closure->tramp;

  എങ്കിൽ (cif->abi != FFI_SYSV)
    തിരികെ FFI_BAD_ABI;


#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
  എങ്കിൽ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      പോകുക out;
    }
#endif

  memcpy (tramp, template, വലുപ്പം(template));
  tramp[2] = (unsigned long)codeloc;
  tramp[3] = (unsigned long)&ffi_closure_SYSV;

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  തിരികെ FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*))
{
  എങ്കിൽ (cif->abi != FFI_SYSV)
    തിരികെ FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  തിരികെ FFI_OK;
}

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
ശൂന്യം *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ബാഹ്യ ശൂന്യം *trampoline_code_table;

  *tramp_size = FFI390_TRAMP_SIZE;
  *map_size = FFI390_TRAMP_MAP_SIZE;
  തിരികെ &trampoline_code_table;
}
#endif
