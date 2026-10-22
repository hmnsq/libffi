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

#എങ്കിൽ defined(__i386__) || defined(_M_IX86)
#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <tramp.h>
#അന്തര്ഭാവ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# എങ്കിൽ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#അല്ലെങ്കിൽ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#എങ്കിൽ defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#endif

#എങ്കിൽ defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#define അട്ടി_ALIGN(bytes) (bytes)
#അല്ലെങ്കിൽ
#define അട്ടി_ALIGN(bytes) FFI_ALIGN (bytes, 16)
#endif

/* Perform machine dependent cif processing.  */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  പൂർണ്ണസംഖ്യ i, n, flags, cabi = cif->abi;

  തിരഞ്ഞെടുപ്പ് (cabi)
    {
    അവസ്ഥ FFI_SYSV:
    അവസ്ഥ FFI_STDCALL:
    അവസ്ഥ FFI_THISCALL:
    അവസ്ഥ FFI_FASTCALL:
    അവസ്ഥ FFI_MS_CDECL:
    അവസ്ഥ FFI_PASCAL:
    അവസ്ഥ FFI_REGISTER:
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_ABI;
    }

  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
    അവസ്ഥ FFI_TYPE_UINT32:
    അവസ്ഥ FFI_TYPE_POINTER:
      flags = X86_RET_INT32;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_STRUCT:
      {
#എങ്കിൽ defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t size = cif->rtype->size;
        എങ്കിൽ (size == 1)
          flags = X86_RET_STRUCT_1B;
        അല്ലെങ്കിൽ എങ്കിൽ (size == 2)
          flags = X86_RET_STRUCT_2B;
        അല്ലെങ്കിൽ എങ്കിൽ (size == 4)
          flags = X86_RET_INT32;
        അല്ലെങ്കിൽ എങ്കിൽ (size == 8)
          flags = X86_RET_INT64;
        അല്ലെങ്കിൽ
#endif
          {
          do_struct:
            തിരഞ്ഞെടുപ്പ് (cabi)
              {
              അവസ്ഥ FFI_THISCALL:
              അവസ്ഥ FFI_FASTCALL:
              അവസ്ഥ FFI_STDCALL:
              അവസ്ഥ FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                നിർത്തുക;
              സ്ഥിരസ്ഥിതി:
                flags = X86_RET_STRUCTPOP;
                നിർത്തുക;
              }
            /* Allocate space for return value pointer.  */
            bytes += FFI_ALIGN (വലുപ്പം(ശൂന്യം*), FFI_SIZEOF_ARG);
          }
      }
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_COMPLEX:
      തിരഞ്ഞെടുപ്പ് (cif->rtype->elements[0]->type)
	{
	അവസ്ഥ FFI_TYPE_DOUBLE:
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_UINT64:
	  പോകുക do_struct;
	അവസ്ഥ FFI_TYPE_FLOAT:
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
	  തിരികെ FFI_BAD_TYPEDEF;
	}
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  വേണ്ടി (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#എങ്കിൽ defined(X86_WIN32)
      എങ്കിൽ (cabi == FFI_STDCALL)
        bytes = FFI_ALIGN (bytes, FFI_SIZEOF_ARG);
      അല്ലെങ്കിൽ
#endif
        bytes = FFI_ALIGN (bytes, t->alignment);
      bytes += FFI_ALIGN (t->size, FFI_SIZEOF_ARG);
    }
  cif->bytes = bytes;

  തിരികെ FFI_OK;
}

സ്ഥിതം ffi_arg
extend_basic_type(ശൂന്യം *arg, പൂർണ്ണസംഖ്യ type)
{
  തിരഞ്ഞെടുപ്പ് (type)
    {
    അവസ്ഥ FFI_TYPE_SINT8:
      തിരികെ *(SINT8 *)arg;
    അവസ്ഥ FFI_TYPE_UINT8:
      തിരികെ *(UINT8 *)arg;
    അവസ്ഥ FFI_TYPE_SINT16:
      തിരികെ *(SINT16 *)arg;
    അവസ്ഥ FFI_TYPE_UINT16:
      തിരികെ *(UINT16 *)arg;

    അവസ്ഥ FFI_TYPE_SINT32:
    അവസ്ഥ FFI_TYPE_UINT32:
    അവസ്ഥ FFI_TYPE_POINTER:
    അവസ്ഥ FFI_TYPE_FLOAT:
      തിരികെ *(UINT32 *)arg;

    സ്ഥിരസ്ഥിതി:
      abort();
    }
}

