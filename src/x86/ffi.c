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

#ਜੇ defined(__i386__) || defined(_M_IX86)
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ਜੇ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ਬਾਹਰਲਾ ਦਾ date
# endif
#ਨਹੀਂਤਾਂ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#ਜੇ_ਅੰਤ

#ਜੇ defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#ਜੇ_ਅੰਤ

#ਜੇ defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_ALIGN(ਬਾਈਟਾਂ) (ਬਾਈਟਾਂ)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_ALIGN(ਬਾਈਟਾਂ) FFI_ALIGN (ਬਾਈਟਾਂ, 16)
#ਜੇ_ਅੰਤ

/* Perform machine dependent cif processing.  */
ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t ਬਾਈਟਾਂ = 0;
  ਪੂਰਨ i, n, flags, cabi = cif->abi;

  ਚੋਣ (cabi)
    {
    ਹਾਲਤ FFI_SYSV:
    ਹਾਲਤ FFI_STDCALL:
    ਹਾਲਤ FFI_THISCALL:
    ਹਾਲਤ FFI_FASTCALL:
    ਹਾਲਤ FFI_MS_CDECL:
    ਹਾਲਤ FFI_PASCAL:
    ਹਾਲਤ FFI_ਰਜਿਸਟਰ:
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_ABI;
    }

  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_UINT32:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      flags = X86_RET_INT32;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_STRUCT:
      {
#ਜੇ defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t ਮਾਪ = cif->rtype->ਮਾਪ;
        ਜੇ (ਮਾਪ == 1)
          flags = X86_RET_STRUCT_1B;
        ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ == 2)
          flags = X86_RET_STRUCT_2B;
        ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ == 4)
          flags = X86_RET_INT32;
        ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ == 8)
          flags = X86_RET_INT64;
        ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
          {
          do_struct:
            ਚੋਣ (cabi)
              {
              ਹਾਲਤ FFI_THISCALL:
              ਹਾਲਤ FFI_FASTCALL:
              ਹਾਲਤ FFI_STDCALL:
              ਹਾਲਤ FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                ਰੋਕੋ;
              ਮੂਲ:
                flags = X86_RET_STRUCTPOP;
                ਰੋਕੋ;
              }
            /* Allocate space for return value pointer.  */
            ਬਾਈਟਾਂ += FFI_ALIGN (ਆਕਾਰ(ਖਾਲੀ*), FFI_SIZEOF_ARG);
          }
      }
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      ਚੋਣ (cif->rtype->elements[0]->type)
	{
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	  ਜਾਓ do_struct;
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  ਰੋਕੋ;
	ਮੂਲ:
	  ਵਾਪਸ FFI_BAD_TYPEDEF;
	}
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  ਲਈ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#ਜੇ defined(X86_WIN32)
      ਜੇ (cabi == FFI_STDCALL)
        ਬਾਈਟਾਂ = FFI_ALIGN (ਬਾਈਟਾਂ, FFI_SIZEOF_ARG);
      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
        ਬਾਈਟਾਂ = FFI_ALIGN (ਬਾਈਟਾਂ, t->alignment);
      ਬਾਈਟਾਂ += FFI_ALIGN (t->ਮਾਪ, FFI_SIZEOF_ARG);
    }
  cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;

  ਵਾਪਸ FFI_OK;
}

ਸਥਿਰ ffi_arg
extend_basic_type(ਖਾਲੀ *arg, ਪੂਰਨ type)
{
  ਚੋਣ (type)
    {
    ਹਾਲਤ FFI_TYPE_SINT8:
      ਵਾਪਸ *(SINT8 *)arg;
    ਹਾਲਤ FFI_TYPE_UINT8:
      ਵਾਪਸ *(UINT8 *)arg;
    ਹਾਲਤ FFI_TYPE_SINT16:
      ਵਾਪਸ *(SINT16 *)arg;
    ਹਾਲਤ FFI_TYPE_UINT16:
      ਵਾਪਸ *(UINT16 *)arg;

    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_UINT32:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
    ਹਾਲਤ FFI_TYPE_FLOAT:
      ਵਾਪਸ *(UINT32 *)arg;

    ਮੂਲ:
      abort();
    }
}

ਬਣਤਰ call_ਚੌਖਟਾ
{
  ਖਾਲੀ *ebp;		/* 0 */
  ਖਾਲੀ *retaddr;	/* 4 */
  ਖਾਲੀ (*fn)(ਖਾਲੀ);	/* 8 */
  ਪੂਰਨ flags;		/* 12 */
  ਖਾਲੀ *rvalue;		/* 16 */
  ਬਿਨਾਨਿਸ਼ਾਨੀ regs[3];	/* 20-28 */
};

