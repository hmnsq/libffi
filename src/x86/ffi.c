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

#اگر defined(__i386__) || defined(_M_IX86)
#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdint.h>
#شامل <stdlib.h>
#شامل <tramp.h>
#شامل "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# اگر FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#وگرنه
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#اگر defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#endif

#اگر defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#define پشته_ALIGN(bytes) (bytes)
#وگرنه
#define پشته_ALIGN(bytes) FFI_ALIGN (bytes, 16)
#endif

/* Perform machine dependent cif processing.  */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  صحیح i, n, flags, cabi = cif->abi;

  انتخاب (cabi)
    {
    حالت FFI_SYSV:
    حالت FFI_STDCALL:
    حالت FFI_THISCALL:
    حالت FFI_FASTCALL:
    حالت FFI_MS_CDECL:
    حالت FFI_PASCAL:
    حالت FFI_REGISTER:
      بشکن;
    پیشفرض:
      بازگشت FFI_BAD_ABI;
    }

  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      بشکن;
    حالت FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      بشکن;
    حالت FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      بشکن;
    حالت FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      بشکن;
    حالت FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      بشکن;
    حالت FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      بشکن;
    حالت FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      بشکن;
    حالت FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      بشکن;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_POINTER:
      flags = X86_RET_INT32;
      بشکن;
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      بشکن;
    حالت FFI_TYPE_STRUCT:
      {
#اگر defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t size = cif->rtype->size;
        اگر (size == 1)
          flags = X86_RET_STRUCT_1B;
        وگرنه اگر (size == 2)
          flags = X86_RET_STRUCT_2B;
        وگرنه اگر (size == 4)
          flags = X86_RET_INT32;
        وگرنه اگر (size == 8)
          flags = X86_RET_INT64;
        وگرنه
#endif
          {
          do_struct:
            انتخاب (cabi)
              {
              حالت FFI_THISCALL:
              حالت FFI_FASTCALL:
              حالت FFI_STDCALL:
              حالت FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                بشکن;
              پیشفرض:
                flags = X86_RET_STRUCTPOP;
                بشکن;
              }
            /* Allocate space for return value pointer.  */
            bytes += FFI_ALIGN (اندازه(تهی*), FFI_SIZEOF_ARG);
          }
      }
      بشکن;
    حالت FFI_TYPE_COMPLEX:
      انتخاب (cif->rtype->elements[0]->type)
	{
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	  برو do_struct;
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  بشکن;
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  بشکن;
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  بشکن;
	پیشفرض:
	  بازگشت FFI_BAD_TYPEDEF;
	}
      بشکن;
    پیشفرض:
      بازگشت FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  برای (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#اگر defined(X86_WIN32)
      اگر (cabi == FFI_STDCALL)
        bytes = FFI_ALIGN (bytes, FFI_SIZEOF_ARG);
      وگرنه
#endif
        bytes = FFI_ALIGN (bytes, t->alignment);
      bytes += FFI_ALIGN (t->size, FFI_SIZEOF_ARG);
    }
  cif->bytes = bytes;

  بازگشت FFI_OK;
}

ایستا ffi_arg
extend_basic_type(تهی *arg, صحیح type)
{
  انتخاب (type)
    {
    حالت FFI_TYPE_SINT8:
      بازگشت *(SINT8 *)arg;
    حالت FFI_TYPE_UINT8:
      بازگشت *(UINT8 *)arg;
    حالت FFI_TYPE_SINT16:
      بازگشت *(SINT16 *)arg;
    حالت FFI_TYPE_UINT16:
      بازگشت *(UINT16 *)arg;

    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_POINTER:
    حالت FFI_TYPE_FLOAT:
      بازگشت *(UINT32 *)arg;

    پیشفرض:
      abort();
    }
}

ساختار call_frame
{
  تهی *ebp;		/* 0 */
  تهی *retaddr;	/* 4 */
  تهی (*fn)(تهی);	/* 8 */
  صحیح flags;		/* 12 */
  تهی *rvalue;		/* 16 */
  unsigned regs[3];	/* 20-28 */
};

ساختار abi_params
{
  صحیح dir;		/* parameter growth direction */
  صحیح static_chain;	/* the static chain register used by gcc */
  صحیح nregs;		/* number of register parameters */
  صحیح regs[3];
};

