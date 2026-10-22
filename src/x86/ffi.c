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

#إذا defined(__i386__) || defined(_M_IX86)
#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <stdint.h>
#ضمن <stdlib.h>
#ضمن <tramp.h>
#ضمن "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# إذا FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ٻاهريون جو date
# endif
#وإلا
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#جيڪڏهن_پورو

#إذا defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#جيڪڏهن_پورو

#إذا defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#وصف_ڪريو ڍير_ALIGN(بائيٽون) (بائيٽون)
#وإلا
#وصف_ڪريو ڍير_ALIGN(بائيٽون) FFI_ALIGN (بائيٽون, 16)
#جيڪڏهن_پورو

/* Perform machine dependent cif processing.  */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t بائيٽون = 0;
  صحيح i, n, flags, cabi = cif->abi;

  بدل (cabi)
    {
    حالة FFI_SYSV:
    حالة FFI_STDCALL:
    حالة FFI_THISCALL:
    حالة FFI_FASTCALL:
    حالة FFI_MS_CDECL:
    حالة FFI_PASCAL:
    حالة FFI_رجسٽر:
      اكسر;
    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      اكسر;
    حالة FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      اكسر;
    حالة FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      اكسر;
    حالة FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      اكسر;
    حالة FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      اكسر;
    حالة FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      اكسر;
    حالة FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      اكسر;
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_اشارو:
      flags = X86_RET_INT32;
      اكسر;
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      اكسر;
    حالة FFI_TYPE_STRUCT:
      {
#إذا defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t سائيز = cif->rtype->سائيز;
        إذا (سائيز == 1)
          flags = X86_RET_STRUCT_1B;
        وإلا إذا (سائيز == 2)
          flags = X86_RET_STRUCT_2B;
        وإلا إذا (سائيز == 4)
          flags = X86_RET_INT32;
        وإلا إذا (سائيز == 8)
          flags = X86_RET_INT64;
        وإلا
#جيڪڏهن_پورو
          {
          do_struct:
            بدل (cabi)
              {
              حالة FFI_THISCALL:
              حالة FFI_FASTCALL:
              حالة FFI_STDCALL:
              حالة FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                اكسر;
              افتراضي:
                flags = X86_RET_STRUCTPOP;
                اكسر;
              }
            /* Allocate space for return value pointer.  */
            بائيٽون += FFI_ALIGN (حجم(فراغ*), FFI_SIZEOF_ARG);
          }
      }
      اكسر;
    حالة FFI_TYPE_COMPLEX:
      بدل (cif->rtype->elements[0]->type)
	{
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	  اذهب do_struct;
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  اكسر;
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  اكسر;
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  اكسر;
	افتراضي:
	  أرجع FFI_BAD_TYPEDEF;
	}
      اكسر;
    افتراضي:
      أرجع FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  لكل (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#إذا defined(X86_WIN32)
      إذا (cabi == FFI_STDCALL)
        بائيٽون = FFI_ALIGN (بائيٽون, FFI_SIZEOF_ARG);
      وإلا
#جيڪڏهن_پورو
        بائيٽون = FFI_ALIGN (بائيٽون, t->alignment);
      بائيٽون += FFI_ALIGN (t->سائيز, FFI_SIZEOF_ARG);
    }
  cif->بائيٽون = بائيٽون;

  أرجع FFI_OK;
}

ساكن ffi_arg
extend_basic_type(فراغ *arg, صحيح type)
{
  بدل (type)
    {
    حالة FFI_TYPE_SINT8:
      أرجع *(SINT8 *)arg;
    حالة FFI_TYPE_UINT8:
      أرجع *(UINT8 *)arg;
    حالة FFI_TYPE_SINT16:
      أرجع *(SINT16 *)arg;
    حالة FFI_TYPE_UINT16:
      أرجع *(UINT16 *)arg;

    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_اشارو:
    حالة FFI_TYPE_FLOAT:
      أرجع *(UINT32 *)arg;

    افتراضي:
      abort();
    }
}

بنية call_چوکٺو
{
  فراغ *ebp;		/* 0 */
  فراغ *retaddr;	/* 4 */
  فراغ (*fn)(فراغ);	/* 8 */
  صحيح flags;		/* 12 */
  فراغ *rvalue;		/* 16 */
  بنانشان regs[3];	/* 20-28 */
};

بنية abi_params
{
  صحيح dir;		/* parameter growth direction */
  صحيح static_chain;	/* the static chain register used by gcc */
  صحيح nregs;		/* number of register parameters */
  صحيح regs[3];
};

