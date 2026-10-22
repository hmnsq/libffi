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

#ఒకవేళ defined(__i386__) || defined(_M_IX86)
#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <tramp.h>
#అన్తర్భావ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ఒకవేళ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE బహిర్గతం యొక్క date
# endif
#లేకపోతే
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#అయితే_ముగింపు

#ఒకవేళ defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#అయితే_ముగింపు

#ఒకవేళ defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#నిర్వచించు దొంతర_ALIGN(బైట్లు) (బైట్లు)
#లేకపోతే
#నిర్వచించు దొంతర_ALIGN(బైట్లు) FFI_ALIGN (బైట్లు, 16)
#అయితే_ముగింపు

/* Perform machine dependent cif processing.  */
ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t బైట్లు = 0;
  పూర్ణాంకం i, n, flags, cabi = cif->abi;

  ఎంపిక (cabi)
    {
    స్థితి FFI_SYSV:
    స్థితి FFI_STDCALL:
    స్థితి FFI_THISCALL:
    స్థితి FFI_FASTCALL:
    స్థితి FFI_MS_CDECL:
    స్థితి FFI_PASCAL:
    స్థితి FFI_రిజిస్టరు:
      ఆపు;
    అప్రమేయ:
      తిరిగి FFI_BAD_ABI;
    }

  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      ఆపు;
    స్థితి FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      ఆపు;
    స్థితి FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      ఆపు;
    స్థితి FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      ఆపు;
    స్థితి FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      ఆపు;
    స్థితి FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      ఆపు;
    స్థితి FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      ఆపు;
    స్థితి FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      ఆపు;
    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_UINT32:
    స్థితి FFI_TYPE_సూచిక:
      flags = X86_RET_INT32;
      ఆపు;
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      ఆపు;
    స్థితి FFI_TYPE_STRUCT:
      {
#ఒకవేళ defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        size_t కొలత = cif->rtype->కొలత;
        ఒకవేళ (కొలత == 1)
          flags = X86_RET_STRUCT_1B;
        లేకపోతే ఒకవేళ (కొలత == 2)
          flags = X86_RET_STRUCT_2B;
        లేకపోతే ఒకవేళ (కొలత == 4)
          flags = X86_RET_INT32;
        లేకపోతే ఒకవేళ (కొలత == 8)
          flags = X86_RET_INT64;
        లేకపోతే
#అయితే_ముగింపు
          {
          do_struct:
            ఎంపిక (cabi)
              {
              స్థితి FFI_THISCALL:
              స్థితి FFI_FASTCALL:
              స్థితి FFI_STDCALL:
              స్థితి FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                ఆపు;
              అప్రమేయ:
                flags = X86_RET_STRUCTPOP;
                ఆపు;
              }
            /* Allocate space for return value pointer.  */
            బైట్లు += FFI_ALIGN (పరిమాణం(శూన్య*), FFI_SIZEOF_ARG);
          }
      }
      ఆపు;
    స్థితి FFI_TYPE_COMPLEX:
      ఎంపిక (cif->rtype->elements[0]->type)
	{
	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_LONGDOUBLE:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	  వెళ్ళు do_struct;
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  ఆపు;
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  ఆపు;
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  ఆపు;
	అప్రమేయ:
	  తిరిగి FFI_BAD_TYPEDEF;
	}
      ఆపు;
    అప్రమేయ:
      తిరిగి FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  కొరకు (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#ఒకవేళ defined(X86_WIN32)
      ఒకవేళ (cabi == FFI_STDCALL)
        బైట్లు = FFI_ALIGN (బైట్లు, FFI_SIZEOF_ARG);
      లేకపోతే
#అయితే_ముగింపు
        బైట్లు = FFI_ALIGN (బైట్లు, t->alignment);
      బైట్లు += FFI_ALIGN (t->కొలత, FFI_SIZEOF_ARG);
    }
  cif->బైట్లు = బైట్లు;

  తిరిగి FFI_OK;
}

