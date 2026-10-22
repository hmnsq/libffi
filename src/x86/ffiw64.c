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

#ਜੇ defined(__x86_64__) || defined(_M_AMD64)
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ X86_WIN64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EFI64(name) name
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EFI64(name) FFI_HIDDEN name##_efi64
#ਜੇ_ਅੰਤ

ਬਣਤਰ win64_call_ਚੌਖਟਾ
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

ਬਾਹਰੀ ਖਾਲੀ ffi_call_win64 (ਖਾਲੀ *ਢੇਰ, ਬਣਤਰ win64_call_ਚੌਖਟਾ *,
			    ਖਾਲੀ *ਘੇਰਾ) FFI_HIDDEN;

ffi_ਸਥਿਤੀ FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  ਪੂਰਨ flags, n;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_WIN64:
    ਹਾਲਤ FFI_GNUW64:
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  ਚੋਣ (flags)
    {
    ਮੂਲ:
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      ਜੇ (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਚੋਣ (cif->rtype->ਮਾਪ)
	{
	ਹਾਲਤ 8:
	  flags = FFI_TYPE_UINT64;
	  ਰੋਕੋ;
	ਹਾਲਤ 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  ਰੋਕੋ;
	ਹਾਲਤ 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  ਰੋਕੋ;
	ਹਾਲਤ 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  ਰੋਕੋ;
	}
      ਰੋਕੋ;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  ਜੇ (n < 4)
    n = 4;
  cif->ਬਾਈਟਾਂ = n * 8;

  ਵਾਪਸ FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ਜੇ defined(_MSC_VER)
#ਹਦਾਇਤ runtime_checks("s", off)
#ਜੇ_ਅੰਤ
FFI_ASAN_NO_SANITIZE
ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	      ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ਪੂਰਨ i, j, n, flags;
  UINT64 *ਢੇਰ;
  size_t rsize;
  ਬਣਤਰ win64_call_ਚੌਖਟਾ *ਚੌਖਟਾ;
  ffi_type **arg_types = cif->arg_types;
  ਖਾਲੀ **avalue_copy = ਨਿਰਾ;
  ਪੂਰਨ nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  ਲਈ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ਪੂਰਨ ਮਾਪ = at->ਮਾਪ;
      bool needcopy = false;

      ਚੋਣ (at->type)
	{
	ਹਾਲਤ FFI_TYPE_UINT128:
	ਹਾਲਤ FFI_TYPE_SINT128:
	  needcopy = true;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਚੋਣ (ਮਾਪ)
	    {
	    ਹਾਲਤ 1:
	    ਹਾਲਤ 2:
	    ਹਾਲਤ 4:
	    ਹਾਲਤ 8:
	      ਰੋਕੋ;
	    ਮੂਲ:
	      needcopy = true;
	    }
	}
      ਜੇ (needcopy)
        {
          ਅੱਖਰ *argcopy = alloca (ਮਾਪ);
          ਜੇ (avalue_copy == ਨਿਰਾ)
            {
              avalue_copy = alloca (nargs * ਆਕਾਰ (ਖਾਲੀ *));
              ਸਮ੍ਰਿਤੀ_ਨਕਲ (avalue_copy, avalue, nargs * ਆਕਾਰ (ਖਾਲੀ *));
              avalue = avalue_copy;
            }
          ਸਮ੍ਰਿਤੀ_ਨਕਲ (argcopy, avalue[i], ਮਾਪ);
          avalue[i] = argcopy;
        }
    }

  flags = cif->flags;
  rsize = 0;

  /* If we have no return value for a structure, we need to create one.
     Otherwise we can ignore the return type entirely.  */
  ਜੇ (rvalue == ਨਿਰਾ)
    {
      ਜੇ (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->ਮਾਪ;
      ਨਹੀਂਤਾਂ
	flags = FFI_TYPE_VOID;
    }

  ਢੇਰ = alloca(cif->ਬਾਈਟਾਂ + ਆਕਾਰ(ਬਣਤਰ win64_call_ਚੌਖਟਾ) + rsize);
  ਚੌਖਟਾ = (ਬਣਤਰ win64_call_ਚੌਖਟਾ *)((ਅੱਖਰ *)ਢੇਰ + cif->ਬਾਈਟਾਂ);
  ਜੇ (rsize)
    rvalue = ਚੌਖਟਾ + 1;

  ਚੌਖਟਾ->fn = (uintptr_t)fn;
  ਚੌਖਟਾ->flags = flags;
  ਚੌਖਟਾ->rvalue = (uintptr_t)rvalue;

  j = 0;
  ਜੇ (flags == FFI_TYPE_STRUCT)
    {
      ਢੇਰ[0] = (uintptr_t)rvalue;
      j = 1;
    }

  ਲਈ (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      ਚੋਣ (cif->arg_types[i]->ਮਾਪ)
	{
	ਹਾਲਤ 8:
	  ਢੇਰ[j] = *(UINT64 *)avalue[i];
	  ਰੋਕੋ;
	ਹਾਲਤ 4:
	  ਢੇਰ[j] = *(UINT32 *)avalue[i];
	  ਰੋਕੋ;
	ਹਾਲਤ 2:
	  ਢੇਰ[j] = *(UINT16 *)avalue[i];
	  ਰੋਕੋ;
	ਹਾਲਤ 1:
	  ਢੇਰ[j] = *(UINT8 *)avalue[i];
	  ਰੋਕੋ;
	ਮੂਲ:
	  ਢੇਰ[j] = (uintptr_t)avalue[i];
	  ਰੋਕੋ;
	}
    }

  ffi_call_win64 (ਢੇਰ, ਚੌਖਟਾ, ਘੇਰਾ);
}
#ਜੇ defined(_MSC_VER)
#ਹਦਾਇਤ runtime_checks("s", restore)
#ਜੇ_ਅੰਤ

ਖਾਲੀ
EFI64(ffi_call)(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਨਿਰਾ);
}

