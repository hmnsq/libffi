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

#જો defined(__x86_64__) || defined(_M_AMD64)
#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <tramp.h>

#ifdef X86_WIN64
#define EFI64(name) name
#નહીંતર
#define EFI64(name) FFI_HIDDEN name##_efi64
#endif

સંરચના win64_call_frame
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

બાહ્ય શૂન્ય ffi_call_win64 (શૂન્ય *ઢગલો, સંરચના win64_call_frame *,
			    શૂન્ય *closure) FFI_HIDDEN;

ffi_status FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  પૂર્ણાંક flags, n;

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_WIN64:
    સ્થિતિ FFI_GNUW64:
      તોડો;
    મૂળભૂત:
      પાછા FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  પસંદગી (flags)
    {
    મૂળભૂત:
      તોડો;
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      જો (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      તોડો;
    સ્થિતિ FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    સ્થિતિ FFI_TYPE_STRUCT:
      પસંદગી (cif->rtype->size)
	{
	સ્થિતિ 8:
	  flags = FFI_TYPE_UINT64;
	  તોડો;
	સ્થિતિ 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  તોડો;
	સ્થિતિ 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  તોડો;
	સ્થિતિ 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  તોડો;
	}
      તોડો;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  જો (n < 4)
    n = 4;
  cif->bytes = n * 8;

  પાછા FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#જો defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
FFI_ASAN_NO_SANITIZE
સ્થિર શૂન્ય
ffi_call_int (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	      શૂન્ય **avalue, શૂન્ય *closure)
{
  પૂર્ણાંક i, j, n, flags;
  UINT64 *ઢગલો;
  size_t rsize;
  સંરચના win64_call_frame *frame;
  ffi_type **arg_types = cif->arg_types;
  શૂન્ય **avalue_copy = ખાલી;
  પૂર્ણાંક nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  માટે (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      પૂર્ણાંક size = at->size;
      bool needcopy = false;

      પસંદગી (at->type)
	{
	સ્થિતિ FFI_TYPE_UINT128:
	સ્થિતિ FFI_TYPE_SINT128:
	  needcopy = true;
	  તોડો;
	સ્થિતિ FFI_TYPE_STRUCT:
	  પસંદગી (size)
	    {
	    સ્થિતિ 1:
	    સ્થિતિ 2:
	    સ્થિતિ 4:
	    સ્થિતિ 8:
	      તોડો;
	    મૂળભૂત:
	      needcopy = true;
	    }
	}
      જો (needcopy)
        {
          અક્ષર *argcopy = alloca (size);
          જો (avalue_copy == ખાલી)
            {
              avalue_copy = alloca (nargs * કદ (શૂન્ય *));
              memcpy (avalue_copy, avalue, nargs * કદ (શૂન્ય *));
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
  જો (rvalue == ખાલી)
    {
      જો (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->size;
      નહીંતર
	flags = FFI_TYPE_VOID;
    }

  ઢગલો = alloca(cif->bytes + કદ(સંરચના win64_call_frame) + rsize);
  frame = (સંરચના win64_call_frame *)((અક્ષર *)ઢગલો + cif->bytes);
  જો (rsize)
    rvalue = frame + 1;

  frame->fn = (uintptr_t)fn;
  frame->flags = flags;
  frame->rvalue = (uintptr_t)rvalue;

  j = 0;
  જો (flags == FFI_TYPE_STRUCT)
    {
      ઢગલો[0] = (uintptr_t)rvalue;
      j = 1;
    }

  માટે (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      પસંદગી (cif->arg_types[i]->size)
	{
	સ્થિતિ 8:
	  ઢગલો[j] = *(UINT64 *)avalue[i];
	  તોડો;
	સ્થિતિ 4:
	  ઢગલો[j] = *(UINT32 *)avalue[i];
	  તોડો;
	સ્થિતિ 2:
	  ઢગલો[j] = *(UINT16 *)avalue[i];
	  તોડો;
	સ્થિતિ 1:
	  ઢગલો[j] = *(UINT8 *)avalue[i];
	  તોડો;
	મૂળભૂત:
	  ઢગલો[j] = (uintptr_t)avalue[i];
	  તોડો;
	}
    }

  ffi_call_win64 (ઢગલો, frame, closure);
}
#જો defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

શૂન્ય
EFI64(ffi_call)(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ખાલી);
}

શૂન્ય
EFI64(ffi_call_go)(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


બાહ્ય શૂન્ય ffi_closure_win64(શૂન્ય) FFI_HIDDEN;
#જો defined(FFI_EXEC_STATIC_TRAMP)
બાહ્ય શૂન્ય ffi_closure_win64_alt(શૂન્ય) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
બાહ્ય શૂન્ય ffi_go_closure_win64(શૂન્ય) FFI_HIDDEN;
#endif

ffi_status
EFI64(ffi_prep_closure_loc)(ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc MAYBE_UNUSED)
{
  સ્થિર અચલ unsigned અક્ષર trampoline[FFI_TRAMPOLINE_SIZE - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  અક્ષર *tramp = closure->tramp;

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_WIN64:
    સ્થિતિ FFI_GNUW64:
      તોડો;
    મૂળભૂત:
      પાછા FFI_BAD_ABI;
    }

#જો defined(FFI_EXEC_STATIC_TRAMP)
  જો (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_win64_alt, closure);
      જાઓ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, કદ(trampoline));
  *(UINT64 *)(tramp + કદ (trampoline)) = (uintptr_t)ffi_closure_win64;

#જો defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  પાછા FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
EFI64(ffi_prep_go_closure)(ffi_go_closure* closure, ffi_cif* cif,
		     શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*))
{
  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_WIN64:
    સ્થિતિ FFI_GNUW64:
      તોડો;
    મૂળભૂત:
      પાછા FFI_BAD_ABI;
    }

  closure->tramp = ffi_go_closure_win64;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}
#endif

સંરચના win64_closure_frame
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
પૂર્ણાંક FFI_HIDDEN __attribute__((ms_abi))
ffi_closure_win64_inner(ffi_cif *cif,
			શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
			શૂન્ય *user_data,
			સંરચના win64_closure_frame *frame)
{
  શૂન્ય **avalue;
  શૂન્ય *rvalue;
  પૂર્ણાંક i, n, nreg, flags;

  avalue = alloca(cif->nargs * કદ(શૂન્ય *));
  rvalue = frame->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  જો (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (શૂન્ય *)(uintptr_t)frame->args[0];
      frame->rvalue[0] = frame->args[0];
      nreg = 1;
    }

  માટે (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t size = cif->arg_types[i]->size;
      size_t type = cif->arg_types[i]->type;
      શૂન્ય *a;

      જો (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  જો (nreg < 4)
	    a = &frame->fargs[nreg];
	  નહીંતર
	    a = &frame->args[nreg];
	}
      નહીંતર જો (size == 1 || size == 2 || size == 4 || size == 8)
	a = &frame->args[nreg];
      નહીંતર
	a = (શૂન્ય *)(uintptr_t)frame->args[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);
  પાછા flags;
}

#endif /* __x86_64__ */