ਬਣਤਰ abi_params
{
  ਪੂਰਨ dir;		/* parameter growth direction */
  ਪੂਰਨ static_chain;	/* the static chain register used by gcc */
  ਪੂਰਨ nregs;		/* number of register parameters */
  ਪੂਰਨ regs[3];
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ abi_params abi_params[FFI_ਆਖਰੀ_ABI] = {
  [FFI_SYSV] = { 1, R_ECX, 0 },
  [FFI_THISCALL] = { 1, R_EAX, 1, { R_ECX } },
  [FFI_FASTCALL] = { 1, R_EAX, 2, { R_ECX, R_EDX } },
  [FFI_STDCALL] = { 1, R_ECX, 0 },
  [FFI_PASCAL] = { -1, R_ECX, 0 },
  /* ??? No defined static chain; gcc does not support REGISTER.  */
  [FFI_ਰਜਿਸਟਰ] = { -1, R_ECX, 3, { R_EAX, R_EDX, R_ECX } },
  [FFI_MS_CDECL] = { 1, R_ECX, 0 }
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_FASTCALL
  #ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_DECLARE_FASTCALL __fastcall
  #ਨਹੀਂਤਾਂ
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_DECLARE_FASTCALL __declspec(fastcall)
  #ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_DECLARE_FASTCALL
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਖਾਲੀ FFI_DECLARE_FASTCALL ffi_call_i386(ਬਣਤਰ call_ਚੌਖਟਾ *, ਅੱਖਰ *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ਜੇ defined(_MSC_VER)
#ਹਦਾਇਤ runtime_checks("s", off)
#ਜੇ_ਅੰਤ
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	      ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  size_t rsize, ਬਾਈਟਾਂ;
  ਬਣਤਰ call_ਚੌਖਟਾ *ਚੌਖਟਾ;
  ਅੱਖਰ *ਢੇਰ, *argp;
  ffi_type **arg_types;
  ਪੂਰਨ flags, cabi, i, n, dir, narg_reg;
  ਅਚਲ ਬਣਤਰ abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  ਜੇ (rvalue == ਨਿਰਾ)
    {
      ਚੋਣ (flags)
	{
	ਹਾਲਤ X86_RET_FLOAT:
	ਹਾਲਤ X86_RET_DOUBLE:
	ਹਾਲਤ X86_RET_LDOUBLE:
	ਹਾਲਤ X86_RET_STRUCTPOP:
	ਹਾਲਤ X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->ਮਾਪ;
	  ਰੋਕੋ;
	ਮੂਲ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  ਰੋਕੋ;
	}
    }

  ਬਾਈਟਾਂ = ਢੇਰ_ALIGN (cif->ਬਾਈਟਾਂ);
  ਢੇਰ = alloca(ਬਾਈਟਾਂ + ਆਕਾਰ(*ਚੌਖਟਾ) + rsize);
  argp = (dir < 0 ? ਢੇਰ + ਬਾਈਟਾਂ : ਢੇਰ);
  ਚੌਖਟਾ = (ਬਣਤਰ call_ਚੌਖਟਾ *)(ਢੇਰ + ਬਾਈਟਾਂ);
  ਜੇ (rsize)
    rvalue = ਚੌਖਟਾ + 1;

  ਚੌਖਟਾ->fn = fn;
  ਚੌਖਟਾ->flags = flags;
  ਚੌਖਟਾ->rvalue = rvalue;
  ਚੌਖਟਾ->regs[pabi->static_chain] = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਘੇਰਾ;

  narg_reg = 0;
  ਚੋਣ (flags)
    {
    ਹਾਲਤ X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ਜੇ (pabi->nregs > 0)
	{
	  ਚੌਖਟਾ->regs[pabi->regs[0]] = (ਬਿਨਾਨਿਸ਼ਾਨੀ)rvalue;
	  narg_reg = 1;
	  ਰੋਕੋ;
	}
      /* fallthru */
    ਹਾਲਤ X86_RET_STRUCTPOP:
      *(ਖਾਲੀ **)argp = rvalue;
      argp += ਆਕਾਰ(ਖਾਲੀ *);
      ਰੋਕੋ;
    }

  arg_types = cif->arg_types;
  ਲਈ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ਖਾਲੀ *valp = avalue[i];
      size_t z = ty->ਮਾਪ;
      ਪੂਰਨ t = ty->type;

      ਜੇ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  ਜੇ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    ਚੌਖਟਾ->regs[pabi->regs[narg_reg++]] = val;
	  ਨਹੀਂਤਾਂ ਜੇ (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      ਨਹੀਂਤਾਂ
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ਜੇ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  ਜੇ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  ਜੇ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, valp, z);
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      argp = (ਅੱਖਰ *)FFI_ALIGN (argp, align);
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == ਢੇਰ);

  ffi_call_i386 (ਚੌਖਟਾ, ਢੇਰ);
}
#ਜੇ defined(_MSC_VER)
#ਹਦਾਇਤ runtime_checks("s", restore)
#ਜੇ_ਅੰਤ

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਨਿਰਾ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES
ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਘੇਰਾ);
}
#ਜੇ_ਅੰਤ

