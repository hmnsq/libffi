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

#ಆದರೆ defined(__i386__) || defined(_M_IX86)
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>
#ಅನ್ತರ್ಭಾವ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ಆದರೆ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ಹೊರಹೋಗು ದ date
# endif
#ಇಲ್ಲದಿದ್ದರೆ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_ALIGN(ಬೈಟುಗಳು) (ಬೈಟುಗಳು)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_ALIGN(ಬೈಟುಗಳು) FFI_ALIGN (ಬೈಟುಗಳು, 16)
#ಆದರೆ_ಅಂತ್ಯ

/* Perform machine dependent cif processing.  */
ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t ಬೈಟುಗಳು = 0;
  ಪೂರ್ಣಾಂಕ i, n, flags, cabi = cif->abi;

  ಆಯ್ಕೆ (cabi)
    {
    ಸ್ಥಿತಿ FFI_SYSV:
    ಸ್ಥಿತಿ FFI_STDCALL:
    ಸ್ಥಿತಿ FFI_THISCALL:
    ಸ್ಥಿತಿ FFI_FASTCALL:
    ಸ್ಥಿತಿ FFI_MS_CDECL:
    ಸ್ಥಿತಿ FFI_PASCAL:
    ಸ್ಥಿತಿ FFI_ರಿಜಿಸ್ಟರ್:
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
    }

  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      flags = X86_RET_INT32;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      {
#ಆದರೆ defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t ಗಾತ್ರ_2 = cif->rtype->ಗಾತ್ರ_2;
        ಆದರೆ (ಗಾತ್ರ_2 == 1)
          flags = X86_RET_STRUCT_1B;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 == 2)
          flags = X86_RET_STRUCT_2B;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 == 4)
          flags = X86_RET_INT32;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 == 8)
          flags = X86_RET_INT64;
        ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
          {
          do_struct:
            ಆಯ್ಕೆ (cabi)
              {
              ಸ್ಥಿತಿ FFI_THISCALL:
              ಸ್ಥಿತಿ FFI_FASTCALL:
              ಸ್ಥಿತಿ FFI_STDCALL:
              ಸ್ಥಿತಿ FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                ನಿಲ್ಲಿಸು;
              ಪೂರ್ವನಿಯೋಜಿತ:
                flags = X86_RET_STRUCTPOP;
                ನಿಲ್ಲಿಸು;
              }
            /* Allocate space for return value pointer.  */
            ಬೈಟುಗಳು += FFI_ALIGN (ಗಾತ್ರ(ಶೂನ್ಯ*), FFI_SIZEOF_ARG);
          }
      }
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      ಆಯ್ಕೆ (cif->rtype->elements[0]->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	  ಹೋಗು do_struct;
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
	}
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#ಆದರೆ defined(X86_WIN32)
      ಆದರೆ (cabi == FFI_STDCALL)
        ಬೈಟುಗಳು = FFI_ALIGN (ಬೈಟುಗಳು, FFI_SIZEOF_ARG);
      ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
        ಬೈಟುಗಳು = FFI_ALIGN (ಬೈಟುಗಳು, t->alignment);
      ಬೈಟುಗಳು += FFI_ALIGN (t->ಗಾತ್ರ_2, FFI_SIZEOF_ARG);
    }
  cif->ಬೈಟುಗಳು = ಬೈಟುಗಳು;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಸ್ಥಿರ ffi_arg
extend_basic_type(ಶೂನ್ಯ *arg, ಪೂರ್ಣಾಂಕ type)
{
  ಆಯ್ಕೆ (type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      ಹಿಂತಿರುಗಿ *(SINT8 *)arg;
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      ಹಿಂತಿರುಗಿ *(UINT8 *)arg;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      ಹಿಂತಿರುಗಿ *(SINT16 *)arg;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ಹಿಂತಿರುಗಿ *(UINT16 *)arg;

    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      ಹಿಂತಿರುಗಿ *(UINT32 *)arg;

    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }
}

ರಚನೆ call_ಚೌಕಟ್ಟು
{
  ಶೂನ್ಯ *ebp;		/* 0 */
  ಶೂನ್ಯ *retaddr;	/* 4 */
  ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ);	/* 8 */
  ಪೂರ್ಣಾಂಕ flags;		/* 12 */
  ಶೂನ್ಯ *rvalue;		/* 16 */
  ಚಿಹ್ನೆರಹಿತ regs[3];	/* 20-28 */
};

