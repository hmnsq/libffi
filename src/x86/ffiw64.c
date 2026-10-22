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

#अगर defined(__x86_64__) || defined(_M_AMD64)
#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश <stdlib.h>
#समावेश <stdint.h>
#समावेश <stdbool.h>
#समावेश <tramp.h>

#अगर_घोषित X86_WIN64
#घोषणा EFI64(name) name
#अन्यथा
#घोषणा EFI64(name) FFI_HIDDEN name##_efi64
#पूर्ण_अगर

काष्ठा win64_call_frame
{
  UINT64 rbp;		/* 0 */
  UINT64 retaddr;	/* 8 */
  UINT64 fn;		/* 16 */
  UINT64 flags;		/* 24 */
  UINT64 rvalue;	/* 32 */
};

बाह्य व्योम ffi_call_win64 (व्योम *ढेर, काष्ठा win64_call_frame *,
			    व्योम *closure) FFI_HIDDEN;

ffi_status FFI_HIDDEN
EFI64(ffi_prep_cif_machdep)(ffi_cif *cif)
{
  पूर्णांक flags, n;

  चयन (cif->abi)
    {
    हाल FFI_WIN64:
    हाल FFI_GNUW64:
      अवरोध;
    शेष:
      वापस FFI_BAD_ABI;
    }

  flags = cif->rtype->type;
  चयन (flags)
    {
    शेष:
      अवरोध;
    हाल FFI_TYPE_LONGDOUBLE:
      /* GCC returns long double values by reference, like a struct */
      अगर (cif->abi == FFI_GNUW64)
	flags = FFI_TYPE_STRUCT;
      अवरोध;
    हाल FFI_TYPE_COMPLEX:
      flags = FFI_TYPE_STRUCT;
      /* FALLTHRU */
    हाल FFI_TYPE_STRUCT:
      चयन (cif->rtype->आकार)
	{
	हाल 8:
	  flags = FFI_TYPE_UINT64;
	  अवरोध;
	हाल 4:
	  flags = FFI_TYPE_SMALL_STRUCT_4B;
	  अवरोध;
	हाल 2:
	  flags = FFI_TYPE_SMALL_STRUCT_2B;
	  अवरोध;
	हाल 1:
	  flags = FFI_TYPE_SMALL_STRUCT_1B;
	  अवरोध;
	}
      अवरोध;
    }
  cif->flags = flags;

  /* Each argument either fits in a register, an 8 byte slot, or is
     passed by reference with the pointer in the 8 byte slot.  */
  n = cif->nargs;
  n += (flags == FFI_TYPE_STRUCT);
  अगर (n < 4)
    n = 4;
  cif->bytes = n * 8;

  वापस FFI_OK;
}

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_win64() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#अगर defined(_MSC_VER)
#आशय runtime_checks("s", off)
#पूर्ण_अगर
FFI_ASAN_NO_SANITIZE
अटल व्योम
ffi_call_int (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
	      व्योम **avalue, व्योम *closure)
{
  पूर्णांक i, j, n, flags;
  UINT64 *ढेर;
  माप_प्रकार rsize;
  काष्ठा win64_call_frame *frame;
  ffi_type **arg_types = cif->arg_types;
  व्योम **avalue_copy = शून्य;
  पूर्णांक nargs = cif->nargs;

  FFI_ASSERT(cif->abi == FFI_GNUW64 || cif->abi == FFI_WIN64);

  /* If we have any int128 or irregularly sized structure arguments,
     make a copy so we are passing by value.  The pointer array is cloned
     first: the caller owns avalue[] and may reuse it for another call,
     so it must not be modified.  */
  क्रम (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णांक आकार = at->आकार;
      bool needcopy = false;

      चयन (at->type)
	{
	हाल FFI_TYPE_UINT128:
	हाल FFI_TYPE_SINT128:
	  needcopy = true;
	  अवरोध;
	हाल FFI_TYPE_STRUCT:
	  चयन (आकार)
	    {
	    हाल 1:
	    हाल 2:
	    हाल 4:
	    हाल 8:
	      अवरोध;
	    शेष:
	      needcopy = true;
	    }
	}
      अगर (needcopy)
        {
          अक्षर *argcopy = alloca (आकार);
          अगर (avalue_copy == शून्य)
            {
              avalue_copy = alloca (nargs * माप (व्योम *));
              स_नकल (avalue_copy, avalue, nargs * माप (व्योम *));
              avalue = avalue_copy;
            }
          स_नकल (argcopy, avalue[i], आकार);
          avalue[i] = argcopy;
        }
    }

  flags = cif->flags;
  rsize = 0;

  /* If we have no return value for a structure, we need to create one.
     Otherwise we can ignore the return type entirely.  */
  अगर (rvalue == शून्य)
    {
      अगर (flags == FFI_TYPE_STRUCT)
	rsize = cif->rtype->आकार;
      अन्यथा
	flags = FFI_TYPE_VOID;
    }

  ढेर = alloca(cif->bytes + माप(काष्ठा win64_call_frame) + rsize);
  frame = (काष्ठा win64_call_frame *)((अक्षर *)ढेर + cif->bytes);
  अगर (rsize)
    rvalue = frame + 1;

  frame->fn = (uintptr_t)fn;
  frame->flags = flags;
  frame->rvalue = (uintptr_t)rvalue;

  j = 0;
  अगर (flags == FFI_TYPE_STRUCT)
    {
      ढेर[0] = (uintptr_t)rvalue;
      j = 1;
    }

  क्रम (i = 0, n = cif->nargs; i < n; ++i, ++j)
    {
      चयन (cif->arg_types[i]->आकार)
	{
	हाल 8:
	  ढेर[j] = *(UINT64 *)avalue[i];
	  अवरोध;
	हाल 4:
	  ढेर[j] = *(UINT32 *)avalue[i];
	  अवरोध;
	हाल 2:
	  ढेर[j] = *(UINT16 *)avalue[i];
	  अवरोध;
	हाल 1:
	  ढेर[j] = *(UINT8 *)avalue[i];
	  अवरोध;
	शेष:
	  ढेर[j] = (uintptr_t)avalue[i];
	  अवरोध;
	}
    }

  ffi_call_win64 (ढेर, frame, closure);
}
#अगर defined(_MSC_VER)
#आशय runtime_checks("s", restore)
#पूर्ण_अगर

