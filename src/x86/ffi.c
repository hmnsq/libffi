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
#  error FFI_TYPE_LONGDOUBLE بیرون از date
# endif
#وإلا
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#پایان_اگر

#إذا defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#پایان_اگر

#إذا defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#تعریف_کن پشته_ALIGN(بایتها) (بایتها)
#وإلا
#تعریف_کن پشته_ALIGN(بایتها) FFI_ALIGN (بایتها, 16)
#پایان_اگر

/* Perform machine dependent cif processing.  */
ffi_وضعیت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t بایتها = 0;
  صحيح i, n, flags, cabi = cif->abi;

  بدل (cabi)
    {
    حالة FFI_SYSV:
    حالة FFI_STDCALL:
    حالة FFI_THISCALL:
    حالة FFI_FASTCALL:
    حالة FFI_MS_CDECL:
    حالة FFI_PASCAL:
    حالة FFI_ثبات:
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
    حالة FFI_TYPE_اشاره_گر:
      flags = X86_RET_INT32;
      اكسر;
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      اكسر;
    حالة FFI_TYPE_STRUCT:
      {
#إذا defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t اندازه = cif->rtype->اندازه;
        إذا (اندازه == 1)
          flags = X86_RET_STRUCT_1B;
        وإلا إذا (اندازه == 2)
          flags = X86_RET_STRUCT_2B;
        وإلا إذا (اندازه == 4)
          flags = X86_RET_INT32;
        وإلا إذا (اندازه == 8)
          flags = X86_RET_INT64;
        وإلا
#پایان_اگر
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
            بایتها += FFI_ALIGN (حجم(فراغ*), FFI_SIZEOF_ARG);
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
        بایتها = FFI_ALIGN (بایتها, FFI_SIZEOF_ARG);
      وإلا
#پایان_اگر
        بایتها = FFI_ALIGN (بایتها, t->alignment);
      بایتها += FFI_ALIGN (t->اندازه, FFI_SIZEOF_ARG);
    }
  cif->بایتها = بایتها;

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
    حالة FFI_TYPE_اشاره_گر:
    حالة FFI_TYPE_FLOAT:
      أرجع *(UINT32 *)arg;

    افتراضي:
      abort();
    }
}

بنية call_قاب
{
  فراغ *ebp;		/* 0 */
  فراغ *retaddr;	/* 4 */
  فراغ (*fn)(فراغ);	/* 8 */
  صحيح flags;		/* 12 */
  فراغ *rvalue;		/* 16 */
  بیعلامت regs[3];	/* 20-28 */
};

بنية abi_params
{
  صحيح dir;		/* parameter growth direction */
  صحيح static_chain;	/* the static chain register used by gcc */
  صحيح nregs;		/* number of register parameters */
  صحيح regs[3];
};

ساكن ثابت بنية abi_params abi_params[FFI_آخرین_ABI] = {
  [FFI_SYSV] = { 1, R_ECX, 0 },
  [FFI_THISCALL] = { 1, R_EAX, 1, { R_ECX } },
  [FFI_FASTCALL] = { 1, R_EAX, 2, { R_ECX, R_EDX } },
  [FFI_STDCALL] = { 1, R_ECX, 0 },
  [FFI_PASCAL] = { -1, R_ECX, 0 },
  /* ??? No defined static chain; gcc does not support REGISTER.  */
  [FFI_ثبات] = { -1, R_ECX, 3, { R_EAX, R_EDX, R_ECX } },
  [FFI_MS_CDECL] = { 1, R_ECX, 0 }
};

#اگر_تعریف_شده HAVE_FASTCALL
  #اگر_تعریف_شده _MSC_VER
    #تعریف_کن FFI_DECLARE_FASTCALL __fastcall
  #وإلا
    #تعریف_کن FFI_DECLARE_FASTCALL __declspec(fastcall)
  #پایان_اگر
#وإلا
  #تعریف_کن FFI_DECLARE_FASTCALL
#پایان_اگر

