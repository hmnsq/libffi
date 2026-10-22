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

#যদি defined(__x86_64__) || defined(_M_AMD64)
#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <tramp.h>

#ifdef X86_WIN64
#define EFI64(name) name
#অন্যথা
#define EFI64(name) FFI_HIDDEN name##_efi64
#endif

কাঠামো win64_call_frame
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

বাহ্যিক শূন্য ffi_call_win64 (শূন্য *স্তূপ, কাঠামো win64_call_frame *,
			    শূন্য *closure) FFI_HIDDEN;

ffi_status FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  পূর্ণসংখ্যা flags, n;

  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_WIN64:
    ক্ষেত্র FFI_GNUW64:
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  নির্বাচন (flags)
    {
    পূর্বনির্ধারিত:
      বিরতি;
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      যদি (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      বিরতি;
    ক্ষেত্র FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    ক্ষেত্র FFI_TYPE_STRUCT:
      নির্বাচন (cif->rtype->আকার_2)
	{
	ক্ষেত্র 8:
	  flags = FFI_TYPE_UINT64;
	  বিরতি;
	ক্ষেত্র 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  বিরতি;
	ক্ষেত্র 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  বিরতি;
	ক্ষেত্র 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  বিরতি;
	}
      বিরতি;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  যদি (n < 4)
    n = 4;
  cif->bytes = n * 8;

  ফেরত FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#যদি defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
FFI_ASAN_NO_SANITIZE
স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	      শূন্য **avalue, শূন্য *closure)
{
  পূর্ণসংখ্যা i, j, n, flags;
  UINT64 *স্তূপ;
  size_t rsize;
  কাঠামো win64_call_frame *frame;
  ffi_type **arg_types = cif->arg_types;
  শূন্য **avalue_copy = ফাঁকা;
  পূর্ণসংখ্যা nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  জন্য (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      পূর্ণসংখ্যা আকার_2 = at->আকার_2;
      bool needcopy = false;

      নির্বাচন (at->type)
	{
	ক্ষেত্র FFI_TYPE_UINT128:
	ক্ষেত্র FFI_TYPE_SINT128:
	  needcopy = true;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_STRUCT:
	  নির্বাচন (আকার_2)
	    {
	    ক্ষেত্র 1:
	    ক্ষেত্র 2:
	    ক্ষেত্র 4:
	    ক্ষেত্র 8:
	      বিরতি;
	    পূর্বনির্ধারিত:
	      needcopy = true;
	    }
	}
      যদি (needcopy)
        {
          অক্ষর *argcopy = alloca (আকার_2);
          যদি (avalue_copy == ফাঁকা)
            {
              avalue_copy = alloca (nargs * আকার (শূন্য *));
              memcpy (avalue_copy, avalue, nargs * আকার (শূন্য *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], আকার_2);
          avalue[i] = argcopy;
        }
    }

  flags = cif->flags;
  rsize = 0;

  /* If we have no return value for a structure, we need to create one.
     Otherwise we can ignore the return type entirely.  */
  যদি (rvalue == ফাঁকা)
    {
      যদি (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->আকার_2;
      অন্যথা
	flags = FFI_TYPE_VOID;
    }

  স্তূপ = alloca(cif->bytes + আকার(কাঠামো win64_call_frame) + rsize);
  frame = (কাঠামো win64_call_frame *)((অক্ষর *)স্তূপ + cif->bytes);
  যদি (rsize)
    rvalue = frame + 1;

  frame->fn = (uintptr_t)fn;
  frame->flags = flags;
  frame->rvalue = (uintptr_t)rvalue;

  j = 0;
  যদি (flags == FFI_TYPE_STRUCT)
    {
      স্তূপ[0] = (uintptr_t)rvalue;
      j = 1;
    }

  জন্য (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      নির্বাচন (cif->arg_types[i]->আকার_2)
	{
	ক্ষেত্র 8:
	  স্তূপ[j] = *(UINT64 *)avalue[i];
	  বিরতি;
	ক্ষেত্র 4:
	  স্তূপ[j] = *(UINT32 *)avalue[i];
	  বিরতি;
	ক্ষেত্র 2:
	  স্তূপ[j] = *(UINT16 *)avalue[i];
	  বিরতি;
	ক্ষেত্র 1:
	  স্তূপ[j] = *(UINT8 *)avalue[i];
	  বিরতি;
	পূর্বনির্ধারিত:
	  স্তূপ[j] = (uintptr_t)avalue[i];
	  বিরতি;
	}
    }

  ffi_call_win64 (স্তূপ, frame, closure);
}
#যদি defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

শূন্য
EFI64(ffi_call)(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ফাঁকা);
}

শূন্য
EFI64(ffi_call_go)(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


বাহ্যিক শূন্য ffi_closure_win64(শূন্য) FFI_HIDDEN;
#যদি defined(FFI_EXEC_STATIC_TRAMP)
বাহ্যিক শূন্য ffi_closure_win64_alt(শূন্য) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
বাহ্যিক শূন্য ffi_go_closure_win64(শূন্য) FFI_HIDDEN;
#endif

ffi_status
EFI64(ffi_prep_closure_loc)(ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc MAYBE_UNUSED)
{
  স্থির ধ্রুবক unsigned অক্ষর trampoline[FFI_TRAMPOLINE_আকার - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  অক্ষর *tramp = closure->tramp;

  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_WIN64:
    ক্ষেত্র FFI_GNUW64:
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_ABI;
    }

#যদি defined(FFI_EXEC_STATIC_TRAMP)
  যদি (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_win64_alt, closure);
      যাও out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, আকার(trampoline));
  *(UINT64 *)(tramp + আকার (trampoline)) = (uintptr_t)ffi_closure_win64;

#যদি defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

  ফেরত FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
EFI64(ffi_prep_go_closure)(ffi_go_closure* closure, ffi_cif* cif,
		     শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*))
{
  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_WIN64:
    ক্ষেত্র FFI_GNUW64:
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_ABI;
    }

  closure->tramp = ffi_go_closure_win64;
  closure->cif = cif;
  closure->fun = fun;

  ফেরত FFI_OK;
}
#endif

কাঠামো win64_closure_frame
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
পূর্ণসংখ্যা FFI_HIDDEN __attribute__((ms_abi))
ffi_closure_win64_inner(ffi_cif *cif,
			শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
			শূন্য *user_তথ্য,
			কাঠামো win64_closure_frame *frame)
{
  শূন্য **avalue;
  শূন্য *rvalue;
  পূর্ণসংখ্যা i, n, nreg, flags;

  avalue = alloca(cif->nargs * আকার(শূন্য *));
  rvalue = frame->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  যদি (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (শূন্য *)(uintptr_t)frame->args[0];
      frame->rvalue[0] = frame->args[0];
      nreg = 1;
    }

  জন্য (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t আকার_2 = cif->arg_types[i]->আকার_2;
      size_t type = cif->arg_types[i]->type;
      শূন্য *a;

      যদি (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  যদি (nreg < 4)
	    a = &frame->fargs[nreg];
	  অন্যথা
	    a = &frame->args[nreg];
	}
      অন্যথা যদি (আকার_2 == 1 || আকার_2 == 2 || আকার_2 == 4 || আকার_2 == 8)
	a = &frame->args[nreg];
      অন্যথা
	a = (শূন্য *)(uintptr_t)frame->args[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_তথ্য);
  ফেরত flags;
}

#endif /* __x86_64__ */
