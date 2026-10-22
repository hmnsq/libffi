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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# यदि FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE बहिर्गमनम् सम्बन्धि date
# endif
#अन्यथा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#यदि_अन्तः

#यदि_परिभाषितम् SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

स्थिर पूर्णाङ्क
ffi_struct_float_आवरणम् (ffi_type *outer_type, पूर्णाङ्क आकार_आवरणम्)
{
  ffi_type **elts;
  ffi_type *t;

  यदि (outer_type->type == FFI_TYPE_COMPLEX)
    {
      पूर्णाङ्क m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->आकार;

      यदि (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      प्रत्यागम (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  प्रति (elts = outer_type->elements; (t = *elts) != रिक्त; elts++)
    {
      size_t z = t->आकार;
      पूर्णाङ्क o, m, tt;

      आकार_आवरणम् = FFI_ALIGN(आकार_आवरणम्, t->alignment);
      विकल्प (t->type)
	{
	पक्ष FFI_TYPE_STRUCT:
	  आकार_आवरणम् = ffi_struct_float_आवरणम् (t, आकार_आवरणम्);
	  अनुवर्त;
	पक्ष FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  यदि (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    भङ्ग;
	  /* FALLTHRU */
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (आकार_आवरणम् >> 2) & 0x3f;	/* extract word offset */
	  आकार_आवरणम् |= m << (o + 8);	/* insert mask into place */
	  भङ्ग;
	}
      आकार_आवरणम् += z;
    }

  आकार_आवरणम् = FFI_ALIGN(आकार_आवरणम्, outer_type->alignment);
  FFI_ASSERT ((आकार_आवरणम् & 0xff) == outer_type->आकार);

  प्रत्यागम आकार_आवरणम्;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

स्थिर शून्य *
ffi_struct_float_merge (पूर्णाङ्क आकार_आवरणम्, शून्य *vi, शून्य *vf)
{
  पूर्णाङ्क आकार = आकार_आवरणम् & 0xff;
  पूर्णाङ्क आवरणम् = आकार_आवरणम् >> 8;
  पूर्णाङ्क n = आकार >> 2;

  यदि (आवरणम् == 0)
    प्रत्यागम vi;
  अन्यथा यदि (आवरणम् == (1 << n) - 1)
    प्रत्यागम vf;
  अन्यथा
    {
      अचिह्नित पूर्णाङ्क *wi = vi, *wf = vf;
      पूर्णाङ्क i;

      प्रति (i = 0; i < n; ++i)
	यदि ((आवरणम् >> i) & 1)
	  wi[i] = wf[i];

      प्रत्यागम vi;
    }
}

/* Similar, but place the data into VD in the end.  */

शून्य FFI_HIDDEN
ffi_struct_float_copy (पूर्णाङ्क आकार_आवरणम्, शून्य *vd, शून्य *vi, शून्य *vf)
{
  पूर्णाङ्क आकार = आकार_आवरणम् & 0xff;
  पूर्णाङ्क आवरणम् = आकार_आवरणम् >> 8;
  पूर्णाङ्क n = आकार >> 2;

  यदि (आवरणम् == 0)
    ;
  अन्यथा यदि (आवरणम् == (1 << n) - 1)
    vi = vf;
  अन्यथा
    {
      अचिह्नित पूर्णाङ्क *wd = vd, *wi = vi, *wf = vf;
      पूर्णाङ्क i;

      प्रति (i = 0; i < n; ++i)
	wd[i] = ((आवरणम् >> i) & 1 ? wf : wi)[i];
      प्रत्यागम;
    }
  स्मृतिं_प्रतिलिख (vd, vi, आकार);
}

/* Perform machine dependent cif processing */

स्थिर ffi_स्थितिः
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  पूर्णाङ्क rtt = rtype->type;
  size_t अष्टकानि = 0;
  पूर्णाङ्क i, n, flags;

  /* Set the return type flag */
  विकल्प (rtt)
    {
    पक्ष FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      भङ्ग;
    पक्ष FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      भङ्ग;
    पक्ष FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      भङ्ग;

    पक्ष FFI_TYPE_COMPLEX:
    पक्ष FFI_TYPE_STRUCT:
      यदि (rtype->आकार > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_अन्तर्_MEM;
	  अष्टकानि = 8;
	}
      अन्यथा
	{
	  पूर्णाङ्क आकार_आवरणम् = ffi_struct_float_आवरणम् (rtype, 0);
	  पूर्णाङ्क word_आकार = (आकार_आवरणम् >> 2) & 0x3f;
	  पूर्णाङ्क all_आवरणम् = (1 << word_आकार) - 1;
	  पूर्णाङ्क fp_आवरणम् = आकार_आवरणम् >> 8;

	  flags = (आकार_आवरणम् << SPARC_SIZEMASK_सरणम्) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  यदि (fp_आवरणम् == 0)
	    {
	      यदि (rtype->alignment >= 8)
		{
		  यदि (rtype->आकार == 8)
		    flags = SPARC_RET_INT64;
		  अन्यथा यदि (rtype->आकार == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  अन्यथा यदि (fp_आवरणम् == all_आवरणम्)
	    विकल्प (word_आकार)
	      {
	      पक्ष 1: flags = SPARC_RET_F_1; भङ्ग;
	      पक्ष 2: flags = SPARC_RET_F_2; भङ्ग;
	      पक्ष 3: flags = SP_V9_RET_F_3; भङ्ग;
	      पक्ष 4: flags = SPARC_RET_F_4; भङ्ग;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      पक्ष 6: flags = SPARC_RET_F_6; भङ्ग;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      पक्ष 8: flags = SPARC_RET_F_8; भङ्ग;
	      }
	}
      भङ्ग;

    पक्ष FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      भङ्ग;
    पक्ष FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      भङ्ग;
    पक्ष FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      भङ्ग;
    पक्ष FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      भङ्ग;
    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      भङ्ग;
    पक्ष FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      भङ्ग;
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_POINTER:
      flags = SPARC_RET_INT64;
      भङ्ग;

    स्वभाव:
      abort();
    }

  अष्टकानि = 0;
  प्रति (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->आकार;
      size_t a = ty->alignment;

      विकल्प (ty->type)
	{
	पक्ष FFI_TYPE_COMPLEX:
	पक्ष FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  यदि (z > 16)
	    {
	      a = z = 8;
	      भङ्ग;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  यदि (अष्टकानि >= 16*8)
	    भङ्ग;
	  यदि ((ffi_struct_float_आवरणम् (ty, 0) & 0xff00) == 0)
	    भङ्ग;
	  /* FALLTHRU */
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_तर्काः;
	  भङ्ग;
	}
      अष्टकानि = FFI_ALIGN(अष्टकानि, a);
      अष्टकानि += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  यदि (अष्टकानि < 6 * 8)
    अष्टकानि = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  अष्टकानि = FFI_ALIGN(अष्टकानि, 16);

  /* Include the call frame to prep_args.  */
  अष्टकानि += 8*16 + 8*8;

  cif->अष्टकानि = अष्टकानि;
  cif->flags = flags;
  प्रत्यागम FFI_OK;
}

ffi_स्थितिः FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  प्रत्यागम ffi_prep_cif_machdep_core(cif);
}

ffi_स्थितिः FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, अचिह्नित nfixedargs, अचिह्नित ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  प्रत्यागम ffi_prep_cif_machdep_core(cif);
}

बाह्य शून्य ffi_call_v9(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
			शून्य **avalue, size_t अष्टकानि, शून्य *संवृतिः) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

पूर्णाङ्क FFI_HIDDEN
ffi_prep_तर्काः_v9(ffi_cif *cif, अचिह्नित दीर्घ *argp, शून्य *rvalue, शून्य **avalue)
{
  ffi_type **p_arg;
  पूर्णाङ्क flags = cif->flags;
  पूर्णाङ्क i, nargs;

  यदि (rvalue == रिक्त)
    {
      यदि (flags & SPARC_FLAG_RET_अन्तर्_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (वर्ण *)argp + cif->अष्टकानि;
	}
      अन्यथा
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

#यदि_परिभाषितम् USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  स्मृतिं_पूरय(argp, 0, 6*8);
#यदि_अन्तः

  यदि (flags & SPARC_FLAG_RET_अन्तर्_MEM)
    *argp++ = (अचिह्नित दीर्घ)rvalue;

  p_arg = cif->arg_types;
  प्रति (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      शून्य *a = avalue[i];
      size_t z;

      विकल्प (ty->type)
	{
	पक्ष FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_POINTER:
	  *argp++ = *(UINT64 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_तर्काः;
	  *argp++ = *(UINT32 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_तर्काः;
	  *argp++ = *(UINT64 *)a;
	  भङ्ग;

	पक्ष FFI_TYPE_LONGDOUBLE:
	पक्ष FFI_TYPE_COMPLEX:
	पक्ष FFI_TYPE_STRUCT:
	  z = ty->आकार;
	  यदि (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (अचिह्नित दीर्घ)a;
	      भङ्ग;
	    }
	  यदि (((अचिह्नित दीर्घ)argp & 15) && ty->alignment > 8)
	    argp++;
	  स्मृतिं_प्रतिलिख(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  भङ्ग;

	स्वभाव:
	  abort();
	}
    }

  प्रत्यागम flags;
}

स्थिर शून्य
ffi_call_int(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *संवृतिः)
{
  size_t अष्टकानि = cif->अष्टकानि;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue_copy = रिक्त;

  FFI_ASSERT (cif->abi == FFI_V9);

  यदि (rvalue == रिक्त && (cif->flags & SPARC_FLAG_RET_अन्तर्_MEM))
    अष्टकानि += FFI_ALIGN (cif->rtype->आकार, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  प्रति (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णाङ्क आकार = at->आकार;
      यदि (at->type == FFI_TYPE_STRUCT && आकार > 4)
        {
          वर्ण *argcopy = alloca (आकार);
          यदि (avalue_copy == रिक्त)
            {
              avalue_copy = alloca (nargs * परिमाण (शून्य *));
              स्मृतिं_प्रतिलिख (avalue_copy, avalue, nargs * परिमाण (शून्य *));
              avalue = avalue_copy;
            }
          स्मृतिं_प्रतिलिख (argcopy, avalue[i], आकार);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -अष्टकानि, संवृतिः);
}

शून्य
ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	    शून्य **avalue, शून्य *संवृतिः)
{
  ffi_call_int(cif, fn, rvalue, avalue, संवृतिः);
}

#यदि_परिभाषितम् __GNUC__
स्थिर अन्तःस्थ शून्य
ffi_flush_icache (शून्य *p)
{
  __asm__ चञ्चल ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#अन्यथा
बाह्य शून्य ffi_flush_icache (शून्य *) FFI_HIDDEN;
#यदि_अन्तः

बाह्य शून्य ffi_संवृतिः_v9(शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_go_संवृतिः_v9(शून्य) FFI_HIDDEN;

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः* संवृतिः,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_दत्त,
		      शून्य *codeloc)
{
  अचिह्नित पूर्णाङ्क *tramp = (अचिह्नित पूर्णाङ्क *) &संवृतिः->tramp[0];
  अचिह्नित दीर्घ fn;

  यदि (cif->abi != FFI_V9)
    प्रत्यागम FFI_BAD_ABI;

  /* Trampoline address is equal to the closure address.  We take advantage
     of that to reduce the trampoline size by 8 bytes. */
  fn = (अचिह्नित दीर्घ) ffi_संवृतिः_v9;
  tramp[0] = 0x83414000;	/* rd	%pc, %g1	*/
  tramp[1] = 0xca586010;	/* ldx	[%g1+16], %g5	*/
  tramp[2] = 0x81c14000;	/* jmp	%g5		*/
  tramp[3] = 0x01000000;	/* nop			*/
  *((अचिह्नित दीर्घ *) &tramp[4]) = fn;

  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;

  ffi_flush_icache (संवृतिः);

  प्रत्यागम FFI_OK;
}

ffi_स्थितिः
ffi_prep_go_संवृतिः (ffi_go_संवृतिः* संवृतिः, ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*))
{
  यदि (cif->abi != FFI_V9)
    प्रत्यागम FFI_BAD_ABI;

  संवृतिः->tramp = ffi_go_संवृतिः_v9;
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;

  प्रत्यागम FFI_OK;
}

पूर्णाङ्क FFI_HIDDEN
ffi_संवृतिः_sparc_inner_v9(ffi_cif *cif,
			   शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			   शून्य *user_दत्त, शून्य *rvalue,
			   अचिह्नित दीर्घ *gpr, अचिह्नित दीर्घ *fpr)
{
  ffi_type **arg_types;
  शून्य **avalue;
  पूर्णाङ्क i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * परिमाण(शून्य *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  यदि (flags & SPARC_FLAG_RET_अन्तर्_MEM)
    {
      rvalue = (शून्य *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  अन्यथा
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  प्रति (i = 0; i < nargs; i++, argn = argx)
    {
      पूर्णाङ्क named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      शून्य *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      विकल्प (ty->type)
	{
	पक्ष FFI_TYPE_COMPLEX:
	पक्ष FFI_TYPE_STRUCT:
	  z = ty->आकार;
	  यदि (z > 16)
	    a = *(शून्य **)a;
	  अन्यथा
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      यदि (named && argn < 16)
		{
		  पूर्णाङ्क आकार_आवरणम् = ffi_struct_float_आवरणम् (ty, 0);
		  पूर्णाङ्क argn_आवरणम् = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  आकार_आवरणम् = (आकार_आवरणम् & 0xff) | (आकार_आवरणम् & argn_आवरणम्);
		  a = ffi_struct_float_merge (आकार_आवरणम्, gpr+argn, fpr+argn);
		}
	    }
	  भङ्ग;

	पक्ष FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  भङ्ग;
	पक्ष FFI_TYPE_DOUBLE:
	  यदि (named && argn < 16)
	    a = fpr + argn;
	  भङ्ग;
	पक्ष FFI_TYPE_FLOAT:
	  यदि (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  भङ्ग;

	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_POINTER:
	  भङ्ग;
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT32:
	  a += 4;
	  भङ्ग;
        पक्ष FFI_TYPE_UINT16:
        पक्ष FFI_TYPE_SINT16:
	  a += 6;
	  भङ्ग;
        पक्ष FFI_TYPE_UINT8:
        पक्ष FFI_TYPE_SINT8:
	  a += 7;
	  भङ्ग;

	स्वभाव:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_दत्त);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  प्रत्यागम flags;
}
#यदि_अन्तः /* SPARC64 */
