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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <unistd.h>

শূন্য ffi_call_OBSD (unsigned পূর্ণাঙ্ক, extended_cif *, unsigned পূর্ণাঙ্ক, শূন্য *,
		    শূন্য (*fn) ());
শূন্য *ffi_prep_args (শূন্য *, extended_cif *);
শূন্য ffi_closure_OBSD (ffi_closure *);
শূন্য ffi_closure_struct_OBSD (ffi_closure *);
unsigned পূর্ণাঙ্ক ffi_closure_OBSD_inner (ffi_closure *, শূন্য *, unsigned পূর্ণাঙ্ক *,
				     ৱর্ণ *);
শূন্য ffi_cacheflush_OBSD (unsigned পূর্ণাঙ্ক, unsigned পূর্ণাঙ্ক);

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
  unsigned পূর্ণাঙ্ক i;
  শূন্য **p_argv;
  ৱর্ণ *argp, *stackp;
  unsigned পূর্ণাঙ্ক *regp;
  unsigned পূর্ণাঙ্ক regused;
  ffi_type **p_arg;
  শূন্য *struct_মান_ptr;

  regp = (unsigned পূর্ণাঙ্ক *)স্তূপ;
  stackp = (ৱর্ণ *)(regp + 8);
  regused = 0;

  যদি (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_মান_ptr = ecif->rvalue;
  অন্যথা
    struct_মান_ptr = রিক্ত;

  p_argv = ecif->avalue;

  বাবে (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
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
      যদি (t == FFI_TYPE_STRUCT)
	{
	  যদি (z == পরিমাণ (পূর্ণাঙ্ক) && a == পরিমাণ (পূর্ণাঙ্ক) && regused < 8)
	    argp = (ৱর্ণ *)regp;
	  অন্যথা
	    argp = stackp;
	}
      অন্যথা
	{
	  যদি (z > পরিমাণ (পূর্ণাঙ্ক) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      যদি (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  যদি (regused < 8)
	    argp = (ৱর্ণ *)regp;
	  অন্যথা
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      যদি (argp == stackp && a > পরিমাণ (পূর্ণাঙ্ক))
	{
	  stackp = (ৱর্ণ *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ৱিকল্প (t)
	{
	পক্ষ FFI_TYPE_SINT8:
	  *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক) *(SINT8 *) *p_argv;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT8:
	  *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক) *(UINT8 *) *p_argv;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT16:
	  *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক) *(SINT16 *) *p_argv;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT16:
	  *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক) *(UINT16 *) *p_argv;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_INT:
	পক্ষ FFI_TYPE_FLOAT:
	পক্ষ FFI_TYPE_UINT32:
	পক্ষ FFI_TYPE_SINT32:
	পক্ষ FFI_TYPE_POINTER:
	  *(unsigned পূর্ণাঙ্ক *) argp = *(unsigned পূর্ণাঙ্ক *) *p_argv;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_DOUBLE:
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  ভঙ্গ;

	স্ৱভাৱ:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      যদি ((পরিমাণ (পূর্ণাঙ্ক) - 1) & z)
	z = FFI_ALIGN(z, পরিমাণ (পূর্ণাঙ্ক));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      যদি (argp == (ৱর্ণ *)regp && regused < 8)
	{
	  regp += z / পরিমাণ (পূর্ণাঙ্ক);
	  regused += z / পরিমাণ (পূর্ণাঙ্ক);
	}
      অন্যথা
	stackp += z;
    }

  ঘূৰাই struct_মান_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ৱিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_VOID:
      cif->flags = 0;
      ভঙ্গ;

    পক্ষ FFI_TYPE_STRUCT:
      যদি (cif->rtype->size == পরিমাণ (পূর্ণাঙ্ক) &&
	  cif->rtype->alignment == পরিমাণ (পূর্ণাঙ্ক))
	cif->flags = CIF_FLAGS_INT;
      অন্যথা
	cif->flags = 0;
      ভঙ্গ;

    পক্ষ FFI_TYPE_DOUBLE:
    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ভঙ্গ;

    স্ৱভাৱ:
      cif->flags = CIF_FLAGS_INT;
      ভঙ্গ;
    }

  ঘূৰাই FFI_OK;
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  যদি (rvalue == রিক্ত
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->size != পরিমাণ (পূর্ণাঙ্ক)
	  || cif->rtype->alignment != পরিমাণ (পূর্ণাঙ্ক)))
    ecif.rvalue = alloca (cif->rtype->size);
  অন্যথা
    ecif.rvalue = rvalue;

  ৱিকল্প (cif->abi)
    {
    পক্ষ FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      ভঙ্গ;

    স্ৱভাৱ:
      FFI_ASSERT (0);
      ভঙ্গ;
    }
}

/*
 * Closure API
 */

স্থির শূন্য
ffi_prep_closure_args_OBSD (ffi_cif *cif, শূন্য **avalue, unsigned পূর্ণাঙ্ক *regp,
			    ৱর্ণ *stackp)
{
  unsigned পূর্ণাঙ্ক i;
  শূন্য **p_argv;
  ৱর্ণ *argp;
  unsigned পূর্ণাঙ্ক regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  বাবে (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
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
      যদি (t == FFI_TYPE_STRUCT)
	{
	  যদি (z == পরিমাণ (পূর্ণাঙ্ক) && a == পরিমাণ (পূর্ণাঙ্ক) && regused < 8)
	    argp = (ৱর্ণ *)regp;
	  অন্যথা
	    argp = stackp;
	}
      অন্যথা
	{
	  যদি (z > পরিমাণ (পূর্ণাঙ্ক) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      যদি (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  যদি (regused < 8)
	    argp = (ৱর্ণ *)regp;
	  অন্যথা
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      যদি (argp == stackp && a > পরিমাণ (পূর্ণাঙ্ক))
	{
	  stackp = (ৱর্ণ *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      যদি (z < পরিমাণ (পূর্ণাঙ্ক) && t != FFI_TYPE_STRUCT)
	*p_argv = (শূন্য *) (argp + পরিমাণ (পূর্ণাঙ্ক) - z);
      অন্যথা
	*p_argv = (শূন্য *) argp;

      /* Align if necessary */
      যদি ((পরিমাণ (পূর্ণাঙ্ক) - 1) & z)
	z = FFI_ALIGN(z, পরিমাণ (পূর্ণাঙ্ক));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      যদি (argp == (ৱর্ণ *)regp && regused < 8)
	{
	  regp += z / পরিমাণ (পূর্ণাঙ্ক);
	  regused += z / পরিমাণ (পূর্ণাঙ্ক);
	}
      অন্যথা
	stackp += z;
    }
}

unsigned পূর্ণাঙ্ক
ffi_closure_OBSD_inner (ffi_closure *closure, শূন্য *resp, unsigned পূর্ণাঙ্ক *regp,
			ৱর্ণ *stackp)
{
  ffi_cif *cif;
  শূন্য **arg_area;

  cif = closure->cif;
  arg_area = (শূন্য**) alloca (cif->nargs * পরিমাণ (শূন্য *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  ঘূৰাই cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
		      শূন্য *user_data, শূন্য *codeloc)
{
  unsigned পূর্ণাঙ্ক *tramp = (unsigned পূর্ণাঙ্ক *) codeloc;
  শূন্য *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  যদি (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  অন্যথা
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned পূর্ণাঙ্ক)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned পূর্ণাঙ্ক)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned পূর্ণাঙ্ক)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned পূর্ণাঙ্ক)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned পূর্ণাঙ্ক)codeloc, FFI_TRAMPOLINE_SIZE);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ঘূৰাই FFI_OK;
}
