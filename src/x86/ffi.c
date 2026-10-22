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

#যদি defined(__i386__) || defined(_M_IX86)
#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <tramp.h>
#অন্তর্ভাব "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# যদি FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#অন্যথা
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#যদি defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#endif

#যদি defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#define স্তূপ_ALIGN(bytes) (bytes)
#অন্যথা
#define স্তূপ_ALIGN(bytes) FFI_ALIGN (bytes, 16)
#endif

/* Perform machine dependent cif processing.  */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  পূর্ণসংখ্যা i, n, flags, cabi = cif->abi;

  নির্বাচন (cabi)
    {
    ক্ষেত্র FFI_SYSV:
    ক্ষেত্র FFI_STDCALL:
    ক্ষেত্র FFI_THISCALL:
    ক্ষেত্র FFI_FASTCALL:
    ক্ষেত্র FFI_MS_CDECL:
    ক্ষেত্র FFI_PASCAL:
    ক্ষেত্র FFI_REGISTER:
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_ABI;
    }

  নির্বাচন (cif->rtype->type)
    {
    ক্ষেত্র FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      বিরতি;
    ক্ষেত্র FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      বিরতি;
    ক্ষেত্র FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      বিরতি;
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      বিরতি;
    ক্ষেত্র FFI_TYPE_INT:
    ক্ষেত্র FFI_TYPE_SINT32:
    ক্ষেত্র FFI_TYPE_UINT32:
    ক্ষেত্র FFI_TYPE_POINTER:
      flags = X86_RET_INT32;
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      বিরতি;
    ক্ষেত্র FFI_TYPE_STRUCT:
      {
#যদি defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t আকার_2 = cif->rtype->আকার_2;
        যদি (আকার_2 == 1)
          flags = X86_RET_STRUCT_1B;
        অন্যথা যদি (আকার_2 == 2)
          flags = X86_RET_STRUCT_2B;
        অন্যথা যদি (আকার_2 == 4)
          flags = X86_RET_INT32;
        অন্যথা যদি (আকার_2 == 8)
          flags = X86_RET_INT64;
        অন্যথা
#endif
          {
          do_struct:
            নির্বাচন (cabi)
              {
              ক্ষেত্র FFI_THISCALL:
              ক্ষেত্র FFI_FASTCALL:
              ক্ষেত্র FFI_STDCALL:
              ক্ষেত্র FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                বিরতি;
              পূর্বনির্ধারিত:
                flags = X86_RET_STRUCTPOP;
                বিরতি;
              }
            /* Allocate space for return value pointer.  */
            bytes += FFI_ALIGN (আকার(শূন্য*), FFI_SIZEOF_ARG);
          }
      }
      বিরতি;
    ক্ষেত্র FFI_TYPE_COMPLEX:
      নির্বাচন (cif->rtype->elements[0]->type)
	{
	ক্ষেত্র FFI_TYPE_DOUBLE:
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	ক্ষেত্র FFI_TYPE_SINT64:
	ক্ষেত্র FFI_TYPE_UINT64:
	  যাও do_struct;
	ক্ষেত্র FFI_TYPE_FLOAT:
	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT8:
	ক্ষেত্র FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  বিরতি;
	পূর্বনির্ধারিত:
	  ফেরত FFI_BAD_TYPEDEF;
	}
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  জন্য (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#যদি defined(X86_WIN32)
      যদি (cabi == FFI_STDCALL)
        bytes = FFI_ALIGN (bytes, FFI_SIZEOF_ARG);
      অন্যথা
#endif
        bytes = FFI_ALIGN (bytes, t->alignment);
      bytes += FFI_ALIGN (t->আকার_2, FFI_SIZEOF_ARG);
    }
  cif->bytes = bytes;

  ফেরত FFI_OK;
}

স্থির ffi_arg
extend_basic_type(শূন্য *arg, পূর্ণসংখ্যা type)
{
  নির্বাচন (type)
    {
    ক্ষেত্র FFI_TYPE_SINT8:
      ফেরত *(SINT8 *)arg;
    ক্ষেত্র FFI_TYPE_UINT8:
      ফেরত *(UINT8 *)arg;
    ক্ষেত্র FFI_TYPE_SINT16:
      ফেরত *(SINT16 *)arg;
    ক্ষেত্র FFI_TYPE_UINT16:
      ফেরত *(UINT16 *)arg;

    ক্ষেত্র FFI_TYPE_SINT32:
    ক্ষেত্র FFI_TYPE_UINT32:
    ক্ষেত্র FFI_TYPE_POINTER:
    ক্ষেত্র FFI_TYPE_FLOAT:
      ফেরত *(UINT32 *)arg;

    পূর্বনির্ধারিত:
      abort();
    }
}

