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

#जर defined(__i386__) || defined(_M_IX86)
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <tramp.h>
#अन्तर्भाव "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 80-bit type.  */
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# जर FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अन्यथा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#जर defined(__GNUC__) && !defined(__declspec)
# define __declspec(x)  __attribute__((x))
#endif

#जर defined(_MSC_VER) && defined(_M_IX86)
/* Stack is not 16-byte aligned on Windows.  */
#define चळत_ALIGN(bytes) (bytes)
#अन्यथा
#define चळत_ALIGN(bytes) FFI_ALIGN (bytes, 16)
#endif

/* Perform machine dependent cif processing.  */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  माप_प्रकार bytes = 0;
  पूर्णांक i, n, flags, cabi = cif->abi;

  निवड (cabi)
    {
    स्थिती FFI_SYSV:
    स्थिती FFI_STDCALL:
    स्थिती FFI_THISCALL:
    स्थिती FFI_FASTCALL:
    स्थिती FFI_MS_CDECL:
    स्थिती FFI_PASCAL:
    स्थिती FFI_REGISTER:
      थांबा;
    पूर्वनिर्धारित:
      परत FFI_BAD_ABI;
    }

  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
      flags = X86_RET_VOID;
      थांबा;
    स्थिती FFI_TYPE_FLOAT:
      flags = X86_RET_FLOAT;
      थांबा;
    स्थिती FFI_TYPE_DOUBLE:
      flags = X86_RET_DOUBLE;
      थांबा;
    स्थिती FFI_TYPE_LONGDOUBLE:
      flags = X86_RET_LDOUBLE;
      थांबा;
    स्थिती FFI_TYPE_UINT8:
      flags = X86_RET_UINT8;
      थांबा;
    स्थिती FFI_TYPE_UINT16:
      flags = X86_RET_UINT16;
      थांबा;
    स्थिती FFI_TYPE_SINT8:
      flags = X86_RET_SINT8;
      थांबा;
    स्थिती FFI_TYPE_SINT16:
      flags = X86_RET_SINT16;
      थांबा;
    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_UINT32:
    स्थिती FFI_TYPE_POINTER:
      flags = X86_RET_INT32;
      थांबा;
    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
      flags = X86_RET_INT64;
      थांबा;
    स्थिती FFI_TYPE_STRUCT:
      {
#जर defined(X86_WIN32) || defined(X86_DARWIN) || defined(X86_FREEBSD)
        माप_प्रकार size = cif->rtype->size;
        जर (size == 1)
          flags = X86_RET_STRUCT_1B;
        अन्यथा जर (size == 2)
          flags = X86_RET_STRUCT_2B;
        अन्यथा जर (size == 4)
          flags = X86_RET_INT32;
        अन्यथा जर (size == 8)
          flags = X86_RET_INT64;
        अन्यथा
#endif
          {
          do_struct:
            निवड (cabi)
              {
              स्थिती FFI_THISCALL:
              स्थिती FFI_FASTCALL:
              स्थिती FFI_STDCALL:
              स्थिती FFI_MS_CDECL:
                flags = X86_RET_STRUCTARG;
                थांबा;
              पूर्वनिर्धारित:
                flags = X86_RET_STRUCTPOP;
                थांबा;
              }
            /* Allocate space for return value pointer.  */
            bytes += FFI_ALIGN (आकार(शून्य*), FFI_SIZEOF_ARG);
          }
      }
      थांबा;
    स्थिती FFI_TYPE_COMPLEX:
      निवड (cif->rtype->elements[0]->type)
	{
	स्थिती FFI_TYPE_DOUBLE:
	स्थिती FFI_TYPE_LONGDOUBLE:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
	  जा do_struct;
	स्थिती FFI_TYPE_FLOAT:
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT32:
	  flags = X86_RET_INT64;
	  थांबा;
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT16:
	  flags = X86_RET_INT32;
	  थांबा;
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT8:
	  flags = X86_RET_STRUCT_2B;
	  थांबा;
	पूर्वनिर्धारित:
	  परत FFI_BAD_TYPEDEF;
	}
      थांबा;
    पूर्वनिर्धारित:
      परत FFI_BAD_TYPEDEF;
    }
  cif->flags = flags;

  साठी (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *t = cif->arg_types[i];

#जर defined(X86_WIN32)
      जर (cabi == FFI_STDCALL)
        bytes = FFI_ALIGN (bytes, FFI_SIZEOF_ARG);
      अन्यथा
#endif
        bytes = FFI_ALIGN (bytes, t->alignment);
      bytes += FFI_ALIGN (t->size, FFI_SIZEOF_ARG);
    }
  cif->bytes = bytes;

  परत FFI_OK;
}

