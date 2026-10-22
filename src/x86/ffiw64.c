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

#إذا defined(__x86_64__) || defined(_M_AMD64)
#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <stdlib.h>
#ضمن <stdint.h>
#ضمن <stdbool.h>
#ضمن <tramp.h>

#جے_تعریف_شدہ X86_WIN64
#تعریف_کرو EFI64(name) name
#وإلا
#تعریف_کرو EFI64(name) FFI_HIDDEN name##_efi64
#جے_انت

بنية win64_call_چوکھٹا
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

خارجي فراغ ffi_call_win64 (فراغ *ڈھیر, بنية win64_call_چوکھٹا *,
			    فراغ *گھیرا) FFI_HIDDEN;

ffi_حالت FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  صحيح flags, n;

  بدل (cif->abi)
    {
    حالة FFI_WIN64:
    حالة FFI_GNUW64:
      اكسر;
    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  بدل (flags)
    {
    افتراضي:
      اكسر;
    حالة FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      إذا (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      اكسر;
    حالة FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    حالة FFI_TYPE_STRUCT:
      بدل (cif->rtype->ناپ)
	{
	حالة 8:
	  flags = FFI_TYPE_UINT64;
	  اكسر;
	حالة 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  اكسر;
	حالة 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  اكسر;
	حالة 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  اكسر;
	}
      اكسر;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  إذا (n < 4)
    n = 4;
  cif->بائٹاں = n * 8;

  أرجع FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#إذا defined(_MSC_VER)
#ہدایت runtime_checks("s", off)
#جے_انت
FFI_ASAN_NO_SANITIZE
ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	      فراغ **avalue, فراغ *گھیرا)
{
  صحيح i, j, n, flags;
  UINT64 *ڈھیر;
  size_t rsize;
  بنية win64_call_چوکھٹا *چوکھٹا;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue_copy = عدم;
  صحيح nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحيح ناپ = at->ناپ;
      bool needcopy = false;

      بدل (at->type)
	{
	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_SINT128:
	  needcopy = true;
	  اكسر;
	حالة FFI_TYPE_STRUCT:
	  بدل (ناپ)
	    {
	    حالة 1:
	    حالة 2:
	    حالة 4:
	    حالة 8:
	      اكسر;
	    افتراضي:
	      needcopy = true;
	    }
	}
      إذا (needcopy)
        {
          حرف *argcopy = alloca (ناپ);
          إذا (avalue_copy == عدم)
            {
              avalue_copy = alloca (nargs * حجم (فراغ *));
              حافظہ_نقل (avalue_copy, avalue, nargs * حجم (فراغ *));
              avalue = avalue_copy;
            }
          حافظہ_نقل (argcopy, avalue[i], ناپ);
          avalue[i] = argcopy;
        }
    }

  flags = cif->flags;
  rsize = 0;

  /* If we have no return value for a structure, we need to create one.
     Otherwise we can ignore the return type entirely.  */
  إذا (rvalue == عدم)
    {
      إذا (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->ناپ;
      وإلا
	flags = FFI_TYPE_VOID;
    }

  ڈھیر = alloca(cif->بائٹاں + حجم(بنية win64_call_چوکھٹا) + rsize);
  چوکھٹا = (بنية win64_call_چوکھٹا *)((حرف *)ڈھیر + cif->بائٹاں);
  إذا (rsize)
    rvalue = چوکھٹا + 1;

  چوکھٹا->fn = (uintptr_t)fn;
  چوکھٹا->flags = flags;
  چوکھٹا->rvalue = (uintptr_t)rvalue;

  j = 0;
  إذا (flags == FFI_TYPE_STRUCT)
    {
      ڈھیر[0] = (uintptr_t)rvalue;
      j = 1;
    }

  لكل (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      بدل (cif->arg_types[i]->ناپ)
	{
	حالة 8:
	  ڈھیر[j] = *(UINT64 *)avalue[i];
	  اكسر;
	حالة 4:
	  ڈھیر[j] = *(UINT32 *)avalue[i];
	  اكسر;
	حالة 2:
	  ڈھیر[j] = *(UINT16 *)avalue[i];
	  اكسر;
	حالة 1:
	  ڈھیر[j] = *(UINT8 *)avalue[i];
	  اكسر;
	افتراضي:
	  ڈھیر[j] = (uintptr_t)avalue[i];
	  اكسر;
	}
    }

  ffi_call_win64 (ڈھیر, چوکھٹا, گھیرا);
}
#إذا defined(_MSC_VER)
#ہدایت runtime_checks("s", restore)
#جے_انت

فراغ
EFI64(ffi_call)(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

فراغ
EFI64(ffi_call_go)(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گھیرا)
{
  ffi_call_int (cif, fn, rvalue, avalue, گھیرا);
}


خارجي فراغ ffi_گھیرا_win64(فراغ) FFI_HIDDEN;
#إذا defined(FFI_EXEC_STATIC_TRAMP)
خارجي فراغ ffi_گھیرا_win64_alt(فراغ) FFI_HIDDEN;
#جے_انت

#جے_تعریف_شدہ FFI_GO_CLOSURES
خارجي فراغ ffi_go_گھیرا_win64(فراغ) FFI_HIDDEN;
#جے_انت

ffi_حالت
EFI64(ffi_prep_گھیرا_loc)(ffi_گھیرا* گھیرا,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_انکڑے,
		      فراغ *codeloc MAYBE_بن_ورتیا)
{
  ساكن ثابت بنانشانی حرف trampoline[FFI_TRAMPOLINE_ناپ - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  حرف *tramp = گھیرا->tramp;

  بدل (cif->abi)
    {
    حالة FFI_WIN64:
    حالة FFI_GNUW64:
      اكسر;
    افتراضي:
      أرجع FFI_BAD_ABI;
    }

#إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(گھیرا))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_سیٹ_parms (گھیرا->ftramp, ffi_گھیرا_win64_alt, گھیرا);
      اذهب باہرلا;
    }
#جے_انت

  /* Initialize the dynamic trampoline. */
  حافظہ_نقل (tramp, trampoline, حجم(trampoline));
  *(UINT64 *)(tramp + حجم (trampoline)) = (uintptr_t)ffi_گھیرا_win64;

#إذا defined(FFI_EXEC_STATIC_TRAMP)
باہرلا:
#جے_انت
  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;

  أرجع FFI_OK;
}

#جے_تعریف_شدہ FFI_GO_CLOSURES
ffi_حالت
EFI64(ffi_prep_go_گھیرا)(ffi_go_گھیرا* گھیرا, ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*))
{
  بدل (cif->abi)
    {
    حالة FFI_WIN64:
    حالة FFI_GNUW64:
      اكسر;
    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  گھیرا->tramp = ffi_go_گھیرا_win64;
  گھیرا->cif = cif;
  گھیرا->fun = fun;

  أرجع FFI_OK;
}
#جے_انت

بنية win64_گھیرا_چوکھٹا
{
  UINT64 rvalue[2];
  UINT64 fargs[4];
  UINT64 retaddr;
  UINT64 دلیلاں[];
};

/* Force the inner function to use the MS ABI.  When compiling on win64
   this is a nop.  When compiling on unix, this simplifies the assembly,
   and places the burden of saving the extra call-saved registers on
   the compiler.  */
صحيح FFI_HIDDEN __attribute__((ms_abi))
ffi_گھیرا_win64_inner(ffi_cif *cif,
			فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
			فراغ *user_انکڑے,
			بنية win64_گھیرا_چوکھٹا *چوکھٹا)
{
  فراغ **avalue;
  فراغ *rvalue;
  صحيح i, n, nreg, flags;

  avalue = alloca(cif->nargs * حجم(فراغ *));
  rvalue = چوکھٹا->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  إذا (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (فراغ *)(uintptr_t)چوکھٹا->دلیلاں[0];
      چوکھٹا->rvalue[0] = چوکھٹا->دلیلاں[0];
      nreg = 1;
    }

  لكل (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t ناپ = cif->arg_types[i]->ناپ;
      size_t type = cif->arg_types[i]->type;
      فراغ *a;

      إذا (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  إذا (nreg < 4)
	    a = &چوکھٹا->fargs[nreg];
	  وإلا
	    a = &چوکھٹا->دلیلاں[nreg];
	}
      وإلا إذا (ناپ == 1 || ناپ == 2 || ناپ == 4 || ناپ == 8)
	a = &چوکھٹا->دلیلاں[nreg];
      وإلا
	a = (فراغ *)(uintptr_t)چوکھٹا->دلیلاں[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_انکڑے);
  أرجع flags;
}

#جے_انت /* __x86_64__ */
