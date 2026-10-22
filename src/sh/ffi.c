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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>

#define NGREGARG 4
#எனில் defined(__SH4__)
#define NFREGARG 8
#endif

#எனில் defined(__HITACHI__)
#define STRUCT_மதிப்பு_ADDRESS_WITH_ARG 1
#இல்லையெனில்
#define STRUCT_மதிப்பு_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
நிலையான முழு
simple_type (ffi_type *arg)
{
  எனில் (arg->type != FFI_TYPE_STRUCT)
    திரும்பு arg->type;
  இல்லையெனில் எனில் (arg->elements[1])
    திரும்பு FFI_TYPE_STRUCT;

  திரும்பு simple_type (arg->elements[0]);
}

நிலையான முழு
return_type (ffi_type *arg)
{
  unsigned short type;

  எனில் (arg->type != FFI_TYPE_STRUCT)
    திரும்பு arg->type;

  type = simple_type (arg->elements[0]);
  எனில் (! arg->elements[1])
    {
      தேர்வு (type)
	{
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT8:
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT16:
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT32:
	  திரும்பு FFI_TYPE_INT;

	இயல்பு:
	  திரும்பு type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  எனில் (arg->size <= 2 * அளவு (முழு))
    {
      முழு i = 0;
      ffi_type *e;

      போது ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  தேர்வு (type)
	    {
	    நிலை FFI_TYPE_SINT32:
	    நிலை FFI_TYPE_UINT32:
	    நிலை FFI_TYPE_INT:
	    நிலை FFI_TYPE_FLOAT:
	      திரும்பு FFI_TYPE_UINT64;

	    இயல்பு:
	      நிறுத்து;
	    }
	}
    }

  திரும்பு FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

வெறுமை ffi_prep_args(எழுத்து *அடுக்கு, extended_cif *ecif)
{
  பதிவு unsigned முழு i;
  பதிவு முழு tmp;
  பதிவு unsigned முழு avn;
  பதிவு வெறுமை **p_argv;
  பதிவு எழுத்து *argp;
  பதிவு ffi_type **p_arg;
  முழு greg, ireg;
#எனில் defined(__SH4__)
  முழு freg = 0;
#endif

  tmp = 0;
  argp = அடுக்கு;

  எனில் (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(வெறுமை **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_மதிப்பு_ADDRESS_WITH_ARG ? 1 : 0;
    }
  இல்லையெனில்
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  க்கு (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      எனில் (z < அளவு(முழு))
	{
	  எனில் (greg++ >= NGREGARG)
	    தொடர்;

	  z = அளவு(முழு);
	  தேர்வு ((*p_arg)->type)
	    {
	    நிலை FFI_TYPE_SINT8:
	      *(signed முழு *) argp = (signed முழு)*(SINT8 *)(* p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_UINT8:
	      *(unsigned முழு *) argp = (unsigned முழு)*(UINT8 *)(* p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_SINT16:
	      *(signed முழு *) argp = (signed முழு)*(SINT16 *)(* p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_UINT16:
	      *(unsigned முழு *) argp = (unsigned முழு)*(UINT16 *)(* p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_STRUCT:
	      *(unsigned முழு *) argp = (unsigned முழு)*(UINT32 *)(* p_argv);
	      நிறுத்து;

	    இயல்பு:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      இல்லையெனில் எனில் (z == அளவு(முழு))
	{
#எனில் defined(__SH4__)
	  எனில் ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      எனில் (freg++ >= NFREGARG)
		தொடர்;
	    }
	  இல்லையெனில்
#endif
	    {
	      எனில் (greg++ >= NGREGARG)
		தொடர்;
	    }
	  *(unsigned முழு *) argp = (unsigned முழு)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#எனில் defined(__SH4__)
      இல்லையெனில் எனில் ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  எனில் (freg + 1 >= NFREGARG)
	    தொடர்;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      இல்லையெனில்
	{
	  முழு n = (z + அளவு (முழு) - 1) / அளவு (முழு);
#எனில் defined(__SH4__)
	  எனில் (greg + n - 1 >= NGREGARG)
	    தொடர்;
#இல்லையெனில்
	  எனில் (greg >= NGREGARG)
	    தொடர்;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * அளவு (முழு);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#எனில் defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  க்கு (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      எனில் (z < அளவு(முழு))
	{
	  எனில் (greg++ < NGREGARG)
	    தொடர்;

	  z = அளவு(முழு);
	  தேர்வு ((*p_arg)->type)
	    {
	    நிலை FFI_TYPE_SINT8:
	      *(signed முழு *) argp = (signed முழு)*(SINT8 *)(* p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_UINT8:
	      *(unsigned முழு *) argp = (unsigned முழு)*(UINT8 *)(* p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_SINT16:
	      *(signed முழு *) argp = (signed முழு)*(SINT16 *)(* p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_UINT16:
	      *(unsigned முழு *) argp = (unsigned முழு)*(UINT16 *)(* p_argv);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_STRUCT:
	      *(unsigned முழு *) argp = (unsigned முழு)*(UINT32 *)(* p_argv);
	      நிறுத்து;

	    இயல்பு:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      இல்லையெனில் எனில் (z == அளவு(முழு))
	{
#எனில் defined(__SH4__)
	  எனில் ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      எனில் (freg++ < NFREGARG)
		தொடர்;
	    }
	  இல்லையெனில்
#endif
	    {
	      எனில் (greg++ < NGREGARG)
		தொடர்;
	    }
	  *(unsigned முழு *) argp = (unsigned முழு)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#எனில் defined(__SH4__)
      இல்லையெனில் எனில் ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  எனில் (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      தொடர்;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      இல்லையெனில்
	{
	  முழு n = (z + அளவு (முழு) - 1) / அளவு (முழு);
	  எனில் (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      தொடர்;
	    }
#எனில் (! defined(__SH4__))
	  இல்லையெனில் எனில் (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      தொடர்;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * அளவு (முழு);
	}
    }

  திரும்பு;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  முழு i, j;
  முழு size, type;
  முழு n, m;
  முழு greg;
#எனில் defined(__SH4__)
  முழு freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_மதிப்பு_ADDRESS_WITH_ARG) ? 1 : 0;

#எனில் defined(__SH4__)
  க்கு (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      தேர்வு (type)
	{
	நிலை FFI_TYPE_FLOAT:
	  எனில் (freg >= NFREGARG)
	    தொடர்;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	  எனில் ((freg + 1) >= NFREGARG)
	    தொடர்;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  நிறுத்து;
	      
	இயல்பு:
	  size = (cif->arg_types)[i]->size;
	  n = (size + அளவு (முழு) - 1) / அளவு (முழு);
	  எனில் (greg + n - 1 >= NGREGARG)
		தொடர்;
	  greg += n;
	  க்கு (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  நிறுத்து;
	}
    }
#இல்லையெனில்
  க்கு (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + அளவு (முழு) - 1) / அளவு (முழு);
      எனில் (greg >= NGREGARG)
	தொடர்;
      இல்லையெனில் எனில் (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      க்கு (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_STRUCT:
      cif->flags += (unsigned) (return_type (cif->rtype)) << 24;
      நிறுத்து;

    நிலை FFI_TYPE_VOID:
    நிலை FFI_TYPE_FLOAT:
    நிலை FFI_TYPE_DOUBLE:
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
      cif->flags += (unsigned) cif->rtype->type << 24;
      நிறுத்து;

    இயல்பு:
      cif->flags += FFI_TYPE_INT << 24;
      நிறுத்து;
    }

  திரும்பு FFI_OK;
}

வெளி வெறுமை ffi_call_SYSV(வெறுமை (*)(எழுத்து *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, வெறுமை (*fn)(வெறுமை));

வெறுமை ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  எனில் (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  இல்லையெனில் எனில் ((rvalue == வெற்று) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  இல்லையெனில்
    ecif.rvalue = rvalue;

  தேர்வு (cif->abi) 
    {
    நிலை FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      நிறுத்து;
    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
    }

  எனில் (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

வெளி வெறுமை ffi_closure_SYSV (வெறுமை);
#எனில் defined(__SH4__)
வெளி வெறுமை __ic_invalidate (வெறுமை *வரி);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  unsigned முழு *tramp;
  unsigned முழு insn;

  எனில் (cif->abi != FFI_SYSV)
    திரும்பு FFI_BAD_ABI;

  tramp = (unsigned முழு *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#இல்லையெனில்
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(வெறுமை **) &tramp[2] = (வெறுமை *)codeloc;          /* ctx */
  *(வெறுமை **) &tramp[3] = (வெறுமை *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#எனில் defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  திரும்பு FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

#ifdef __LITTLE_ENDIAN__
#define OFS_INT8	0
#define OFS_INT16	0
#இல்லையெனில்
#define OFS_INT8	3
#define OFS_INT16	2
#endif

முழு
ffi_closure_helper_SYSV (ffi_closure *closure, வெறுமை *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  வெறுமை **avalue;
  ffi_type **p_arg;
  முழு i, avn;
  முழு ireg, greg = 0;
#எனில் defined(__SH4__)
  முழு freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * அளவு(வெறுமை *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  எனில் (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_மதிப்பு_ADDRESS_WITH_ARG)
    {
      rvalue = (வெறுமை *) *pgr++;
      ireg = 1;
    }
  இல்லையெனில்
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  க்கு (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      எனில் (z < அளவு(முழு))
	{
	  எனில் (greg++ >= NGREGARG)
	    தொடர்;

	  z = அளவு(முழு);
	  தேர்வு ((*p_arg)->type)
	    {
	    நிலை FFI_TYPE_SINT8:
	    நிலை FFI_TYPE_UINT8:
	      avalue[i] = (((எழுத்து *)pgr) + OFS_INT8);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_SINT16:
	    நிலை FFI_TYPE_UINT16:
	      avalue[i] = (((எழுத்து *)pgr) + OFS_INT16);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      நிறுத்து;

	    இயல்பு:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      இல்லையெனில் எனில் (z == அளவு(முழு))
	{
#எனில் defined(__SH4__)
	  எனில் ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      எனில் (freg++ >= NFREGARG)
		தொடர்;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  இல்லையெனில்
#endif
	    {
	      எனில் (greg++ >= NGREGARG)
		தொடர்;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#எனில் defined(__SH4__)
      இல்லையெனில் எனில் ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  எனில் (freg + 1 >= NFREGARG)
	    தொடர்;
	  எனில் (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      இல்லையெனில்
	{
	  முழு n = (z + அளவு (முழு) - 1) / அளவு (முழு);
#எனில் defined(__SH4__)
	  எனில் (greg + n - 1 >= NGREGARG)
	    தொடர்;
#இல்லையெனில்
	  எனில் (greg >= NGREGARG)
	    தொடர்;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#எனில் defined(__SH4__)
  freg = 0;
#endif

  க்கு (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      எனில் (z < அளவு(முழு))
	{
	  எனில் (greg++ < NGREGARG)
	    தொடர்;

	  z = அளவு(முழு);
	  தேர்வு ((*p_arg)->type)
	    {
	    நிலை FFI_TYPE_SINT8:
	    நிலை FFI_TYPE_UINT8:
	      avalue[i] = (((எழுத்து *)pst) + OFS_INT8);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_SINT16:
	    நிலை FFI_TYPE_UINT16:
	      avalue[i] = (((எழுத்து *)pst) + OFS_INT16);
	      நிறுத்து;
  
	    நிலை FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      நிறுத்து;

	    இயல்பு:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      இல்லையெனில் எனில் (z == அளவு(முழு))
	{
#எனில் defined(__SH4__)
	  எனில் ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      எனில் (freg++ < NFREGARG)
		தொடர்;
	    }
	  இல்லையெனில்
#endif
	    {
	      எனில் (greg++ < NGREGARG)
		தொடர்;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#எனில் defined(__SH4__)
      இல்லையெனில் எனில் ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  எனில் (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      தொடர்;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      இல்லையெனில்
	{
	  முழு n = (z + அளவு (முழு) - 1) / அளவு (முழு);
	  எனில் (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      தொடர்;
	    }
#எனில் (! defined(__SH4__))
	  இல்லையெனில் எனில் (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      தொடர்;
	    }
#endif
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  திரும்பு return_type (cif->rtype);
}
