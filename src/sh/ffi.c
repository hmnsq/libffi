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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>

#define NGREGARG 4
#જો defined(__SH4__)
#define NFREGARG 8
#endif

#જો defined(__HITACHI__)
#define STRUCT_કિંમત_ADDRESS_WITH_ARG 1
#નહીંતર
#define STRUCT_કિંમત_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
સ્થિર પૂર્ણાંક
simple_type (ffi_type *arg)
{
  જો (arg->type != FFI_TYPE_STRUCT)
    પાછા arg->type;
  નહીંતર જો (arg->elements[1])
    પાછા FFI_TYPE_STRUCT;

  પાછા simple_type (arg->elements[0]);
}

સ્થિર પૂર્ણાંક
return_type (ffi_type *arg)
{
  unsigned short type;

  જો (arg->type != FFI_TYPE_STRUCT)
    પાછા arg->type;

  type = simple_type (arg->elements[0]);
  જો (! arg->elements[1])
    {
      પસંદગી (type)
	{
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT8:
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT16:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT32:
	  પાછા FFI_TYPE_INT;

	મૂળભૂત:
	  પાછા type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  જો (arg->size <= 2 * કદ (પૂર્ણાંક))
    {
      પૂર્ણાંક i = 0;
      ffi_type *e;

      જ્યારે ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  પસંદગી (type)
	    {
	    સ્થિતિ FFI_TYPE_SINT32:
	    સ્થિતિ FFI_TYPE_UINT32:
	    સ્થિતિ FFI_TYPE_INT:
	    સ્થિતિ FFI_TYPE_FLOAT:
	      પાછા FFI_TYPE_UINT64;

	    મૂળભૂત:
	      તોડો;
	    }
	}
    }

  પાછા FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

શૂન્ય ffi_prep_args(અક્ષર *ઢગલો, extended_cif *ecif)
{
  નોંધણી unsigned પૂર્ણાંક i;
  નોંધણી પૂર્ણાંક tmp;
  નોંધણી unsigned પૂર્ણાંક avn;
  નોંધણી શૂન્ય **p_argv;
  નોંધણી અક્ષર *argp;
  નોંધણી ffi_type **p_arg;
  પૂર્ણાંક greg, ireg;
#જો defined(__SH4__)
  પૂર્ણાંક freg = 0;
#endif

  tmp = 0;
  argp = ઢગલો;

  જો (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(શૂન્ય **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_કિંમત_ADDRESS_WITH_ARG ? 1 : 0;
    }
  નહીંતર
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  માટે (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      જો (z < કદ(પૂર્ણાંક))
	{
	  જો (greg++ >= NGREGARG)
	    ચાલુ;

	  z = કદ(પૂર્ણાંક);
	  પસંદગી ((*p_arg)->type)
	    {
	    સ્થિતિ FFI_TYPE_SINT8:
	      *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT8 *)(* p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_UINT8:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT8 *)(* p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_SINT16:
	      *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT16 *)(* p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_UINT16:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT16 *)(* p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_STRUCT:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT32 *)(* p_argv);
	      તોડો;

	    મૂળભૂત:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      નહીંતર જો (z == કદ(પૂર્ણાંક))
	{
#જો defined(__SH4__)
	  જો ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      જો (freg++ >= NFREGARG)
		ચાલુ;
	    }
	  નહીંતર
#endif
	    {
	      જો (greg++ >= NGREGARG)
		ચાલુ;
	    }
	  *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#જો defined(__SH4__)
      નહીંતર જો ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  જો (freg + 1 >= NFREGARG)
	    ચાલુ;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      નહીંતર
	{
	  પૂર્ણાંક n = (z + કદ (પૂર્ણાંક) - 1) / કદ (પૂર્ણાંક);
#જો defined(__SH4__)
	  જો (greg + n - 1 >= NGREGARG)
	    ચાલુ;
#નહીંતર
	  જો (greg >= NGREGARG)
	    ચાલુ;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * કદ (પૂર્ણાંક);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#જો defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  માટે (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      જો (z < કદ(પૂર્ણાંક))
	{
	  જો (greg++ < NGREGARG)
	    ચાલુ;

	  z = કદ(પૂર્ણાંક);
	  પસંદગી ((*p_arg)->type)
	    {
	    સ્થિતિ FFI_TYPE_SINT8:
	      *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT8 *)(* p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_UINT8:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT8 *)(* p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_SINT16:
	      *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT16 *)(* p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_UINT16:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT16 *)(* p_argv);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_STRUCT:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT32 *)(* p_argv);
	      તોડો;

	    મૂળભૂત:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      નહીંતર જો (z == કદ(પૂર્ણાંક))
	{
#જો defined(__SH4__)
	  જો ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      જો (freg++ < NFREGARG)
		ચાલુ;
	    }
	  નહીંતર
#endif
	    {
	      જો (greg++ < NGREGARG)
		ચાલુ;
	    }
	  *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#જો defined(__SH4__)
      નહીંતર જો ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  જો (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ચાલુ;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      નહીંતર
	{
	  પૂર્ણાંક n = (z + કદ (પૂર્ણાંક) - 1) / કદ (પૂર્ણાંક);
	  જો (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ચાલુ;
	    }
#જો (! defined(__SH4__))
	  નહીંતર જો (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      ચાલુ;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * કદ (પૂર્ણાંક);
	}
    }

  પાછા;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  પૂર્ણાંક i, j;
  પૂર્ણાંક size, type;
  પૂર્ણાંક n, m;
  પૂર્ણાંક greg;
#જો defined(__SH4__)
  પૂર્ણાંક freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_કિંમત_ADDRESS_WITH_ARG) ? 1 : 0;

#જો defined(__SH4__)
  માટે (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      પસંદગી (type)
	{
	સ્થિતિ FFI_TYPE_FLOAT:
	  જો (freg >= NFREGARG)
	    ચાલુ;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	  જો ((freg + 1) >= NFREGARG)
	    ચાલુ;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  તોડો;
	      
	મૂળભૂત:
	  size = (cif->arg_types)[i]->size;
	  n = (size + કદ (પૂર્ણાંક) - 1) / કદ (પૂર્ણાંક);
	  જો (greg + n - 1 >= NGREGARG)
		ચાલુ;
	  greg += n;
	  માટે (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  તોડો;
	}
    }
#નહીંતર
  માટે (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + કદ (પૂર્ણાંક) - 1) / કદ (પૂર્ણાંક);
      જો (greg >= NGREGARG)
	ચાલુ;
      નહીંતર જો (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      માટે (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_STRUCT:
      cif->flags += (unsigned) (return_type (cif->rtype)) << 24;
      તોડો;

    સ્થિતિ FFI_TYPE_VOID:
    સ્થિતિ FFI_TYPE_FLOAT:
    સ્થિતિ FFI_TYPE_DOUBLE:
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
      cif->flags += (unsigned) cif->rtype->type << 24;
      તોડો;

    મૂળભૂત:
      cif->flags += FFI_TYPE_INT << 24;
      તોડો;
    }

  પાછા FFI_OK;
}

બાહ્ય શૂન્ય ffi_call_SYSV(શૂન્ય (*)(અક્ષર *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, શૂન્ય (*fn)(શૂન્ય));

શૂન્ય ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  જો (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  નહીંતર જો ((rvalue == ખાલી) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  નહીંતર
    ecif.rvalue = rvalue;

  પસંદગી (cif->abi) 
    {
    સ્થિતિ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      તોડો;
    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
    }

  જો (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

બાહ્ય શૂન્ય ffi_closure_SYSV (શૂન્ય);
#જો defined(__SH4__)
બાહ્ય શૂન્ય __ic_invalidate (શૂન્ય *લીટી);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  unsigned પૂર્ણાંક *tramp;
  unsigned પૂર્ણાંક insn;

  જો (cif->abi != FFI_SYSV)
    પાછા FFI_BAD_ABI;

  tramp = (unsigned પૂર્ણાંક *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#નહીંતર
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(શૂન્ય **) &tramp[2] = (શૂન્ય *)codeloc;          /* ctx */
  *(શૂન્ય **) &tramp[3] = (શૂન્ય *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#જો defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  પાછા FFI_OK;
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
#નહીંતર
#define OFS_INT8	3
#define OFS_INT16	2
#endif

પૂર્ણાંક
ffi_closure_helper_SYSV (ffi_closure *closure, શૂન્ય *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  શૂન્ય **avalue;
  ffi_type **p_arg;
  પૂર્ણાંક i, avn;
  પૂર્ણાંક ireg, greg = 0;
#જો defined(__SH4__)
  પૂર્ણાંક freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * કદ(શૂન્ય *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  જો (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_કિંમત_ADDRESS_WITH_ARG)
    {
      rvalue = (શૂન્ય *) *pgr++;
      ireg = 1;
    }
  નહીંતર
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  માટે (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      જો (z < કદ(પૂર્ણાંક))
	{
	  જો (greg++ >= NGREGARG)
	    ચાલુ;

	  z = કદ(પૂર્ણાંક);
	  પસંદગી ((*p_arg)->type)
	    {
	    સ્થિતિ FFI_TYPE_SINT8:
	    સ્થિતિ FFI_TYPE_UINT8:
	      avalue[i] = (((અક્ષર *)pgr) + OFS_INT8);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_SINT16:
	    સ્થિતિ FFI_TYPE_UINT16:
	      avalue[i] = (((અક્ષર *)pgr) + OFS_INT16);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      તોડો;

	    મૂળભૂત:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      નહીંતર જો (z == કદ(પૂર્ણાંક))
	{
#જો defined(__SH4__)
	  જો ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      જો (freg++ >= NFREGARG)
		ચાલુ;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  નહીંતર
#endif
	    {
	      જો (greg++ >= NGREGARG)
		ચાલુ;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#જો defined(__SH4__)
      નહીંતર જો ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  જો (freg + 1 >= NFREGARG)
	    ચાલુ;
	  જો (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      નહીંતર
	{
	  પૂર્ણાંક n = (z + કદ (પૂર્ણાંક) - 1) / કદ (પૂર્ણાંક);
#જો defined(__SH4__)
	  જો (greg + n - 1 >= NGREGARG)
	    ચાલુ;
#નહીંતર
	  જો (greg >= NGREGARG)
	    ચાલુ;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#જો defined(__SH4__)
  freg = 0;
#endif

  માટે (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      જો (z < કદ(પૂર્ણાંક))
	{
	  જો (greg++ < NGREGARG)
	    ચાલુ;

	  z = કદ(પૂર્ણાંક);
	  પસંદગી ((*p_arg)->type)
	    {
	    સ્થિતિ FFI_TYPE_SINT8:
	    સ્થિતિ FFI_TYPE_UINT8:
	      avalue[i] = (((અક્ષર *)pst) + OFS_INT8);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_SINT16:
	    સ્થિતિ FFI_TYPE_UINT16:
	      avalue[i] = (((અક્ષર *)pst) + OFS_INT16);
	      તોડો;
  
	    સ્થિતિ FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      તોડો;

	    મૂળભૂત:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      નહીંતર જો (z == કદ(પૂર્ણાંક))
	{
#જો defined(__SH4__)
	  જો ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      જો (freg++ < NFREGARG)
		ચાલુ;
	    }
	  નહીંતર
#endif
	    {
	      જો (greg++ < NGREGARG)
		ચાલુ;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#જો defined(__SH4__)
      નહીંતર જો ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  જો (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ચાલુ;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      નહીંતર
	{
	  પૂર્ણાંક n = (z + કદ (પૂર્ણાંક) - 1) / કદ (પૂર્ણાંક);
	  જો (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ચાલુ;
	    }
#જો (! defined(__SH4__))
	  નહીંતર જો (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      ચાલુ;
	    }
#endif
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  પાછા return_type (cif->rtype);
}
