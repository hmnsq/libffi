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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <unistd.h>

શૂન્ય ffi_call_OBSD (unsigned પૂર્ણાંક, extended_cif *, unsigned પૂર્ણાંક, શૂન્ય *,
		    શૂન્ય (*fn) ());
શૂન્ય *ffi_prep_args (શૂન્ય *, extended_cif *);
શૂન્ય ffi_closure_OBSD (ffi_closure *);
શૂન્ય ffi_closure_struct_OBSD (ffi_closure *);
unsigned પૂર્ણાંક ffi_closure_OBSD_inner (ffi_closure *, શૂન્ય *, unsigned પૂર્ણાંક *,
				     અક્ષર *);
શૂન્ય ffi_cacheflush_OBSD (unsigned પૂર્ણાંક, unsigned પૂર્ણાંક);

#define CIF_FLAGS_INT		(1 << 0)
#define CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

શૂન્ય *
ffi_prep_args (શૂન્ય *ઢગલો, extended_cif *ecif)
{
  unsigned પૂર્ણાંક i;
  શૂન્ય **p_argv;
  અક્ષર *argp, *stackp;
  unsigned પૂર્ણાંક *regp;
  unsigned પૂર્ણાંક regused;
  ffi_type **p_arg;
  શૂન્ય *struct_કિંમત_ptr;

  regp = (unsigned પૂર્ણાંક *)ઢગલો;
  stackp = (અક્ષર *)(regp + 8);
  regused = 0;

  જો (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_કિંમત_ptr = ecif->rvalue;
  નહીંતર
    struct_કિંમત_ptr = ખાલી;

  p_argv = ecif->avalue;

  માટે (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
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
      જો (t == FFI_TYPE_STRUCT)
	{
	  જો (z == કદ (પૂર્ણાંક) && a == કદ (પૂર્ણાંક) && regused < 8)
	    argp = (અક્ષર *)regp;
	  નહીંતર
	    argp = stackp;
	}
      નહીંતર
	{
	  જો (z > કદ (પૂર્ણાંક) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      જો (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  જો (regused < 8)
	    argp = (અક્ષર *)regp;
	  નહીંતર
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      જો (argp == stackp && a > કદ (પૂર્ણાંક))
	{
	  stackp = (અક્ષર *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      પસંદગી (t)
	{
	સ્થિતિ FFI_TYPE_SINT8:
	  *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક) *(SINT8 *) *p_argv;
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT8:
	  *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક) *(UINT8 *) *p_argv;
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT16:
	  *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક) *(SINT16 *) *p_argv;
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT16:
	  *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક) *(UINT16 *) *p_argv;
	  તોડો;

	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_POINTER:
	  *(unsigned પૂર્ણાંક *) argp = *(unsigned પૂર્ણાંક *) *p_argv;
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  તોડો;

	મૂળભૂત:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      જો ((કદ (પૂર્ણાંક) - 1) & z)
	z = FFI_ALIGN(z, કદ (પૂર્ણાંક));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      જો (argp == (અક્ષર *)regp && regused < 8)
	{
	  regp += z / કદ (પૂર્ણાંક);
	  regused += z / કદ (પૂર્ણાંક);
	}
      નહીંતર
	stackp += z;
    }

  પાછા struct_કિંમત_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      cif->flags = 0;
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
      જો (cif->rtype->size == કદ (પૂર્ણાંક) &&
	  cif->rtype->alignment == કદ (પૂર્ણાંક))
	cif->flags = CIF_FLAGS_INT;
      નહીંતર
	cif->flags = 0;
      તોડો;

    સ્થિતિ FFI_TYPE_DOUBLE:
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      તોડો;

    મૂળભૂત:
      cif->flags = CIF_FLAGS_INT;
      તોડો;
    }

  પાછા FFI_OK;
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn) (), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  જો (rvalue == ખાલી
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->size != કદ (પૂર્ણાંક)
	  || cif->rtype->alignment != કદ (પૂર્ણાંક)))
    ecif.rvalue = alloca (cif->rtype->size);
  નહીંતર
    ecif.rvalue = rvalue;

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      તોડો;

    મૂળભૂત:
      FFI_ASSERT (0);
      તોડો;
    }
}

/*
 * Closure API
 */

સ્થિર શૂન્ય
ffi_prep_closure_args_OBSD (ffi_cif *cif, શૂન્ય **avalue, unsigned પૂર્ણાંક *regp,
			    અક્ષર *stackp)
{
  unsigned પૂર્ણાંક i;
  શૂન્ય **p_argv;
  અક્ષર *argp;
  unsigned પૂર્ણાંક regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  માટે (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
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
      જો (t == FFI_TYPE_STRUCT)
	{
	  જો (z == કદ (પૂર્ણાંક) && a == કદ (પૂર્ણાંક) && regused < 8)
	    argp = (અક્ષર *)regp;
	  નહીંતર
	    argp = stackp;
	}
      નહીંતર
	{
	  જો (z > કદ (પૂર્ણાંક) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      જો (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  જો (regused < 8)
	    argp = (અક્ષર *)regp;
	  નહીંતર
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      જો (argp == stackp && a > કદ (પૂર્ણાંક))
	{
	  stackp = (અક્ષર *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      જો (z < કદ (પૂર્ણાંક) && t != FFI_TYPE_STRUCT)
	*p_argv = (શૂન્ય *) (argp + કદ (પૂર્ણાંક) - z);
      નહીંતર
	*p_argv = (શૂન્ય *) argp;

      /* Align if necessary */
      જો ((કદ (પૂર્ણાંક) - 1) & z)
	z = FFI_ALIGN(z, કદ (પૂર્ણાંક));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      જો (argp == (અક્ષર *)regp && regused < 8)
	{
	  regp += z / કદ (પૂર્ણાંક);
	  regused += z / કદ (પૂર્ણાંક);
	}
      નહીંતર
	stackp += z;
    }
}

unsigned પૂર્ણાંક
ffi_closure_OBSD_inner (ffi_closure *closure, શૂન્ય *resp, unsigned પૂર્ણાંક *regp,
			અક્ષર *stackp)
{
  ffi_cif *cif;
  શૂન્ય **arg_area;

  cif = closure->cif;
  arg_area = (શૂન્ય**) alloca (cif->nargs * કદ (શૂન્ય *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  પાછા cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
		      શૂન્ય *user_data, શૂન્ય *codeloc)
{
  unsigned પૂર્ણાંક *tramp = (unsigned પૂર્ણાંક *) codeloc;
  શૂન્ય *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  જો (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  નહીંતર
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned પૂર્ણાંક)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned પૂર્ણાંક)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned પૂર્ણાંક)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned પૂર્ણાંક)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned પૂર્ણાંક)codeloc, FFI_TRAMPOLINE_SIZE);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  પાછા FFI_OK;
}
