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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <unistd.h>

ശൂന്യം ffi_call_OBSD (unsigned പൂർണ്ണസംഖ്യ, extended_cif *, unsigned പൂർണ്ണസംഖ്യ, ശൂന്യം *,
		    ശൂന്യം (*fn) ());
ശൂന്യം *ffi_prep_args (ശൂന്യം *, extended_cif *);
ശൂന്യം ffi_closure_OBSD (ffi_closure *);
ശൂന്യം ffi_closure_struct_OBSD (ffi_closure *);
unsigned പൂർണ്ണസംഖ്യ ffi_closure_OBSD_inner (ffi_closure *, ശൂന്യം *, unsigned പൂർണ്ണസംഖ്യ *,
				     അക്ഷരം *);
ശൂന്യം ffi_cacheflush_OBSD (unsigned പൂർണ്ണസംഖ്യ, unsigned പൂർണ്ണസംഖ്യ);

#define CIF_FLAGS_INT		(1 << 0)
#define CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

ശൂന്യം *
ffi_prep_args (ശൂന്യം *അട്ടി, extended_cif *ecif)
{
  unsigned പൂർണ്ണസംഖ്യ i;
  ശൂന്യം **p_argv;
  അക്ഷരം *argp, *stackp;
  unsigned പൂർണ്ണസംഖ്യ *regp;
  unsigned പൂർണ്ണസംഖ്യ regused;
  ffi_type **p_arg;
  ശൂന്യം *struct_മൂല്യം_ptr;

  regp = (unsigned പൂർണ്ണസംഖ്യ *)അട്ടി;
  stackp = (അക്ഷരം *)(regp + 8);
  regused = 0;

  എങ്കിൽ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_മൂല്യം_ptr = ecif->rvalue;
  അല്ലെങ്കിൽ
    struct_മൂല്യം_ptr = ശൂന്യ;

  p_argv = ecif->avalue;

  വേണ്ടി (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
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
      എങ്കിൽ (t == FFI_TYPE_STRUCT)
	{
	  എങ്കിൽ (z == വലുപ്പം (പൂർണ്ണസംഖ്യ) && a == വലുപ്പം (പൂർണ്ണസംഖ്യ) && regused < 8)
	    argp = (അക്ഷരം *)regp;
	  അല്ലെങ്കിൽ
	    argp = stackp;
	}
      അല്ലെങ്കിൽ
	{
	  എങ്കിൽ (z > വലുപ്പം (പൂർണ്ണസംഖ്യ) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      എങ്കിൽ (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  എങ്കിൽ (regused < 8)
	    argp = (അക്ഷരം *)regp;
	  അല്ലെങ്കിൽ
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      എങ്കിൽ (argp == stackp && a > വലുപ്പം (പൂർണ്ണസംഖ്യ))
	{
	  stackp = (അക്ഷരം *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      തിരഞ്ഞെടുപ്പ് (t)
	{
	അവസ്ഥ FFI_TYPE_SINT8:
	  *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ) *(SINT8 *) *p_argv;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT8:
	  *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ) *(UINT8 *) *p_argv;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT16:
	  *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ) *(SINT16 *) *p_argv;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT16:
	  *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ) *(UINT16 *) *p_argv;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_FLOAT:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_POINTER:
	  *(unsigned പൂർണ്ണസംഖ്യ *) argp = *(unsigned പൂർണ്ണസംഖ്യ *) *p_argv;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      എങ്കിൽ ((വലുപ്പം (പൂർണ്ണസംഖ്യ) - 1) & z)
	z = FFI_ALIGN(z, വലുപ്പം (പൂർണ്ണസംഖ്യ));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      എങ്കിൽ (argp == (അക്ഷരം *)regp && regused < 8)
	{
	  regp += z / വലുപ്പം (പൂർണ്ണസംഖ്യ);
	  regused += z / വലുപ്പം (പൂർണ്ണസംഖ്യ);
	}
      അല്ലെങ്കിൽ
	stackp += z;
    }

  തിരികെ struct_മൂല്യം_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      cif->flags = 0;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
      എങ്കിൽ (cif->rtype->size == വലുപ്പം (പൂർണ്ണസംഖ്യ) &&
	  cif->rtype->alignment == വലുപ്പം (പൂർണ്ണസംഖ്യ))
	cif->flags = CIF_FLAGS_INT;
      അല്ലെങ്കിൽ
	cif->flags = 0;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_DOUBLE:
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      cif->flags = CIF_FLAGS_INT;
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn) (), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  എങ്കിൽ (rvalue == ശൂന്യ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->size != വലുപ്പം (പൂർണ്ണസംഖ്യ)
	  || cif->rtype->alignment != വലുപ്പം (പൂർണ്ണസംഖ്യ)))
    ecif.rvalue = alloca (cif->rtype->size);
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT (0);
      നിർത്തുക;
    }
}