কাঠামো call_frame
{
  শূন্য *ebp;		/* 0 */
  শূন্য *retaddr;	/* 4 */
  শূন্য (*fn)(শূন্য);	/* 8 */
  পূর্ণসংখ্যা flags;		/* 12 */
  শূন্য *rvalue;		/* 16 */
  unsigned regs[3];	/* 20-28 */
};

কাঠামো abi_params
{
  পূর্ণসংখ্যা dir;		/* parameter growth direction */
  পূর্ণসংখ্যা static_chain;	/* the static chain register used by gcc */
  পূর্ণসংখ্যা nregs;		/* number of register parameters */
  পূর্ণসংখ্যা regs[3];
};

স্থির ধ্রুবক কাঠামো abi_params abi_params[FFI_LAST_ABI] = {
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
  #অন্যথা
    #define FFI_DECLARE_FASTCALL __declspec(fastcall)
  #endif
#অন্যথা
  #define FFI_DECLARE_FASTCALL
#endif

বাহ্যিক শূন্য FFI_DECLARE_FASTCALL ffi_call_i386(কাঠামো call_frame *, অক্ষর *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#যদি defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	      শূন্য **avalue, শূন্য *closure)
{
  size_t rsize, bytes;
  কাঠামো call_frame *frame;
  অক্ষর *স্তূপ, *argp;
  ffi_type **arg_types;
  পূর্ণসংখ্যা flags, cabi, i, n, dir, narg_reg;
  ধ্রুবক কাঠামো abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  যদি (rvalue == ফাঁকা)
    {
      নির্বাচন (flags)
	{
	ক্ষেত্র X86_RET_FLOAT:
	ক্ষেত্র X86_RET_DOUBLE:
	ক্ষেত্র X86_RET_LDOUBLE:
	ক্ষেত্র X86_RET_STRUCTPOP:
	ক্ষেত্র X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->আকার_2;
	  বিরতি;
	পূর্বনির্ধারিত:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  বিরতি;
	}
    }

  bytes = স্তূপ_ALIGN (cif->bytes);
  স্তূপ = alloca(bytes + আকার(*frame) + rsize);
  argp = (dir < 0 ? স্তূপ + bytes : স্তূপ);
  frame = (কাঠামো call_frame *)(স্তূপ + bytes);
  যদি (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;
  frame->regs[pabi->static_chain] = (unsigned)closure;

  narg_reg = 0;
  নির্বাচন (flags)
    {
    ক্ষেত্র X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      যদি (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  বিরতি;
	}
      /* fallthru */
    ক্ষেত্র X86_RET_STRUCTPOP:
      *(শূন্য **)argp = rvalue;
      argp += আকার(শূন্য *);
      বিরতি;
    }

  arg_types = cif->arg_types;
  জন্য (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      শূন্য *valp = avalue[i];
      size_t z = ty->আকার_2;
      পূর্ণসংখ্যা t = ty->type;

      যদি (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  যদি (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    frame->regs[pabi->regs[narg_reg++]] = val;
	  অন্যথা যদি (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  অন্যথা
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      অন্যথা
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  যদি ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  যদি (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  যদি (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      memcpy (argp, valp, z);
	    }
	  অন্যথা
	    {
	      argp = (অক্ষর *)FFI_ALIGN (argp, align);
	      memcpy (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == স্তূপ);

  ffi_call_i386 (frame, স্তূপ);
}
#যদি defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ফাঁকা);
}

#ifdef FFI_GO_CLOSURES
শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

/** private members **/

শূন্য FFI_HIDDEN ffi_closure_i386(শূন্য);
শূন্য FFI_HIDDEN ffi_closure_STDCALL(শূন্য);
শূন্য FFI_HIDDEN ffi_closure_REGISTER(শূন্য);
#যদি defined(FFI_EXEC_STATIC_TRAMP)
শূন্য FFI_HIDDEN ffi_closure_i386_alt(শূন্য);
শূন্য FFI_HIDDEN ffi_closure_STDCALL_alt(শূন্য);
শূন্য FFI_HIDDEN ffi_closure_REGISTER_alt(শূন্য);
#endif

কাঠামো closure_frame
{
  unsigned rettemp[4];				/* 0 */
  unsigned regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*);	/* 32 */
  শূন্য *user_তথ্য;				/* 36 */
};

পূর্ণসংখ্যা FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_closure_inner (কাঠামো closure_frame *frame, অক্ষর *স্তূপ)
{
  ffi_cif *cif = frame->cif;
  পূর্ণসংখ্যা cabi, i, n, flags, dir, narg_reg;
  ধ্রুবক কাঠামো abi_params *pabi;
  ffi_type **arg_types;
  অক্ষর *argp;
  শূন্য *rvalue;
  শূন্য **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = frame->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? স্তূপ + স্তূপ_ALIGN (cif->bytes) : স্তূপ);

  নির্বাচন (flags)
    {
    ক্ষেত্র X86_RET_STRUCTARG:
      যদি (pabi->nregs > 0)
	{
	  rvalue = (শূন্য *)frame->regs[pabi->regs[0]];
	  narg_reg = 1;
	  frame->rettemp[0] = (unsigned)rvalue;
	  বিরতি;
	}
      /* fallthru */
    ক্ষেত্র X86_RET_STRUCTPOP:
      rvalue = *(শূন্য **)argp;
      argp += আকার(শূন্য *);
      frame->rettemp[0] = (unsigned)rvalue;
      বিরতি;
    }

  n = cif->nargs;
  avalue = alloca(আকার(শূন্য *) * n);

  arg_types = cif->arg_types;
  জন্য (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->আকার_2;
      পূর্ণসংখ্যা t = ty->type;
      শূন্য *valp;

      যদি (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  যদি (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &frame->regs[pabi->regs[narg_reg++]];
	  অন্যথা যদি (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  অন্যথা
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      অন্যথা
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  যদি (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  যদি ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  যদি (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  অন্যথা
	    {
	      argp = (অক্ষর *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  frame->fun (cif, rvalue, avalue, frame->user_তথ্য);

  নির্বাচন (cabi)
    {
    ক্ষেত্র FFI_STDCALL:
      ফেরত flags | (cif->bytes << X86_RET_বের_করো_SHIFT);
    ক্ষেত্র FFI_THISCALL:
    ক্ষেত্র FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      ফেরত flags | (((unsigned) (argp - স্তূপ)) << X86_RET_বের_করো_SHIFT);
    পূর্বনির্ধারিত:
      ফেরত flags;
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
                      শূন্য *user_তথ্য,
                      শূন্য *codeloc)
{
  অক্ষর *tramp = closure->tramp;
  শূন্য (*dest)(শূন্য);
  পূর্ণসংখ্যা op = 0xb8;  /* movl imm, %eax */

  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_SYSV:
    ক্ষেত্র FFI_MS_CDECL:
      dest = ffi_closure_i386;
      বিরতি;
    ক্ষেত্র FFI_STDCALL:
    ক্ষেত্র FFI_THISCALL:
    ক্ষেত্র FFI_FASTCALL:
    ক্ষেত্র FFI_PASCAL:
      dest = ffi_closure_STDCALL;
      বিরতি;
    ক্ষেত্র FFI_REGISTER:
      dest = ffi_closure_REGISTER;
      op = 0x68;  /* pushl imm */
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_ABI;
    }

#যদি defined(FFI_EXEC_STATIC_TRAMP)
  যদি (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      যদি (dest == ffi_closure_i386)
        dest = ffi_closure_i386_alt;
      অন্যথা যদি (dest == ffi_closure_STDCALL)
        dest = ffi_closure_STDCALL_alt;
      অন্যথা
        dest = ffi_closure_REGISTER_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      যাও out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(শূন্য **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(unsigned *)(tramp + 10) = (unsigned)dest - ((unsigned)codeloc + 14);

#যদি defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

  ফেরত FFI_OK;
}

#ifdef FFI_GO_CLOSURES

শূন্য FFI_HIDDEN ffi_go_closure_EAX(শূন্য);
শূন্য FFI_HIDDEN ffi_go_closure_ECX(শূন্য);
শূন্য FFI_HIDDEN ffi_go_closure_STDCALL(শূন্য);

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*))
{
  শূন্য (*dest)(শূন্য);

  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_SYSV:
    ক্ষেত্র FFI_MS_CDECL:
      dest = ffi_go_closure_ECX;
      বিরতি;
    ক্ষেত্র FFI_THISCALL:
    ক্ষেত্র FFI_FASTCALL:
      dest = ffi_go_closure_EAX;
      বিরতি;
    ক্ষেত্র FFI_STDCALL:
    ক্ষেত্র FFI_PASCAL:
      dest = ffi_go_closure_STDCALL;
      বিরতি;
    ক্ষেত্র FFI_REGISTER:
    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_ABI;
    }

  closure->tramp = dest;
  closure->cif = cif;
  closure->fun = fun;

  ফেরত FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#যদি !FFI_NO_RAW_API

শূন্য FFI_HIDDEN ffi_closure_raw_SYSV(শূন্য);
শূন্য FFI_HIDDEN ffi_closure_raw_THISCALL(শূন্য);

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure *closure,
                          ffi_cif *cif,
                          শূন্য (*fun)(ffi_cif*,শূন্য*,ffi_raw*,শূন্য*),
                          শূন্য *user_তথ্য,
                          শূন্য *codeloc)
{
  অক্ষর *tramp = closure->tramp;
  শূন্য (*dest)(শূন্য);
  পূর্ণসংখ্যা i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  জন্য (i = cif->nargs-1; i >= 0; i--)
    নির্বাচন (cif->arg_types[i]->type)
      {
      ক্ষেত্র FFI_TYPE_STRUCT:
      ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	ফেরত FFI_BAD_TYPEDEF;
      }

  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_THISCALL:
      dest = ffi_closure_raw_THISCALL;
      বিরতি;
    ক্ষেত্র FFI_SYSV:
      dest = ffi_closure_raw_SYSV;
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(শূন্য **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(unsigned *)(tramp + 6) = (unsigned)dest - ((unsigned)codeloc + 10);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

  ফেরত FFI_OK;
}

শূন্য
ffi_raw_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, ffi_raw *avalue)
{
  size_t rsize, bytes;
  কাঠামো call_frame *frame;
  অক্ষর *স্তূপ, *argp;
  ffi_type **arg_types;
  পূর্ণসংখ্যা flags, cabi, i, n, narg_reg;
  ধ্রুবক কাঠামো abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  যদি (rvalue == ফাঁকা)
    {
      নির্বাচন (flags)
	{
	ক্ষেত্র X86_RET_FLOAT:
	ক্ষেত্র X86_RET_DOUBLE:
	ক্ষেত্র X86_RET_LDOUBLE:
	ক্ষেত্র X86_RET_STRUCTPOP:
	ক্ষেত্র X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->আকার_2;
	  বিরতি;
	পূর্বনির্ধারিত:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  বিরতি;
	}
    }

  bytes = স্তূপ_ALIGN (cif->bytes);
  argp = স্তূপ =
      (শূন্য *)((uintptr_t)alloca(bytes + আকার(*frame) + rsize + 15) & ~16);
  frame = (কাঠামো call_frame *)(স্তূপ + bytes);
  যদি (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;

  narg_reg = 0;
  নির্বাচন (flags)
    {
    ক্ষেত্র X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      যদি (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  বিরতি;
	}
      /* fallthru */
    ক্ষেত্র X86_RET_STRUCTPOP:
      *(শূন্য **)argp = rvalue;
      argp += আকার(শূন্য *);
      bytes -= আকার(শূন্য *);
      বিরতি;
    }

  arg_types = cif->arg_types;
  জন্য (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->আকার_2;
      পূর্ণসংখ্যা t = ty->type;

      যদি (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  frame->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      অন্যথা
	{
	  memcpy (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      bytes -= z;
    }
  যদি (i < n)
    memcpy (argp, avalue, bytes);

  ffi_call_i386 (frame, স্তূপ);
}
#endif /* !FFI_NO_RAW_API */

#যদি defined(FFI_EXEC_STATIC_TRAMP)
শূন্য *
ffi_tramp_arch (size_t *tramp_আকার, size_t *map_আকার)
{
  বাহ্যিক শূন্য *trampoline_code_table;

  *map_আকার = X86_TRAMP_MAP_আকার;
  *tramp_আকার = X86_TRAMP_আকার;
  ফেরত &trampoline_code_table;
}
#endif

#endif /* __i386__ */
