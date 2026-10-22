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

#include <ffi.h>
#include <ffi_common.h>
#include <stdint.h>
#include "internal.h"
#include <tramp.h>

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
#ᯀᯮᯝᯂᯪ
#define MAX_FPRARGS 2
#endif

/* Round to multiple of 16.  */
#define ROUND_SIZE(size) (((size) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

sa_nracanaa call_frame
{
  void *back_chain;
  void *eos;
  unsigned long gpr_args[5];
  unsigned long gpr_save[9];
  unsigned long long fpr_args[4];
};

extern void FFI_HIDDEN ffi_call_SYSV(sa_nracanaa call_frame *, unsigned, void *,
			             void (*fn)(void), void *);

extern void ffi_closure_SYSV(void);
extern void ffi_go_closure_SYSV(void);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

static puur_naa_nka
ffi_check_struct_type (ffi_type *arg)
{
  size_t size = arg->size;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  ᯀᯬᯝ (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  switch (size)
    {
      pak_sa 1:
        ᯀᯮᯞ FFI_TYPE_UINT8;

      pak_sa 2:
        ᯀᯮᯞ FFI_TYPE_UINT16;

      pak_sa 4:
	ᯀᯮᯝ (arg->type == FFI_TYPE_FLOAT)
          ᯀᯮᯞ FFI_TYPE_FLOAT;
	ᯀᯮᯝᯂᯪ
	  ᯀᯮᯞ FFI_TYPE_UINT32;

      pak_sa 8:
	ᯀᯮᯝ (arg->type == FFI_TYPE_DOUBLE)
          ᯀᯮᯞ FFI_TYPE_DOUBLE;
	ᯀᯮᯝᯂᯪ
	  ᯀᯮᯞ FFI_TYPE_UINT64;

      default:
	bha_nga;
    }

  /* Other structs are passed via a pointer to the data.  */
  ᯀᯮᯞ FFI_TYPE_POINTER;
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
  puur_naa_nka n_gpr = 0;
  puur_naa_nka n_fpr = 0;
  puur_naa_nka n_ov = 0;

  ffi_type **ptr;
  puur_naa_nka i;

  /* Determine return value handling.  */

  switch (cif->rtype->type)
    {
      /* Void is easy.  */
      pak_sa FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	bha_nga;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      pak_sa FFI_TYPE_STRUCT:
      pak_sa FFI_TYPE_COMPLEX:
      pak_sa FFI_TYPE_SINT128:
      pak_sa FFI_TYPE_UINT128:
#ᯀᯮᯝ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      pak_sa FFI_TYPE_LONGDOUBLE:
#endif
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	bha_nga;

      /* Floating point values are returned in fpr 0.  */
      pak_sa FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	bha_nga;

      pak_sa FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	bha_nga;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      pak_sa FFI_TYPE_UINT64:
      pak_sa FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	bha_nga;

      pak_sa FFI_TYPE_POINTER:
      pak_sa FFI_TYPE_INT:
      pak_sa FFI_TYPE_UINT32:
      pak_sa FFI_TYPE_SINT32:
      pak_sa FFI_TYPE_UINT16:
      pak_sa FFI_TYPE_SINT16:
      pak_sa FFI_TYPE_UINT8:
      pak_sa FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#ᯀᯮᯝᯂᯪ
	cif->flags = FFI390_RET_INT32;
#endif
	bha_nga;

      default:
        FFI_ASSERT (0);
        bha_nga;
    }

  /* Now for the arguments.  */

  for (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      puur_naa_nka type = (*ptr)->type;

      switch (type)
	{
	pak_sa FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  ᯀᯮᯝ (type != FFI_TYPE_POINTER)
	    bha_nga;
	  /* fall through */

	pak_sa FFI_TYPE_COMPLEX:
	pak_sa FFI_TYPE_SINT128:
	pak_sa FFI_TYPE_UINT128:
#ᯀᯮᯝ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	pak_sa FFI_TYPE_LONGDOUBLE:
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_size += ROUND_SIZE ((*ptr)->size);
	  bha_nga;
	}

      /* Now handle all primitive int/float data types.  */
      switch (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  pak_sa FFI_TYPE_DOUBLE:
	    ᯀᯮᯝ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ᯀᯮᯝᯂᯪ
	      n_ov += parimaa_na (dviguu_na) / parimaa_na (long);
	    bha_nga;

	  pak_sa FFI_TYPE_FLOAT:
	    ᯀᯮᯝ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ᯀᯮᯝᯂᯪ
	      n_ov++;
	    bha_nga;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ifndef __s390x__
	  pak_sa FFI_TYPE_UINT64:
	  pak_sa FFI_TYPE_SINT64:
	    ᯀᯮᯝ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ᯀᯮᯝ (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    ᯀᯮᯝᯂᯪ
	      n_ov += 2;
	    bha_nga;
#endif

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  default:
	    ᯀᯮᯝ (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    ᯀᯮᯝᯂᯪ
	      n_ov++;
	    bha_nga;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_SIZE (n_ov * parimaa_na (long)) + struct_size;

  ᯀᯮᯞ FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

static void
ffi_call_int(ffi_cif *cif,
	     void (*fn)(void),
	     void *rvalue,
	     void **avalue,
	     void *closure)
{
  puur_naa_nka ret_type = cif->flags;
  size_t rsize = 0, bytes = cif->bytes;
  unsigned var_na *stack, *p_struct;
  sa_nracanaa call_frame *frame;
  unsigned long *p_ov, *p_gpr;
  unsigned long long *p_fpr;
  puur_naa_nka n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  ᯀᯮᯝ (rvalue == NULL)
    {
      ᯀᯮᯝ (ret_type & FFI390_RET_IN_MEM)
	rsize = cif->rtype->size;
      ᯀᯮᯝᯂᯪ
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

  stack = alloca (bytes + parimaa_na(sa_nracanaa call_frame) + rsize);
  frame = (sa_nracanaa call_frame *)(stack + bytes);
  ᯀᯮᯝ (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (unsigned long *)stack;
  p_struct = (unsigned var_na *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  ᯀᯮᯝ (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  for (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      void *arg = avalue[i];
      puur_naa_nka type = ty->type;
      ffi_arg val;

    restart:
      switch (type)
	{
	pak_sa FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  goto do_int;
	pak_sa FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  goto do_int;
	pak_sa FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  goto do_int;
	pak_sa FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  goto do_int;
	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  goto do_int;
	pak_sa FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  goto do_int;
	pak_sa FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  bha_nga;

	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_SINT64:
#ifdef __s390x__
	  val = *(UINT64 *)arg;
	  goto do_int;
#ᯀᯮᯝᯂᯪ
	  ᯀᯮᯝ (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  ᯀᯮᯝ (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  ᯀᯮᯝᯂᯪ
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	  bha_nga;

	pak_sa FFI_TYPE_DOUBLE:
	  ᯀᯮᯝ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  ᯀᯮᯝᯂᯪ
	    {
#ifdef __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#ᯀᯮᯝᯂᯪ
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	    }
	  bha_nga;

	pak_sa FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  ᯀᯮᯝ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  ᯀᯮᯝᯂᯪ
	    p_ov[n_ov++] = val;
	  bha_nga;

	pak_sa FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  ᯀᯮᯝ (type != FFI_TYPE_POINTER)
	    goto restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#ᯀᯮᯝ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	pak_sa FFI_TYPE_LONGDOUBLE:
#endif
	pak_sa FFI_TYPE_COMPLEX:
	pak_sa FFI_TYPE_SINT128:
	pak_sa FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_SIZE (ty->size);
	  memcpy (p_struct, arg, ty->size);
	  val = (uintptr_t)p_struct;
	  goto do_int;

	default:
	  FFI_ASSERT (0);
	  bha_nga;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

void
ffi_call (ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, NULL);
}

void
ffi_call_go (ffi_cif *cif, void (*fn)(void), void *rvalue,
	     void **avalue, void *closure)
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

void FFI_HIDDEN
ffi_closure_helper_SYSV (ffi_cif *cif,
			 void (*fun)(ffi_cif*,void*,void**,void*),
			 void *user_data,
			 unsigned long *p_gpr,
			 unsigned long long *p_fpr,
			 unsigned long *p_ov)
{
  unsigned long long ret_buffer;

  void *rvalue = &ret_buffer;
  void **avalue;
  void **p_arg;

  puur_naa_nka n_gpr = 0;
  puur_naa_nka n_fpr = 0;
  puur_naa_nka n_ov = 0;

  ffi_type **ptr;
  puur_naa_nka i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * parimaa_na (void *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  ᯀᯮᯝ (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (void *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  for (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      puur_naa_nka deref_struct_pointer = 0;
      puur_naa_nka type = (*ptr)->type;

#ᯀᯮᯝ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      ᯀᯮᯝ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif

      /* Check how a structure type is passed.  */
      ᯀᯮᯝ (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  ᯀᯮᯝ (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  ᯀᯮᯝᯂᯪ
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  ᯀᯮᯝ (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      ᯀᯮᯝ (type == FFI_TYPE_POINTER)
	{
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#ᯀᯮᯝᯂᯪ
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      switch (type)
	{
	  pak_sa FFI_TYPE_DOUBLE:
	    ᯀᯮᯝ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ᯀᯮᯝᯂᯪ
	      *p_arg = &p_ov[n_ov],
	      n_ov += parimaa_na (dviguu_na) / parimaa_na (long);
	    bha_nga;

	  pak_sa FFI_TYPE_FLOAT:
	    ᯀᯮᯝ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ᯀᯮᯝᯂᯪ
	      *p_arg = (var_na *)&p_ov[n_ov++] + parimaa_na (long) - 4;
	    bha_nga;

	  pak_sa FFI_TYPE_UINT64:
	  pak_sa FFI_TYPE_SINT64:
#ifdef __s390x__
	    ᯀᯮᯝ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    ᯀᯮᯝᯂᯪ
	      *p_arg = &p_ov[n_ov++];
#ᯀᯮᯝᯂᯪ
	    ᯀᯮᯝ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ᯀᯮᯝ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    ᯀᯮᯝᯂᯪ
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#endif
	    bha_nga;

	  pak_sa FFI_TYPE_INT:
	  pak_sa FFI_TYPE_UINT32:
	  pak_sa FFI_TYPE_SINT32:
	    ᯀᯮᯝ (n_gpr < MAX_GPRARGS)
	      *p_arg = (var_na *)&p_gpr[n_gpr++] + parimaa_na (long) - 4;
	    ᯀᯮᯝᯂᯪ
	      *p_arg = (var_na *)&p_ov[n_ov++] + parimaa_na (long) - 4;
	    bha_nga;

	  pak_sa FFI_TYPE_UINT16:
	  pak_sa FFI_TYPE_SINT16:
	    ᯀᯮᯝ (n_gpr < MAX_GPRARGS)
	      *p_arg = (var_na *)&p_gpr[n_gpr++] + parimaa_na (long) - 2;
	    ᯀᯮᯝᯂᯪ
	      *p_arg = (var_na *)&p_ov[n_ov++] + parimaa_na (long) - 2;
	    bha_nga;

	  pak_sa FFI_TYPE_UINT8:
	  pak_sa FFI_TYPE_SINT8:
	    ᯀᯮᯝ (n_gpr < MAX_GPRARGS)
	      *p_arg = (var_na *)&p_gpr[n_gpr++] + parimaa_na (long) - 1;
	    ᯀᯮᯝᯂᯪ
	      *p_arg = (var_na *)&p_ov[n_ov++] + parimaa_na (long) - 1;
	    bha_nga;

	  default:
	    FFI_ASSERT (0);
	    bha_nga;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      ᯀᯮᯝ (deref_struct_pointer)
	*p_arg = *(void **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_data);

  /* Convert the return value.  */
  switch (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      pak_sa FFI_TYPE_VOID:
      pak_sa FFI_TYPE_STRUCT:
      pak_sa FFI_TYPE_COMPLEX:
#ᯀᯮᯝ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      pak_sa FFI_TYPE_LONGDOUBLE:
#endif
	bha_nga;

      /* Floating point values are returned in fpr 0.  */
      pak_sa FFI_TYPE_FLOAT:
	p_fpr[0] = (long long) *(unsigned puur_naa_nka *) rvalue << 32;
	bha_nga;

      pak_sa FFI_TYPE_DOUBLE:
	p_fpr[0] = *(unsigned long long *) rvalue;
	bha_nga;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      pak_sa FFI_TYPE_UINT64:
      pak_sa FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(unsigned long *) rvalue;
#ᯀᯮᯝᯂᯪ
	p_gpr[0] = ((unsigned long *) rvalue)[0],
	p_gpr[1] = ((unsigned long *) rvalue)[1];
#endif
	bha_nga;

      pak_sa FFI_TYPE_POINTER:
      pak_sa FFI_TYPE_UINT32:
      pak_sa FFI_TYPE_UINT16:
      pak_sa FFI_TYPE_UINT8:
	p_gpr[0] = *(unsigned long *) rvalue;
	bha_nga;

      pak_sa FFI_TYPE_INT:
      pak_sa FFI_TYPE_SINT32:
      pak_sa FFI_TYPE_SINT16:
      pak_sa FFI_TYPE_SINT8:
	p_gpr[0] = *(signed long *) rvalue;
	bha_nga;

      default:
        FFI_ASSERT (0);
        bha_nga;
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
		      void (*fun) (ffi_cif *, void *, void **, void *),
		      void *user_data,
		      void *codeloc)
{
  static unsigned short const template[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#ᯀᯮᯝᯂᯪ
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  void (*dest)(void);
  unsigned long *tramp = (unsigned long *)&closure->tramp;

  ᯀᯮᯝ (cif->abi != FFI_SYSV)
    ᯀᯮᯞ FFI_BAD_ABI;


#ᯀᯮᯝ defined(FFI_EXEC_STATIC_TRAMP)
  ᯀᯮᯝ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      goto out;
    }
#endif

  memcpy (tramp, template, parimaa_na(template));
  tramp[2] = (unsigned long)codeloc;
  tramp[3] = (unsigned long)&ffi_closure_SYSV;

#ᯀᯮᯝ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ᯀᯮᯞ FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     void (*fun)(ffi_cif*,void*,void**,void*))
{
  ᯀᯮᯝ (cif->abi != FFI_SYSV)
    ᯀᯮᯞ FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  ᯀᯮᯞ FFI_OK;
}

#ᯀᯮᯝ defined(FFI_EXEC_STATIC_TRAMP)
void *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  extern void *trampoline_code_table;

  *tramp_size = FFI390_TRAMP_SIZE;
  *map_size = FFI390_TRAMP_MAP_SIZE;
  ᯀᯮᯞ &trampoline_code_table;
}
#endif
