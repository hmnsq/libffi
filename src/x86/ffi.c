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

#எனில் defined(__i386__) || defined(_M_IX86)
#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <tramp.h>
#அந்தர்பாவ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# எனில் FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#இல்லையெனில்
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#எனில் defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#endif

#எனில் defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#define அடுக்கு_ALIGN(bytes) (bytes)
#இல்லையெனில்
#define அடுக்கு_ALIGN(bytes) FFI_ALIGN (bytes, 16)
#endif

/* Perform machine dependent cif processing.  */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  முழு i, n, flags, cabi = cif->abi;

  தேர்வு (cabi)
    {
    நிலை FFI_SYSV:
    நிலை FFI_STDCALL:
    நிலை FFI_THISCALL:
    நிலை FFI_FASTCALL:
    நிலை FFI_MS_CDECL:
    நிலை FFI_PASCAL:
    நிலை FFI_REGISTER:
      நிறுத்து;
    இயல்பு:
      திரும்பு FFI_BAD_ABI;
    }

  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      நிறுத்து;
    நிலை FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      நிறுத்து;
    நிலை FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      நிறுத்து;
    நிலை FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      நிறுத்து;
    நிலை FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      நிறுத்து;
    நிலை FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      நிறுத்து;
    நிலை FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      நிறுத்து;
    நிலை FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      நிறுத்து;
    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_SINT32:
    நிலை FFI_TYPE_UINT32:
    நிலை FFI_TYPE_POINTER:
      flags = X86_RET_INT32;
      நிறுத்து;
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      நிறுத்து;
    நிலை FFI_TYPE_STRUCT:
      {
#எனில் defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t size = cif->rtype->size;
        எனில் (size == 1)
          flags = X86_RET_STRUCT_1B;
        இல்லையெனில் எனில் (size == 2)
          flags = X86_RET_STRUCT_2B;
        இல்லையெனில் எனில் (size == 4)
          flags = X86_RET_INT32;
        இல்லையெனில் எனில் (size == 8)
          flags = X86_RET_INT64;
        இல்லையெனில்
#endif
          {
          do_struct:
            தேர்வு (cabi)
              {
              நிலை FFI_THISCALL:
              நிலை FFI_FASTCALL:
              நிலை FFI_STDCALL:
              நிலை FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                நிறுத்து;
              இயல்பு:
                flags = X86_RET_STRUCTPOP;
                நிறுத்து;
              }
            /* Allocate space for return value pointer.  */
            bytes += FFI_ALIGN (அளவு(வெறுமை*), FFI_SIZEOF_ARG);
          }
      }
      நிறுத்து;
    நிலை FFI_TYPE_COMPLEX:
      தேர்வு (cif->rtype->elements[0]->type)
	{
	நிலை FFI_TYPE_DOUBLE:
	நிலை FFI_TYPE_LONGDOUBLE:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
	  செல் do_struct;
	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  நிறுத்து;
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  நிறுத்து;
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  நிறுத்து;
	இயல்பு:
	  திரும்பு FFI_BAD_TYPEDEF;
	}
      நிறுத்து;
    இயல்பு:
      திரும்பு FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  க்கு (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#எனில் defined(X86_WIN32)
      எனில் (cabi == FFI_STDCALL)
        bytes = FFI_ALIGN (bytes, FFI_SIZEOF_ARG);
      இல்லையெனில்
#endif
        bytes = FFI_ALIGN (bytes, t->alignment);
      bytes += FFI_ALIGN (t->size, FFI_SIZEOF_ARG);
    }
  cif->bytes = bytes;

  திரும்பு FFI_OK;
}

நிலையான ffi_arg
extend_basic_type(வெறுமை *arg, முழு type)
{
  தேர்வு (type)
    {
    நிலை FFI_TYPE_SINT8:
      திரும்பு *(SINT8 *)arg;
    நிலை FFI_TYPE_UINT8:
      திரும்பு *(UINT8 *)arg;
    நிலை FFI_TYPE_SINT16:
      திரும்பு *(SINT16 *)arg;
    நிலை FFI_TYPE_UINT16:
      திரும்பு *(UINT16 *)arg;

    நிலை FFI_TYPE_SINT32:
    நிலை FFI_TYPE_UINT32:
    நிலை FFI_TYPE_POINTER:
    நிலை FFI_TYPE_FLOAT:
      திரும்பு *(UINT32 *)arg;

    இயல்பு:
      abort();
    }
}

