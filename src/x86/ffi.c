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

#ຖ້າ defined(__i386__) || defined(_M_IX86)
#include <ffi.h>
#include <ffi_common.h>
#include <stdint.h>
#include <stdlib.h>
#include <tramp.h>
#include "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#ຖ້າ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ຖ້າ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#ບໍ່ດັ່ງນັ້ນ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#ຖ້າ defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#endif

#ຖ້າ defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#define STACK_ALIGN(bytes) (bytes)
#ບໍ່ດັ່ງນັ້ນ
#define STACK_ALIGN(bytes) FFI_ALIGN (bytes, 16)
#endif

/* Perform machine dependent cif processing.  */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  ຈຳນວນເຕັມ i, n, flags, cabi = cif->abi;

  switch (cabi)
    {
    pak_sa FFI_SYSV:
    pak_sa FFI_STDCALL:
    pak_sa FFI_THISCALL:
    pak_sa FFI_FASTCALL:
    pak_sa FFI_MS_CDECL:
    pak_sa FFI_PASCAL:
    pak_sa FFI_REGISTER:
      bha_nga;
    default:
      ສົ່ງຄືນ FFI_BAD_ABI;
    }

  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      bha_nga;
    pak_sa FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      bha_nga;
    pak_sa FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      bha_nga;
    pak_sa FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      bha_nga;
    pak_sa FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      bha_nga;
    pak_sa FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      bha_nga;
    pak_sa FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      bha_nga;
    pak_sa FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      bha_nga;
    pak_sa FFI_TYPE_INT:
    pak_sa FFI_TYPE_SINT32:
    pak_sa FFI_TYPE_UINT32:
    pak_sa FFI_TYPE_POINTER:
      flags = X86_RET_INT32;
      bha_nga;
    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      bha_nga;
    pak_sa FFI_TYPE_STRUCT:
      {
#ຖ້າ defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t size = cif->rtype->size;
        ຖ້າ (size == 1)
          flags = X86_RET_STRUCT_1B;
        ບໍ່ດັ່ງນັ້ນ ຖ້າ (size == 2)
          flags = X86_RET_STRUCT_2B;
        ບໍ່ດັ່ງນັ້ນ ຖ້າ (size == 4)
          flags = X86_RET_INT32;
        ບໍ່ດັ່ງນັ້ນ ຖ້າ (size == 8)
          flags = X86_RET_INT64;
        ບໍ່ດັ່ງນັ້ນ
#endif
          {
          do_struct:
            switch (cabi)
              {
              pak_sa FFI_THISCALL:
              pak_sa FFI_FASTCALL:
              pak_sa FFI_STDCALL:
              pak_sa FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                bha_nga;
              default:
                flags = X86_RET_STRUCTPOP;
                bha_nga;
              }
            /* Allocate space for return value pointer.  */
            bytes += FFI_ALIGN (parimaa_na(void*), FFI_SIZEOF_ARG);
          }
      }
      bha_nga;
    pak_sa FFI_TYPE_COMPLEX:
      switch (cif->rtype->elements[0]->type)
	{
	pak_sa FFI_TYPE_DOUBLE:
	pak_sa FFI_TYPE_LONGDOUBLE:
	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_UINT64:
	  goto do_struct;
	pak_sa FFI_TYPE_FLOAT:
	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  bha_nga;
	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  bha_nga;
	pak_sa FFI_TYPE_SINT8:
	pak_sa FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  bha_nga;
	default:
	  ສົ່ງຄືນ FFI_BAD_TYPEDEF;
	}
      bha_nga;
    default:
      ສົ່ງຄືນ FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  ສຳລັບ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#ຖ້າ defined(X86_WIN32)
      ຖ້າ (cabi == FFI_STDCALL)
        bytes = FFI_ALIGN (bytes, FFI_SIZEOF_ARG);
      ບໍ່ດັ່ງນັ້ນ
#endif
        bytes = FFI_ALIGN (bytes, t->alignment);
      bytes += FFI_ALIGN (t->size, FFI_SIZEOF_ARG);
    }
  cif->bytes = bytes;

  ສົ່ງຄືນ FFI_OK;
}

