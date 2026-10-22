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

#ଯଦି defined(__i386__) || defined(_M_IX86)
#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <tramp.h>
#ଅନ୍ତର୍ଭାଵ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ଯଦି FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#ଅନ୍ୟଥା
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#ଯଦି defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#endif

#ଯଦି defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#define ଗଦା_ALIGN(bytes) (bytes)
#ଅନ୍ୟଥା
#define ଗଦା_ALIGN(bytes) FFI_ALIGN (bytes, 16)
#endif

/* Perform machine dependent cif processing.  */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n, flags, cabi = cif->abi;

  ବାଛ (cabi)
    {
    ସ୍ଥିତି FFI_SYSV:
    ସ୍ଥିତି FFI_STDCALL:
    ସ୍ଥିତି FFI_THISCALL:
    ସ୍ଥିତି FFI_FASTCALL:
    ସ୍ଥିତି FFI_MS_CDECL:
    ସ୍ଥିତି FFI_PASCAL:
    ସ୍ଥିତି FFI_REGISTER:
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_ABI;
    }

  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_INT:
    ସ୍ଥିତି FFI_TYPE_SINT32:
    ସ୍ଥିତି FFI_TYPE_UINT32:
    ସ୍ଥିତି FFI_TYPE_POINTER:
      flags = X86_RET_INT32;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      {
#ଯଦି defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t size = cif->rtype->size;
        ଯଦି (size == 1)
          flags = X86_RET_STRUCT_1B;
        ଅନ୍ୟଥା ଯଦି (size == 2)
          flags = X86_RET_STRUCT_2B;
        ଅନ୍ୟଥା ଯଦି (size == 4)
          flags = X86_RET_INT32;
        ଅନ୍ୟଥା ଯଦି (size == 8)
          flags = X86_RET_INT64;
        ଅନ୍ୟଥା
#endif
          {
          do_struct:
            ବାଛ (cabi)
              {
              ସ୍ଥିତି FFI_THISCALL:
              ସ୍ଥିତି FFI_FASTCALL:
              ସ୍ଥିତି FFI_STDCALL:
              ସ୍ଥିତି FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                ଭାଙ୍ଗ;
              ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
                flags = X86_RET_STRUCTPOP;
                ଭାଙ୍ଗ;
              }
            /* Allocate space for return value pointer.  */
            bytes += FFI_ALIGN (ଆକାର(ଶୂନ୍ୟ*), FFI_SIZEOF_ARG);
          }
      }
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      ବାଛ (cif->rtype->elements[0]->type)
	{
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	  ଯାଅ do_struct;
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;
	}
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  ପାଇଁ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#ଯଦି defined(X86_WIN32)
      ଯଦି (cabi == FFI_STDCALL)
        bytes = FFI_ALIGN (bytes, FFI_SIZEOF_ARG);
      ଅନ୍ୟଥା
#endif
        bytes = FFI_ALIGN (bytes, t->alignment);
      bytes += FFI_ALIGN (t->size, FFI_SIZEOF_ARG);
    }
  cif->bytes = bytes;

  ଫେରନ୍ତୁ FFI_OK;
}

ସ୍ଥିର ffi_arg
extend_basic_type(ଶୂନ୍ୟ *arg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type)
{
  ବାଛ (type)
    {
    ସ୍ଥିତି FFI_TYPE_SINT8:
      ଫେରନ୍ତୁ *(SINT8 *)arg;
    ସ୍ଥିତି FFI_TYPE_UINT8:
      ଫେରନ୍ତୁ *(UINT8 *)arg;
    ସ୍ଥିତି FFI_TYPE_SINT16:
      ଫେରନ୍ତୁ *(SINT16 *)arg;
    ସ୍ଥିତି FFI_TYPE_UINT16:
      ଫେରନ୍ତୁ *(UINT16 *)arg;

    ସ୍ଥିତି FFI_TYPE_SINT32:
    ସ୍ଥିତି FFI_TYPE_UINT32:
    ସ୍ଥିତି FFI_TYPE_POINTER:
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      ଫେରନ୍ତୁ *(UINT32 *)arg;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort();
    }
}

ସଂରଚନା call_frame
{
  ଶୂନ୍ୟ *ebp;		/* 0 */
  ଶୂନ୍ୟ *retaddr;	/* 4 */
  ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ);	/* 8 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;		/* 12 */
  ଶୂନ୍ୟ *rvalue;		/* 16 */
  unsigned regs[3];	/* 20-28 */
};

