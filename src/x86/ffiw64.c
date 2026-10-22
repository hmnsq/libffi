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

#اگر defined(__x86_64__) || defined(_M_AMD64)
#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdlib.h>
#شامل <stdint.h>
#شامل <stdbool.h>
#شامل <tramp.h>

#اگر_تعریف_شدہ X86_WIN64
#تعریف_کرو EFI64(name) name
#ورنہ
#تعریف_کرو EFI64(name) FFI_HIDDEN name##_efi64
#اگر_ختم

ساخت win64_call_چوکھٹا
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

خارجی خالی ffi_call_win64 (خالی *ڈھیر, ساخت win64_call_چوکھٹا *,
			    خالی *احاطہ) FFI_HIDDEN;

ffi_حالت FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  صحیح flags, n;

  انتخاب (cif->abi)
    {
    حالت FFI_WIN64:
    حالت FFI_GNUW64:
      توڑو;
    طےشدہ:
      واپس FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  انتخاب (flags)
    {
    طےشدہ:
      توڑو;
    حالت FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      اگر (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      توڑو;
    حالت FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    حالت FFI_TYPE_STRUCT:
      انتخاب (cif->rtype->سائز)
	{
	حالت 8:
	  flags = FFI_TYPE_UINT64;
	  توڑو;
	حالت 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  توڑو;
	حالت 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  توڑو;
	حالت 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  توڑو;
	}
      توڑو;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  اگر (n < 4)
    n = 4;
  cif->بائٹیں = n * 8;

  واپس FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#اگر defined(_MSC_VER)
#ہدایت runtime_checks("s", off)
#اگر_ختم
FFI_ASAN_NO_SANITIZE
ساکن خالی
ffi_call_int (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	      خالی **avalue, خالی *احاطہ)
{
  صحیح i, j, n, flags;
  UINT64 *ڈھیر;
  size_t rsize;
  ساخت win64_call_چوکھٹا *چوکھٹا;
  ffi_type **arg_types = cif->arg_types;
  خالی **avalue_copy = تہی;
  صحیح nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  برائے (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحیح سائز = at->سائز;
      bool needcopy = false;

      انتخاب (at->type)
	{
	حالت FFI_TYPE_UINT128:
	حالت FFI_TYPE_SINT128:
	  needcopy = true;
	  توڑو;
	حالت FFI_TYPE_STRUCT:
	  انتخاب (سائز)
	    {
	    حالت 1:
	    حالت 2:
	    حالت 4:
	    حالت 8:
	      توڑو;
	    طےشدہ:
	      needcopy = true;
	    }
	}
      اگر (needcopy)
        {
          حرف *argcopy = alloca (سائز);
          اگر (avalue_copy == تہی)
            {
              avalue_copy = alloca (nargs * حجم (خالی *));
              حافظہ_نقل (avalue_copy, avalue, nargs * حجم (خالی *));
              avalue = avalue_copy;
            }
          حافظہ_نقل (argcopy, avalue[i], سائز);
          avalue[i] = argcopy;
        }
    }

  flags = cif->flags;
  rsize = 0;

  /* If we have no return value for a structure, we need to create one.
     Otherwise we can ignore the return type entirely.  */
  اگر (rvalue == تہی)
    {
      اگر (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->سائز;
      ورنہ
	flags = FFI_TYPE_VOID;
    }

  ڈھیر = alloca(cif->بائٹیں + حجم(ساخت win64_call_چوکھٹا) + rsize);
  چوکھٹا = (ساخت win64_call_چوکھٹا *)((حرف *)ڈھیر + cif->بائٹیں);
  اگر (rsize)
    rvalue = چوکھٹا + 1;

  چوکھٹا->fn = (uintptr_t)fn;
  چوکھٹا->flags = flags;
  چوکھٹا->rvalue = (uintptr_t)rvalue;

  j = 0;
  اگر (flags == FFI_TYPE_STRUCT)
    {
      ڈھیر[0] = (uintptr_t)rvalue;
      j = 1;
    }

  برائے (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      انتخاب (cif->arg_types[i]->سائز)
	{
	حالت 8:
	  ڈھیر[j] = *(UINT64 *)avalue[i];
	  توڑو;
	حالت 4:
	  ڈھیر[j] = *(UINT32 *)avalue[i];
	  توڑو;
	حالت 2:
	  ڈھیر[j] = *(UINT16 *)avalue[i];
	  توڑو;
	حالت 1:
	  ڈھیر[j] = *(UINT8 *)avalue[i];
	  توڑو;
	طےشدہ:
	  ڈھیر[j] = (uintptr_t)avalue[i];
	  توڑو;
	}
    }

  ffi_call_win64 (ڈھیر, چوکھٹا, احاطہ);
}
#اگر defined(_MSC_VER)
#ہدایت runtime_checks("s", restore)
#اگر_ختم

خالی
EFI64(ffi_call)(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, تہی);
}

خالی
EFI64(ffi_call_go)(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  ffi_call_int (cif, fn, rvalue, avalue, احاطہ);
}


خارجی خالی ffi_احاطہ_win64(خالی) FFI_HIDDEN;
#اگر defined(FFI_EXEC_STATIC_TRAMP)
خارجی خالی ffi_احاطہ_win64_alt(خالی) FFI_HIDDEN;
#اگر_ختم

#اگر_تعریف_شدہ FFI_GO_CLOSURES
خارجی خالی ffi_go_احاطہ_win64(خالی) FFI_HIDDEN;
#اگر_ختم

ffi_حالت
EFI64(ffi_prep_احاطہ_loc)(ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc MAYBE_غیر_مستعمل)
{
  ساکن ثابت غیرعلامتی حرف trampoline[FFI_TRAMPOLINE_سائز - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  حرف *tramp = احاطہ->tramp;

  انتخاب (cif->abi)
    {
    حالت FFI_WIN64:
    حالت FFI_GNUW64:
      توڑو;
    طےشدہ:
      واپس FFI_BAD_ABI;
    }

#اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(احاطہ))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_سیٹ_parms (احاطہ->ftramp, ffi_احاطہ_win64_alt, احاطہ);
      جاؤ برآمد;
    }
#اگر_ختم

  /* Initialize the dynamic trampoline. */
  حافظہ_نقل (tramp, trampoline, حجم(trampoline));
  *(UINT64 *)(tramp + حجم (trampoline)) = (uintptr_t)ffi_احاطہ_win64;

#اگر defined(FFI_EXEC_STATIC_TRAMP)
برآمد:
#اگر_ختم
  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  واپس FFI_OK;
}

#اگر_تعریف_شدہ FFI_GO_CLOSURES
ffi_حالت
EFI64(ffi_prep_go_احاطہ)(ffi_go_احاطہ* احاطہ, ffi_cif* cif,
		     خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*))
{
  انتخاب (cif->abi)
    {
    حالت FFI_WIN64:
    حالت FFI_GNUW64:
      توڑو;
    طےشدہ:
      واپس FFI_BAD_ABI;
    }

  احاطہ->tramp = ffi_go_احاطہ_win64;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}
#اگر_ختم

ساخت win64_احاطہ_چوکھٹا
{
  UINT64 rvalue[2];
  UINT64 fargs[4];
  UINT64 retaddr;
  UINT64 دلائل[];
};

/* Force the inner function to use the MS ABI.  When compiling on win64
   this is a nop.  When compiling on unix, this simplifies the assembly,
   and places the burden of saving the extra call-saved registers on
   the compiler.  */
صحیح FFI_HIDDEN __attribute__((ms_abi))
ffi_احاطہ_win64_inner(ffi_cif *cif,
			خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
			خالی *user_ڈیٹا,
			ساخت win64_احاطہ_چوکھٹا *چوکھٹا)
{
  خالی **avalue;
  خالی *rvalue;
  صحیح i, n, nreg, flags;

  avalue = alloca(cif->nargs * حجم(خالی *));
  rvalue = چوکھٹا->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  اگر (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (خالی *)(uintptr_t)چوکھٹا->دلائل[0];
      چوکھٹا->rvalue[0] = چوکھٹا->دلائل[0];
      nreg = 1;
    }

  برائے (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t سائز = cif->arg_types[i]->سائز;
      size_t type = cif->arg_types[i]->type;
      خالی *a;

      اگر (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  اگر (nreg < 4)
	    a = &چوکھٹا->fargs[nreg];
	  ورنہ
	    a = &چوکھٹا->دلائل[nreg];
	}
      ورنہ اگر (سائز == 1 || سائز == 2 || سائز == 4 || سائز == 8)
	a = &چوکھٹا->دلائل[nreg];
      ورنہ
	a = (خالی *)(uintptr_t)چوکھٹا->دلائل[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ڈیٹا);
  واپس flags;
}

#اگر_ختم /* __x86_64__ */
