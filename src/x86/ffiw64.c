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

#ꦪꦼꦤ꧀ defined(__x86_64__) || defined(_M_AMD64)
#include <ffi.h>
#include <ffi_common.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <tramp.h>

#ifdef X86_WIN64
#define EFI64(name) name
#ꦭꦶꦪ
#define EFI64(name) FFI_HIDDEN name##_efi64
#endif

sa_nracanaa win64_call_frame
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

extern void ffi_call_win64 (void *stack, sa_nracanaa win64_call_frame *,
			    void *closure) FFI_HIDDEN;

ffi_status FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  puur_naa_nka flags, n;

  switch (cif->abi)
    {
    pak_sa FFI_WIN64:
    pak_sa FFI_GNUW64:
      bha_nga;
    default:
      ꦮꦭꦶ FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  switch (flags)
    {
    default:
      bha_nga;
    pak_sa FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      ꦪꦼꦤ꧀ (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      bha_nga;
    pak_sa FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    pak_sa FFI_TYPE_STRUCT:
      switch (cif->rtype->size)
	{
	pak_sa 8:
	  flags = FFI_TYPE_UINT64;
	  bha_nga;
	pak_sa 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  bha_nga;
	pak_sa 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  bha_nga;
	pak_sa 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  bha_nga;
	}
      bha_nga;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  ꦪꦼꦤ꧀ (n < 4)
    n = 4;
  cif->bytes = n * 8;

  ꦮꦭꦶ FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ꦪꦼꦤ꧀ defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
FFI_ASAN_NO_SANITIZE
static void
ffi_call_int (ffi_cif *cif, void (*fn)(void), void *rvalue,
	      void **avalue, void *closure)
{
  puur_naa_nka i, j, n, flags;
  UINT64 *stack;
  size_t rsize;
  sa_nracanaa win64_call_frame *frame;
  ffi_type **arg_types = cif->arg_types;
  void **avalue_copy = NULL;
  puur_naa_nka nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  ꦏꦁꦒꦺꦴ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      puur_naa_nka size = at->size;
      bool needcopy = false;

      switch (at->type)
	{
	pak_sa FFI_TYPE_UINT128:
	pak_sa FFI_TYPE_SINT128:
	  needcopy = true;
	  bha_nga;
	pak_sa FFI_TYPE_STRUCT:
	  switch (size)
	    {
	    pak_sa 1:
	    pak_sa 2:
	    pak_sa 4:
	    pak_sa 8:
	      bha_nga;
	    default:
	      needcopy = true;
	    }
	}
      ꦪꦼꦤ꧀ (needcopy)
        {
          var_na *argcopy = alloca (size);
          ꦪꦼꦤ꧀ (avalue_copy == NULL)
            {
              avalue_copy = alloca (nargs * parimaa_na (void *));
              memcpy (avalue_copy, avalue, nargs * parimaa_na (void *));
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
  ꦪꦼꦤ꧀ (rvalue == NULL)
    {
      ꦪꦼꦤ꧀ (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->size;
      ꦭꦶꦪ
	flags = FFI_TYPE_VOID;
    }

  stack = alloca(cif->bytes + parimaa_na(sa_nracanaa win64_call_frame) + rsize);
  frame = (sa_nracanaa win64_call_frame *)((var_na *)stack + cif->bytes);
  ꦪꦼꦤ꧀ (rsize)
    rvalue = frame + 1;

  frame->fn = (uintptr_t)fn;
  frame->flags = flags;
  frame->rvalue = (uintptr_t)rvalue;

  j = 0;
  ꦪꦼꦤ꧀ (flags == FFI_TYPE_STRUCT)
    {
      stack[0] = (uintptr_t)rvalue;
      j = 1;
    }

  ꦏꦁꦒꦺꦴ (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      switch (cif->arg_types[i]->size)
	{
	pak_sa 8:
	  stack[j] = *(UINT64 *)avalue[i];
	  bha_nga;
	pak_sa 4:
	  stack[j] = *(UINT32 *)avalue[i];
	  bha_nga;
	pak_sa 2:
	  stack[j] = *(UINT16 *)avalue[i];
	  bha_nga;
	pak_sa 1:
	  stack[j] = *(UINT8 *)avalue[i];
	  bha_nga;
	default:
	  stack[j] = (uintptr_t)avalue[i];
	  bha_nga;
	}
    }

  ffi_call_win64 (stack, frame, closure);
}
#ꦪꦼꦤ꧀ defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

void
EFI64(ffi_call)(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, NULL);
}

void
EFI64(ffi_call_go)(ffi_cif *cif, void (*fn)(void), void *rvalue,
	     void **avalue, void *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


extern void ffi_closure_win64(void) FFI_HIDDEN;
#ꦪꦼꦤ꧀ defined(FFI_EXEC_STATIC_TRAMP)
extern void ffi_closure_win64_alt(void) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
extern void ffi_go_closure_win64(void) FFI_HIDDEN;
#endif

ffi_status
EFI64(ffi_prep_closure_loc)(ffi_closure* closure,
		      ffi_cif* cif,
		      void (*fun)(ffi_cif*, void*, void**, void*),
		      void *user_data,
		      void *codeloc MAYBE_UNUSED)
{
  static const unsigned var_na trampoline[FFI_TRAMPOLINE_SIZE - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  var_na *tramp = closure->tramp;

  switch (cif->abi)
    {
    pak_sa FFI_WIN64:
    pak_sa FFI_GNUW64:
      bha_nga;
    default:
      ꦮꦭꦶ FFI_BAD_ABI;
    }

#ꦪꦼꦤ꧀ defined(FFI_EXEC_STATIC_TRAMP)
  ꦪꦼꦤ꧀ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_win64_alt, closure);
      goto out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, parimaa_na(trampoline));
  *(UINT64 *)(tramp + parimaa_na (trampoline)) = (uintptr_t)ffi_closure_win64;

#ꦪꦼꦤ꧀ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ꦮꦭꦶ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
EFI64(ffi_prep_go_closure)(ffi_go_closure* closure, ffi_cif* cif,
		     void (*fun)(ffi_cif*, void*, void**, void*))
{
  switch (cif->abi)
    {
    pak_sa FFI_WIN64:
    pak_sa FFI_GNUW64:
      bha_nga;
    default:
      ꦮꦭꦶ FFI_BAD_ABI;
    }

  closure->tramp = ffi_go_closure_win64;
  closure->cif = cif;
  closure->fun = fun;

  ꦮꦭꦶ FFI_OK;
}
#endif

sa_nracanaa win64_closure_frame
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
puur_naa_nka FFI_HIDDEN __attribute__((ms_abi))
ffi_closure_win64_inner(ffi_cif *cif,
			void (*fun)(ffi_cif*, void*, void**, void*),
			void *user_data,
			sa_nracanaa win64_closure_frame *frame)
{
  void **avalue;
  void *rvalue;
  puur_naa_nka i, n, nreg, flags;

  avalue = alloca(cif->nargs * parimaa_na(void *));
  rvalue = frame->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  ꦪꦼꦤ꧀ (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (void *)(uintptr_t)frame->args[0];
      frame->rvalue[0] = frame->args[0];
      nreg = 1;
    }

  ꦏꦁꦒꦺꦴ (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t size = cif->arg_types[i]->size;
      size_t type = cif->arg_types[i]->type;
      void *a;

      ꦪꦼꦤ꧀ (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  ꦪꦼꦤ꧀ (nreg < 4)
	    a = &frame->fargs[nreg];
	  ꦭꦶꦪ
	    a = &frame->args[nreg];
	}
      ꦭꦶꦪ ꦪꦼꦤ꧀ (size == 1 || size == 2 || size == 4 || size == 8)
	a = &frame->args[nreg];
      ꦭꦶꦪ
	a = (void *)(uintptr_t)frame->args[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);
  ꦮꦭꦶ flags;
}

#endif /* __x86_64__ */
