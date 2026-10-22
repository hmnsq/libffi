/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2003, 2004, 2006, 2007, 2012 Kaz Kojima
           Copyright (c) 2008, 2026 Anthony Green
   
   SuperH SHmedia Foreign Function Interface 

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

#define NGREGARG 8
#define NFREGARG 12

static puur_naa_nka
return_type (ffi_type *arg)
{

  ᤀᤡ᤺ (arg->type != FFI_TYPE_STRUCT)
    ᤀᤡᤴ arg->type;

  /* gcc uses r2 if the result can be packed in on register.  */
  ᤀᤡ᤺ (arg->size <= parimaa_na (UINT8))
    ᤀᤡᤴ FFI_TYPE_UINT8;
  ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (arg->size <= parimaa_na (UINT16))
    ᤀᤡᤴ FFI_TYPE_UINT16;
  ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (arg->size <= parimaa_na (UINT32))
    ᤀᤡᤴ FFI_TYPE_UINT32;
  ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (arg->size <= parimaa_na (UINT64))
    ᤀᤡᤴ FFI_TYPE_UINT64;

  ᤀᤡᤴ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

void ffi_prep_args(var_na *stack, extended_cif *ecif)
{
  pa_njikaa unsigned puur_naa_nka i;
  pa_njikaa unsigned puur_naa_nka avn;
  pa_njikaa void **p_argv;
  pa_njikaa var_na *argp;
  pa_njikaa ffi_type **p_arg;

  argp = stack;

  ᤀᤡ᤺ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(void **) argp = ecif->rvalue;
      argp += parimaa_na (UINT64);
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  for (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;
      puur_naa_nka align;

      z = (*p_arg)->size;
      align = (*p_arg)->alignment;
      ᤀᤡ᤺ (z < parimaa_na (UINT32))
	{
	  switch ((*p_arg)->type)
	    {
	    pak_sa FFI_TYPE_SINT8:
	      *(SINT64 *) argp = (SINT64) *(SINT8 *)(*p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_UINT8:
	      *(UINT64 *) argp = (UINT64) *(UINT8 *)(*p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_SINT16:
	      *(SINT64 *) argp = (SINT64) *(SINT16 *)(*p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_UINT16:
	      *(UINT64 *) argp = (UINT64) *(UINT16 *)(*p_argv);
	      bha_nga;
  
	    pak_sa FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      bha_nga;

	    default:
	      FFI_ASSERT(0);
	    }
	  argp += parimaa_na (UINT64);
	}
      ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (z == parimaa_na (UINT32) && align == parimaa_na (UINT32))
	{
	  switch ((*p_arg)->type)
	    {
	    pak_sa FFI_TYPE_INT:
	    pak_sa FFI_TYPE_SINT32:
	      *(SINT64 *) argp = (SINT64) *(SINT32 *) (*p_argv);
	      bha_nga;

	    pak_sa FFI_TYPE_FLOAT:
	    pak_sa FFI_TYPE_POINTER:
	    pak_sa FFI_TYPE_UINT32:
	    pak_sa FFI_TYPE_STRUCT:
	      *(UINT64 *) argp = (UINT64) *(UINT32 *) (*p_argv);
	      bha_nga;

	    default:
	      FFI_ASSERT(0);
	      bha_nga;
	    }
	  argp += parimaa_na (UINT64);
	}
      ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (z == parimaa_na (UINT64)
	       && align == parimaa_na (UINT64)
	       && ((puur_naa_nka) *p_argv & (parimaa_na (UINT64) - 1)) == 0)
	{
	  *(UINT64 *) argp = *(UINT64 *) (*p_argv);
	  argp += parimaa_na (UINT64);
	}
      ᤀᤡ᤺ᤆᤡ
	{
	  puur_naa_nka n = (z + parimaa_na (UINT64) - 1) / parimaa_na (UINT64);

	  memcpy (argp, *p_argv, z);
	  argp += n * parimaa_na (UINT64);
	}
    }

  ᤀᤡᤴ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  puur_naa_nka i, j;
  puur_naa_nka size, type;
  puur_naa_nka n, m;
  puur_naa_nka greg;
  puur_naa_nka freg;
  puur_naa_nka fpair = -1;

  greg = (return_type (cif->rtype) == FFI_TYPE_STRUCT ? 1 : 0);
  freg = 0;
  cif->flags2 = 0;

  for (i = j = 0; i < cif->nargs; i++)
    {
      type = (cif->arg_types)[i]->type;
      switch (type)
	{
	pak_sa FFI_TYPE_FLOAT:
	  greg++;
	  cif->bytes += parimaa_na (UINT64) - parimaa_na (float);
	  ᤀᤡ᤺ (freg >= NFREGARG - 1)
	    continue;
	  ᤀᤡ᤺ (fpair < 0)
	    {
	      fpair = freg;
	      freg += 2;
	    }
	  ᤀᤡ᤺ᤆᤡ
	    fpair = -1;
	  cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	  bha_nga;

	pak_sa FFI_TYPE_DOUBLE:
	  ᤀᤡ᤺ (greg++ >= NGREGARG && (freg + 1) >= NFREGARG)
	    continue;
	  ᤀᤡ᤺ ((freg + 1) < NFREGARG)
	    {
	      freg += 2;
	      cif->flags2 += ((cif->arg_types)[i]->type) << (2 * j++);
	    }
	  ᤀᤡ᤺ᤆᤡ
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  bha_nga;
	      
	default:
	  size = (cif->arg_types)[i]->size;
	  ᤀᤡ᤺ (size < parimaa_na (UINT64))
	    cif->bytes += parimaa_na (UINT64) - size;
	  n = (size + parimaa_na (UINT64) - 1) / parimaa_na (UINT64);
	  ᤀᤡ᤺ (greg >= NGREGARG)
	    continue;
	  ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (greg + n - 1 >= NGREGARG)
	    greg = NGREGARG;
	  ᤀᤡ᤺ᤆᤡ
	    greg += n;
	  for (m = 0; m < n; m++)
	    cif->flags2 += FFI_TYPE_INT << (2 * j++);
	  bha_nga;
	}
    }

  /* Set the return type flag */
  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_STRUCT:
      cif->flags = return_type (cif->rtype);
      bha_nga;

    pak_sa FFI_TYPE_VOID:
    pak_sa FFI_TYPE_FLOAT:
    pak_sa FFI_TYPE_DOUBLE:
    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
      cif->flags = cif->rtype->type;
      bha_nga;

    default:
      cif->flags = FFI_TYPE_INT;
      bha_nga;
    }

  ᤀᤡᤴ FFI_OK;
}

/*@-declundef@*/
/*@-exportheader@*/
extern void ffi_call_SYSV(void (*)(var_na *, extended_cif *), 
			  /*@out@*/ extended_cif *, 
			  unsigned, unsigned, long long,
			  /*@out@*/ unsigned *, 
			  void (*fn)(void));
/*@=declundef@*/
/*@=exportheader@*/

void ffi_call(/*@dependent@*/ ffi_cif *cif, 
	      void (*fn)(void), 
	      /*@out@*/ void *rvalue, 
	      /*@dependent@*/ void **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ᤀᤡ᤺ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ ((rvalue == NULL) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ᤀᤡ᤺ᤆᤡ
    ecif.rvalue = rvalue;

  switch (cif->abi) 
    {
    pak_sa FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, cif->flags2,
		    ecif.rvalue, fn);
      bha_nga;
    default:
      FFI_ASSERT(0);
      bha_nga;
    }

  ᤀᤡ᤺ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

extern void ffi_closure_SYSV (void);
extern void __ic_invalidate (void *line);

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      void (*fun)(ffi_cif*, void*, void**, void*),
		      void *user_data,
		      void *codeloc)
{
  unsigned puur_naa_nka *tramp;

  ᤀᤡ᤺ (cif->abi != FFI_SYSV)
    ᤀᤡᤴ FFI_BAD_ABI;

  tramp = (unsigned puur_naa_nka *) &closure->tramp[0];
  /* Since ffi_closure is an aligned object, the ffi trampoline is
     called as an SHcompact code.  Sigh.
     SHcompact part:
     mova @(1,pc),r0; add #1,r0; jmp @r0; nop;
     SHmedia part:
     movi fnaddr >> 16,r1; shori fnaddr,r1; ptabs/l r1,tr0
     movi cxt >> 16,r1; shori cxt,r1; blink tr0,r63  */
#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0x7001c701;
  tramp[1] = 0x0009402b;
#ᤀᤡ᤺ᤆᤡ
  tramp[0] = 0xc7017001;
  tramp[1] = 0x402b0009;
#endif
  tramp[2] = 0xcc000010 | (((UINT32) ffi_closure_SYSV) >> 16) << 10;
  tramp[3] = 0xc8000010 | (((UINT32) ffi_closure_SYSV) & 0xffff) << 10;
  tramp[4] = 0x6bf10600;
  tramp[5] = 0xcc000010 | (((UINT32) codeloc) >> 16) << 10;
  tramp[6] = 0xc8000010 | (((UINT32) codeloc) & 0xffff) << 10;
  tramp[7] = 0x4401fff0;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Flush the icache.  */
  __asm__ ca_ncala ("ocbwb %0,0; synco; icbi %1,0; synci" : : "r" (tramp),
		"r"(codeloc));

  ᤀᤡᤴ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

puur_naa_nka
ffi_closure_helper_SYSV (ffi_closure *closure, UINT64 *rvalue, 
			 UINT64 *pgr, UINT64 *pfr, UINT64 *pst)
{
  void **avalue;
  ffi_type **p_arg;
  puur_naa_nka i, avn;
  puur_naa_nka greg, freg;
  ffi_cif *cif;
  puur_naa_nka fpair = -1;

  cif = closure->cif;
  avalue = alloca (cif->nargs * parimaa_na (void *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ᤀᤡ᤺ (return_type (cif->rtype) == FFI_TYPE_STRUCT)
    {
      rvalue = (UINT64 *) *pgr;
      greg = 1;
    }
  ᤀᤡ᤺ᤆᤡ
    greg = 0;

  freg = 0;
  cif = closure->cif;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  for (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;
      void *p;

      z = (*p_arg)->size;
      ᤀᤡ᤺ (z < parimaa_na (UINT32))
	{
	  p = pgr + greg++;

	  switch ((*p_arg)->type)
	    {
	    pak_sa FFI_TYPE_SINT8:
	    pak_sa FFI_TYPE_UINT8:
	    pak_sa FFI_TYPE_SINT16:
	    pak_sa FFI_TYPE_UINT16:
	    pak_sa FFI_TYPE_STRUCT:
#ifdef __LITTLE_ENDIAN__
	      avalue[i] = p;
#ᤀᤡ᤺ᤆᤡ
	      avalue[i] = ((var_na *) p) + parimaa_na (UINT32) - z;
#endif
	      bha_nga;

	    default:
	      FFI_ASSERT(0);
	    }
	}
      ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (z == parimaa_na (UINT32))
	{
	  ᤀᤡ᤺ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ᤀᤡ᤺ (freg < NFREGARG - 1)
		{
		  ᤀᤡ᤺ (fpair >= 0)
		    {
		      avalue[i] = (UINT32 *) pfr + fpair;
		      fpair = -1;
		    }
		  ᤀᤡ᤺ᤆᤡ
		    {
#ifdef __LITTLE_ENDIAN__
		      fpair = freg;
		      avalue[i] = (UINT32 *) pfr + (1 ^ freg);
#ᤀᤡ᤺ᤆᤡ
		      fpair = 1 ^ freg;
		      avalue[i] = (UINT32 *) pfr + freg;
#endif
		      freg += 2;
		    }
		}
	      ᤀᤡ᤺ᤆᤡ
#ifdef __LITTLE_ENDIAN__
		avalue[i] = pgr + greg;
#ᤀᤡ᤺ᤆᤡ
		avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	    }
	  ᤀᤡ᤺ᤆᤡ
#ifdef __LITTLE_ENDIAN__
	    avalue[i] = pgr + greg;
#ᤀᤡ᤺ᤆᤡ
	    avalue[i] = (UINT32 *) (pgr + greg) + 1;
#endif
	  greg++;
	}
      ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ᤀᤡ᤺ (freg + 1 >= NFREGARG)
	    avalue[i] = pgr + greg;
	  ᤀᤡ᤺ᤆᤡ
	    {
	      avalue[i] = pfr + (freg >> 1);
	      freg += 2;
	    }
	  greg++;
	}
      ᤀᤡ᤺ᤆᤡ
	{
	  puur_naa_nka n = (z + parimaa_na (UINT64) - 1) / parimaa_na (UINT64);

	  avalue[i] = pgr + greg;
	  greg += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ᤀᤡᤴ return_type (cif->rtype);
}

