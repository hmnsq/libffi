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

#ಯದಿ defined(__i386__) || defined(_M_IX86)
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>
#ಅನ್ತರ್ಭಾವ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ಯದಿ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#ಅನ್ಯಥಾ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#ಯದಿ defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#endif

#ಯದಿ defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#define ರಾಶಿ_ALIGN(bytes) (bytes)
#ಅನ್ಯಥಾ
#define ರಾಶಿ_ALIGN(bytes) FFI_ALIGN (bytes, 16)
#endif

/* Perform machine dependent cif processing.  */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  ಪೂರ್ಣಾಙ್ಕ i, n, flags, cabi = cif->abi;

  ವಿಕಲ್ಪ (cabi)
    {
    ಪಕ್ಷ FFI_SYSV:
    ಪಕ್ಷ FFI_STDCALL:
    ಪಕ್ಷ FFI_THISCALL:
    ಪಕ್ಷ FFI_FASTCALL:
    ಪಕ್ಷ FFI_MS_CDECL:
    ಪಕ್ಷ FFI_PASCAL:
    ಪಕ್ಷ FFI_REGISTER:
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
    }

  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_SINT32:
    ಪಕ್ಷ FFI_TYPE_UINT32:
    ಪಕ್ಷ FFI_TYPE_POINTER:
      flags = X86_RET_INT32;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_STRUCT:
      {
#ಯದಿ defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t size = cif->rtype->size;
        ಯದಿ (size == 1)
          flags = X86_RET_STRUCT_1B;
        ಅನ್ಯಥಾ ಯದಿ (size == 2)
          flags = X86_RET_STRUCT_2B;
        ಅನ್ಯಥಾ ಯದಿ (size == 4)
          flags = X86_RET_INT32;
        ಅನ್ಯಥಾ ಯದಿ (size == 8)
          flags = X86_RET_INT64;
        ಅನ್ಯಥಾ
#endif
          {
          do_struct:
            ವಿಕಲ್ಪ (cabi)
              {
              ಪಕ್ಷ FFI_THISCALL:
              ಪಕ್ಷ FFI_FASTCALL:
              ಪಕ್ಷ FFI_STDCALL:
              ಪಕ್ಷ FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                ಭಙ್ಗ;
              ಸ್ವಭಾವ:
                flags = X86_RET_STRUCTPOP;
                ಭಙ್ಗ;
              }
            /* Allocate space for return value pointer.  */
            bytes += FFI_ALIGN (ಪರಿಮಾಣ(ಶೂನ್ಯ*), FFI_SIZEOF_ARG);
          }
      }
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      ವಿಕಲ್ಪ (cif->rtype->elements[0]->type)
	{
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	  ಗಚ್ಛ do_struct;
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  ಭಙ್ಗ;
	ಸ್ವಭಾವ:
	  ಪ್ರತ್ಯಾಗಮ FFI_BAD_TYPEDEF;
	}
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#ಯದಿ defined(X86_WIN32)
      ಯದಿ (cabi == FFI_STDCALL)
        bytes = FFI_ALIGN (bytes, FFI_SIZEOF_ARG);
      ಅನ್ಯಥಾ
#endif
        bytes = FFI_ALIGN (bytes, t->alignment);
      bytes += FFI_ALIGN (t->size, FFI_SIZEOF_ARG);
    }
  cif->bytes = bytes;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಸ್ಥಿರ ffi_arg
extend_basic_type(ಶೂನ್ಯ *arg, ಪೂರ್ಣಾಙ್ಕ type)
{
  ವಿಕಲ್ಪ (type)
    {
    ಪಕ್ಷ FFI_TYPE_SINT8:
      ಪ್ರತ್ಯಾಗಮ *(SINT8 *)arg;
    ಪಕ್ಷ FFI_TYPE_UINT8:
      ಪ್ರತ್ಯಾಗಮ *(UINT8 *)arg;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      ಪ್ರತ್ಯಾಗಮ *(SINT16 *)arg;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      ಪ್ರತ್ಯಾಗಮ *(UINT16 *)arg;

    ಪಕ್ಷ FFI_TYPE_SINT32:
    ಪಕ್ಷ FFI_TYPE_UINT32:
    ಪಕ್ಷ FFI_TYPE_POINTER:
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      ಪ್ರತ್ಯಾಗಮ *(UINT32 *)arg;

    ಸ್ವಭಾವ:
      abort();
    }
}

