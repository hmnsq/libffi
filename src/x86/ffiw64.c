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

#ఒకవేళ defined(__x86_64__) || defined(_M_AMD64)
#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <tramp.h>

#నిర్వచితమైతే X86_WIN64
#నిర్వచించు EFI64(name) name
#లేకపోతే
#నిర్వచించు EFI64(name) FFI_HIDDEN name##_efi64
#అయితే_ముగింపు

నిర్మాణం win64_call_చట్రం
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

బాహ్య శూన్య ffi_call_win64 (శూన్య *దొంతర, నిర్మాణం win64_call_చట్రం *,
			    శూన్య *సంవృతం) FFI_HIDDEN;

ffi_స్థితి FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  పూర్ణాంకం flags, n;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_WIN64:
    స్థితి FFI_GNUW64:
      ఆపు;
    అప్రమేయ:
      తిరిగి FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  ఎంపిక (flags)
    {
    అప్రమేయ:
      ఆపు;
    స్థితి FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      ఒకవేళ (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      ఆపు;
    స్థితి FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    స్థితి FFI_TYPE_STRUCT:
      ఎంపిక (cif->rtype->కొలత)
	{
	స్థితి 8:
	  flags = FFI_TYPE_UINT64;
	  ఆపు;
	స్థితి 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  ఆపు;
	స్థితి 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  ఆపు;
	స్థితి 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  ఆపు;
	}
      ఆపు;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  ఒకవేళ (n < 4)
    n = 4;
  cif->బైట్లు = n * 8;

  తిరిగి FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ఒకవేళ defined(_MSC_VER)
#సూచన runtime_checks("s", off)
#అయితే_ముగింపు
FFI_ASAN_NO_SANITIZE
స్థిర శూన్య
ffi_call_int (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	      శూన్య **avalue, శూన్య *సంవృతం)
{
  పూర్ణాంకం i, j, n, flags;
  UINT64 *దొంతర;
  size_t rsize;
  నిర్మాణం win64_call_చట్రం *చట్రం;
  ffi_type **arg_types = cif->arg_types;
  శూన్య **avalue_copy = ఖాళీ;
  పూర్ణాంకం nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  కొరకు (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      పూర్ణాంకం కొలత = at->కొలత;
      bool needcopy = false;

      ఎంపిక (at->type)
	{
	స్థితి FFI_TYPE_UINT128:
	స్థితి FFI_TYPE_SINT128:
	  needcopy = true;
	  ఆపు;
	స్థితి FFI_TYPE_STRUCT:
	  ఎంపిక (కొలత)
	    {
	    స్థితి 1:
	    స్థితి 2:
	    స్థితి 4:
	    స్థితి 8:
	      ఆపు;
	    అప్రమేయ:
	      needcopy = true;
	    }
	}
      ఒకవేళ (needcopy)
        {
          అక్షరం *argcopy = alloca (కొలత);
          ఒకవేళ (avalue_copy == ఖాళీ)
            {
              avalue_copy = alloca (nargs * పరిమాణం (శూన్య *));
              జ్ఞాపకం_నకలు (avalue_copy, avalue, nargs * పరిమాణం (శూన్య *));
              avalue = avalue_copy;
            }
          జ్ఞాపకం_నకలు (argcopy, avalue[i], కొలత);
          avalue[i] = argcopy;
        }
    }

  flags = cif->flags;
  rsize = 0;

  /* If we have no return value for a structure, we need to create one.
     Otherwise we can ignore the return type entirely.  */
  ఒకవేళ (rvalue == ఖాళీ)
    {
      ఒకవేళ (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->కొలత;
      లేకపోతే
	flags = FFI_TYPE_VOID;
    }

  దొంతర = alloca(cif->బైట్లు + పరిమాణం(నిర్మాణం win64_call_చట్రం) + rsize);
  చట్రం = (నిర్మాణం win64_call_చట్రం *)((అక్షరం *)దొంతర + cif->బైట్లు);
  ఒకవేళ (rsize)
    rvalue = చట్రం + 1;

  చట్రం->fn = (uintptr_t)fn;
  చట్రం->flags = flags;
  చట్రం->rvalue = (uintptr_t)rvalue;

  j = 0;
  ఒకవేళ (flags == FFI_TYPE_STRUCT)
    {
      దొంతర[0] = (uintptr_t)rvalue;
      j = 1;
    }

  కొరకు (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      ఎంపిక (cif->arg_types[i]->కొలత)
	{
	స్థితి 8:
	  దొంతర[j] = *(UINT64 *)avalue[i];
	  ఆపు;
	స్థితి 4:
	  దొంతర[j] = *(UINT32 *)avalue[i];
	  ఆపు;
	స్థితి 2:
	  దొంతర[j] = *(UINT16 *)avalue[i];
	  ఆపు;
	స్థితి 1:
	  దొంతర[j] = *(UINT8 *)avalue[i];
	  ఆపు;
	అప్రమేయ:
	  దొంతర[j] = (uintptr_t)avalue[i];
	  ఆపు;
	}
    }

  ffi_call_win64 (దొంతర, చట్రం, సంవృతం);
}
#ఒకవేళ defined(_MSC_VER)
#సూచన runtime_checks("s", restore)
#అయితే_ముగింపు

శూన్య
EFI64(ffi_call)(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ఖాళీ);
}

శూన్య
EFI64(ffi_call_go)(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
  ffi_call_int (cif, fn, rvalue, avalue, సంవృతం);
}


బాహ్య శూన్య ffi_సంవృతం_win64(శూన్య) FFI_HIDDEN;
#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
బాహ్య శూన్య ffi_సంవృతం_win64_alt(శూన్య) FFI_HIDDEN;
#అయితే_ముగింపు

#నిర్వచితమైతే FFI_GO_CLOSURES
బాహ్య శూన్య ffi_go_సంవృతం_win64(శూన్య) FFI_HIDDEN;
#అయితే_ముగింపు

ffi_స్థితి
EFI64(ffi_prep_సంవృతం_loc)(ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc MAYBE_వాడని)
{
  స్థిర స్థిరరాశి గుర్తులేని అక్షరం trampoline[FFI_TRAMPOLINE_కొలత - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  అక్షరం *tramp = సంవృతం->tramp;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_WIN64:
    స్థితి FFI_GNUW64:
      ఆపు;
    అప్రమేయ:
      తిరిగి FFI_BAD_ABI;
    }

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
  ఒకవేళ (ffi_tramp_is_present(సంవృతం))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_సమితి_parms (సంవృతం->ftramp, ffi_సంవృతం_win64_alt, సంవృతం);
      వెళ్ళు బహిర్గతం;
    }
#అయితే_ముగింపు

  /* Initialize the dynamic trampoline. */
  జ్ఞాపకం_నకలు (tramp, trampoline, పరిమాణం(trampoline));
  *(UINT64 *)(tramp + పరిమాణం (trampoline)) = (uintptr_t)ffi_సంవృతం_win64;

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
బహిర్గతం:
#అయితే_ముగింపు
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}

#నిర్వచితమైతే FFI_GO_CLOSURES
ffi_స్థితి
EFI64(ffi_prep_go_సంవృతం)(ffi_go_సంవృతం* సంవృతం, ffi_cif* cif,
		     శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*))
{
  ఎంపిక (cif->abi)
    {
    స్థితి FFI_WIN64:
    స్థితి FFI_GNUW64:
      ఆపు;
    అప్రమేయ:
      తిరిగి FFI_BAD_ABI;
    }

  సంవృతం->tramp = ffi_go_సంవృతం_win64;
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}
#అయితే_ముగింపు

