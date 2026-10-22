/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2011 Timothy Wall
           Copyright (c) 2011 Plausible Labs Cooperative, Inc.
           Copyright (c) 2011 Anthony Green
	   Copyright (c) 2011 Free Software Foundation
           Copyright (c) 1998, 2008, 2011  Red Hat, Inc.

   ARM Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#എങ്കിൽ defined(__arm__) || defined(_M_ARM)
#അന്തര്ഭാവ <fficonfig.h>
#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <tramp.h>
#അന്തര്ഭാവ "internal.h"

#എങ്കിൽ defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#അന്തര്ഭാവ <windows.h>
#endif

#എങ്കിൽ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#അന്തര്ഭാവ <mach/machine/vm_param.h>
#endif

#അല്ലെങ്കിൽ
#ifndef _WIN32
ബാഹ്യ unsigned പൂർണ്ണസംഖ്യ ffi_arm_trampoline[2] FFI_HIDDEN;
#അല്ലെങ്കിൽ
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
ബാഹ്യ unsigned അക്ഷരം ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#എങ്കിൽ defined(__FreeBSD__) && defined(__arm__)
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <machine/sysarch.h>
#endif

#എങ്കിൽ defined(__QNX__)
#അന്തര്ഭാവ <sys/mman.h>
#endif

/* Forward declares. */
സ്ഥിതം പൂർണ്ണസംഖ്യ vfp_type_p (സ്ഥിരം ffi_type *);
സ്ഥിതം ശൂന്യം layout_vfp_args (ffi_cif *);

സ്ഥിതം ശൂന്യം *
ffi_align (ffi_type *ty, ശൂന്യം *p)
{
  /* Align if necessary */
  size_t alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#അല്ലെങ്കിൽ
  alignment = ty->alignment;
  എങ്കിൽ (alignment < 4)
    alignment = 4;
#endif
  തിരികെ (ശൂന്യം *) FFI_ALIGN (p, alignment);
}

