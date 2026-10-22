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

#જો defined(__i386__) || defined(_M_IX86)
#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <tramp.h>
#અન્તર્ભાવ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# જો FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#નહીંતર
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#જો defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#endif

#જો defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#define ઢગલો_ALIGN(bytes) (bytes)
#નહીંતર
#define ઢગલો_ALIGN(bytes) FFI_ALIGN (bytes, 16)
#endif

/* Perform machine dependent cif processing.  */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  પૂર્ણાંક i, n, flags, cabi = cif->abi;

  પસંદગી (cabi)
    {
    સ્થિતિ FFI_SYSV:
    સ્થિતિ FFI_STDCALL:
    સ્થિતિ FFI_THISCALL:
    સ્થિતિ FFI_FASTCALL:
    સ્થિતિ FFI_MS_CDECL:
    સ્થિતિ FFI_PASCAL:
    સ્થિતિ FFI_REGISTER:
      તોડો;
    મૂળભૂત:
      પાછા FFI_BAD_ABI;
    }

  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      તોડો;
    સ્થિતિ FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      તોડો;
    સ્થિતિ FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      તોડો;
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      તોડો;
    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_SINT32:
    સ્થિતિ FFI_TYPE_UINT32:
    સ્થિતિ FFI_TYPE_POINTER:
      flags = X86_RET_INT32;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      તોડો;
    સ્થિતિ FFI_TYPE_STRUCT:
      {
#જો defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t size = cif->rtype->size;
        જો (size == 1)
          flags = X86_RET_STRUCT_1B;
        નહીંતર જો (size == 2)
          flags = X86_RET_STRUCT_2B;
        નહીંતર જો (size == 4)
          flags = X86_RET_INT32;
        નહીંતર જો (size == 8)
          flags = X86_RET_INT64;
        નહીંતર
#endif
          {
          do_struct:
            પસંદગી (cabi)
              {
              સ્થિતિ FFI_THISCALL:
              સ્થિતિ FFI_FASTCALL:
              સ્થિતિ FFI_STDCALL:
              સ્થિતિ FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                તોડો;
              મૂળભૂત:
                flags = X86_RET_STRUCTPOP;
                તોડો;
              }
            /* Allocate space for return value pointer.  */
            bytes += FFI_ALIGN (કદ(શૂન્ય*), FFI_SIZEOF_ARG);
          }
      }
      તોડો;
    સ્થિતિ FFI_TYPE_COMPLEX:
      પસંદગી (cif->rtype->elements[0]->type)
	{
	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	  જાઓ do_struct;
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  તોડો;
	મૂળભૂત:
	  પાછા FFI_BAD_TYPEDEF;
	}
      તોડો;
    મૂળભૂત:
      પાછા FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  માટે (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#જો defined(X86_WIN32)
      જો (cabi == FFI_STDCALL)
        bytes = FFI_ALIGN (bytes, FFI_SIZEOF_ARG);
      નહીંતર
#endif
        bytes = FFI_ALIGN (bytes, t->alignment);
      bytes += FFI_ALIGN (t->size, FFI_SIZEOF_ARG);
    }
  cif->bytes = bytes;

  પાછા FFI_OK;
}

સ્થિર ffi_arg
extend_basic_type(શૂન્ય *arg, પૂર્ણાંક type)
{
  પસંદગી (type)
    {
    સ્થિતિ FFI_TYPE_SINT8:
      પાછા *(SINT8 *)arg;
    સ્થિતિ FFI_TYPE_UINT8:
      પાછા *(UINT8 *)arg;
    સ્થિતિ FFI_TYPE_SINT16:
      પાછા *(SINT16 *)arg;
    સ્થિતિ FFI_TYPE_UINT16:
      પાછા *(UINT16 *)arg;

    સ્થિતિ FFI_TYPE_SINT32:
    સ્થિતિ FFI_TYPE_UINT32:
    સ્થિતિ FFI_TYPE_POINTER:
    સ્થિતિ FFI_TYPE_FLOAT:
      પાછા *(UINT32 *)arg;

    મૂળભૂત:
      abort();
    }
}