స్థిర ffi_arg
extend_basic_type(శూన్య *arg, పూర్ణాంకం type)
{
  ఎంపిక (type)
    {
    స్థితి FFI_TYPE_SINT8:
      తిరిగి *(SINT8 *)arg;
    స్థితి FFI_TYPE_UINT8:
      తిరిగి *(UINT8 *)arg;
    స్థితి FFI_TYPE_SINT16:
      తిరిగి *(SINT16 *)arg;
    స్థితి FFI_TYPE_UINT16:
      తిరిగి *(UINT16 *)arg;

    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_UINT32:
    స్థితి FFI_TYPE_సూచిక:
    స్థితి FFI_TYPE_FLOAT:
      తిరిగి *(UINT32 *)arg;

    అప్రమేయ:
      abort();
    }
}

నిర్మాణం call_చట్రం
{
  శూన్య *ebp;		/* 0 */
  శూన్య *retaddr;	/* 4 */
  శూన్య (*fn)(శూన్య);	/* 8 */
  పూర్ణాంకం flags;		/* 12 */
  శూన్య *rvalue;		/* 16 */
  గుర్తులేని regs[3];	/* 20-28 */
};

నిర్మాణం abi_params
{
  పూర్ణాంకం dir;		/* parameter growth direction */
  పూర్ణాంకం static_chain;	/* the static chain register used by gcc */
  పూర్ణాంకం nregs;		/* number of register parameters */
  పూర్ణాంకం regs[3];
};

స్థిర స్థిరరాశి నిర్మాణం abi_params abi_params[FFI_చివరి_ABI] = {
  [FFI_SYSV] = { 1, R_ECX, 0 },
  [FFI_THISCALL] = { 1, R_EAX, 1, { R_ECX } },
  [FFI_FASTCALL] = { 1, R_EAX, 2, { R_ECX, R_EDX } },
  [FFI_STDCALL] = { 1, R_ECX, 0 },
  [FFI_PASCAL] = { -1, R_ECX, 0 },
  /* ??? No defined static chain; gcc does not support REGISTER.  */
  [FFI_రిజిస్టరు] = { -1, R_ECX, 3, { R_EAX, R_EDX, R_ECX } },
  [FFI_MS_CDECL] = { 1, R_ECX, 0 }
};

#నిర్వచితమైతే HAVE_FASTCALL
  #నిర్వచితమైతే _MSC_VER
    #నిర్వచించు FFI_DECLARE_FASTCALL __fastcall
  #లేకపోతే
    #నిర్వచించు FFI_DECLARE_FASTCALL __declspec(fastcall)
  #అయితే_ముగింపు
#లేకపోతే
  #నిర్వచించు FFI_DECLARE_FASTCALL
#అయితే_ముగింపు

బాహ్య శూన్య FFI_DECLARE_FASTCALL ffi_call_i386(నిర్మాణం call_చట్రం *, అక్షరం *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#ఒకవేళ defined(_MSC_VER)
#సూచన runtime_checks("s", off)
#అయితే_ముగింపు
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
స్థిర శూన్య
ffi_call_int (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	      శూన్య **avalue, శూన్య *సంవృతం)
{
  size_t rsize, బైట్లు;
  నిర్మాణం call_చట్రం *చట్రం;
  అక్షరం *దొంతర, *argp;
  ffi_type **arg_types;
  పూర్ణాంకం flags, cabi, i, n, dir, narg_reg;
  స్థిరరాశి నిర్మాణం abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  ఒకవేళ (rvalue == ఖాళీ)
    {
      ఎంపిక (flags)
	{
	స్థితి X86_RET_FLOAT:
	స్థితి X86_RET_DOUBLE:
	స్థితి X86_RET_LDOUBLE:
	స్థితి X86_RET_STRUCTPOP:
	స్థితి X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->కొలత;
	  ఆపు;
	అప్రమేయ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  ఆపు;
	}
    }

  బైట్లు = దొంతర_ALIGN (cif->బైట్లు);
  దొంతర = alloca(బైట్లు + పరిమాణం(*చట్రం) + rsize);
  argp = (dir < 0 ? దొంతర + బైట్లు : దొంతర);
  చట్రం = (నిర్మాణం call_చట్రం *)(దొంతర + బైట్లు);
  ఒకవేళ (rsize)
    rvalue = చట్రం + 1;

  చట్రం->fn = fn;
  చట్రం->flags = flags;
  చట్రం->rvalue = rvalue;
  చట్రం->regs[pabi->static_chain] = (గుర్తులేని)సంవృతం;

  narg_reg = 0;
  ఎంపిక (flags)
    {
    స్థితి X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ఒకవేళ (pabi->nregs > 0)
	{
	  చట్రం->regs[pabi->regs[0]] = (గుర్తులేని)rvalue;
	  narg_reg = 1;
	  ఆపు;
	}
      /* fallthru */
    స్థితి X86_RET_STRUCTPOP:
      *(శూన్య **)argp = rvalue;
      argp += పరిమాణం(శూన్య *);
      ఆపు;
    }

  arg_types = cif->arg_types;
  కొరకు (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      శూన్య *valp = avalue[i];
      size_t z = ty->కొలత;
      పూర్ణాంకం t = ty->type;

      ఒకవేళ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  ఒకవేళ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    చట్రం->regs[pabi->regs[narg_reg++]] = val;
	  లేకపోతే ఒకవేళ (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  లేకపోతే
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      లేకపోతే
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ఒకవేళ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  ఒకవేళ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  ఒకవేళ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      జ్ఞాపకం_నకలు (argp, valp, z);
	    }
	  లేకపోతే
	    {
	      argp = (అక్షరం *)FFI_ALIGN (argp, align);
	      జ్ఞాపకం_నకలు (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == దొంతర);

  ffi_call_i386 (చట్రం, దొంతర);
}
#ఒకవేళ defined(_MSC_VER)
#సూచన runtime_checks("s", restore)
#అయితే_ముగింపు

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ఖాళీ);
}

#నిర్వచితమైతే FFI_GO_CLOSURES
శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
  ffi_call_int (cif, fn, rvalue, avalue, సంవృతం);
}
#అయితే_ముగింపు