സ്ഥിതം size_t
ffi_put_arg (ffi_type *ty, ശൂന്യം *src, ശൂന്യം *dst)
{
  size_t z = ty->size;

  തിരഞ്ഞെടുപ്പ് (ty->type)
    {
    അവസ്ഥ FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
    അവസ്ഥ FFI_TYPE_UINT32:
    അവസ്ഥ FFI_TYPE_POINTER:
#ifndef _WIN32
    അവസ്ഥ FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      നിർത്തുക;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    അവസ്ഥ FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(ദശാംശം *)dst = *(ദശാംശം *)src;
        നിർത്തുക;
#endif

    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
    അവസ്ഥ FFI_TYPE_COMPLEX:
      memcpy (dst, src, z);
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      abort();
    }

  തിരികെ FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
സ്ഥിതം ശൂന്യം
ffi_prep_args_SYSV (ffi_cif *cif, പൂർണ്ണസംഖ്യ flags, ശൂന്യം *rvalue,
		    ശൂന്യം **avalue, അക്ഷരം *argp)
{
  ffi_type **arg_types = cif->arg_types;
  പൂർണ്ണസംഖ്യ i, n;

  എങ്കിൽ (flags == ARM_TYPE_STRUCT)
    {
      *(ശൂന്യം **) argp = rvalue;
      argp += 4;
    }

  വേണ്ടി (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

സ്ഥിതം ശൂന്യം
ffi_prep_args_VFP (ffi_cif *cif, പൂർണ്ണസംഖ്യ flags, ശൂന്യം *rvalue,
                   ശൂന്യം **avalue, അക്ഷരം *അട്ടി, അക്ഷരം *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  പൂർണ്ണസംഖ്യ i, n, vi = 0;
  അക്ഷരം *argp, *regp, *eo_regp;
  അക്ഷരം അട്ടി_used = 0;
  അക്ഷരം done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = അട്ടി;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  എങ്കിൽ (flags == ARM_TYPE_STRUCT)
    {
      *(ശൂന്യം **) regp = rvalue;
      regp += 4;
    }

  വേണ്ടി (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ശൂന്യം *a = avalue[i];
      പൂർണ്ണസംഖ്യ is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      എങ്കിൽ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  അക്ഷരം *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  തുടരുക;
	}
      /* Try allocating in core registers. */
      അല്ലെങ്കിൽ എങ്കിൽ (!done_with_regs && !is_vfp_type)
	{
	  അക്ഷരം *tregp = ffi_align (ty, regp);
	  size_t size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  എങ്കിൽ (tregp + size <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      തുടരുക;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  അല്ലെങ്കിൽ എങ്കിൽ (!അട്ടി_used)
	    {
	      അട്ടി_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      തുടരുക;
	    }
	}
      /* Base case, arguments are passed on the stack */
      അട്ടി_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  പൂർണ്ണസംഖ്യ flags = 0, cabi = cif->abi;
  size_t bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  എങ്കിൽ (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_UINT8:
    അവസ്ഥ FFI_TYPE_SINT8:
    അവസ്ഥ FFI_TYPE_UINT16:
    അവസ്ഥ FFI_TYPE_SINT16:
    അവസ്ഥ FFI_TYPE_UINT32:
    അവസ്ഥ FFI_TYPE_SINT32:
    അവസ്ഥ FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
    അവസ്ഥ FFI_TYPE_COMPLEX:
      എങ്കിൽ (cabi == FFI_VFP)
	{
	  പൂർണ്ണസംഖ്യ h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  എങ്കിൽ (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  എങ്കിൽ (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  എങ്കിൽ (h != 0)
	      നിർത്തുക;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      എങ്കിൽ (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      അല്ലെങ്കിൽ
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  എങ്കിൽ (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  തിരികെ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  unsigned പൂർണ്ണസംഖ്യ nfixedargs, unsigned പൂർണ്ണസംഖ്യ ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  എങ്കിൽ (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  തിരികെ ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

ഘടന call_frame
{
  ശൂന്യം *fp;
  ശൂന്യം *lr;
  ശൂന്യം *rvalue;
  പൂർണ്ണസംഖ്യ flags;
  ശൂന്യം *closure;
};

ബാഹ്യ ശൂന്യം ffi_call_SYSV (ശൂന്യം *അട്ടി, ഘടന call_frame *,
			   ശൂന്യം (*fn) (ശൂന്യം)) FFI_HIDDEN;
ബാഹ്യ ശൂന്യം ffi_call_VFP (ശൂന്യം *vfp_space, ഘടന call_frame *,
			   ശൂന്യം (*fn) (ശൂന്യം), unsigned vfp_used) FFI_HIDDEN;

സ്ഥിതം ശൂന്യം
ffi_call_int (ffi_cif * cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue,
	      ശൂന്യം **avalue, ശൂന്യം *closure)
{
  പൂർണ്ണസംഖ്യ flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t bytes, rsize, vfp_size;
  അക്ഷരം *അട്ടി, *vfp_space, *new_rvalue;
  ഘടന call_frame *frame;

  rsize = 0;
  എങ്കിൽ (rvalue == ശൂന്യ)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      എങ്കിൽ (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      അല്ലെങ്കിൽ
	flags = ARM_TYPE_VOID;
    }
  അല്ലെങ്കിൽ എങ്കിൽ (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  അല്ലെങ്കിൽ എങ്കിൽ (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  അട്ടി = alloca (vfp_size + bytes + വലുപ്പം(ഘടന call_frame) + rsize);

  vfp_space = ശൂന്യ;
  എങ്കിൽ (vfp_size)
    {
      vfp_space = അട്ടി;
      അട്ടി += vfp_size;
    }

  frame = (ഘടന call_frame *)(അട്ടി + bytes);

  new_rvalue = rvalue;
  എങ്കിൽ (rsize)
    new_rvalue = (ശൂന്യം *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  എങ്കിൽ (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, അട്ടി, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  അല്ലെങ്കിൽ
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, അട്ടി);
      ffi_call_SYSV (അട്ടി, frame, fn);
    }

  എങ്കിൽ (rvalue && rvalue != new_rvalue)
    memcpy (rvalue, new_rvalue, rtype->size);
}

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ശൂന്യ);
}

#ifdef FFI_GO_CLOSURES
ശൂന്യം
ffi_call_go (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue,
	     ശൂന്യം **avalue, ശൂന്യം *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

സ്ഥിതം ശൂന്യം *
ffi_prep_incoming_args_SYSV (ffi_cif *cif, ശൂന്യം *rvalue,
			     അക്ഷരം *argp, ശൂന്യം **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  പൂർണ്ണസംഖ്യ i, n;

  എങ്കിൽ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(ശൂന്യം **) argp;
      argp += 4;
    }
  അല്ലെങ്കിൽ
    {
      എങ്കിൽ (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  വേണ്ടി (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      എങ്കിൽ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (ശൂന്യം *) (argp + 4 - z);
      അല്ലെങ്കിൽ
#endif
      avalue[i] = (ശൂന്യം *) argp;
      argp += z;
    }

  തിരികെ rvalue;
}

സ്ഥിതം ശൂന്യം *
ffi_prep_incoming_args_VFP (ffi_cif *cif, ശൂന്യം *rvalue, അക്ഷരം *അട്ടി,
			    അക്ഷരം *vfp_space, ശൂന്യം **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  പൂർണ്ണസംഖ്യ i, n, vi = 0;
  അക്ഷരം *argp, *regp, *eo_regp;
  അക്ഷരം done_with_regs = 0;
  അക്ഷരം അട്ടി_used = 0;

  regp = അട്ടി;
  eo_regp = argp = regp + 16;

  എങ്കിൽ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(ശൂന്യം **) regp;
      regp += 4;
    }

  വേണ്ടി (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      പൂർണ്ണസംഖ്യ is_vfp_type = vfp_type_p (ty);
      size_t z = ty->size;

      എങ്കിൽ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  തുടരുക;
	}
      അല്ലെങ്കിൽ എങ്കിൽ (!done_with_regs && !is_vfp_type)
	{
	  അക്ഷരം *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  എങ്കിൽ (tregp + z <= eo_regp || !അട്ടി_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      എങ്കിൽ (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (ശൂന്യം *) (tregp + 4 - z);
	      അല്ലെങ്കിൽ
#endif
	      avalue[i] = (ശൂന്യം *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      എങ്കിൽ (regp > eo_regp)
		{
		  FFI_ASSERT (!അട്ടി_used);
		  argp = regp;
		}
	      എങ്കിൽ (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  അട്ടി_used = 1;
		}
	      തുടരുക;
	    }
	}

      അട്ടി_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      എങ്കിൽ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (ശൂന്യം *) (argp + 4 - z);
      അല്ലെങ്കിൽ
#endif
      avalue[i] = (ശൂന്യം *) argp;
      argp += z;
    }

  തിരികെ rvalue;
}

#എങ്കിൽ FFI_CLOSURES

ഘടന closure_frame
{
  അക്ഷരം vfp_space[8*8] __attribute__((aligned(8)));
  അക്ഷരം result[8*4];
  അക്ഷരം argp[];
};

പൂർണ്ണസംഖ്യ FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
		        ശൂന്യം *user_data,
		        ഘടന closure_frame *frame)
{
  ശൂന്യം **avalue = (ശൂന്യം **) alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));
  ശൂന്യം *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  തിരികെ cif->flags;
}

പൂർണ്ണസംഖ്യ FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
		       ശൂന്യം *user_data,
		       ഘടന closure_frame *frame)
{
  ശൂന്യം **avalue = (ശൂന്യം **) alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));
  ശൂന്യം *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  തിരികെ cif->flags;
}

ശൂന്യം ffi_closure_SYSV (ശൂന്യം) FFI_HIDDEN;
ശൂന്യം ffi_closure_VFP (ശൂന്യം) FFI_HIDDEN;
#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
ശൂന്യം ffi_closure_SYSV_alt (ശൂന്യം) FFI_HIDDEN;
ശൂന്യം ffi_closure_VFP_alt (ശൂന്യം) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
ശൂന്യം ffi_go_closure_SYSV (ശൂന്യം) FFI_HIDDEN;
ശൂന്യം ffi_go_closure_VFP (ശൂന്യം) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#എങ്കിൽ defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) ചെയ്യുക { \
		ഘടന arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (അക്ഷരം *)(end) - (അക്ഷരം *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} ആകുമ്പോൾ (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
		      ശൂന്യം *user_data, ശൂന്യം *codeloc)
{
  ശൂന്യം (*closure_func) (ശൂന്യം) = ffi_closure_SYSV;

  എങ്കിൽ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      എങ്കിൽ (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  അല്ലെങ്കിൽ എങ്കിൽ (cif->abi != FFI_SYSV)
    തിരികെ FFI_BAD_ABI;

#എങ്കിൽ FFI_EXEC_TRAMPOLINE_TABLE
  ശൂന്യം **config = (ശൂന്യം **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#അല്ലെങ്കിൽ

# എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
  എങ്കിൽ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      എങ്കിൽ (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      അല്ലെങ്കിൽ
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      പോകുക out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  memcpy(closure->tramp, ffi_arm_trampoline, 8);
# അല്ലെങ്കിൽ
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  memcpy(closure->tramp, (ശൂന്യം*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# എങ്കിൽ defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# അല്ലെങ്കിൽ
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(ശൂന്യം(**)(ശൂന്യം))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# അല്ലെങ്കിൽ
  *(ശൂന്യം (**)(ശൂന്യം))(closure->tramp + 8) = closure_func;
# endif
# എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  തിരികെ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *))
{
  ശൂന്യം (*closure_func) (ശൂന്യം) = ffi_go_closure_SYSV;

  എങ്കിൽ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      എങ്കിൽ (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  അല്ലെങ്കിൽ എങ്കിൽ (cif->abi != FFI_SYSV)
    തിരികെ FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  തിരികെ FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ
is_hfa0 (സ്ഥിരം ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  പൂർണ്ണസംഖ്യ i, ret = -1;

  എങ്കിൽ (elements != ശൂന്യ)
    വേണ്ടി (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        എങ്കിൽ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            എങ്കിൽ (ret < 0)
              തുടരുക;
          }
        നിർത്തുക;
      }

  തിരികെ ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ
is_hfa1 (സ്ഥിരം ffi_type *ty, പൂർണ്ണസംഖ്യ candidate)
{
  ffi_type **elements = ty->elements;
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ (elements != ശൂന്യ)
    വേണ്ടി (i = 0; elements[i]; ++i)
      {
        പൂർണ്ണസംഖ്യ t = elements[i]->type;
        എങ്കിൽ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            എങ്കിൽ (!is_hfa1 (elements[i], candidate))
              തിരികെ 0;
          }
        അല്ലെങ്കിൽ എങ്കിൽ (t != candidate)
          തിരികെ 0;
      }

  തിരികെ 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ
vfp_type_p (സ്ഥിരം ffi_type *ty)
{
  ffi_type **elements;
  പൂർണ്ണസംഖ്യ candidate, i;
  size_t size, ele_എണ്ണം;

  /* Quickest tests first.  */
  candidate = ty->type;
  തിരഞ്ഞെടുപ്പ് (ty->type)
    {
    സ്ഥിരസ്ഥിതി:
      തിരികെ 0;
    അവസ്ഥ FFI_TYPE_FLOAT:
    അവസ്ഥ FFI_TYPE_DOUBLE:
      ele_എണ്ണം = 1;
      പോകുക done;
    അവസ്ഥ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      എങ്കിൽ (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	തിരികെ 0;
      ele_എണ്ണം = 2;
      പോകുക done;
    അവസ്ഥ FFI_TYPE_STRUCT:
      നിർത്തുക;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  എങ്കിൽ (size < 4 || size > 32)
    തിരികെ 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  എങ്കിൽ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      വേണ്ടി (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          എങ്കിൽ (candidate >= 0)
            നിർത്തുക;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  തിരഞ്ഞെടുപ്പ് (candidate)
    {
    അവസ്ഥ FFI_TYPE_FLOAT:
      ele_എണ്ണം = size / വലുപ്പം(ദശാംശം);
      എങ്കിൽ (size != ele_എണ്ണം * വലുപ്പം(ദശാംശം))
        തിരികെ 0;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_DOUBLE:
      ele_എണ്ണം = size / വലുപ്പം(ഇരട്ട);
      എങ്കിൽ (size != ele_എണ്ണം * വലുപ്പം(ഇരട്ട))
        തിരികെ 0;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ 0;
    }
  എങ്കിൽ (ele_എണ്ണം > 4)
    തിരികെ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  വേണ്ടി (i = 0; elements[i]; ++i)
    {
      പൂർണ്ണസംഖ്യ t = elements[i]->type;
      എങ്കിൽ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          എങ്കിൽ (!is_hfa1 (elements[i], candidate))
            തിരികെ 0;
        }
      അല്ലെങ്കിൽ എങ്കിൽ (t != candidate)
        തിരികെ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  തിരികെ (ele_എണ്ണം << 8) | candidate;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
place_vfp_arg (ffi_cif *cif, പൂർണ്ണസംഖ്യ h)
{
  unsigned short reg = cif->vfp_reg_free;
  പൂർണ്ണസംഖ്യ align = 1, nregs = h >> 8;

  എങ്കിൽ ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  എങ്കിൽ ((reg & 1) && align == 2)
    reg++;

  ആകുമ്പോൾ (reg + nregs <= 16)
    {
      പൂർണ്ണസംഖ്യ s, new_used = 0;
      വേണ്ടി (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  എങ്കിൽ (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      പോകുക next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (signed അക്ഷരം)reg;

      /* Update vfp_reg_free. */
      എങ്കിൽ (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  ആകുമ്പോൾ (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      തിരികെ 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  തിരികെ 1;
}

സ്ഥിതം ശൂന്യം
layout_vfp_args (ffi_cif * cif)
{
  unsigned പൂർണ്ണസംഖ്യ i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_args, -1, 16);	/* Init to -1. */

  വേണ്ടി (i = 0; i < cif->nargs; i++)
    {
      പൂർണ്ണസംഖ്യ h = vfp_type_p (cif->arg_types[i]);
      എങ്കിൽ (h && place_vfp_arg (cif, h) == 1)
	നിർത്തുക;
    }
}

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
ശൂന്യം *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ബാഹ്യ ശൂന്യം *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  തിരികെ &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
