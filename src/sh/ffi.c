/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2002-2008, 2012 Kaz Kojima
           Copyright (c) 2008 Red Hat, Inc.
   
   SuperH Foreign Function Interface 

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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NGREGARG 4
#ਜੇ defined(__SH4__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NFREGARG 8
#ਜੇ_ਅੰਤ

#ਜੇ defined(__HITACHI__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STRUCT_ਮੁੱਲ_ਪਤਾ_WITH_ARG 1
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STRUCT_ਮੁੱਲ_ਪਤਾ_WITH_ARG 0
#ਜੇ_ਅੰਤ

/* If the structure has essentially an unique element, return its type.  */
ਸਥਿਰ ਪੂਰਨ
simple_type (ffi_type *arg)
{
  ਜੇ (arg->type != FFI_TYPE_STRUCT)
    ਵਾਪਸ arg->type;
  ਨਹੀਂਤਾਂ ਜੇ (arg->elements[1])
    ਵਾਪਸ FFI_TYPE_STRUCT;

  ਵਾਪਸ simple_type (arg->elements[0]);
}

ਸਥਿਰ ਪੂਰਨ
return_type (ffi_type *arg)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ type;

  ਜੇ (arg->type != FFI_TYPE_STRUCT)
    ਵਾਪਸ arg->type;

  type = simple_type (arg->elements[0]);
  ਜੇ (! arg->elements[1])
    {
      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	  ਵਾਪਸ FFI_TYPE_INT;

	ਮੂਲ:
	  ਵਾਪਸ type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  ਜੇ (arg->ਮਾਪ <= 2 * ਆਕਾਰ (ਪੂਰਨ))
    {
      ਪੂਰਨ i = 0;
      ffi_type *e;

      ਜਦੋਂ ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  ਚੋਣ (type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT32:
	    ਹਾਲਤ FFI_TYPE_UINT32:
	    ਹਾਲਤ FFI_TYPE_INT:
	    ਹਾਲਤ FFI_TYPE_FLOAT:
	      ਵਾਪਸ FFI_TYPE_UINT64;

	    ਮੂਲ:
	      ਰੋਕੋ;
	    }
	}
    }

  ਵਾਪਸ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ਖਾਲੀ ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, extended_cif *ecif)
{
  ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਰਜਿਸਟਰ ਪੂਰਨ tmp;
  ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ avn;
  ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
  ਰਜਿਸਟਰ ਅੱਖਰ *argp;
  ਰਜਿਸਟਰ ffi_type **p_arg;
  ਪੂਰਨ greg, ireg;
#ਜੇ defined(__SH4__)
  ਪੂਰਨ freg = 0;
#ਜੇ_ਅੰਤ

  tmp = 0;
  argp = ਢੇਰ;

  ਜੇ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(ਖਾਲੀ **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_ਮੁੱਲ_ਪਤਾ_WITH_ARG ? 1 : 0;
    }
  ਨਹੀਂਤਾਂ
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ਲਈ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->ਮਾਪ;
      ਜੇ (z < ਆਕਾਰ(ਪੂਰਨ))
	{
	  ਜੇ (greg++ >= NGREGARG)
	    ਜਾਰੀ;

	  z = ਆਕਾਰ(ਪੂਰਨ);
	  ਚੋਣ ((*p_arg)->type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT8:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT8 *)(* p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_UINT8:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT8 *)(* p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_SINT16:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT16 *)(* p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_UINT16:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT16 *)(* p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_STRUCT:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT32 *)(* p_argv);
	      ਰੋਕੋ;

	    ਮੂਲ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ(ਪੂਰਨ))
	{
#ਜੇ defined(__SH4__)
	  ਜੇ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ਜੇ (freg++ >= NFREGARG)
		ਜਾਰੀ;
	    }
	  ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	    {
	      ਜੇ (greg++ >= NGREGARG)
		ਜਾਰੀ;
	    }
	  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ਜੇ defined(__SH4__)
      ਨਹੀਂਤਾਂ ਜੇ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ਜੇ (freg + 1 >= NFREGARG)
	    ਜਾਰੀ;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
	  argp += z;
	}