ರಚನೆ abi_params
{
  ಪೂರ್ಣಾಂಕ dir;		/* parameter growth direction */
  ಪೂರ್ಣಾಂಕ static_chain;	/* the static chain register used by gcc */
  ಪೂರ್ಣಾಂಕ nregs;		/* number of register parameters */
  ಪೂರ್ಣಾಂಕ regs[3];
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ abi_params abi_params[FFI_ಕೊನೆಯ_ABI] = {
  [FFI_SYSV] = { 1, R_ECX, 0 },
  [FFI_THISCALL] = { 1, R_EAX, 1, { R_ECX } },
  [FFI_FASTCALL] = { 1, R_EAX, 2, { R_ECX, R_EDX } },
  [FFI_STDCALL] = { 1, R_ECX, 0 },
  [FFI_PASCAL] = { -1, R_ECX, 0 },
  /* ??? No defined static chain; gcc does not support REGISTER.  */
  [FFI_ರಿಜಿಸ್ಟರ್] = { -1, R_ECX, 3, { R_EAX, R_EDX, R_ECX } },
  [FFI_MS_CDECL] = { 1, R_ECX, 0 }
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_FASTCALL
  #ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
    #ವ್ಯಾಖ್ಯಾನಿಸು FFI_DECLARE_FASTCALL __fastcall
  #ಇಲ್ಲದಿದ್ದರೆ
    #ವ್ಯಾಖ್ಯಾನಿಸು FFI_DECLARE_FASTCALL __declspec(fastcall)
  #ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು FFI_DECLARE_FASTCALL
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ ಶೂನ್ಯ FFI_DECLARE_FASTCALL ffi_call_i386(ರಚನೆ call_ಚೌಕಟ್ಟು *, ಅಕ್ಷರ *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ಆದರೆ defined(_MSC_VER)
#ಸೂಚನೆ runtime_checks("s", off)
#ಆದರೆ_ಅಂತ್ಯ
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  size_t rsize, ಬೈಟುಗಳು;
  ರಚನೆ call_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು;
  ಅಕ್ಷರ *ರಾಶಿ, *argp;
  ffi_type **arg_types;
  ಪೂರ್ಣಾಂಕ flags, cabi, i, n, dir, narg_reg;
  ಸ್ಥಿರಾಂಕ ರಚನೆ abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  ಆದರೆ (rvalue == ಖಾಲಿ)
    {
      ಆಯ್ಕೆ (flags)
	{
	ಸ್ಥಿತಿ X86_RET_FLOAT:
	ಸ್ಥಿತಿ X86_RET_DOUBLE:
	ಸ್ಥಿತಿ X86_RET_LDOUBLE:
	ಸ್ಥಿತಿ X86_RET_STRUCTPOP:
	ಸ್ಥಿತಿ X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->ಗಾತ್ರ_2;
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  ನಿಲ್ಲಿಸು;
	}
    }

  ಬೈಟುಗಳು = ರಾಶಿ_ALIGN (cif->ಬೈಟುಗಳು);
  ರಾಶಿ = alloca(ಬೈಟುಗಳು + ಗಾತ್ರ(*ಚೌಕಟ್ಟು) + rsize);
  argp = (dir < 0 ? ರಾಶಿ + ಬೈಟುಗಳು : ರಾಶಿ);
  ಚೌಕಟ್ಟು = (ರಚನೆ call_ಚೌಕಟ್ಟು *)(ರಾಶಿ + ಬೈಟುಗಳು);
  ಆದರೆ (rsize)
    rvalue = ಚೌಕಟ್ಟು + 1;

  ಚೌಕಟ್ಟು->fn = fn;
  ಚೌಕಟ್ಟು->flags = flags;
  ಚೌಕಟ್ಟು->rvalue = rvalue;
  ಚೌಕಟ್ಟು->regs[pabi->static_chain] = (ಚಿಹ್ನೆರಹಿತ)ಆವರಣ;

  narg_reg = 0;
  ಆಯ್ಕೆ (flags)
    {
    ಸ್ಥಿತಿ X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ಆದರೆ (pabi->nregs > 0)
	{
	  ಚೌಕಟ್ಟು->regs[pabi->regs[0]] = (ಚಿಹ್ನೆರಹಿತ)rvalue;
	  narg_reg = 1;
	  ನಿಲ್ಲಿಸು;
	}
      /* fallthru */
    ಸ್ಥಿತಿ X86_RET_STRUCTPOP:
      *(ಶೂನ್ಯ **)argp = rvalue;
      argp += ಗಾತ್ರ(ಶೂನ್ಯ *);
      ನಿಲ್ಲಿಸು;
    }

  arg_types = cif->arg_types;
  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ಶೂನ್ಯ *valp = avalue[i];
      size_t z = ty->ಗಾತ್ರ_2;
      ಪೂರ್ಣಾಂಕ t = ty->type;

      ಆದರೆ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  ಆದರೆ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    ಚೌಕಟ್ಟು->regs[pabi->regs[narg_reg++]] = val;
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ಆದರೆ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  ಆದರೆ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  ಆದರೆ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      ಸ್ಮರಣೆ_ನಕಲು (argp, valp, z);
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      argp = (ಅಕ್ಷರ *)FFI_ALIGN (argp, align);
	      ಸ್ಮರಣೆ_ನಕಲು (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == ರಾಶಿ);

  ffi_call_i386 (ಚೌಕಟ್ಟು, ರಾಶಿ);
}
#ಆದರೆ defined(_MSC_VER)
#ಸೂಚನೆ runtime_checks("s", restore)
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಖಾಲಿ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES
ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಆವರಣ);
}
#ಆದರೆ_ಅಂತ್ಯ

