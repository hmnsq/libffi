/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 1998 Cygnus Solutions
           Copyright (c) 2004 Simon Posnjak
	   Copyright (c) 2005 Axis Communications AB
	   Copyright (C) 2007 Free Software Foundation, Inc.

   CRIS Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL SIMON POSNJAK BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#define تيندنن_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

ساكن ffi_status
initialize_aggregate_packed_struct (ffi_type * arg)
{
  ffi_type **ptr;

  FFI_ASSERT (arg != عدم);

  FFI_ASSERT (arg->elements != عدم);
  FFI_ASSERT (arg->size == 0);
  FFI_ASSERT (arg->alignment == 0);

  ptr = &(arg->elements[0]);

  طالما ((*ptr) != عدم)
    {
      إذا (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	أرجع FFI_BAD_TYPEDEF;

      FFI_ASSERT (ffi_type_test ((*ptr)));

      arg->size += (*ptr)->size;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  إذا (arg->size == 0)
    أرجع FFI_BAD_TYPEDEF;
  وإلا
    أرجع FFI_OK;
}

صحيح
ffi_prep_args (حرف *تيندنن, extended_cif * ecif)
{
  unsigned صحيح i;
  unsigned صحيح struct_کيراءن = 0;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  argp = تيندنن;

  p_argv = ecif->avalue;

  لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0); i--, p_arg++)
    {
      size_t z;

      بدل ((*p_arg)->type)
	{
	حالة FFI_TYPE_STRUCT:
	  {
	    z = (*p_arg)->size;
	    إذا (z <= 4)
	      {
		memcpy (argp, *p_argv, z);
		z = 4;
	      }
	    وإلا إذا (z <= 8)
	      {
		memcpy (argp, *p_argv, z);
		z = 8;
	      }
	    وإلا
	      {
		unsigned صحيح ui_Loc_On_تيندنن;
		z = حجم (فراغ *);
		ui_Loc_On_تيندنن = 4 * ecif->cif->nargs + struct_کيراءن;
		struct_کيراءن = struct_کيراءن + (*p_arg)->size;
		*(unsigned صحيح *) argp =
		  (unsigned صحيح) (UINT32 *) (تيندنن + ui_Loc_On_تيندنن);
		memcpy ((تيندنن + ui_Loc_On_تيندنن), *p_argv, (*p_arg)->size);
	      }
	    اكسر;
	  }
	افتراضي:
	  z = (*p_arg)->size;
	  إذا (z < حجم (صحيح))
	    {
	      بدل ((*p_arg)->type)
		{
		حالة FFI_TYPE_SINT8:
		  *(signed صحيح *) argp = (signed صحيح) *(SINT8 *) (*p_argv);
		  اكسر;

		حالة FFI_TYPE_UINT8:
		  *(unsigned صحيح *) argp =
		    (unsigned صحيح) *(UINT8 *) (*p_argv);
		  اكسر;

		حالة FFI_TYPE_SINT16:
		  *(signed صحيح *) argp = (signed صحيح) *(SINT16 *) (*p_argv);
		  اكسر;

		حالة FFI_TYPE_UINT16:
		  *(unsigned صحيح *) argp =
		    (unsigned صحيح) *(UINT16 *) (*p_argv);
		  اكسر;

		افتراضي:
		  FFI_ASSERT (0);
		}
	      z = حجم (صحيح);
	    }
	  وإلا إذا (z == حجم (صحيح))
	    *(unsigned صحيح *) argp = (unsigned صحيح) *(UINT32 *) (*p_argv);
	  وإلا
	    memcpy (argp, *p_argv, z);
	  اكسر;
	}
      p_argv++;
      argp += z;
    }

  أرجع (struct_کيراءن);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_core (ffi_cif * cif,
	           ffi_abi abi, unsigned صحيح isvariadic,
		   unsigned صحيح nfixedargs, unsigned صحيح ntotalargs,
	           ffi_type * rtype, ffi_type ** atypes)
{
  unsigned bytes = 0;
  unsigned صحيح i;
  ffi_type **ptr;

  FFI_ASSERT (cif != عدم);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);
  FFI_ASSERT (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI);

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

  cif->flags = 0;

  إذا ((cif->rtype->size == 0)
      && (initialize_aggregate_packed_struct (cif->rtype) != FFI_OK))
    أرجع FFI_BAD_TYPEDEF;

  FFI_ASSERT_VALID_TYPE (cif->rtype);

  لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      إذا (((*ptr)->size == 0)
	  && (initialize_aggregate_packed_struct ((*ptr)) != FFI_OK))
	أرجع FFI_BAD_TYPEDEF;

      FFI_ASSERT_VALID_TYPE (*ptr);

      إذا (((*ptr)->alignment - 1) & bytes)
	bytes = FFI_ALIGN (bytes, (*ptr)->alignment);
      إذا ((*ptr)->type == FFI_TYPE_STRUCT)
	{
	  إذا ((*ptr)->size > 8)
	    {
	      bytes += (*ptr)->size;
	      bytes += حجم (فراغ *);
	    }
	  وإلا
	    {
	      إذا ((*ptr)->size > 4)
		bytes += 8;
	      وإلا
		bytes += 4;
	    }
	}
      وإلا
	bytes += تيندنن_ARG_SIZE ((*ptr)->size);
    }

  cif->bytes = bytes;

  أرجع ffi_prep_cif_machdep (cif);
}

ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
    حالة FFI_TYPE_STRUCT:
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags = (unsigned) cif->rtype->type;
      اكسر;

    افتراضي:
      cif->flags = FFI_TYPE_INT;
      اكسر;
    }

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_SYSV (صحيح (*)(حرف *, extended_cif *),
			   extended_cif *,
			   unsigned, unsigned, unsigned *, فراغ (*fn) ())
     __attribute__ ((__visibility__ ("hidden")));