ساكن ثابت بنية abi_params abi_params[FFI_آخري_ABI] = {
  [FFI_SYSV] = { 1, R_ECX, 0 },
  [FFI_THISCALL] = { 1, R_EAX, 1, { R_ECX } },
  [FFI_FASTCALL] = { 1, R_EAX, 2, { R_ECX, R_EDX } },
  [FFI_STDCALL] = { 1, R_ECX, 0 },
  [FFI_PASCAL] = { -1, R_ECX, 0 },
  /* ??? No defined static chain; gcc does not support REGISTER.  */
  [FFI_رجسٽر] = { -1, R_ECX, 3, { R_EAX, R_EDX, R_ECX } },
  [FFI_MS_CDECL] = { 1, R_ECX, 0 }
};

#جيڪڏهن_وصف_ٿيل HAVE_FASTCALL
  #جيڪڏهن_وصف_ٿيل _MSC_VER
    #وصف_ڪريو FFI_DECLARE_FASTCALL __fastcall
  #وإلا
    #وصف_ڪريو FFI_DECLARE_FASTCALL __declspec(fastcall)
  #جيڪڏهن_پورو
#وإلا
  #وصف_ڪريو FFI_DECLARE_FASTCALL
#جيڪڏهن_پورو

خارجي فراغ FFI_DECLARE_FASTCALL ffi_call_i386(بنية call_چوکٺو *, حرف *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#إذا defined(_MSC_VER)
#هدايت runtime_checks("s", off)
#جيڪڏهن_پورو
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	      فراغ **avalue, فراغ *گهيرو)
{
  size_t rsize, بائيٽون;
  بنية call_چوکٺو *چوکٺو;
  حرف *ڍير, *argp;
  ffi_type **arg_types;
  صحيح flags, cabi, i, n, dir, narg_reg;
  ثابت بنية abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  إذا (rvalue == عدم)
    {
      بدل (flags)
	{
	حالة X86_RET_FLOAT:
	حالة X86_RET_DOUBLE:
	حالة X86_RET_LDOUBLE:
	حالة X86_RET_STRUCTPOP:
	حالة X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->سائيز;
	  اكسر;
	افتراضي:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  اكسر;
	}
    }

  بائيٽون = ڍير_ALIGN (cif->بائيٽون);
  ڍير = alloca(بائيٽون + حجم(*چوکٺو) + rsize);
  argp = (dir < 0 ? ڍير + بائيٽون : ڍير);
  چوکٺو = (بنية call_چوکٺو *)(ڍير + بائيٽون);
  إذا (rsize)
    rvalue = چوکٺو + 1;

  چوکٺو->fn = fn;
  چوکٺو->flags = flags;
  چوکٺو->rvalue = rvalue;
  چوکٺو->regs[pabi->static_chain] = (بنانشان)گهيرو;

  narg_reg = 0;
  بدل (flags)
    {
    حالة X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      إذا (pabi->nregs > 0)
	{
	  چوکٺو->regs[pabi->regs[0]] = (بنانشان)rvalue;
	  narg_reg = 1;
	  اكسر;
	}
      /* fallthru */
    حالة X86_RET_STRUCTPOP:
      *(فراغ **)argp = rvalue;
      argp += حجم(فراغ *);
      اكسر;
    }

  arg_types = cif->arg_types;
  لكل (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      فراغ *valp = avalue[i];
      size_t z = ty->سائيز;
      صحيح t = ty->type;

      إذا (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  إذا (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    چوکٺو->regs[pabi->regs[narg_reg++]] = val;
	  وإلا إذا (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  وإلا
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      وإلا
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  إذا ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  إذا (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  إذا (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      يادگيري_نقل (argp, valp, z);
	    }
	  وإلا
	    {
	      argp = (حرف *)FFI_ALIGN (argp, align);
	      يادگيري_نقل (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == ڍير);

  ffi_call_i386 (چوکٺو, ڍير);
}
#إذا defined(_MSC_VER)
#هدايت runtime_checks("s", restore)
#جيڪڏهن_پورو

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

#جيڪڏهن_وصف_ٿيل FFI_GO_CLOSURES
فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گهيرو)
{
  ffi_call_int (cif, fn, rvalue, avalue, گهيرو);
}
#جيڪڏهن_پورو

/** private members **/

فراغ FFI_HIDDEN ffi_گهيرو_i386(فراغ);
فراغ FFI_HIDDEN ffi_گهيرو_STDCALL(فراغ);
فراغ FFI_HIDDEN ffi_گهيرو_رجسٽر(فراغ);
#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ FFI_HIDDEN ffi_گهيرو_i386_alt(فراغ);
فراغ FFI_HIDDEN ffi_گهيرو_STDCALL_alt(فراغ);
فراغ FFI_HIDDEN ffi_گهيرو_رجسٽر_alt(فراغ);
#جيڪڏهن_پورو

بنية گهيرو_چوکٺو
{
  بنانشان rettemp[4];				/* 0 */
  بنانشان regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*);	/* 32 */
  فراغ *user_معلومات;				/* 36 */
};

صحيح FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_گهيرو_inner (بنية گهيرو_چوکٺو *چوکٺو, حرف *ڍير)
{
  ffi_cif *cif = چوکٺو->cif;
  صحيح cabi, i, n, flags, dir, narg_reg;
  ثابت بنية abi_params *pabi;
  ffi_type **arg_types;
  حرف *argp;
  فراغ *rvalue;
  فراغ **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = چوکٺو->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? ڍير + ڍير_ALIGN (cif->بائيٽون) : ڍير);

  بدل (flags)
    {
    حالة X86_RET_STRUCTARG:
      إذا (pabi->nregs > 0)
	{
	  rvalue = (فراغ *)چوکٺو->regs[pabi->regs[0]];
	  narg_reg = 1;
	  چوکٺو->rettemp[0] = (بنانشان)rvalue;
	  اكسر;
	}
      /* fallthru */
    حالة X86_RET_STRUCTPOP:
      rvalue = *(فراغ **)argp;
      argp += حجم(فراغ *);
      چوکٺو->rettemp[0] = (بنانشان)rvalue;
      اكسر;
    }

  n = cif->nargs;
  avalue = alloca(حجم(فراغ *) * n);

  arg_types = cif->arg_types;
  لكل (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->سائيز;
      صحيح t = ty->type;
      فراغ *valp;

      إذا (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  إذا (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &چوکٺو->regs[pabi->regs[narg_reg++]];
	  وإلا إذا (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  وإلا
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      وإلا
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  إذا (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  إذا ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  إذا (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  وإلا
	    {
	      argp = (حرف *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  چوکٺو->fun (cif, rvalue, avalue, چوکٺو->user_معلومات);

  بدل (cabi)
    {
    حالة FFI_STDCALL:
      أرجع flags | (cif->بائيٽون << X86_RET_ڪڍو_سرڪايو);
    حالة FFI_THISCALL:
    حالة FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      أرجع flags | (((بنانشان) (argp - ڍير)) << X86_RET_ڪڍو_سرڪايو);
    افتراضي:
      أرجع flags;
    }
}

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو* گهيرو,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_معلومات,
                      فراغ *codeloc)
{
  حرف *tramp = گهيرو->tramp;
  فراغ (*dest)(فراغ);
  صحيح op = 0xb8;  /* movl imm, %eax */

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
    حالة FFI_MS_CDECL:
      dest = ffi_گهيرو_i386;
      اكسر;
    حالة FFI_STDCALL:
    حالة FFI_THISCALL:
    حالة FFI_FASTCALL:
    حالة FFI_PASCAL:
      dest = ffi_گهيرو_STDCALL;
      اكسر;
    حالة FFI_رجسٽر:
      dest = ffi_گهيرو_رجسٽر;
      op = 0x68;  /* pushl imm */
      اكسر;
    افتراضي:
      أرجع FFI_BAD_ABI;
    }

#إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(گهيرو))
    {
      /* Initialize the static trampoline's parameters. */
      إذا (dest == ffi_گهيرو_i386)
        dest = ffi_گهيرو_i386_alt;
      وإلا إذا (dest == ffi_گهيرو_STDCALL)
        dest = ffi_گهيرو_STDCALL_alt;
      وإلا
        dest = ffi_گهيرو_رجسٽر_alt;
      ffi_tramp_سيٽ_parms (گهيرو->ftramp, dest, گهيرو);
      اذهب ٻاهريون;
    }
#جيڪڏهن_پورو

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(فراغ **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(بنانشان *)(tramp + 10) = (بنانشان)dest - ((بنانشان)codeloc + 14);

#إذا defined(FFI_EXEC_STATIC_TRAMP)
ٻاهريون:
#جيڪڏهن_پورو
  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

  أرجع FFI_OK;
}

#جيڪڏهن_وصف_ٿيل FFI_GO_CLOSURES

فراغ FFI_HIDDEN ffi_go_گهيرو_EAX(فراغ);
فراغ FFI_HIDDEN ffi_go_گهيرو_ECX(فراغ);
فراغ FFI_HIDDEN ffi_go_گهيرو_STDCALL(فراغ);

ffi_حالت
ffi_prep_go_گهيرو (ffi_go_گهيرو* گهيرو, ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*))
{
  فراغ (*dest)(فراغ);

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
    حالة FFI_MS_CDECL:
      dest = ffi_go_گهيرو_ECX;
      اكسر;
    حالة FFI_THISCALL:
    حالة FFI_FASTCALL:
      dest = ffi_go_گهيرو_EAX;
      اكسر;
    حالة FFI_STDCALL:
    حالة FFI_PASCAL:
      dest = ffi_go_گهيرو_STDCALL;
      اكسر;
    حالة FFI_رجسٽر:
    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  گهيرو->tramp = dest;
  گهيرو->cif = cif;
  گهيرو->fun = fun;

  أرجع FFI_OK;
}

#جيڪڏهن_پورو /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#إذا !FFI_NO_RAW_API

فراغ FFI_HIDDEN ffi_گهيرو_raw_SYSV(فراغ);
فراغ FFI_HIDDEN ffi_گهيرو_raw_THISCALL(فراغ);

ffi_حالت
ffi_prep_raw_گهيرو_loc (ffi_raw_گهيرو *گهيرو,
                          ffi_cif *cif,
                          فراغ (*fun)(ffi_cif*,فراغ*,ffi_raw*,فراغ*),
                          فراغ *user_معلومات,
                          فراغ *codeloc)
{
  حرف *tramp = گهيرو->tramp;
  فراغ (*dest)(فراغ);
  صحيح i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  لكل (i = cif->nargs-1; i >= 0; i--)
    بدل (cif->arg_types[i]->type)
      {
      حالة FFI_TYPE_STRUCT:
      حالة FFI_TYPE_LONGDOUBLE:
	أرجع FFI_BAD_TYPEDEF;
      }

  بدل (cif->abi)
    {
    حالة FFI_THISCALL:
      dest = ffi_گهيرو_raw_THISCALL;
      اكسر;
    حالة FFI_SYSV:
      dest = ffi_گهيرو_raw_SYSV;
      اكسر;
    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(فراغ **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(بنانشان *)(tramp + 6) = (بنانشان)dest - ((بنانشان)codeloc + 10);

  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

  أرجع FFI_OK;
}

فراغ
ffi_raw_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, ffi_raw *avalue)
{
  size_t rsize, بائيٽون;
  بنية call_چوکٺو *چوکٺو;
  حرف *ڍير, *argp;
  ffi_type **arg_types;
  صحيح flags, cabi, i, n, narg_reg;
  ثابت بنية abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  إذا (rvalue == عدم)
    {
      بدل (flags)
	{
	حالة X86_RET_FLOAT:
	حالة X86_RET_DOUBLE:
	حالة X86_RET_LDOUBLE:
	حالة X86_RET_STRUCTPOP:
	حالة X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->سائيز;
	  اكسر;
	افتراضي:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  اكسر;
	}
    }

  بائيٽون = ڍير_ALIGN (cif->بائيٽون);
  argp = ڍير =
      (فراغ *)((uintptr_t)alloca(بائيٽون + حجم(*چوکٺو) + rsize + 15) & ~16);
  چوکٺو = (بنية call_چوکٺو *)(ڍير + بائيٽون);
  إذا (rsize)
    rvalue = چوکٺو + 1;

  چوکٺو->fn = fn;
  چوکٺو->flags = flags;
  چوکٺو->rvalue = rvalue;

  narg_reg = 0;
  بدل (flags)
    {
    حالة X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      إذا (pabi->nregs > 0)
	{
	  چوکٺو->regs[pabi->regs[0]] = (بنانشان)rvalue;
	  narg_reg = 1;
	  اكسر;
	}
      /* fallthru */
    حالة X86_RET_STRUCTPOP:
      *(فراغ **)argp = rvalue;
      argp += حجم(فراغ *);
      بائيٽون -= حجم(فراغ *);
      اكسر;
    }

  arg_types = cif->arg_types;
  لكل (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->سائيز;
      صحيح t = ty->type;

      إذا (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  چوکٺو->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      وإلا
	{
	  يادگيري_نقل (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      بائيٽون -= z;
    }
  إذا (i < n)
    يادگيري_نقل (argp, avalue, بائيٽون);

  ffi_call_i386 (چوکٺو, ڍير);
}
#جيڪڏهن_پورو /* !FFI_NO_RAW_API */

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_سائيز, size_t *map_سائيز)
{
  خارجي فراغ *trampoline_ڪوڊ_جدول;

  *map_سائيز = X86_TRAMP_MAP_سائيز;
  *tramp_سائيز = X86_TRAMP_سائيز;
  أرجع &trampoline_ڪوڊ_جدول;
}
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* __i386__ */
