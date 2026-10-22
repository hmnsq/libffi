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

ಶೂನ್ಯ ffi_call_OBSD (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ, extended_cif *, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ, ಶೂನ್ಯ *,
		    ಶೂನ್ಯ (*fn) ());
ಶೂನ್ಯ *ffi_prep_ವಾದಗಳು (ಶೂನ್ಯ *, extended_cif *);
ಶೂನ್ಯ ffi_ಆವರಣ_OBSD (ffi_ಆವರಣ *);
ಶೂನ್ಯ ffi_ಆವರಣ_struct_OBSD (ffi_ಆವರಣ *);
ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ffi_ಆವರಣ_OBSD_inner (ffi_ಆವರಣ *, ಶೂನ್ಯ *, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *,
				     ಅಕ್ಷರ *);
ಶೂನ್ಯ ffi_cacheflush_OBSD (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);

#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_INT		(1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

ಶೂನ್ಯ *
ffi_prep_ವಾದಗಳು (ಶೂನ್ಯ *ರಾಶಿ, extended_cif *ecif)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಶೂನ್ಯ **p_argv;
  ಅಕ್ಷರ *argp, *stackp;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *regp;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ regused;
  ffi_type **p_arg;
  ಶೂನ್ಯ *struct_ಮೌಲ್ಯ_ptr;

  regp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *)ರಾಶಿ;
  stackp = (ಅಕ್ಷರ *)(regp + 8);
  regused = 0;

  ಆದರೆ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_ಮೌಲ್ಯ_ptr = ecif->rvalue;
  ಇಲ್ಲದಿದ್ದರೆ
    struct_ಮೌಲ್ಯ_ptr = ಖಾಲಿ;

  p_argv = ecif->avalue;

  ಗಾಗಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ t, a;

      z = (*p_arg)->ಗಾತ್ರ_2;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument can be passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      ಆದರೆ (t == FFI_TYPE_STRUCT)
	{
	  ಆದರೆ (z == ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) && a == ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) && regused < 8)
	    argp = (ಅಕ್ಷರ *)regp;
	  ಇಲ್ಲದಿದ್ದರೆ
	    argp = stackp;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಆದರೆ (z > ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ಆದರೆ (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ಆದರೆ (regused < 8)
	    argp = (ಅಕ್ಷರ *)regp;
	  ಇಲ್ಲದಿದ್ದರೆ
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ಆದರೆ (argp == stackp && a > ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	{
	  stackp = (ಅಕ್ಷರ *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ಆಯ್ಕೆ (t)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ) *(SINT8 *) *p_argv;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) *(UINT8 *) *p_argv;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ) *(SINT16 *) *p_argv;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) *(UINT16 *) *p_argv;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) *p_argv;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      ಆದರೆ ((ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - 1) & z)
	z = FFI_ALIGN(z, ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      ಆದರೆ (argp == (ಅಕ್ಷರ *)regp && regused < 8)
	{
	  regp += z / ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	  regused += z / ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	}
      ಇಲ್ಲದಿದ್ದರೆ
	stackp += z;
    }

  ಹಿಂತಿರುಗಿ struct_ಮೌಲ್ಯ_ptr;
}

/* Perform machine dependent cif processing */
ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      cif->flags = 0;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 == ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) &&
	  cif->rtype->alignment == ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	cif->flags = CIF_FLAGS_INT;
      ಇಲ್ಲದಿದ್ದರೆ
	cif->flags = 0;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      cif->flags = CIF_FLAGS_INT;
      ನಿಲ್ಲಿಸು;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ಆದರೆ (rvalue == ಖಾಲಿ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->ಗಾತ್ರ_2 != ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ)
	  || cif->rtype->alignment != ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ)))
    ecif.rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_OBSD:
      ffi_call_OBSD (cif->ಬೈಟುಗಳು, &ecif, cif->flags, ecif.rvalue, fn);
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT (0);
      ನಿಲ್ಲಿಸು;
    }
}

/*
 * Closure API
 */

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_ಆವರಣ_ವಾದಗಳು_OBSD (ffi_cif *cif, ಶೂನ್ಯ **avalue, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *regp,
			    ಅಕ್ಷರ *stackp)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಶೂನ್ಯ **p_argv;
  ಅಕ್ಷರ *argp;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  ಗಾಗಿ (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ t, a;

      z = (*p_arg)->ಗಾತ್ರ_2;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument has been passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      ಆದರೆ (t == FFI_TYPE_STRUCT)
	{
	  ಆದರೆ (z == ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) && a == ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) && regused < 8)
	    argp = (ಅಕ್ಷರ *)regp;
	  ಇಲ್ಲದಿದ್ದರೆ
	    argp = stackp;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಆದರೆ (z > ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ಆದರೆ (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ಆದರೆ (regused < 8)
	    argp = (ಅಕ್ಷರ *)regp;
	  ಇಲ್ಲದಿದ್ದರೆ
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ಆದರೆ (argp == stackp && a > ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	{
	  stackp = (ಅಕ್ಷರ *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ಆದರೆ (z < ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) && t != FFI_TYPE_STRUCT)
	*p_argv = (ಶೂನ್ಯ *) (argp + ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - z);
      ಇಲ್ಲದಿದ್ದರೆ
	*p_argv = (ಶೂನ್ಯ *) argp;

      /* Align if necessary */
      ಆದರೆ ((ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - 1) & z)
	z = FFI_ALIGN(z, ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      ಆದರೆ (argp == (ಅಕ್ಷರ *)regp && regused < 8)
	{
	  regp += z / ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	  regused += z / ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	}
      ಇಲ್ಲದಿದ್ದರೆ
	stackp += z;
    }
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_OBSD_inner (ffi_ಆವರಣ *ಆವರಣ, ಶೂನ್ಯ *resp, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *regp,
			ಅಕ್ಷರ *stackp)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **arg_area;

  cif = ಆವರಣ->cif;
  arg_area = (ಶೂನ್ಯ**) alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));

  ffi_prep_ಆವರಣ_ವಾದಗಳು_OBSD(cif, arg_area, regp, stackp);

  (ಆವರಣ->fun) (cif, resp, arg_area, ಆವರಣ->user_ದತ್ತಾಂಶ);

  ಹಿಂತಿರುಗಿ cif->flags;
}

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ, ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) codeloc;
  ಶೂನ್ಯ *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_ಆವರಣ_struct_OBSD;
  ಇಲ್ಲದಿದ್ದರೆ
    fn = &ffi_ಆವರಣ_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)ಆವರಣ >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)ಆವರಣ & 0xffff);

  ffi_cacheflush_OBSD((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)codeloc, FFI_TRAMPOLINE_ಗಾತ್ರ);

  ಆವರಣ->cif  = cif;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
  ಆವರಣ->fun  = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}
