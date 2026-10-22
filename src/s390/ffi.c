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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ "internal.h"
#ಅನ್ತರ್ಭಾವ <tramp.h>

/*====================== End of Includes =============================*/

/*====================================================================*/
/*ನಿರೂಪಿಸುತ್ತದೆ*/
/*                           -------                                  */
/*====================================================================*/

/* Maximum number of GPRs available for argument passing.  */
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_GPRARGS 5

/* Maximum number of FPRs available for argument passing.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __s390x__
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_FPRARGS 4
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_FPRARGS 2
#ಆದರೆ_ಅಂತ್ಯ

/* Round to multiple of 16.  */
#ವ್ಯಾಖ್ಯಾನಿಸು ಸುತ್ತು_ಗಾತ್ರ(ಗಾತ್ರ_2) (((ಗಾತ್ರ_2) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

ರಚನೆ call_ಚೌಕಟ್ಟು
{
  ಶೂನ್ಯ *ಹಿಂಭಾಗ_chain;
  ಶೂನ್ಯ *eos;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ gpr_ವಾದಗಳು[5];
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ gpr_save[9];
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ fpr_ವಾದಗಳು[4];
};

ಬಾಹ್ಯ ಶೂನ್ಯ FFI_HIDDEN ffi_call_SYSV(ರಚನೆ call_ಚೌಕಟ್ಟು *, ಚಿಹ್ನೆರಹಿತ, ಶೂನ್ಯ *,
			             ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *);

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_SYSV(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_SYSV(ಶೂನ್ಯ);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ffi_check_struct_type (ffi_type *arg)
{
  size_t ಗಾತ್ರ_2 = arg->ಗಾತ್ರ_2;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  ಆಗ (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  ಆಯ್ಕೆ (ಗಾತ್ರ_2)
    {
      ಸ್ಥಿತಿ 1:
        ಹಿಂತಿರುಗಿ FFI_TYPE_UINT8;

      ಸ್ಥಿತಿ 2:
        ಹಿಂತಿರುಗಿ FFI_TYPE_UINT16;

      ಸ್ಥಿತಿ 4:
	ಆದರೆ (arg->type == FFI_TYPE_FLOAT)
          ಹಿಂತಿರುಗಿ FFI_TYPE_FLOAT;
	ಇಲ್ಲದಿದ್ದರೆ
	  ಹಿಂತಿರುಗಿ FFI_TYPE_UINT32;

      ಸ್ಥಿತಿ 8:
	ಆದರೆ (arg->type == FFI_TYPE_DOUBLE)
          ಹಿಂತಿರುಗಿ FFI_TYPE_DOUBLE;
	ಇಲ್ಲದಿದ್ದರೆ
	  ಹಿಂತಿರುಗಿ FFI_TYPE_UINT64;

      ಪೂರ್ವನಿಯೋಜಿತ:
	ನಿಲ್ಲಿಸು;
    }

  /* Other structs are passed via a pointer to the data.  */
  ಹಿಂತಿರುಗಿ FFI_TYPE_ಸೂಚಕ;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_prep_cif_machdep.                                   */
/*                                                                    */
/* Function - Perform machine dependent CIF processing.               */
/*                                                                    */
/*====================================================================*/

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t struct_ಗಾತ್ರ = 0;
  ಪೂರ್ಣಾಂಕ n_gpr = 0;
  ಪೂರ್ಣಾಂಕ n_fpr = 0;
  ಪೂರ್ಣಾಂಕ n_ov = 0;

  ffi_type **ptr;
  ಪೂರ್ಣಾಂಕ i;

  /* Determine return value handling.  */

  ಆಯ್ಕೆ (cif->rtype->type)
    {
      /* Void is easy.  */
      ಸ್ಥಿತಿ FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	ನಿಲ್ಲಿಸು;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      ಸ್ಥಿತಿ FFI_TYPE_SINT128:
      ಸ್ಥಿತಿ FFI_TYPE_UINT128:
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#ಆದರೆ_ಅಂತ್ಯ
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	ನಿಲ್ಲಿಸು;

      /* Floating point values are returned in fpr 0.  */
      ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	ನಿಲ್ಲಿಸು;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      ಸ್ಥಿತಿ FFI_TYPE_INT:
      ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __s390x__
	cif->flags = FFI390_RET_INT64;
#ಇಲ್ಲದಿದ್ದರೆ
	cif->flags = FFI390_RET_INT32;
#ಆದರೆ_ಅಂತ್ಯ
	ನಿಲ್ಲಿಸು;

      ಪೂರ್ವನಿಯೋಜಿತ:
        FFI_ASSERT (0);
        ನಿಲ್ಲಿಸು;
    }

  /* Now for the arguments.  */

  ಗಾಗಿ (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      ಪೂರ್ಣಾಂಕ type = (*ptr)->type;

      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  ಆದರೆ (type != FFI_TYPE_ಸೂಚಕ)
	    ನಿಲ್ಲಿಸು;
	  /* fall through */

	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	ಸ್ಥಿತಿ FFI_TYPE_SINT128:
	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#ಆದರೆ_ಅಂತ್ಯ
	  type = FFI_TYPE_ಸೂಚಕ;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_ಗಾತ್ರ += ಸುತ್ತು_ಗಾತ್ರ ((*ptr)->ಗಾತ್ರ_2);
	  ನಿಲ್ಲಿಸು;
	}

      /* Now handle all primitive int/float data types.  */
      ಆಯ್ಕೆ (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	    ಆದರೆ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ಇಲ್ಲದಿದ್ದರೆ
	      n_ov += ಗಾತ್ರ (ದ್ವಿಗುಣ) / ಗಾತ್ರ (ದೀರ್ಘ);
	    ನಿಲ್ಲಿಸು;

	  ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	    ಆದರೆ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ಇಲ್ಲದಿದ್ದರೆ
	      n_ov++;
	    ನಿಲ್ಲಿಸು;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __s390x__
	  ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	  ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	    ಆದರೆ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ಆದರೆ (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    ಇಲ್ಲದಿದ್ದರೆ
	      n_ov += 2;
	    ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  ಪೂರ್ವನಿಯೋಜಿತ:
	    ಆದರೆ (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    ಇಲ್ಲದಿದ್ದರೆ
	      n_ov++;
	    ನಿಲ್ಲಿಸು;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->ಬೈಟುಗಳು = ಸುತ್ತು_ಗಾತ್ರ (n_ov * ಗಾತ್ರ (ದೀರ್ಘ)) + struct_ಗಾತ್ರ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int(ffi_cif *cif,
	     ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ),
	     ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue,
	     ಶೂನ್ಯ *ಆವರಣ)
{
  ಪೂರ್ಣಾಂಕ ret_type = cif->flags;
  size_t rsize = 0, ಬೈಟುಗಳು = cif->ಬೈಟುಗಳು;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ರಾಶಿ, *p_struct;
  ರಚನೆ call_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *p_ov, *p_gpr;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ *p_fpr;
  ಪೂರ್ಣಾಂಕ n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  ಆದರೆ (rvalue == ಖಾಲಿ)
    {
      ಆದರೆ (ret_type & FFI390_RET_ಒಳ_MEM)
	rsize = cif->rtype->ಗಾತ್ರ_2;
      ಇಲ್ಲದಿದ್ದರೆ
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

  ರಾಶಿ = alloca (ಬೈಟುಗಳು + ಗಾತ್ರ(ರಚನೆ call_ಚೌಕಟ್ಟು) + rsize);
  ಚೌಕಟ್ಟು = (ರಚನೆ call_ಚೌಕಟ್ಟು *)(ರಾಶಿ + ಬೈಟುಗಳು);
  ಆದರೆ (rsize)
    rvalue = ಚೌಕಟ್ಟು + 1;

  /* Link the new frame back to the one from this function.  */
  ಚೌಕಟ್ಟು->ಹಿಂಭಾಗ_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *)ರಾಶಿ;
  p_struct = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)ಚೌಕಟ್ಟು;
  p_gpr = ಚೌಕಟ್ಟು->gpr_ವಾದಗಳು;
  p_fpr = ಚೌಕಟ್ಟು->fpr_ವಾದಗಳು;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  ಆದರೆ (cif->flags & FFI390_RET_ಒಳ_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      ಶೂನ್ಯ *arg = avalue[i];
      ಪೂರ್ಣಾಂಕ type = ty->type;
      ffi_arg val;

    restart:
      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  ಹೋಗು do_int;
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  ಹೋಗು do_int;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  ಹೋಗು do_int;
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  ಹೋಗು do_int;
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  ಹೋಗು do_int;
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  ಹೋಗು do_int;
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __s390x__
	  val = *(UINT64 *)arg;
	  ಹೋಗು do_int;
#ಇಲ್ಲದಿದ್ದರೆ
	  ಆದರೆ (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  ಆದರೆ (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  ಇಲ್ಲದಿದ್ದರೆ
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಆದರೆ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#ಇಲ್ಲದಿದ್ದರೆ
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#ಆದರೆ_ಅಂತ್ಯ
	    }
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  ಆದರೆ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  ಇಲ್ಲದಿದ್ದರೆ
	    p_ov[n_ov++] = val;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  ಆದರೆ (type != FFI_TYPE_ಸೂಚಕ)
	    ಹೋಗು restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	ಸ್ಥಿತಿ FFI_TYPE_SINT128:
	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ಸುತ್ತು_ಗಾತ್ರ (ty->ಗಾತ್ರ_2);
	  ಸ್ಮರಣೆ_ನಕಲು (p_struct, arg, ty->ಗಾತ್ರ_2);
	  val = (uintptr_t)p_struct;
	  ಹೋಗು do_int;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  FFI_ASSERT (0);
	  ನಿಲ್ಲಿಸು;
        }
    }

  ffi_call_SYSV (ಚೌಕಟ್ಟು, ret_type & FFI360_RET_ಮುಖವಾಡ, rvalue, fn, ಆವರಣ);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ಖಾಲಿ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int(cif, fn, rvalue, avalue, ಆವರಣ);
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_closure_helper_SYSV.                                */
/*                                                                    */
/* Function - Call a FFI closure target function.                     */
/*                                                                    */
/*====================================================================*/

ಶೂನ್ಯ FFI_HIDDEN
ffi_ಆವರಣ_helper_SYSV (ffi_cif *cif,
			 ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
			 ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *p_gpr,
			 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ *p_fpr,
			 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *p_ov)
{
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ret_ಬಫರ್;

  ಶೂನ್ಯ *rvalue = &ret_ಬಫರ್;
  ಶೂನ್ಯ **avalue;
  ಶೂನ್ಯ **p_arg;

  ಪೂರ್ಣಾಂಕ n_gpr = 0;
  ಪೂರ್ಣಾಂಕ n_fpr = 0;
  ಪೂರ್ಣಾಂಕ n_ov = 0;

  ffi_type **ptr;
  ಪೂರ್ಣಾಂಕ i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  ಆದರೆ (cif->flags & FFI390_RET_ಒಳ_MEM)
    rvalue = (ಶೂನ್ಯ *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  ಗಾಗಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      ಪೂರ್ಣಾಂಕ deref_struct_ಸೂಚಕ = 0;
      ಪೂರ್ಣಾಂಕ type = (*ptr)->type;

#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      ಆದರೆ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#ಆದರೆ_ಅಂತ್ಯ

      /* Check how a structure type is passed.  */
      ಆದರೆ (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  ಆದರೆ (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_ಸೂಚಕ;
	  ಇಲ್ಲದಿದ್ದರೆ
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  ಆದರೆ (type == FFI_TYPE_ಸೂಚಕ)
	    deref_struct_ಸೂಚಕ = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      ಆದರೆ (type == FFI_TYPE_ಸೂಚಕ)
	{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __s390x__
	  type = FFI_TYPE_UINT64;
#ಇಲ್ಲದಿದ್ದರೆ
	  type = FFI_TYPE_UINT32;
#ಆದರೆ_ಅಂತ್ಯ
	}

      /* Now handle all primitive int/float data types.  */
      ಆಯ್ಕೆ (type)
	{
	  ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	    ಆದರೆ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ಇಲ್ಲದಿದ್ದರೆ
	      *p_arg = &p_ov[n_ov],
	      n_ov += ಗಾತ್ರ (ದ್ವಿಗುಣ) / ಗಾತ್ರ (ದೀರ್ಘ);
	    ನಿಲ್ಲಿಸು;

	  ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	    ಆದರೆ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ಇಲ್ಲದಿದ್ದರೆ
	      *p_arg = (ಅಕ್ಷರ *)&p_ov[n_ov++] + ಗಾತ್ರ (ದೀರ್ಘ) - 4;
	    ನಿಲ್ಲಿಸು;

	  ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	  ಸ್ಥಿತಿ FFI_TYPE_SINT64:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __s390x__
	    ಆದರೆ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    ಇಲ್ಲದಿದ್ದರೆ
	      *p_arg = &p_ov[n_ov++];
#ಇಲ್ಲದಿದ್ದರೆ
	    ಆದರೆ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ಆದರೆ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    ಇಲ್ಲದಿದ್ದರೆ
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#ಆದರೆ_ಅಂತ್ಯ
	    ನಿಲ್ಲಿಸು;

	  ಸ್ಥಿತಿ FFI_TYPE_INT:
	  ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	    ಆದರೆ (n_gpr < MAX_GPRARGS)
	      *p_arg = (ಅಕ್ಷರ *)&p_gpr[n_gpr++] + ಗಾತ್ರ (ದೀರ್ಘ) - 4;
	    ಇಲ್ಲದಿದ್ದರೆ
	      *p_arg = (ಅಕ್ಷರ *)&p_ov[n_ov++] + ಗಾತ್ರ (ದೀರ್ಘ) - 4;
	    ನಿಲ್ಲಿಸು;

	  ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	    ಆದರೆ (n_gpr < MAX_GPRARGS)
	      *p_arg = (ಅಕ್ಷರ *)&p_gpr[n_gpr++] + ಗಾತ್ರ (ದೀರ್ಘ) - 2;
	    ಇಲ್ಲದಿದ್ದರೆ
	      *p_arg = (ಅಕ್ಷರ *)&p_ov[n_ov++] + ಗಾತ್ರ (ದೀರ್ಘ) - 2;
	    ನಿಲ್ಲಿಸು;

	  ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	    ಆದರೆ (n_gpr < MAX_GPRARGS)
	      *p_arg = (ಅಕ್ಷರ *)&p_gpr[n_gpr++] + ಗಾತ್ರ (ದೀರ್ಘ) - 1;
	    ಇಲ್ಲದಿದ್ದರೆ
	      *p_arg = (ಅಕ್ಷರ *)&p_ov[n_ov++] + ಗಾತ್ರ (ದೀರ್ಘ) - 1;
	    ನಿಲ್ಲಿಸು;

	  ಪೂರ್ವನಿಯೋಜಿತ:
	    FFI_ASSERT (0);
	    ನಿಲ್ಲಿಸು;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      ಆದರೆ (deref_struct_ಸೂಚಕ)
	*p_arg = *(ಶೂನ್ಯ **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

  /* Convert the return value.  */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      ಸ್ಥಿತಿ FFI_TYPE_VOID:
      ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#ಆದರೆ_ಅಂತ್ಯ
	ನಿಲ್ಲಿಸು;

      /* Floating point values are returned in fpr 0.  */
      ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	p_fpr[0] = (ದೀರ್ಘ ದೀರ್ಘ) *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) rvalue << 32;
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	p_fpr[0] = *(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ *) rvalue;
	ನಿಲ್ಲಿಸು;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      ಸ್ಥಿತಿ FFI_TYPE_SINT64:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __s390x__
	p_gpr[0] = *(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) rvalue;
#ಇಲ್ಲದಿದ್ದರೆ
	p_gpr[0] = ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) rvalue)[0],
	p_gpr[1] = ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) rvalue)[1];
#ಆದರೆ_ಅಂತ್ಯ
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	p_gpr[0] = *(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) rvalue;
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_INT:
      ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	p_gpr[0] = *(ಚಿಹ್ನೆಯುಳ್ಳ ದೀರ್ಘ *) rvalue;
	ನಿಲ್ಲಿಸು;

      ಪೂರ್ವನಿಯೋಜಿತ:
        FFI_ASSERT (0);
        ನಿಲ್ಲಿಸು;
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

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ *ಆವರಣ,
		      ffi_cif *cif,
		      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಸ್ಥಿರಾಂಕ ಮಾದರಿ[] = {
    0x0d10,			/* basr %r1,0 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#ಇಲ್ಲದಿದ್ದರೆ
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#ಆದರೆ_ಅಂತ್ಯ
    0x07f1			/* br %r1 */
  };
  ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ);
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *tramp = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *)&ಆವರಣ->tramp;

  ಆದರೆ (cif->abi != FFI_SYSV)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;


#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
  ಆದರೆ (ffi_tramp_is_present(ಆವರಣ))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_ಆವರಣ_SYSV;
      ffi_tramp_ಗಣ_parms (ಆವರಣ->ftramp, dest, ಆವರಣ);
      ಹೋಗು ಹೊರಹೋಗು;
    }
#ಆದರೆ_ಅಂತ್ಯ

  ಸ್ಮರಣೆ_ನಕಲು (tramp, ಮಾದರಿ, ಗಾತ್ರ(ಮಾದರಿ));
  tramp[2] = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)codeloc;
  tramp[3] = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)&ffi_ಆವರಣ_SYSV;

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಹೊರಹೋಗು:
#ಆದರೆ_ಅಂತ್ಯ
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ *ಆವರಣ, ffi_cif *cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*))
{
  ಆದರೆ (cif->abi != FFI_SYSV)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಆವರಣ->tramp = ffi_go_ಆವರಣ_SYSV;
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_ಗಾತ್ರ, size_t *map_ಗಾತ್ರ)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;

  *tramp_ಗಾತ್ರ = FFI390_TRAMP_ಗಾತ್ರ;
  *map_ಗಾತ್ರ = FFI390_TRAMP_MAP_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ &trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;
}
#ಆದರೆ_ಅಂತ್ಯ