/** private members **/

ಶೂನ್ಯ FFI_HIDDEN ffi_ಆವರಣ_i386(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_ಆವರಣ_STDCALL(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_ಆವರಣ_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ);
#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ FFI_HIDDEN ffi_ಆವರಣ_i386_alt(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_ಆವರಣ_STDCALL_alt(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_ಆವರಣ_ರಿಜಿಸ್ಟರ್_alt(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ

ರಚನೆ ಆವರಣ_ಚೌಕಟ್ಟು
{
  ಚಿಹ್ನೆರಹಿತ rettemp[4];				/* 0 */
  ಚಿಹ್ನೆರಹಿತ regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*);	/* 32 */
  ಶೂನ್ಯ *user_ದತ್ತಾಂಶ;				/* 36 */
};

ಪೂರ್ಣಾಂಕ FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_ಆವರಣ_inner (ರಚನೆ ಆವರಣ_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು, ಅಕ್ಷರ *ರಾಶಿ)
{
  ffi_cif *cif = ಚೌಕಟ್ಟು->cif;
  ಪೂರ್ಣಾಂಕ cabi, i, n, flags, dir, narg_reg;
  ಸ್ಥಿರಾಂಕ ರಚನೆ abi_params *pabi;
  ffi_type **arg_types;
  ಅಕ್ಷರ *argp;
  ಶೂನ್ಯ *rvalue;
  ಶೂನ್ಯ **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = ಚೌಕಟ್ಟು->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? ರಾಶಿ + ರಾಶಿ_ALIGN (cif->ಬೈಟುಗಳು) : ರಾಶಿ);

  ಆಯ್ಕೆ (flags)
    {
    ಸ್ಥಿತಿ X86_RET_STRUCTARG:
      ಆದರೆ (pabi->nregs > 0)
	{
	  rvalue = (ಶೂನ್ಯ *)ಚೌಕಟ್ಟು->regs[pabi->regs[0]];
	  narg_reg = 1;
	  ಚೌಕಟ್ಟು->rettemp[0] = (ಚಿಹ್ನೆರಹಿತ)rvalue;
	  ನಿಲ್ಲಿಸು;
	}
      /* fallthru */
    ಸ್ಥಿತಿ X86_RET_STRUCTPOP:
      rvalue = *(ಶೂನ್ಯ **)argp;
      argp += ಗಾತ್ರ(ಶೂನ್ಯ *);
      ಚೌಕಟ್ಟು->rettemp[0] = (ಚಿಹ್ನೆರಹಿತ)rvalue;
      ನಿಲ್ಲಿಸು;
    }

  n = cif->nargs;
  avalue = alloca(ಗಾತ್ರ(ಶೂನ್ಯ *) * n);

  arg_types = cif->arg_types;
  ಗಾಗಿ (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->ಗಾತ್ರ_2;
      ಪೂರ್ಣಾಂಕ t = ty->type;
      ಶೂನ್ಯ *valp;

      ಆದರೆ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  ಆದರೆ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &ಚೌಕಟ್ಟು->regs[pabi->regs[narg_reg++]];
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  ಆದರೆ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ಆದರೆ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  ಆದರೆ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      argp = (ಅಕ್ಷರ *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  ಚೌಕಟ್ಟು->fun (cif, rvalue, avalue, ಚೌಕಟ್ಟು->user_ದತ್ತಾಂಶ);

  ಆಯ್ಕೆ (cabi)
    {
    ಸ್ಥಿತಿ FFI_STDCALL:
      ಹಿಂತಿರುಗಿ flags | (cif->ಬೈಟುಗಳು << X86_RET_ತೆಗೆ_ಸರಿಸು);
    ಸ್ಥಿತಿ FFI_THISCALL:
    ಸ್ಥಿತಿ FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      ಹಿಂತಿರುಗಿ flags | (((ಚಿಹ್ನೆರಹಿತ) (argp - ರಾಶಿ)) << X86_RET_ತೆಗೆ_ಸರಿಸು);
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ flags;
    }
}

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
                      ffi_cif* cif,
                      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
                      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
                      ಶೂನ್ಯ *codeloc)
{
  ಅಕ್ಷರ *tramp = ಆವರಣ->tramp;
  ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಂಕ op = 0xb8;  /* movl imm, %eax */

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_SYSV:
    ಸ್ಥಿತಿ FFI_MS_CDECL:
      dest = ffi_ಆವರಣ_i386;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_STDCALL:
    ಸ್ಥಿತಿ FFI_THISCALL:
    ಸ್ಥಿತಿ FFI_FASTCALL:
    ಸ್ಥಿತಿ FFI_PASCAL:
      dest = ffi_ಆವರಣ_STDCALL;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_ರಿಜಿಸ್ಟರ್:
      dest = ffi_ಆವರಣ_ರಿಜಿಸ್ಟರ್;
      op = 0x68;  /* pushl imm */
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
    }

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
  ಆದರೆ (ffi_tramp_is_present(ಆವರಣ))
    {
      /* Initialize the static trampoline's parameters. */
      ಆದರೆ (dest == ffi_ಆವರಣ_i386)
        dest = ffi_ಆವರಣ_i386_alt;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dest == ffi_ಆವರಣ_STDCALL)
        dest = ffi_ಆವರಣ_STDCALL_alt;
      ಇಲ್ಲದಿದ್ದರೆ
        dest = ffi_ಆವರಣ_ರಿಜಿಸ್ಟರ್_alt;
      ffi_tramp_ಗಣ_parms (ಆವರಣ->ftramp, dest, ಆವರಣ);
      ಹೋಗು ಹೊರಹೋಗು;
    }
