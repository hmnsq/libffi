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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <unistd.h>

শূন্য ffi_call_OBSD (unsigned পূর্ণসংখ্যা, extended_cif *, unsigned পূর্ণসংখ্যা, শূন্য *,
		    শূন্য (*fn) ());
শূন্য *ffi_prep_args (শূন্য *, extended_cif *);
শূন্য ffi_closure_OBSD (ffi_closure *);
শূন্য ffi_closure_struct_OBSD (ffi_closure *);
unsigned পূর্ণসংখ্যা ffi_closure_OBSD_inner (ffi_closure *, শূন্য *, unsigned পূর্ণসংখ্যা *,
				     অক্ষর *);
শূন্য ffi_cacheflush_OBSD (unsigned পূর্ণসংখ্যা, unsigned পূর্ণসংখ্যা);

#define CIF_FLAGS_INT		(1 << 0)
#define CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

শূন্য *
ffi_prep_args (শূন্য *স্তূপ, extended_cif *ecif)
{
  unsigned পূর্ণসংখ্যা i;
  শূন্য **p_argv;
  অক্ষর *argp, *stackp;
  unsigned পূর্ণসংখ্যা *regp;
  unsigned পূর্ণসংখ্যা regused;
  ffi_type **p_arg;
  শূন্য *struct_মান_ptr;

  regp = (unsigned পূর্ণসংখ্যা *)স্তূপ;
  stackp = (অক্ষর *)(regp + 8);
  regused = 0;

  যদি (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_মান_ptr = ecif->rvalue;
  অন্যথা
    struct_মান_ptr = ফাঁকা;

  p_argv = ecif->avalue;

  জন্য (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      unsigned short t, a;

      z = (*p_arg)->আকার_2;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument can be passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      যদি (t == FFI_TYPE_STRUCT)
	{
	  যদি (z == আকার (পূর্ণসংখ্যা) && a == আকার (পূর্ণসংখ্যা) && regused < 8)
	    argp = (অক্ষর *)regp;
	  অন্যথা
	    argp = stackp;
	}
      অন্যথা
	{
	  যদি (z > আকার (পূর্ণসংখ্যা) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      যদি (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  যদি (regused < 8)
	    argp = (অক্ষর *)regp;
	  অন্যথা
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      যদি (argp == stackp && a > আকার (পূর্ণসংখ্যা))
	{
	  stackp = (অক্ষর *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      নির্বাচন (t)
	{
	ক্ষেত্র FFI_TYPE_SINT8:
	  *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা) *(SINT8 *) *p_argv;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT8:
	  *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা) *(UINT8 *) *p_argv;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_SINT16:
	  *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা) *(SINT16 *) *p_argv;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT16:
	  *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা) *(UINT16 *) *p_argv;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_FLOAT:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_POINTER:
	  *(unsigned পূর্ণসংখ্যা *) argp = *(unsigned পূর্ণসংখ্যা *) *p_argv;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_DOUBLE:
	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_SINT64:
	ক্ষেত্র FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  বিরতি;

	পূর্বনির্ধারিত:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      যদি ((আকার (পূর্ণসংখ্যা) - 1) & z)
	z = FFI_ALIGN(z, আকার (পূর্ণসংখ্যা));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      যদি (argp == (অক্ষর *)regp && regused < 8)
	{
	  regp += z / আকার (পূর্ণসংখ্যা);
	  regused += z / আকার (পূর্ণসংখ্যা);
	}
      অন্যথা
	stackp += z;
    }

  ফেরত struct_মান_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  নির্বাচন (cif->rtype->type)
    {
    ক্ষেত্র FFI_TYPE_VOID:
      cif->flags = 0;
      বিরতি;

    ক্ষেত্র FFI_TYPE_STRUCT:
      যদি (cif->rtype->আকার_2 == আকার (পূর্ণসংখ্যা) &&
	  cif->rtype->alignment == আকার (পূর্ণসংখ্যা))
	cif->flags = CIF_FLAGS_INT;
      অন্যথা
	cif->flags = 0;
      বিরতি;

    ক্ষেত্র FFI_TYPE_DOUBLE:
    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      বিরতি;

    পূর্বনির্ধারিত:
      cif->flags = CIF_FLAGS_INT;
      বিরতি;
    }

  ফেরত FFI_OK;
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  যদি (rvalue == ফাঁকা
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->আকার_2 != আকার (পূর্ণসংখ্যা)
	  || cif->rtype->alignment != আকার (পূর্ণসংখ্যা)))
    ecif.rvalue = alloca (cif->rtype->আকার_2);
  অন্যথা
    ecif.rvalue = rvalue;

  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      বিরতি;

    পূর্বনির্ধারিত:
      FFI_ASSERT (0);
      বিরতি;
    }
}

/*
 * Closure API
 */

স্থির শূন্য
ffi_prep_closure_args_OBSD (ffi_cif *cif, শূন্য **avalue, unsigned পূর্ণসংখ্যা *regp,
			    অক্ষর *stackp)
{
  unsigned পূর্ণসংখ্যা i;
  শূন্য **p_argv;
  অক্ষর *argp;
  unsigned পূর্ণসংখ্যা regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  জন্য (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      unsigned short t, a;

      z = (*p_arg)->আকার_2;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument has been passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      যদি (t == FFI_TYPE_STRUCT)
	{
	  যদি (z == আকার (পূর্ণসংখ্যা) && a == আকার (পূর্ণসংখ্যা) && regused < 8)
	    argp = (অক্ষর *)regp;
	  অন্যথা
	    argp = stackp;
	}
      অন্যথা
	{
	  যদি (z > আকার (পূর্ণসংখ্যা) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      যদি (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  যদি (regused < 8)
	    argp = (অক্ষর *)regp;
	  অন্যথা
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      যদি (argp == stackp && a > আকার (পূর্ণসংখ্যা))
	{
	  stackp = (অক্ষর *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      যদি (z < আকার (পূর্ণসংখ্যা) && t != FFI_TYPE_STRUCT)
	*p_argv = (শূন্য *) (argp + আকার (পূর্ণসংখ্যা) - z);
      অন্যথা
	*p_argv = (শূন্য *) argp;

      /* Align if necessary */
      যদি ((আকার (পূর্ণসংখ্যা) - 1) & z)
	z = FFI_ALIGN(z, আকার (পূর্ণসংখ্যা));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      যদি (argp == (অক্ষর *)regp && regused < 8)
	{
	  regp += z / আকার (পূর্ণসংখ্যা);
	  regused += z / আকার (পূর্ণসংখ্যা);
	}
      অন্যথা
	stackp += z;
    }
}

unsigned পূর্ণসংখ্যা
ffi_closure_OBSD_inner (ffi_closure *closure, শূন্য *resp, unsigned পূর্ণসংখ্যা *regp,
			অক্ষর *stackp)
{
  ffi_cif *cif;
  শূন্য **arg_area;

  cif = closure->cif;
  arg_area = (শূন্য**) alloca (cif->nargs * আকার (শূন্য *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_তথ্য);

  ফেরত cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
		      শূন্য *user_তথ্য, শূন্য *codeloc)
{
  unsigned পূর্ণসংখ্যা *tramp = (unsigned পূর্ণসংখ্যা *) codeloc;
  শূন্য *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  যদি (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  অন্যথা
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned পূর্ণসংখ্যা)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned পূর্ণসংখ্যা)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned পূর্ণসংখ্যা)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned পূর্ণসংখ্যা)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned পূর্ণসংখ্যা)codeloc, FFI_TRAMPOLINE_আকার);

  closure->cif  = cif;
  closure->user_তথ্য = user_তথ্য;
  closure->fun  = fun;

  ফেরত FFI_OK;
}