ഘടന call_frame
{
  ശൂന്യം *ebp;		/* 0 */
  ശൂന്യം *retaddr;	/* 4 */
  ശൂന്യം (*fn)(ശൂന്യം);	/* 8 */
  പൂർണ്ണസംഖ്യ flags;		/* 12 */
  ശൂന്യം *rvalue;		/* 16 */
  unsigned regs[3];	/* 20-28 */
};

ഘടന abi_params
{
  പൂർണ്ണസംഖ്യ dir;		/* parameter growth direction */
  പൂർണ്ണസംഖ്യ static_chain;	/* the static chain register used by gcc */
  പൂർണ്ണസംഖ്യ nregs;		/* number of register parameters */
  പൂർണ്ണസംഖ്യ regs[3];
};

സ്ഥിതം സ്ഥിരം ഘടന abi_params abi_params[FFI_LAST_ABI] = {
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
  #അല്ലെങ്കിൽ
    #define FFI_DECLARE_FASTCALL __declspec(fastcall)
  #endif
#അല്ലെങ്കിൽ
  #define FFI_DECLARE_FASTCALL
#endif

ബാഹ്യ ശൂന്യം FFI_DECLARE_FASTCALL ffi_call_i386(ഘടന call_frame *, അക്ഷരം *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#എങ്കിൽ defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
സ്ഥിതം ശൂന്യം
ffi_call_int (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
	      ശൂന്യം **avalue, ശൂന്യം *closure)
{
  size_t rsize, bytes;
  ഘടന call_frame *frame;
  അക്ഷരം *അട്ടി, *argp;
  ffi_type **arg_types;
  പൂർണ്ണസംഖ്യ flags, cabi, i, n, dir, narg_reg;
  സ്ഥിരം ഘടന abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  എങ്കിൽ (rvalue == ശൂന്യ)
    {
      തിരഞ്ഞെടുപ്പ് (flags)
	{
	അവസ്ഥ X86_RET_FLOAT:
	അവസ്ഥ X86_RET_DOUBLE:
	അവസ്ഥ X86_RET_LDOUBLE:
	അവസ്ഥ X86_RET_STRUCTPOP:
	അവസ്ഥ X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  നിർത്തുക;
	}
    }

  bytes = അട്ടി_ALIGN (cif->bytes);
  അട്ടി = alloca(bytes + വലുപ്പം(*frame) + rsize);
  argp = (dir < 0 ? അട്ടി + bytes : അട്ടി);
  frame = (ഘടന call_frame *)(അട്ടി + bytes);
  എങ്കിൽ (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;
  frame->regs[pabi->static_chain] = (unsigned)closure;

  narg_reg = 0;
  തിരഞ്ഞെടുപ്പ് (flags)
    {
    അവസ്ഥ X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      എങ്കിൽ (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  നിർത്തുക;
	}
      /* fallthru */
    അവസ്ഥ X86_RET_STRUCTPOP:
      *(ശൂന്യം **)argp = rvalue;
      argp += വലുപ്പം(ശൂന്യം *);
      നിർത്തുക;
    }

  arg_types = cif->arg_types;
  വേണ്ടി (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ശൂന്യം *valp = avalue[i];
      size_t z = ty->size;
      പൂർണ്ണസംഖ്യ t = ty->type;

      എങ്കിൽ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  എങ്കിൽ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    frame->regs[pabi->regs[narg_reg++]] = val;
	  അല്ലെങ്കിൽ എങ്കിൽ (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  അല്ലെങ്കിൽ
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      അല്ലെങ്കിൽ
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  എങ്കിൽ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  എങ്കിൽ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  എങ്കിൽ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      memcpy (argp, valp, z);
	    }
	  അല്ലെങ്കിൽ
	    {
	      argp = (അക്ഷരം *)FFI_ALIGN (argp, align);
	      memcpy (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == അട്ടി);

  ffi_call_i386 (frame, അട്ടി);
}
#എങ്കിൽ defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ശൂന്യ);
}

#ifdef FFI_GO_CLOSURES
ശൂന്യം
ffi_call_go (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
	     ശൂന്യം **avalue, ശൂന്യം *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

/** private members **/

ശൂന്യം FFI_HIDDEN ffi_closure_i386(ശൂന്യം);
ശൂന്യം FFI_HIDDEN ffi_closure_STDCALL(ശൂന്യം);
ശൂന്യം FFI_HIDDEN ffi_closure_REGISTER(ശൂന്യം);
#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
ശൂന്യം FFI_HIDDEN ffi_closure_i386_alt(ശൂന്യം);
ശൂന്യം FFI_HIDDEN ffi_closure_STDCALL_alt(ശൂന്യം);
ശൂന്യം FFI_HIDDEN ffi_closure_REGISTER_alt(ശൂന്യം);
#endif

ഘടന closure_frame
{
  unsigned rettemp[4];				/* 0 */
  unsigned regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*);	/* 32 */
  ശൂന്യം *user_data;				/* 36 */
};

പൂർണ്ണസംഖ്യ FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_closure_inner (ഘടന closure_frame *frame, അക്ഷരം *അട്ടി)
{
  ffi_cif *cif = frame->cif;
  പൂർണ്ണസംഖ്യ cabi, i, n, flags, dir, narg_reg;
  സ്ഥിരം ഘടന abi_params *pabi;
  ffi_type **arg_types;
  അക്ഷരം *argp;
  ശൂന്യം *rvalue;
  ശൂന്യം **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = frame->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? അട്ടി + അട്ടി_ALIGN (cif->bytes) : അട്ടി);

  തിരഞ്ഞെടുപ്പ് (flags)
    {
    അവസ്ഥ X86_RET_STRUCTARG:
      എങ്കിൽ (pabi->nregs > 0)
	{
	  rvalue = (ശൂന്യം *)frame->regs[pabi->regs[0]];
	  narg_reg = 1;
	  frame->rettemp[0] = (unsigned)rvalue;
	  നിർത്തുക;
	}
      /* fallthru */
    അവസ്ഥ X86_RET_STRUCTPOP:
      rvalue = *(ശൂന്യം **)argp;
      argp += വലുപ്പം(ശൂന്യം *);
      frame->rettemp[0] = (unsigned)rvalue;
      നിർത്തുക;
    }

  n = cif->nargs;
  avalue = alloca(വലുപ്പം(ശൂന്യം *) * n);

  arg_types = cif->arg_types;
  വേണ്ടി (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      പൂർണ്ണസംഖ്യ t = ty->type;
      ശൂന്യം *valp;

      എങ്കിൽ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  എങ്കിൽ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &frame->regs[pabi->regs[narg_reg++]];
	  അല്ലെങ്കിൽ എങ്കിൽ (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  അല്ലെങ്കിൽ
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      അല്ലെങ്കിൽ
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  എങ്കിൽ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  എങ്കിൽ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  എങ്കിൽ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  അല്ലെങ്കിൽ
	    {
	      argp = (അക്ഷരം *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  frame->fun (cif, rvalue, avalue, frame->user_data);

  തിരഞ്ഞെടുപ്പ് (cabi)
    {
    അവസ്ഥ FFI_STDCALL:
      തിരികെ flags | (cif->bytes << X86_RET_എടുക്കുക_SHIFT);
    അവസ്ഥ FFI_THISCALL:
    അവസ്ഥ FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      തിരികെ flags | (((unsigned) (argp - അട്ടി)) << X86_RET_എടുക്കുക_SHIFT);
    സ്ഥിരസ്ഥിതി:
      തിരികെ flags;
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
                      ശൂന്യം *user_data,
                      ശൂന്യം *codeloc)
{
  അക്ഷരം *tramp = closure->tramp;
  ശൂന്യം (*dest)(ശൂന്യം);
  പൂർണ്ണസംഖ്യ op = 0xb8;  /* movl imm, %eax */

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_SYSV:
    അവസ്ഥ FFI_MS_CDECL:
      dest = ffi_closure_i386;
      നിർത്തുക;
    അവസ്ഥ FFI_STDCALL:
    അവസ്ഥ FFI_THISCALL:
    അവസ്ഥ FFI_FASTCALL:
    അവസ്ഥ FFI_PASCAL:
      dest = ffi_closure_STDCALL;
      നിർത്തുക;
    അവസ്ഥ FFI_REGISTER:
      dest = ffi_closure_REGISTER;
      op = 0x68;  /* pushl imm */
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_ABI;
    }

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
  എങ്കിൽ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      എങ്കിൽ (dest == ffi_closure_i386)
        dest = ffi_closure_i386_alt;
      അല്ലെങ്കിൽ എങ്കിൽ (dest == ffi_closure_STDCALL)
        dest = ffi_closure_STDCALL_alt;
      അല്ലെങ്കിൽ
        dest = ffi_closure_REGISTER_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      പോകുക out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(ശൂന്യം **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(unsigned *)(tramp + 10) = (unsigned)dest - ((unsigned)codeloc + 14);

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  തിരികെ FFI_OK;
}

#ifdef FFI_GO_CLOSURES

ശൂന്യം FFI_HIDDEN ffi_go_closure_EAX(ശൂന്യം);
ശൂന്യം FFI_HIDDEN ffi_go_closure_ECX(ശൂന്യം);
ശൂന്യം FFI_HIDDEN ffi_go_closure_STDCALL(ശൂന്യം);

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*))
{
  ശൂന്യം (*dest)(ശൂന്യം);

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_SYSV:
    അവസ്ഥ FFI_MS_CDECL:
      dest = ffi_go_closure_ECX;
      നിർത്തുക;
    അവസ്ഥ FFI_THISCALL:
    അവസ്ഥ FFI_FASTCALL:
      dest = ffi_go_closure_EAX;
      നിർത്തുക;
    അവസ്ഥ FFI_STDCALL:
    അവസ്ഥ FFI_PASCAL:
      dest = ffi_go_closure_STDCALL;
      നിർത്തുക;
    അവസ്ഥ FFI_REGISTER:
    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_ABI;
    }

  closure->tramp = dest;
  closure->cif = cif;
  closure->fun = fun;

  തിരികെ FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#എങ്കിൽ !FFI_NO_RAW_API

ശൂന്യം FFI_HIDDEN ffi_closure_raw_SYSV(ശൂന്യം);
ശൂന്യം FFI_HIDDEN ffi_closure_raw_THISCALL(ശൂന്യം);

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure *closure,
                          ffi_cif *cif,
                          ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ffi_raw*,ശൂന്യം*),
                          ശൂന്യം *user_data,
                          ശൂന്യം *codeloc)
{
  അക്ഷരം *tramp = closure->tramp;
  ശൂന്യം (*dest)(ശൂന്യം);
  പൂർണ്ണസംഖ്യ i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  വേണ്ടി (i = cif->nargs-1; i >= 0; i--)
    തിരഞ്ഞെടുപ്പ് (cif->arg_types[i]->type)
      {
      അവസ്ഥ FFI_TYPE_STRUCT:
      അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	തിരികെ FFI_BAD_TYPEDEF;
      }

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_THISCALL:
      dest = ffi_closure_raw_THISCALL;
      നിർത്തുക;
    അവസ്ഥ FFI_SYSV:
      dest = ffi_closure_raw_SYSV;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(ശൂന്യം **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(unsigned *)(tramp + 6) = (unsigned)dest - ((unsigned)codeloc + 10);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  തിരികെ FFI_OK;
}

ശൂന്യം
ffi_raw_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ffi_raw *avalue)
{
  size_t rsize, bytes;
  ഘടന call_frame *frame;
  അക്ഷരം *അട്ടി, *argp;
  ffi_type **arg_types;
  പൂർണ്ണസംഖ്യ flags, cabi, i, n, narg_reg;
  സ്ഥിരം ഘടന abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  എങ്കിൽ (rvalue == ശൂന്യ)
    {
      തിരഞ്ഞെടുപ്പ് (flags)
	{
	അവസ്ഥ X86_RET_FLOAT:
	അവസ്ഥ X86_RET_DOUBLE:
	അവസ്ഥ X86_RET_LDOUBLE:
	അവസ്ഥ X86_RET_STRUCTPOP:
	അവസ്ഥ X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  നിർത്തുക;
	}
    }

  bytes = അട്ടി_ALIGN (cif->bytes);
  argp = അട്ടി =
      (ശൂന്യം *)((uintptr_t)alloca(bytes + വലുപ്പം(*frame) + rsize + 15) & ~16);
  frame = (ഘടന call_frame *)(അട്ടി + bytes);
  എങ്കിൽ (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;

  narg_reg = 0;
  തിരഞ്ഞെടുപ്പ് (flags)
    {
    അവസ്ഥ X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      എങ്കിൽ (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  നിർത്തുക;
	}
      /* fallthru */
    അവസ്ഥ X86_RET_STRUCTPOP:
      *(ശൂന്യം **)argp = rvalue;
      argp += വലുപ്പം(ശൂന്യം *);
      bytes -= വലുപ്പം(ശൂന്യം *);
      നിർത്തുക;
    }

  arg_types = cif->arg_types;
  വേണ്ടി (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      പൂർണ്ണസംഖ്യ t = ty->type;

      എങ്കിൽ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  frame->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      അല്ലെങ്കിൽ
	{
	  memcpy (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      bytes -= z;
    }
  എങ്കിൽ (i < n)
    memcpy (argp, avalue, bytes);

  ffi_call_i386 (frame, അട്ടി);
}
#endif /* !FFI_NO_RAW_API */

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
ശൂന്യം *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ബാഹ്യ ശൂന്യം *trampoline_code_table;

  *map_size = X86_TRAMP_MAP_SIZE;
  *tramp_size = X86_TRAMP_SIZE;
  തിരികെ &trampoline_code_table;
}
#endif

#endif /* __i386__ */