#ಆದರೆ_ಅಂತ್ಯ

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(ಶೂನ್ಯ **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(ಚಿಹ್ನೆರಹಿತ *)(tramp + 10) = (ಚಿಹ್ನೆರಹಿತ)dest - ((ಚಿಹ್ನೆರಹಿತ)codeloc + 14);

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಹೊರಹೋಗು:
#ಆದರೆ_ಅಂತ್ಯ
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES

ಶೂನ್ಯ FFI_HIDDEN ffi_go_ಆವರಣ_EAX(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_go_ಆವರಣ_ECX(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_go_ಆವರಣ_STDCALL(ಶೂನ್ಯ);

ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ* ಆವರಣ, ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*))
{
  ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ);

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_SYSV:
    ಸ್ಥಿತಿ FFI_MS_CDECL:
      dest = ffi_go_ಆವರಣ_ECX;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_THISCALL:
    ಸ್ಥಿತಿ FFI_FASTCALL:
      dest = ffi_go_ಆವರಣ_EAX;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_STDCALL:
    ಸ್ಥಿತಿ FFI_PASCAL:
      dest = ffi_go_ಆವರಣ_STDCALL;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_ರಿಜಿಸ್ಟರ್:
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
    }

  ಆವರಣ->tramp = dest;
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#ಆದರೆ !FFI_NO_RAW_API

