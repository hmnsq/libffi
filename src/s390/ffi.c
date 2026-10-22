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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ "internal.h"
#અન્તર્ભાવ <tramp.h>

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
#નહીંતર
#define MAX_FPRARGS 2
#endif

/* Round to multiple of 16.  */
#define ROUND_SIZE(size) (((size) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

સંરચના call_frame
{
  શૂન્ય *back_chain;
  શૂન્ય *eos;
  unsigned long gpr_args[5];
  unsigned long gpr_save[9];
  unsigned long long fpr_args[4];
};

બાહ્ય શૂન્ય FFI_HIDDEN ffi_call_SYSV(સંરચના call_frame *, unsigned, શૂન્ય *,
			             શૂન્ય (*fn)(શૂન્ય), શૂન્ય *);

બાહ્ય શૂન્ય ffi_closure_SYSV(શૂન્ય);
બાહ્ય શૂન્ય ffi_go_closure_SYSV(શૂન્ય);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

સ્થિર પૂર્ણાંક
ffi_check_struct_type (ffi_type *arg)
{
  size_t size = arg->size;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  જ્યારે (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  પસંદગી (size)
    {
      સ્થિતિ 1:
        પાછા FFI_TYPE_UINT8;

      સ્થિતિ 2:
        પાછા FFI_TYPE_UINT16;

      સ્થિતિ 4:
	જો (arg->type == FFI_TYPE_FLOAT)
          પાછા FFI_TYPE_FLOAT;
	નહીંતર
	  પાછા FFI_TYPE_UINT32;

      સ્થિતિ 8:
	જો (arg->type == FFI_TYPE_DOUBLE)
          પાછા FFI_TYPE_DOUBLE;
	નહીંતર
	  પાછા FFI_TYPE_UINT64;

      મૂળભૂત:
	તોડો;
    }

  /* Other structs are passed via a pointer to the data.  */
  પાછા FFI_TYPE_POINTER;
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
  પૂર્ણાંક n_gpr = 0;
  પૂર્ણાંક n_fpr = 0;
  પૂર્ણાંક n_ov = 0;

  ffi_type **ptr;
  પૂર્ણાંક i;

  /* Determine return value handling.  */

  પસંદગી (cif->rtype->type)
    {
      /* Void is easy.  */
      સ્થિતિ FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	તોડો;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      સ્થિતિ FFI_TYPE_STRUCT:
      સ્થિતિ FFI_TYPE_COMPLEX:
      સ્થિતિ FFI_TYPE_SINT128:
      સ્થિતિ FFI_TYPE_UINT128:
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      સ્થિતિ FFI_TYPE_LONGDOUBLE:
#endif
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	તોડો;

      /* Floating point values are returned in fpr 0.  */
      સ્થિતિ FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	તોડો;

      સ્થિતિ FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	તોડો;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      સ્થિતિ FFI_TYPE_UINT64:
      સ્થિતિ FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	તોડો;

      સ્થિતિ FFI_TYPE_POINTER:
      સ્થિતિ FFI_TYPE_INT:
      સ્થિતિ FFI_TYPE_UINT32:
      સ્થિતિ FFI_TYPE_SINT32:
      સ્થિતિ FFI_TYPE_UINT16:
      સ્થિતિ FFI_TYPE_SINT16:
      સ્થિતિ FFI_TYPE_UINT8:
      સ્થિતિ FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#નહીંતર
	cif->flags = FFI390_RET_INT32;
#endif
	તોડો;

      મૂળભૂત:
        FFI_ASSERT (0);
        તોડો;
    }

  /* Now for the arguments.  */

  માટે (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      પૂર્ણાંક type = (*ptr)->type;

      પસંદગી (type)
	{
	સ્થિતિ FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  જો (type != FFI_TYPE_POINTER)
	    તોડો;
	  /* fall through */

	સ્થિતિ FFI_TYPE_COMPLEX:
	સ્થિતિ FFI_TYPE_SINT128:
	સ્થિતિ FFI_TYPE_UINT128:
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_size += ROUND_SIZE ((*ptr)->size);
	  તોડો;
	}

      /* Now handle all primitive int/float data types.  */
      પસંદગી (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  સ્થિતિ FFI_TYPE_DOUBLE:
	    જો (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    નહીંતર
	      n_ov += કદ (દ્વિગુણ) / કદ (long);
	    તોડો;

	  સ્થિતિ FFI_TYPE_FLOAT:
	    જો (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    નહીંતર
	      n_ov++;
	    તોડો;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ifndef __s390x__
	  સ્થિતિ FFI_TYPE_UINT64:
	  સ્થિતિ FFI_TYPE_SINT64:
	    જો (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    જો (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    નહીંતર
	      n_ov += 2;
	    તોડો;
#endif

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  મૂળભૂત:
	    જો (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    નહીંતર
	      n_ov++;
	    તોડો;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_SIZE (n_ov * કદ (long)) + struct_size;

  પાછા FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

સ્થિર શૂન્ય
ffi_call_int(ffi_cif *cif,
	     શૂન્ય (*fn)(શૂન્ય),
	     શૂન્ય *rvalue,
	     શૂન્ય **avalue,
	     શૂન્ય *closure)
{
  પૂર્ણાંક ret_type = cif->flags;
  size_t rsize = 0, bytes = cif->bytes;
  unsigned અક્ષર *ઢગલો, *p_struct;
  સંરચના call_frame *frame;
  unsigned long *p_ov, *p_gpr;
  unsigned long long *p_fpr;
  પૂર્ણાંક n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  જો (rvalue == ખાલી)
    {
      જો (ret_type & FFI390_RET_IN_MEM)
	rsize = cif->rtype->size;
      નહીંતર
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

  ઢગલો = alloca (bytes + કદ(સંરચના call_frame) + rsize);
  frame = (સંરચના call_frame *)(ઢગલો + bytes);
  જો (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (unsigned long *)ઢગલો;
  p_struct = (unsigned અક્ષર *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  જો (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  માટે (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      શૂન્ય *arg = avalue[i];
      પૂર્ણાંક type = ty->type;
      ffi_arg val;

    restart:
      પસંદગી (type)
	{
	સ્થિતિ FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  જાઓ do_int;
	સ્થિતિ FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  જાઓ do_int;
	સ્થિતિ FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  જાઓ do_int;
	સ્થિતિ FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  જાઓ do_int;
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  જાઓ do_int;
	સ્થિતિ FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  જાઓ do_int;
	સ્થિતિ FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
#ifdef __s390x__
	  val = *(UINT64 *)arg;
	  જાઓ do_int;
#નહીંતર
	  જો (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  જો (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  નહીંતર
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	  જો (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  નહીંતર
	    {
#ifdef __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#નહીંતર
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	    }
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  જો (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  નહીંતર
	    p_ov[n_ov++] = val;
	  તોડો;

	સ્થિતિ FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  જો (type != FFI_TYPE_POINTER)
	    જાઓ restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
#endif
	સ્થિતિ FFI_TYPE_COMPLEX:
	સ્થિતિ FFI_TYPE_SINT128:
	સ્થિતિ FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_SIZE (ty->size);
	  memcpy (p_struct, arg, ty->size);
	  val = (uintptr_t)p_struct;
	  જાઓ do_int;

	મૂળભૂત:
	  FFI_ASSERT (0);
	  તોડો;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ખાલી);
}

શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
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

શૂન્ય FFI_HIDDEN
ffi_closure_helper_SYSV (ffi_cif *cif,
			 શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
			 શૂન્ય *user_data,
			 unsigned long *p_gpr,
			 unsigned long long *p_fpr,
			 unsigned long *p_ov)
{
  unsigned long long ret_buffer;

  શૂન્ય *rvalue = &ret_buffer;
  શૂન્ય **avalue;
  શૂન્ય **p_arg;

  પૂર્ણાંક n_gpr = 0;
  પૂર્ણાંક n_fpr = 0;
  પૂર્ણાંક n_ov = 0;

  ffi_type **ptr;
  પૂર્ણાંક i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * કદ (શૂન્ય *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  જો (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (શૂન્ય *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  માટે (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      પૂર્ણાંક deref_struct_pointer = 0;
      પૂર્ણાંક type = (*ptr)->type;

#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      જો (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif

      /* Check how a structure type is passed.  */
      જો (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  જો (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  નહીંતર
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  જો (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      જો (type == FFI_TYPE_POINTER)
	{
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#નહીંતર
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      પસંદગી (type)
	{
	  સ્થિતિ FFI_TYPE_DOUBLE:
	    જો (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    નહીંતર
	      *p_arg = &p_ov[n_ov],
	      n_ov += કદ (દ્વિગુણ) / કદ (long);
	    તોડો;

	  સ્થિતિ FFI_TYPE_FLOAT:
	    જો (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    નહીંતર
	      *p_arg = (અક્ષર *)&p_ov[n_ov++] + કદ (long) - 4;
	    તોડો;

	  સ્થિતિ FFI_TYPE_UINT64:
	  સ્થિતિ FFI_TYPE_SINT64:
#ifdef __s390x__
	    જો (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    નહીંતર
	      *p_arg = &p_ov[n_ov++];
#નહીંતર
	    જો (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    જો (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    નહીંતર
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#endif
	    તોડો;

	  સ્થિતિ FFI_TYPE_INT:
	  સ્થિતિ FFI_TYPE_UINT32:
	  સ્થિતિ FFI_TYPE_SINT32:
	    જો (n_gpr < MAX_GPRARGS)
	      *p_arg = (અક્ષર *)&p_gpr[n_gpr++] + કદ (long) - 4;
	    નહીંતર
	      *p_arg = (અક્ષર *)&p_ov[n_ov++] + કદ (long) - 4;
	    તોડો;

	  સ્થિતિ FFI_TYPE_UINT16:
	  સ્થિતિ FFI_TYPE_SINT16:
	    જો (n_gpr < MAX_GPRARGS)
	      *p_arg = (અક્ષર *)&p_gpr[n_gpr++] + કદ (long) - 2;
	    નહીંતર
	      *p_arg = (અક્ષર *)&p_ov[n_ov++] + કદ (long) - 2;
	    તોડો;

	  સ્થિતિ FFI_TYPE_UINT8:
	  સ્થિતિ FFI_TYPE_SINT8:
	    જો (n_gpr < MAX_GPRARGS)
	      *p_arg = (અક્ષર *)&p_gpr[n_gpr++] + કદ (long) - 1;
	    નહીંતર
	      *p_arg = (અક્ષર *)&p_ov[n_ov++] + કદ (long) - 1;
	    તોડો;

	  મૂળભૂત:
	    FFI_ASSERT (0);
	    તોડો;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      જો (deref_struct_pointer)
	*p_arg = *(શૂન્ય **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_data);

  /* Convert the return value.  */
  પસંદગી (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      સ્થિતિ FFI_TYPE_VOID:
      સ્થિતિ FFI_TYPE_STRUCT:
      સ્થિતિ FFI_TYPE_COMPLEX:
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      સ્થિતિ FFI_TYPE_LONGDOUBLE:
#endif
	તોડો;

      /* Floating point values are returned in fpr 0.  */
      સ્થિતિ FFI_TYPE_FLOAT:
	p_fpr[0] = (long long) *(unsigned પૂર્ણાંક *) rvalue << 32;
	તોડો;

      સ્થિતિ FFI_TYPE_DOUBLE:
	p_fpr[0] = *(unsigned long long *) rvalue;
	તોડો;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      સ્થિતિ FFI_TYPE_UINT64:
      સ્થિતિ FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(unsigned long *) rvalue;
#નહીંતર
	p_gpr[0] = ((unsigned long *) rvalue)[0],
	p_gpr[1] = ((unsigned long *) rvalue)[1];
#endif
	તોડો;

      સ્થિતિ FFI_TYPE_POINTER:
      સ્થિતિ FFI_TYPE_UINT32:
      સ્થિતિ FFI_TYPE_UINT16:
      સ્થિતિ FFI_TYPE_UINT8:
	p_gpr[0] = *(unsigned long *) rvalue;
	તોડો;

      સ્થિતિ FFI_TYPE_INT:
      સ્થિતિ FFI_TYPE_SINT32:
      સ્થિતિ FFI_TYPE_SINT16:
      સ્થિતિ FFI_TYPE_SINT8:
	p_gpr[0] = *(signed long *) rvalue;
	તોડો;

      મૂળભૂત:
        FFI_ASSERT (0);
        તોડો;
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
		      શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  સ્થિર unsigned short અચલ template[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#નહીંતર
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  શૂન્ય (*dest)(શૂન્ય);
  unsigned long *tramp = (unsigned long *)&closure->tramp;

  જો (cif->abi != FFI_SYSV)
    પાછા FFI_BAD_ABI;


#જો defined(FFI_EXEC_STATIC_TRAMP)
  જો (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      જાઓ out;
    }
#endif

  memcpy (tramp, template, કદ(template));
  tramp[2] = (unsigned long)codeloc;
  tramp[3] = (unsigned long)&ffi_closure_SYSV;

#જો defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  પાછા FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*))
{
  જો (cif->abi != FFI_SYSV)
    પાછા FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}

#જો defined(FFI_EXEC_STATIC_TRAMP)
શૂન્ય *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  બાહ્ય શૂન્ય *trampoline_code_table;

  *tramp_size = FFI390_TRAMP_SIZE;
  *map_size = FFI390_TRAMP_MAP_SIZE;
  પાછા &trampoline_code_table;
}
#endif
