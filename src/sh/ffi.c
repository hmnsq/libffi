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
#如果 defined(__SH4__)
#define NFREGARG 8
#endif

#如果 defined(__HITACHI__)
#define STRUCT_VALUE_ADDRESS_WITH_ARG 1
#否则
#define STRUCT_VALUE_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
static 整数
simple_type (ffi_type *arg)
{
  如果 (arg->type != FFI_TYPE_STRUCT)
    返回 arg->type;
  否则 如果 (arg->elements[1])
    返回 FFI_TYPE_STRUCT;

  返回 simple_type (arg->elements[0]);
}

static 整数
return_type (ffi_type *arg)
{
  unsigned short type;

  如果 (arg->type != FFI_TYPE_STRUCT)
    返回 arg->type;

  type = simple_type (arg->elements[0]);
  如果 (! arg->elements[1])
    {
      switch (type)
	{
	case FFI_TYPE_SINT8:
	case FFI_TYPE_UINT8:
	case FFI_TYPE_SINT16:
	case FFI_TYPE_UINT16:
	case FFI_TYPE_SINT32:
	case FFI_TYPE_UINT32:
	  返回 FFI_TYPE_INT;

	default:
	  返回 type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  如果 (arg->size <= 2 * sizeof (整数))
    {
      整数 i = 0;
      ffi_type *e;

      当 ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  switch (type)
	    {
	    case FFI_TYPE_SINT32:
	    case FFI_TYPE_UINT32:
	    case FFI_TYPE_INT:
	    case FFI_TYPE_FLOAT:
	      返回 FFI_TYPE_UINT64;

	    default:
	      break;
	    }
	}
    }

  返回 FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

void ffi_prep_args(char *stack, extended_cif *ecif)
{
  register unsigned 整数 i;
  register 整数 tmp;
  register unsigned 整数 avn;
  register void **p_argv;
  register char *argp;
  register ffi_type **p_arg;
  整数 greg, ireg;
#如果 defined(__SH4__)
  整数 freg = 0;
#endif

  tmp = 0;
  argp = stack;

  如果 (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(void **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_VALUE_ADDRESS_WITH_ARG ? 1 : 0;
    }
  否则
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  为 (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      如果 (z < sizeof(整数))
	{
	  如果 (greg++ >= NGREGARG)
	    continue;

	  z = sizeof(整数);
	  switch ((*p_arg)->type)
	    {
	    case FFI_TYPE_SINT8:
	      *(signed 整数 *) argp = (signed 整数)*(SINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT8:
	      *(unsigned 整数 *) argp = (unsigned 整数)*(UINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_SINT16:
	      *(signed 整数 *) argp = (signed 整数)*(SINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT16:
	      *(unsigned 整数 *) argp = (unsigned 整数)*(UINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_STRUCT:
	      *(unsigned 整数 *) argp = (unsigned 整数)*(UINT32 *)(* p_argv);
	      break;

	    default:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      否则 如果 (z == sizeof(整数))
	{
#如果 defined(__SH4__)
	  如果 ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      如果 (freg++ >= NFREGARG)
		continue;
	    }
	  否则
#endif
	    {
	      如果 (greg++ >= NGREGARG)
		continue;
	    }
	  *(unsigned 整数 *) argp = (unsigned 整数)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#如果 defined(__SH4__)
      否则 如果 ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  如果 (freg + 1 >= NFREGARG)
	    continue;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      否则
	{
	  整数 n = (z + sizeof (整数) - 1) / sizeof (整数);
#如果 defined(__SH4__)
	  如果 (greg + n - 1 >= NGREGARG)
	    continue;
#否则
	  如果 (greg >= NGREGARG)
	    continue;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * sizeof (整数);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#如果 defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  为 (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      如果 (z < sizeof(整数))
	{
	  如果 (greg++ < NGREGARG)
	    continue;

	  z = sizeof(整数);
	  switch ((*p_arg)->type)
	    {
	    case FFI_TYPE_SINT8:
	      *(signed 整数 *) argp = (signed 整数)*(SINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT8:
	      *(unsigned 整数 *) argp = (unsigned 整数)*(UINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_SINT16:
	      *(signed 整数 *) argp = (signed 整数)*(SINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT16:
	      *(unsigned 整数 *) argp = (unsigned 整数)*(UINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_STRUCT:
	      *(unsigned 整数 *) argp = (unsigned 整数)*(UINT32 *)(* p_argv);
	      break;

	    default:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      否则 如果 (z == sizeof(整数))
	{
#如果 defined(__SH4__)
	  如果 ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      如果 (freg++ < NFREGARG)
		continue;
	    }
	  否则
#endif
	    {
	      如果 (greg++ < NGREGARG)
		continue;
	    }
	  *(unsigned 整数 *) argp = (unsigned 整数)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#如果 defined(__SH4__)
      否则 如果 ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  如果 (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      continue;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      否则
	{
	  整数 n = (z + sizeof (整数) - 1) / sizeof (整数);
	  如果 (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      continue;
	    }
#如果 (! defined(__SH4__))
	  否则 如果 (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      continue;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * sizeof (整数);
	}
    }

  返回;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  整数 i, j;
  整数 size, type;
  整数 n, m;
  整数 greg;
#如果 defined(__SH4__)
  整数 freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_VALUE_ADDRESS_WITH_ARG) ? 1 : 0;

#如果 defined(__SH4__)
  为 (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      switch (type)
	{
	case FFI_TYPE_FLOAT:
	  如果 (freg >= NFREGARG)
	    continue;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  break;

	case FFI_TYPE_DOUBLE:
	  如果 ((freg + 1) >= NFREGARG)
	    continue;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  break;
	      
	default:
	  size = (cif->arg_types)[i]->size;
	  n = (size + sizeof (整数) - 1) / sizeof (整数);
	  如果 (greg + n - 1 >= NGREGARG)
		continue;
	  greg += n;
	  为 (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  break;
	}
    }
#否则
  为 (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + sizeof (整数) - 1) / sizeof (整数);
      如果 (greg >= NGREGARG)
	continue;
      否则 如果 (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      为 (m = 0; m < n; m++)
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

  返回 FFI_OK;
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

  如果 (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  否则 如果 ((rvalue == NULL) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  否则
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

  如果 (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

extern void ffi_closure_SYSV (void);
#如果 defined(__SH4__)
extern void __ic_invalidate (void *line);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      void (*fun)(ffi_cif*, void*, void**, void*),
		      void *user_data,
		      void *codeloc)
{
  unsigned 整数 *tramp;
  unsigned 整数 insn;

  如果 (cif->abi != FFI_SYSV)
    返回 FFI_BAD_ABI;

  tramp = (unsigned 整数 *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#否则
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(void **) &tramp[2] = (void *)codeloc;          /* ctx */
  *(void **) &tramp[3] = (void *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#如果 defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  返回 FFI_OK;
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
#否则
#define OFS_INT8	3
#define OFS_INT16	2
#endif

整数
ffi_closure_helper_SYSV (ffi_closure *closure, void *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  void **avalue;
  ffi_type **p_arg;
  整数 i, avn;
  整数 ireg, greg = 0;
#如果 defined(__SH4__)
  整数 freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * sizeof(void *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  如果 (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_VALUE_ADDRESS_WITH_ARG)
    {
      rvalue = (void *) *pgr++;
      ireg = 1;
    }
  否则
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  为 (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      如果 (z < sizeof(整数))
	{
	  如果 (greg++ >= NGREGARG)
	    continue;

	  z = sizeof(整数);
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
      否则 如果 (z == sizeof(整数))
	{
#如果 defined(__SH4__)
	  如果 ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      如果 (freg++ >= NFREGARG)
		continue;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  否则
#endif
	    {
	      如果 (greg++ >= NGREGARG)
		continue;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#如果 defined(__SH4__)
      否则 如果 ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  如果 (freg + 1 >= NFREGARG)
	    continue;
	  如果 (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      否则
	{
	  整数 n = (z + sizeof (整数) - 1) / sizeof (整数);
#如果 defined(__SH4__)
	  如果 (greg + n - 1 >= NGREGARG)
	    continue;
#否则
	  如果 (greg >= NGREGARG)
	    continue;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#如果 defined(__SH4__)
  freg = 0;
#endif

  为 (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      如果 (z < sizeof(整数))
	{
	  如果 (greg++ < NGREGARG)
	    continue;

	  z = sizeof(整数);
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
      否则 如果 (z == sizeof(整数))
	{
#如果 defined(__SH4__)
	  如果 ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      如果 (freg++ < NFREGARG)
		continue;
	    }
	  否则
#endif
	    {
	      如果 (greg++ < NGREGARG)
		continue;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#如果 defined(__SH4__)
      否则 如果 ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  如果 (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      continue;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      否则
	{
	  整数 n = (z + sizeof (整数) - 1) / sizeof (整数);
	  如果 (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      continue;
	    }
#如果 (! defined(__SH4__))
	  否则 如果 (greg < NGREGARG)
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
  返回 return_type (cif->rtype);
}
