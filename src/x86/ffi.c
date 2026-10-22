/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2017, 2022, 2026  Anthony Green
           Copyright (c) 1996, 1998, 1999, 2001, 2007, 2008  Red Hat, Inc.
           Copyright (c) 2002  Ranjit Mathew
           Copyright (c) 2002  Bo Thorsen
           Copyright (c) 2002  Roger Sayle
           Copyright (C) 2008, 2010  Free Software Foundation, Inc.

   x86 Foreign Function Interface

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

#जइ defined(__i386__) || defined(_M_IX86)
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <tramp.h>
#अन्तर्भाव "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# जइ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अण्णहा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#जइ defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#endif

#जइ defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#define राशिः_ALIGN(bytes) (bytes)
#अण्णहा
#define राशिः_ALIGN(bytes) FFI_ALIGN (bytes, 16)
#endif

/* Perform machine dependent cif processing.  */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  पूर्णाङ्क i, n, flags, cabi = cif->abi;

  विकल्प (cabi)
    {
    पक्ष FFI_SYSV:
    पक्ष FFI_STDCALL:
    पक्ष FFI_THISCALL:
    पक्ष FFI_FASTCALL:
    पक्ष FFI_MS_CDECL:
    पक्ष FFI_PASCAL:
    पक्ष FFI_REGISTER:
      भङ्ग;
    स्वभाव:
      प्रत्यागम FFI_BAD_ABI;
    }

  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      भङ्ग;
    पक्ष FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      भङ्ग;
    पक्ष FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      भङ्ग;
    पक्ष FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      भङ्ग;
    पक्ष FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      भङ्ग;
    पक्ष FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      भङ्ग;
    पक्ष FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      भङ्ग;
    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_UINT32:
    पक्ष FFI_TYPE_POINTER:
      flags = X86_RET_INT32;
      भङ्ग;
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      भङ्ग;
    पक्ष FFI_TYPE_STRUCT:
      {
#जइ defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t size = cif->rtype->size;
        जइ (size == 1)
          flags = X86_RET_STRUCT_1B;
        अण्णहा जइ (size == 2)
          flags = X86_RET_STRUCT_2B;
        अण्णहा जइ (size == 4)
          flags = X86_RET_INT32;
        अण्णहा जइ (size == 8)
          flags = X86_RET_INT64;
        अण्णहा
#endif
          {
          do_struct:
            विकल्प (cabi)
              {
              पक्ष FFI_THISCALL:
              पक्ष FFI_FASTCALL:
              पक्ष FFI_STDCALL:
              पक्ष FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                भङ्ग;
              स्वभाव:
                flags = X86_RET_STRUCTPOP;
                भङ्ग;
              }
            /* Allocate space for return value pointer.  */
            bytes += FFI_ALIGN (परिमाण(शून्य*), FFI_SIZEOF_ARG);
          }
      }
      भङ्ग;
    पक्ष FFI_TYPE_COMPLEX:
      विकल्प (cif->rtype->elements[0]->type)
	{
	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_LONGDOUBLE:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
	  गच्छ do_struct;
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  भङ्ग;
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  भङ्ग;
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  भङ्ग;
	स्वभाव:
	  प्रत्यागम FFI_BAD_TYPEDEF;
	}
      भङ्ग;
    स्वभाव:
      प्रत्यागम FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  प्रति (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#जइ defined(X86_WIN32)
      जइ (cabi == FFI_STDCALL)
        bytes = FFI_ALIGN (bytes, FFI_SIZEOF_ARG);
      अण्णहा
#endif
        bytes = FFI_ALIGN (bytes, t->alignment);
      bytes += FFI_ALIGN (t->size, FFI_SIZEOF_ARG);
    }
  cif->bytes = bytes;

  प्रत्यागम FFI_OK;
}

स्थिर ffi_arg
extend_basic_type(शून्य *arg, पूर्णाङ्क type)
{
  विकल्प (type)
    {
    पक्ष FFI_TYPE_SINT8:
      प्रत्यागम *(SINT8 *)arg;
    पक्ष FFI_TYPE_UINT8:
      प्रत्यागम *(UINT8 *)arg;
    पक्ष FFI_TYPE_SINT16:
      प्रत्यागम *(SINT16 *)arg;
    पक्ष FFI_TYPE_UINT16:
      प्रत्यागम *(UINT16 *)arg;

    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_UINT32:
    पक्ष FFI_TYPE_POINTER:
    पक्ष FFI_TYPE_FLOAT:
      प्रत्यागम *(UINT32 *)arg;

    स्वभाव:
      abort();
    }
}

