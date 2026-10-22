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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>

शून्य ffi_call_OBSD (unsigned पूर्णाङ्क, extended_cif *, unsigned पूर्णाङ्क, शून्य *,
		    शून्य (*fn) ());
शून्य *ffi_prep_args (शून्य *, extended_cif *);
शून्य ffi_closure_OBSD (ffi_closure *);
शून्य ffi_closure_struct_OBSD (ffi_closure *);
unsigned पूर्णाङ्क ffi_closure_OBSD_inner (ffi_closure *, शून्य *, unsigned पूर्णाङ्क *,
				     वर्ण *);
शून्य ffi_cacheflush_OBSD (unsigned पूर्णाङ्क, unsigned पूर्णाङ्क);

#define CIF_FLAGS_INT		(1 << 0)
#define CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

शून्य *
ffi_prep_args (शून्य *चळत, extended_cif *ecif)
{
  unsigned पूर्णाङ्क i;
  शून्य **p_argv;
  वर्ण *argp, *stackp;
  unsigned पूर्णाङ्क *regp;
  unsigned पूर्णाङ्क regused;
  ffi_type **p_arg;
  शून्य *struct_मूल्य_ptr;

  regp = (unsigned पूर्णाङ्क *)चळत;
  stackp = (वर्ण *)(regp + 8);
  regused = 0;

  यदि (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_मूल्य_ptr = ecif->rvalue;
  अन्यथा
    struct_मूल्य_ptr = रिक्त;

  p_argv = ecif->avalue;

  प्रति (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
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
      यदि (t == FFI_TYPE_STRUCT)
	{
	  यदि (z == परिमाण (पूर्णाङ्क) && a == परिमाण (पूर्णाङ्क) && regused < 8)
	    argp = (वर्ण *)regp;
	  अन्यथा
	    argp = stackp;
	}
      अन्यथा
	{
	  यदि (z > परिमाण (पूर्णाङ्क) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      यदि (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  यदि (regused < 8)
	    argp = (वर्ण *)regp;
	  अन्यथा
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      यदि (argp == stackp && a > परिमाण (पूर्णाङ्क))
	{
	  stackp = (वर्ण *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      विकल्प (t)
	{
	पक्ष FFI_TYPE_SINT8:
	  *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क) *(SINT8 *) *p_argv;
	  भङ्ग;

	पक्ष FFI_TYPE_UINT8:
	  *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क) *(UINT8 *) *p_argv;
	  भङ्ग;

	पक्ष FFI_TYPE_SINT16:
	  *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क) *(SINT16 *) *p_argv;
	  भङ्ग;

	पक्ष FFI_TYPE_UINT16:
	  *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क) *(UINT16 *) *p_argv;
	  भङ्ग;

	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_POINTER:
	  *(unsigned पूर्णाङ्क *) argp = *(unsigned पूर्णाङ्क *) *p_argv;
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  भङ्ग;

	स्वभाव:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      यदि ((परिमाण (पूर्णाङ्क) - 1) & z)
	z = FFI_ALIGN(z, परिमाण (पूर्णाङ्क));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      यदि (argp == (वर्ण *)regp && regused < 8)
	{
	  regp += z / परिमाण (पूर्णाङ्क);
	  regused += z / परिमाण (पूर्णाङ्क);
	}
      अन्यथा
	stackp += z;
    }

  प्रत्यागम struct_मूल्य_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      cif->flags = 0;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      यदि (cif->rtype->size == परिमाण (पूर्णाङ्क) &&
	  cif->rtype->alignment == परिमाण (पूर्णाङ्क))
	cif->flags = CIF_FLAGS_INT;
      अन्यथा
	cif->flags = 0;
      भङ्ग;

    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      भङ्ग;

    स्वभाव:
      cif->flags = CIF_FLAGS_INT;
      भङ्ग;
    }

  प्रत्यागम FFI_OK;
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  यदि (rvalue == रिक्त
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->size != परिमाण (पूर्णाङ्क)
	  || cif->rtype->alignment != परिमाण (पूर्णाङ्क)))
    ecif.rvalue = alloca (cif->rtype->size);
  अन्यथा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      भङ्ग;

    स्वभाव:
      FFI_ASSERT (0);
      भङ्ग;
    }
}

/*
 * Closure API
 */

स्थिर शून्य
ffi_prep_closure_args_OBSD (ffi_cif *cif, शून्य **avalue, unsigned पूर्णाङ्क *regp,
			    वर्ण *stackp)
{
  unsigned पूर्णाङ्क i;
  शून्य **p_argv;
  वर्ण *argp;
  unsigned पूर्णाङ्क regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  प्रति (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
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
      यदि (t == FFI_TYPE_STRUCT)
	{
	  यदि (z == परिमाण (पूर्णाङ्क) && a == परिमाण (पूर्णाङ्क) && regused < 8)
	    argp = (वर्ण *)regp;
	  अन्यथा
	    argp = stackp;
	}
      अन्यथा
	{
	  यदि (z > परिमाण (पूर्णाङ्क) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      यदि (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  यदि (regused < 8)
	    argp = (वर्ण *)regp;
	  अन्यथा
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      यदि (argp == stackp && a > परिमाण (पूर्णाङ्क))
	{
	  stackp = (वर्ण *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      यदि (z < परिमाण (पूर्णाङ्क) && t != FFI_TYPE_STRUCT)
	*p_argv = (शून्य *) (argp + परिमाण (पूर्णाङ्क) - z);
      अन्यथा
	*p_argv = (शून्य *) argp;

      /* Align if necessary */
      यदि ((परिमाण (पूर्णाङ्क) - 1) & z)
	z = FFI_ALIGN(z, परिमाण (पूर्णाङ्क));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      यदि (argp == (वर्ण *)regp && regused < 8)
	{
	  regp += z / परिमाण (पूर्णाङ्क);
	  regused += z / परिमाण (पूर्णाङ्क);
	}
      अन्यथा
	stackp += z;
    }
}

unsigned पूर्णाङ्क
ffi_closure_OBSD_inner (ffi_closure *closure, शून्य *resp, unsigned पूर्णाङ्क *regp,
			वर्ण *stackp)
{
  ffi_cif *cif;
  शून्य **arg_area;

  cif = closure->cif;
  arg_area = (शून्य**) alloca (cif->nargs * परिमाण (शून्य *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  प्रत्यागम cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		      शून्य *user_data, शून्य *codeloc)
{
  unsigned पूर्णाङ्क *tramp = (unsigned पूर्णाङ्क *) codeloc;
  शून्य *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  यदि (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  अन्यथा
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned पूर्णाङ्क)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned पूर्णाङ्क)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned पूर्णाङ्क)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned पूर्णाङ्क)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned पूर्णाङ्क)codeloc, FFI_TRAMPOLINE_SIZE);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  प्रत्यागम FFI_OK;
}
