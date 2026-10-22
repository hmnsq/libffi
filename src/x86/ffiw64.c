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

#ଯଦି defined(__x86_64__) || defined(_M_AMD64)
#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <tramp.h>

#ifdef X86_WIN64
#define EFI64(name) name
#ଅନ୍ୟଥା
#define EFI64(name) FFI_HIDDEN name##_efi64
#endif

ସଂରଚନା win64_call_frame
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_win64 (ଶୂନ୍ୟ *ଗଦା, ସଂରଚନା win64_call_frame *,
			    ଶୂନ୍ୟ *closure) FFI_HIDDEN;

ffi_status FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, n;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_WIN64:
    ସ୍ଥିତି FFI_GNUW64:
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  ବାଛ (flags)
    {
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      ଯଦି (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      ବାଛ (cif->rtype->size)
	{
	ସ୍ଥିତି 8:
	  flags = FFI_TYPE_UINT64;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  ଭାଙ୍ଗ;
	}
      ଭାଙ୍ଗ;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  ଯଦି (n < 4)
    n = 4;
  cif->bytes = n * 8;

  ଫେରନ୍ତୁ FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ଯଦି defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
FFI_ASAN_NO_SANITIZE
ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	      ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j, n, flags;
  UINT64 *ଗଦା;
  size_t rsize;
  ସଂରଚନା win64_call_frame *frame;
  ffi_type **arg_types = cif->arg_types;
  ଶୂନ୍ୟ **avalue_copy = ଖାଲି;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  ପାଇଁ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = at->size;
      bool needcopy = false;

      ବାଛ (at->type)
	{
	ସ୍ଥିତି FFI_TYPE_UINT128:
	ସ୍ଥିତି FFI_TYPE_SINT128:
	  needcopy = true;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  ବାଛ (size)
	    {
	    ସ୍ଥିତି 1:
	    ସ୍ଥିତି 2:
	    ସ୍ଥିତି 4:
	    ସ୍ଥିତି 8:
	      ଭାଙ୍ଗ;
	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      needcopy = true;
	    }
	}
      ଯଦି (needcopy)
        {
          ଅକ୍ଷର *argcopy = alloca (size);
          ଯଦି (avalue_copy == ଖାଲି)
            {
              avalue_copy = alloca (nargs * ଆକାର (ଶୂନ୍ୟ *));
              memcpy (avalue_copy, avalue, nargs * ଆକାର (ଶୂନ୍ୟ *));
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
  ଯଦି (rvalue == ଖାଲି)
    {
      ଯଦି (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->size;
      ଅନ୍ୟଥା
	flags = FFI_TYPE_VOID;
    }

  ଗଦା = alloca(cif->bytes + ଆକାର(ସଂରଚନା win64_call_frame) + rsize);
  frame = (ସଂରଚନା win64_call_frame *)((ଅକ୍ଷର *)ଗଦା + cif->bytes);
  ଯଦି (rsize)
    rvalue = frame + 1;

  frame->fn = (uintptr_t)fn;
  frame->flags = flags;
  frame->rvalue = (uintptr_t)rvalue;

  j = 0;
  ଯଦି (flags == FFI_TYPE_STRUCT)
    {
      ଗଦା[0] = (uintptr_t)rvalue;
      j = 1;
    }

  ପାଇଁ (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      ବାଛ (cif->arg_types[i]->size)
	{
	ସ୍ଥିତି 8:
	  ଗଦା[j] = *(UINT64 *)avalue[i];
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି 4:
	  ଗଦା[j] = *(UINT32 *)avalue[i];
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି 2:
	  ଗଦା[j] = *(UINT16 *)avalue[i];
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି 1:
	  ଗଦା[j] = *(UINT8 *)avalue[i];
	  ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  ଗଦା[j] = (uintptr_t)avalue[i];
	  ଭାଙ୍ଗ;
	}
    }

  ffi_call_win64 (ଗଦା, frame, closure);
}
#ଯଦି defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

ଶୂନ୍ୟ
EFI64(ffi_call)(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ଖାଲି);
}

ଶୂନ୍ୟ
EFI64(ffi_call_go)(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_win64(ଶୂନ୍ୟ) FFI_HIDDEN;
#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_win64_alt(ଶୂନ୍ୟ) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_win64(ଶୂନ୍ୟ) FFI_HIDDEN;
#endif

ffi_status
EFI64(ffi_prep_closure_loc)(ffi_closure* closure,
		      ffi_cif* cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc MAYBE_UNUSED)
{
  ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର trampoline[FFI_TRAMPOLINE_SIZE - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  ଅକ୍ଷର *tramp = closure->tramp;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_WIN64:
    ସ୍ଥିତି FFI_GNUW64:
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_ABI;
    }

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
  ଯଦି (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_win64_alt, closure);
      ଯାଅ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, ଆକାର(trampoline));
  *(UINT64 *)(tramp + ଆକାର (trampoline)) = (uintptr_t)ffi_closure_win64;

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ଫେରନ୍ତୁ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
EFI64(ffi_prep_go_closure)(ffi_go_closure* closure, ffi_cif* cif,
		     ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*))
{
  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_WIN64:
    ସ୍ଥିତି FFI_GNUW64:
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_ABI;
    }

  closure->tramp = ffi_go_closure_win64;
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}
#endif

ସଂରଚନା win64_closure_frame
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN __attribute__((ms_abi))
ffi_closure_win64_inner(ffi_cif *cif,
			ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
			ଶୂନ୍ୟ *user_data,
			ସଂରଚନା win64_closure_frame *frame)
{
  ଶୂନ୍ୟ **avalue;
  ଶୂନ୍ୟ *rvalue;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n, nreg, flags;

  avalue = alloca(cif->nargs * ଆକାର(ଶୂନ୍ୟ *));
  rvalue = frame->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  ଯଦି (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (ଶୂନ୍ୟ *)(uintptr_t)frame->args[0];
      frame->rvalue[0] = frame->args[0];
      nreg = 1;
    }

  ପାଇଁ (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t size = cif->arg_types[i]->size;
      size_t type = cif->arg_types[i]->type;
      ଶୂନ୍ୟ *a;

      ଯଦି (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  ଯଦି (nreg < 4)
	    a = &frame->fargs[nreg];
	  ଅନ୍ୟଥା
	    a = &frame->args[nreg];
	}
      ଅନ୍ୟଥା ଯଦି (size == 1 || size == 2 || size == 4 || size == 8)
	a = &frame->args[nreg];
      ଅନ୍ୟଥା
	a = (ଶୂନ୍ୟ *)(uintptr_t)frame->args[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);
  ଫେରନ୍ତୁ flags;
}

#endif /* __x86_64__ */
