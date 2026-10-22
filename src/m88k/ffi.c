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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

ਖਾਲੀ ffi_call_OBSD (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ, extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ, ਖਾਲੀ *,
		    ਖਾਲੀ (*fn) ());
ਖਾਲੀ *ffi_prep_ਦਲੀਲਾਂ (ਖਾਲੀ *, extended_cif *);
ਖਾਲੀ ffi_ਘੇਰਾ_OBSD (ffi_ਘੇਰਾ *);
ਖਾਲੀ ffi_ਘੇਰਾ_struct_OBSD (ffi_ਘੇਰਾ *);
ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ffi_ਘੇਰਾ_OBSD_inner (ffi_ਘੇਰਾ *, ਖਾਲੀ *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *,
				     ਅੱਖਰ *);
ਖਾਲੀ ffi_cacheflush_OBSD (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_INT		(1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

ਖਾਲੀ *
ffi_prep_ਦਲੀਲਾਂ (ਖਾਲੀ *ਢੇਰ, extended_cif *ecif)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਖਾਲੀ **p_argv;
  ਅੱਖਰ *argp, *stackp;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *regp;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ regused;
  ffi_type **p_arg;
  ਖਾਲੀ *struct_ਮੁੱਲ_ptr;

  regp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)ਢੇਰ;
  stackp = (ਅੱਖਰ *)(regp + 8);
  regused = 0;

  ਜੇ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_ਮੁੱਲ_ptr = ecif->rvalue;
  ਨਹੀਂਤਾਂ
    struct_ਮੁੱਲ_ptr = ਨਿਰਾ;

  p_argv = ecif->avalue;

  ਲਈ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ t, a;

      z = (*p_arg)->ਮਾਪ;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument can be passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      ਜੇ (t == FFI_TYPE_STRUCT)
	{
	  ਜੇ (z == ਆਕਾਰ (ਪੂਰਨ) && a == ਆਕਾਰ (ਪੂਰਨ) && regused < 8)
	    argp = (ਅੱਖਰ *)regp;
	  ਨਹੀਂਤਾਂ
	    argp = stackp;
	}
      ਨਹੀਂਤਾਂ
	{
	  ਜੇ (z > ਆਕਾਰ (ਪੂਰਨ) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ਜੇ (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ਜੇ (regused < 8)
	    argp = (ਅੱਖਰ *)regp;
	  ਨਹੀਂਤਾਂ
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ਜੇ (argp == stackp && a > ਆਕਾਰ (ਪੂਰਨ))
	{
	  stackp = (ਅੱਖਰ *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ਚੋਣ (t)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	  *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ) *(SINT8 *) *p_argv;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT8:
	  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) *(UINT8 *) *p_argv;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT16:
	  *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ) *(SINT16 *) *p_argv;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT16:
	  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) *(UINT16 *) *p_argv;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) *p_argv;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
	  ਰੋਕੋ;

	ਮੂਲ:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      ਜੇ ((ਆਕਾਰ (ਪੂਰਨ) - 1) & z)
	z = FFI_ALIGN(z, ਆਕਾਰ (ਪੂਰਨ));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      ਜੇ (argp == (ਅੱਖਰ *)regp && regused < 8)
	{
	  regp += z / ਆਕਾਰ (ਪੂਰਨ);
	  regused += z / ਆਕਾਰ (ਪੂਰਨ);
	}
      ਨਹੀਂਤਾਂ
	stackp += z;
    }

  ਵਾਪਸ struct_ਮੁੱਲ_ptr;
}

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      cif->flags = 0;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਜੇ (cif->rtype->ਮਾਪ == ਆਕਾਰ (ਪੂਰਨ) &&
	  cif->rtype->alignment == ਆਕਾਰ (ਪੂਰਨ))
	cif->flags = CIF_FLAGS_INT;
      ਨਹੀਂਤਾਂ
	cif->flags = 0;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_DOUBLE:
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ਰੋਕੋ;

    ਮੂਲ:
      cif->flags = CIF_FLAGS_INT;
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn) (), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ਜੇ (rvalue == ਨਿਰਾ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->ਮਾਪ != ਆਕਾਰ (ਪੂਰਨ)
	  || cif->rtype->alignment != ਆਕਾਰ (ਪੂਰਨ)))
    ecif.rvalue = alloca (cif->rtype->ਮਾਪ);
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_OBSD:
      ffi_call_OBSD (cif->ਬਾਈਟਾਂ, &ecif, cif->flags, ecif.rvalue, fn);
      ਰੋਕੋ;

    ਮੂਲ:
      FFI_ASSERT (0);
      ਰੋਕੋ;
    }
}

