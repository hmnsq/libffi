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
#nếu defined(__SH4__)
#define NFREGARG 8
#endif

#nếu defined(__HITACHI__)
#define STRUCT_VALUE_ADDRESS_WITH_ARG 1
#không_thì
#define STRUCT_VALUE_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
static số_nguyên
simple_type (ffi_type *arg)
{
  nếu (arg->type != FFI_TYPE_STRUCT)
    trả_về arg->type;
  không_thì nếu (arg->elements[1])
    trả_về FFI_TYPE_STRUCT;

  trả_về simple_type (arg->elements[0]);
}

static số_nguyên
return_type (ffi_type *arg)
{
  unsigned short type;

  nếu (arg->type != FFI_TYPE_STRUCT)
    trả_về arg->type;

  type = simple_type (arg->elements[0]);
  nếu (! arg->elements[1])
    {
      switch (type)
	{
	case FFI_TYPE_SINT8:
	case FFI_TYPE_UINT8:
	case FFI_TYPE_SINT16:
	case FFI_TYPE_UINT16:
	case FFI_TYPE_SINT32:
	case FFI_TYPE_UINT32:
	  trả_về FFI_TYPE_INT;

	default:
	  trả_về type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  nếu (arg->size <= 2 * sizeof (số_nguyên))
    {
      số_nguyên i = 0;
      ffi_type *e;

      trong_khi ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  switch (type)
	    {
	    case FFI_TYPE_SINT32:
	    case FFI_TYPE_UINT32:
	    case FFI_TYPE_INT:
	    case FFI_TYPE_FLOAT:
	      trả_về FFI_TYPE_UINT64;

	    default:
	      break;
	    }
	}
    }

  trả_về FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

void ffi_prep_args(char *stack, extended_cif *ecif)
{
  register unsigned số_nguyên i;
  register số_nguyên tmp;
  register unsigned số_nguyên avn;
  register void **p_argv;
  register char *argp;
  register ffi_type **p_arg;
  số_nguyên greg, ireg;
#nếu defined(__SH4__)
  số_nguyên freg = 0;
#endif

  tmp = 0;
  argp = stack;

  nếu (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(void **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_VALUE_ADDRESS_WITH_ARG ? 1 : 0;
    }
  không_thì
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  cho (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      nếu (z < sizeof(số_nguyên))
	{
	  nếu (greg++ >= NGREGARG)
	    continue;

	  z = sizeof(số_nguyên);
	  switch ((*p_arg)->type)
	    {
	    case FFI_TYPE_SINT8:
	      *(signed số_nguyên *) argp = (signed số_nguyên)*(SINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT8:
	      *(unsigned số_nguyên *) argp = (unsigned số_nguyên)*(UINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_SINT16:
	      *(signed số_nguyên *) argp = (signed số_nguyên)*(SINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT16:
	      *(unsigned số_nguyên *) argp = (unsigned số_nguyên)*(UINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_STRUCT:
	      *(unsigned số_nguyên *) argp = (unsigned số_nguyên)*(UINT32 *)(* p_argv);
	      break;

	    default:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      không_thì nếu (z == sizeof(số_nguyên))
	{
#nếu defined(__SH4__)
	  nếu ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      nếu (freg++ >= NFREGARG)
		continue;
	    }
	  không_thì
#endif
	    {
	      nếu (greg++ >= NGREGARG)
		continue;
	    }
	  *(unsigned số_nguyên *) argp = (unsigned số_nguyên)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#nếu defined(__SH4__)
      không_thì nếu ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  nếu (freg + 1 >= NFREGARG)
	    continue;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      không_thì
	{
	  số_nguyên n = (z + sizeof (số_nguyên) - 1) / sizeof (số_nguyên);
#nếu defined(__SH4__)
	  nếu (greg + n - 1 >= NGREGARG)
	    continue;
#không_thì
	  nếu (greg >= NGREGARG)
	    continue;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * sizeof (số_nguyên);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#nếu defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  cho (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      nếu (z < sizeof(số_nguyên))
	{
	  nếu (greg++ < NGREGARG)
	    continue;

	  z = sizeof(số_nguyên);
	  switch ((*p_arg)->type)
	    {
	    case FFI_TYPE_SINT8:
	      *(signed số_nguyên *) argp = (signed số_nguyên)*(SINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT8:
	      *(unsigned số_nguyên *) argp = (unsigned số_nguyên)*(UINT8 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_SINT16:
	      *(signed số_nguyên *) argp = (signed số_nguyên)*(SINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_UINT16:
	      *(unsigned số_nguyên *) argp = (unsigned số_nguyên)*(UINT16 *)(* p_argv);
	      break;
  
	    case FFI_TYPE_STRUCT:
	      *(unsigned số_nguyên *) argp = (unsigned số_nguyên)*(UINT32 *)(* p_argv);
	      break;

	    default:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      không_thì nếu (z == sizeof(số_nguyên))
	{
#nếu defined(__SH4__)
	  nếu ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      nếu (freg++ < NFREGARG)
		continue;
	    }
	  không_thì
#endif
	    {
	      nếu (greg++ < NGREGARG)
		continue;
	    }
	  *(unsigned số_nguyên *) argp = (unsigned số_nguyên)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#nếu defined(__SH4__)
      không_thì nếu ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  nếu (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      continue;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      không_thì
	{
	  số_nguyên n = (z + sizeof (số_nguyên) - 1) / sizeof (số_nguyên);
	  nếu (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      continue;
	    }
#nếu (! defined(__SH4__))
	  không_thì nếu (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      continue;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * sizeof (số_nguyên);
	}
    }

  trả_về;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  số_nguyên i, j;
  số_nguyên size, type;
  số_nguyên n, m;
  số_nguyên greg;
#nếu defined(__SH4__)
  số_nguyên freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_VALUE_ADDRESS_WITH_ARG) ? 1 : 0;

#nếu defined(__SH4__)
  cho (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      switch (type)
	{
	case FFI_TYPE_FLOAT:
	  nếu (freg >= NFREGARG)
	    continue;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  break;

	case FFI_TYPE_DOUBLE:
	  nếu ((freg + 1) >= NFREGARG)
	    continue;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  break;
	      
	default:
	  size = (cif->arg_types)[i]->size;
	  n = (size + sizeof (số_nguyên) - 1) / sizeof (số_nguyên);
	  nếu (greg + n - 1 >= NGREGARG)
		continue;
	  greg += n;
	  cho (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  break;
	}
    }
#không_thì
  cho (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + sizeof (số_nguyên) - 1) / sizeof (số_nguyên);
      nếu (greg >= NGREGARG)
	continue;
      không_thì nếu (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      cho (m = 0; m < n; m++)
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

  trả_về FFI_OK;
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

  nếu (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  không_thì nếu ((rvalue == NULL) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  không_thì
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

  nếu (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

extern void ffi_closure_SYSV (void);
#nếu defined(__SH4__)
extern void __ic_invalidate (void *line);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      void (*fun)(ffi_cif*, void*, void**, void*),
		      void *user_data,
		      void *codeloc)
{
  unsigned số_nguyên *tramp;
  unsigned số_nguyên insn;

  nếu (cif->abi != FFI_SYSV)
    trả_về FFI_BAD_ABI;

  tramp = (unsigned số_nguyên *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#không_thì
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(void **) &tramp[2] = (void *)codeloc;          /* ctx */
  *(void **) &tramp[3] = (void *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#nếu defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  trả_về FFI_OK;
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
#không_thì
#define OFS_INT8	3
#define OFS_INT16	2
#endif

số_nguyên
ffi_closure_helper_SYSV (ffi_closure *closure, void *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  void **avalue;
  ffi_type **p_arg;
  số_nguyên i, avn;
  số_nguyên ireg, greg = 0;
#nếu defined(__SH4__)
  số_nguyên freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * sizeof(void *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  nếu (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_VALUE_ADDRESS_WITH_ARG)
    {
      rvalue = (void *) *pgr++;
      ireg = 1;
    }
  không_thì
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  cho (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      nếu (z < sizeof(số_nguyên))
	{
	  nếu (greg++ >= NGREGARG)
	    continue;

	  z = sizeof(số_nguyên);
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
      không_thì nếu (z == sizeof(số_nguyên))
	{
#nếu defined(__SH4__)
	  nếu ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      nếu (freg++ >= NFREGARG)
		continue;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  không_thì
#endif
	    {
	      nếu (greg++ >= NGREGARG)
		continue;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#nếu defined(__SH4__)
      không_thì nếu ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  nếu (freg + 1 >= NFREGARG)
	    continue;
	  nếu (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      không_thì
	{
	  số_nguyên n = (z + sizeof (số_nguyên) - 1) / sizeof (số_nguyên);
#nếu defined(__SH4__)
	  nếu (greg + n - 1 >= NGREGARG)
	    continue;
#không_thì
	  nếu (greg >= NGREGARG)
	    continue;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#nếu defined(__SH4__)
  freg = 0;
#endif

  cho (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      nếu (z < sizeof(số_nguyên))
	{
	  nếu (greg++ < NGREGARG)
	    continue;

	  z = sizeof(số_nguyên);
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
      không_thì nếu (z == sizeof(số_nguyên))
	{
#nếu defined(__SH4__)
	  nếu ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      nếu (freg++ < NFREGARG)
		continue;
	    }
	  không_thì
#endif
	    {
	      nếu (greg++ < NGREGARG)
		continue;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#nếu defined(__SH4__)
      không_thì nếu ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  nếu (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      continue;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      không_thì
	{
	  số_nguyên n = (z + sizeof (số_nguyên) - 1) / sizeof (số_nguyên);
	  nếu (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      continue;
	    }
#nếu (! defined(__SH4__))
	  không_thì nếu (greg < NGREGARG)
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
  trả_về return_type (cif->rtype);
}