فراغ
ffi_call (ffi_cif * cif, فراغ (*fn) (), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  إذا ((rvalue == عدم) && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
      ffi_call_SYSV (ffi_prep_args, &ecif, cif->bytes,
		     cif->flags, ecif.rvalue, fn);
      اكسر;
    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

/* Because the following variables are not exported outside libffi, we
   mark them hidden.  */

/* Assembly code for the jump stub.  */
خارجي ثابت حرف ffi_cris_trampoline_template[]
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   ffi_prep_closure_inner function.  */
خارجي ثابت صحيح ffi_cris_trampoline_fn_offset
 __attribute__ ((__visibility__ ("hidden")));

/* Offset into ffi_cris_trampoline_template of where to put the
   closure data.  */
خارجي ثابت صحيح ffi_cris_trampoline_closure_offset
 __attribute__ ((__visibility__ ("hidden")));

/* This function is sibling-called (jumped to) by the closure
   trampoline.  We get R10..R13 at PARAMS[0..3] and a copy of [SP] at
   PARAMS[4] to simplify handling of a straddling parameter.  A copy
   of R9 is at PARAMS[5] and SP at PARAMS[6].  These parameters are
   put at the appropriate place in CLOSURE which is then executed and
   the return value is passed back to the caller.  */

ساكن unsigned long long
ffi_prep_closure_inner (فراغ **params, ffi_closure* closure)
{
  حرف *register_args = (حرف *) params;
  فراغ *struct_ret = params[5];
  حرف *تيندنن_args = params[6];
  حرف *ptr = register_args;
  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;

  /* Max room needed is number of arguments as 64-bit values.  */
  فراغ **avalue = alloca (closure->cif->nargs * حجم(فراغ *));
  صحيح i;
  صحيح doing_regs;
  long long llret = 0;

  /* Find the address of each argument.  */
  لكل (i = 0, doing_regs = 1; i < cif->nargs; i++)
    {
      /* Types up to and including 8 bytes go by-value.  */
      إذا (arg_types[i]->size <= 4)
	{
	  avalue[i] = ptr;
	  ptr += 4;
	}
      وإلا إذا (arg_types[i]->size <= 8)
	{
	  avalue[i] = ptr;
	  ptr += 8;
	}
      وإلا
	{
	  FFI_ASSERT (arg_types[i]->type == FFI_TYPE_STRUCT);

	  /* Passed by-reference, so copy the pointer.  */
	  avalue[i] = *(فراغ **) ptr;
	  ptr += 4;
	}

      /* If we've handled more arguments than fit in registers, start
	 looking at the those passed on the stack.  Step over the
	 first one if we had a straddling parameter.  */
      إذا (doing_regs && ptr >= register_args + 4*4)
	{
	  ptr = تيندنن_args + ((ptr > register_args + 4*4) ? 4 : 0);
	  doing_regs = 0;
	}
    }

  /* Invoke the closure.  */
  (closure->fun) (cif,

		  cif->rtype->type == FFI_TYPE_STRUCT
		  /* The caller allocated space for the return
		     structure, and passed a pointer to this space in
		     R9.  */
		  ? struct_ret

		  /* We take advantage of being able to ignore that
		     the high part isn't set if the return value is
		     not in R10:R11, but in R10 only.  */
		  : (فراغ *) &llret,

		  avalue, closure->user_data);

  أرجع llret;
}

/* API function: Prepare the trampoline.  */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif *, فراغ *, فراغ **, فراغ*),
		      فراغ *user_data,
		      فراغ *codeloc)
{
  فراغ *innerfn = ffi_prep_closure_inner;
  FFI_ASSERT (cif->abi == FFI_SYSV);
  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;
  memcpy (closure->tramp, ffi_cris_trampoline_template,
	  FFI_CRIS_TRAMPOLINE_CODE_PART_SIZE);
  memcpy (closure->tramp + ffi_cris_trampoline_fn_offset,
	  &innerfn, حجم (فراغ *));
  memcpy (closure->tramp + ffi_cris_trampoline_closure_offset,
	  &codeloc, حجم (فراغ *));

  أرجع FFI_OK;
}