خارجي فراغ FFI_DECLARE_FASTCALL ffi_call_i386(بنية call_قاب *, حرف *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#إذا defined(_MSC_VER)
#رهنمود runtime_checks("s", off)
#پایان_اگر
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	      فراغ **avalue, فراغ *بستگی)
{
  size_t rsize, بایتها;
  بنية call_قاب *قاب;
  حرف *پشته, *argp;
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
	  rsize = cif->rtype->اندازه;
	  اكسر;
	افتراضي:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  اكسر;
	}
    }

  بایتها = پشته_ALIGN (cif->بایتها);
  پشته = alloca(بایتها + حجم(*قاب) + rsize);
  argp = (dir < 0 ? پشته + بایتها : پشته);
  قاب = (بنية call_قاب *)(پشته + بایتها);
  إذا (rsize)
    rvalue = قاب + 1;

  قاب->fn = fn;
  قاب->flags = flags;
  قاب->rvalue = rvalue;
  قاب->regs[pabi->static_chain] = (بیعلامت)بستگی;

  narg_reg = 0;
  بدل (flags)
    {
    حالة X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      إذا (pabi->nregs > 0)
	{
	  قاب->regs[pabi->regs[0]] = (بیعلامت)rvalue;
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
      size_t z = ty->اندازه;
      صحيح t = ty->type;

      إذا (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  إذا (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    قاب->regs[pabi->regs[narg_reg++]] = val;
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
	      کپی_حافظه (argp, valp, z);
	    }
	  وإلا
	    {
	      argp = (حرف *)FFI_ALIGN (argp, align);
	      کپی_حافظه (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == پشته);

  ffi_call_i386 (قاب, پشته);
}
#إذا defined(_MSC_VER)
#رهنمود runtime_checks("s", restore)
#پایان_اگر

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

#اگر_تعریف_شده FFI_GO_CLOSURES
فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *بستگی)
{
  ffi_call_int (cif, fn, rvalue, avalue, بستگی);
}
#پایان_اگر

/** private members **/

فراغ FFI_HIDDEN ffi_بستگی_i386(فراغ);
فراغ FFI_HIDDEN ffi_بستگی_STDCALL(فراغ);
فراغ FFI_HIDDEN ffi_بستگی_ثبات(فراغ);
#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ FFI_HIDDEN ffi_بستگی_i386_alt(فراغ);
فراغ FFI_HIDDEN ffi_بستگی_STDCALL_alt(فراغ);
فراغ FFI_HIDDEN ffi_بستگی_ثبات_alt(فراغ);
#پایان_اگر

بنية بستگی_قاب
{
  بیعلامت rettemp[4];				/* 0 */
  بیعلامت regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*);	/* 32 */
  فراغ *user_داده;				/* 36 */
};

صحيح FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_بستگی_inner (بنية بستگی_قاب *قاب, حرف *پشته)
{
  ffi_cif *cif = قاب->cif;
  صحيح cabi, i, n, flags, dir, narg_reg;
  ثابت بنية abi_params *pabi;
  ffi_type **arg_types;
  حرف *argp;
  فراغ *rvalue;
  فراغ **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = قاب->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? پشته + پشته_ALIGN (cif->بایتها) : پشته);

  بدل (flags)
    {
    حالة X86_RET_STRUCTARG:
      إذا (pabi->nregs > 0)
	{
	  rvalue = (فراغ *)قاب->regs[pabi->regs[0]];
	  narg_reg = 1;
	  قاب->rettemp[0] = (بیعلامت)rvalue;
	  اكسر;
	}
      /* fallthru */
    حالة X86_RET_STRUCTPOP:
      rvalue = *(فراغ **)argp;
      argp += حجم(فراغ *);
      قاب->rettemp[0] = (بیعلامت)rvalue;
      اكسر;
    }

  n = cif->nargs;
  avalue = alloca(حجم(فراغ *) * n);

  arg_types = cif->arg_types;
  لكل (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->اندازه;
      صحيح t = ty->type;
      فراغ *valp;

      إذا (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  إذا (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &قاب->regs[pabi->regs[narg_reg++]];
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

  قاب->fun (cif, rvalue, avalue, قاب->user_داده);

  بدل (cabi)
    {
    حالة FFI_STDCALL:
      أرجع flags | (cif->بایتها << X86_RET_برداشت_لغزش);
    حالة FFI_THISCALL:
    حالة FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      أرجع flags | (((بیعلامت) (argp - پشته)) << X86_RET_برداشت_لغزش);
    افتراضي:
      أرجع flags;
    }
}

ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی* بستگی,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_داده,
                      فراغ *codeloc)
{
  حرف *tramp = بستگی->tramp;
  فراغ (*dest)(فراغ);
  صحيح op = 0xb8;  /* movl imm, %eax */

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
    حالة FFI_MS_CDECL:
      dest = ffi_بستگی_i386;
      اكسر;
    حالة FFI_STDCALL:
    حالة FFI_THISCALL:
    حالة FFI_FASTCALL:
    حالة FFI_PASCAL:
      dest = ffi_بستگی_STDCALL;
      اكسر;
    حالة FFI_ثبات:
      dest = ffi_بستگی_ثبات;
      op = 0x68;  /* pushl imm */
      اكسر;
    افتراضي:
      أرجع FFI_BAD_ABI;
    }

#إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(بستگی))
    {
      /* Initialize the static trampoline's parameters. */
      إذا (dest == ffi_بستگی_i386)
        dest = ffi_بستگی_i386_alt;
      وإلا إذا (dest == ffi_بستگی_STDCALL)
        dest = ffi_بستگی_STDCALL_alt;
      وإلا
        dest = ffi_بستگی_ثبات_alt;
      ffi_tramp_مجموعه_parms (بستگی->ftramp, dest, بستگی);
      اذهب بیرون;
    }
#پایان_اگر

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(فراغ **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(بیعلامت *)(tramp + 10) = (بیعلامت)dest - ((بیعلامت)codeloc + 14);

#إذا defined(FFI_EXEC_STATIC_TRAMP)
بیرون:
#پایان_اگر
  بستگی->cif = cif;
  بستگی->fun = fun;
  بستگی->user_داده = user_داده;

  أرجع FFI_OK;
}

#اگر_تعریف_شده FFI_GO_CLOSURES

فراغ FFI_HIDDEN ffi_go_بستگی_EAX(فراغ);
فراغ FFI_HIDDEN ffi_go_بستگی_ECX(فراغ);
فراغ FFI_HIDDEN ffi_go_بستگی_STDCALL(فراغ);

ffi_وضعیت
ffi_prep_go_بستگی (ffi_go_بستگی* بستگی, ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*))
{
  فراغ (*dest)(فراغ);

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
    حالة FFI_MS_CDECL:
      dest = ffi_go_بستگی_ECX;
      اكسر;
    حالة FFI_THISCALL:
    حالة FFI_FASTCALL:
      dest = ffi_go_بستگی_EAX;
      اكسر;
    حالة FFI_STDCALL:
    حالة FFI_PASCAL:
      dest = ffi_go_بستگی_STDCALL;
      اكسر;
    حالة FFI_ثبات:
    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  بستگی->tramp = dest;
  بستگی->cif = cif;
  بستگی->fun = fun;

  أرجع FFI_OK;
}

#پایان_اگر /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#إذا !FFI_NO_RAW_API

فراغ FFI_HIDDEN ffi_بستگی_raw_SYSV(فراغ);
فراغ FFI_HIDDEN ffi_بستگی_raw_THISCALL(فراغ);

ffi_وضعیت
ffi_prep_raw_بستگی_loc (ffi_raw_بستگی *بستگی,
                          ffi_cif *cif,
                          فراغ (*fun)(ffi_cif*,فراغ*,ffi_raw*,فراغ*),
                          فراغ *user_داده,
                          فراغ *codeloc)
{
  حرف *tramp = بستگی->tramp;
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
      dest = ffi_بستگی_raw_THISCALL;
      اكسر;
    حالة FFI_SYSV:
      dest = ffi_بستگی_raw_SYSV;
      اكسر;
    افتراضي:
      أرجع FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(فراغ **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(بیعلامت *)(tramp + 6) = (بیعلامت)dest - ((بیعلامت)codeloc + 10);

  بستگی->cif = cif;
  بستگی->fun = fun;
  بستگی->user_داده = user_داده;

  أرجع FFI_OK;
}

فراغ
ffi_raw_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, ffi_raw *avalue)
{
  size_t rsize, بایتها;
  بنية call_قاب *قاب;
  حرف *پشته, *argp;
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
	  rsize = cif->rtype->اندازه;
	  اكسر;
	افتراضي:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  اكسر;
	}
    }

  بایتها = پشته_ALIGN (cif->بایتها);
  argp = پشته =
      (فراغ *)((uintptr_t)alloca(بایتها + حجم(*قاب) + rsize + 15) & ~16);
  قاب = (بنية call_قاب *)(پشته + بایتها);
  إذا (rsize)
    rvalue = قاب + 1;

  قاب->fn = fn;
  قاب->flags = flags;
  قاب->rvalue = rvalue;

  narg_reg = 0;
  بدل (flags)
    {
    حالة X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      إذا (pabi->nregs > 0)
	{
	  قاب->regs[pabi->regs[0]] = (بیعلامت)rvalue;
	  narg_reg = 1;
	  اكسر;
	}
      /* fallthru */
    حالة X86_RET_STRUCTPOP:
      *(فراغ **)argp = rvalue;
      argp += حجم(فراغ *);
      بایتها -= حجم(فراغ *);
      اكسر;
    }

  arg_types = cif->arg_types;
  لكل (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->اندازه;
      صحيح t = ty->type;

      إذا (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  قاب->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      وإلا
	{
	  کپی_حافظه (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      بایتها -= z;
    }
  إذا (i < n)
    کپی_حافظه (argp, avalue, بایتها);

  ffi_call_i386 (قاب, پشته);
}
#پایان_اگر /* !FFI_NO_RAW_API */

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_اندازه, size_t *map_اندازه)
{
  خارجي فراغ *trampoline_کد_جدول;

  *map_اندازه = X86_TRAMP_MAP_اندازه;
  *tramp_اندازه = X86_TRAMP_اندازه;
  أرجع &trampoline_کد_جدول;
}
#پایان_اگر

#پایان_اگر /* __i386__ */
