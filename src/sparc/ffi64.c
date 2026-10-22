/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2011, 2013, 2026 Anthony Green
           Copyright (c) 1996, 2003-2004, 2007-2008 Red Hat, Inc.

   SPARC Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश <stdlib.h>
#समावेश "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# अगर FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अन्यथा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#पूर्ण_अगर

#अगर_घोषित SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

अटल पूर्णांक
ffi_struct_float_mask (ffi_type *outer_type, पूर्णांक आकार_mask)
{
  ffi_type **elts;
  ffi_type *t;

  अगर (outer_type->type == FFI_TYPE_COMPLEX)
    {
      पूर्णांक m = 0, tt = outer_type->elements[0]->type;
      माप_प्रकार z = outer_type->आकार;

      अगर (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      वापस (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  क्रम (elts = outer_type->elements; (t = *elts) != शून्य; elts++)
    {
      माप_प्रकार z = t->आकार;
      पूर्णांक o, m, tt;

      आकार_mask = FFI_ALIGN(आकार_mask, t->alignment);
      चयन (t->type)
	{
	हाल FFI_TYPE_STRUCT:
	  आकार_mask = ffi_struct_float_mask (t, आकार_mask);
	  जारी;
	हाल FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  अगर (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    अवरोध;
	  /* FALLTHRU */
	हाल FFI_TYPE_FLOAT:
	हाल FFI_TYPE_DOUBLE:
	हाल FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (आकार_mask >> 2) & 0x3f;	/* extract word offset */
	  आकार_mask |= m << (o + 8);	/* insert mask into place */
	  अवरोध;
	}
      आकार_mask += z;
    }

  आकार_mask = FFI_ALIGN(आकार_mask, outer_type->alignment);
  FFI_ASSERT ((आकार_mask & 0xff) == outer_type->आकार);

  वापस आकार_mask;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

अटल व्योम *
ffi_struct_float_merge (पूर्णांक आकार_mask, व्योम *vi, व्योम *vf)
{
  पूर्णांक आकार = आकार_mask & 0xff;
  पूर्णांक mask = आकार_mask >> 8;
  पूर्णांक n = आकार >> 2;

  अगर (mask == 0)
    वापस vi;
  अन्यथा अगर (mask == (1 << n) - 1)
    वापस vf;
  अन्यथा
    {
      अचिन्हित पूर्णांक *wi = vi, *wf = vf;
      पूर्णांक i;

      क्रम (i = 0; i < n; ++i)
	अगर ((mask >> i) & 1)
	  wi[i] = wf[i];

      वापस vi;
    }
}

/* Similar, but place the data into VD in the end.  */

व्योम FFI_HIDDEN
ffi_struct_float_copy (पूर्णांक आकार_mask, व्योम *vd, व्योम *vi, व्योम *vf)
{
  पूर्णांक आकार = आकार_mask & 0xff;
  पूर्णांक mask = आकार_mask >> 8;
  पूर्णांक n = आकार >> 2;

  अगर (mask == 0)
    ;
  अन्यथा अगर (mask == (1 << n) - 1)
    vi = vf;
  अन्यथा
    {
      अचिन्हित पूर्णांक *wd = vd, *wi = vi, *wf = vf;
      पूर्णांक i;

      क्रम (i = 0; i < n; ++i)
	wd[i] = ((mask >> i) & 1 ? wf : wi)[i];
      वापस;
    }
  स_नकल (vd, vi, आकार);
}

/* Perform machine dependent cif processing */

अटल ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  पूर्णांक rtt = rtype->type;
  माप_प्रकार bytes = 0;
  पूर्णांक i, n, flags;

  /* Set the return type flag */
  चयन (rtt)
    {
    हाल FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      अवरोध;
    हाल FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      अवरोध;
    हाल FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      अवरोध;
    हाल FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      अवरोध;

    हाल FFI_TYPE_COMPLEX:
    हाल FFI_TYPE_STRUCT:
      अगर (rtype->आकार > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_IN_MEM;
	  bytes = 8;
	}
      अन्यथा
	{
	  पूर्णांक आकार_mask = ffi_struct_float_mask (rtype, 0);
	  पूर्णांक word_आकार = (आकार_mask >> 2) & 0x3f;
	  पूर्णांक all_mask = (1 << word_आकार) - 1;
	  पूर्णांक fp_mask = आकार_mask >> 8;

	  flags = (आकार_mask << SPARC_SIZEMASK_SHIFT) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  अगर (fp_mask == 0)
	    {
	      अगर (rtype->alignment >= 8)
		{
		  अगर (rtype->आकार == 8)
		    flags = SPARC_RET_INT64;
		  अन्यथा अगर (rtype->आकार == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  अन्यथा अगर (fp_mask == all_mask)
	    चयन (word_आकार)
	      {
	      हाल 1: flags = SPARC_RET_F_1; अवरोध;
	      हाल 2: flags = SPARC_RET_F_2; अवरोध;
	      हाल 3: flags = SP_V9_RET_F_3; अवरोध;
	      हाल 4: flags = SPARC_RET_F_4; अवरोध;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      हाल 6: flags = SPARC_RET_F_6; अवरोध;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      हाल 8: flags = SPARC_RET_F_8; अवरोध;
	      }
	}
      अवरोध;

    हाल FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      अवरोध;
    हाल FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      अवरोध;
    हाल FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      अवरोध;
    हाल FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      अवरोध;
    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      अवरोध;
    हाल FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      अवरोध;
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_POINTER:
      flags = SPARC_RET_INT64;
      अवरोध;

    शेष:
      पात();
    }

  bytes = 0;
  क्रम (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      माप_प्रकार z = ty->आकार;
      माप_प्रकार a = ty->alignment;

      चयन (ty->type)
	{
	हाल FFI_TYPE_COMPLEX:
	हाल FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  अगर (z > 16)
	    {
	      a = z = 8;
	      अवरोध;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  अगर (bytes >= 16*8)
	    अवरोध;
	  अगर ((ffi_struct_float_mask (ty, 0) & 0xff00) == 0)
	    अवरोध;
	  /* FALLTHRU */
	हाल FFI_TYPE_FLOAT:
	हाल FFI_TYPE_DOUBLE:
	हाल FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  अवरोध;
	}
      bytes = FFI_ALIGN(bytes, a);
      bytes += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  अगर (bytes < 6 * 8)
    bytes = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 16);

  /* Include the call frame to prep_args.  */
  bytes += 8*16 + 8*8;

  cif->bytes = bytes;
  cif->flags = flags;
  वापस FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  वापस ffi_prep_cif_machdep_core(cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, अचिन्हित nfixedargs, अचिन्हित ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  वापस ffi_prep_cif_machdep_core(cif);
}

बाह्य व्योम ffi_call_v9(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
			व्योम **avalue, माप_प्रकार bytes, व्योम *closure) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

पूर्णांक FFI_HIDDEN
ffi_prep_args_v9(ffi_cif *cif, अचिन्हित दीर्घ *argp, व्योम *rvalue, व्योम **avalue)
{
  ffi_type **p_arg;
  पूर्णांक flags = cif->flags;
  पूर्णांक i, nargs;

  अगर (rvalue == शून्य)
    {
      अगर (flags & SPARC_FLAG_RET_IN_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (अक्षर *)argp + cif->bytes;
	}
      अन्यथा
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

#अगर_घोषित USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  स_रखो(argp, 0, 6*8);
#पूर्ण_अगर

  अगर (flags & SPARC_FLAG_RET_IN_MEM)
    *argp++ = (अचिन्हित दीर्घ)rvalue;

  p_arg = cif->arg_types;
  क्रम (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      व्योम *a = avalue[i];
      माप_प्रकार z;

      चयन (ty->type)
	{
	हाल FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  अवरोध;
	हाल FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  अवरोध;
	हाल FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  अवरोध;
	हाल FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  अवरोध;
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  अवरोध;
	हाल FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  अवरोध;
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_POINTER:
	  *argp++ = *(UINT64 *)a;
	  अवरोध;
	हाल FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT32 *)a;
	  अवरोध;
	हाल FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT64 *)a;
	  अवरोध;

	हाल FFI_TYPE_LONGDOUBLE:
	हाल FFI_TYPE_COMPLEX:
	हाल FFI_TYPE_STRUCT:
	  z = ty->आकार;
	  अगर (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (अचिन्हित दीर्घ)a;
	      अवरोध;
	    }
	  अगर (((अचिन्हित दीर्घ)argp & 15) && ty->alignment > 8)
	    argp++;
	  स_नकल(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  अवरोध;

	शेष:
	  पात();
	}
    }

  वापस flags;
}

अटल व्योम
ffi_call_int(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
	     व्योम **avalue, व्योम *closure)
{
  माप_प्रकार bytes = cif->bytes;
  माप_प्रकार i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  व्योम **avalue_copy = शून्य;

  FFI_ASSERT (cif->abi == FFI_V9);

  अगर (rvalue == शून्य && (cif->flags & SPARC_FLAG_RET_IN_MEM))
    bytes += FFI_ALIGN (cif->rtype->आकार, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  क्रम (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णांक आकार = at->आकार;
      अगर (at->type == FFI_TYPE_STRUCT && आकार > 4)
        {
          अक्षर *argcopy = alloca (आकार);
          अगर (avalue_copy == शून्य)
            {
              avalue_copy = alloca (nargs * माप (व्योम *));
              स_नकल (avalue_copy, avalue, nargs * माप (व्योम *));
              avalue = avalue_copy;
            }
          स_नकल (argcopy, avalue[i], आकार);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -bytes, closure);
}

व्योम
ffi_call(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, शून्य);
}

व्योम
ffi_call_go(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
	    व्योम **avalue, व्योम *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

#अगर_घोषित __GNUC__
अटल अंतरभूत व्योम
ffi_flush_icache (व्योम *p)
{
  __asm__ अस्थिर ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#अन्यथा
बाह्य व्योम ffi_flush_icache (व्योम *) FFI_HIDDEN;
#पूर्ण_अगर

बाह्य व्योम ffi_closure_v9(व्योम) FFI_HIDDEN;
बाह्य व्योम ffi_go_closure_v9(व्योम) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  अचिन्हित पूर्णांक *tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
  अचिन्हित दीर्घ fn;

  अगर (cif->abi != FFI_V9)
    वापस FFI_BAD_ABI;

  /* Trampoline address is equal to the closure address.  We take advantage
     of that to reduce the trampoline size by 8 bytes. */
  fn = (अचिन्हित दीर्घ) ffi_closure_v9;
  tramp[0] = 0x83414000;	/* rd	%pc, %g1	*/
  tramp[1] = 0xca586010;	/* ldx	[%g1+16], %g5	*/
  tramp[2] = 0x81c14000;	/* jmp	%g5		*/
  tramp[3] = 0x01000000;	/* nop			*/
  *((अचिन्हित दीर्घ *) &tramp[4]) = fn;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  ffi_flush_icache (closure);

  वापस FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*))
{
  अगर (cif->abi != FFI_V9)
    वापस FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v9;
  closure->cif = cif;
  closure->fun = fun;

  वापस FFI_OK;
}

पूर्णांक FFI_HIDDEN
ffi_closure_sparc_inner_v9(ffi_cif *cif,
			   व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
			   व्योम *user_तथ्य, व्योम *rvalue,
			   अचिन्हित दीर्घ *gpr, अचिन्हित दीर्घ *fpr)
{
  ffi_type **arg_types;
  व्योम **avalue;
  पूर्णांक i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * माप(व्योम *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  अगर (flags & SPARC_FLAG_RET_IN_MEM)
    {
      rvalue = (व्योम *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  अन्यथा
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  क्रम (i = 0; i < nargs; i++, argn = argx)
    {
      पूर्णांक named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      व्योम *a = &gpr[argn];
      माप_प्रकार z;

      argx = argn + 1;
      चयन (ty->type)
	{
	हाल FFI_TYPE_COMPLEX:
	हाल FFI_TYPE_STRUCT:
	  z = ty->आकार;
	  अगर (z > 16)
	    a = *(व्योम **)a;
	  अन्यथा
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      अगर (named && argn < 16)
		{
		  पूर्णांक आकार_mask = ffi_struct_float_mask (ty, 0);
		  पूर्णांक argn_mask = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  आकार_mask = (आकार_mask & 0xff) | (आकार_mask & argn_mask);
		  a = ffi_struct_float_merge (आकार_mask, gpr+argn, fpr+argn);
		}
	    }
	  अवरोध;

	हाल FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  अवरोध;
	हाल FFI_TYPE_DOUBLE:
	  अगर (named && argn < 16)
	    a = fpr + argn;
	  अवरोध;
	हाल FFI_TYPE_FLOAT:
	  अगर (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  अवरोध;

	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_POINTER:
	  अवरोध;
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT32:
	  a += 4;
	  अवरोध;
        हाल FFI_TYPE_UINT16:
        हाल FFI_TYPE_SINT16:
	  a += 6;
	  अवरोध;
        हाल FFI_TYPE_UINT8:
        हाल FFI_TYPE_SINT8:
	  a += 7;
	  अवरोध;

	शेष:
	  पात();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_तथ्य);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  वापस flags;
}
#पूर्ण_अगर /* SPARC64 */
