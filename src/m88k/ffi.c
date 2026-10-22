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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <unistd.h>

வெறுமை ffi_call_OBSD (unsigned முழு, extended_cif *, unsigned முழு, வெறுமை *,
		    வெறுமை (*fn) ());
வெறுமை *ffi_prep_args (வெறுமை *, extended_cif *);
வெறுமை ffi_closure_OBSD (ffi_closure *);
வெறுமை ffi_closure_struct_OBSD (ffi_closure *);
unsigned முழு ffi_closure_OBSD_inner (ffi_closure *, வெறுமை *, unsigned முழு *,
				     எழுத்து *);
வெறுமை ffi_cacheflush_OBSD (unsigned முழு, unsigned முழு);

#define CIF_FLAGS_INT		(1 << 0)
#define CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

வெறுமை *
ffi_prep_args (வெறுமை *அடுக்கு, extended_cif *ecif)
{
  unsigned முழு i;
  வெறுமை **p_argv;
  எழுத்து *argp, *stackp;
  unsigned முழு *regp;
  unsigned முழு regused;
  ffi_type **p_arg;
  வெறுமை *struct_மதிப்பு_ptr;

  regp = (unsigned முழு *)அடுக்கு;
  stackp = (எழுத்து *)(regp + 8);
  regused = 0;

  எனில் (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_மதிப்பு_ptr = ecif->rvalue;
  இல்லையெனில்
    struct_மதிப்பு_ptr = வெற்று;

  p_argv = ecif->avalue;

  க்கு (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
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
      எனில் (t == FFI_TYPE_STRUCT)
	{
	  எனில் (z == அளவு (முழு) && a == அளவு (முழு) && regused < 8)
	    argp = (எழுத்து *)regp;
	  இல்லையெனில்
	    argp = stackp;
	}
      இல்லையெனில்
	{
	  எனில் (z > அளவு (முழு) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      எனில் (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  எனில் (regused < 8)
	    argp = (எழுத்து *)regp;
	  இல்லையெனில்
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      எனில் (argp == stackp && a > அளவு (முழு))
	{
	  stackp = (எழுத்து *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      தேர்வு (t)
	{
	நிலை FFI_TYPE_SINT8:
	  *(signed முழு *) argp = (signed முழு) *(SINT8 *) *p_argv;
	  நிறுத்து;

	நிலை FFI_TYPE_UINT8:
	  *(unsigned முழு *) argp = (unsigned முழு) *(UINT8 *) *p_argv;
	  நிறுத்து;

	நிலை FFI_TYPE_SINT16:
	  *(signed முழு *) argp = (signed முழு) *(SINT16 *) *p_argv;
	  நிறுத்து;

	நிலை FFI_TYPE_UINT16:
	  *(unsigned முழு *) argp = (unsigned முழு) *(UINT16 *) *p_argv;
	  நிறுத்து;

	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_POINTER:
	  *(unsigned முழு *) argp = *(unsigned முழு *) *p_argv;
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  நிறுத்து;

	இயல்பு:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      எனில் ((அளவு (முழு) - 1) & z)
	z = FFI_ALIGN(z, அளவு (முழு));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      எனில் (argp == (எழுத்து *)regp && regused < 8)
	{
	  regp += z / அளவு (முழு);
	  regused += z / அளவு (முழு);
	}
      இல்லையெனில்
	stackp += z;
    }

  திரும்பு struct_மதிப்பு_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      cif->flags = 0;
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
      எனில் (cif->rtype->size == அளவு (முழு) &&
	  cif->rtype->alignment == அளவு (முழு))
	cif->flags = CIF_FLAGS_INT;
      இல்லையெனில்
	cif->flags = 0;
      நிறுத்து;

    நிலை FFI_TYPE_DOUBLE:
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      நிறுத்து;

    இயல்பு:
      cif->flags = CIF_FLAGS_INT;
      நிறுத்து;
    }

  திரும்பு FFI_OK;
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn) (), வெறுமை *rvalue, வெறுமை **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  எனில் (rvalue == வெற்று
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->size != அளவு (முழு)
	  || cif->rtype->alignment != அளவு (முழு)))
    ecif.rvalue = alloca (cif->rtype->size);
  இல்லையெனில்
    ecif.rvalue = rvalue;

  தேர்வு (cif->abi)
    {
    நிலை FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      நிறுத்து;

    இயல்பு:
      FFI_ASSERT (0);
      நிறுத்து;
    }
}

/*
 * Closure API
 */

நிலையான வெறுமை
ffi_prep_closure_args_OBSD (ffi_cif *cif, வெறுமை **avalue, unsigned முழு *regp,
			    எழுத்து *stackp)
{
  unsigned முழு i;
  வெறுமை **p_argv;
  எழுத்து *argp;
  unsigned முழு regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  க்கு (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
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
      எனில் (t == FFI_TYPE_STRUCT)
	{
	  எனில் (z == அளவு (முழு) && a == அளவு (முழு) && regused < 8)
	    argp = (எழுத்து *)regp;
	  இல்லையெனில்
	    argp = stackp;
	}
      இல்லையெனில்
	{
	  எனில் (z > அளவு (முழு) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      எனில் (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  எனில் (regused < 8)
	    argp = (எழுத்து *)regp;
	  இல்லையெனில்
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      எனில் (argp == stackp && a > அளவு (முழு))
	{
	  stackp = (எழுத்து *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      எனில் (z < அளவு (முழு) && t != FFI_TYPE_STRUCT)
	*p_argv = (வெறுமை *) (argp + அளவு (முழு) - z);
      இல்லையெனில்
	*p_argv = (வெறுமை *) argp;

      /* Align if necessary */
      எனில் ((அளவு (முழு) - 1) & z)
	z = FFI_ALIGN(z, அளவு (முழு));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      எனில் (argp == (எழுத்து *)regp && regused < 8)
	{
	  regp += z / அளவு (முழு);
	  regused += z / அளவு (முழு);
	}
      இல்லையெனில்
	stackp += z;
    }
}

unsigned முழு
ffi_closure_OBSD_inner (ffi_closure *closure, வெறுமை *resp, unsigned முழு *regp,
			எழுத்து *stackp)
{
  ffi_cif *cif;
  வெறுமை **arg_area;

  cif = closure->cif;
  arg_area = (வெறுமை**) alloca (cif->nargs * அளவு (வெறுமை *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  திரும்பு cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
		      வெறுமை *user_data, வெறுமை *codeloc)
{
  unsigned முழு *tramp = (unsigned முழு *) codeloc;
  வெறுமை *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  எனில் (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  இல்லையெனில்
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned முழு)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned முழு)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned முழு)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned முழு)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned முழு)codeloc, FFI_TRAMPOLINE_SIZE);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  திரும்பு FFI_OK;
}
