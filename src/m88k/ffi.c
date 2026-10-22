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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>
#समावेश <unistd.h>

व्योम ffi_call_OBSD (अचिन्हित पूर्णांक, extended_cif *, अचिन्हित पूर्णांक, व्योम *,
		    व्योम (*fn) ());
व्योम *ffi_prep_args (व्योम *, extended_cif *);
व्योम ffi_closure_OBSD (ffi_closure *);
व्योम ffi_closure_struct_OBSD (ffi_closure *);
अचिन्हित पूर्णांक ffi_closure_OBSD_inner (ffi_closure *, व्योम *, अचिन्हित पूर्णांक *,
				     अक्षर *);
व्योम ffi_cacheflush_OBSD (अचिन्हित पूर्णांक, अचिन्हित पूर्णांक);

#घोषणा CIF_FLAGS_INT		(1 << 0)
#घोषणा CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

व्योम *
ffi_prep_args (व्योम *ढेर, extended_cif *ecif)
{
  अचिन्हित पूर्णांक i;
  व्योम **p_argv;
  अक्षर *argp, *stackp;
  अचिन्हित पूर्णांक *regp;
  अचिन्हित पूर्णांक regused;
  ffi_type **p_arg;
  व्योम *struct_मूल्य_ptr;

  regp = (अचिन्हित पूर्णांक *)ढेर;
  stackp = (अक्षर *)(regp + 8);
  regused = 0;

  अगर (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_मूल्य_ptr = ecif->rvalue;
  अन्यथा
    struct_मूल्य_ptr = शून्य;

  p_argv = ecif->avalue;

  क्रम (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
    {
      माप_प्रकार z;
      अचिन्हित लघु t, a;

      z = (*p_arg)->आकार;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument can be passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      अगर (t == FFI_TYPE_STRUCT)
	{
	  अगर (z == माप (पूर्णांक) && a == माप (पूर्णांक) && regused < 8)
	    argp = (अक्षर *)regp;
	  अन्यथा
	    argp = stackp;
	}
      अन्यथा
	{
	  अगर (z > माप (पूर्णांक) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      अगर (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  अगर (regused < 8)
	    argp = (अक्षर *)regp;
	  अन्यथा
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      अगर (argp == stackp && a > माप (पूर्णांक))
	{
	  stackp = (अक्षर *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      चयन (t)
	{
	हाल FFI_TYPE_SINT8:
	  *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक) *(SINT8 *) *p_argv;
	  अवरोध;

	हाल FFI_TYPE_UINT8:
	  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक) *(UINT8 *) *p_argv;
	  अवरोध;

	हाल FFI_TYPE_SINT16:
	  *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक) *(SINT16 *) *p_argv;
	  अवरोध;

	हाल FFI_TYPE_UINT16:
	  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक) *(UINT16 *) *p_argv;
	  अवरोध;

	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_FLOAT:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_POINTER:
	  *(अचिन्हित पूर्णांक *) argp = *(अचिन्हित पूर्णांक *) *p_argv;
	  अवरोध;

	हाल FFI_TYPE_DOUBLE:
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_STRUCT:
	  स_नकल (argp, *p_argv, z);
	  अवरोध;

	शेष:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      अगर ((माप (पूर्णांक) - 1) & z)
	z = FFI_ALIGN(z, माप (पूर्णांक));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      अगर (argp == (अक्षर *)regp && regused < 8)
	{
	  regp += z / माप (पूर्णांक);
	  regused += z / माप (पूर्णांक);
	}
      अन्यथा
	stackp += z;
    }

  वापस struct_मूल्य_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_VOID:
      cif->flags = 0;
      अवरोध;

    हाल FFI_TYPE_STRUCT:
      अगर (cif->rtype->आकार == माप (पूर्णांक) &&
	  cif->rtype->alignment == माप (पूर्णांक))
	cif->flags = CIF_FLAGS_INT;
      अन्यथा
	cif->flags = 0;
      अवरोध;

    हाल FFI_TYPE_DOUBLE:
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      अवरोध;

    शेष:
      cif->flags = CIF_FLAGS_INT;
      अवरोध;
    }

  वापस FFI_OK;
}

व्योम
ffi_call (ffi_cif *cif, व्योम (*fn) (), व्योम *rvalue, व्योम **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  अगर (rvalue == शून्य
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->आकार != माप (पूर्णांक)
	  || cif->rtype->alignment != माप (पूर्णांक)))
    ecif.rvalue = alloca (cif->rtype->आकार);
  अन्यथा
    ecif.rvalue = rvalue;

  चयन (cif->abi)
    {
    हाल FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      अवरोध;

    शेष:
      FFI_ASSERT (0);
      अवरोध;
    }
}

/*
 * Closure API
 */

अटल व्योम
ffi_prep_closure_args_OBSD (ffi_cif *cif, व्योम **avalue, अचिन्हित पूर्णांक *regp,
			    अक्षर *stackp)
{
  अचिन्हित पूर्णांक i;
  व्योम **p_argv;
  अक्षर *argp;
  अचिन्हित पूर्णांक regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  क्रम (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      माप_प्रकार z;
      अचिन्हित लघु t, a;

      z = (*p_arg)->आकार;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument has been passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      अगर (t == FFI_TYPE_STRUCT)
	{
	  अगर (z == माप (पूर्णांक) && a == माप (पूर्णांक) && regused < 8)
	    argp = (अक्षर *)regp;
	  अन्यथा
	    argp = stackp;
	}
      अन्यथा
	{
	  अगर (z > माप (पूर्णांक) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      अगर (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  अगर (regused < 8)
	    argp = (अक्षर *)regp;
	  अन्यथा
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      अगर (argp == stackp && a > माप (पूर्णांक))
	{
	  stackp = (अक्षर *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      अगर (z < माप (पूर्णांक) && t != FFI_TYPE_STRUCT)
	*p_argv = (व्योम *) (argp + माप (पूर्णांक) - z);
      अन्यथा
	*p_argv = (व्योम *) argp;

      /* Align if necessary */
      अगर ((माप (पूर्णांक) - 1) & z)
	z = FFI_ALIGN(z, माप (पूर्णांक));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      अगर (argp == (अक्षर *)regp && regused < 8)
	{
	  regp += z / माप (पूर्णांक);
	  regused += z / माप (पूर्णांक);
	}
      अन्यथा
	stackp += z;
    }
}

अचिन्हित पूर्णांक
ffi_closure_OBSD_inner (ffi_closure *closure, व्योम *resp, अचिन्हित पूर्णांक *regp,
			अक्षर *stackp)
{
  ffi_cif *cif;
  व्योम **arg_area;

  cif = closure->cif;
  arg_area = (व्योम**) alloca (cif->nargs * माप (व्योम *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_तथ्य);

  वापस cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
		      व्योम *user_तथ्य, व्योम *codeloc)
{
  अचिन्हित पूर्णांक *tramp = (अचिन्हित पूर्णांक *) codeloc;
  व्योम *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  अगर (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  अन्यथा
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((अचिन्हित पूर्णांक)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((अचिन्हित पूर्णांक)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((अचिन्हित पूर्णांक)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((अचिन्हित पूर्णांक)closure & 0xffff);

  ffi_cacheflush_OBSD((अचिन्हित पूर्णांक)codeloc, FFI_TRAMPOLINE_आकार);

  closure->cif  = cif;
  closure->user_तथ्य = user_तथ्य;
  closure->fun  = fun;

  वापस FFI_OK;
}