स्थिर ffi_arg
extend_basic_type(शून्य *arg, पूर्णांक type)
{
  निवड (type)
    {
    स्थिती FFI_TYPE_SINT8:
      परत *(SINT8 *)arg;
    स्थिती FFI_TYPE_UINT8:
      परत *(UINT8 *)arg;
    स्थिती FFI_TYPE_SINT16:
      परत *(SINT16 *)arg;
    स्थिती FFI_TYPE_UINT16:
      परत *(UINT16 *)arg;

    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_UINT32:
    स्थिती FFI_TYPE_POINTER:
    स्थिती FFI_TYPE_FLOAT:
      परत *(UINT32 *)arg;

    पूर्वनिर्धारित:
      पात();
    }
}

संरचना call_frame
{
  शून्य *ebp;		/* 0 */
  शून्य *retaddr;	/* 4 */
  शून्य (*fn)(शून्य);	/* 8 */
  पूर्णांक flags;		/* 12 */
  शून्य *rvalue;		/* 16 */
  अचिन्हित regs[3];	/* 20-28 */
};

संरचना abi_params
{
  पूर्णांक dir;		/* parameter growth direction */
  पूर्णांक static_chain;	/* the static chain register used by gcc */
  पूर्णांक nregs;		/* number of register parameters */
  पूर्णांक regs[3];
};