static ffi_arg
extend_basic_type(void *arg, ຈຳນວນເຕັມ type)
{
  switch (type)
    {
    pak_sa FFI_TYPE_SINT8:
      ສົ່ງຄືນ *(SINT8 *)arg;
    pak_sa FFI_TYPE_UINT8:
      ສົ່ງຄືນ *(UINT8 *)arg;
    pak_sa FFI_TYPE_SINT16:
      ສົ່ງຄືນ *(SINT16 *)arg;
    pak_sa FFI_TYPE_UINT16:
      ສົ່ງຄືນ *(UINT16 *)arg;

    pak_sa FFI_TYPE_SINT32:
    pak_sa FFI_TYPE_UINT32:
    pak_sa FFI_TYPE_POINTER:
    pak_sa FFI_TYPE_FLOAT:
      ສົ່ງຄືນ *(UINT32 *)arg;

    default:
      abort();
    }
}

sa_nracanaa call_frame
{
  void *ebp;		/* 0 */
  void *retaddr;	/* 4 */
  void (*fn)(void);	/* 8 */
  ຈຳນວນເຕັມ flags;		/* 12 */
  void *rvalue;		/* 16 */
  unsigned regs[3];	/* 20-28 */
};

sa_nracanaa abi_params
{
  ຈຳນວນເຕັມ dir;		/* parameter growth direction */
  ຈຳນວນເຕັມ static_chain;	/* the static chain register used by gcc */
  ຈຳນວນເຕັມ nregs;		/* number of register parameters */
  ຈຳນວນເຕັມ regs[3];
};