व्योम
EFI64(ffi_call)(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, शून्य);
}

व्योम
EFI64(ffi_call_go)(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
	     व्योम **avalue, व्योम *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


बाह्य व्योम ffi_closure_win64(व्योम) FFI_HIDDEN;
#अगर defined(FFI_EXEC_STATIC_TRAMP)
बाह्य व्योम ffi_closure_win64_alt(व्योम) FFI_HIDDEN;
#पूर्ण_अगर

#अगर_घोषित FFI_GO_CLOSURES
बाह्य व्योम ffi_go_closure_win64(व्योम) FFI_HIDDEN;
#पूर्ण_अगर

ffi_status
EFI64(ffi_prep_closure_loc)(ffi_closure* closure,
		      ffi_cif* cif,
		      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc MAYBE_UNUSED)
{
  अटल स्थिर अचिन्हित अक्षर trampoline[FFI_TRAMPOLINE_आकार - 8] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  अक्षर *tramp = closure->tramp;

  चयन (cif->abi)
    {
    हाल FFI_WIN64:
    हाल FFI_GNUW64:
      अवरोध;
    शेष:
      वापस FFI_BAD_ABI;
    }

#अगर defined(FFI_EXEC_STATIC_TRAMP)
  अगर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_win64_alt, closure);
      जाओ out;
    }
#पूर्ण_अगर

  /* Initialize the dynamic trampoline. */
  स_नकल (tramp, trampoline, माप(trampoline));
  *(UINT64 *)(tramp + माप (trampoline)) = (uintptr_t)ffi_closure_win64;

#अगर defined(FFI_EXEC_STATIC_TRAMP)
out:
#पूर्ण_अगर
  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  वापस FFI_OK;
}

#अगर_घोषित FFI_GO_CLOSURES
ffi_status
EFI64(ffi_prep_go_closure)(ffi_go_closure* closure, ffi_cif* cif,
		     व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*))
{
  चयन (cif->abi)
    {
    हाल FFI_WIN64:
    हाल FFI_GNUW64:
      अवरोध;
    शेष:
      वापस FFI_BAD_ABI;
    }

  closure->tramp = ffi_go_closure_win64;
  closure->cif = cif;
  closure->fun = fun;

  वापस FFI_OK;
}
#पूर्ण_अगर

काष्ठा win64_closure_frame
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
पूर्णांक FFI_HIDDEN __attribute__((ms_abi))
ffi_closure_win64_inner(ffi_cif *cif,
			व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
			व्योम *user_तथ्य,
			काष्ठा win64_closure_frame *frame)
{
  व्योम **avalue;
  व्योम *rvalue;
  पूर्णांक i, n, nreg, flags;

  avalue = alloca(cif->nargs * माप(व्योम *));
  rvalue = frame->rvalue;
  nreg = 0;

  /* When returning a structure, the address is in the first argument.
     We must also be prepared to return the same address in eax, so
     install that address in the frame and pretend we return a pointer.  */
  flags = cif->flags;
  अगर (flags == FFI_TYPE_STRUCT)
    {
      rvalue = (व्योम *)(uintptr_t)frame->args[0];
      frame->rvalue[0] = frame->args[0];
      nreg = 1;
    }

  क्रम (i = 0, n = cif->nargs; i < n; ++i, ++nreg)
    {
      माप_प्रकार आकार = cif->arg_types[i]->आकार;
      माप_प्रकार type = cif->arg_types[i]->type;
      व्योम *a;

      अगर (type == FFI_TYPE_DOUBLE || type == FFI_TYPE_FLOAT)
	{
	  अगर (nreg < 4)
	    a = &frame->fargs[nreg];
	  अन्यथा
	    a = &frame->args[nreg];
	}
      अन्यथा अगर (आकार == 1 || आकार == 2 || आकार == 4 || आकार == 8)
	a = &frame->args[nreg];
      अन्यथा
	a = (व्योम *)(uintptr_t)frame->args[nreg];

      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_तथ्य);
  वापस flags;
}

#पूर्ण_अगर /* __x86_64__ */
