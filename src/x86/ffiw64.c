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

#ಆದರೆ defined(__x86_64__) || defined(_M_AMD64)
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ X86_WIN64
#ವ್ಯಾಖ್ಯಾನಿಸು EFI64(name) name
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು EFI64(name) FFI_HIDDEN name##_efi64
#ಆದರೆ_ಅಂತ್ಯ

ರಚನೆ win64_call_ಚೌಕಟ್ಟು
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_win64 (ಶೂನ್ಯ *ರಾಶಿ, ರಚನೆ win64_call_ಚೌಕಟ್ಟು *,
			    ಶೂನ್ಯ *ಆವರಣ) FFI_HIDDEN;

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  ಪೂರ್ಣಾಂಕ flags, n;

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_WIN64:
    ಸ್ಥಿತಿ FFI_GNUW64:
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  ಆಯ್ಕೆ (flags)
    {
    ಪೂರ್ವನಿಯೋಜಿತ:
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      ಆದರೆ (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಆಯ್ಕೆ (cif->rtype->ಗಾತ್ರ_2)
	{
	ಸ್ಥಿತಿ 8:
	  flags = FFI_TYPE_UINT64;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  ನಿಲ್ಲಿಸು;
	}
      ನಿಲ್ಲಿಸು;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  ಆದರೆ (n < 4)
    n = 4;
  cif->ಬೈಟುಗಳು = n * 8;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ಆದರೆ defined(_MSC_VER)
#ಸೂಚನೆ runtime_checks("s", off)
#ಆದರೆ_ಅಂತ್ಯ
FFI_ASAN_NO_SANITIZE
ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ಪೂರ್ಣಾಂಕ i, j, n, flags;
  UINT64 *ರಾಶಿ;
  size_t rsize;
  ರಚನೆ win64_call_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ಖಾಲಿ;
  ಪೂರ್ಣಾಂಕ nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  ಗಾಗಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = at->ಗಾತ್ರ_2;
      bool needcopy = false;

      ಆಯ್ಕೆ (at->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	ಸ್ಥಿತಿ FFI_TYPE_SINT128:
	  needcopy = true;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಆಯ್ಕೆ (ಗಾತ್ರ_2)
	    {
	    ಸ್ಥಿತಿ 1:
	    ಸ್ಥಿತಿ 2:
	    ಸ್ಥಿತಿ 4:
	    ಸ್ಥಿತಿ 8:
	      ನಿಲ್ಲಿಸು;
	    ಪೂರ್ವನಿಯೋಜಿತ:
	      needcopy = true;
	    }
	}
      ಆದರೆ (needcopy)
        {
          ಅಕ್ಷರ *argcopy = alloca (ಗಾತ್ರ_2);
          ಆದರೆ (avalue_copy == ಖಾಲಿ)
            {
              avalue_copy = alloca (nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
              ಸ್ಮರಣೆ_ನಕಲು (avalue_copy, avalue, nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
              avalue = avalue_copy;
            }
          ಸ್ಮರಣೆ_ನಕಲು (argcopy, avalue[i], ಗಾತ್ರ_2);
          avalue[i] = argcopy;
        }
    }

  flags = cif->flags;
  rsize = 0;

  /* If we have no return value for a structure, we need to create one.
     Otherwise we can ignore the return type entirely.  */
  ಆದರೆ (rvalue == ಖಾಲಿ)
    {
      ಆದರೆ (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->ಗಾತ್ರ_2;
      ಇಲ್ಲದಿದ್ದರೆ
	flags = FFI_TYPE_VOID;
    }

  ರಾಶಿ = alloca(cif->ಬೈಟುಗಳು + ಗಾತ್ರ(ರಚನೆ win64_call_ಚೌಕಟ್ಟು) + rsize);
  ಚೌಕಟ್ಟು = (ರಚನೆ win64_call_ಚೌಕಟ್ಟು *)((ಅಕ್ಷರ *)ರಾಶಿ + cif->ಬೈಟುಗಳು);
  ಆದರೆ (rsize)
    rvalue = ಚೌಕಟ್ಟು + 1;

  ಚೌಕಟ್ಟು->fn = (uintptr_t)fn;
  ಚೌಕಟ್ಟು->flags = flags;
  ಚೌಕಟ್ಟು->rvalue = (uintptr_t)rvalue;

  j = 0;
  ಆದರೆ (flags == FFI_TYPE_STRUCT)
    {
      ರಾಶಿ[0] = (uintptr_t)rvalue;
      j = 1;
    }

  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      ಆಯ್ಕೆ (cif->arg_types[i]->ಗಾತ್ರ_2)
	{
	ಸ್ಥಿತಿ 8:
	  ರಾಶಿ[j] = *(UINT64 *)avalue[i];
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 4:
	  ರಾಶಿ[j] = *(UINT32 *)avalue[i];
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 2:
	  ರಾಶಿ[j] = *(UINT16 *)avalue[i];
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 1:
	  ರಾಶಿ[j] = *(UINT8 *)avalue[i];
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  ರಾಶಿ[j] = (uintptr_t)avalue[i];
	  ನಿಲ್ಲಿಸು;
	}
    }

  ffi_call_win64 (ರಾಶಿ, ಚೌಕಟ್ಟು, ಆವರಣ);
}
#ಆದರೆ defined(_MSC_VER)
#ಸೂಚನೆ runtime_checks("s", restore)
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ
EFI64(ffi_call)(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಖಾಲಿ);
}

ಶೂನ್ಯ
EFI64(ffi_call_go)(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಆವರಣ);
}


ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_win64(ಶೂನ್ಯ) FFI_HIDDEN;
#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_win64_alt(ಶೂನ್ಯ) FFI_HIDDEN;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_win64(ಶೂನ್ಯ) FFI_HIDDEN;
#ಆದರೆ_ಅಂತ್ಯ