ਖਾਲੀ
EFI64(ffi_call_go)(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਘੇਰਾ);
}


ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_win64(ਖਾਲੀ) FFI_HIDDEN;
#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_win64_alt(ਖਾਲੀ) FFI_HIDDEN;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_win64(ਖਾਲੀ) FFI_HIDDEN;
#ਜੇ_ਅੰਤ

ffi_ਸਥਿਤੀ
EFI64(ffi_prep_ਘੇਰਾ_loc)(ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc MAYBE_ਅਣਵਰਤਿਆ)
{
  ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ trampoline[FFI_TRAMPOLINE_ਮਾਪ - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  ਅੱਖਰ *tramp = ਘੇਰਾ->tramp;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_WIN64:
    ਹਾਲਤ FFI_GNUW64:
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_ABI;
    }

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
  ਜੇ (ffi_tramp_is_present(ਘੇਰਾ))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_ਸੈੱਟ_parms (ਘੇਰਾ->ftramp, ffi_ਘੇਰਾ_win64_alt, ਘੇਰਾ);
      ਜਾਓ ਬਾਹਰਲਾ;
    }
#ਜੇ_ਅੰਤ

  /* Initialize the dynamic trampoline. */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ (tramp, trampoline, ਆਕਾਰ(trampoline));
  *(UINT64 *)(tramp + ਆਕਾਰ (trampoline)) = (uintptr_t)ffi_ਘੇਰਾ_win64;

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਬਾਹਰਲਾ:
#ਜੇ_ਅੰਤ
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ਵਾਪਸ FFI_OK;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES
ffi_ਸਥਿਤੀ
EFI64(ffi_prep_go_ਘੇਰਾ)(ffi_go_ਘੇਰਾ* ਘੇਰਾ, ffi_cif* cif,
		     ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*))
{
  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_WIN64:
    ਹਾਲਤ FFI_GNUW64:
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_ABI;
    }

  ਘੇਰਾ->tramp = ffi_go_ਘੇਰਾ_win64;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}
#ਜੇ_ਅੰਤ

ਬਣਤਰ win64_ਘੇਰਾ_ਚੌਖਟਾ
{
  UINT64 rvalue[2];
  UINT64 fargs[4];
  UINT64 retaddr;
  UINT64 ਦਲੀਲਾਂ[];
};

/* Force the inner function to use the MS ABI.  When compiling on win64
   this is a nop.  When compiling on unix, this simplifies the assembly,
   and places the burden of saving the extra call-saved registers on
   the compiler.  */
ਪੂਰਨ FFI_HIDDEN __attribute__((ms_abi))
ffi_ਘੇਰਾ_win64_inner(ffi_cif *cif,
			ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
			ਖਾਲੀ *user_ਅੰਕੜੇ,
			ਬਣਤਰ win64_ਘੇਰਾ_ਚੌਖਟਾ *ਚੌਖਟਾ)
{
  ਖਾਲੀ **avalue;
  ਖਾਲੀ *rvalue;
  ਪੂਰਨ i, n, nreg, flags;

  avalue = alloca(cif->nargs * ਆਕਾਰ(ਖਾਲੀ *));
  rvalue = ਚੌਖਟਾ->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  ਜੇ (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (ਖਾਲੀ *)(uintptr_t)ਚੌਖਟਾ->ਦਲੀਲਾਂ[0];
      ਚੌਖਟਾ->rvalue[0] = ਚੌਖਟਾ->ਦਲੀਲਾਂ[0];
      nreg = 1;
    }

  ਲਈ (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t ਮਾਪ = cif->arg_types[i]->ਮਾਪ;
      size_t type = cif->arg_types[i]->type;
      ਖਾਲੀ *a;

      ਜੇ (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  ਜੇ (nreg < 4)
	    a = &ਚੌਖਟਾ->fargs[nreg];
	  ਨਹੀਂਤਾਂ
	    a = &ਚੌਖਟਾ->ਦਲੀਲਾਂ[nreg];
	}
      ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ == 1 || ਮਾਪ == 2 || ਮਾਪ == 4 || ਮਾਪ == 8)
	a = &ਚੌਖਟਾ->ਦਲੀਲਾਂ[nreg];
      ਨਹੀਂਤਾਂ
	a = (ਖਾਲੀ *)(uintptr_t)ਚੌਖਟਾ->ਦਲੀਲਾਂ[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);
  ਵਾਪਸ flags;
}

#ਜੇ_ਅੰਤ /* __x86_64__ */