/*
 * Closure API
 */

ਸਥਿਰ ਖਾਲੀ
ffi_prep_ਘੇਰਾ_ਦਲੀਲਾਂ_OBSD (ffi_cif *cif, ਖਾਲੀ **avalue, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *regp,
			    ਅੱਖਰ *stackp)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਖਾਲੀ **p_argv;
  ਅੱਖਰ *argp;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  ਲਈ (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ t, a;

      z = (*p_arg)->ਮਾਪ;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument has been passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      ਜੇ (t == FFI_TYPE_STRUCT)
	{
	  ਜੇ (z == ਆਕਾਰ (ਪੂਰਨ) && a == ਆਕਾਰ (ਪੂਰਨ) && regused < 8)
	    argp = (ਅੱਖਰ *)regp;
	  ਨਹੀਂਤਾਂ
	    argp = stackp;
	}
      ਨਹੀਂਤਾਂ
	{
	  ਜੇ (z > ਆਕਾਰ (ਪੂਰਨ) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ਜੇ (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ਜੇ (regused < 8)
	    argp = (ਅੱਖਰ *)regp;
	  ਨਹੀਂਤਾਂ
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ਜੇ (argp == stackp && a > ਆਕਾਰ (ਪੂਰਨ))
	{
	  stackp = (ਅੱਖਰ *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ਜੇ (z < ਆਕਾਰ (ਪੂਰਨ) && t != FFI_TYPE_STRUCT)
	*p_argv = (ਖਾਲੀ *) (argp + ਆਕਾਰ (ਪੂਰਨ) - z);
      ਨਹੀਂਤਾਂ
	*p_argv = (ਖਾਲੀ *) argp;

      /* Align if necessary */
      ਜੇ ((ਆਕਾਰ (ਪੂਰਨ) - 1) & z)
	z = FFI_ALIGN(z, ਆਕਾਰ (ਪੂਰਨ));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      ਜੇ (argp == (ਅੱਖਰ *)regp && regused < 8)
	{
	  regp += z / ਆਕਾਰ (ਪੂਰਨ);
	  regused += z / ਆਕਾਰ (ਪੂਰਨ);
	}
      ਨਹੀਂਤਾਂ
	stackp += z;
    }
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
ffi_ਘੇਰਾ_OBSD_inner (ffi_ਘੇਰਾ *ਘੇਰਾ, ਖਾਲੀ *resp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *regp,
			ਅੱਖਰ *stackp)
{
  ffi_cif *cif;
  ਖਾਲੀ **arg_area;

  cif = ਘੇਰਾ->cif;
  arg_area = (ਖਾਲੀ**) alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));

  ffi_prep_ਘੇਰਾ_ਦਲੀਲਾਂ_OBSD(cif, arg_area, regp, stackp);

  (ਘੇਰਾ->fun) (cif, resp, arg_area, ਘੇਰਾ->user_ਅੰਕੜੇ);

  ਵਾਪਸ cif->flags;
}

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ, ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ, ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) codeloc;
  ਖਾਲੀ *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_ਘੇਰਾ_struct_OBSD;
  ਨਹੀਂਤਾਂ
    fn = &ffi_ਘੇਰਾ_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਘੇਰਾ >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਘੇਰਾ & 0xffff);

  ffi_cacheflush_OBSD((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)codeloc, FFI_TRAMPOLINE_ਮਾਪ);

  ਘੇਰਾ->cif  = cif;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  ਘੇਰਾ->fun  = fun;

  ਵਾਪਸ FFI_OK;
}
