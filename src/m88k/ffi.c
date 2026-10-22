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

تهی ffi_call_OBSD (unsigned صحیح, extended_cif *, unsigned صحیح, تهی *,
		    تهی (*fn) ());
تهی *ffi_prep_args (تهی *, extended_cif *);
تهی ffi_closure_OBSD (ffi_closure *);
تهی ffi_closure_struct_OBSD (ffi_closure *);
unsigned صحیح ffi_closure_OBSD_inner (ffi_closure *, تهی *, unsigned صحیح *,
				     نویسه *);
تهی ffi_cacheflush_OBSD (unsigned صحیح, unsigned صحیح);

#define CIF_FLAGS_INT		(1 << 0)
#define CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

تهی *
ffi_prep_args (تهی *پشته, extended_cif *ecif)
{
  unsigned صحیح i;
  تهی **p_argv;
  نویسه *argp, *stackp;
  unsigned صحیح *regp;
  unsigned صحیح regused;
  ffi_type **p_arg;
  تهی *struct_مقدار_ptr;

  regp = (unsigned صحیح *)پشته;
  stackp = (نویسه *)(regp + 8);
  regused = 0;

  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_مقدار_ptr = ecif->rvalue;
  وگرنه
    struct_مقدار_ptr = پوچ;

  p_argv = ecif->avalue;

  برای (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
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
      اگر (t == FFI_TYPE_STRUCT)
	{
	  اگر (z == اندازه (صحیح) && a == اندازه (صحیح) && regused < 8)
	    argp = (نویسه *)regp;
	  وگرنه
	    argp = stackp;
	}
      وگرنه
	{
	  اگر (z > اندازه (صحیح) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      اگر (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  اگر (regused < 8)
	    argp = (نویسه *)regp;
	  وگرنه
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      اگر (argp == stackp && a > اندازه (صحیح))
	{
	  stackp = (نویسه *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      انتخاب (t)
	{
	حالت FFI_TYPE_SINT8:
	  *(signed صحیح *) argp = (signed صحیح) *(SINT8 *) *p_argv;
	  بشکن;

	حالت FFI_TYPE_UINT8:
	  *(unsigned صحیح *) argp = (unsigned صحیح) *(UINT8 *) *p_argv;
	  بشکن;

	حالت FFI_TYPE_SINT16:
	  *(signed صحیح *) argp = (signed صحیح) *(SINT16 *) *p_argv;
	  بشکن;

	حالت FFI_TYPE_UINT16:
	  *(unsigned صحیح *) argp = (unsigned صحیح) *(UINT16 *) *p_argv;
	  بشکن;

	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_POINTER:
	  *(unsigned صحیح *) argp = *(unsigned صحیح *) *p_argv;
	  بشکن;

	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  بشکن;

	پیشفرض:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      اگر ((اندازه (صحیح) - 1) & z)
	z = FFI_ALIGN(z, اندازه (صحیح));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      اگر (argp == (نویسه *)regp && regused < 8)
	{
	  regp += z / اندازه (صحیح);
	  regused += z / اندازه (صحیح);
	}
      وگرنه
	stackp += z;
    }

  بازگشت struct_مقدار_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      cif->flags = 0;
      بشکن;

    حالت FFI_TYPE_STRUCT:
      اگر (cif->rtype->size == اندازه (صحیح) &&
	  cif->rtype->alignment == اندازه (صحیح))
	cif->flags = CIF_FLAGS_INT;
      وگرنه
	cif->flags = 0;
      بشکن;

    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      بشکن;

    پیشفرض:
      cif->flags = CIF_FLAGS_INT;
      بشکن;
    }

  بازگشت FFI_OK;
}

تهی
ffi_call (ffi_cif *cif, تهی (*fn) (), تهی *rvalue, تهی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  اگر (rvalue == پوچ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->size != اندازه (صحیح)
	  || cif->rtype->alignment != اندازه (صحیح)))
    ecif.rvalue = alloca (cif->rtype->size);
  وگرنه
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      بشکن;

    پیشفرض:
      FFI_ASSERT (0);
      بشکن;
    }
}

/*
 * Closure API
 */

ایستا تهی
ffi_prep_closure_args_OBSD (ffi_cif *cif, تهی **avalue, unsigned صحیح *regp,
			    نویسه *stackp)
{
  unsigned صحیح i;
  تهی **p_argv;
  نویسه *argp;
  unsigned صحیح regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  برای (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
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
      اگر (t == FFI_TYPE_STRUCT)
	{
	  اگر (z == اندازه (صحیح) && a == اندازه (صحیح) && regused < 8)
	    argp = (نویسه *)regp;
	  وگرنه
	    argp = stackp;
	}
      وگرنه
	{
	  اگر (z > اندازه (صحیح) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      اگر (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  اگر (regused < 8)
	    argp = (نویسه *)regp;
	  وگرنه
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      اگر (argp == stackp && a > اندازه (صحیح))
	{
	  stackp = (نویسه *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      اگر (z < اندازه (صحیح) && t != FFI_TYPE_STRUCT)
	*p_argv = (تهی *) (argp + اندازه (صحیح) - z);
      وگرنه
	*p_argv = (تهی *) argp;

      /* Align if necessary */
      اگر ((اندازه (صحیح) - 1) & z)
	z = FFI_ALIGN(z, اندازه (صحیح));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      اگر (argp == (نویسه *)regp && regused < 8)
	{
	  regp += z / اندازه (صحیح);
	  regused += z / اندازه (صحیح);
	}
      وگرنه
	stackp += z;
    }
}

unsigned صحیح
ffi_closure_OBSD_inner (ffi_closure *closure, تهی *resp, unsigned صحیح *regp,
			نویسه *stackp)
{
  ffi_cif *cif;
  تهی **arg_area;

  cif = closure->cif;
  arg_area = (تهی**) alloca (cif->nargs * اندازه (تهی *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  بازگشت cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*),
		      تهی *user_data, تهی *codeloc)
{
  unsigned صحیح *tramp = (unsigned صحیح *) codeloc;
  تهی *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  اگر (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  وگرنه
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned صحیح)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned صحیح)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned صحیح)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned صحیح)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned صحیح)codeloc, FFI_TRAMPOLINE_SIZE);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  بازگشت FFI_OK;
}