/** private members **/

శూన్య FFI_HIDDEN ffi_సంవృతం_i386(శూన్య);
శూన్య FFI_HIDDEN ffi_సంవృతం_STDCALL(శూన్య);
శూన్య FFI_HIDDEN ffi_సంవృతం_రిజిస్టరు(శూన్య);
#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
శూన్య FFI_HIDDEN ffi_సంవృతం_i386_alt(శూన్య);
శూన్య FFI_HIDDEN ffi_సంవృతం_STDCALL_alt(శూన్య);
శూన్య FFI_HIDDEN ffi_సంవృతం_రిజిస్టరు_alt(శూన్య);
#అయితే_ముగింపు

నిర్మాణం సంవృతం_చట్రం
{
  గుర్తులేని rettemp[4];				/* 0 */
  గుర్తులేని regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*);	/* 32 */
  శూన్య *user_దత్తాంశం;				/* 36 */
};

పూర్ణాంకం FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_సంవృతం_inner (నిర్మాణం సంవృతం_చట్రం *చట్రం, అక్షరం *దొంతర)
{
  ffi_cif *cif = చట్రం->cif;
  పూర్ణాంకం cabi, i, n, flags, dir, narg_reg;
  స్థిరరాశి నిర్మాణం abi_params *pabi;
  ffi_type **arg_types;
  అక్షరం *argp;
  శూన్య *rvalue;
  శూన్య **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = చట్రం->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? దొంతర + దొంతర_ALIGN (cif->బైట్లు) : దొంతర);

  ఎంపిక (flags)
    {
    స్థితి X86_RET_STRUCTARG:
      ఒకవేళ (pabi->nregs > 0)
	{
	  rvalue = (శూన్య *)చట్రం->regs[pabi->regs[0]];
	  narg_reg = 1;
	  చట్రం->rettemp[0] = (గుర్తులేని)rvalue;
	  ఆపు;
	}
      /* fallthru */
    స్థితి X86_RET_STRUCTPOP:
      rvalue = *(శూన్య **)argp;
      argp += పరిమాణం(శూన్య *);
      చట్రం->rettemp[0] = (గుర్తులేని)rvalue;
      ఆపు;
    }

  n = cif->nargs;
  avalue = alloca(పరిమాణం(శూన్య *) * n);

  arg_types = cif->arg_types;
  కొరకు (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->కొలత;
      పూర్ణాంకం t = ty->type;
      శూన్య *valp;

      ఒకవేళ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  ఒకవేళ (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &చట్రం->regs[pabi->regs[narg_reg++]];
	  లేకపోతే ఒకవేళ (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  లేకపోతే
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      లేకపోతే
	{
	  size_t za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  size_t align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  ఒకవేళ (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  ఒకవేళ ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  ఒకవేళ (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  లేకపోతే
	    {
	      argp = (అక్షరం *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  చట్రం->fun (cif, rvalue, avalue, చట్రం->user_దత్తాంశం);

  ఎంపిక (cabi)
    {
    స్థితి FFI_STDCALL:
      తిరిగి flags | (cif->బైట్లు << X86_RET_తీయి_జరుపు);
    స్థితి FFI_THISCALL:
    స్థితి FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      తిరిగి flags | (((గుర్తులేని) (argp - దొంతర)) << X86_RET_తీయి_జరుపు);
    అప్రమేయ:
      తిరిగి flags;
    }
}

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
                      ffi_cif* cif,
                      శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
                      శూన్య *user_దత్తాంశం,
                      శూన్య *codeloc)
{
  అక్షరం *tramp = సంవృతం->tramp;
  శూన్య (*dest)(శూన్య);
  పూర్ణాంకం op = 0xb8;  /* movl imm, %eax */

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_SYSV:
    స్థితి FFI_MS_CDECL:
      dest = ffi_సంవృతం_i386;
      ఆపు;
    స్థితి FFI_STDCALL:
    స్థితి FFI_THISCALL:
    స్థితి FFI_FASTCALL:
    స్థితి FFI_PASCAL:
      dest = ffi_సంవృతం_STDCALL;
      ఆపు;
    స్థితి FFI_రిజిస్టరు:
      dest = ffi_సంవృతం_రిజిస్టరు;
      op = 0x68;  /* pushl imm */
      ఆపు;
    అప్రమేయ:
      తిరిగి FFI_BAD_ABI;
    }

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
  ఒకవేళ (ffi_tramp_is_present(సంవృతం))
    {
      /* Initialize the static trampoline's parameters. */
      ఒకవేళ (dest == ffi_సంవృతం_i386)
        dest = ffi_సంవృతం_i386_alt;
      లేకపోతే ఒకవేళ (dest == ffi_సంవృతం_STDCALL)
        dest = ffi_సంవృతం_STDCALL_alt;
      లేకపోతే
        dest = ffi_సంవృతం_రిజిస్టరు_alt;
      ffi_tramp_సమితి_parms (సంవృతం->ftramp, dest, సంవృతం);
      వెళ్ళు బహిర్గతం;
    }
#అయితే_ముగింపు

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(శూన్య **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(గుర్తులేని *)(tramp + 10) = (గుర్తులేని)dest - ((గుర్తులేని)codeloc + 14);

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
బహిర్గతం:
#అయితే_ముగింపు
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}

#నిర్వచితమైతే FFI_GO_CLOSURES

శూన్య FFI_HIDDEN ffi_go_సంవృతం_EAX(శూన్య);
శూన్య FFI_HIDDEN ffi_go_సంవృతం_ECX(శూన్య);
శూన్య FFI_HIDDEN ffi_go_సంవృతం_STDCALL(శూన్య);

ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం* సంవృతం, ffi_cif* cif,
		     శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*))
{
  శూన్య (*dest)(శూన్య);

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_SYSV:
    స్థితి FFI_MS_CDECL:
      dest = ffi_go_సంవృతం_ECX;
      ఆపు;
    స్థితి FFI_THISCALL:
    స్థితి FFI_FASTCALL:
      dest = ffi_go_సంవృతం_EAX;
      ఆపు;
    స్థితి FFI_STDCALL:
    స్థితి FFI_PASCAL:
      dest = ffi_go_సంవృతం_STDCALL;
      ఆపు;
    స్థితి FFI_రిజిస్టరు:
    అప్రమేయ:
      తిరిగి FFI_BAD_ABI;
    }

  సంవృతం->tramp = dest;
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}

#అయితే_ముగింపు /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#ఒకవేళ !FFI_NO_RAW_API

శూన్య FFI_HIDDEN ffi_సంవృతం_raw_SYSV(శూన్య);
శూన్య FFI_HIDDEN ffi_సంవృతం_raw_THISCALL(శూన్య);

ffi_స్థితి
ffi_prep_raw_సంవృతం_loc (ffi_raw_సంవృతం *సంవృతం,
                          ffi_cif *cif,
                          శూన్య (*fun)(ffi_cif*,శూన్య*,ffi_raw*,శూన్య*),
                          శూన్య *user_దత్తాంశం,
                          శూన్య *codeloc)
{
  అక్షరం *tramp = సంవృతం->tramp;
  శూన్య (*dest)(శూన్య);
  పూర్ణాంకం i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  కొరకు (i = cif->nargs-1; i >= 0; i--)
    ఎంపిక (cif->arg_types[i]->type)
      {
      స్థితి FFI_TYPE_STRUCT:
      స్థితి FFI_TYPE_LONGDOUBLE:
	తిరిగి FFI_BAD_TYPEDEF;
      }

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_THISCALL:
      dest = ffi_సంవృతం_raw_THISCALL;
      ఆపు;
    స్థితి FFI_SYSV:
      dest = ffi_సంవృతం_raw_SYSV;
      ఆపు;
    అప్రమేయ:
      తిరిగి FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(శూన్య **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(గుర్తులేని *)(tramp + 6) = (గుర్తులేని)dest - ((గుర్తులేని)codeloc + 10);

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}

శూన్య
ffi_raw_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, ffi_raw *avalue)
{
  size_t rsize, బైట్లు;
  నిర్మాణం call_చట్రం *చట్రం;
  అక్షరం *దొంతర, *argp;
  ffi_type **arg_types;
  పూర్ణాంకం flags, cabi, i, n, narg_reg;
  స్థిరరాశి నిర్మాణం abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  ఒకవేళ (rvalue == ఖాళీ)
    {
      ఎంపిక (flags)
	{
	స్థితి X86_RET_FLOAT:
	స్థితి X86_RET_DOUBLE:
	స్థితి X86_RET_LDOUBLE:
	స్థితి X86_RET_STRUCTPOP:
	స్థితి X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->కొలత;
	  ఆపు;
	అప్రమేయ:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  ఆపు;
	}
    }

  బైట్లు = దొంతర_ALIGN (cif->బైట్లు);
  argp = దొంతర =
      (శూన్య *)((uintptr_t)alloca(బైట్లు + పరిమాణం(*చట్రం) + rsize + 15) & ~16);
  చట్రం = (నిర్మాణం call_చట్రం *)(దొంతర + బైట్లు);
  ఒకవేళ (rsize)
    rvalue = చట్రం + 1;

  చట్రం->fn = fn;
  చట్రం->flags = flags;
  చట్రం->rvalue = rvalue;

  narg_reg = 0;
  ఎంపిక (flags)
    {
    స్థితి X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      ఒకవేళ (pabi->nregs > 0)
	{
	  చట్రం->regs[pabi->regs[0]] = (గుర్తులేని)rvalue;
	  narg_reg = 1;
	  ఆపు;
	}
      /* fallthru */
    స్థితి X86_RET_STRUCTPOP:
      *(శూన్య **)argp = rvalue;
      argp += పరిమాణం(శూన్య *);
      బైట్లు -= పరిమాణం(శూన్య *);
      ఆపు;
    }

  arg_types = cif->arg_types;
  కొరకు (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->కొలత;
      పూర్ణాంకం t = ty->type;

      ఒకవేళ (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  చట్రం->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      లేకపోతే
	{
	  జ్ఞాపకం_నకలు (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      బైట్లు -= z;
    }
  ఒకవేళ (i < n)
    జ్ఞాపకం_నకలు (argp, avalue, బైట్లు);

  ffi_call_i386 (చట్రం, దొంతర);
}
#అయితే_ముగింపు /* !FFI_NO_RAW_API */

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
శూన్య *
ffi_tramp_arch (size_t *tramp_కొలత, size_t *map_కొలత)
{
  బాహ్య శూన్య *trampoline_సంకేతం_పట్టిక;

  *map_కొలత = X86_TRAMP_MAP_కొలత;
  *tramp_కొలత = X86_TRAMP_కొలత;
  తిరిగి &trampoline_సంకేతం_పట్టిక;
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* __i386__ */