સંરચના call_frame
{
  શૂન્ય *ebp;		/* 0 */
  શૂન્ય *retaddr;	/* 4 */
  શૂન્ય (*fn)(શૂન્ય);	/* 8 */
  પૂર્ણાંક flags;		/* 12 */
  શૂન્ય *rvalue;		/* 16 */
  unsigned regs[3];	/* 20-28 */
};

સંરચના abi_params
{
  પૂર્ણાંક dir;		/* parameter growth direction */
  પૂર્ણાંક static_chain;	/* the static chain register used by gcc */
  પૂર્ણાંક nregs;		/* number of register parameters */
  પૂર્ણાંક regs[3];
};

સ્થિર અચલ સંરચના abi_params abi_params[FFI_LAST_ABI] = {
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
  #નહીંતર
    #define FFI_DECLARE_FASTCALL __declspec(fastcall)
  #endif
#નહીંતર
  #define FFI_DECLARE_FASTCALL
#endif

બાહ્ય શૂન્ય FFI_DECLARE_FASTCALL ffi_call_i386(સંરચના call_frame *, અક્ષર *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#જો defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
સ્થિર શૂન્ય
ffi_call_int (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	      શૂન્ય **avalue, શૂન્ય *closure)
{
  size_t rsize, bytes;
  સંરચના call_frame *frame;
  અક્ષર *ઢગલો, *argp;
  ffi_type **arg_types;
  પૂર્ણાંક flags, cabi, i, n, dir, narg_reg;
  અચલ સંરચના abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  જો (rvalue == ખાલી)
    {
      પસંદગી (flags)
	{
	સ્થિતિ X86_RET_FLOAT:
	સ્થિતિ X86_RET_DOUBLE:
	સ્થિતિ X86_RET_LDOUBLE:
	સ્થિતિ X86_RET_STRUCTPOP:
	સ્થિતિ X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  તોડો;
	મૂળભૂત:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  તોડો;
	}
    }

  bytes = ઢગલો_ALIGN (cif->bytes);
  ઢગલો = alloca(bytes + કદ(*frame) + rsize);
  argp = (dir < 0 ? ઢગલો + bytes : ઢગલો);
  frame = (સંરચના call_frame *)(ઢગલો + bytes);
  જો (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;
  frame->regs[pabi->static_chain] = (unsigned)closure;

  narg_reg = 0;
  પસંદગી (flags)
    {
    સ્થિતિ X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      જો (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  તોડો;
	}
      /* fallthru */
    સ્થિતિ X86_RET_STRUCTPOP:
      *(શૂન્ય **)argp = rvalue;
      argp += કદ(શૂન્ય *);
      તોડો;
    }

  arg_types = cif->arg_types;
  માટે (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      શૂન્ય *valp = avalue[i];
      size_t z = ty->size;
      પૂર્ણાંક t = ty->type;

      જો (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  જો (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    frame->regs[pabi->regs[narg_reg++]] = val;
	  નહીંતર જો (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  નહીંતર
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      નહીંતર
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  જો ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  જો (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  જો (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      memcpy (argp, valp, z);
	    }
	  નહીંતર
	    {
	      argp = (અક્ષર *)FFI_ALIGN (argp, align);
	      memcpy (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == ઢગલો);

  ffi_call_i386 (frame, ઢગલો);
}
#જો defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ખાલી);
}

#ifdef FFI_GO_CLOSURES
શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

/** private members **/

શૂન્ય FFI_HIDDEN ffi_closure_i386(શૂન્ય);
શૂન્ય FFI_HIDDEN ffi_closure_STDCALL(શૂન્ય);
શૂન્ય FFI_HIDDEN ffi_closure_REGISTER(શૂન્ય);
#જો defined(FFI_EXEC_STATIC_TRAMP)
શૂન્ય FFI_HIDDEN ffi_closure_i386_alt(શૂન્ય);
શૂન્ય FFI_HIDDEN ffi_closure_STDCALL_alt(શૂન્ય);
શૂન્ય FFI_HIDDEN ffi_closure_REGISTER_alt(શૂન્ય);
#endif

સંરચના closure_frame
{
  unsigned rettemp[4];				/* 0 */
  unsigned regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*);	/* 32 */
  શૂન્ય *user_data;				/* 36 */
};

પૂર્ણાંક FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_closure_inner (સંરચના closure_frame *frame, અક્ષર *ઢગલો)
{
  ffi_cif *cif = frame->cif;
  પૂર્ણાંક cabi, i, n, flags, dir, narg_reg;
  અચલ સંરચના abi_params *pabi;
  ffi_type **arg_types;
  અક્ષર *argp;
  શૂન્ય *rvalue;
  શૂન્ય **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = frame->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? ઢગલો + ઢગલો_ALIGN (cif->bytes) : ઢગલો);

  પસંદગી (flags)
    {
    સ્થિતિ X86_RET_STRUCTARG:
      જો (pabi->nregs > 0)
	{
	  rvalue = (શૂન્ય *)frame->regs[pabi->regs[0]];
	  narg_reg = 1;
	  frame->rettemp[0] = (unsigned)rvalue;
	  તોડો;
	}
      /* fallthru */
    સ્થિતિ X86_RET_STRUCTPOP:
      rvalue = *(શૂન્ય **)argp;
      argp += કદ(શૂન્ય *);
      frame->rettemp[0] = (unsigned)rvalue;
      તોડો;
    }

  n = cif->nargs;
  avalue = alloca(કદ(શૂન્ય *) * n);

  arg_types = cif->arg_types;
  માટે (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      પૂર્ણાંક t = ty->type;
      શૂન્ય *valp;

      જો (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  જો (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &frame->regs[pabi->regs[narg_reg++]];
	  નહીંતર જો (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  નહીંતર
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      નહીંતર
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  જો (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  જો ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  જો (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  નહીંતર
	    {
	      argp = (અક્ષર *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  frame->fun (cif, rvalue, avalue, frame->user_data);

  પસંદગી (cabi)
    {
    સ્થિતિ FFI_STDCALL:
      પાછા flags | (cif->bytes << X86_RET_કાઢો_SHIFT);
    સ્થિતિ FFI_THISCALL:
    સ્થિતિ FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      પાછા flags | (((unsigned) (argp - ઢગલો)) << X86_RET_કાઢો_SHIFT);
    મૂળભૂત:
      પાછા flags;
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
                      શૂન્ય *user_data,
                      શૂન્ય *codeloc)
{
  અક્ષર *tramp = closure->tramp;
  શૂન્ય (*dest)(શૂન્ય);
  પૂર્ણાંક op = 0xb8;  /* movl imm, %eax */

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_SYSV:
    સ્થિતિ FFI_MS_CDECL:
      dest = ffi_closure_i386;
      તોડો;
    સ્થિતિ FFI_STDCALL:
    સ્થિતિ FFI_THISCALL:
    સ્થિતિ FFI_FASTCALL:
    સ્થિતિ FFI_PASCAL:
      dest = ffi_closure_STDCALL;
      તોડો;
    સ્થિતિ FFI_REGISTER:
      dest = ffi_closure_REGISTER;
      op = 0x68;  /* pushl imm */
      તોડો;
    મૂળભૂત:
      પાછા FFI_BAD_ABI;
    }

#જો defined(FFI_EXEC_STATIC_TRAMP)
  જો (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      જો (dest == ffi_closure_i386)
        dest = ffi_closure_i386_alt;
      નહીંતર જો (dest == ffi_closure_STDCALL)
        dest = ffi_closure_STDCALL_alt;
      નહીંતર
        dest = ffi_closure_REGISTER_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      જાઓ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(શૂન્ય **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(unsigned *)(tramp + 10) = (unsigned)dest - ((unsigned)codeloc + 14);

#જો defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  પાછા FFI_OK;
}

#ifdef FFI_GO_CLOSURES

શૂન્ય FFI_HIDDEN ffi_go_closure_EAX(શૂન્ય);
શૂન્ય FFI_HIDDEN ffi_go_closure_ECX(શૂન્ય);
શૂન્ય FFI_HIDDEN ffi_go_closure_STDCALL(શૂન્ય);

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*))
{
  શૂન્ય (*dest)(શૂન્ય);

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_SYSV:
    સ્થિતિ FFI_MS_CDECL:
      dest = ffi_go_closure_ECX;
      તોડો;
    સ્થિતિ FFI_THISCALL:
    સ્થિતિ FFI_FASTCALL:
      dest = ffi_go_closure_EAX;
      તોડો;
    સ્થિતિ FFI_STDCALL:
    સ્થિતિ FFI_PASCAL:
      dest = ffi_go_closure_STDCALL;
      તોડો;
    સ્થિતિ FFI_REGISTER:
    મૂળભૂત:
      પાછા FFI_BAD_ABI;
    }

  closure->tramp = dest;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#જો !FFI_NO_RAW_API

શૂન્ય FFI_HIDDEN ffi_closure_raw_SYSV(શૂન્ય);
શૂન્ય FFI_HIDDEN ffi_closure_raw_THISCALL(શૂન્ય);

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure *closure,
                          ffi_cif *cif,
                          શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,ffi_raw*,શૂન્ય*),
                          શૂન્ય *user_data,
                          શૂન્ય *codeloc)
{
  અક્ષર *tramp = closure->tramp;
  શૂન્ય (*dest)(શૂન્ય);
  પૂર્ણાંક i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  માટે (i = cif->nargs-1; i >= 0; i--)
    પસંદગી (cif->arg_types[i]->type)
      {
      સ્થિતિ FFI_TYPE_STRUCT:
      સ્થિતિ FFI_TYPE_LONGDOUBLE:
	પાછા FFI_BAD_TYPEDEF;
      }

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_THISCALL:
      dest = ffi_closure_raw_THISCALL;
      તોડો;
    સ્થિતિ FFI_SYSV:
      dest = ffi_closure_raw_SYSV;
      તોડો;
    મૂળભૂત:
      પાછા FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(શૂન્ય **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(unsigned *)(tramp + 6) = (unsigned)dest - ((unsigned)codeloc + 10);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  પાછા FFI_OK;
}

શૂન્ય
ffi_raw_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, ffi_raw *avalue)
{
  size_t rsize, bytes;
  સંરચના call_frame *frame;
  અક્ષર *ઢગલો, *argp;
  ffi_type **arg_types;
  પૂર્ણાંક flags, cabi, i, n, narg_reg;
  અચલ સંરચના abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  જો (rvalue == ખાલી)
    {
      પસંદગી (flags)
	{
	સ્થિતિ X86_RET_FLOAT:
	સ્થિતિ X86_RET_DOUBLE:
	સ્થિતિ X86_RET_LDOUBLE:
	સ્થિતિ X86_RET_STRUCTPOP:
	સ્થિતિ X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  તોડો;
	મૂળભૂત:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  તોડો;
	}
    }

  bytes = ઢગલો_ALIGN (cif->bytes);
  argp = ઢગલો =
      (શૂન્ય *)((uintptr_t)alloca(bytes + કદ(*frame) + rsize + 15) & ~16);
  frame = (સંરચના call_frame *)(ઢગલો + bytes);
  જો (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;

  narg_reg = 0;
  પસંદગી (flags)
    {
    સ્થિતિ X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      જો (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  તોડો;
	}
      /* fallthru */
    સ્થિતિ X86_RET_STRUCTPOP:
      *(શૂન્ય **)argp = rvalue;
      argp += કદ(શૂન્ય *);
      bytes -= કદ(શૂન્ય *);
      તોડો;
    }

  arg_types = cif->arg_types;
  માટે (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      પૂર્ણાંક t = ty->type;

      જો (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  frame->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      નહીંતર
	{
	  memcpy (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      bytes -= z;
    }
  જો (i < n)
    memcpy (argp, avalue, bytes);

  ffi_call_i386 (frame, ઢગલો);
}
#endif /* !FFI_NO_RAW_API */

#જો defined(FFI_EXEC_STATIC_TRAMP)
શૂન્ય *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  બાહ્ય શૂન્ય *trampoline_code_table;

  *map_size = X86_TRAMP_MAP_SIZE;
  *tramp_size = X86_TRAMP_SIZE;
  પાછા &trampoline_code_table;
}
#endif

#endif /* __i386__ */
