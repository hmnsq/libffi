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

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>

#define NGREGARG 4
#якщо defined(__SH4__)
#define NFREGARG 8
#endif

#якщо defined(__HITACHI__)
#define STRUCT_VALUE_ADDRESS_WITH_ARG 1
#інакше
#define STRUCT_VALUE_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
static ціле
simple_type (ffi_type *arg)
{
  якщо (arg->type != FFI_TYPE_STRUCT)
    повернути arg->type;
  інакше якщо (arg->elements[1])
    повернути FFI_TYPE_STRUCT;

  повернути simple_type (arg->elements[0]);
}

static ціле
return_type (ffi_type *arg)
{
  unsigned short type;

  якщо (arg->type != FFI_TYPE_STRUCT)
    повернути arg->type;

  type = simple_type (arg->elements[0]);
  якщо (! arg->elements[1])
    {
      switch (type)
	{
	case FFI_TYPE_SINT8:
	case FFI_TYPE_UINT8:
	case FFI_TYPE_SINT16:
	case FFI_TYPE_UINT16:
	case FFI_TYPE_SINT32:
	case FFI_TYPE_UINT32:
	  повернути FFI_TYPE_INT;

	default:
	  повернути type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  якщо (arg->size <= 2 * sizeof (ціле))
    {
      ціле i = 0;
      ffi_type *e;

      поки ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  switch (type)
	    {
	    case FFI_TYPE_SINT32:
	    case FFI_TYPE_UINT32:
	    case FFI_TYPE_INT:
	    case FFI_TYPE_FLOAT:
	      повернути FFI_TYPE_UINT64;

	    default:
	      break;
	    }
	}
    }

  повернути FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

void ffi_prep_args(char *stack, extended_cif *ecif)
{
  register unsigned ціле i;
  register ціле tmp;
  register unsigned ціле avn;
  register void **p_argv;
  register char *argp;
  register ffi_type **p_arg;
  ціле greg, ireg;
#якщо defined(__SH4__)
  ціле freg = 0;
#endif

  tmp = 0;
  argp = stack;

  якщо (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(void **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_VALUE_ADDRESS_WITH_ARG ? 1 : 0;
    }
  інакше
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  для (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      якщо (z < sizeof(ціле))
	{
	  якщо (greg++ >= NGREGARG)
	    continue;

	  z = sizeof(ціле);
	  switch ((*p_arg)->type)
	    {
	    case FFI_TYPE_SINT8:
	      *(signed ціле *) argp = (signed ціле)*(SINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT8:
	      *(unsigned ціле *) argp = (unsigned ціле)*(UINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_SINT16:
	      *(signed ціле *) argp = (signed ціле)*(SINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT16:
	      *(unsigned ціле *) argp = (unsigned ціле)*(UINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_STRUCT:
	      *(unsigned ціле *) argp = (unsigned ціле)*(UINT32 *)(* p_argv);
	      break;

	    default:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      інакше якщо (z == sizeof(ціле))
	{
#якщо defined(__SH4__)
	  якщо ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      якщо (freg++ >= NFREGARG)
		continue;
	    }
	  інакше
#endif
	    {
	      якщо (greg++ >= NGREGARG)
		continue;
	    }
	  *(unsigned ціле *) argp = (unsigned ціле)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#якщо defined(__SH4__)
      інакше якщо ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  якщо (freg + 1 >= NFREGARG)
	    continue;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      інакше
	{
	  ціле n = (z + sizeof (ціле) - 1) / sizeof (ціле);
#якщо defined(__SH4__)
	  якщо (greg + n - 1 >= NGREGARG)
	    continue;
#інакше
	  якщо (greg >= NGREGARG)
	    continue;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * sizeof (ціле);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#якщо defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  для (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      якщо (z < sizeof(ціле))
	{
	  якщо (greg++ < NGREGARG)
	    continue;

	  z = sizeof(ціле);
	  switch ((*p_arg)->type)
	    {
	    case FFI_TYPE_SINT8:
	      *(signed ціле *) argp = (signed ціле)*(SINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT8:
	      *(unsigned ціле *) argp = (unsigned ціле)*(UINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_SINT16:
	      *(signed ціле *) argp = (signed ціле)*(SINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT16:
	      *(unsigned ціле *) argp = (unsigned ціле)*(UINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_STRUCT:
	      *(unsigned ціле *) argp = (unsigned ціле)*(UINT32 *)(* p_argv);
	      break;

	    default:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      інакше якщо (z == sizeof(ціле))
	{
#якщо defined(__SH4__)
	  якщо ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      якщо (freg++ < NFREGARG)
		continue;
	    }
	  інакше
#endif
	    {
	      якщо (greg++ < NGREGARG)
		continue;
	    }
	  *(unsigned ціле *) argp = (unsigned ціле)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#якщо defined(__SH4__)
      інакше якщо ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  якщо (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      continue;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      інакше
	{
	  ціле n = (z + sizeof (ціле) - 1) / sizeof (ціле);
	  якщо (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      continue;
	    }
#якщо (! defined(__SH4__))
	  інакше якщо (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      continue;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * sizeof (ціле);
	}
    }

  повернути;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ціле i, j;
  ціле size, type;
  ціле n, m;
  ціле greg;
#якщо defined(__SH4__)
  ціле freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_VALUE_ADDRESS_WITH_ARG) ? 1 : 0;

#якщо defined(__SH4__)
  для (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      switch (type)
	{
	case FFI_TYPE_FLOAT:
	  якщо (freg >= NFREGARG)
	    continue;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  break;

	case FFI_TYPE_DOUBLE:
	  якщо ((freg + 1) >= NFREGARG)
	    continue;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  break;
	      
	default:
	  size = (cif->arg_types)[i]->size;
	  n = (size + sizeof (ціле) - 1) / sizeof (ціле);
	  якщо (greg + n - 1 >= NGREGARG)
		continue;
	  greg += n;
	  для (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  break;
	}
    }
#інакше
  для (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + sizeof (ціле) - 1) / sizeof (ціле);
      якщо (greg >= NGREGARG)
	continue;
      інакше якщо (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      для (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  switch (cif->rtype->type)
    {
    case FFI_TYPE_STRUCT:
      cif->flags += (unsigned) (return_type (cif->rtype)) << 24;
      break;

    case FFI_TYPE_VOID:
    case FFI_TYPE_FLOAT:
    case FFI_TYPE_DOUBLE:
    case FFI_TYPE_SINT64:
    case FFI_TYPE_UINT64:
      cif->flags += (unsigned) cif->rtype->type << 24;
      break;

    default:
      cif->flags += FFI_TYPE_INT << 24;
      break;
    }

  повернути FFI_OK;
}

extern void ffi_call_SYSV(void (*)(char *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, void (*fn)(void));

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  якщо (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  інакше якщо ((rvalue == NULL) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  інакше
    ecif.rvalue = rvalue;

  switch (cif->abi) 
    {
    case FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      break;
    default:
      FFI_ASSERT(0);
      break;
    }

  якщо (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

extern void ffi_closure_SYSV (void);
#якщо defined(__SH4__)
extern void __ic_invalidate (void *line);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      void (*fun)(ffi_cif*, void*, void**, void*),
		      void *user_data,
		      void *codeloc)
{
  unsigned ціле *tramp;
  unsigned ціле insn;

  якщо (cif->abi != FFI_SYSV)
    повернути FFI_BAD_ABI;

  tramp = (unsigned ціле *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#інакше
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(void **) &tramp[2] = (void *)codeloc;          /* ctx */
  *(void **) &tramp[3] = (void *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#якщо defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  повернути FFI_OK;
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
#інакше
#define OFS_INT8	3
#define OFS_INT16	2
#endif

ціле
ffi_closure_helper_SYSV (ffi_closure *closure, void *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  void **avalue;
  ffi_type **p_arg;
  ціле i, avn;
  ціле ireg, greg = 0;
#якщо defined(__SH4__)
  ціле freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * sizeof(void *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  якщо (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_VALUE_ADDRESS_WITH_ARG)
    {
      rvalue = (void *) *pgr++;
      ireg = 1;
    }
  інакше
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  для (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      якщо (z < sizeof(ціле))
	{
	  якщо (greg++ >= NGREGARG)
	    continue;

	  z = sizeof(ціле);
	  switch ((*p_arg)->type)
	    {
	    case FFI_TYPE_SINT8:
	    case FFI_TYPE_UINT8:
	      avalue[i] = (((char *)pgr) + OFS_INT8);
	      break;
  
	    case FFI_TYPE_SINT16:
	    case FFI_TYPE_UINT16:
	      avalue[i] = (((char *)pgr) + OFS_INT16);
	      break;
  
	    case FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      break;

	    default:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      інакше якщо (z == sizeof(ціле))
	{
#якщо defined(__SH4__)
	  якщо ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      якщо (freg++ >= NFREGARG)
		continue;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  інакше
#endif
	    {
	      якщо (greg++ >= NGREGARG)
		continue;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#якщо defined(__SH4__)
      інакше якщо ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  якщо (freg + 1 >= NFREGARG)
	    continue;
	  якщо (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      інакше
	{
	  ціле n = (z + sizeof (ціле) - 1) / sizeof (ціле);
#якщо defined(__SH4__)
	  якщо (greg + n - 1 >= NGREGARG)
	    continue;
#інакше
	  якщо (greg >= NGREGARG)
	    continue;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#якщо defined(__SH4__)
  freg = 0;
#endif

  для (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      якщо (z < sizeof(ціле))
	{
	  якщо (greg++ < NGREGARG)
	    continue;

	  z = sizeof(ціле);
	  switch ((*p_arg)->type)
	    {
	    case FFI_TYPE_SINT8:
	    case FFI_TYPE_UINT8:
	      avalue[i] = (((char *)pst) + OFS_INT8);
	      break;
  
	    case FFI_TYPE_SINT16:
	    case FFI_TYPE_UINT16:
	      avalue[i] = (((char *)pst) + OFS_INT16);
	      break;
  
	    case FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      break;

	    default:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      інакше якщо (z == sizeof(ціле))
	{
#якщо defined(__SH4__)
	  якщо ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      якщо (freg++ < NFREGARG)
		continue;
	    }
	  інакше
#endif
	    {
	      якщо (greg++ < NGREGARG)
		continue;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#якщо defined(__SH4__)
      інакше якщо ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  якщо (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      continue;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      інакше
	{
	  ціле n = (z + sizeof (ціле) - 1) / sizeof (ціле);
	  якщо (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      continue;
	    }
#якщо (! defined(__SH4__))
	  інакше якщо (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      continue;
	    }
#endif
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  повернути return_type (cif->rtype);
}