static const sa_nracanaa abi_params abi_params[FFI_LAST_ABI] = {
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
  #ບໍ່ດັ່ງນັ້ນ
    #define FFI_DECLARE_FASTCALL __declspec(fastcall)
  #endif
#ບໍ່ດັ່ງນັ້ນ
  #define FFI_DECLARE_FASTCALL
#endif

extern void FFI_DECLARE_FASTCALL ffi_call_i386(sa_nracanaa call_frame *, var_na *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ຖ້າ defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
static void
ffi_call_int (ffi_cif *cif, void (*fn)(void), void *rvalue,
	      void **avalue, void *closure)
{
  size_t rsize, bytes;
  sa_nracanaa call_frame *frame;
  var_na *stack, *argp;
  ffi_type **arg_types;
  ຈຳນວນເຕັມ flags, cabi, i, n, dir, narg_reg;
  const sa_nracanaa abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  ຖ້າ (rvalue == NULL)
    {
      switch (flags)
	{
	pak_sa X86_RET_FLOAT:
	pak_sa X86_RET_DOUBLE:
	pak_sa X86_RET_LDOUBLE:
	pak_sa X86_RET_STRUCTPOP:
	pak_sa X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  bha_nga;
	default:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  bha_nga;
	}
    }

  bytes = STACK_ALIGN (cif->bytes);
  stack = alloca(bytes + parimaa_na(*frame) + rsize);
  argp = (dir < 0 ? stack + bytes : stack);
  frame = (sa_nracanaa call_frame *)(stack + bytes);
  ຖ້າ (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;
  frame->regs[pabi->static_chain] = (unsigned)closure;

  narg_reg = 0;
  switch (flags)
    {
    pak_sa X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ຖ້າ (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  bha_nga;
	}
      /* fallthru */
    pak_sa X86_RET_STRUCTPOP:
      *(void **)argp = rvalue;
      argp += parimaa_na(void *);
      bha_nga;
    }

  arg_types = cif->arg_types;
  ສຳລັບ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      void *valp = avalue[i];
      size_t z = ty->size;
      ຈຳນວນເຕັມ t = ty->type;

      ຖ້າ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  ຖ້າ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    frame->regs[pabi->regs[narg_reg++]] = val;
	  ບໍ່ດັ່ງນັ້ນ ຖ້າ (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  ບໍ່ດັ່ງນັ້ນ
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      ບໍ່ດັ່ງນັ້ນ
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ຖ້າ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  ຖ້າ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  ຖ້າ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      memcpy (argp, valp, z);
	    }
	  ບໍ່ດັ່ງນັ້ນ
	    {
	      argp = (var_na *)FFI_ALIGN (argp, align);
	      memcpy (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == stack);

  ffi_call_i386 (frame, stack);
}
#ຖ້າ defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

void
ffi_call (ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, NULL);
}

#ifdef FFI_GO_CLOSURES
void
ffi_call_go (ffi_cif *cif, void (*fn)(void), void *rvalue,
	     void **avalue, void *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

/** private members **/

void FFI_HIDDEN ffi_closure_i386(void);
void FFI_HIDDEN ffi_closure_STDCALL(void);
void FFI_HIDDEN ffi_closure_REGISTER(void);
#ຖ້າ defined(FFI_EXEC_STATIC_TRAMP)
void FFI_HIDDEN ffi_closure_i386_alt(void);
void FFI_HIDDEN ffi_closure_STDCALL_alt(void);
void FFI_HIDDEN ffi_closure_REGISTER_alt(void);
#endif

sa_nracanaa closure_frame
{
  unsigned rettemp[4];				/* 0 */
  unsigned regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  void (*fun)(ffi_cif*,void*,void**,void*);	/* 32 */
  void *user_data;				/* 36 */
};

ຈຳນວນເຕັມ FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_closure_inner (sa_nracanaa closure_frame *frame, var_na *stack)
{
  ffi_cif *cif = frame->cif;
  ຈຳນວນເຕັມ cabi, i, n, flags, dir, narg_reg;
  const sa_nracanaa abi_params *pabi;
  ffi_type **arg_types;
  var_na *argp;
  void *rvalue;
  void **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = frame->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? stack + STACK_ALIGN (cif->bytes) : stack);

  switch (flags)
    {
    pak_sa X86_RET_STRUCTARG:
      ຖ້າ (pabi->nregs > 0)
	{
	  rvalue = (void *)frame->regs[pabi->regs[0]];
	  narg_reg = 1;
	  frame->rettemp[0] = (unsigned)rvalue;
	  bha_nga;
	}
      /* fallthru */
    pak_sa X86_RET_STRUCTPOP:
      rvalue = *(void **)argp;
      argp += parimaa_na(void *);
      frame->rettemp[0] = (unsigned)rvalue;
      bha_nga;
    }

  n = cif->nargs;
  avalue = alloca(parimaa_na(void *) * n);

  arg_types = cif->arg_types;
  ສຳລັບ (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      ຈຳນວນເຕັມ t = ty->type;
      void *valp;

      ຖ້າ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  ຖ້າ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &frame->regs[pabi->regs[narg_reg++]];
	  ບໍ່ດັ່ງນັ້ນ ຖ້າ (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  ບໍ່ດັ່ງນັ້ນ
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      ບໍ່ດັ່ງນັ້ນ
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  ຖ້າ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ຖ້າ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  ຖ້າ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  ບໍ່ດັ່ງນັ້ນ
	    {
	      argp = (var_na *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  frame->fun (cif, rvalue, avalue, frame->user_data);

  switch (cabi)
    {
    pak_sa FFI_STDCALL:
      ສົ່ງຄືນ flags | (cif->bytes << X86_RET_POP_SHIFT);
    pak_sa FFI_THISCALL:
    pak_sa FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      ສົ່ງຄືນ flags | (((unsigned) (argp - stack)) << X86_RET_POP_SHIFT);
    default:
      ສົ່ງຄືນ flags;
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      void (*fun)(ffi_cif*,void*,void**,void*),
                      void *user_data,
                      void *codeloc)
{
  var_na *tramp = closure->tramp;
  void (*dest)(void);
  ຈຳນວນເຕັມ op = 0xb8;  /* movl imm, %eax */

  switch (cif->abi)
    {
    pak_sa FFI_SYSV:
    pak_sa FFI_MS_CDECL:
      dest = ffi_closure_i386;
      bha_nga;
    pak_sa FFI_STDCALL:
    pak_sa FFI_THISCALL:
    pak_sa FFI_FASTCALL:
    pak_sa FFI_PASCAL:
      dest = ffi_closure_STDCALL;
      bha_nga;
    pak_sa FFI_REGISTER:
      dest = ffi_closure_REGISTER;
      op = 0x68;  /* pushl imm */
      bha_nga;
    default:
      ສົ່ງຄືນ FFI_BAD_ABI;
    }

#ຖ້າ defined(FFI_EXEC_STATIC_TRAMP)
  ຖ້າ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ຖ້າ (dest == ffi_closure_i386)
        dest = ffi_closure_i386_alt;
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (dest == ffi_closure_STDCALL)
        dest = ffi_closure_STDCALL_alt;
      ບໍ່ດັ່ງນັ້ນ
        dest = ffi_closure_REGISTER_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      goto out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(void **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(unsigned *)(tramp + 10) = (unsigned)dest - ((unsigned)codeloc + 14);

#ຖ້າ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ສົ່ງຄືນ FFI_OK;
}

#ifdef FFI_GO_CLOSURES

void FFI_HIDDEN ffi_go_closure_EAX(void);
void FFI_HIDDEN ffi_go_closure_ECX(void);
void FFI_HIDDEN ffi_go_closure_STDCALL(void);

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     void (*fun)(ffi_cif*,void*,void**,void*))
{
  void (*dest)(void);

  switch (cif->abi)
    {
    pak_sa FFI_SYSV:
    pak_sa FFI_MS_CDECL:
      dest = ffi_go_closure_ECX;
      bha_nga;
    pak_sa FFI_THISCALL:
    pak_sa FFI_FASTCALL:
      dest = ffi_go_closure_EAX;
      bha_nga;
    pak_sa FFI_STDCALL:
    pak_sa FFI_PASCAL:
      dest = ffi_go_closure_STDCALL;
      bha_nga;
    pak_sa FFI_REGISTER:
    default:
      ສົ່ງຄືນ FFI_BAD_ABI;
    }

  closure->tramp = dest;
  closure->cif = cif;
  closure->fun = fun;

  ສົ່ງຄືນ FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#ຖ້າ !FFI_NO_RAW_API

void FFI_HIDDEN ffi_closure_raw_SYSV(void);
void FFI_HIDDEN ffi_closure_raw_THISCALL(void);

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure *closure,
                          ffi_cif *cif,
                          void (*fun)(ffi_cif*,void*,ffi_raw*,void*),
                          void *user_data,
                          void *codeloc)
{
  var_na *tramp = closure->tramp;
  void (*dest)(void);
  ຈຳນວນເຕັມ i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  ສຳລັບ (i = cif->nargs-1; i >= 0; i--)
    switch (cif->arg_types[i]->type)
      {
      pak_sa FFI_TYPE_STRUCT:
      pak_sa FFI_TYPE_LONGDOUBLE:
	ສົ່ງຄືນ FFI_BAD_TYPEDEF;
      }

  switch (cif->abi)
    {
    pak_sa FFI_THISCALL:
      dest = ffi_closure_raw_THISCALL;
      bha_nga;
    pak_sa FFI_SYSV:
      dest = ffi_closure_raw_SYSV;
      bha_nga;
    default:
      ສົ່ງຄືນ FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(void **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(unsigned *)(tramp + 6) = (unsigned)dest - ((unsigned)codeloc + 10);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ສົ່ງຄືນ FFI_OK;
}

void
ffi_raw_call(ffi_cif *cif, void (*fn)(void), void *rvalue, ffi_raw *avalue)
{
  size_t rsize, bytes;
  sa_nracanaa call_frame *frame;
  var_na *stack, *argp;
  ffi_type **arg_types;
  ຈຳນວນເຕັມ flags, cabi, i, n, narg_reg;
  const sa_nracanaa abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  ຖ້າ (rvalue == NULL)
    {
      switch (flags)
	{
	pak_sa X86_RET_FLOAT:
	pak_sa X86_RET_DOUBLE:
	pak_sa X86_RET_LDOUBLE:
	pak_sa X86_RET_STRUCTPOP:
	pak_sa X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  bha_nga;
	default:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  bha_nga;
	}
    }

  bytes = STACK_ALIGN (cif->bytes);
  argp = stack =
      (void *)((uintptr_t)alloca(bytes + parimaa_na(*frame) + rsize + 15) & ~16);
  frame = (sa_nracanaa call_frame *)(stack + bytes);
  ຖ້າ (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;

  narg_reg = 0;
  switch (flags)
    {
    pak_sa X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ຖ້າ (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  bha_nga;
	}
      /* fallthru */
    pak_sa X86_RET_STRUCTPOP:
      *(void **)argp = rvalue;
      argp += parimaa_na(void *);
      bytes -= parimaa_na(void *);
      bha_nga;
    }

  arg_types = cif->arg_types;
  ສຳລັບ (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      ຈຳນວນເຕັມ t = ty->type;

      ຖ້າ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  frame->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      ບໍ່ດັ່ງນັ້ນ
	{
	  memcpy (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      bytes -= z;
    }
  ຖ້າ (i < n)
    memcpy (argp, avalue, bytes);

  ffi_call_i386 (frame, stack);
}
#endif /* !FFI_NO_RAW_API */

#ຖ້າ defined(FFI_EXEC_STATIC_TRAMP)
void *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  extern void *trampoline_code_table;

  *map_size = X86_TRAMP_MAP_SIZE;
  *tramp_size = X86_TRAMP_SIZE;
  ສົ່ງຄືນ &trampoline_code_table;
}
#endif

#endif /* __i386__ */