#ਜੇ_ਅੰਤ
      ਨਹੀਂਤਾਂ
	{
	  ਪੂਰਨ n = (z + ਆਕਾਰ (ਪੂਰਨ) - 1) / ਆਕਾਰ (ਪੂਰਨ);
#ਜੇ defined(__SH4__)
	  ਜੇ (greg + n - 1 >= NGREGARG)
	    ਜਾਰੀ;
#ਨਹੀਂਤਾਂ
	  ਜੇ (greg >= NGREGARG)
	    ਜਾਰੀ;
#ਜੇ_ਅੰਤ
	  greg += n;
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
	  argp += n * ਆਕਾਰ (ਪੂਰਨ);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#ਜੇ defined(__SH4__)
  freg = 0;
#ਜੇ_ਅੰਤ
  p_argv = ecif->avalue;

  ਲਈ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->ਮਾਪ;
      ਜੇ (z < ਆਕਾਰ(ਪੂਰਨ))
	{
	  ਜੇ (greg++ < NGREGARG)
	    ਜਾਰੀ;

	  z = ਆਕਾਰ(ਪੂਰਨ);
	  ਚੋਣ ((*p_arg)->type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT8:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT8 *)(* p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_UINT8:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT8 *)(* p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_SINT16:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT16 *)(* p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_UINT16:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT16 *)(* p_argv);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_STRUCT:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT32 *)(* p_argv);
	      ਰੋਕੋ;

	    ਮੂਲ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ(ਪੂਰਨ))
	{
#ਜੇ defined(__SH4__)
	  ਜੇ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ਜੇ (freg++ < NFREGARG)
		ਜਾਰੀ;
	    }
	  ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	    {
	      ਜੇ (greg++ < NGREGARG)
		ਜਾਰੀ;
	    }
	  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ਜੇ defined(__SH4__)
      ਨਹੀਂਤਾਂ ਜੇ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ਜੇ (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ਜਾਰੀ;
	    }
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
	  argp += z;
	}
#ਜੇ_ਅੰਤ
      ਨਹੀਂਤਾਂ
	{
	  ਪੂਰਨ n = (z + ਆਕਾਰ (ਪੂਰਨ) - 1) / ਆਕਾਰ (ਪੂਰਨ);
	  ਜੇ (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ਜਾਰੀ;
	    }
#ਜੇ (! defined(__SH4__))
	  ਨਹੀਂਤਾਂ ਜੇ (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      ਜਾਰੀ;
	    }
#ਜੇ_ਅੰਤ
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
	  argp += n * ਆਕਾਰ (ਪੂਰਨ);
	}
    }

  ਵਾਪਸ;
}

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
  ਪੂਰਨ i, j;
  ਪੂਰਨ ਮਾਪ, type;
  ਪੂਰਨ n, m;
  ਪੂਰਨ greg;
#ਜੇ defined(__SH4__)
  ਪੂਰਨ freg = 0;
#ਜੇ_ਅੰਤ

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_ਮੁੱਲ_ਪਤਾ_WITH_ARG) ? 1 : 0;

#ਜੇ defined(__SH4__)
  ਲਈ (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_FLOAT:
	  ਜੇ (freg >= NFREGARG)
	    ਜਾਰੀ;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਜੇ ((freg + 1) >= NFREGARG)
	    ਜਾਰੀ;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ਰੋਕੋ;
	      
	ਮੂਲ:
	  ਮਾਪ = (cif->arg_types)[i]->ਮਾਪ;
	  n = (ਮਾਪ + ਆਕਾਰ (ਪੂਰਨ) - 1) / ਆਕਾਰ (ਪੂਰਨ);
	  ਜੇ (greg + n - 1 >= NGREGARG)
		ਜਾਰੀ;
	  greg += n;
	  ਲਈ (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  ਰੋਕੋ;
	}
    }