கட்டமைப்பு call_frame
{
  வெறுமை *ebp;		/* 0 */
  வெறுமை *retaddr;	/* 4 */
  வெறுமை (*fn)(வெறுமை);	/* 8 */
  முழு flags;		/* 12 */
  வெறுமை *rvalue;		/* 16 */
  unsigned regs[3];	/* 20-28 */
};

கட்டமைப்பு abi_params
{
  முழு dir;		/* parameter growth direction */
  முழு static_chain;	/* the static chain register used by gcc */
  முழு nregs;		/* number of register parameters */
  முழு regs[3];
};

நிலையான மாறிலி கட்டமைப்பு abi_params abi_params[FFI_LAST_ABI] = {
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
  #இல்லையெனில்
    #define FFI_DECLARE_FASTCALL __declspec(fastcall)
  #endif
#இல்லையெனில்
  #define FFI_DECLARE_FASTCALL
#endif

வெளி வெறுமை FFI_DECLARE_FASTCALL ffi_call_i386(கட்டமைப்பு call_frame *, எழுத்து *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#எனில் defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
நிலையான வெறுமை
ffi_call_int (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	      வெறுமை **avalue, வெறுமை *closure)
{
  size_t rsize, bytes;
  கட்டமைப்பு call_frame *frame;
  எழுத்து *அடுக்கு, *argp;
  ffi_type **arg_types;
  முழு flags, cabi, i, n, dir, narg_reg;
  மாறிலி கட்டமைப்பு abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  எனில் (rvalue == வெற்று)
    {
      தேர்வு (flags)
	{
	நிலை X86_RET_FLOAT:
	நிலை X86_RET_DOUBLE:
	நிலை X86_RET_LDOUBLE:
	நிலை X86_RET_STRUCTPOP:
	நிலை X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  நிறுத்து;
	இயல்பு:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  நிறுத்து;
	}
    }

  bytes = அடுக்கு_ALIGN (cif->bytes);
  அடுக்கு = alloca(bytes + அளவு(*frame) + rsize);
  argp = (dir < 0 ? அடுக்கு + bytes : அடுக்கு);
  frame = (கட்டமைப்பு call_frame *)(அடுக்கு + bytes);
  எனில் (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;
  frame->regs[pabi->static_chain] = (unsigned)closure;

  narg_reg = 0;
  தேர்வு (flags)
    {
    நிலை X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      எனில் (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  நிறுத்து;
	}
      /* fallthru */
    நிலை X86_RET_STRUCTPOP:
      *(வெறுமை **)argp = rvalue;
      argp += அளவு(வெறுமை *);
      நிறுத்து;
    }

  arg_types = cif->arg_types;
  க்கு (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      வெறுமை *valp = avalue[i];
      size_t z = ty->size;
      முழு t = ty->type;

      எனில் (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  எனில் (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    frame->regs[pabi->regs[narg_reg++]] = val;
	  இல்லையெனில் எனில் (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  இல்லையெனில்
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      இல்லையெனில்
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  எனில் ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  எனில் (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  எனில் (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      memcpy (argp, valp, z);
	    }
	  இல்லையெனில்
	    {
	      argp = (எழுத்து *)FFI_ALIGN (argp, align);
	      memcpy (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == அடுக்கு);

  ffi_call_i386 (frame, அடுக்கு);
}
#எனில் defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, வெற்று);
}

#ifdef FFI_GO_CLOSURES
வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

/** private members **/

வெறுமை FFI_HIDDEN ffi_closure_i386(வெறுமை);
வெறுமை FFI_HIDDEN ffi_closure_STDCALL(வெறுமை);
வெறுமை FFI_HIDDEN ffi_closure_REGISTER(வெறுமை);
#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெறுமை FFI_HIDDEN ffi_closure_i386_alt(வெறுமை);
வெறுமை FFI_HIDDEN ffi_closure_STDCALL_alt(வெறுமை);
வெறுமை FFI_HIDDEN ffi_closure_REGISTER_alt(வெறுமை);
#endif

கட்டமைப்பு closure_frame
{
  unsigned rettemp[4];				/* 0 */
  unsigned regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*);	/* 32 */
  வெறுமை *user_data;				/* 36 */
};

முழு FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_closure_inner (கட்டமைப்பு closure_frame *frame, எழுத்து *அடுக்கு)
{
  ffi_cif *cif = frame->cif;
  முழு cabi, i, n, flags, dir, narg_reg;
  மாறிலி கட்டமைப்பு abi_params *pabi;
  ffi_type **arg_types;
  எழுத்து *argp;
  வெறுமை *rvalue;
  வெறுமை **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = frame->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? அடுக்கு + அடுக்கு_ALIGN (cif->bytes) : அடுக்கு);

  தேர்வு (flags)
    {
    நிலை X86_RET_STRUCTARG:
      எனில் (pabi->nregs > 0)
	{
	  rvalue = (வெறுமை *)frame->regs[pabi->regs[0]];
	  narg_reg = 1;
	  frame->rettemp[0] = (unsigned)rvalue;
	  நிறுத்து;
	}
      /* fallthru */
    நிலை X86_RET_STRUCTPOP:
      rvalue = *(வெறுமை **)argp;
      argp += அளவு(வெறுமை *);
      frame->rettemp[0] = (unsigned)rvalue;
      நிறுத்து;
    }

  n = cif->nargs;
  avalue = alloca(அளவு(வெறுமை *) * n);

  arg_types = cif->arg_types;
  க்கு (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      முழு t = ty->type;
      வெறுமை *valp;

      எனில் (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  எனில் (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &frame->regs[pabi->regs[narg_reg++]];
	  இல்லையெனில் எனில் (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  இல்லையெனில்
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      இல்லையெனில்
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  எனில் (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  எனில் ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  எனில் (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  இல்லையெனில்
	    {
	      argp = (எழுத்து *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  frame->fun (cif, rvalue, avalue, frame->user_data);

  தேர்வு (cabi)
    {
    நிலை FFI_STDCALL:
      திரும்பு flags | (cif->bytes << X86_RET_எடு_SHIFT);
    நிலை FFI_THISCALL:
    நிலை FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      திரும்பு flags | (((unsigned) (argp - அடுக்கு)) << X86_RET_எடு_SHIFT);
    இயல்பு:
      திரும்பு flags;
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
                      வெறுமை *user_data,
                      வெறுமை *codeloc)
{
  எழுத்து *tramp = closure->tramp;
  வெறுமை (*dest)(வெறுமை);
  முழு op = 0xb8;  /* movl imm, %eax */

  தேர்வு (cif->abi)
    {
    நிலை FFI_SYSV:
    நிலை FFI_MS_CDECL:
      dest = ffi_closure_i386;
      நிறுத்து;
    நிலை FFI_STDCALL:
    நிலை FFI_THISCALL:
    நிலை FFI_FASTCALL:
    நிலை FFI_PASCAL:
      dest = ffi_closure_STDCALL;
      நிறுத்து;
    நிலை FFI_REGISTER:
      dest = ffi_closure_REGISTER;
      op = 0x68;  /* pushl imm */
      நிறுத்து;
    இயல்பு:
      திரும்பு FFI_BAD_ABI;
    }

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
  எனில் (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      எனில் (dest == ffi_closure_i386)
        dest = ffi_closure_i386_alt;
      இல்லையெனில் எனில் (dest == ffi_closure_STDCALL)
        dest = ffi_closure_STDCALL_alt;
      இல்லையெனில்
        dest = ffi_closure_REGISTER_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      செல் out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(வெறுமை **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(unsigned *)(tramp + 10) = (unsigned)dest - ((unsigned)codeloc + 14);

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  திரும்பு FFI_OK;
}

#ifdef FFI_GO_CLOSURES

வெறுமை FFI_HIDDEN ffi_go_closure_EAX(வெறுமை);
வெறுமை FFI_HIDDEN ffi_go_closure_ECX(வெறுமை);
வெறுமை FFI_HIDDEN ffi_go_closure_STDCALL(வெறுமை);

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*))
{
  வெறுமை (*dest)(வெறுமை);

  தேர்வு (cif->abi)
    {
    நிலை FFI_SYSV:
    நிலை FFI_MS_CDECL:
      dest = ffi_go_closure_ECX;
      நிறுத்து;
    நிலை FFI_THISCALL:
    நிலை FFI_FASTCALL:
      dest = ffi_go_closure_EAX;
      நிறுத்து;
    நிலை FFI_STDCALL:
    நிலை FFI_PASCAL:
      dest = ffi_go_closure_STDCALL;
      நிறுத்து;
    நிலை FFI_REGISTER:
    இயல்பு:
      திரும்பு FFI_BAD_ABI;
    }

  closure->tramp = dest;
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#எனில் !FFI_NO_RAW_API

வெறுமை FFI_HIDDEN ffi_closure_raw_SYSV(வெறுமை);
வெறுமை FFI_HIDDEN ffi_closure_raw_THISCALL(வெறுமை);

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure *closure,
                          ffi_cif *cif,
                          வெறுமை (*fun)(ffi_cif*,வெறுமை*,ffi_raw*,வெறுமை*),
                          வெறுமை *user_data,
                          வெறுமை *codeloc)
{
  எழுத்து *tramp = closure->tramp;
  வெறுமை (*dest)(வெறுமை);
  முழு i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  க்கு (i = cif->nargs-1; i >= 0; i--)
    தேர்வு (cif->arg_types[i]->type)
      {
      நிலை FFI_TYPE_STRUCT:
      நிலை FFI_TYPE_LONGDOUBLE:
	திரும்பு FFI_BAD_TYPEDEF;
      }

  தேர்வு (cif->abi)
    {
    நிலை FFI_THISCALL:
      dest = ffi_closure_raw_THISCALL;
      நிறுத்து;
    நிலை FFI_SYSV:
      dest = ffi_closure_raw_SYSV;
      நிறுத்து;
    இயல்பு:
      திரும்பு FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(வெறுமை **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(unsigned *)(tramp + 6) = (unsigned)dest - ((unsigned)codeloc + 10);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  திரும்பு FFI_OK;
}

வெறுமை
ffi_raw_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, ffi_raw *avalue)
{
  size_t rsize, bytes;
  கட்டமைப்பு call_frame *frame;
  எழுத்து *அடுக்கு, *argp;
  ffi_type **arg_types;
  முழு flags, cabi, i, n, narg_reg;
  மாறிலி கட்டமைப்பு abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  எனில் (rvalue == வெற்று)
    {
      தேர்வு (flags)
	{
	நிலை X86_RET_FLOAT:
	நிலை X86_RET_DOUBLE:
	நிலை X86_RET_LDOUBLE:
	நிலை X86_RET_STRUCTPOP:
	நிலை X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  நிறுத்து;
	இயல்பு:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  நிறுத்து;
	}
    }

  bytes = அடுக்கு_ALIGN (cif->bytes);
  argp = அடுக்கு =
      (வெறுமை *)((uintptr_t)alloca(bytes + அளவு(*frame) + rsize + 15) & ~16);
  frame = (கட்டமைப்பு call_frame *)(அடுக்கு + bytes);
  எனில் (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;

  narg_reg = 0;
  தேர்வு (flags)
    {
    நிலை X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      எனில் (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  நிறுத்து;
	}
      /* fallthru */
    நிலை X86_RET_STRUCTPOP:
      *(வெறுமை **)argp = rvalue;
      argp += அளவு(வெறுமை *);
      bytes -= அளவு(வெறுமை *);
      நிறுத்து;
    }

  arg_types = cif->arg_types;
  க்கு (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      முழு t = ty->type;

      எனில் (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  frame->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      இல்லையெனில்
	{
	  memcpy (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      bytes -= z;
    }
  எனில் (i < n)
    memcpy (argp, avalue, bytes);

  ffi_call_i386 (frame, அடுக்கு);
}
#endif /* !FFI_NO_RAW_API */

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெறுமை *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  வெளி வெறுமை *trampoline_code_table;

  *map_size = X86_TRAMP_MAP_SIZE;
  *tramp_size = X86_TRAMP_SIZE;
  திரும்பு &trampoline_code_table;
}
#endif

#endif /* __i386__ */
