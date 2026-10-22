/*
 * Copyright (c) 2013 Miodrag Vallat.  <miod@openbsd.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * m88k Foreign Function Interface
 *
 * This file attempts to provide all the FFI entry points which can reliably
 * be implemented in C.
 *
 * Only OpenBSD/m88k is currently supported; other platforms (such as
 * Motorola's SysV/m88k) could be supported with the following tweaks:
 *
 * - non-OpenBSD systems use an `outgoing parameter area' as part of the
 *   88BCS calling convention, which is not supported under OpenBSD from
 *   release 3.6 onwards.  Supporting it should be as easy as taking it
 *   into account when adjusting the stack, in the assembly code.
 *
 * - the logic deciding whether a function argument gets passed through
 *   registers, or on the stack, has changed several times in OpenBSD in
 *   edge cases (especially for structs larger than 32 bytes being passed
 *   by value). The code below attemps to match the logic used by the
 *   system compiler of OpenBSD 5.3, i.e. gcc 3.3.6 with many m88k backend
 *   fixes.
 */

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

ಶೂನ್ಯ ffi_call_OBSD (unsigned ಪೂರ್ಣಾಙ್ಕ, extended_cif *, unsigned ಪೂರ್ಣಾಙ್ಕ, ಶೂನ್ಯ *,
		    ಶೂನ್ಯ (*fn) ());
ಶೂನ್ಯ *ffi_prep_args (ಶೂನ್ಯ *, extended_cif *);
ಶೂನ್ಯ ffi_closure_OBSD (ffi_closure *);
ಶೂನ್ಯ ffi_closure_struct_OBSD (ffi_closure *);
unsigned ಪೂರ್ಣಾಙ್ಕ ffi_closure_OBSD_inner (ffi_closure *, ಶೂನ್ಯ *, unsigned ಪೂರ್ಣಾಙ್ಕ *,
				     ವರ್ಣ *);
ಶೂನ್ಯ ffi_cacheflush_OBSD (unsigned ಪೂರ್ಣಾಙ್ಕ, unsigned ಪೂರ್ಣಾಙ್ಕ);

#define CIF_FLAGS_INT		(1 << 0)
#define CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

ಶೂನ್ಯ *
ffi_prep_args (ಶೂನ್ಯ *ರಾಶಿ, extended_cif *ecif)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಶೂನ್ಯ **p_argv;
  ವರ್ಣ *argp, *stackp;
  unsigned ಪೂರ್ಣಾಙ್ಕ *regp;
  unsigned ಪೂರ್ಣಾಙ್ಕ regused;
  ffi_type **p_arg;
  ಶೂನ್ಯ *struct_ಮೌಲ್ಯ_ptr;

  regp = (unsigned ಪೂರ್ಣಾಙ್ಕ *)ರಾಶಿ;
  stackp = (ವರ್ಣ *)(regp + 8);
  regused = 0;

  ಯದಿ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_ಮೌಲ್ಯ_ptr = ecif->rvalue;
  ಅನ್ಯಥಾ
    struct_ಮೌಲ್ಯ_ptr = ರಿಕ್ತ;

  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      unsigned short t, a;

      z = (*p_arg)->size;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument can be passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      ಯದಿ (t == FFI_TYPE_STRUCT)
	{
	  ಯದಿ (z == ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) && a == ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) && regused < 8)
	    argp = (ವರ್ಣ *)regp;
	  ಅನ್ಯಥಾ
	    argp = stackp;
	}
      ಅನ್ಯಥಾ
	{
	  ಯದಿ (z > ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ಯದಿ (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ಯದಿ (regused < 8)
	    argp = (ವರ್ಣ *)regp;
	  ಅನ್ಯಥಾ
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ಯದಿ (argp == stackp && a > ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	{
	  stackp = (ವರ್ಣ *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ವಿಕಲ್ಪ (t)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	  *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ) *(SINT8 *) *p_argv;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT8:
	  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ) *(UINT8 *) *p_argv;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT16:
	  *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ) *(SINT16 *) *p_argv;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT16:
	  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ) *(UINT16 *) *p_argv;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = *(unsigned ಪೂರ್ಣಾಙ್ಕ *) *p_argv;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      ಯದಿ ((ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - 1) & z)
	z = FFI_ALIGN(z, ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      ಯದಿ (argp == (ವರ್ಣ *)regp && regused < 8)
	{
	  regp += z / ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	  regused += z / ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	}
      ಅನ್ಯಥಾ
	stackp += z;
    }

  ಪ್ರತ್ಯಾಗಮ struct_ಮೌಲ್ಯ_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      cif->flags = 0;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      ಯದಿ (cif->rtype->size == ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) &&
	  cif->rtype->alignment == ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	cif->flags = CIF_FLAGS_INT;
      ಅನ್ಯಥಾ
	cif->flags = 0;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_DOUBLE:
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      cif->flags = CIF_FLAGS_INT;
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ಯದಿ (rvalue == ರಿಕ್ತ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->size != ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ)
	  || cif->rtype->alignment != ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ)))
    ecif.rvalue = alloca (cif->rtype->size);
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      FFI_ASSERT (0);
      ಭಙ್ಗ;
    }
}