ସଂରଚନା abi_params
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir;		/* parameter growth direction */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା static_chain;	/* the static chain register used by gcc */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nregs;		/* number of register parameters */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regs[3];
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା abi_params abi_params[FFI_LAST_ABI] = {
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
  #ଅନ୍ୟଥା
    #define FFI_DECLARE_FASTCALL __declspec(fastcall)
  #endif
#ଅନ୍ୟଥା
  #define FFI_DECLARE_FASTCALL
#endif

ବାହ୍ୟ ଶୂନ୍ୟ FFI_DECLARE_FASTCALL ffi_call_i386(ସଂରଚନା call_frame *, ଅକ୍ଷର *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ଯଦି defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	      ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  size_t rsize, bytes;
  ସଂରଚନା call_frame *frame;
  ଅକ୍ଷର *ଗଦା, *argp;
  ffi_type **arg_types;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, cabi, i, n, dir, narg_reg;
  ଅଚଳ ସଂରଚନା abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  ଯଦି (rvalue == ଖାଲି)
    {
      ବାଛ (flags)
	{
	ସ୍ଥିତି X86_RET_FLOAT:
	ସ୍ଥିତି X86_RET_DOUBLE:
	ସ୍ଥିତି X86_RET_LDOUBLE:
	ସ୍ଥିତି X86_RET_STRUCTPOP:
	ସ୍ଥିତି X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  ଭାଙ୍ଗ;
	}
    }

  bytes = ଗଦା_ALIGN (cif->bytes);
  ଗଦା = alloca(bytes + ଆକାର(*frame) + rsize);
  argp = (dir < 0 ? ଗଦା + bytes : ଗଦା);
  frame = (ସଂରଚନା call_frame *)(ଗଦା + bytes);
  ଯଦି (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;
  frame->regs[pabi->static_chain] = (unsigned)closure;

  narg_reg = 0;
  ବାଛ (flags)
    {
    ସ୍ଥିତି X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ଯଦି (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  ଭାଙ୍ଗ;
	}
      /* fallthru */
    ସ୍ଥିତି X86_RET_STRUCTPOP:
      *(ଶୂନ୍ୟ **)argp = rvalue;
      argp += ଆକାର(ଶୂନ୍ୟ *);
      ଭାଙ୍ଗ;
    }

  arg_types = cif->arg_types;
  ପାଇଁ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ଶୂନ୍ୟ *valp = avalue[i];
      size_t z = ty->size;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = ty->type;

      ଯଦି (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  ଯଦି (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    frame->regs[pabi->regs[narg_reg++]] = val;
	  ଅନ୍ୟଥା ଯଦି (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  ଅନ୍ୟଥା
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      ଅନ୍ୟଥା
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ଯଦି ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  ଯଦି (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  ଯଦି (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      memcpy (argp, valp, z);
	    }
	  ଅନ୍ୟଥା
	    {
	      argp = (ଅକ୍ଷର *)FFI_ALIGN (argp, align);
	      memcpy (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == ଗଦା);

  ffi_call_i386 (frame, ଗଦା);
}
#ଯଦି defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ଖାଲି);
}

#ifdef FFI_GO_CLOSURES
ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

/** private members **/

ଶୂନ୍ୟ FFI_HIDDEN ffi_closure_i386(ଶୂନ୍ୟ);
ଶୂନ୍ୟ FFI_HIDDEN ffi_closure_STDCALL(ଶୂନ୍ୟ);
ଶୂନ୍ୟ FFI_HIDDEN ffi_closure_REGISTER(ଶୂନ୍ୟ);
#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ଶୂନ୍ୟ FFI_HIDDEN ffi_closure_i386_alt(ଶୂନ୍ୟ);
ଶୂନ୍ୟ FFI_HIDDEN ffi_closure_STDCALL_alt(ଶୂନ୍ୟ);
ଶୂନ୍ୟ FFI_HIDDEN ffi_closure_REGISTER_alt(ଶୂନ୍ୟ);
#endif

ସଂରଚନା closure_frame
{
  unsigned rettemp[4];				/* 0 */
  unsigned regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*);	/* 32 */
  ଶୂନ୍ୟ *user_data;				/* 36 */
};

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_closure_inner (ସଂରଚନା closure_frame *frame, ଅକ୍ଷର *ଗଦା)
{
  ffi_cif *cif = frame->cif;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cabi, i, n, flags, dir, narg_reg;
  ଅଚଳ ସଂରଚନା abi_params *pabi;
  ffi_type **arg_types;
  ଅକ୍ଷର *argp;
  ଶୂନ୍ୟ *rvalue;
  ଶୂନ୍ୟ **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = frame->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? ଗଦା + ଗଦା_ALIGN (cif->bytes) : ଗଦା);

  ବାଛ (flags)
    {
    ସ୍ଥିତି X86_RET_STRUCTARG:
      ଯଦି (pabi->nregs > 0)
	{
	  rvalue = (ଶୂନ୍ୟ *)frame->regs[pabi->regs[0]];
	  narg_reg = 1;
	  frame->rettemp[0] = (unsigned)rvalue;
	  ଭାଙ୍ଗ;
	}
      /* fallthru */
    ସ୍ଥିତି X86_RET_STRUCTPOP:
      rvalue = *(ଶୂନ୍ୟ **)argp;
      argp += ଆକାର(ଶୂନ୍ୟ *);
      frame->rettemp[0] = (unsigned)rvalue;
      ଭାଙ୍ଗ;
    }

  n = cif->nargs;
  avalue = alloca(ଆକାର(ଶୂନ୍ୟ *) * n);

  arg_types = cif->arg_types;
  ପାଇଁ (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = ty->type;
      ଶୂନ୍ୟ *valp;

      ଯଦି (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  ଯଦି (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &frame->regs[pabi->regs[narg_reg++]];
	  ଅନ୍ୟଥା ଯଦି (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  ଅନ୍ୟଥା
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      ଅନ୍ୟଥା
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  ଯଦି (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ଯଦି ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  ଯଦି (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  ଅନ୍ୟଥା
	    {
	      argp = (ଅକ୍ଷର *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  frame->fun (cif, rvalue, avalue, frame->user_data);

  ବାଛ (cabi)
    {
    ସ୍ଥିତି FFI_STDCALL:
      ଫେରନ୍ତୁ flags | (cif->bytes << X86_RET_କାଢ଼_SHIFT);
    ସ୍ଥିତି FFI_THISCALL:
    ସ୍ଥିତି FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      ଫେରନ୍ତୁ flags | (((unsigned) (argp - ଗଦା)) << X86_RET_କାଢ଼_SHIFT);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ flags;
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
                      ଶୂନ୍ୟ *user_data,
                      ଶୂନ୍ୟ *codeloc)
{
  ଅକ୍ଷର *tramp = closure->tramp;
  ଶୂନ୍ୟ (*dest)(ଶୂନ୍ୟ);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = 0xb8;  /* movl imm, %eax */

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_SYSV:
    ସ୍ଥିତି FFI_MS_CDECL:
      dest = ffi_closure_i386;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_STDCALL:
    ସ୍ଥିତି FFI_THISCALL:
    ସ୍ଥିତି FFI_FASTCALL:
    ସ୍ଥିତି FFI_PASCAL:
      dest = ffi_closure_STDCALL;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_REGISTER:
      dest = ffi_closure_REGISTER;
      op = 0x68;  /* pushl imm */
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_ABI;
    }

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
  ଯଦି (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ଯଦି (dest == ffi_closure_i386)
        dest = ffi_closure_i386_alt;
      ଅନ୍ୟଥା ଯଦି (dest == ffi_closure_STDCALL)
        dest = ffi_closure_STDCALL_alt;
      ଅନ୍ୟଥା
        dest = ffi_closure_REGISTER_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      ଯାଅ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(ଶୂନ୍ୟ **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(unsigned *)(tramp + 10) = (unsigned)dest - ((unsigned)codeloc + 14);

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ଫେରନ୍ତୁ FFI_OK;
}

#ifdef FFI_GO_CLOSURES

ଶୂନ୍ୟ FFI_HIDDEN ffi_go_closure_EAX(ଶୂନ୍ୟ);
ଶୂନ୍ୟ FFI_HIDDEN ffi_go_closure_ECX(ଶୂନ୍ୟ);
ଶୂନ୍ୟ FFI_HIDDEN ffi_go_closure_STDCALL(ଶୂନ୍ୟ);

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*))
{
  ଶୂନ୍ୟ (*dest)(ଶୂନ୍ୟ);

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_SYSV:
    ସ୍ଥିତି FFI_MS_CDECL:
      dest = ffi_go_closure_ECX;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_THISCALL:
    ସ୍ଥିତି FFI_FASTCALL:
      dest = ffi_go_closure_EAX;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_STDCALL:
    ସ୍ଥିତି FFI_PASCAL:
      dest = ffi_go_closure_STDCALL;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_REGISTER:
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_ABI;
    }

  closure->tramp = dest;
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#ଯଦି !FFI_NO_RAW_API

ଶୂନ୍ୟ FFI_HIDDEN ffi_closure_raw_SYSV(ଶୂନ୍ୟ);
ଶୂନ୍ୟ FFI_HIDDEN ffi_closure_raw_THISCALL(ଶୂନ୍ୟ);

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure *closure,
                          ffi_cif *cif,
                          ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ffi_raw*,ଶୂନ୍ୟ*),
                          ଶୂନ୍ୟ *user_data,
                          ଶୂନ୍ୟ *codeloc)
{
  ଅକ୍ଷର *tramp = closure->tramp;
  ଶୂନ୍ୟ (*dest)(ଶୂନ୍ୟ);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  ପାଇଁ (i = cif->nargs-1; i >= 0; i--)
    ବାଛ (cif->arg_types[i]->type)
      {
      ସ୍ଥିତି FFI_TYPE_STRUCT:
      ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;
      }

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_THISCALL:
      dest = ffi_closure_raw_THISCALL;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_SYSV:
      dest = ffi_closure_raw_SYSV;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(ଶୂନ୍ୟ **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(unsigned *)(tramp + 6) = (unsigned)dest - ((unsigned)codeloc + 10);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ଫେରନ୍ତୁ FFI_OK;
}

ଶୂନ୍ୟ
ffi_raw_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ffi_raw *avalue)
{
  size_t rsize, bytes;
  ସଂରଚନା call_frame *frame;
  ଅକ୍ଷର *ଗଦା, *argp;
  ffi_type **arg_types;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, cabi, i, n, narg_reg;
  ଅଚଳ ସଂରଚନା abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  ଯଦି (rvalue == ଖାଲି)
    {
      ବାଛ (flags)
	{
	ସ୍ଥିତି X86_RET_FLOAT:
	ସ୍ଥିତି X86_RET_DOUBLE:
	ସ୍ଥିତି X86_RET_LDOUBLE:
	ସ୍ଥିତି X86_RET_STRUCTPOP:
	ସ୍ଥିତି X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  ଭାଙ୍ଗ;
	}
    }

  bytes = ଗଦା_ALIGN (cif->bytes);
  argp = ଗଦା =
      (ଶୂନ୍ୟ *)((uintptr_t)alloca(bytes + ଆକାର(*frame) + rsize + 15) & ~16);
  frame = (ସଂରଚନା call_frame *)(ଗଦା + bytes);
  ଯଦି (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;

  narg_reg = 0;
  ବାଛ (flags)
    {
    ସ୍ଥିତି X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ଯଦି (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  ଭାଙ୍ଗ;
	}
      /* fallthru */
    ସ୍ଥିତି X86_RET_STRUCTPOP:
      *(ଶୂନ୍ୟ **)argp = rvalue;
      argp += ଆକାର(ଶୂନ୍ୟ *);
      bytes -= ଆକାର(ଶୂନ୍ୟ *);
      ଭାଙ୍ଗ;
    }

  arg_types = cif->arg_types;
  ପାଇଁ (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = ty->type;

      ଯଦି (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  frame->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      ଅନ୍ୟଥା
	{
	  memcpy (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      bytes -= z;
    }
  ଯଦି (i < n)
    memcpy (argp, avalue, bytes);

  ffi_call_i386 (frame, ଗଦା);
}
#endif /* !FFI_NO_RAW_API */

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ଶୂନ୍ୟ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ବାହ୍ୟ ଶୂନ୍ୟ *trampoline_code_table;

  *map_size = X86_TRAMP_MAP_SIZE;
  *tramp_size = X86_TRAMP_SIZE;
  ଫେରନ୍ତୁ &trampoline_code_table;
}
#endif

#endif /* __i386__ */