నిర్మాణం win64_సంవృతం_చట్రం
{
  UINT64 rvalue[2];
  UINT64 fargs[4];
  UINT64 retaddr;
  UINT64 వాదనలు[];
};

/* Force the inner function to use the MS ABI.  When compiling on win64
   this is a nop.  When compiling on unix, this simplifies the assembly,
   and places the burden of saving the extra call-saved registers on
   the compiler.  */
పూర్ణాంకం FFI_HIDDEN __attribute__((ms_abi))
ffi_సంవృతం_win64_inner(ffi_cif *cif,
			శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
			శూన్య *user_దత్తాంశం,
			నిర్మాణం win64_సంవృతం_చట్రం *చట్రం)
{
  శూన్య **avalue;
  శూన్య *rvalue;
  పూర్ణాంకం i, n, nreg, flags;

  avalue = alloca(cif->nargs * పరిమాణం(శూన్య *));
  rvalue = చట్రం->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  ఒకవేళ (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (శూన్య *)(uintptr_t)చట్రం->వాదనలు[0];
      చట్రం->rvalue[0] = చట్రం->వాదనలు[0];
      nreg = 1;
    }

  కొరకు (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      size_t కొలత = cif->arg_types[i]->కొలత;
      size_t type = cif->arg_types[i]->type;
      శూన్య *a;

      ఒకవేళ (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  ఒకవేళ (nreg < 4)
	    a = &చట్రం->fargs[nreg];
	  లేకపోతే
	    a = &చట్రం->వాదనలు[nreg];
	}
      లేకపోతే ఒకవేళ (కొలత == 1 || కొలత == 2 || కొలత == 4 || కొలత == 8)
	a = &చట్రం->వాదనలు[nreg];
      లేకపోతే
	a = (శూన్య *)(uintptr_t)చట్రం->వాదనలు[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_దత్తాంశం);
  తిరిగి flags;
}

#అయితే_ముగింపు /* __x86_64__ */