ಶೂನ್ಯ FFI_HIDDEN ffi_ಆವರಣ_raw_SYSV(ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_ಆವರಣ_raw_THISCALL(ಶೂನ್ಯ);

ffi_ಸ್ಥಿತಿ
ffi_prep_raw_ಆವರಣ_loc (ffi_raw_ಆವರಣ *ಆವರಣ,
                          ffi_cif *cif,
                          ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ffi_raw*,ಶೂನ್ಯ*),
                          ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
                          ಶೂನ್ಯ *codeloc)
{
  ಅಕ್ಷರ *tramp = ಆವರಣ->tramp;
  ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಂಕ i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  ಗಾಗಿ (i = cif->nargs-1; i >= 0; i--)
    ಆಯ್ಕೆ (cif->arg_types[i]->type)
      {
      ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
      }

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_THISCALL:
      dest = ffi_ಆವರಣ_raw_THISCALL;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_SYSV:
      dest = ffi_ಆವರಣ_raw_SYSV;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(ಶೂನ್ಯ **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(ಚಿಹ್ನೆರಹಿತ *)(tramp + 6) = (ಚಿಹ್ನೆರಹಿತ)dest - ((ಚಿಹ್ನೆರಹಿತ)codeloc + 10);

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಶೂನ್ಯ
ffi_raw_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ffi_raw *avalue)
{
  size_t rsize, ಬೈಟುಗಳು;
  ರಚನೆ call_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು;
  ಅಕ್ಷರ *ರಾಶಿ, *argp;
  ffi_type **arg_types;
  ಪೂರ್ಣಾಂಕ flags, cabi, i, n, narg_reg;
  ಸ್ಥಿರಾಂಕ ರಚನೆ abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  ಆದರೆ (rvalue == ಖಾಲಿ)
    {
      ಆಯ್ಕೆ (flags)
	{
	ಸ್ಥಿತಿ X86_RET_FLOAT:
	ಸ್ಥಿತಿ X86_RET_DOUBLE:
	ಸ್ಥಿತಿ X86_RET_LDOUBLE:
	ಸ್ಥಿತಿ X86_RET_STRUCTPOP:
	ಸ್ಥಿತಿ X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->ಗಾತ್ರ_2;
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  ನಿಲ್ಲಿಸು;
	}
    }

  ಬೈಟುಗಳು = ರಾಶಿ_ALIGN (cif->ಬೈಟುಗಳು);
  argp = ರಾಶಿ =
      (ಶೂನ್ಯ *)((uintptr_t)alloca(ಬೈಟುಗಳು + ಗಾತ್ರ(*ಚೌಕಟ್ಟು) + rsize + 15) & ~16);
  ಚೌಕಟ್ಟು = (ರಚನೆ call_ಚೌಕಟ್ಟು *)(ರಾಶಿ + ಬೈಟುಗಳು);
  ಆದರೆ (rsize)
    rvalue = ಚೌಕಟ್ಟು + 1;

  ಚೌಕಟ್ಟು->fn = fn;
  ಚೌಕಟ್ಟು->flags = flags;
  ಚೌಕಟ್ಟು->rvalue = rvalue;

  narg_reg = 0;
  ಆಯ್ಕೆ (flags)
    {
    ಸ್ಥಿತಿ X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ಆದರೆ (pabi->nregs > 0)
	{
	  ಚೌಕಟ್ಟು->regs[pabi->regs[0]] = (ಚಿಹ್ನೆರಹಿತ)rvalue;
	  narg_reg = 1;
	  ನಿಲ್ಲಿಸು;
	}
      /* fallthru */
    ಸ್ಥಿತಿ X86_RET_STRUCTPOP:
      *(ಶೂನ್ಯ **)argp = rvalue;
      argp += ಗಾತ್ರ(ಶೂನ್ಯ *);
      ಬೈಟುಗಳು -= ಗಾತ್ರ(ಶೂನ್ಯ *);
      ನಿಲ್ಲಿಸು;
    }

  arg_types = cif->arg_types;
  ಗಾಗಿ (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->ಗಾತ್ರ_2;
      ಪೂರ್ಣಾಂಕ t = ty->type;

      ಆದರೆ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  ಚೌಕಟ್ಟು->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಸ್ಮರಣೆ_ನಕಲು (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      ಬೈಟುಗಳು -= z;
    }
  ಆದರೆ (i < n)
    ಸ್ಮರಣೆ_ನಕಲು (argp, avalue, ಬೈಟುಗಳು);

  ffi_call_i386 (ಚೌಕಟ್ಟು, ರಾಶಿ);
}
#ಆದರೆ_ಅಂತ್ಯ /* !FFI_NO_RAW_API */

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_ಗಾತ್ರ, size_t *map_ಗಾತ್ರ)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;

  *map_ಗಾತ್ರ = X86_TRAMP_MAP_ಗಾತ್ರ;
  *tramp_ಗಾತ್ರ = X86_TRAMP_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ &trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* __i386__ */
