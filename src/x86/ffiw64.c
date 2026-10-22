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

#எனில் defined(__x86_64__) || defined(_M_AMD64)
#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <tramp.h>

#ifdef X86_WIN64
#define EFI64(name) name
#இல்லையெனில்
#define EFI64(name) FFI_HIDDEN name##_efi64
#endif

கட்டமைப்பு win64_call_frame
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

வெளி வெறுமை ffi_call_win64 (வெறுமை *அடுக்கு, கட்டமைப்பு win64_call_frame *,
			    வெறுமை *closure) FFI_HIDDEN;

ffi_status FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  முழு flags, n;

  தேர்வு (cif->abi)
    {
    நிலை FFI_WIN64:
    நிலை FFI_GNUW64:
      நிறுத்து;
    இயல்பு:
      திரும்பு FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  தேர்வு (flags)
    {
    இயல்பு:
      நிறுத்து;
    நிலை FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      எனில் (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      நிறுத்து;
    நிலை FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    நிலை FFI_TYPE_STRUCT:
      தேர்வு (cif->rtype->size)
	{
	நிலை 8:
	  flags = FFI_TYPE_UINT64;
	  நிறுத்து;
	நிலை 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  நிறுத்து;
	நிலை 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  நிறுத்து;
	நிலை 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  நிறுத்து;
	}
      நிறுத்து;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  எனில் (n < 4)
    n = 4;
  cif->bytes = n * 8;

  திரும்பு FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#எனில் defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
FFI_ASAN_NO_SANITIZE
நிலையான வெறுமை
ffi_call_int (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	      வெறுமை **avalue, வெறுமை *closure)
{
  முழு i, j, n, flags;
  UINT64 *அடுக்கு;
  size_t rsize;
  கட்டமைப்பு win64_call_frame *frame;
  ffi_type **arg_types = cif->arg_types;
  வெறுமை **avalue_copy = வெற்று;
  முழு nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  க்கு (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      முழு size = at->size;
      bool needcopy = false;

      தேர்வு (at->type)
	{
	நிலை FFI_TYPE_UINT128:
	நிலை FFI_TYPE_SINT128:
	  needcopy = true;
	  நிறுத்து;
	நிலை FFI_TYPE_STRUCT:
	  தேர்வு (size)
	    {
	    நிலை 1:
	    நிலை 2:
	    நிலை 4:
	    நிலை 8:
	      நிறுத்து;
	    இயல்பு:
	      needcopy = true;
	    }
	}
      எனில் (needcopy)
        {
          எழுத்து *argcopy = alloca (size);
          எனில் (avalue_copy == வெற்று)
            {
              avalue_copy = alloca (nargs * அளவு (வெறுமை *));
              memcpy (avalue_copy, avalue, nargs * அளவு (வெறுமை *));
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
  எனில் (rvalue == வெற்று)
    {
      எனில் (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->size;
      இல்லையெனில்
	flags = FFI_TYPE_VOID;
    }

  அடுக்கு = alloca(cif->bytes + அளவு(கட்டமைப்பு win64_call_frame) + rsize);
  frame = (கட்டமைப்பு win64_call_frame *)((எழுத்து *)அடுக்கு + cif->bytes);
  எனில் (rsize)
    rvalue = frame + 1;

  frame->fn = (uintptr_t)fn;
  frame->flags = flags;
  frame->rvalue = (uintptr_t)rvalue;

  j = 0;
  எனில் (flags == FFI_TYPE_STRUCT)
    {
      அடுக்கு[0] = (uintptr_t)rvalue;
      j = 1;
    }

  க்கு (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      தேர்வு (cif->arg_types[i]->size)
	{
	நிலை 8:
	  அடுக்கு[j] = *(UINT64 *)avalue[i];
	  நிறுத்து;
	நிலை 4:
	  அடுக்கு[j] = *(UINT32 *)avalue[i];
	  நிறுத்து;
	நிலை 2:
	  அடுக்கு[j] = *(UINT16 *)avalue[i];
	  நிறுத்து;
	நிலை 1:
	  அடுக்கு[j] = *(UINT8 *)avalue[i];
	  நிறுத்து;
	இயல்பு:
	  அடுக்கு[j] = (uintptr_t)avalue[i];
	  நிறுத்து;
	}
    }

  ffi_call_win64 (அடுக்கு, frame, closure);
}
#எனில் defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

வெறுமை
EFI64(ffi_call)(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, வெற்று);
}

வெறுமை
EFI64(ffi_call_go)(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


வெளி வெறுமை ffi_closure_win64(வெறுமை) FFI_HIDDEN;
#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெளி வெறுமை ffi_closure_win64_alt(வெறுமை) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
வெளி வெறுமை ffi_go_closure_win64(வெறுமை) FFI_HIDDEN;
#endif

ffi_status
EFI64(ffi_prep_closure_loc)(ffi_closure* closure,
		      ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc MAYBE_UNUSED)
{
  நிலையான மாறிலி unsigned எழுத்து trampoline[FFI_TRAMPOLINE_SIZE - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  எழுத்து *tramp = closure->tramp;

  தேர்வு (cif->abi)
    {
    நிலை FFI_WIN64:
    நிலை FFI_GNUW64:
      நிறுத்து;
    இயல்பு:
      திரும்பு FFI_BAD_ABI;
    }

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
  எனில் (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_win64_alt, closure);
      செல் out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, அளவு(trampoline));
  *(UINT64 *)(tramp + அளவு (trampoline)) = (uintptr_t)ffi_closure_win64;

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  திரும்பு FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
EFI64(ffi_prep_go_closure)(ffi_go_closure* closure, ffi_cif* cif,
		     வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*))
{
  தேர்வு (cif->abi)
    {
    நிலை FFI_WIN64:
    நிலை FFI_GNUW64:
      நிறுத்து;
    இயல்பு:
      திரும்பு FFI_BAD_ABI;
    }

  closure->tramp = ffi_go_closure_win64;
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
}
#endif

கட்டமைப்பு win64_closure_frame
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
முழு FFI_HIDDEN __attribute__((ms_abi))
ffi_closure_win64_inner(ffi_cif *cif,
			வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
			வெறுமை *user_data,
			கட்டமைப்பு win64_closure_frame *frame)
{
  வெறுமை **avalue;
  வெறுமை *rvalue;
  முழு i, n, nreg, flags;

  avalue = alloca(cif->nargs * அளவு(வெறுமை *));
  rvalue = frame->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  எனில் (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (வெறுமை *)(uintptr_t)frame->args[0];
      frame->rvalue[0] = frame->args[0];
      nreg = 1;
    }

  க்கு (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t size = cif->arg_types[i]->size;
      size_t type = cif->arg_types[i]->type;
      வெறுமை *a;

      எனில் (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  எனில் (nreg < 4)
	    a = &frame->fargs[nreg];
	  இல்லையெனில்
	    a = &frame->args[nreg];
	}
      இல்லையெனில் எனில் (size == 1 || size == 2 || size == 4 || size == 8)
	a = &frame->args[nreg];
      இல்லையெனில்
	a = (வெறுமை *)(uintptr_t)frame->args[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);
  திரும்பு flags;
}

#endif /* __x86_64__ */
