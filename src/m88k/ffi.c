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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>
#شامل <unistd.h>

خالی ffi_call_OBSD (غیرعلامتی صحیح, extended_cif *, غیرعلامتی صحیح, خالی *,
		    خالی (*fn) ());
خالی *ffi_prep_دلائل (خالی *, extended_cif *);
خالی ffi_احاطہ_OBSD (ffi_احاطہ *);
خالی ffi_احاطہ_struct_OBSD (ffi_احاطہ *);
غیرعلامتی صحیح ffi_احاطہ_OBSD_inner (ffi_احاطہ *, خالی *, غیرعلامتی صحیح *,
				     حرف *);
خالی ffi_cacheflush_OBSD (غیرعلامتی صحیح, غیرعلامتی صحیح);

#تعریف_کرو CIF_FLAGS_INT		(1 << 0)
#تعریف_کرو CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

خالی *
ffi_prep_دلائل (خالی *ڈھیر, extended_cif *ecif)
{
  غیرعلامتی صحیح i;
  خالی **p_argv;
  حرف *argp, *stackp;
  غیرعلامتی صحیح *regp;
  غیرعلامتی صحیح regused;
  ffi_type **p_arg;
  خالی *struct_قدر_ptr;

  regp = (غیرعلامتی صحیح *)ڈھیر;
  stackp = (حرف *)(regp + 8);
  regused = 0;

  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_قدر_ptr = ecif->rvalue;
  ورنہ
    struct_قدر_ptr = تہی;

  p_argv = ecif->avalue;

  برائے (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      غیرعلامتی مختصر t, a;

      z = (*p_arg)->سائز;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument can be passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      اگر (t == FFI_TYPE_STRUCT)
	{
	  اگر (z == حجم (صحیح) && a == حجم (صحیح) && regused < 8)
	    argp = (حرف *)regp;
	  ورنہ
	    argp = stackp;
	}
      ورنہ
	{
	  اگر (z > حجم (صحیح) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      اگر (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  اگر (regused < 8)
	    argp = (حرف *)regp;
	  ورنہ
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      اگر (argp == stackp && a > حجم (صحیح))
	{
	  stackp = (حرف *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      انتخاب (t)
	{
	حالت FFI_TYPE_SINT8:
	  *(علامتی صحیح *) argp = (علامتی صحیح) *(SINT8 *) *p_argv;
	  توڑو;

	حالت FFI_TYPE_UINT8:
	  *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح) *(UINT8 *) *p_argv;
	  توڑو;

	حالت FFI_TYPE_SINT16:
	  *(علامتی صحیح *) argp = (علامتی صحیح) *(SINT16 *) *p_argv;
	  توڑو;

	حالت FFI_TYPE_UINT16:
	  *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح) *(UINT16 *) *p_argv;
	  توڑو;

	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_اشارہ_گر:
	  *(غیرعلامتی صحیح *) argp = *(غیرعلامتی صحیح *) *p_argv;
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_STRUCT:
	  حافظہ_نقل (argp, *p_argv, z);
	  توڑو;

	طےشدہ:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      اگر ((حجم (صحیح) - 1) & z)
	z = FFI_ALIGN(z, حجم (صحیح));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      اگر (argp == (حرف *)regp && regused < 8)
	{
	  regp += z / حجم (صحیح);
	  regused += z / حجم (صحیح);
	}
      ورنہ
	stackp += z;
    }

  واپس struct_قدر_ptr;
}

/* Perform machine dependent cif processing */
ffi_حالت
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      cif->flags = 0;
      توڑو;

    حالت FFI_TYPE_STRUCT:
      اگر (cif->rtype->سائز == حجم (صحیح) &&
	  cif->rtype->alignment == حجم (صحیح))
	cif->flags = CIF_FLAGS_INT;
      ورنہ
	cif->flags = 0;
      توڑو;

    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      توڑو;

    طےشدہ:
      cif->flags = CIF_FLAGS_INT;
      توڑو;
    }

  واپس FFI_OK;
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn) (), خالی *rvalue, خالی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  اگر (rvalue == تہی
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->سائز != حجم (صحیح)
	  || cif->rtype->alignment != حجم (صحیح)))
    ecif.rvalue = alloca (cif->rtype->سائز);
  ورنہ
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_OBSD:
      ffi_call_OBSD (cif->بائٹیں, &ecif, cif->flags, ecif.rvalue, fn);
      توڑو;

    طےشدہ:
      FFI_ASSERT (0);
      توڑو;
    }
}

/*
 * Closure API
 */

ساکن خالی
ffi_prep_احاطہ_دلائل_OBSD (ffi_cif *cif, خالی **avalue, غیرعلامتی صحیح *regp,
			    حرف *stackp)
{
  غیرعلامتی صحیح i;
  خالی **p_argv;
  حرف *argp;
  غیرعلامتی صحیح regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  برائے (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      غیرعلامتی مختصر t, a;

      z = (*p_arg)->سائز;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument has been passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      اگر (t == FFI_TYPE_STRUCT)
	{
	  اگر (z == حجم (صحیح) && a == حجم (صحیح) && regused < 8)
	    argp = (حرف *)regp;
	  ورنہ
	    argp = stackp;
	}
      ورنہ
	{
	  اگر (z > حجم (صحیح) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      اگر (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  اگر (regused < 8)
	    argp = (حرف *)regp;
	  ورنہ
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      اگر (argp == stackp && a > حجم (صحیح))
	{
	  stackp = (حرف *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      اگر (z < حجم (صحیح) && t != FFI_TYPE_STRUCT)
	*p_argv = (خالی *) (argp + حجم (صحیح) - z);
      ورنہ
	*p_argv = (خالی *) argp;

      /* Align if necessary */
      اگر ((حجم (صحیح) - 1) & z)
	z = FFI_ALIGN(z, حجم (صحیح));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      اگر (argp == (حرف *)regp && regused < 8)
	{
	  regp += z / حجم (صحیح);
	  regused += z / حجم (صحیح);
	}
      ورنہ
	stackp += z;
    }
}

غیرعلامتی صحیح
ffi_احاطہ_OBSD_inner (ffi_احاطہ *احاطہ, خالی *resp, غیرعلامتی صحیح *regp,
			حرف *stackp)
{
  ffi_cif *cif;
  خالی **arg_area;

  cif = احاطہ->cif;
  arg_area = (خالی**) alloca (cif->nargs * حجم (خالی *));

  ffi_prep_احاطہ_دلائل_OBSD(cif, arg_area, regp, stackp);

  (احاطہ->fun) (cif, resp, arg_area, احاطہ->user_ڈیٹا);

  واپس cif->flags;
}

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ, ffi_cif* cif,
		      خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
		      خالی *user_ڈیٹا, خالی *codeloc)
{
  غیرعلامتی صحیح *tramp = (غیرعلامتی صحیح *) codeloc;
  خالی *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  اگر (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_احاطہ_struct_OBSD;
  ورنہ
    fn = &ffi_احاطہ_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((غیرعلامتی صحیح)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((غیرعلامتی صحیح)احاطہ >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((غیرعلامتی صحیح)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((غیرعلامتی صحیح)احاطہ & 0xffff);

  ffi_cacheflush_OBSD((غیرعلامتی صحیح)codeloc, FFI_TRAMPOLINE_سائز);

  احاطہ->cif  = cif;
  احاطہ->user_ڈیٹا = user_ڈیٹا;
  احاطہ->fun  = fun;

  واپس FFI_OK;
}