ffi_ಸ್ಥಿತಿ
EFI64(ffi_prep_ಆವರಣ_loc)(ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc MAYBE_ಬಳಸದ)
{
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ trampoline[FFI_TRAMPOLINE_ಗಾತ್ರ - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  ಅಕ್ಷರ *tramp = ಆವರಣ->tramp;

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_WIN64:
    ಸ್ಥಿತಿ FFI_GNUW64:
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
    }

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
  ಆದರೆ (ffi_tramp_is_present(ಆವರಣ))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_ಗಣ_parms (ಆವರಣ->ftramp, ffi_ಆವರಣ_win64_alt, ಆವರಣ);
      ಹೋಗು ಹೊರಹೋಗು;
    }
#ಆದರೆ_ಅಂತ್ಯ

  /* Initialize the dynamic trampoline. */
  ಸ್ಮರಣೆ_ನಕಲು (tramp, trampoline, ಗಾತ್ರ(trampoline));
  *(UINT64 *)(tramp + ಗಾತ್ರ (trampoline)) = (uintptr_t)ffi_ಆವರಣ_win64;

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಹೊರಹೋಗು:
#ಆದರೆ_ಅಂತ್ಯ
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES
ffi_ಸ್ಥಿತಿ
EFI64(ffi_prep_go_ಆವರಣ)(ffi_go_ಆವರಣ* ಆವರಣ, ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*))
{
  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_WIN64:
    ಸ್ಥಿತಿ FFI_GNUW64:
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
    }

  ಆವರಣ->tramp = ffi_go_ಆವರಣ_win64;
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}
#ಆದರೆ_ಅಂತ್ಯ

ರಚನೆ win64_ಆವರಣ_ಚೌಕಟ್ಟು
{
  UINT64 rvalue[2];
  UINT64 fargs[4];
  UINT64 retaddr;
  UINT64 ವಾದಗಳು[];
};

/* Force the inner function to use the MS ABI.  When compiling on win64
   this is a nop.  When compiling on unix, this simplifies the assembly,
   and places the burden of saving the extra call-saved registers on
   the compiler.  */
ಪೂರ್ಣಾಂಕ FFI_HIDDEN __attribute__((ms_abi))
ffi_ಆವರಣ_win64_inner(ffi_cif *cif,
			ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
			ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			ರಚನೆ win64_ಆವರಣ_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು)
{
  ಶೂನ್ಯ **avalue;
  ಶೂನ್ಯ *rvalue;
  ಪೂರ್ಣಾಂಕ i, n, nreg, flags;

  avalue = alloca(cif->nargs * ಗಾತ್ರ(ಶೂನ್ಯ *));
  rvalue = ಚೌಕಟ್ಟು->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  ಆದರೆ (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (ಶೂನ್ಯ *)(uintptr_t)ಚೌಕಟ್ಟು->ವಾದಗಳು[0];
      ಚೌಕಟ್ಟು->rvalue[0] = ಚೌಕಟ್ಟು->ವಾದಗಳು[0];
      nreg = 1;
    }

  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t ಗಾತ್ರ_2 = cif->arg_types[i]->ಗಾತ್ರ_2;
      size_t type = cif->arg_types[i]->type;
      ಶೂನ್ಯ *a;

      ಆದರೆ (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  ಆದರೆ (nreg < 4)
	    a = &ಚೌಕಟ್ಟು->fargs[nreg];
	  ಇಲ್ಲದಿದ್ದರೆ
	    a = &ಚೌಕಟ್ಟು->ವಾದಗಳು[nreg];
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 == 1 || ಗಾತ್ರ_2 == 2 || ಗಾತ್ರ_2 == 4 || ಗಾತ್ರ_2 == 8)
	a = &ಚೌಕಟ್ಟು->ವಾದಗಳು[nreg];
      ಇಲ್ಲದಿದ್ದರೆ
	a = (ಶೂನ್ಯ *)(uintptr_t)ಚೌಕಟ್ಟು->ವಾದಗಳು[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);
  ಹಿಂತಿರುಗಿ flags;
}

#ಆದರೆ_ಅಂತ್ಯ /* __x86_64__ */
