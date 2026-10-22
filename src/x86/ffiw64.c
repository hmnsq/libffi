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

#ಯದಿ defined(__x86_64__) || defined(_M_AMD64)
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>

#ifdef X86_WIN64
#define EFI64(name) name
#ಅನ್ಯಥಾ
#define EFI64(name) FFI_HIDDEN name##_efi64
#endif

ಸಂರಚನಾ win64_call_frame
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_win64 (ಶೂನ್ಯ *ರಾಶಿ, ಸಂರಚನಾ win64_call_frame *,
			    ಶೂನ್ಯ *closure) FFI_HIDDEN;

ffi_status FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  ಪೂರ್ಣಾಙ್ಕ flags, n;

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_WIN64:
    ಪಕ್ಷ FFI_GNUW64:
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  ವಿಕಲ್ಪ (flags)
    {
    ಸ್ವಭಾವ:
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      ಯದಿ (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    ಪಕ್ಷ FFI_TYPE_STRUCT:
      ವಿಕಲ್ಪ (cif->rtype->size)
	{
	ಪಕ್ಷ 8:
	  flags = FFI_TYPE_UINT64;
	  ಭಙ್ಗ;
	ಪಕ್ಷ 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  ಭಙ್ಗ;
	ಪಕ್ಷ 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  ಭಙ್ಗ;
	ಪಕ್ಷ 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  ಭಙ್ಗ;
	}
      ಭಙ್ಗ;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  ಯದಿ (n < 4)
    n = 4;
  cif->bytes = n * 8;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ಯದಿ defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
FFI_ASAN_NO_SANITIZE
ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ಪೂರ್ಣಾಙ್ಕ i, j, n, flags;
  UINT64 *ರಾಶಿ;
  size_t rsize;
  ಸಂರಚನಾ win64_call_frame *frame;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ರಿಕ್ತ;
  ಪೂರ್ಣಾಙ್ಕ nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  ಪ್ರತಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಙ್ಕ size = at->size;
      bool needcopy = false;

      ವಿಕಲ್ಪ (at->type)
	{
	ಪಕ್ಷ FFI_TYPE_UINT128:
	ಪಕ್ಷ FFI_TYPE_SINT128:
	  needcopy = true;
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  ವಿಕಲ್ಪ (size)
	    {
	    ಪಕ್ಷ 1:
	    ಪಕ್ಷ 2:
	    ಪಕ್ಷ 4:
	    ಪಕ್ಷ 8:
	      ಭಙ್ಗ;
	    ಸ್ವಭಾವ:
	      needcopy = true;
	    }
	}
      ಯದಿ (needcopy)
        {
          ವರ್ಣ *argcopy = alloca (size);
          ಯದಿ (avalue_copy == ರಿಕ್ತ)
            {
              avalue_copy = alloca (nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
              memcpy (avalue_copy, avalue, nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
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
  ಯದಿ (rvalue == ರಿಕ್ತ)
    {
      ಯದಿ (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->size;
      ಅನ್ಯಥಾ
	flags = FFI_TYPE_VOID;
    }

  ರಾಶಿ = alloca(cif->bytes + ಪರಿಮಾಣ(ಸಂರಚನಾ win64_call_frame) + rsize);
  frame = (ಸಂರಚನಾ win64_call_frame *)((ವರ್ಣ *)ರಾಶಿ + cif->bytes);
  ಯದಿ (rsize)
    rvalue = frame + 1;

  frame->fn = (uintptr_t)fn;
  frame->flags = flags;
  frame->rvalue = (uintptr_t)rvalue;

  j = 0;
  ಯದಿ (flags == FFI_TYPE_STRUCT)
    {
      ರಾಶಿ[0] = (uintptr_t)rvalue;
      j = 1;
    }

  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      ವಿಕಲ್ಪ (cif->arg_types[i]->size)
	{
	ಪಕ್ಷ 8:
	  ರಾಶಿ[j] = *(UINT64 *)avalue[i];
	  ಭಙ್ಗ;
	ಪಕ್ಷ 4:
	  ರಾಶಿ[j] = *(UINT32 *)avalue[i];
	  ಭಙ್ಗ;
	ಪಕ್ಷ 2:
	  ರಾಶಿ[j] = *(UINT16 *)avalue[i];
	  ಭಙ್ಗ;
	ಪಕ್ಷ 1:
	  ರಾಶಿ[j] = *(UINT8 *)avalue[i];
	  ಭಙ್ಗ;
	ಸ್ವಭಾವ:
	  ರಾಶಿ[j] = (uintptr_t)avalue[i];
	  ಭಙ್ಗ;
	}
    }

  ffi_call_win64 (ರಾಶಿ, frame, closure);
}
#ಯದಿ defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

ಶೂನ್ಯ
EFI64(ffi_call)(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ರಿಕ್ತ);
}

ಶೂನ್ಯ
EFI64(ffi_call_go)(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_win64(ಶೂನ್ಯ) FFI_HIDDEN;
#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_win64_alt(ಶೂನ್ಯ) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_win64(ಶೂನ್ಯ) FFI_HIDDEN;
#endif

ffi_status
EFI64(ffi_prep_closure_loc)(ffi_closure* closure,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc MAYBE_UNUSED)
{
  ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ trampoline[FFI_TRAMPOLINE_SIZE - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  ವರ್ಣ *tramp = closure->tramp;

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_WIN64:
    ಪಕ್ಷ FFI_GNUW64:
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
    }

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
  ಯದಿ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_win64_alt, closure);
      ಗಚ್ಛ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, ಪರಿಮಾಣ(trampoline));
  *(UINT64 *)(tramp + ಪರಿಮಾಣ (trampoline)) = (uintptr_t)ffi_closure_win64;

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
EFI64(ffi_prep_go_closure)(ffi_go_closure* closure, ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*))
{
  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_WIN64:
    ಪಕ್ಷ FFI_GNUW64:
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
    }

  closure->tramp = ffi_go_closure_win64;
  closure->cif = cif;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}
#endif

ಸಂರಚನಾ win64_closure_frame
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
ಪೂರ್ಣಾಙ್ಕ FFI_HIDDEN __attribute__((ms_abi))
ffi_closure_win64_inner(ffi_cif *cif,
			ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
			ಶೂನ್ಯ *user_data,
			ಸಂರಚನಾ win64_closure_frame *frame)
{
  ಶೂನ್ಯ **avalue;
  ಶೂನ್ಯ *rvalue;
  ಪೂರ್ಣಾಙ್ಕ i, n, nreg, flags;

  avalue = alloca(cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ *));
  rvalue = frame->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  ಯದಿ (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (ಶೂನ್ಯ *)(uintptr_t)frame->args[0];
      frame->rvalue[0] = frame->args[0];
      nreg = 1;
    }

  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t size = cif->arg_types[i]->size;
      size_t type = cif->arg_types[i]->type;
      ಶೂನ್ಯ *a;

      ಯದಿ (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  ಯದಿ (nreg < 4)
	    a = &frame->fargs[nreg];
	  ಅನ್ಯಥಾ
	    a = &frame->args[nreg];
	}
      ಅನ್ಯಥಾ ಯದಿ (size == 1 || size == 2 || size == 4 || size == 8)
	a = &frame->args[nreg];
      ಅನ್ಯಥಾ
	a = (ಶೂನ್ಯ *)(uintptr_t)frame->args[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);
  ಪ್ರತ್ಯಾಗಮ flags;
}

#endif /* __x86_64__ */