#ਨਹੀਂਤਾਂ
  ਲਈ (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      ਮਾਪ = (cif->arg_types)[i]->ਮਾਪ;
      n = (ਮਾਪ + ਆਕਾਰ (ਪੂਰਨ) - 1) / ਆਕਾਰ (ਪੂਰਨ);
      ਜੇ (greg >= NGREGARG)
	ਜਾਰੀ;
      ਨਹੀਂਤਾਂ ਜੇ (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      ਲਈ (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#ਜੇ_ਅੰਤ

  /* Set the return type flag */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_STRUCT:
      cif->flags += (ਬਿਨਾਨਿਸ਼ਾਨੀ) (return_type (cif->rtype)) << 24;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_VOID:
    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      cif->flags += (ਬਿਨਾਨਿਸ਼ਾਨੀ) cif->rtype->type << 24;
      ਰੋਕੋ;

    ਮੂਲ:
      cif->flags += FFI_TYPE_INT << 24;
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV(ਖਾਲੀ (*)(ਅੱਖਰ *, extended_cif *), extended_cif *,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *, ਖਾਲੀ (*fn)(ਖਾਲੀ));

ਖਾਲੀ ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  ਨਹੀਂਤਾਂ ਜੇ ((rvalue == ਨਿਰਾ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ਮਾਪ);
    }
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  ਚੋਣ (cif->abi) 
    {
    ਹਾਲਤ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_ਦਲੀਲਾਂ, &ecif, cif->ਬਾਈਟਾਂ, cif->flags, ecif.rvalue,
		    fn);
      ਰੋਕੋ;
    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }

  ਜੇ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, &trvalue, cif->rtype->ਮਾਪ);
}

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_SYSV (ਖਾਲੀ);
#ਜੇ defined(__SH4__)
ਬਾਹਰੀ ਖਾਲੀ __ic_invalidate (ਖਾਲੀ *ਸਤਰ);
#ਜੇ_ਅੰਤ

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ insn;

  ਜੇ (cif->abi != FFI_SYSV)
    ਵਾਪਸ FFI_BAD_ABI;

  tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &ਘੇਰਾ->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#ਨਹੀਂਤਾਂ
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#ਜੇ_ਅੰਤ
  *(ਖਾਲੀ **) &tramp[2] = (ਖਾਲੀ *)codeloc;          /* ctx */
  *(ਖਾਲੀ **) &tramp[3] = (ਖਾਲੀ *)ffi_ਘੇਰਾ_SYSV; /* funaddr */

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

#ਜੇ defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#ਜੇ_ਅੰਤ

  ਵਾਪਸ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OFS_INT8	0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OFS_INT16	0
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OFS_INT8	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OFS_INT16	2
#ਜੇ_ਅੰਤ