/*
 * Closure API
 */

സ്ഥിതം ശൂന്യം
ffi_prep_closure_args_OBSD (ffi_cif *cif, ശൂന്യം **avalue, unsigned പൂർണ്ണസംഖ്യ *regp,
			    അക്ഷരം *stackp)
{
  unsigned പൂർണ്ണസംഖ്യ i;
  ശൂന്യം **p_argv;
  അക്ഷരം *argp;
  unsigned പൂർണ്ണസംഖ്യ regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  വേണ്ടി (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
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
      എങ്കിൽ (t == FFI_TYPE_STRUCT)
	{
	  എങ്കിൽ (z == വലുപ്പം (പൂർണ്ണസംഖ്യ) && a == വലുപ്പം (പൂർണ്ണസംഖ്യ) && regused < 8)
	    argp = (അക്ഷരം *)regp;
	  അല്ലെങ്കിൽ
	    argp = stackp;
	}
      അല്ലെങ്കിൽ
	{
	  എങ്കിൽ (z > വലുപ്പം (പൂർണ്ണസംഖ്യ) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      എങ്കിൽ (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  എങ്കിൽ (regused < 8)
	    argp = (അക്ഷരം *)regp;
	  അല്ലെങ്കിൽ
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      എങ്കിൽ (argp == stackp && a > വലുപ്പം (പൂർണ്ണസംഖ്യ))
	{
	  stackp = (അക്ഷരം *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      എങ്കിൽ (z < വലുപ്പം (പൂർണ്ണസംഖ്യ) && t != FFI_TYPE_STRUCT)
	*p_argv = (ശൂന്യം *) (argp + വലുപ്പം (പൂർണ്ണസംഖ്യ) - z);
      അല്ലെങ്കിൽ
	*p_argv = (ശൂന്യം *) argp;

      /* Align if necessary */
      എങ്കിൽ ((വലുപ്പം (പൂർണ്ണസംഖ്യ) - 1) & z)
	z = FFI_ALIGN(z, വലുപ്പം (പൂർണ്ണസംഖ്യ));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      എങ്കിൽ (argp == (അക്ഷരം *)regp && regused < 8)
	{
	  regp += z / വലുപ്പം (പൂർണ്ണസംഖ്യ);
	  regused += z / വലുപ്പം (പൂർണ്ണസംഖ്യ);
	}
      അല്ലെങ്കിൽ
	stackp += z;
    }
}

unsigned പൂർണ്ണസംഖ്യ
ffi_closure_OBSD_inner (ffi_closure *closure, ശൂന്യം *resp, unsigned പൂർണ്ണസംഖ്യ *regp,
			അക്ഷരം *stackp)
{
  ffi_cif *cif;
  ശൂന്യം **arg_area;

  cif = closure->cif;
  arg_area = (ശൂന്യം**) alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  തിരികെ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
		      ശൂന്യം *user_data, ശൂന്യം *codeloc)
{
  unsigned പൂർണ്ണസംഖ്യ *tramp = (unsigned പൂർണ്ണസംഖ്യ *) codeloc;
  ശൂന്യം *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  അല്ലെങ്കിൽ
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned പൂർണ്ണസംഖ്യ)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned പൂർണ്ണസംഖ്യ)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned പൂർണ്ണസംഖ്യ)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned പൂർണ്ണസംഖ്യ)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned പൂർണ്ണസംഖ്യ)codeloc, FFI_TRAMPOLINE_SIZE);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  തിരികെ FFI_OK;
}