/*
 * Closure API
 */

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_closure_args_OBSD (ffi_cif *cif, ಶೂನ್ಯ **avalue, unsigned ಪೂರ್ಣಾಙ್ಕ *regp,
			    ವರ್ಣ *stackp)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಶೂನ್ಯ **p_argv;
  ವರ್ಣ *argp;
  unsigned ಪೂರ್ಣಾಙ್ಕ regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  ಪ್ರತಿ (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      unsigned short t, a;

      z = (*p_arg)->size;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument has been passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      ಯದಿ (t == FFI_TYPE_STRUCT)
	{
	  ಯದಿ (z == ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) && a == ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) && regused < 8)
	    argp = (ವರ್ಣ *)regp;
	  ಅನ್ಯಥಾ
	    argp = stackp;
	}
      ಅನ್ಯಥಾ
	{
	  ಯದಿ (z > ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ಯದಿ (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ಯದಿ (regused < 8)
	    argp = (ವರ್ಣ *)regp;
	  ಅನ್ಯಥಾ
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ಯದಿ (argp == stackp && a > ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	{
	  stackp = (ವರ್ಣ *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ಯದಿ (z < ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) && t != FFI_TYPE_STRUCT)
	*p_argv = (ಶೂನ್ಯ *) (argp + ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - z);
      ಅನ್ಯಥಾ
	*p_argv = (ಶೂನ್ಯ *) argp;

      /* Align if necessary */
      ಯದಿ ((ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - 1) & z)
	z = FFI_ALIGN(z, ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      ಯದಿ (argp == (ವರ್ಣ *)regp && regused < 8)
	{
	  regp += z / ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	  regused += z / ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	}
      ಅನ್ಯಥಾ
	stackp += z;
    }
}

unsigned ಪೂರ್ಣಾಙ್ಕ
ffi_closure_OBSD_inner (ffi_closure *closure, ಶೂನ್ಯ *resp, unsigned ಪೂರ್ಣಾಙ್ಕ *regp,
			ವರ್ಣ *stackp)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **arg_area;

  cif = closure->cif;
  arg_area = (ಶೂನ್ಯ**) alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  ಪ್ರತ್ಯಾಗಮ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data, ಶೂನ್ಯ *codeloc)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ *tramp = (unsigned ಪೂರ್ಣಾಙ್ಕ *) codeloc;
  ಶೂನ್ಯ *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  ಅನ್ಯಥಾ
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned ಪೂರ್ಣಾಙ್ಕ)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned ಪೂರ್ಣಾಙ್ಕ)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned ಪೂರ್ಣಾಙ್ಕ)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned ಪೂರ್ಣಾಙ್ಕ)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned ಪೂರ್ಣಾಙ್ಕ)codeloc, FFI_TRAMPOLINE_SIZE);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}
