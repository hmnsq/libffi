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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

ଶୂନ୍ୟ ffi_call_OBSD (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା, extended_cif *, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଶୂନ୍ୟ *,
		    ଶୂନ୍ୟ (*fn) ());
ଶୂନ୍ୟ *ffi_prep_args (ଶୂନ୍ୟ *, extended_cif *);
ଶୂନ୍ୟ ffi_closure_OBSD (ffi_closure *);
ଶୂନ୍ୟ ffi_closure_struct_OBSD (ffi_closure *);
unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_closure_OBSD_inner (ffi_closure *, ଶୂନ୍ୟ *, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *,
				     ଅକ୍ଷର *);
ଶୂନ୍ୟ ffi_cacheflush_OBSD (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

#define CIF_FLAGS_INT		(1 << 0)
#define CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

ଶୂନ୍ୟ *
ffi_prep_args (ଶୂନ୍ୟ *ଗଦା, extended_cif *ecif)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଶୂନ୍ୟ **p_argv;
  ଅକ୍ଷର *argp, *stackp;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *regp;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା regused;
  ffi_type **p_arg;
  ଶୂନ୍ୟ *struct_ମୂଲ୍ୟ_ptr;

  regp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)ଗଦା;
  stackp = (ଅକ୍ଷର *)(regp + 8);
  regused = 0;

  ଯଦି (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_ମୂଲ୍ୟ_ptr = ecif->rvalue;
  ଅନ୍ୟଥା
    struct_ମୂଲ୍ୟ_ptr = ଖାଲି;

  p_argv = ecif->avalue;

  ପାଇଁ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
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
      ଯଦି (t == FFI_TYPE_STRUCT)
	{
	  ଯଦି (z == ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) && a == ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) && regused < 8)
	    argp = (ଅକ୍ଷର *)regp;
	  ଅନ୍ୟଥା
	    argp = stackp;
	}
      ଅନ୍ୟଥା
	{
	  ଯଦି (z > ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ଯଦି (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ଯଦି (regused < 8)
	    argp = (ଅକ୍ଷର *)regp;
	  ଅନ୍ୟଥା
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ଯଦି (argp == stackp && a > ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  stackp = (ଅକ୍ଷର *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ବାଛ (t)
	{
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(SINT8 *) *p_argv;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT8:
	  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(UINT8 *) *p_argv;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_SINT16:
	  *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(SINT16 *) *p_argv;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT16:
	  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(UINT16 *) *p_argv;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) *p_argv;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      ଯଦି ((ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) & z)
	z = FFI_ALIGN(z, ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      ଯଦି (argp == (ଅକ୍ଷର *)regp && regused < 8)
	{
	  regp += z / ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  regused += z / ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	}
      ଅନ୍ୟଥା
	stackp += z;
    }

  ଫେରନ୍ତୁ struct_ମୂଲ୍ୟ_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      cif->flags = 0;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
      ଯଦି (cif->rtype->size == ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) &&
	  cif->rtype->alignment == ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	cif->flags = CIF_FLAGS_INT;
      ଅନ୍ୟଥା
	cif->flags = 0;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_DOUBLE:
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      cif->flags = CIF_FLAGS_INT;
      ଭାଙ୍ଗ;
    }

  ଫେରନ୍ତୁ FFI_OK;
}

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ଯଦି (rvalue == ଖାଲି
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->size != ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)
	  || cif->rtype->alignment != ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)))
    ecif.rvalue = alloca (cif->rtype->size);
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT (0);
      ଭାଙ୍ଗ;
    }
}

/*
 * Closure API
 */

ସ୍ଥିର ଶୂନ୍ୟ
ffi_prep_closure_args_OBSD (ffi_cif *cif, ଶୂନ୍ୟ **avalue, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *regp,
			    ଅକ୍ଷର *stackp)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଶୂନ୍ୟ **p_argv;
  ଅକ୍ଷର *argp;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  ପାଇଁ (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
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
      ଯଦି (t == FFI_TYPE_STRUCT)
	{
	  ଯଦି (z == ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) && a == ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) && regused < 8)
	    argp = (ଅକ୍ଷର *)regp;
	  ଅନ୍ୟଥା
	    argp = stackp;
	}
      ଅନ୍ୟଥା
	{
	  ଯଦି (z > ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ଯଦି (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ଯଦି (regused < 8)
	    argp = (ଅକ୍ଷର *)regp;
	  ଅନ୍ୟଥା
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ଯଦି (argp == stackp && a > ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  stackp = (ଅକ୍ଷର *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ଯଦି (z < ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) && t != FFI_TYPE_STRUCT)
	*p_argv = (ଶୂନ୍ୟ *) (argp + ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - z);
      ଅନ୍ୟଥା
	*p_argv = (ଶୂନ୍ୟ *) argp;

      /* Align if necessary */
      ଯଦି ((ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) & z)
	z = FFI_ALIGN(z, ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      ଯଦି (argp == (ଅକ୍ଷର *)regp && regused < 8)
	{
	  regp += z / ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  regused += z / ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	}
      ଅନ୍ୟଥା
	stackp += z;
    }
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_OBSD_inner (ffi_closure *closure, ଶୂନ୍ୟ *resp, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *regp,
			ଅକ୍ଷର *stackp)
{
  ffi_cif *cif;
  ଶୂନ୍ୟ **arg_area;

  cif = closure->cif;
  arg_area = (ଶୂନ୍ୟ**) alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  ଫେରନ୍ତୁ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data, ଶୂନ୍ୟ *codeloc)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tramp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) codeloc;
  ଶୂନ୍ୟ *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  ଅନ୍ୟଥା
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)codeloc, FFI_TRAMPOLINE_SIZE);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ଫେରନ୍ତୁ FFI_OK;
}
