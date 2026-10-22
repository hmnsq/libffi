/* -----------------------------------------------------------------------
   ffiw64.c - Copyright (c) 2018, 2026 Anthony Green
              Copyright (c) 2014 Red Hat, Inc.

   x86 win64 Foreign Function Interface

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

#यदि defined(__x86_64__) || defined(_M_AMD64)
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <tramp.h>

#यदि_परिभाषितम् X86_WIN64
#परिभाषय EFI64(name) name
#अन्यथा
#परिभाषय EFI64(name) FFI_HIDDEN name##_efi64
#यदि_अन्तः

संरचना win64_call_frame
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

बाह्य शून्य ffi_call_win64 (शून्य *राशिः, संरचना win64_call_frame *,
			    शून्य *संवृतिः) FFI_HIDDEN;

ffi_स्थितिः FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  पूर्णाङ्क flags, n;

  विकल्प (cif->abi)
    {
    पक्ष FFI_WIN64:
    पक्ष FFI_GNUW64:
      भङ्ग;
    स्वभाव:
      प्रत्यागम FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  विकल्प (flags)
    {
    स्वभाव:
      भङ्ग;
    पक्ष FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      यदि (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      भङ्ग;
    पक्ष FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    पक्ष FFI_TYPE_STRUCT:
      विकल्प (cif->rtype->आकार)
	{
	पक्ष 8:
	  flags = FFI_TYPE_UINT64;
	  भङ्ग;
	पक्ष 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  भङ्ग;
	पक्ष 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  भङ्ग;
	पक्ष 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  भङ्ग;
	}
      भङ्ग;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  यदि (n < 4)
    n = 4;
  cif->अष्टकानि = n * 8;

  प्रत्यागम FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#यदि defined(_MSC_VER)
#निर्देशः runtime_checks("s", off)
#यदि_अन्तः
FFI_ASAN_NO_SANITIZE
स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *संवृतिः)
{
  पूर्णाङ्क i, j, n, flags;
  UINT64 *राशिः;
  size_t rsize;
  संरचना win64_call_frame *frame;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue_copy = रिक्त;
  पूर्णाङ्क nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  प्रति (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णाङ्क आकार = at->आकार;
      bool needcopy = false;

      विकल्प (at->type)
	{
	पक्ष FFI_TYPE_UINT128:
	पक्ष FFI_TYPE_SINT128:
	  needcopy = true;
	  भङ्ग;
	पक्ष FFI_TYPE_STRUCT:
	  विकल्प (आकार)
	    {
	    पक्ष 1:
	    पक्ष 2:
	    पक्ष 4:
	    पक्ष 8:
	      भङ्ग;
	    स्वभाव:
	      needcopy = true;
	    }
	}
      यदि (needcopy)
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

  flags = cif->flags;
  rsize = 0;

  /* If we have no return value for a structure, we need to create one.
     Otherwise we can ignore the return type entirely.  */
  यदि (rvalue == रिक्त)
    {
      यदि (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->आकार;
      अन्यथा
	flags = FFI_TYPE_VOID;
    }

  राशिः = alloca(cif->अष्टकानि + परिमाण(संरचना win64_call_frame) + rsize);
  frame = (संरचना win64_call_frame *)((वर्ण *)राशिः + cif->अष्टकानि);
  यदि (rsize)
    rvalue = frame + 1;

  frame->fn = (uintptr_t)fn;
  frame->flags = flags;
  frame->rvalue = (uintptr_t)rvalue;

  j = 0;
  यदि (flags == FFI_TYPE_STRUCT)
    {
      राशिः[0] = (uintptr_t)rvalue;
      j = 1;
    }

  प्रति (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      विकल्प (cif->arg_types[i]->आकार)
	{
	पक्ष 8:
	  राशिः[j] = *(UINT64 *)avalue[i];
	  भङ्ग;
	पक्ष 4:
	  राशिः[j] = *(UINT32 *)avalue[i];
	  भङ्ग;
	पक्ष 2:
	  राशिः[j] = *(UINT16 *)avalue[i];
	  भङ्ग;
	पक्ष 1:
	  राशिः[j] = *(UINT8 *)avalue[i];
	  भङ्ग;
	स्वभाव:
	  राशिः[j] = (uintptr_t)avalue[i];
	  भङ्ग;
	}
    }

  ffi_call_win64 (राशिः, frame, संवृतिः);
}
#यदि defined(_MSC_VER)
#निर्देशः runtime_checks("s", restore)
#यदि_अन्तः