ಸಂರಚನಾ call_frame
{
  ಶೂನ್ಯ *ebp;		/* 0 */
  ಶೂನ್ಯ *retaddr;	/* 4 */
  ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ);	/* 8 */
  ಪೂರ್ಣಾಙ್ಕ flags;		/* 12 */
  ಶೂನ್ಯ *rvalue;		/* 16 */
  unsigned regs[3];	/* 20-28 */
};

ಸಂರಚನಾ abi_params
{
  ಪೂರ್ಣಾಙ್ಕ dir;		/* parameter growth direction */
  ಪೂರ್ಣಾಙ್ಕ static_chain;	/* the static chain register used by gcc */
  ಪೂರ್ಣಾಙ್ಕ nregs;		/* number of register parameters */
  ಪೂರ್ಣಾಙ್ಕ regs[3];
};

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ abi_params abi_params[FFI_LAST_ABI] = {
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
  #ಅನ್ಯಥಾ
    #define FFI_DECLARE_FASTCALL __declspec(fastcall)
  #endif
#ಅನ್ಯಥಾ
  #define FFI_DECLARE_FASTCALL
#endif

ಬಾಹ್ಯ ಶೂನ್ಯ FFI_DECLARE_FASTCALL ffi_call_i386(ಸಂರಚನಾ call_frame *, ವರ್ಣ *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ಯದಿ defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  size_t rsize, bytes;
  ಸಂರಚನಾ call_frame *frame;
  ವರ್ಣ *ರಾಶಿ, *argp;
  ffi_type **arg_types;
  ಪೂರ್ಣಾಙ್ಕ flags, cabi, i, n, dir, narg_reg;
  ಧ್ರುವ ಸಂರಚನಾ abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  ಯದಿ (rvalue == ರಿಕ್ತ)
    {
      ವಿಕಲ್ಪ (flags)
	{
	ಪಕ್ಷ X86_RET_FLOAT:
	ಪಕ್ಷ X86_RET_DOUBLE:
	ಪಕ್ಷ X86_RET_LDOUBLE:
	ಪಕ್ಷ X86_RET_STRUCTPOP:
	ಪಕ್ಷ X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  ಭಙ್ಗ;
	ಸ್ವಭಾವ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  ಭಙ್ಗ;
	}
    }

  bytes = ರಾಶಿ_ALIGN (cif->bytes);
  ರಾಶಿ = alloca(bytes + ಪರಿಮಾಣ(*frame) + rsize);
  argp = (dir < 0 ? ರಾಶಿ + bytes : ರಾಶಿ);
  frame = (ಸಂರಚನಾ call_frame *)(ರಾಶಿ + bytes);
  ಯದಿ (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;
  frame->regs[pabi->static_chain] = (unsigned)closure;

  narg_reg = 0;
  ವಿಕಲ್ಪ (flags)
    {
    ಪಕ್ಷ X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ಯದಿ (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  ಭಙ್ಗ;
	}
      /* fallthru */
    ಪಕ್ಷ X86_RET_STRUCTPOP:
      *(ಶೂನ್ಯ **)argp = rvalue;
      argp += ಪರಿಮಾಣ(ಶೂನ್ಯ *);
      ಭಙ್ಗ;
    }

  arg_types = cif->arg_types;
  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ಶೂನ್ಯ *valp = avalue[i];
      size_t z = ty->size;
      ಪೂರ್ಣಾಙ್ಕ t = ty->type;

      ಯದಿ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  ಯದಿ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    frame->regs[pabi->regs[narg_reg++]] = val;
	  ಅನ್ಯಥಾ ಯದಿ (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  ಅನ್ಯಥಾ
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      ಅನ್ಯಥಾ
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ಯದಿ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  ಯದಿ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  ಯದಿ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      memcpy (argp, valp, z);
	    }
	  ಅನ್ಯಥಾ
	    {
	      argp = (ವರ್ಣ *)FFI_ALIGN (argp, align);
	      memcpy (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == ರಾಶಿ);

  ffi_call_i386 (frame, ರಾಶಿ);
}
#ಯದಿ defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ರಿಕ್ತ);
}

#ifdef FFI_GO_CLOSURES
ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

/** private members **/

ಶೂನ್ಯ FFI_HIDDEN ffi_closure_i386(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_closure_STDCALL(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_closure_REGISTER(ಶೂನ್ಯ);
#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ FFI_HIDDEN ffi_closure_i386_alt(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_closure_STDCALL_alt(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_closure_REGISTER_alt(ಶೂನ್ಯ);
#endif

ಸಂರಚನಾ closure_frame
{
  unsigned rettemp[4];				/* 0 */
  unsigned regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*);	/* 32 */
  ಶೂನ್ಯ *user_data;				/* 36 */
};

ಪೂರ್ಣಾಙ್ಕ FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_closure_inner (ಸಂರಚನಾ closure_frame *frame, ವರ್ಣ *ರಾಶಿ)
{
  ffi_cif *cif = frame->cif;
  ಪೂರ್ಣಾಙ್ಕ cabi, i, n, flags, dir, narg_reg;
  ಧ್ರುವ ಸಂರಚನಾ abi_params *pabi;
  ffi_type **arg_types;
  ವರ್ಣ *argp;
  ಶೂನ್ಯ *rvalue;
  ಶೂನ್ಯ **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = frame->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? ರಾಶಿ + ರಾಶಿ_ALIGN (cif->bytes) : ರಾಶಿ);

  ವಿಕಲ್ಪ (flags)
    {
    ಪಕ್ಷ X86_RET_STRUCTARG:
      ಯದಿ (pabi->nregs > 0)
	{
	  rvalue = (ಶೂನ್ಯ *)frame->regs[pabi->regs[0]];
	  narg_reg = 1;
	  frame->rettemp[0] = (unsigned)rvalue;
	  ಭಙ್ಗ;
	}
      /* fallthru */
    ಪಕ್ಷ X86_RET_STRUCTPOP:
      rvalue = *(ಶೂನ್ಯ **)argp;
      argp += ಪರಿಮಾಣ(ಶೂನ್ಯ *);
      frame->rettemp[0] = (unsigned)rvalue;
      ಭಙ್ಗ;
    }

  n = cif->nargs;
  avalue = alloca(ಪರಿಮಾಣ(ಶೂನ್ಯ *) * n);

  arg_types = cif->arg_types;
  ಪ್ರತಿ (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      ಪೂರ್ಣಾಙ್ಕ t = ty->type;
      ಶೂನ್ಯ *valp;

      ಯದಿ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  ಯದಿ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &frame->regs[pabi->regs[narg_reg++]];
	  ಅನ್ಯಥಾ ಯದಿ (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  ಅನ್ಯಥಾ
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      ಅನ್ಯಥಾ
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  ಯದಿ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ಯದಿ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  ಯದಿ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  ಅನ್ಯಥಾ
	    {
	      argp = (ವರ್ಣ *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  frame->fun (cif, rvalue, avalue, frame->user_data);

  ವಿಕಲ್ಪ (cabi)
    {
    ಪಕ್ಷ FFI_STDCALL:
      ಪ್ರತ್ಯಾಗಮ flags | (cif->bytes << X86_RET_ತೆಗೆ_SHIFT);
    ಪಕ್ಷ FFI_THISCALL:
    ಪಕ್ಷ FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      ಪ್ರತ್ಯಾಗಮ flags | (((unsigned) (argp - ರಾಶಿ)) << X86_RET_ತೆಗೆ_SHIFT);
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ flags;
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
                      ಶೂನ್ಯ *user_data,
                      ಶೂನ್ಯ *codeloc)
{
  ವರ್ಣ *tramp = closure->tramp;
  ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಙ್ಕ op = 0xb8;  /* movl imm, %eax */

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_SYSV:
    ಪಕ್ಷ FFI_MS_CDECL:
      dest = ffi_closure_i386;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_STDCALL:
    ಪಕ್ಷ FFI_THISCALL:
    ಪಕ್ಷ FFI_FASTCALL:
    ಪಕ್ಷ FFI_PASCAL:
      dest = ffi_closure_STDCALL;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_REGISTER:
      dest = ffi_closure_REGISTER;
      op = 0x68;  /* pushl imm */
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
    }

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
  ಯದಿ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ಯದಿ (dest == ffi_closure_i386)
        dest = ffi_closure_i386_alt;
      ಅನ್ಯಥಾ ಯದಿ (dest == ffi_closure_STDCALL)
        dest = ffi_closure_STDCALL_alt;
      ಅನ್ಯಥಾ
        dest = ffi_closure_REGISTER_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      ಗಚ್ಛ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(ಶೂನ್ಯ **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(unsigned *)(tramp + 10) = (unsigned)dest - ((unsigned)codeloc + 14);

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

#ifdef FFI_GO_CLOSURES

ಶೂನ್ಯ FFI_HIDDEN ffi_go_closure_EAX(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_go_closure_ECX(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_go_closure_STDCALL(ಶೂನ್ಯ);

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*))
{
  ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ);

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_SYSV:
    ಪಕ್ಷ FFI_MS_CDECL:
      dest = ffi_go_closure_ECX;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_THISCALL:
    ಪಕ್ಷ FFI_FASTCALL:
      dest = ffi_go_closure_EAX;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_STDCALL:
    ಪಕ್ಷ FFI_PASCAL:
      dest = ffi_go_closure_STDCALL;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_REGISTER:
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
    }

  closure->tramp = dest;
  closure->cif = cif;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#ಯದಿ !FFI_NO_RAW_API

ಶೂನ್ಯ FFI_HIDDEN ffi_closure_raw_SYSV(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_closure_raw_THISCALL(ಶೂನ್ಯ);

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure *closure,
                          ffi_cif *cif,
                          ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ffi_raw*,ಶೂನ್ಯ*),
                          ಶೂನ್ಯ *user_data,
                          ಶೂನ್ಯ *codeloc)
{
  ವರ್ಣ *tramp = closure->tramp;
  ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಙ್ಕ i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  ಪ್ರತಿ (i = cif->nargs-1; i >= 0; i--)
    ವಿಕಲ್ಪ (cif->arg_types[i]->type)
      {
      ಪಕ್ಷ FFI_TYPE_STRUCT:
      ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	ಪ್ರತ್ಯಾಗಮ FFI_BAD_TYPEDEF;
      }

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_THISCALL:
      dest = ffi_closure_raw_THISCALL;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_SYSV:
      dest = ffi_closure_raw_SYSV;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(ಶೂನ್ಯ **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(unsigned *)(tramp + 6) = (unsigned)dest - ((unsigned)codeloc + 10);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಶೂನ್ಯ
ffi_raw_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ffi_raw *avalue)
{
  size_t rsize, bytes;
  ಸಂರಚನಾ call_frame *frame;
  ವರ್ಣ *ರಾಶಿ, *argp;
  ffi_type **arg_types;
  ಪೂರ್ಣಾಙ್ಕ flags, cabi, i, n, narg_reg;
  ಧ್ರುವ ಸಂರಚನಾ abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  ಯದಿ (rvalue == ರಿಕ್ತ)
    {
      ವಿಕಲ್ಪ (flags)
	{
	ಪಕ್ಷ X86_RET_FLOAT:
	ಪಕ್ಷ X86_RET_DOUBLE:
	ಪಕ್ಷ X86_RET_LDOUBLE:
	ಪಕ್ಷ X86_RET_STRUCTPOP:
	ಪಕ್ಷ X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  ಭಙ್ಗ;
	ಸ್ವಭಾವ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  ಭಙ್ಗ;
	}
    }

  bytes = ರಾಶಿ_ALIGN (cif->bytes);
  argp = ರಾಶಿ =
      (ಶೂನ್ಯ *)((uintptr_t)alloca(bytes + ಪರಿಮಾಣ(*frame) + rsize + 15) & ~16);
  frame = (ಸಂರಚನಾ call_frame *)(ರಾಶಿ + bytes);
  ಯದಿ (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;

  narg_reg = 0;
  ವಿಕಲ್ಪ (flags)
    {
    ಪಕ್ಷ X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ಯದಿ (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  ಭಙ್ಗ;
	}
      /* fallthru */
    ಪಕ್ಷ X86_RET_STRUCTPOP:
      *(ಶೂನ್ಯ **)argp = rvalue;
      argp += ಪರಿಮಾಣ(ಶೂನ್ಯ *);
      bytes -= ಪರಿಮಾಣ(ಶೂನ್ಯ *);
      ಭಙ್ಗ;
    }

  arg_types = cif->arg_types;
  ಪ್ರತಿ (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      ಪೂರ್ಣಾಙ್ಕ t = ty->type;

      ಯದಿ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  frame->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      ಅನ್ಯಥಾ
	{
	  memcpy (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      bytes -= z;
    }
  ಯದಿ (i < n)
    memcpy (argp, avalue, bytes);

  ffi_call_i386 (frame, ರಾಶಿ);
}
#endif /* !FFI_NO_RAW_API */

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_code_table;

  *map_size = X86_TRAMP_MAP_SIZE;
  *tramp_size = X86_TRAMP_SIZE;
  ಪ್ರತ್ಯಾಗಮ &trampoline_code_table;
}
#endif

#endif /* __i386__ */