/** private members **/

ਖਾਲੀ FFI_HIDDEN ffi_ਘੇਰਾ_i386(ਖਾਲੀ);
ਖਾਲੀ FFI_HIDDEN ffi_ਘੇਰਾ_STDCALL(ਖਾਲੀ);
ਖਾਲੀ FFI_HIDDEN ffi_ਘੇਰਾ_ਰਜਿਸਟਰ(ਖਾਲੀ);
#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਖਾਲੀ FFI_HIDDEN ffi_ਘੇਰਾ_i386_alt(ਖਾਲੀ);
ਖਾਲੀ FFI_HIDDEN ffi_ਘੇਰਾ_STDCALL_alt(ਖਾਲੀ);
ਖਾਲੀ FFI_HIDDEN ffi_ਘੇਰਾ_ਰਜਿਸਟਰ_alt(ਖਾਲੀ);
#ਜੇ_ਅੰਤ

ਬਣਤਰ ਘੇਰਾ_ਚੌਖਟਾ
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ rettemp[4];				/* 0 */
  ਬਿਨਾਨਿਸ਼ਾਨੀ regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*);	/* 32 */
  ਖਾਲੀ *user_ਅੰਕੜੇ;				/* 36 */
};

ਪੂਰਨ FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_ਘੇਰਾ_inner (ਬਣਤਰ ਘੇਰਾ_ਚੌਖਟਾ *ਚੌਖਟਾ, ਅੱਖਰ *ਢੇਰ)
{
  ffi_cif *cif = ਚੌਖਟਾ->cif;
  ਪੂਰਨ cabi, i, n, flags, dir, narg_reg;
  ਅਚਲ ਬਣਤਰ abi_params *pabi;
  ffi_type **arg_types;
  ਅੱਖਰ *argp;
  ਖਾਲੀ *rvalue;
  ਖਾਲੀ **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = ਚੌਖਟਾ->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? ਢੇਰ + ਢੇਰ_ALIGN (cif->ਬਾਈਟਾਂ) : ਢੇਰ);

  ਚੋਣ (flags)
    {
    ਹਾਲਤ X86_RET_STRUCTARG:
      ਜੇ (pabi->nregs > 0)
	{
	  rvalue = (ਖਾਲੀ *)ਚੌਖਟਾ->regs[pabi->regs[0]];
	  narg_reg = 1;
	  ਚੌਖਟਾ->rettemp[0] = (ਬਿਨਾਨਿਸ਼ਾਨੀ)rvalue;
	  ਰੋਕੋ;
	}
      /* fallthru */
    ਹਾਲਤ X86_RET_STRUCTPOP:
      rvalue = *(ਖਾਲੀ **)argp;
      argp += ਆਕਾਰ(ਖਾਲੀ *);
      ਚੌਖਟਾ->rettemp[0] = (ਬਿਨਾਨਿਸ਼ਾਨੀ)rvalue;
      ਰੋਕੋ;
    }

  n = cif->nargs;
  avalue = alloca(ਆਕਾਰ(ਖਾਲੀ *) * n);

  arg_types = cif->arg_types;
  ਲਈ (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->ਮਾਪ;
      ਪੂਰਨ t = ty->type;
      ਖਾਲੀ *valp;

      ਜੇ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  ਜੇ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &ਚੌਖਟਾ->regs[pabi->regs[narg_reg++]];
	  ਨਹੀਂਤਾਂ ਜੇ (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      ਨਹੀਂਤਾਂ
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  ਜੇ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ਜੇ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  ਜੇ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      argp = (ਅੱਖਰ *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  ਚੌਖਟਾ->fun (cif, rvalue, avalue, ਚੌਖਟਾ->user_ਅੰਕੜੇ);

  ਚੋਣ (cabi)
    {
    ਹਾਲਤ FFI_STDCALL:
      ਵਾਪਸ flags | (cif->ਬਾਈਟਾਂ << X86_RET_ਕੱਢੋ_ਖਿਸਕਾਓ);
    ਹਾਲਤ FFI_THISCALL:
    ਹਾਲਤ FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      ਵਾਪਸ flags | (((ਬਿਨਾਨਿਸ਼ਾਨੀ) (argp - ਢੇਰ)) << X86_RET_ਕੱਢੋ_ਖਿਸਕਾਓ);
    ਮੂਲ:
      ਵਾਪਸ flags;
    }
}

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
                      ffi_cif* cif,
                      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
                      ਖਾਲੀ *user_ਅੰਕੜੇ,
                      ਖਾਲੀ *codeloc)
{
  ਅੱਖਰ *tramp = ਘੇਰਾ->tramp;
  ਖਾਲੀ (*dest)(ਖਾਲੀ);
  ਪੂਰਨ op = 0xb8;  /* movl imm, %eax */

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_SYSV:
    ਹਾਲਤ FFI_MS_CDECL:
      dest = ffi_ਘੇਰਾ_i386;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_STDCALL:
    ਹਾਲਤ FFI_THISCALL:
    ਹਾਲਤ FFI_FASTCALL:
    ਹਾਲਤ FFI_PASCAL:
      dest = ffi_ਘੇਰਾ_STDCALL;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_ਰਜਿਸਟਰ:
      dest = ffi_ਘੇਰਾ_ਰਜਿਸਟਰ;
      op = 0x68;  /* pushl imm */
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_ABI;
    }

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
  ਜੇ (ffi_tramp_is_present(ਘੇਰਾ))
    {
      /* Initialize the static trampoline's parameters. */
      ਜੇ (dest == ffi_ਘੇਰਾ_i386)
        dest = ffi_ਘੇਰਾ_i386_alt;
      ਨਹੀਂਤਾਂ ਜੇ (dest == ffi_ਘੇਰਾ_STDCALL)
        dest = ffi_ਘੇਰਾ_STDCALL_alt;
      ਨਹੀਂਤਾਂ
        dest = ffi_ਘੇਰਾ_ਰਜਿਸਟਰ_alt;
      ffi_tramp_ਸੈੱਟ_parms (ਘੇਰਾ->ftramp, dest, ਘੇਰਾ);
      ਜਾਓ ਬਾਹਰਲਾ;
    }
#ਜੇ_ਅੰਤ

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(ਖਾਲੀ **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(ਬਿਨਾਨਿਸ਼ਾਨੀ *)(tramp + 10) = (ਬਿਨਾਨਿਸ਼ਾਨੀ)dest - ((ਬਿਨਾਨਿਸ਼ਾਨੀ)codeloc + 14);

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਬਾਹਰਲਾ:
#ਜੇ_ਅੰਤ
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ਵਾਪਸ FFI_OK;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES

ਖਾਲੀ FFI_HIDDEN ffi_go_ਘੇਰਾ_EAX(ਖਾਲੀ);
ਖਾਲੀ FFI_HIDDEN ffi_go_ਘੇਰਾ_ECX(ਖਾਲੀ);
ਖਾਲੀ FFI_HIDDEN ffi_go_ਘੇਰਾ_STDCALL(ਖਾਲੀ);

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ* ਘੇਰਾ, ffi_cif* cif,
		     ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*))
{
  ਖਾਲੀ (*dest)(ਖਾਲੀ);

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_SYSV:
    ਹਾਲਤ FFI_MS_CDECL:
      dest = ffi_go_ਘੇਰਾ_ECX;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_THISCALL:
    ਹਾਲਤ FFI_FASTCALL:
      dest = ffi_go_ਘੇਰਾ_EAX;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_STDCALL:
    ਹਾਲਤ FFI_PASCAL:
      dest = ffi_go_ਘੇਰਾ_STDCALL;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_ਰਜਿਸਟਰ:
    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_ABI;
    }

  ਘੇਰਾ->tramp = dest;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}