स्थिर स्थिरांक संरचना abi_params abi_params[FFI_LAST_ABI] = {
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
  #अन्यथा
    #define FFI_DECLARE_FASTCALL __declspec(fastcall)
  #endif
#अन्यथा
  #define FFI_DECLARE_FASTCALL
#endif

बाह्य शून्य FFI_DECLARE_FASTCALL ffi_call_i386(संरचना call_frame *, अक्षर *) FFI_HIDDEN;

/* We perform some black magic here to use some of the parent's stack frame in
 * ffi_call_i386() that breaks with the MSVC compiler with the /RTCs or /GZ
 * flags.  Disable the 'Stack frame run time error checking' for this function
 * so we don't hit weird exceptions in debug builds. */
#जर defined(_MSC_VER)
#pragma runtime_checks("s", off)
#endif
/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
{
  माप_प्रकार rsize, bytes;
  संरचना call_frame *frame;
  अक्षर *चळत, *argp;
  ffi_type **arg_types;
  पूर्णांक flags, cabi, i, n, dir, narg_reg;
  स्थिरांक संरचना abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];
  dir = pabi->dir;

  rsize = 0;
  जर (rvalue == रिक्त)
    {
      निवड (flags)
	{
	स्थिती X86_RET_FLOAT:
	स्थिती X86_RET_DOUBLE:
	स्थिती X86_RET_LDOUBLE:
	स्थिती X86_RET_STRUCTPOP:
	स्थिती X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  थांबा;
	पूर्वनिर्धारित:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  थांबा;
	}
    }

  bytes = चळत_ALIGN (cif->bytes);
  चळत = alloca(bytes + आकार(*frame) + rsize);
  argp = (dir < 0 ? चळत + bytes : चळत);
  frame = (संरचना call_frame *)(चळत + bytes);
  जर (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;
  frame->regs[pabi->static_chain] = (अचिन्हित)closure;

  narg_reg = 0;
  निवड (flags)
    {
    स्थिती X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      जर (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (अचिन्हित)rvalue;
	  narg_reg = 1;
	  थांबा;
	}
      /* fallthru */
    स्थिती X86_RET_STRUCTPOP:
      *(शून्य **)argp = rvalue;
      argp += आकार(शून्य *);
      थांबा;
    }

  arg_types = cif->arg_types;
  साठी (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      शून्य *valp = avalue[i];
      माप_प्रकार z = ty->size;
      पूर्णांक t = ty->type;

      जर (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
        {
	  ffi_arg val = extend_basic_type (valp, t);

	  जर (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    frame->regs[pabi->regs[narg_reg++]] = val;
	  अन्यथा जर (dir < 0)
	    {
	      argp -= 4;
	      *(ffi_arg *)argp = val;
	    }
	  अन्यथा
	    {
	      *(ffi_arg *)argp = val;
	      argp += 4;
	    }
	}
      अन्यथा
	{
	  माप_प्रकार za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  माप_प्रकार align = FFI_SIZEOF_ARG;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  जर ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
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
	  जर (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  जर (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      स_नकल (argp, valp, z);
	    }
	  अन्यथा
	    {
	      argp = (अक्षर *)FFI_ALIGN (argp, align);
	      स_नकल (argp, valp, z);
	      argp += za;
	    }
	}
    }
  FFI_ASSERT (dir > 0 || argp == चळत);

  ffi_call_i386 (frame, चळत);
}
#जर defined(_MSC_VER)
#pragma runtime_checks("s", restore)
#endif

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

#ifdef FFI_GO_CLOSURES
शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

/** private members **/

शून्य FFI_HIDDEN ffi_closure_i386(शून्य);
शून्य FFI_HIDDEN ffi_closure_STDCALL(शून्य);
शून्य FFI_HIDDEN ffi_closure_REGISTER(शून्य);
#जर defined(FFI_EXEC_STATIC_TRAMP)
शून्य FFI_HIDDEN ffi_closure_i386_alt(शून्य);
शून्य FFI_HIDDEN ffi_closure_STDCALL_alt(शून्य);
शून्य FFI_HIDDEN ffi_closure_REGISTER_alt(शून्य);
#endif

संरचना closure_frame
{
  अचिन्हित rettemp[4];				/* 0 */
  अचिन्हित regs[3];				/* 16-24 */
  ffi_cif *cif;					/* 28 */
  शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*);	/* 32 */
  शून्य *user_data;				/* 36 */
};

पूर्णांक FFI_HIDDEN FFI_DECLARE_FASTCALL
ffi_closure_inner (संरचना closure_frame *frame, अक्षर *चळत)
{
  ffi_cif *cif = frame->cif;
  पूर्णांक cabi, i, n, flags, dir, narg_reg;
  स्थिरांक संरचना abi_params *pabi;
  ffi_type **arg_types;
  अक्षर *argp;
  शून्य *rvalue;
  शून्य **avalue;

  cabi = cif->abi;
  flags = cif->flags;
  narg_reg = 0;
  rvalue = frame->rettemp;
  pabi = &abi_params[cabi];
  dir = pabi->dir;
  argp = (dir < 0 ? चळत + चळत_ALIGN (cif->bytes) : चळत);

  निवड (flags)
    {
    स्थिती X86_RET_STRUCTARG:
      जर (pabi->nregs > 0)
	{
	  rvalue = (शून्य *)frame->regs[pabi->regs[0]];
	  narg_reg = 1;
	  frame->rettemp[0] = (अचिन्हित)rvalue;
	  थांबा;
	}
      /* fallthru */
    स्थिती X86_RET_STRUCTPOP:
      rvalue = *(शून्य **)argp;
      argp += आकार(शून्य *);
      frame->rettemp[0] = (अचिन्हित)rvalue;
      थांबा;
    }

  n = cif->nargs;
  avalue = alloca(आकार(शून्य *) * n);

  arg_types = cif->arg_types;
  साठी (i = 0; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      माप_प्रकार z = ty->size;
      पूर्णांक t = ty->type;
      शून्य *valp;

      जर (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT)
	{
	  जर (t != FFI_TYPE_FLOAT && narg_reg < pabi->nregs)
	    valp = &frame->regs[pabi->regs[narg_reg++]];
	  अन्यथा जर (dir < 0)
	    {
	      argp -= 4;
	      valp = argp;
	    }
	  अन्यथा
	    {
	      valp = argp;
	      argp += 4;
	    }
	}
      अन्यथा
	{
	  माप_प्रकार za = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  माप_प्रकार align = FFI_SIZEOF_ARG;

	  /* See the comment in ffi_call_int.  */
	  जर (t == FFI_TYPE_STRUCT && ty->alignment >= 16)
	    align = 16;

	  /* Issue 434: For thiscall and fastcall, if the parameter passed
	     as 64-bit integer or struct, all following integer parameters
	     will be passed on stack.  */
	  जर ((cabi == FFI_THISCALL || cabi == FFI_FASTCALL)
	      && (t == FFI_TYPE_SINT64
		  || t == FFI_TYPE_UINT64
		  || t == FFI_TYPE_STRUCT))
	    narg_reg = 2;

	  जर (dir < 0)
	    {
	      /* ??? These reverse argument ABIs are probably too old
		 to have cared about alignment.  Someone should check.  */
	      argp -= za;
	      valp = argp;
	    }
	  अन्यथा
	    {
	      argp = (अक्षर *)FFI_ALIGN (argp, align);
	      valp = argp;
	      argp += za;
	    }
	}

      avalue[i] = valp;
    }

  frame->fun (cif, rvalue, avalue, frame->user_data);

  निवड (cabi)
    {
    स्थिती FFI_STDCALL:
      परत flags | (cif->bytes << X86_RET_काढा_SHIFT);
    स्थिती FFI_THISCALL:
    स्थिती FFI_FASTCALL:
      /* The callee must pop exactly the bytes that were passed on the
	 stack.  Deriving that from cif->bytes minus narg_reg * 4 is wrong
	 once narg_reg has been force-bumped to 2 (above) for a 64-bit or
	 struct argument that is itself placed on the stack: the subtraction
	 then discounts register slots that were never used, under-popping
	 the stack.  argp has advanced past exactly the stack-resident
	 arguments (dir == 1 for these ABIs), so use that directly.  */
      परत flags | (((अचिन्हित) (argp - चळत)) << X86_RET_काढा_SHIFT);
    पूर्वनिर्धारित:
      परत flags;
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
                      शून्य *user_data,
                      शून्य *codeloc)
{
  अक्षर *tramp = closure->tramp;
  शून्य (*dest)(शून्य);
  पूर्णांक op = 0xb8;  /* movl imm, %eax */

  निवड (cif->abi)
    {
    स्थिती FFI_SYSV:
    स्थिती FFI_MS_CDECL:
      dest = ffi_closure_i386;
      थांबा;
    स्थिती FFI_STDCALL:
    स्थिती FFI_THISCALL:
    स्थिती FFI_FASTCALL:
    स्थिती FFI_PASCAL:
      dest = ffi_closure_STDCALL;
      थांबा;
    स्थिती FFI_REGISTER:
      dest = ffi_closure_REGISTER;
      op = 0x68;  /* pushl imm */
      थांबा;
    पूर्वनिर्धारित:
      परत FFI_BAD_ABI;
    }

#जर defined(FFI_EXEC_STATIC_TRAMP)
  जर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      जर (dest == ffi_closure_i386)
        dest = ffi_closure_i386_alt;
      अन्यथा जर (dest == ffi_closure_STDCALL)
        dest = ffi_closure_STDCALL_alt;
      अन्यथा
        dest = ffi_closure_REGISTER_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      जा out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  /* endbr32.  */
  *(UINT32 *) tramp = 0xfb1e0ff3;

  /* movl or pushl immediate.  */
  tramp[4] = op;
  *(शून्य **)(tramp + 5) = codeloc;

  /* jmp dest */
  tramp[9] = 0xe9;
  *(अचिन्हित *)(tramp + 10) = (अचिन्हित)dest - ((अचिन्हित)codeloc + 14);

#जर defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  परत FFI_OK;
}

#ifdef FFI_GO_CLOSURES

शून्य FFI_HIDDEN ffi_go_closure_EAX(शून्य);
शून्य FFI_HIDDEN ffi_go_closure_ECX(शून्य);
शून्य FFI_HIDDEN ffi_go_closure_STDCALL(शून्य);

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*))
{
  शून्य (*dest)(शून्य);

  निवड (cif->abi)
    {
    स्थिती FFI_SYSV:
    स्थिती FFI_MS_CDECL:
      dest = ffi_go_closure_ECX;
      थांबा;
    स्थिती FFI_THISCALL:
    स्थिती FFI_FASTCALL:
      dest = ffi_go_closure_EAX;
      थांबा;
    स्थिती FFI_STDCALL:
    स्थिती FFI_PASCAL:
      dest = ffi_go_closure_STDCALL;
      थांबा;
    स्थिती FFI_REGISTER:
    पूर्वनिर्धारित:
      परत FFI_BAD_ABI;
    }

  closure->tramp = dest;
  closure->cif = cif;
  closure->fun = fun;

  परत FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

/* ------- Native raw API support -------------------------------- */

#जर !FFI_NO_RAW_API

शून्य FFI_HIDDEN ffi_closure_raw_SYSV(शून्य);
शून्य FFI_HIDDEN ffi_closure_raw_THISCALL(शून्य);

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure *closure,
                          ffi_cif *cif,
                          शून्य (*fun)(ffi_cif*,शून्य*,ffi_raw*,शून्य*),
                          शून्य *user_data,
                          शून्य *codeloc)
{
  अक्षर *tramp = closure->tramp;
  शून्य (*dest)(शून्य);
  पूर्णांक i;

  /* We currently don't support certain kinds of arguments for raw
     closures.  This should be implemented by a separate assembly
     language routine, since it would require argument processing,
     something we don't do now for performance.  */
  साठी (i = cif->nargs-1; i >= 0; i--)
    निवड (cif->arg_types[i]->type)
      {
      स्थिती FFI_TYPE_STRUCT:
      स्थिती FFI_TYPE_LONGDOUBLE:
	परत FFI_BAD_TYPEDEF;
      }

  निवड (cif->abi)
    {
    स्थिती FFI_THISCALL:
      dest = ffi_closure_raw_THISCALL;
      थांबा;
    स्थिती FFI_SYSV:
      dest = ffi_closure_raw_SYSV;
      थांबा;
    पूर्वनिर्धारित:
      परत FFI_BAD_ABI;
    }

  /* movl imm, %eax.  */
  tramp[0] = 0xb8;
  *(शून्य **)(tramp + 1) = codeloc;

  /* jmp dest */
  tramp[5] = 0xe9;
  *(अचिन्हित *)(tramp + 6) = (अचिन्हित)dest - ((अचिन्हित)codeloc + 10);

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  परत FFI_OK;
}

शून्य
ffi_raw_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, ffi_raw *avalue)
{
  माप_प्रकार rsize, bytes;
  संरचना call_frame *frame;
  अक्षर *चळत, *argp;
  ffi_type **arg_types;
  पूर्णांक flags, cabi, i, n, narg_reg;
  स्थिरांक संरचना abi_params *pabi;

  flags = cif->flags;
  cabi = cif->abi;
  pabi = &abi_params[cabi];

  rsize = 0;
  जर (rvalue == रिक्त)
    {
      निवड (flags)
	{
	स्थिती X86_RET_FLOAT:
	स्थिती X86_RET_DOUBLE:
	स्थिती X86_RET_LDOUBLE:
	स्थिती X86_RET_STRUCTPOP:
	स्थिती X86_RET_STRUCTARG:
	  /* The float cases need to pop the 387 stack.
	     The struct cases need to pass a valid pointer to the callee.  */
	  rsize = cif->rtype->size;
	  थांबा;
	पूर्वनिर्धारित:
	  /* We can pretend that the callee returns nothing.  */
	  flags = X86_RET_VOID;
	  थांबा;
	}
    }

  bytes = चळत_ALIGN (cif->bytes);
  argp = चळत =
      (शून्य *)((uintptr_t)alloca(bytes + आकार(*frame) + rsize + 15) & ~16);
  frame = (संरचना call_frame *)(चळत + bytes);
  जर (rsize)
    rvalue = frame + 1;

  frame->fn = fn;
  frame->flags = flags;
  frame->rvalue = rvalue;

  narg_reg = 0;
  निवड (flags)
    {
    स्थिती X86_RET_STRUCTARG:
      /* The pointer is passed as the first argument.  */
      जर (pabi->nregs > 0)
	{
	  frame->regs[pabi->regs[0]] = (अचिन्हित)rvalue;
	  narg_reg = 1;
	  थांबा;
	}
      /* fallthru */
    स्थिती X86_RET_STRUCTPOP:
      *(शून्य **)argp = rvalue;
      argp += आकार(शून्य *);
      bytes -= आकार(शून्य *);
      थांबा;
    }

  arg_types = cif->arg_types;
  साठी (i = 0, n = cif->nargs; narg_reg < pabi->nregs && i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      माप_प्रकार z = ty->size;
      पूर्णांक t = ty->type;

      जर (z <= FFI_SIZEOF_ARG && t != FFI_TYPE_STRUCT && t != FFI_TYPE_FLOAT)
	{
	  ffi_arg val = extend_basic_type (avalue, t);
	  frame->regs[pabi->regs[narg_reg++]] = val;
	  z = FFI_SIZEOF_ARG;
	}
      अन्यथा
	{
	  स_नकल (argp, avalue, z);
	  z = FFI_ALIGN (z, FFI_SIZEOF_ARG);
	  argp += z;
	}
      avalue += z;
      bytes -= z;
    }
  जर (i < n)
    स_नकल (argp, avalue, bytes);

  ffi_call_i386 (frame, चळत);
}
#endif /* !FFI_NO_RAW_API */

#जर defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (माप_प्रकार *tramp_size, माप_प्रकार *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *map_size = X86_TRAMP_MAP_SIZE;
  *tramp_size = X86_TRAMP_SIZE;
  परत &trampoline_code_table;
}
#endif

#endif /* __i386__ */
