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
#ถ้า defined(__SH4__)
#define NFREGARG 8
#endif

#ถ้า defined(__HITACHI__)
#define STRUCT_VALUE_ADDRESS_WITH_ARG 1
#มิฉะนั้น
#define STRUCT_VALUE_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
static จำนวนเต็ม
simple_type (ffi_type *arg)
{
  ถ้า (arg->type != FFI_TYPE_STRUCT)
    คืน arg->type;
  มิฉะนั้น ถ้า (arg->elements[1])
    คืน FFI_TYPE_STRUCT;

  คืน simple_type (arg->elements[0]);
}

static จำนวนเต็ม
return_type (ffi_type *arg)
{
  unsigned short type;

  ถ้า (arg->type != FFI_TYPE_STRUCT)
    คืน arg->type;

  type = simple_type (arg->elements[0]);
  ถ้า (! arg->elements[1])
    {
      switch (type)
	{
	pak_sa FFI_TYPE_SINT8:
	pak_sa FFI_TYPE_UINT8:
	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT16:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT32:
	  คืน FFI_TYPE_INT;

	default:
	  คืน type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  ถ้า (arg->size <= 2 * parimaa_na (จำนวนเต็ม))
    {
      จำนวนเต็ม i = 0;
      ffi_type *e;

      ขณะ ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  switch (type)
	    {
	    pak_sa FFI_TYPE_SINT32:
	    pak_sa FFI_TYPE_UINT32:
	    pak_sa FFI_TYPE_INT:
	    pak_sa FFI_TYPE_FLOAT:
	      คืน FFI_TYPE_UINT64;

	    default:
	      bha_nga;
	    }
	}
    }

  คืน FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

void ffi_prep_args(var_na *stack, extended_cif *ecif)
{
  pa_njikaa unsigned จำนวนเต็ม i;
  pa_njikaa จำนวนเต็ม tmp;
  pa_njikaa unsigned จำนวนเต็ม avn;
  pa_njikaa void **p_argv;
  pa_njikaa var_na *argp;
  pa_njikaa ffi_type **p_arg;
  จำนวนเต็ม greg, ireg;
#ถ้า defined(__SH4__)
  จำนวนเต็ม freg = 0;
#endif

  tmp = 0;
  argp = stack;

  ถ้า (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(void **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_VALUE_ADDRESS_WITH_ARG ? 1 : 0;
    }
  มิฉะนั้น
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  สำหรับ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      ถ้า (z < parimaa_na(จำนวนเต็ม))
	{
	  ถ้า (greg++ >= NGREGARG)
	    continue;

	  z = parimaa_na(จำนวนเต็ม);
	  switch ((*p_arg)->type)
	    {
	    pak_sa FFI_TYPE_SINT8:
	      *(signed จำนวนเต็ม *) argp = (signed จำนวนเต็ม)*(SINT8 *)(* p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_UINT8:
	      *(unsigned จำนวนเต็ม *) argp = (unsigned จำนวนเต็ม)*(UINT8 *)(* p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_SINT16:
	      *(signed จำนวนเต็ม *) argp = (signed จำนวนเต็ม)*(SINT16 *)(* p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_UINT16:
	      *(unsigned จำนวนเต็ม *) argp = (unsigned จำนวนเต็ม)*(UINT16 *)(* p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_STRUCT:
	      *(unsigned จำนวนเต็ม *) argp = (unsigned จำนวนเต็ม)*(UINT32 *)(* p_argv);
	      bha_nga;

	    default:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      มิฉะนั้น ถ้า (z == parimaa_na(จำนวนเต็ม))
	{
#ถ้า defined(__SH4__)
	  ถ้า ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ถ้า (freg++ >= NFREGARG)
		continue;
	    }
	  มิฉะนั้น
#endif
	    {
	      ถ้า (greg++ >= NGREGARG)
		continue;
	    }
	  *(unsigned จำนวนเต็ม *) argp = (unsigned จำนวนเต็ม)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ถ้า defined(__SH4__)
      มิฉะนั้น ถ้า ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ถ้า (freg + 1 >= NFREGARG)
	    continue;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      มิฉะนั้น
	{
	  จำนวนเต็ม n = (z + parimaa_na (จำนวนเต็ม) - 1) / parimaa_na (จำนวนเต็ม);
#ถ้า defined(__SH4__)
	  ถ้า (greg + n - 1 >= NGREGARG)
	    continue;
#มิฉะนั้น
	  ถ้า (greg >= NGREGARG)
	    continue;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * parimaa_na (จำนวนเต็ม);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#ถ้า defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  สำหรับ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      ถ้า (z < parimaa_na(จำนวนเต็ม))
	{
	  ถ้า (greg++ < NGREGARG)
	    continue;

	  z = parimaa_na(จำนวนเต็ม);
	  switch ((*p_arg)->type)
	    {
	    pak_sa FFI_TYPE_SINT8:
	      *(signed จำนวนเต็ม *) argp = (signed จำนวนเต็ม)*(SINT8 *)(* p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_UINT8:
	      *(unsigned จำนวนเต็ม *) argp = (unsigned จำนวนเต็ม)*(UINT8 *)(* p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_SINT16:
	      *(signed จำนวนเต็ม *) argp = (signed จำนวนเต็ม)*(SINT16 *)(* p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_UINT16:
	      *(unsigned จำนวนเต็ม *) argp = (unsigned จำนวนเต็ม)*(UINT16 *)(* p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_STRUCT:
	      *(unsigned จำนวนเต็ม *) argp = (unsigned จำนวนเต็ม)*(UINT32 *)(* p_argv);
	      bha_nga;

	    default:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      มิฉะนั้น ถ้า (z == parimaa_na(จำนวนเต็ม))
	{
#ถ้า defined(__SH4__)
	  ถ้า ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ถ้า (freg++ < NFREGARG)
		continue;
	    }
	  มิฉะนั้น
#endif
	    {
	      ถ้า (greg++ < NGREGARG)
		continue;
	    }
	  *(unsigned จำนวนเต็ม *) argp = (unsigned จำนวนเต็ม)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ถ้า defined(__SH4__)
      มิฉะนั้น ถ้า ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ถ้า (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      continue;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      มิฉะนั้น
	{
	  จำนวนเต็ม n = (z + parimaa_na (จำนวนเต็ม) - 1) / parimaa_na (จำนวนเต็ม);
	  ถ้า (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      continue;
	    }
#ถ้า (! defined(__SH4__))
	  มิฉะนั้น ถ้า (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      continue;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * parimaa_na (จำนวนเต็ม);
	}
    }

  คืน;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  จำนวนเต็ม i, j;
  จำนวนเต็ม size, type;
  จำนวนเต็ม n, m;
  จำนวนเต็ม greg;
#ถ้า defined(__SH4__)
  จำนวนเต็ม freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_VALUE_ADDRESS_WITH_ARG) ? 1 : 0;

#ถ้า defined(__SH4__)
  สำหรับ (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      switch (type)
	{
	pak_sa FFI_TYPE_FLOAT:
	  ถ้า (freg >= NFREGARG)
	    continue;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  bha_nga;

	pak_sa FFI_TYPE_DOUBLE:
	  ถ้า ((freg + 1) >= NFREGARG)
	    continue;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  bha_nga;
	      
	default:
	  size = (cif->arg_types)[i]->size;
	  n = (size + parimaa_na (จำนวนเต็ม) - 1) / parimaa_na (จำนวนเต็ม);
	  ถ้า (greg + n - 1 >= NGREGARG)
		continue;
	  greg += n;
	  สำหรับ (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  bha_nga;
	}
    }
#มิฉะนั้น
  สำหรับ (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + parimaa_na (จำนวนเต็ม) - 1) / parimaa_na (จำนวนเต็ม);
      ถ้า (greg >= NGREGARG)
	continue;
      มิฉะนั้น ถ้า (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      สำหรับ (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_STRUCT:
      cif->flags += (unsigned) (return_type (cif->rtype)) << 24;
      bha_nga;

    pak_sa FFI_TYPE_VOID:
    pak_sa FFI_TYPE_FLOAT:
    pak_sa FFI_TYPE_DOUBLE:
    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
      cif->flags += (unsigned) cif->rtype->type << 24;
      bha_nga;

    default:
      cif->flags += FFI_TYPE_INT << 24;
      bha_nga;
    }

  คืน FFI_OK;
}

extern void ffi_call_SYSV(void (*)(var_na *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, void (*fn)(void));

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ถ้า (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  มิฉะนั้น ถ้า ((rvalue == NULL) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  มิฉะนั้น
    ecif.rvalue = rvalue;

  switch (cif->abi) 
    {
    pak_sa FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      bha_nga;
    default:
      FFI_ASSERT(0);
      bha_nga;
    }

  ถ้า (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

extern void ffi_closure_SYSV (void);
#ถ้า defined(__SH4__)
extern void __ic_invalidate (void *line);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      void (*fun)(ffi_cif*, void*, void**, void*),
		      void *user_data,
		      void *codeloc)
{
  unsigned จำนวนเต็ม *tramp;
  unsigned จำนวนเต็ม insn;

  ถ้า (cif->abi != FFI_SYSV)
    คืน FFI_BAD_ABI;

  tramp = (unsigned จำนวนเต็ม *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#มิฉะนั้น
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(void **) &tramp[2] = (void *)codeloc;          /* ctx */
  *(void **) &tramp[3] = (void *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#ถ้า defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  คืน FFI_OK;
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
#มิฉะนั้น
#define OFS_INT8	3
#define OFS_INT16	2
#endif

จำนวนเต็ม
ffi_closure_helper_SYSV (ffi_closure *closure, void *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  void **avalue;
  ffi_type **p_arg;
  จำนวนเต็ม i, avn;
  จำนวนเต็ม ireg, greg = 0;
#ถ้า defined(__SH4__)
  จำนวนเต็ม freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * parimaa_na(void *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ถ้า (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_VALUE_ADDRESS_WITH_ARG)
    {
      rvalue = (void *) *pgr++;
      ireg = 1;
    }
  มิฉะนั้น
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  สำหรับ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      ถ้า (z < parimaa_na(จำนวนเต็ม))
	{
	  ถ้า (greg++ >= NGREGARG)
	    continue;

	  z = parimaa_na(จำนวนเต็ม);
	  switch ((*p_arg)->type)
	    {
	    pak_sa FFI_TYPE_SINT8:
	    pak_sa FFI_TYPE_UINT8:
	      avalue[i] = (((var_na *)pgr) + OFS_INT8);
	      bha_nga;
  
	    pak_sa FFI_TYPE_SINT16:
	    pak_sa FFI_TYPE_UINT16:
	      avalue[i] = (((var_na *)pgr) + OFS_INT16);
	      bha_nga;
  
	    pak_sa FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      bha_nga;

	    default:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      มิฉะนั้น ถ้า (z == parimaa_na(จำนวนเต็ม))
	{
#ถ้า defined(__SH4__)
	  ถ้า ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ถ้า (freg++ >= NFREGARG)
		continue;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  มิฉะนั้น
#endif
	    {
	      ถ้า (greg++ >= NGREGARG)
		continue;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#ถ้า defined(__SH4__)
      มิฉะนั้น ถ้า ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ถ้า (freg + 1 >= NFREGARG)
	    continue;
	  ถ้า (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      มิฉะนั้น
	{
	  จำนวนเต็ม n = (z + parimaa_na (จำนวนเต็ม) - 1) / parimaa_na (จำนวนเต็ม);
#ถ้า defined(__SH4__)
	  ถ้า (greg + n - 1 >= NGREGARG)
	    continue;
#มิฉะนั้น
	  ถ้า (greg >= NGREGARG)
	    continue;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#ถ้า defined(__SH4__)
  freg = 0;
#endif

  สำหรับ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      ถ้า (z < parimaa_na(จำนวนเต็ม))
	{
	  ถ้า (greg++ < NGREGARG)
	    continue;

	  z = parimaa_na(จำนวนเต็ม);
	  switch ((*p_arg)->type)
	    {
	    pak_sa FFI_TYPE_SINT8:
	    pak_sa FFI_TYPE_UINT8:
	      avalue[i] = (((var_na *)pst) + OFS_INT8);
	      bha_nga;
  
	    pak_sa FFI_TYPE_SINT16:
	    pak_sa FFI_TYPE_UINT16:
	      avalue[i] = (((var_na *)pst) + OFS_INT16);
	      bha_nga;
  
	    pak_sa FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      bha_nga;

	    default:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      มิฉะนั้น ถ้า (z == parimaa_na(จำนวนเต็ม))
	{
#ถ้า defined(__SH4__)
	  ถ้า ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ถ้า (freg++ < NFREGARG)
		continue;
	    }
	  มิฉะนั้น
#endif
	    {
	      ถ้า (greg++ < NGREGARG)
		continue;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#ถ้า defined(__SH4__)
      มิฉะนั้น ถ้า ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ถ้า (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      continue;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      มิฉะนั้น
	{
	  จำนวนเต็ม n = (z + parimaa_na (จำนวนเต็ม) - 1) / parimaa_na (จำนวนเต็ม);
	  ถ้า (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      continue;
	    }
#ถ้า (! defined(__SH4__))
	  มิฉะนั้น ถ้า (greg < NGREGARG)
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
  คืน return_type (cif->rtype);
}