संरचना call_frame
{
  शून्य *ebp;		/* 0 */
  शून्य *retaddr;	/* 4 */
  शून्य (*fn)(शून्य);	/* 8 */
  पूर्णाङ्क flags;		/* 12 */
  शून्य *rvalue;		/* 16 */
  unsigned regs[3];	/* 20-28 */
};

संरचना abi_params
{
  पूर्णाङ्क dir;		/* parameter growth direction */
  पूर्णाङ्क static_chain;	/* the static chain register used by gcc */
  पूर्णाङ्क nregs;		/* number of register parameters */
  पूर्णाङ्क regs[3];
};

स्थिर ध्रुव संरचना abi_params abi_params[FFI_LAST_ABI] = {
  [FFI_SYSV] = { 1, R_ECX, 0 },
  [FFI_THISCALL] = { 1, R_EAX, 1, { R_ECX } },
  [FFI_FASTCALL] = { 1, R_EAX, 2, { R_ECX, R_EDX } },
  [FFI_STDCALL] = { 1, R_ECX, 0 },
  [FFI_PASCAL] = { -1, R_ECX, 0 },
  /* ??? No defined static chain; gcc does not support REGISTER.  */
  [FFI_REGISTER] = { -1, R_ECX, 3, { R_EAX, R_EDX, R_ECX } },
  [FFI_MS_CDECL] = { 1, R_ECX, 0 }
};

#ifdef HAVE_FASTCALL
  #ifdef _MSC_VER
    #define FFI_DECLARE_FASTCALL __fastcall
  #अण्णहा
    #define FFI_DECLARE_FASTCALL __declspec(fastcall)
  #endif
#अण्णहा
  #define FFI_DECLARE_FASTCALL
#endif

