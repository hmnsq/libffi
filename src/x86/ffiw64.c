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

#सचे defined(__x86_64__) || defined(_M_AMD64)
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <tramp.h>

#ifdef X86_WIN64
#define EFI64(name) name
#अञ्ञथा
#define EFI64(name) FFI_HIDDEN name##_efi64
#endif

संरचना win64_call_frame
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

बाह्य शून्य ffi_call_win64 (शून्य *राशिः, संरचना win64_call_frame *,
			    शून्य *closure) FFI_HIDDEN;

ffi_status FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  पूर्णाङ्क flags, n;

  विकल्प (cif->abi)
    {
    पक्ष FFI_WIN64:
    पक्ष FFI_GNUW64:
      भङ्ग;
    स्वभाव:
      पटिनिवत्त FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  विकल्प (flags)
    {
    स्वभाव:
      भङ्ग;
    पक्ष FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      सचे (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      भङ्ग;
    पक्ष FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    पक्ष FFI_TYPE_STRUCT:
      विकल्प (cif->rtype->size)
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
  सचे (n < 4)
    n = 4;
  cif->bytes = n * 8;

  पटिनिवत्त FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#सचे defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
FFI_ASAN_NO_SANITIZE
स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
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
  अत्थाय (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णाङ्क size = at->size;
      bool needcopy = false;

      विकल्प (at->type)
	{
	पक्ष FFI_TYPE_UINT128:
	पक्ष FFI_TYPE_SINT128:
	  needcopy = true;
	  भङ्ग;
	पक्ष FFI_TYPE_STRUCT:
	  विकल्प (size)
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
      सचे (needcopy)
        {
          वर्ण *argcopy = alloca (size);
          सचे (avalue_copy == रिक्त)
            {
              avalue_copy = alloca (nargs * परिमाण (शून्य *));
              memcpy (avalue_copy, avalue, nargs * परिमाण (शून्य *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }

  flags = cif->flags;
  rsize = 0;

  /* If we have no return value for a structure, we need to create one.
     Otherwise we can ignore the return type entirely.  */
  सचे (rvalue == रिक्त)
    {
      सचे (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->size;
      अञ्ञथा
	flags = FFI_TYPE_VOID;
    }

  राशिः = alloca(cif->bytes + परिमाण(संरचना win64_call_frame) + rsize);
  frame = (संरचना win64_call_frame *)((वर्ण *)राशिः + cif->bytes);
  सचे (rsize)
    rvalue = frame + 1;

  frame->fn = (uintptr_t)fn;
  frame->flags = flags;
  frame->rvalue = (uintptr_t)rvalue;

  j = 0;
  सचे (flags == FFI_TYPE_STRUCT)
    {
      राशिः[0] = (uintptr_t)rvalue;
      j = 1;
    }

  अत्थाय (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      विकल्प (cif->arg_types[i]->size)
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

  ffi_call_win64 (राशिः, frame, closure);
}
#सचे defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

शून्य
EFI64(ffi_call)(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

शून्य
EFI64(ffi_call_go)(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


बाह्य शून्य ffi_closure_win64(शून्य) FFI_HIDDEN;
#सचे defined(FFI_EXEC_STATIC_TRAMP)
बाह्य शून्य ffi_closure_win64_alt(शून्य) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
बाह्य शून्य ffi_go_closure_win64(शून्य) FFI_HIDDEN;
#endif

ffi_status
EFI64(ffi_prep_closure_loc)(ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc MAYBE_UNUSED)
{
  स्थिर ध्रुव unsigned वर्ण trampoline[FFI_TRAMPOLINE_SIZE - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  वर्ण *tramp = closure->tramp;

  विकल्प (cif->abi)
    {
    पक्ष FFI_WIN64:
    पक्ष FFI_GNUW64:
      भङ्ग;
    स्वभाव:
      पटिनिवत्त FFI_BAD_ABI;
    }

#सचे defined(FFI_EXEC_STATIC_TRAMP)
  सचे (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_win64_alt, closure);
      गच्छ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, परिमाण(trampoline));
  *(UINT64 *)(tramp + परिमाण (trampoline)) = (uintptr_t)ffi_closure_win64;

#सचे defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  पटिनिवत्त FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
EFI64(ffi_prep_go_closure)(ffi_go_closure* closure, ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*))
{
  विकल्प (cif->abi)
    {
    पक्ष FFI_WIN64:
    पक्ष FFI_GNUW64:
      भङ्ग;
    स्वभाव:
      पटिनिवत्त FFI_BAD_ABI;
    }

  closure->tramp = ffi_go_closure_win64;
  closure->cif = cif;
  closure->fun = fun;

  पटिनिवत्त FFI_OK;
}
#endif

संरचना win64_closure_frame
{
  UINT64 rvalue[2];
  UINT64 fargs[4];
  UINT64 retaddr;
  UINT64 args[];
};

/* Force the inner function to use the MS ABI.  When compiling on win64
   this is a nop.  When compiling on unix, this simplifies the assembly,
   and places the burden of saving the extra call-saved registers on
   the compiler.  */
पूर्णाङ्क FFI_HIDDEN __attribute__((ms_abi))
ffi_closure_win64_inner(ffi_cif *cif,
			शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			शून्य *user_data,
			संरचना win64_closure_frame *frame)
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
  सचे (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (शून्य *)(uintptr_t)frame->args[0];
      frame->rvalue[0] = frame->args[0];
      nreg = 1;
    }

  अत्थाय (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t size = cif->arg_types[i]->size;
      size_t type = cif->arg_types[i]->type;
      शून्य *a;

      सचे (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  सचे (nreg < 4)
	    a = &frame->fargs[nreg];
	  अञ्ञथा
	    a = &frame->args[nreg];
	}
      अञ्ञथा सचे (size == 1 || size == 2 || size == 4 || size == 8)
	a = &frame->args[nreg];
      अञ्ञथा
	a = (शून्य *)(uintptr_t)frame->args[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);
  पटिनिवत्त flags;
}

#endif /* __x86_64__ */
