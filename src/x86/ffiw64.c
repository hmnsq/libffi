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

#എങ്കിൽ defined(__x86_64__) || defined(_M_AMD64)
#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <tramp.h>

#ifdef X86_WIN64
#define EFI64(name) name
#അല്ലെങ്കിൽ
#define EFI64(name) FFI_HIDDEN name##_efi64
#endif

ഘടന win64_call_frame
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

ബാഹ്യ ശൂന്യം ffi_call_win64 (ശൂന്യം *അട്ടി, ഘടന win64_call_frame *,
			    ശൂന്യം *closure) FFI_HIDDEN;

ffi_status FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  പൂർണ്ണസംഖ്യ flags, n;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_WIN64:
    അവസ്ഥ FFI_GNUW64:
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  തിരഞ്ഞെടുപ്പ് (flags)
    {
    സ്ഥിരസ്ഥിതി:
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      എങ്കിൽ (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    അവസ്ഥ FFI_TYPE_STRUCT:
      തിരഞ്ഞെടുപ്പ് (cif->rtype->size)
	{
	അവസ്ഥ 8:
	  flags = FFI_TYPE_UINT64;
	  നിർത്തുക;
	അവസ്ഥ 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  നിർത്തുക;
	അവസ്ഥ 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  നിർത്തുക;
	അവസ്ഥ 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  നിർത്തുക;
	}
      നിർത്തുക;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  എങ്കിൽ (n < 4)
    n = 4;
  cif->bytes = n * 8;

  തിരികെ FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#എങ്കിൽ defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
FFI_ASAN_NO_SANITIZE
സ്ഥിതം ശൂന്യം
ffi_call_int (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
	      ശൂന്യം **avalue, ശൂന്യം *closure)
{
  പൂർണ്ണസംഖ്യ i, j, n, flags;
  UINT64 *അട്ടി;
  size_t rsize;
  ഘടന win64_call_frame *frame;
  ffi_type **arg_types = cif->arg_types;
  ശൂന്യം **avalue_copy = ശൂന്യ;
  പൂർണ്ണസംഖ്യ nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  വേണ്ടി (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      പൂർണ്ണസംഖ്യ size = at->size;
      bool needcopy = false;

      തിരഞ്ഞെടുപ്പ് (at->type)
	{
	അവസ്ഥ FFI_TYPE_UINT128:
	അവസ്ഥ FFI_TYPE_SINT128:
	  needcopy = true;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_STRUCT:
	  തിരഞ്ഞെടുപ്പ് (size)
	    {
	    അവസ്ഥ 1:
	    അവസ്ഥ 2:
	    അവസ്ഥ 4:
	    അവസ്ഥ 8:
	      നിർത്തുക;
	    സ്ഥിരസ്ഥിതി:
	      needcopy = true;
	    }
	}
      എങ്കിൽ (needcopy)
        {
          അക്ഷരം *argcopy = alloca (size);
          എങ്കിൽ (avalue_copy == ശൂന്യ)
            {
              avalue_copy = alloca (nargs * വലുപ്പം (ശൂന്യം *));
              memcpy (avalue_copy, avalue, nargs * വലുപ്പം (ശൂന്യം *));
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
  എങ്കിൽ (rvalue == ശൂന്യ)
    {
      എങ്കിൽ (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->size;
      അല്ലെങ്കിൽ
	flags = FFI_TYPE_VOID;
    }

  അട്ടി = alloca(cif->bytes + വലുപ്പം(ഘടന win64_call_frame) + rsize);
  frame = (ഘടന win64_call_frame *)((അക്ഷരം *)അട്ടി + cif->bytes);
  എങ്കിൽ (rsize)
    rvalue = frame + 1;

  frame->fn = (uintptr_t)fn;
  frame->flags = flags;
  frame->rvalue = (uintptr_t)rvalue;

  j = 0;
  എങ്കിൽ (flags == FFI_TYPE_STRUCT)
    {
      അട്ടി[0] = (uintptr_t)rvalue;
      j = 1;
    }

  വേണ്ടി (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      തിരഞ്ഞെടുപ്പ് (cif->arg_types[i]->size)
	{
	അവസ്ഥ 8:
	  അട്ടി[j] = *(UINT64 *)avalue[i];
	  നിർത്തുക;
	അവസ്ഥ 4:
	  അട്ടി[j] = *(UINT32 *)avalue[i];
	  നിർത്തുക;
	അവസ്ഥ 2:
	  അട്ടി[j] = *(UINT16 *)avalue[i];
	  നിർത്തുക;
	അവസ്ഥ 1:
	  അട്ടി[j] = *(UINT8 *)avalue[i];
	  നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
	  അട്ടി[j] = (uintptr_t)avalue[i];
	  നിർത്തുക;
	}
    }

  ffi_call_win64 (അട്ടി, frame, closure);
}
#എങ്കിൽ defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

ശൂന്യം
EFI64(ffi_call)(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ശൂന്യ);
}

ശൂന്യം
EFI64(ffi_call_go)(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
	     ശൂന്യം **avalue, ശൂന്യം *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


ബാഹ്യ ശൂന്യം ffi_closure_win64(ശൂന്യം) FFI_HIDDEN;
#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
ബാഹ്യ ശൂന്യം ffi_closure_win64_alt(ശൂന്യം) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
ബാഹ്യ ശൂന്യം ffi_go_closure_win64(ശൂന്യം) FFI_HIDDEN;
#endif

ffi_status
EFI64(ffi_prep_closure_loc)(ffi_closure* closure,
		      ffi_cif* cif,
		      ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc MAYBE_UNUSED)
{
  സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം trampoline[FFI_TRAMPOLINE_SIZE - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  അക്ഷരം *tramp = closure->tramp;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_WIN64:
    അവസ്ഥ FFI_GNUW64:
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_ABI;
    }

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
  എങ്കിൽ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_win64_alt, closure);
      പോകുക out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, വലുപ്പം(trampoline));
  *(UINT64 *)(tramp + വലുപ്പം (trampoline)) = (uintptr_t)ffi_closure_win64;

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  തിരികെ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
EFI64(ffi_prep_go_closure)(ffi_go_closure* closure, ffi_cif* cif,
		     ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*))
{
  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_WIN64:
    അവസ്ഥ FFI_GNUW64:
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_ABI;
    }

  closure->tramp = ffi_go_closure_win64;
  closure->cif = cif;
  closure->fun = fun;

  തിരികെ FFI_OK;
}
#endif

ഘടന win64_closure_frame
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
പൂർണ്ണസംഖ്യ FFI_HIDDEN __attribute__((ms_abi))
ffi_closure_win64_inner(ffi_cif *cif,
			ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
			ശൂന്യം *user_data,
			ഘടന win64_closure_frame *frame)
{
  ശൂന്യം **avalue;
  ശൂന്യം *rvalue;
  പൂർണ്ണസംഖ്യ i, n, nreg, flags;

  avalue = alloca(cif->nargs * വലുപ്പം(ശൂന്യം *));
  rvalue = frame->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  എങ്കിൽ (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (ശൂന്യം *)(uintptr_t)frame->args[0];
      frame->rvalue[0] = frame->args[0];
      nreg = 1;
    }

  വേണ്ടി (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t size = cif->arg_types[i]->size;
      size_t type = cif->arg_types[i]->type;
      ശൂന്യം *a;

      എങ്കിൽ (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  എങ്കിൽ (nreg < 4)
	    a = &frame->fargs[nreg];
	  അല്ലെങ്കിൽ
	    a = &frame->args[nreg];
	}
      അല്ലെങ്കിൽ എങ്കിൽ (size == 1 || size == 2 || size == 4 || size == 8)
	a = &frame->args[nreg];
      അല്ലെങ്കിൽ
	a = (ശൂന്യം *)(uintptr_t)frame->args[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);
  തിരികെ flags;
}

#endif /* __x86_64__ */
