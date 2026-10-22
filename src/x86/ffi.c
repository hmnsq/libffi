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
#  error FFI_TYPE_LONGDOUBLE برآمد کا date
# endif
#ورنہ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#اگر_ختم

#اگر defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#اگر_ختم

#اگر defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#تعریف_کرو ڈھیر_ALIGN(بائٹیں) (بائٹیں)
#ورنہ
#تعریف_کرو ڈھیر_ALIGN(بائٹیں) FFI_ALIGN (بائٹیں, 16)
#اگر_ختم

/* Perform machine dependent cif processing.  */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t بائٹیں = 0;
  صحیح i, n, flags, cabi = cif->abi;

  انتخاب (cabi)
    {
    حالت FFI_SYSV:
    حالت FFI_STDCALL:
    حالت FFI_THISCALL:
    حالت FFI_FASTCALL:
    حالت FFI_MS_CDECL:
    حالت FFI_PASCAL:
    حالت FFI_رجسٹر:
      توڑو;
    طےشدہ:
      واپس FFI_BAD_ABI;
    }

  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      توڑو;
    حالت FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      توڑو;
    حالت FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      توڑو;
    حالت FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      توڑو;
    حالت FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      توڑو;
    حالت FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      توڑو;
    حالت FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      توڑو;
    حالت FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      توڑو;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_اشارہ_گر:
      flags = X86_RET_INT32;
      توڑو;
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      توڑو;
    حالت FFI_TYPE_STRUCT:
      {
#اگر defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t سائز = cif->rtype->سائز;
        اگر (سائز == 1)
          flags = X86_RET_STRUCT_1B;
        ورنہ اگر (سائز == 2)
          flags = X86_RET_STRUCT_2B;
        ورنہ اگر (سائز == 4)
          flags = X86_RET_INT32;
        ورنہ اگر (سائز == 8)
          flags = X86_RET_INT64;
        ورنہ
#اگر_ختم
          {
          do_struct:
            انتخاب (cabi)
              {
              حالت FFI_THISCALL:
              حالت FFI_FASTCALL:
              حالت FFI_STDCALL:
              حالت FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                توڑو;
              طےشدہ:
                flags = X86_RET_STRUCTPOP;
                توڑو;
              }
            /* Allocate space for return value pointer.  */
            بائٹیں += FFI_ALIGN (حجم(خالی*), FFI_SIZEOF_ARG);
          }
      }
      توڑو;
    حالت FFI_TYPE_COMPLEX:
      انتخاب (cif->rtype->elements[0]->type)
	{
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	  جاؤ do_struct;
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  توڑو;
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  توڑو;
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  توڑو;
	طےشدہ:
	  واپس FFI_BAD_TYPEDEF;
	}
      توڑو;
    طےشدہ:
      واپس FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  برائے (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#اگر defined(X86_WIN32)
      اگر (cabi == FFI_STDCALL)
        بائٹیں = FFI_ALIGN (بائٹیں, FFI_SIZEOF_ARG);
      ورنہ
#اگر_ختم
        بائٹیں = FFI_ALIGN (بائٹیں, t->alignment);
      بائٹیں += FFI_ALIGN (t->سائز, FFI_SIZEOF_ARG);
    }
  cif->بائٹیں = بائٹیں;

  واپس FFI_OK;
}

ساکن ffi_arg
extend_basic_type(خالی *arg, صحیح type)
{
  انتخاب (type)
    {
    حالت FFI_TYPE_SINT8:
      واپس *(SINT8 *)arg;
    حالت FFI_TYPE_UINT8:
      واپس *(UINT8 *)arg;
    حالت FFI_TYPE_SINT16:
      واپس *(SINT16 *)arg;
    حالت FFI_TYPE_UINT16:
      واپس *(UINT16 *)arg;

    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_اشارہ_گر:
    حالت FFI_TYPE_FLOAT:
      واپس *(UINT32 *)arg;

    طےشدہ:
      abort();
    }
}

ساخت call_چوکھٹا
{
  خالی *ebp;		/* 0 */
  خالی *retaddr;	/* 4 */
  خالی (*fn)(خالی);	/* 8 */
  صحیح flags;		/* 12 */
  خالی *rvalue;		/* 16 */
  غیرعلامتی regs[3];	/* 20-28 */
};

ساخت abi_params
{
  صحیح dir;		/* parameter growth direction */
  صحیح static_chain;	/* the static chain register used by gcc */
  صحیح nregs;		/* number of register parameters */
  صحیح regs[3];
};

