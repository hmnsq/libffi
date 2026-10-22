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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <unistd.h>

فراغ ffi_call_OBSD (بنانشانی صحيح, extended_cif *, بنانشانی صحيح, فراغ *,
		    فراغ (*fn) ());
فراغ *ffi_prep_دلیلاں (فراغ *, extended_cif *);
فراغ ffi_گھیرا_OBSD (ffi_گھیرا *);
فراغ ffi_گھیرا_struct_OBSD (ffi_گھیرا *);
بنانشانی صحيح ffi_گھیرا_OBSD_inner (ffi_گھیرا *, فراغ *, بنانشانی صحيح *,
				     حرف *);
فراغ ffi_cacheflush_OBSD (بنانشانی صحيح, بنانشانی صحيح);

#تعریف_کرو CIF_FLAGS_INT		(1 << 0)
#تعریف_کرو CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

فراغ *
ffi_prep_دلیلاں (فراغ *ڈھیر, extended_cif *ecif)
{
  بنانشانی صحيح i;
  فراغ **p_argv;
  حرف *argp, *stackp;
  بنانشانی صحيح *regp;
  بنانشانی صحيح regused;
  ffi_type **p_arg;
  فراغ *struct_قدر_ptr;

  regp = (بنانشانی صحيح *)ڈھیر;
  stackp = (حرف *)(regp + 8);
  regused = 0;

  إذا (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_قدر_ptr = ecif->rvalue;
  وإلا
    struct_قدر_ptr = عدم;

  p_argv = ecif->avalue;

  لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      بنانشانی چھوٹا t, a;

      z = (*p_arg)->ناپ;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument can be passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      إذا (t == FFI_TYPE_STRUCT)
	{
	  إذا (z == حجم (صحيح) && a == حجم (صحيح) && regused < 8)
	    argp = (حرف *)regp;
	  وإلا
	    argp = stackp;
	}
      وإلا
	{
	  إذا (z > حجم (صحيح) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      إذا (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  إذا (regused < 8)
	    argp = (حرف *)regp;
	  وإلا
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      إذا (argp == stackp && a > حجم (صحيح))
	{
	  stackp = (حرف *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      بدل (t)
	{
	حالة FFI_TYPE_SINT8:
	  *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح) *(SINT8 *) *p_argv;
	  اكسر;

	حالة FFI_TYPE_UINT8:
	  *(بنانشانی صحيح *) argp = (بنانشانی صحيح) *(UINT8 *) *p_argv;
	  اكسر;

	حالة FFI_TYPE_SINT16:
	  *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح) *(SINT16 *) *p_argv;
	  اكسر;

	حالة FFI_TYPE_UINT16:
	  *(بنانشانی صحيح *) argp = (بنانشانی صحيح) *(UINT16 *) *p_argv;
	  اكسر;

	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_اشارہ_گر:
	  *(بنانشانی صحيح *) argp = *(بنانشانی صحيح *) *p_argv;
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_STRUCT:
	  حافظہ_نقل (argp, *p_argv, z);
	  اكسر;

	افتراضي:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      إذا ((حجم (صحيح) - 1) & z)
	z = FFI_ALIGN(z, حجم (صحيح));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      إذا (argp == (حرف *)regp && regused < 8)
	{
	  regp += z / حجم (صحيح);
	  regused += z / حجم (صحيح);
	}
      وإلا
	stackp += z;
    }

  أرجع struct_قدر_ptr;
}

/* Perform machine dependent cif processing */
ffi_حالت
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      cif->flags = 0;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      إذا (cif->rtype->ناپ == حجم (صحيح) &&
	  cif->rtype->alignment == حجم (صحيح))
	cif->flags = CIF_FLAGS_INT;
      وإلا
	cif->flags = 0;
      اكسر;

    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      اكسر;

    افتراضي:
      cif->flags = CIF_FLAGS_INT;
      اكسر;
    }

  أرجع FFI_OK;
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  إذا (rvalue == عدم
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->ناپ != حجم (صحيح)
	  || cif->rtype->alignment != حجم (صحيح)))
    ecif.rvalue = alloca (cif->rtype->ناپ);
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_OBSD:
      ffi_call_OBSD (cif->بائٹاں, &ecif, cif->flags, ecif.rvalue, fn);
      اكسر;

    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

/*
 * Closure API
 */

ساكن فراغ
ffi_prep_گھیرا_دلیلاں_OBSD (ffi_cif *cif, فراغ **avalue, بنانشانی صحيح *regp,
			    حرف *stackp)
{
  بنانشانی صحيح i;
  فراغ **p_argv;
  حرف *argp;
  بنانشانی صحيح regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  لكل (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      بنانشانی چھوٹا t, a;

      z = (*p_arg)->ناپ;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument has been passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      إذا (t == FFI_TYPE_STRUCT)
	{
	  إذا (z == حجم (صحيح) && a == حجم (صحيح) && regused < 8)
	    argp = (حرف *)regp;
	  وإلا
	    argp = stackp;
	}
      وإلا
	{
	  إذا (z > حجم (صحيح) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      إذا (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  إذا (regused < 8)
	    argp = (حرف *)regp;
	  وإلا
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      إذا (argp == stackp && a > حجم (صحيح))
	{
	  stackp = (حرف *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      إذا (z < حجم (صحيح) && t != FFI_TYPE_STRUCT)
	*p_argv = (فراغ *) (argp + حجم (صحيح) - z);
      وإلا
	*p_argv = (فراغ *) argp;

      /* Align if necessary */
      إذا ((حجم (صحيح) - 1) & z)
	z = FFI_ALIGN(z, حجم (صحيح));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      إذا (argp == (حرف *)regp && regused < 8)
	{
	  regp += z / حجم (صحيح);
	  regused += z / حجم (صحيح);
	}
      وإلا
	stackp += z;
    }
}

بنانشانی صحيح
ffi_گھیرا_OBSD_inner (ffi_گھیرا *گھیرا, فراغ *resp, بنانشانی صحيح *regp,
			حرف *stackp)
{
  ffi_cif *cif;
  فراغ **arg_area;

  cif = گھیرا->cif;
  arg_area = (فراغ**) alloca (cif->nargs * حجم (فراغ *));

  ffi_prep_گھیرا_دلیلاں_OBSD(cif, arg_area, regp, stackp);

  (گھیرا->fun) (cif, resp, arg_area, گھیرا->user_انکڑے);

  أرجع cif->flags;
}

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا* گھیرا, ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		      فراغ *user_انکڑے, فراغ *codeloc)
{
  بنانشانی صحيح *tramp = (بنانشانی صحيح *) codeloc;
  فراغ *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  إذا (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_گھیرا_struct_OBSD;
  وإلا
    fn = &ffi_گھیرا_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((بنانشانی صحيح)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((بنانشانی صحيح)گھیرا >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((بنانشانی صحيح)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((بنانشانی صحيح)گھیرا & 0xffff);

  ffi_cacheflush_OBSD((بنانشانی صحيح)codeloc, FFI_TRAMPOLINE_ناپ);

  گھیرا->cif  = cif;
  گھیرا->user_انکڑے = user_انکڑے;
  گھیرا->fun  = fun;

  أرجع FFI_OK;
}