बाह्य शून्य FFI_DECLARE_FASTCALL ffi_call_i386(संरचना call_frame *, वर्ण *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#जइ defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
{
  size_t rsize, bytes;
  संरचना call_frame *frame;
  वर्ण *राशिः, *argp;
  ffi_type **arg_types;
  पूर्णाङ्क flags, cabi, i, n, dir, narg_reg;
  ध्रुव संरचना abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  जइ (rvalue == रिक्त)
    {
      विकल्प (flags)
	{
	पक्ष X86_RET_FLOAT:
	पक्ष X86_RET_DOUBLE:
	पक्ष X86_RET_LDOUBLE:
	पक्ष X86_RET_STRUCTPOP:
	पक्ष X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  भङ्ग;
	स्वभाव:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  भङ्ग;
	}
    }

  bytes = राशिः_ALIGN (cif->bytes);
  राशिः = alloca(bytes + परिमाण(*frame) + rsize);
  argp = (dir < 0 ? राशिः + bytes : राशिः);
  frame = (संरचना call_frame *)(राशिः + bytes);
  जइ (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;
  frame->regs[pabi->static_chain] = (unsigned)closure;

  narg_reg = 0;
  विकल्प (flags)
    {
    पक्ष X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      जइ (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  भङ्ग;
	}
      /* fallthru */
    पक्ष X86_RET_STRUCTPOP:
      *(शून्य **)argp = rvalue;
      argp += परिमाण(शून्य *);
      भङ्ग;
    }

  arg_types = cif->arg_types;
  प्रति (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      शून्य *valp = avalue[i];
      size_t z = ty->size;
      पूर्णाङ्क t = ty->type;

      जइ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  जइ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    frame->regs[pabi->regs[narg_reg++]] = val;
	  अण्णहा जइ (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  अण्णहा
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      अण्णहा
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  जइ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  /* Alignment rules for arguments are quite complex.  Vectors and
	     structures with 16 byte alignment get it.  Note that long double
	     on Darwin does have 16 byte alignment, and does not get this
	     alignment if passed directly; a structure with a long double
	     inside, however, would get 16 byte alignment.  Since libffi does
	     not support vectors, we need non concern ourselves with other
	     cases.  */
	  जइ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  जइ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      memcpy (argp, valp, z);
	    }
	  अण्णहा
	    {
	      argp = (वर्ण *)FFI_ALIGN (argp, align);
	      memcpy (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == राशिः);

  ffi_call_i386 (frame, राशिः);
}
#जइ defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

#ifdef FFI_GO_CLOSURES
शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

/** private members **/

शून्य FFI_HIDDEN ffi_closure_i386(शून्य);
शून्य FFI_HIDDEN ffi_closure_STDCALL(शून्य);
शून्य FFI_HIDDEN ffi_closure_REGISTER(शून्य);
#जइ defined(FFI_EXEC_STATIC_TRAMP)
शून्य FFI_HIDDEN ffi_closure_i386_alt(शून्य);
शून्य FFI_HIDDEN ffi_closure_STDCALL_alt(शून्य);
शून्य FFI_HIDDEN ffi_closure_REGISTER_alt(शून्य);
#endif

संरचना closure_frame
{
  unsigned rettemp[4];				/* 0 */
  unsigned regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*);	/* 32 */
  शून्य *user_data;				/* 36 */
};

पूर्णाङ्क FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_closure_inner (संरचना closure_frame *frame, वर्ण *राशिः)
{
  ffi_cif *cif = frame->cif;
  पूर्णाङ्क cabi, i, n, flags, dir, narg_reg;
  ध्रुव संरचना abi_params *pabi;
  ffi_type **arg_types;
  वर्ण *argp;
  शून्य *rvalue;
  शून्य **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = frame->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? राशिः + राशिः_ALIGN (cif->bytes) : राशिः);

  विकल्प (flags)
    {
    पक्ष X86_RET_STRUCTARG:
      जइ (pabi->nregs > 0)
	{
	  rvalue = (शून्य *)frame->regs[pabi->regs[0]];
	  narg_reg = 1;
	  frame->rettemp[0] = (unsigned)rvalue;
	  भङ्ग;
	}
      /* fallthru */
    पक्ष X86_RET_STRUCTPOP:
      rvalue = *(शून्य **)argp;
      argp += परिमाण(शून्य *);
      frame->rettemp[0] = (unsigned)rvalue;
      भङ्ग;
    }

  n = cif->nargs;
  avalue = alloca(परिमाण(शून्य *) * n);

  arg_types = cif->arg_types;
  प्रति (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      पूर्णाङ्क t = ty->type;
      शून्य *valp;

      जइ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  जइ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &frame->regs[pabi->regs[narg_reg++]];
	  अण्णहा जइ (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  अण्णहा
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      अण्णहा
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  जइ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  जइ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  जइ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  अण्णहा
	    {
	      argp = (वर्ण *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  frame->fun (cif, rvalue, avalue, frame->user_data);

  विकल्प (cabi)
    {
    पक्ष FFI_STDCALL:
      प्रत्यागम flags | (cif->bytes << X86_RET_निष्कासय_SHIFT);
    पक्ष FFI_THISCALL:
    पक्ष FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      प्रत्यागम flags | (((unsigned) (argp - राशिः)) << X86_RET_निष्कासय_SHIFT);
    स्वभाव:
      प्रत्यागम flags;
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
                      शून्य *user_data,
                      शून्य *codeloc)
{
  वर्ण *tramp = closure->tramp;
  शून्य (*dest)(शून्य);
  पूर्णाङ्क op = 0xb8;  /* movl imm, %eax */

  विकल्प (cif->abi)
    {
    पक्ष FFI_SYSV:
    पक्ष FFI_MS_CDECL:
      dest = ffi_closure_i386;
      भङ्ग;
    पक्ष FFI_STDCALL:
    पक्ष FFI_THISCALL:
    पक्ष FFI_FASTCALL:
    पक्ष FFI_PASCAL:
      dest = ffi_closure_STDCALL;
      भङ्ग;
    पक्ष FFI_REGISTER:
      dest = ffi_closure_REGISTER;
      op = 0x68;  /* pushl imm */
      भङ्ग;
    स्वभाव:
      प्रत्यागम FFI_BAD_ABI;
    }

#जइ defined(FFI_EXEC_STATIC_TRAMP)
  जइ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      जइ (dest == ffi_closure_i386)
        dest = ffi_closure_i386_alt;
      अण्णहा जइ (dest == ffi_closure_STDCALL)
        dest = ffi_closure_STDCALL_alt;
      अण्णहा
        dest = ffi_closure_REGISTER_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      गच्छ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(शून्य **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(unsigned *)(tramp + 10) = (unsigned)dest - ((unsigned)codeloc + 14);

#जइ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  प्रत्यागम FFI_OK;
}

#ifdef FFI_GO_CLOSURES

शून्य FFI_HIDDEN ffi_go_closure_EAX(शून्य);
शून्य FFI_HIDDEN ffi_go_closure_ECX(शून्य);
शून्य FFI_HIDDEN ffi_go_closure_STDCALL(शून्य);

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*))
{
  शून्य (*dest)(शून्य);

  विकल्प (cif->abi)
    {
    पक्ष FFI_SYSV:
    पक्ष FFI_MS_CDECL:
      dest = ffi_go_closure_ECX;
      भङ्ग;
    पक्ष FFI_THISCALL:
    पक्ष FFI_FASTCALL:
      dest = ffi_go_closure_EAX;
      भङ्ग;
    पक्ष FFI_STDCALL:
    पक्ष FFI_PASCAL:
      dest = ffi_go_closure_STDCALL;
      भङ्ग;
    पक्ष FFI_REGISTER:
    स्वभाव:
      प्रत्यागम FFI_BAD_ABI;
    }

  closure->tramp = dest;
  closure->cif = cif;
  closure->fun = fun;

  प्रत्यागम FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#जइ !FFI_NO_RAW_API

शून्य FFI_HIDDEN ffi_closure_raw_SYSV(शून्य);
शून्य FFI_HIDDEN ffi_closure_raw_THISCALL(शून्य);

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure *closure,
                          ffi_cif *cif,
                          शून्य (*fun)(ffi_cif*,शून्य*,ffi_raw*,शून्य*),
                          शून्य *user_data,
                          शून्य *codeloc)
{
  वर्ण *tramp = closure->tramp;
  शून्य (*dest)(शून्य);
  पूर्णाङ्क i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  प्रति (i = cif->nargs-1; i >= 0; i--)
    विकल्प (cif->arg_types[i]->type)
      {
      पक्ष FFI_TYPE_STRUCT:
      पक्ष FFI_TYPE_LONGDOUBLE:
	प्रत्यागम FFI_BAD_TYPEDEF;
      }

  विकल्प (cif->abi)
    {
    पक्ष FFI_THISCALL:
      dest = ffi_closure_raw_THISCALL;
      भङ्ग;
    पक्ष FFI_SYSV:
      dest = ffi_closure_raw_SYSV;
      भङ्ग;
    स्वभाव:
      प्रत्यागम FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(शून्य **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(unsigned *)(tramp + 6) = (unsigned)dest - ((unsigned)codeloc + 10);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  प्रत्यागम FFI_OK;
}

शून्य
ffi_raw_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, ffi_raw *avalue)
{
  size_t rsize, bytes;
  संरचना call_frame *frame;
  वर्ण *राशिः, *argp;
  ffi_type **arg_types;
  पूर्णाङ्क flags, cabi, i, n, narg_reg;
  ध्रुव संरचना abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  जइ (rvalue == रिक्त)
    {
      विकल्प (flags)
	{
	पक्ष X86_RET_FLOAT:
	पक्ष X86_RET_DOUBLE:
	पक्ष X86_RET_LDOUBLE:
	पक्ष X86_RET_STRUCTPOP:
	पक्ष X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  भङ्ग;
	स्वभाव:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  भङ्ग;
	}
    }

  bytes = राशिः_ALIGN (cif->bytes);
  argp = राशिः =
      (शून्य *)((uintptr_t)alloca(bytes + परिमाण(*frame) + rsize + 15) & ~16);
  frame = (संरचना call_frame *)(राशिः + bytes);
  जइ (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;

  narg_reg = 0;
  विकल्प (flags)
    {
    पक्ष X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      जइ (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  भङ्ग;
	}
      /* fallthru */
    पक्ष X86_RET_STRUCTPOP:
      *(शून्य **)argp = rvalue;
      argp += परिमाण(शून्य *);
      bytes -= परिमाण(शून्य *);
      भङ्ग;
    }

  arg_types = cif->arg_types;
  प्रति (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      पूर्णाङ्क t = ty->type;

      जइ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  frame->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      अण्णहा
	{
	  memcpy (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      bytes -= z;
    }
  जइ (i < n)
    memcpy (argp, avalue, bytes);

  ffi_call_i386 (frame, राशिः);
}
#endif /* !FFI_NO_RAW_API */

#जइ defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *map_size = X86_TRAMP_MAP_SIZE;
  *tramp_size = X86_TRAMP_SIZE;
  प्रत्यागम &trampoline_code_table;
}
#endif

#endif /* __i386__ */