ایستا ثابت ساختار abi_params abi_params[FFI_LAST_ABI] = {
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
  #وگرنه
    #define FFI_DECLARE_FASTCALL __declspec(fastcall)
  #endif
#وگرنه
  #define FFI_DECLARE_FASTCALL
#endif

بیرونی تهی FFI_DECLARE_FASTCALL ffi_call_i386(ساختار call_frame *, نویسه *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#اگر defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ایستا تهی
ffi_call_int (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
	      تهی **avalue, تهی *closure)
{
  size_t rsize, bytes;
  ساختار call_frame *frame;
  نویسه *پشته, *argp;
  ffi_type **arg_types;
  صحیح flags, cabi, i, n, dir, narg_reg;
  ثابت ساختار abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  اگر (rvalue == پوچ)
    {
      انتخاب (flags)
	{
	حالت X86_RET_FLOAT:
	حالت X86_RET_DOUBLE:
	حالت X86_RET_LDOUBLE:
	حالت X86_RET_STRUCTPOP:
	حالت X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  بشکن;
	پیشفرض:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  بشکن;
	}
    }

  bytes = پشته_ALIGN (cif->bytes);
  پشته = alloca(bytes + اندازه(*frame) + rsize);
  argp = (dir < 0 ? پشته + bytes : پشته);
  frame = (ساختار call_frame *)(پشته + bytes);
  اگر (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;
  frame->regs[pabi->static_chain] = (unsigned)closure;

  narg_reg = 0;
  انتخاب (flags)
    {
    حالت X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      اگر (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  بشکن;
	}
      /* fallthru */
    حالت X86_RET_STRUCTPOP:
      *(تهی **)argp = rvalue;
      argp += اندازه(تهی *);
      بشکن;
    }

  arg_types = cif->arg_types;
  برای (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      تهی *valp = avalue[i];
      size_t z = ty->size;
      صحیح t = ty->type;

      اگر (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  اگر (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    frame->regs[pabi->regs[narg_reg++]] = val;
	  وگرنه اگر (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  وگرنه
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      وگرنه
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  اگر ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  اگر (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  اگر (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      memcpy (argp, valp, z);
	    }
	  وگرنه
	    {
	      argp = (نویسه *)FFI_ALIGN (argp, align);
	      memcpy (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == پشته);

  ffi_call_i386 (frame, پشته);
}
#اگر defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

تهی
ffi_call (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, پوچ);
}

#ifdef FFI_GO_CLOSURES
تهی
ffi_call_go (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
	     تهی **avalue, تهی *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

/** private members **/

تهی FFI_HIDDEN ffi_closure_i386(تهی);
تهی FFI_HIDDEN ffi_closure_STDCALL(تهی);
تهی FFI_HIDDEN ffi_closure_REGISTER(تهی);
#اگر defined(FFI_EXEC_STATIC_TRAMP)
تهی FFI_HIDDEN ffi_closure_i386_alt(تهی);
تهی FFI_HIDDEN ffi_closure_STDCALL_alt(تهی);
تهی FFI_HIDDEN ffi_closure_REGISTER_alt(تهی);
#endif

ساختار closure_frame
{
  unsigned rettemp[4];				/* 0 */
  unsigned regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*);	/* 32 */
  تهی *user_data;				/* 36 */
};

صحیح FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_closure_inner (ساختار closure_frame *frame, نویسه *پشته)
{
  ffi_cif *cif = frame->cif;
  صحیح cabi, i, n, flags, dir, narg_reg;
  ثابت ساختار abi_params *pabi;
  ffi_type **arg_types;
  نویسه *argp;
  تهی *rvalue;
  تهی **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = frame->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? پشته + پشته_ALIGN (cif->bytes) : پشته);

  انتخاب (flags)
    {
    حالت X86_RET_STRUCTARG:
      اگر (pabi->nregs > 0)
	{
	  rvalue = (تهی *)frame->regs[pabi->regs[0]];
	  narg_reg = 1;
	  frame->rettemp[0] = (unsigned)rvalue;
	  بشکن;
	}
      /* fallthru */
    حالت X86_RET_STRUCTPOP:
      rvalue = *(تهی **)argp;
      argp += اندازه(تهی *);
      frame->rettemp[0] = (unsigned)rvalue;
      بشکن;
    }

  n = cif->nargs;
  avalue = alloca(اندازه(تهی *) * n);

  arg_types = cif->arg_types;
  برای (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      صحیح t = ty->type;
      تهی *valp;

      اگر (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  اگر (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &frame->regs[pabi->regs[narg_reg++]];
	  وگرنه اگر (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  وگرنه
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      وگرنه
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  اگر (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  اگر ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  اگر (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  وگرنه
	    {
	      argp = (نویسه *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  frame->fun (cif, rvalue, avalue, frame->user_data);

  انتخاب (cabi)
    {
    حالت FFI_STDCALL:
      بازگشت flags | (cif->bytes << X86_RET_برداشت_SHIFT);
    حالت FFI_THISCALL:
    حالت FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      بازگشت flags | (((unsigned) (argp - پشته)) << X86_RET_برداشت_SHIFT);
    پیشفرض:
      بازگشت flags;
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*),
                      تهی *user_data,
                      تهی *codeloc)
{
  نویسه *tramp = closure->tramp;
  تهی (*dest)(تهی);
  صحیح op = 0xb8;  /* movl imm, %eax */

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
    حالت FFI_MS_CDECL:
      dest = ffi_closure_i386;
      بشکن;
    حالت FFI_STDCALL:
    حالت FFI_THISCALL:
    حالت FFI_FASTCALL:
    حالت FFI_PASCAL:
      dest = ffi_closure_STDCALL;
      بشکن;
    حالت FFI_REGISTER:
      dest = ffi_closure_REGISTER;
      op = 0x68;  /* pushl imm */
      بشکن;
    پیشفرض:
      بازگشت FFI_BAD_ABI;
    }

#اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      اگر (dest == ffi_closure_i386)
        dest = ffi_closure_i386_alt;
      وگرنه اگر (dest == ffi_closure_STDCALL)
        dest = ffi_closure_STDCALL_alt;
      وگرنه
        dest = ffi_closure_REGISTER_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      برو out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(تهی **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(unsigned *)(tramp + 10) = (unsigned)dest - ((unsigned)codeloc + 14);

#اگر defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  بازگشت FFI_OK;
}

#ifdef FFI_GO_CLOSURES

تهی FFI_HIDDEN ffi_go_closure_EAX(تهی);
تهی FFI_HIDDEN ffi_go_closure_ECX(تهی);
تهی FFI_HIDDEN ffi_go_closure_STDCALL(تهی);

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*))
{
  تهی (*dest)(تهی);

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
    حالت FFI_MS_CDECL:
      dest = ffi_go_closure_ECX;
      بشکن;
    حالت FFI_THISCALL:
    حالت FFI_FASTCALL:
      dest = ffi_go_closure_EAX;
      بشکن;
    حالت FFI_STDCALL:
    حالت FFI_PASCAL:
      dest = ffi_go_closure_STDCALL;
      بشکن;
    حالت FFI_REGISTER:
    پیشفرض:
      بازگشت FFI_BAD_ABI;
    }

  closure->tramp = dest;
  closure->cif = cif;
  closure->fun = fun;

  بازگشت FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#اگر !FFI_NO_RAW_API

تهی FFI_HIDDEN ffi_closure_raw_SYSV(تهی);
تهی FFI_HIDDEN ffi_closure_raw_THISCALL(تهی);

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure *closure,
                          ffi_cif *cif,
                          تهی (*fun)(ffi_cif*,تهی*,ffi_raw*,تهی*),
                          تهی *user_data,
                          تهی *codeloc)
{
  نویسه *tramp = closure->tramp;
  تهی (*dest)(تهی);
  صحیح i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  برای (i = cif->nargs-1; i >= 0; i--)
    انتخاب (cif->arg_types[i]->type)
      {
      حالت FFI_TYPE_STRUCT:
      حالت FFI_TYPE_LONGDOUBLE:
	بازگشت FFI_BAD_TYPEDEF;
      }

  انتخاب (cif->abi)
    {
    حالت FFI_THISCALL:
      dest = ffi_closure_raw_THISCALL;
      بشکن;
    حالت FFI_SYSV:
      dest = ffi_closure_raw_SYSV;
      بشکن;
    پیشفرض:
      بازگشت FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(تهی **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(unsigned *)(tramp + 6) = (unsigned)dest - ((unsigned)codeloc + 10);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  بازگشت FFI_OK;
}

تهی
ffi_raw_call(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, ffi_raw *avalue)
{
  size_t rsize, bytes;
  ساختار call_frame *frame;
  نویسه *پشته, *argp;
  ffi_type **arg_types;
  صحیح flags, cabi, i, n, narg_reg;
  ثابت ساختار abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  اگر (rvalue == پوچ)
    {
      انتخاب (flags)
	{
	حالت X86_RET_FLOAT:
	حالت X86_RET_DOUBLE:
	حالت X86_RET_LDOUBLE:
	حالت X86_RET_STRUCTPOP:
	حالت X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  بشکن;
	پیشفرض:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  بشکن;
	}
    }

  bytes = پشته_ALIGN (cif->bytes);
  argp = پشته =
      (تهی *)((uintptr_t)alloca(bytes + اندازه(*frame) + rsize + 15) & ~16);
  frame = (ساختار call_frame *)(پشته + bytes);
  اگر (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;

  narg_reg = 0;
  انتخاب (flags)
    {
    حالت X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      اگر (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (unsigned)rvalue;
	  narg_reg = 1;
	  بشکن;
	}
      /* fallthru */
    حالت X86_RET_STRUCTPOP:
      *(تهی **)argp = rvalue;
      argp += اندازه(تهی *);
      bytes -= اندازه(تهی *);
      بشکن;
    }

  arg_types = cif->arg_types;
  برای (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;
      صحیح t = ty->type;

      اگر (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  frame->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      وگرنه
	{
	  memcpy (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      bytes -= z;
    }
  اگر (i < n)
    memcpy (argp, avalue, bytes);

  ffi_call_i386 (frame, پشته);
}
#endif /* !FFI_NO_RAW_API */

#اگر defined(FFI_EXEC_STATIC_TRAMP)
تهی *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  بیرونی تهی *trampoline_code_table;

  *map_size = X86_TRAMP_MAP_SIZE;
  *tramp_size = X86_TRAMP_SIZE;
  بازگشت &trampoline_code_table;
}
#endif

#endif /* __i386__ */