ساکن ثابت ساخت abi_params abi_params[FFI_آخری_ABI] = {
  [FFI_SYSV] = { 1, R_ECX, 0 },
  [FFI_THISCALL] = { 1, R_EAX, 1, { R_ECX } },
  [FFI_FASTCALL] = { 1, R_EAX, 2, { R_ECX, R_EDX } },
  [FFI_STDCALL] = { 1, R_ECX, 0 },
  [FFI_PASCAL] = { -1, R_ECX, 0 },
  /* ??? No defined static chain; gcc does not support REGISTER.  */
  [FFI_رجسٹر] = { -1, R_ECX, 3, { R_EAX, R_EDX, R_ECX } },
  [FFI_MS_CDECL] = { 1, R_ECX, 0 }
};

#اگر_تعریف_شدہ HAVE_FASTCALL
  #اگر_تعریف_شدہ _MSC_VER
    #تعریف_کرو FFI_DECLARE_FASTCALL __fastcall
  #ورنہ
    #تعریف_کرو FFI_DECLARE_FASTCALL __declspec(fastcall)
  #اگر_ختم
#ورنہ
  #تعریف_کرو FFI_DECLARE_FASTCALL
#اگر_ختم

خارجی خالی FFI_DECLARE_FASTCALL ffi_call_i386(ساخت call_چوکھٹا *, حرف *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#اگر defined(_MSC_VER)
#ہدایت runtime_checks("s", off)
#اگر_ختم
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ساکن خالی
ffi_call_int (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	      خالی **avalue, خالی *احاطہ)
{
  size_t rsize, بائٹیں;
  ساخت call_چوکھٹا *چوکھٹا;
  حرف *ڈھیر, *argp;
  ffi_type **arg_types;
  صحیح flags, cabi, i, n, dir, narg_reg;
  ثابت ساخت abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  اگر (rvalue == تہی)
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
	  rsize = cif->rtype->سائز;
	  توڑو;
	طےشدہ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  توڑو;
	}
    }

  بائٹیں = ڈھیر_ALIGN (cif->بائٹیں);
  ڈھیر = alloca(بائٹیں + حجم(*چوکھٹا) + rsize);
  argp = (dir < 0 ? ڈھیر + بائٹیں : ڈھیر);
  چوکھٹا = (ساخت call_چوکھٹا *)(ڈھیر + بائٹیں);
  اگر (rsize)
    rvalue = چوکھٹا + 1;

  چوکھٹا->fn = fn;
  چوکھٹا->flags = flags;
  چوکھٹا->rvalue = rvalue;
  چوکھٹا->regs[pabi->static_chain] = (غیرعلامتی)احاطہ;

  narg_reg = 0;
  انتخاب (flags)
    {
    حالت X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      اگر (pabi->nregs > 0)
	{
	  چوکھٹا->regs[pabi->regs[0]] = (غیرعلامتی)rvalue;
	  narg_reg = 1;
	  توڑو;
	}
      /* fallthru */
    حالت X86_RET_STRUCTPOP:
      *(خالی **)argp = rvalue;
      argp += حجم(خالی *);
      توڑو;
    }

  arg_types = cif->arg_types;
  برائے (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      خالی *valp = avalue[i];
      size_t z = ty->سائز;
      صحیح t = ty->type;

      اگر (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  اگر (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    چوکھٹا->regs[pabi->regs[narg_reg++]] = val;
	  ورنہ اگر (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  ورنہ
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      ورنہ
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
	      حافظہ_نقل (argp, valp, z);
	    }
	  ورنہ
	    {
	      argp = (حرف *)FFI_ALIGN (argp, align);
	      حافظہ_نقل (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == ڈھیر);

  ffi_call_i386 (چوکھٹا, ڈھیر);
}
#اگر defined(_MSC_VER)
#ہدایت runtime_checks("s", restore)
#اگر_ختم

خالی
ffi_call (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, تہی);
}

#اگر_تعریف_شدہ FFI_GO_CLOSURES
خالی
ffi_call_go (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  ffi_call_int (cif, fn, rvalue, avalue, احاطہ);
}
#اگر_ختم

/** private members **/

خالی FFI_HIDDEN ffi_احاطہ_i386(خالی);
خالی FFI_HIDDEN ffi_احاطہ_STDCALL(خالی);
خالی FFI_HIDDEN ffi_احاطہ_رجسٹر(خالی);
#اگر defined(FFI_EXEC_STATIC_TRAMP)
خالی FFI_HIDDEN ffi_احاطہ_i386_alt(خالی);
خالی FFI_HIDDEN ffi_احاطہ_STDCALL_alt(خالی);
خالی FFI_HIDDEN ffi_احاطہ_رجسٹر_alt(خالی);
#اگر_ختم

ساخت احاطہ_چوکھٹا
{
  غیرعلامتی rettemp[4];				/* 0 */
  غیرعلامتی regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*);	/* 32 */
  خالی *user_ڈیٹا;				/* 36 */
};

صحیح FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_احاطہ_inner (ساخت احاطہ_چوکھٹا *چوکھٹا, حرف *ڈھیر)
{
  ffi_cif *cif = چوکھٹا->cif;
  صحیح cabi, i, n, flags, dir, narg_reg;
  ثابت ساخت abi_params *pabi;
  ffi_type **arg_types;
  حرف *argp;
  خالی *rvalue;
  خالی **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = چوکھٹا->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? ڈھیر + ڈھیر_ALIGN (cif->بائٹیں) : ڈھیر);

  انتخاب (flags)
    {
    حالت X86_RET_STRUCTARG:
      اگر (pabi->nregs > 0)
	{
	  rvalue = (خالی *)چوکھٹا->regs[pabi->regs[0]];
	  narg_reg = 1;
	  چوکھٹا->rettemp[0] = (غیرعلامتی)rvalue;
	  توڑو;
	}
      /* fallthru */
    حالت X86_RET_STRUCTPOP:
      rvalue = *(خالی **)argp;
      argp += حجم(خالی *);
      چوکھٹا->rettemp[0] = (غیرعلامتی)rvalue;
      توڑو;
    }

  n = cif->nargs;
  avalue = alloca(حجم(خالی *) * n);

  arg_types = cif->arg_types;
  برائے (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->سائز;
      صحیح t = ty->type;
      خالی *valp;

      اگر (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  اگر (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &چوکھٹا->regs[pabi->regs[narg_reg++]];
	  ورنہ اگر (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  ورنہ
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      ورنہ
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
	  ورنہ
	    {
	      argp = (حرف *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  چوکھٹا->fun (cif, rvalue, avalue, چوکھٹا->user_ڈیٹا);

  انتخاب (cabi)
    {
    حالت FFI_STDCALL:
      واپس flags | (cif->بائٹیں << X86_RET_نکالو_سرکاؤ);
    حالت FFI_THISCALL:
    حالت FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      واپس flags | (((غیرعلامتی) (argp - ڈھیر)) << X86_RET_نکالو_سرکاؤ);
    طےشدہ:
      واپس flags;
    }
}

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
                      ffi_cif* cif,
                      خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
                      خالی *user_ڈیٹا,
                      خالی *codeloc)
{
  حرف *tramp = احاطہ->tramp;
  خالی (*dest)(خالی);
  صحیح op = 0xb8;  /* movl imm, %eax */

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
    حالت FFI_MS_CDECL:
      dest = ffi_احاطہ_i386;
      توڑو;
    حالت FFI_STDCALL:
    حالت FFI_THISCALL:
    حالت FFI_FASTCALL:
    حالت FFI_PASCAL:
      dest = ffi_احاطہ_STDCALL;
      توڑو;
    حالت FFI_رجسٹر:
      dest = ffi_احاطہ_رجسٹر;
      op = 0x68;  /* pushl imm */
      توڑو;
    طےشدہ:
      واپس FFI_BAD_ABI;
    }

#اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(احاطہ))
    {
      /* Initialize the static trampoline's parameters. */
      اگر (dest == ffi_احاطہ_i386)
        dest = ffi_احاطہ_i386_alt;
      ورنہ اگر (dest == ffi_احاطہ_STDCALL)
        dest = ffi_احاطہ_STDCALL_alt;
      ورنہ
        dest = ffi_احاطہ_رجسٹر_alt;
      ffi_tramp_سیٹ_parms (احاطہ->ftramp, dest, احاطہ);
      جاؤ برآمد;
    }
#اگر_ختم

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(خالی **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(غیرعلامتی *)(tramp + 10) = (غیرعلامتی)dest - ((غیرعلامتی)codeloc + 14);

#اگر defined(FFI_EXEC_STATIC_TRAMP)
برآمد:
#اگر_ختم
  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  واپس FFI_OK;
}

#اگر_تعریف_شدہ FFI_GO_CLOSURES

خالی FFI_HIDDEN ffi_go_احاطہ_EAX(خالی);
خالی FFI_HIDDEN ffi_go_احاطہ_ECX(خالی);
خالی FFI_HIDDEN ffi_go_احاطہ_STDCALL(خالی);

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ* احاطہ, ffi_cif* cif,
		     خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*))
{
  خالی (*dest)(خالی);

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
    حالت FFI_MS_CDECL:
      dest = ffi_go_احاطہ_ECX;
      توڑو;
    حالت FFI_THISCALL:
    حالت FFI_FASTCALL:
      dest = ffi_go_احاطہ_EAX;
      توڑو;
    حالت FFI_STDCALL:
    حالت FFI_PASCAL:
      dest = ffi_go_احاطہ_STDCALL;
      توڑو;
    حالت FFI_رجسٹر:
    طےشدہ:
      واپس FFI_BAD_ABI;
    }

  احاطہ->tramp = dest;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}

#اگر_ختم /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#اگر !FFI_NO_RAW_API

خالی FFI_HIDDEN ffi_احاطہ_raw_SYSV(خالی);
خالی FFI_HIDDEN ffi_احاطہ_raw_THISCALL(خالی);

ffi_حالت
ffi_prep_raw_احاطہ_loc (ffi_raw_احاطہ *احاطہ,
                          ffi_cif *cif,
                          خالی (*fun)(ffi_cif*,خالی*,ffi_raw*,خالی*),
                          خالی *user_ڈیٹا,
                          خالی *codeloc)
{
  حرف *tramp = احاطہ->tramp;
  خالی (*dest)(خالی);
  صحیح i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  برائے (i = cif->nargs-1; i >= 0; i--)
    انتخاب (cif->arg_types[i]->type)
      {
      حالت FFI_TYPE_STRUCT:
      حالت FFI_TYPE_LONGDOUBLE:
	واپس FFI_BAD_TYPEDEF;
      }

  انتخاب (cif->abi)
    {
    حالت FFI_THISCALL:
      dest = ffi_احاطہ_raw_THISCALL;
      توڑو;
    حالت FFI_SYSV:
      dest = ffi_احاطہ_raw_SYSV;
      توڑو;
    طےشدہ:
      واپس FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(خالی **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(غیرعلامتی *)(tramp + 6) = (غیرعلامتی)dest - ((غیرعلامتی)codeloc + 10);

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  واپس FFI_OK;
}

خالی
ffi_raw_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, ffi_raw *avalue)
{
  size_t rsize, بائٹیں;
  ساخت call_چوکھٹا *چوکھٹا;
  حرف *ڈھیر, *argp;
  ffi_type **arg_types;
  صحیح flags, cabi, i, n, narg_reg;
  ثابت ساخت abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  اگر (rvalue == تہی)
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
	  rsize = cif->rtype->سائز;
	  توڑو;
	طےشدہ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  توڑو;
	}
    }

  بائٹیں = ڈھیر_ALIGN (cif->بائٹیں);
  argp = ڈھیر =
      (خالی *)((uintptr_t)alloca(بائٹیں + حجم(*چوکھٹا) + rsize + 15) & ~16);
  چوکھٹا = (ساخت call_چوکھٹا *)(ڈھیر + بائٹیں);
  اگر (rsize)
    rvalue = چوکھٹا + 1;

  چوکھٹا->fn = fn;
  چوکھٹا->flags = flags;
  چوکھٹا->rvalue = rvalue;

  narg_reg = 0;
  انتخاب (flags)
    {
    حالت X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      اگر (pabi->nregs > 0)
	{
	  چوکھٹا->regs[pabi->regs[0]] = (غیرعلامتی)rvalue;
	  narg_reg = 1;
	  توڑو;
	}
      /* fallthru */
    حالت X86_RET_STRUCTPOP:
      *(خالی **)argp = rvalue;
      argp += حجم(خالی *);
      بائٹیں -= حجم(خالی *);
      توڑو;
    }

  arg_types = cif->arg_types;
  برائے (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->سائز;
      صحیح t = ty->type;

      اگر (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  چوکھٹا->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      ورنہ
	{
	  حافظہ_نقل (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      بائٹیں -= z;
    }
  اگر (i < n)
    حافظہ_نقل (argp, avalue, بائٹیں);

  ffi_call_i386 (چوکھٹا, ڈھیر);
}
#اگر_ختم /* !FFI_NO_RAW_API */

#اگر defined(FFI_EXEC_STATIC_TRAMP)
خالی *
ffi_tramp_arch (size_t *tramp_سائز, size_t *map_سائز)
{
  خارجی خالی *trampoline_رمز_جدول;

  *map_سائز = X86_TRAMP_MAP_سائز;
  *tramp_سائز = X86_TRAMP_سائز;
  واپس &trampoline_رمز_جدول;
}
#اگر_ختم

#اگر_ختم /* __i386__ */