#ਜੇ_ਅੰਤ /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#ਜੇ !FFI_NO_RAW_API

ਖਾਲੀ FFI_HIDDEN ffi_ਘੇਰਾ_raw_SYSV(ਖਾਲੀ);
ਖਾਲੀ FFI_HIDDEN ffi_ਘੇਰਾ_raw_THISCALL(ਖਾਲੀ);

ffi_ਸਥਿਤੀ
ffi_prep_raw_ਘੇਰਾ_loc (ffi_raw_ਘੇਰਾ *ਘੇਰਾ,
                          ffi_cif *cif,
                          ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ffi_raw*,ਖਾਲੀ*),
                          ਖਾਲੀ *user_ਅੰਕੜੇ,
                          ਖਾਲੀ *codeloc)
{
  ਅੱਖਰ *tramp = ਘੇਰਾ->tramp;
  ਖਾਲੀ (*dest)(ਖਾਲੀ);
  ਪੂਰਨ i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  ਲਈ (i = cif->nargs-1; i >= 0; i--)
    ਚੋਣ (cif->arg_types[i]->type)
      {
      ਹਾਲਤ FFI_TYPE_STRUCT:
      ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	ਵਾਪਸ FFI_BAD_TYPEDEF;
      }

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_THISCALL:
      dest = ffi_ਘੇਰਾ_raw_THISCALL;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_SYSV:
      dest = ffi_ਘੇਰਾ_raw_SYSV;
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(ਖਾਲੀ **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(ਬਿਨਾਨਿਸ਼ਾਨੀ *)(tramp + 6) = (ਬਿਨਾਨਿਸ਼ਾਨੀ)dest - ((ਬਿਨਾਨਿਸ਼ਾਨੀ)codeloc + 10);

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ਵਾਪਸ FFI_OK;
}

ਖਾਲੀ
ffi_raw_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ffi_raw *avalue)
{
  size_t rsize, ਬਾਈਟਾਂ;
  ਬਣਤਰ call_ਚੌਖਟਾ *ਚੌਖਟਾ;
  ਅੱਖਰ *ਢੇਰ, *argp;
  ffi_type **arg_types;
  ਪੂਰਨ flags, cabi, i, n, narg_reg;
  ਅਚਲ ਬਣਤਰ abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  ਜੇ (rvalue == ਨਿਰਾ)
    {
      ਚੋਣ (flags)
	{
	ਹਾਲਤ X86_RET_FLOAT:
	ਹਾਲਤ X86_RET_DOUBLE:
	ਹਾਲਤ X86_RET_LDOUBLE:
	ਹਾਲਤ X86_RET_STRUCTPOP:
	ਹਾਲਤ X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->ਮਾਪ;
	  ਰੋਕੋ;
	ਮੂਲ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  ਰੋਕੋ;
	}
    }

  ਬਾਈਟਾਂ = ਢੇਰ_ALIGN (cif->ਬਾਈਟਾਂ);
  argp = ਢੇਰ =
      (ਖਾਲੀ *)((uintptr_t)alloca(ਬਾਈਟਾਂ + ਆਕਾਰ(*ਚੌਖਟਾ) + rsize + 15) & ~16);
  ਚੌਖਟਾ = (ਬਣਤਰ call_ਚੌਖਟਾ *)(ਢੇਰ + ਬਾਈਟਾਂ);
  ਜੇ (rsize)
    rvalue = ਚੌਖਟਾ + 1;

  ਚੌਖਟਾ->fn = fn;
  ਚੌਖਟਾ->flags = flags;
  ਚੌਖਟਾ->rvalue = rvalue;

  narg_reg = 0;
  ਚੋਣ (flags)
    {
    ਹਾਲਤ X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ਜੇ (pabi->nregs > 0)
	{
	  ਚੌਖਟਾ->regs[pabi->regs[0]] = (ਬਿਨਾਨਿਸ਼ਾਨੀ)rvalue;
	  narg_reg = 1;
	  ਰੋਕੋ;
	}
      /* fallthru */
    ਹਾਲਤ X86_RET_STRUCTPOP:
      *(ਖਾਲੀ **)argp = rvalue;
      argp += ਆਕਾਰ(ਖਾਲੀ *);
      ਬਾਈਟਾਂ -= ਆਕਾਰ(ਖਾਲੀ *);
      ਰੋਕੋ;
    }

  arg_types = cif->arg_types;
  ਲਈ (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->ਮਾਪ;
      ਪੂਰਨ t = ty->type;

      ਜੇ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  ਚੌਖਟਾ->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      ਨਹੀਂਤਾਂ
	{
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      ਬਾਈਟਾਂ -= z;
    }
  ਜੇ (i < n)
    ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, avalue, ਬਾਈਟਾਂ);

  ffi_call_i386 (ਚੌਖਟਾ, ਢੇਰ);
}
#ਜੇ_ਅੰਤ /* !FFI_NO_RAW_API */

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਖਾਲੀ *
ffi_tramp_arch (size_t *tramp_ਮਾਪ, size_t *map_ਮਾਪ)
{
  ਬਾਹਰੀ ਖਾਲੀ *trampoline_ਕੋਡ_ਸਾਰਣੀ;

  *map_ਮਾਪ = X86_TRAMP_MAP_ਮਾਪ;
  *tramp_ਮਾਪ = X86_TRAMP_ਮਾਪ;
  ਵਾਪਸ &trampoline_ਕੋਡ_ਸਾਰਣੀ;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* __i386__ */