शून्य
EFI64(ffi_call)(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

शून्य
EFI64(ffi_call_go)(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *संवृतिः)
{
  ffi_call_int (cif, fn, rvalue, avalue, संवृतिः);
}


बाह्य शून्य ffi_संवृतिः_win64(शून्य) FFI_HIDDEN;
#यदि defined(FFI_EXEC_STATIC_TRAMP)
बाह्य शून्य ffi_संवृतिः_win64_alt(शून्य) FFI_HIDDEN;
#यदि_अन्तः

#यदि_परिभाषितम् FFI_GO_CLOSURES
बाह्य शून्य ffi_go_संवृतिः_win64(शून्य) FFI_HIDDEN;
#यदि_अन्तः

ffi_स्थितिः
EFI64(ffi_prep_संवृतिः_loc)(ffi_संवृतिः* संवृतिः,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_दत्त,
		      शून्य *codeloc MAYBE_अप्रयुक्तम्)
{
  स्थिर ध्रुव अचिह्नित वर्ण trampoline[FFI_TRAMPOLINE_आकार - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  वर्ण *tramp = संवृतिः->tramp;

  विकल्प (cif->abi)
    {
    पक्ष FFI_WIN64:
    पक्ष FFI_GNUW64:
      भङ्ग;
    स्वभाव:
      प्रत्यागम FFI_BAD_ABI;
    }

#यदि defined(FFI_EXEC_STATIC_TRAMP)
  यदि (ffi_tramp_is_present(संवृतिः))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_समुच्चयः_parms (संवृतिः->ftramp, ffi_संवृतिः_win64_alt, संवृतिः);
      गच्छ बहिर्गमनम्;
    }
#यदि_अन्तः

  /* Initialize the dynamic trampoline. */
  स्मृतिं_प्रतिलिख (tramp, trampoline, परिमाण(trampoline));
  *(UINT64 *)(tramp + परिमाण (trampoline)) = (uintptr_t)ffi_संवृतिः_win64;

#यदि defined(FFI_EXEC_STATIC_TRAMP)
बहिर्गमनम्:
#यदि_अन्तः
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;

  प्रत्यागम FFI_OK;
}

#यदि_परिभाषितम् FFI_GO_CLOSURES
ffi_स्थितिः
EFI64(ffi_prep_go_संवृतिः)(ffi_go_संवृतिः* संवृतिः, ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*))
{
  विकल्प (cif->abi)
    {
    पक्ष FFI_WIN64:
    पक्ष FFI_GNUW64:
      भङ्ग;
    स्वभाव:
      प्रत्यागम FFI_BAD_ABI;
    }

  संवृतिः->tramp = ffi_go_संवृतिः_win64;
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;

  प्रत्यागम FFI_OK;
}
#यदि_अन्तः

संरचना win64_संवृतिः_frame
{
  UINT64 rvalue[2];
  UINT64 fargs[4];
  UINT64 retaddr;
  UINT64 तर्काः[];
};

/* Force the inner function to use the MS ABI.  When compiling on win64
   this is a nop.  When compiling on unix, this simplifies the assembly,
   and places the burden of saving the extra call-saved registers on
   the compiler.  */
पूर्णाङ्क FFI_HIDDEN __attribute__((ms_abi))
ffi_संवृतिः_win64_inner(ffi_cif *cif,
			शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			शून्य *user_दत्त,
			संरचना win64_संवृतिः_frame *frame)
{
  शून्य **avalue;
  शून्य *rvalue;
  पूर्णाङ्क i, n, nreg, flags;

  avalue = alloca(cif->nargs * परिमाण(शून्य *));
  rvalue = frame->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  यदि (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (शून्य *)(uintptr_t)frame->तर्काः[0];
      frame->rvalue[0] = frame->तर्काः[0];
      nreg = 1;
    }

  प्रति (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t आकार = cif->arg_types[i]->आकार;
      size_t type = cif->arg_types[i]->type;
      शून्य *a;

      यदि (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  यदि (nreg < 4)
	    a = &frame->fargs[nreg];
	  अन्यथा
	    a = &frame->तर्काः[nreg];
	}
      अन्यथा यदि (आकार == 1 || आकार == 2 || आकार == 4 || आकार == 8)
	a = &frame->तर्काः[nreg];
      अन्यथा
	a = (शून्य *)(uintptr_t)frame->तर्काः[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_दत्त);
  प्रत्यागम flags;
}

#यदि_अन्तः /* __x86_64__ */