ਪੂਰਨ
ffi_ਘੇਰਾ_helper_SYSV (ffi_ਘੇਰਾ *ਘੇਰਾ, ਖਾਲੀ *rvalue, 
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *pgr, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *pfr, 
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *pst)
{
  ਖਾਲੀ **avalue;
  ffi_type **p_arg;
  ਪੂਰਨ i, avn;
  ਪੂਰਨ ireg, greg = 0;
#ਜੇ defined(__SH4__)
  ਪੂਰਨ freg = 0;
#ਜੇ_ਅੰਤ
  ffi_cif *cif; 

  cif = ਘੇਰਾ->cif;
  avalue = alloca(cif->nargs * ਆਕਾਰ(ਖਾਲੀ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_ਮੁੱਲ_ਪਤਾ_WITH_ARG)
    {
      rvalue = (ਖਾਲੀ *) *pgr++;
      ireg = 1;
    }
  ਨਹੀਂਤਾਂ
    ireg = 0;

  cif = ਘੇਰਾ->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  ਲਈ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ਮਾਪ;
      ਜੇ (z < ਆਕਾਰ(ਪੂਰਨ))
	{
	  ਜੇ (greg++ >= NGREGARG)
	    ਜਾਰੀ;

	  z = ਆਕਾਰ(ਪੂਰਨ);
	  ਚੋਣ ((*p_arg)->type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT8:
	    ਹਾਲਤ FFI_TYPE_UINT8:
	      avalue[i] = (((ਅੱਖਰ *)pgr) + OFS_INT8);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_SINT16:
	    ਹਾਲਤ FFI_TYPE_UINT16:
	      avalue[i] = (((ਅੱਖਰ *)pgr) + OFS_INT16);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      ਰੋਕੋ;

	    ਮੂਲ:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ(ਪੂਰਨ))
	{
#ਜੇ defined(__SH4__)
	  ਜੇ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ਜੇ (freg++ >= NFREGARG)
		ਜਾਰੀ;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	    {
	      ਜੇ (greg++ >= NGREGARG)
		ਜਾਰੀ;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#ਜੇ defined(__SH4__)
      ਨਹੀਂਤਾਂ ਜੇ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ਜੇ (freg + 1 >= NFREGARG)
	    ਜਾਰੀ;
	  ਜੇ (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#ਜੇ_ਅੰਤ
      ਨਹੀਂਤਾਂ
	{
	  ਪੂਰਨ n = (z + ਆਕਾਰ (ਪੂਰਨ) - 1) / ਆਕਾਰ (ਪੂਰਨ);
#ਜੇ defined(__SH4__)
	  ਜੇ (greg + n - 1 >= NGREGARG)
	    ਜਾਰੀ;
#ਨਹੀਂਤਾਂ
	  ਜੇ (greg >= NGREGARG)
	    ਜਾਰੀ;
#ਜੇ_ਅੰਤ
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#ਜੇ defined(__SH4__)
  freg = 0;
#ਜੇ_ਅੰਤ

  ਲਈ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ਮਾਪ;
      ਜੇ (z < ਆਕਾਰ(ਪੂਰਨ))
	{
	  ਜੇ (greg++ < NGREGARG)
	    ਜਾਰੀ;

	  z = ਆਕਾਰ(ਪੂਰਨ);
	  ਚੋਣ ((*p_arg)->type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT8:
	    ਹਾਲਤ FFI_TYPE_UINT8:
	      avalue[i] = (((ਅੱਖਰ *)pst) + OFS_INT8);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_SINT16:
	    ਹਾਲਤ FFI_TYPE_UINT16:
	      avalue[i] = (((ਅੱਖਰ *)pst) + OFS_INT16);
	      ਰੋਕੋ;
  
	    ਹਾਲਤ FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      ਰੋਕੋ;

	    ਮੂਲ:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ(ਪੂਰਨ))
	{
#ਜੇ defined(__SH4__)
	  ਜੇ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ਜੇ (freg++ < NFREGARG)
		ਜਾਰੀ;
	    }
	  ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	    {
	      ਜੇ (greg++ < NGREGARG)
		ਜਾਰੀ;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#ਜੇ defined(__SH4__)
      ਨਹੀਂਤਾਂ ਜੇ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ਜੇ (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ਜਾਰੀ;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#ਜੇ_ਅੰਤ
      ਨਹੀਂਤਾਂ
	{
	  ਪੂਰਨ n = (z + ਆਕਾਰ (ਪੂਰਨ) - 1) / ਆਕਾਰ (ਪੂਰਨ);
	  ਜੇ (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ਜਾਰੀ;
	    }
#ਜੇ (! defined(__SH4__))
	  ਨਹੀਂਤਾਂ ਜੇ (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      ਜਾਰੀ;
	    }
#ਜੇ_ਅੰਤ
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (ਘੇਰਾ->fun) (cif, rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ਵਾਪਸ return_type (cif->rtype);
}
