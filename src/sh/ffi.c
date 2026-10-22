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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>

#define NGREGARG 4
#ಯದಿ defined(__SH4__)
#define NFREGARG 8
#endif

#ಯದಿ defined(__HITACHI__)
#define STRUCT_ಮೌಲ್ಯ_ADDRESS_WITH_ARG 1
#ಅನ್ಯಥಾ
#define STRUCT_ಮೌಲ್ಯ_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
simple_type (ffi_type *arg)
{
  ಯದಿ (arg->type != FFI_TYPE_STRUCT)
    ಪ್ರತ್ಯಾಗಮ arg->type;
  ಅನ್ಯಥಾ ಯದಿ (arg->elements[1])
    ಪ್ರತ್ಯಾಗಮ FFI_TYPE_STRUCT;

  ಪ್ರತ್ಯಾಗಮ simple_type (arg->elements[0]);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
return_type (ffi_type *arg)
{
  unsigned short type;

  ಯದಿ (arg->type != FFI_TYPE_STRUCT)
    ಪ್ರತ್ಯಾಗಮ arg->type;

  type = simple_type (arg->elements[0]);
  ಯದಿ (! arg->elements[1])
    {
      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	  ಪ್ರತ್ಯಾಗಮ FFI_TYPE_INT;

	ಸ್ವಭಾವ:
	  ಪ್ರತ್ಯಾಗಮ type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  ಯದಿ (arg->size <= 2 * ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
    {
      ಪೂರ್ಣಾಙ್ಕ i = 0;
      ffi_type *e;

      ಯಾವತ್ ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  ವಿಕಲ್ಪ (type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT32:
	    ಪಕ್ಷ FFI_TYPE_UINT32:
	    ಪಕ್ಷ FFI_TYPE_INT:
	    ಪಕ್ಷ FFI_TYPE_FLOAT:
	      ಪ್ರತ್ಯಾಗಮ FFI_TYPE_UINT64;

	    ಸ್ವಭಾವ:
	      ಭಙ್ಗ;
	    }
	}
    }

  ಪ್ರತ್ಯಾಗಮ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಶೂನ್ಯ ffi_prep_args(ವರ್ಣ *ರಾಶಿ, extended_cif *ecif)
{
  ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಪಞ್ಜಿಕಾ ಪೂರ್ಣಾಙ್ಕ tmp;
  ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ avn;
  ಪಞ್ಜಿಕಾ ಶೂನ್ಯ **p_argv;
  ಪಞ್ಜಿಕಾ ವರ್ಣ *argp;
  ಪಞ್ಜಿಕಾ ffi_type **p_arg;
  ಪೂರ್ಣಾಙ್ಕ greg, ireg;
#ಯದಿ defined(__SH4__)
  ಪೂರ್ಣಾಙ್ಕ freg = 0;
#endif

  tmp = 0;
  argp = ರಾಶಿ;

  ಯದಿ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_ಮೌಲ್ಯ_ADDRESS_WITH_ARG ? 1 : 0;
    }
  ಅನ್ಯಥಾ
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      ಯದಿ (z < ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
	{
	  ಯದಿ (greg++ >= NGREGARG)
	    ಅನುವರ್ತ;

	  z = ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ);
	  ವಿಕಲ್ಪ ((*p_arg)->type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	      *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT8 *)(* p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_UINT8:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT8 *)(* p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_SINT16:
	      *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT16 *)(* p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_UINT16:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT16 *)(* p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_STRUCT:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT32 *)(* p_argv);
	      ಭಙ್ಗ;

	    ಸ್ವಭಾವ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
	{
#ಯದಿ defined(__SH4__)
	  ಯದಿ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ಯದಿ (freg++ >= NFREGARG)
		ಅನುವರ್ತ;
	    }
	  ಅನ್ಯಥಾ
#endif
	    {
	      ಯದಿ (greg++ >= NGREGARG)
		ಅನುವರ್ತ;
	    }
	  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ಯದಿ defined(__SH4__)
      ಅನ್ಯಥಾ ಯದಿ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ಯದಿ (freg + 1 >= NFREGARG)
	    ಅನುವರ್ತ;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      ಅನ್ಯಥಾ
	{
	  ಪೂರ್ಣಾಙ್ಕ n = (z + ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - 1) / ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
#ಯದಿ defined(__SH4__)
	  ಯದಿ (greg + n - 1 >= NGREGARG)
	    ಅನುವರ್ತ;
#ಅನ್ಯಥಾ
	  ಯದಿ (greg >= NGREGARG)
	    ಅನುವರ್ತ;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#ಯದಿ defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      ಯದಿ (z < ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
	{
	  ಯದಿ (greg++ < NGREGARG)
	    ಅನುವರ್ತ;

	  z = ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ);
	  ವಿಕಲ್ಪ ((*p_arg)->type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	      *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT8 *)(* p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_UINT8:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT8 *)(* p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_SINT16:
	      *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT16 *)(* p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_UINT16:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT16 *)(* p_argv);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_STRUCT:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT32 *)(* p_argv);
	      ಭಙ್ಗ;

	    ಸ್ವಭಾವ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
	{
#ಯದಿ defined(__SH4__)
	  ಯದಿ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ಯದಿ (freg++ < NFREGARG)
		ಅನುವರ್ತ;
	    }
	  ಅನ್ಯಥಾ
#endif
	    {
	      ಯದಿ (greg++ < NGREGARG)
		ಅನುವರ್ತ;
	    }
	  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ಯದಿ defined(__SH4__)
      ಅನ್ಯಥಾ ಯದಿ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ಯದಿ (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ಅನುವರ್ತ;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      ಅನ್ಯಥಾ
	{
	  ಪೂರ್ಣಾಙ್ಕ n = (z + ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - 1) / ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	  ಯದಿ (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ಅನುವರ್ತ;
	    }
#ಯದಿ (! defined(__SH4__))
	  ಅನ್ಯಥಾ ಯದಿ (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      ಅನುವರ್ತ;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	}
    }

  ಪ್ರತ್ಯಾಗಮ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ಪೂರ್ಣಾಙ್ಕ size, type;
  ಪೂರ್ಣಾಙ್ಕ n, m;
  ಪೂರ್ಣಾಙ್ಕ greg;
#ಯದಿ defined(__SH4__)
  ಪೂರ್ಣಾಙ್ಕ freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_ಮೌಲ್ಯ_ADDRESS_WITH_ARG) ? 1 : 0;

#ಯದಿ defined(__SH4__)
  ಪ್ರತಿ (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  ಯದಿ (freg >= NFREGARG)
	    ಅನುವರ್ತ;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  ಯದಿ ((freg + 1) >= NFREGARG)
	    ಅನುವರ್ತ;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ಭಙ್ಗ;
	      
	ಸ್ವಭಾವ:
	  size = (cif->arg_types)[i]->size;
	  n = (size + ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - 1) / ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	  ಯದಿ (greg + n - 1 >= NGREGARG)
		ಅನುವರ್ತ;
	  greg += n;
	  ಪ್ರತಿ (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  ಭಙ್ಗ;
	}
    }
#ಅನ್ಯಥಾ
  ಪ್ರತಿ (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - 1) / ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
      ಯದಿ (greg >= NGREGARG)
	ಅನುವರ್ತ;
      ಅನ್ಯಥಾ ಯದಿ (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      ಪ್ರತಿ (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_STRUCT:
      cif->flags += (unsigned) (return_type (cif->rtype)) << 24;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_VOID:
    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
      cif->flags += (unsigned) cif->rtype->type << 24;
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      cif->flags += FFI_TYPE_INT << 24;
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ (*)(ವರ್ಣ *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  ಅನ್ಯಥಾ ಯದಿ ((rvalue == ರಿಕ್ತ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  ವಿಕಲ್ಪ (cif->abi) 
    {
    ಪಕ್ಷ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }

  ಯದಿ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_SYSV (ಶೂನ್ಯ);
#ಯದಿ defined(__SH4__)
ಬಾಹ್ಯ ಶೂನ್ಯ __ic_invalidate (ಶೂನ್ಯ *ಸಾಲು);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ *tramp;
  unsigned ಪೂರ್ಣಾಙ್ಕ insn;

  ಯದಿ (cif->abi != FFI_SYSV)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  tramp = (unsigned ಪೂರ್ಣಾಙ್ಕ *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#ಅನ್ಯಥಾ
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(ಶೂನ್ಯ **) &tramp[2] = (ಶೂನ್ಯ *)codeloc;          /* ctx */
  *(ಶೂನ್ಯ **) &tramp[3] = (ಶೂನ್ಯ *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#ಯದಿ defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
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
#ಅನ್ಯಥಾ
#define OFS_INT8	3
#define OFS_INT16	2
#endif

ಪೂರ್ಣಾಙ್ಕ
ffi_closure_helper_SYSV (ffi_closure *closure, ಶೂನ್ಯ *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  ಶೂನ್ಯ **avalue;
  ffi_type **p_arg;
  ಪೂರ್ಣಾಙ್ಕ i, avn;
  ಪೂರ್ಣಾಙ್ಕ ireg, greg = 0;
#ಯದಿ defined(__SH4__)
  ಪೂರ್ಣಾಙ್ಕ freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_ಮೌಲ್ಯ_ADDRESS_WITH_ARG)
    {
      rvalue = (ಶೂನ್ಯ *) *pgr++;
      ireg = 1;
    }
  ಅನ್ಯಥಾ
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಪ್ರತಿ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      ಯದಿ (z < ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
	{
	  ಯದಿ (greg++ >= NGREGARG)
	    ಅನುವರ್ತ;

	  z = ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ);
	  ವಿಕಲ್ಪ ((*p_arg)->type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	    ಪಕ್ಷ FFI_TYPE_UINT8:
	      avalue[i] = (((ವರ್ಣ *)pgr) + OFS_INT8);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_SINT16:
	    ಪಕ್ಷ FFI_TYPE_UINT16:
	      avalue[i] = (((ವರ್ಣ *)pgr) + OFS_INT16);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      ಭಙ್ಗ;

	    ಸ್ವಭಾವ:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
	{
#ಯದಿ defined(__SH4__)
	  ಯದಿ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ಯದಿ (freg++ >= NFREGARG)
		ಅನುವರ್ತ;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ಅನ್ಯಥಾ
#endif
	    {
	      ಯದಿ (greg++ >= NGREGARG)
		ಅನುವರ್ತ;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#ಯದಿ defined(__SH4__)
      ಅನ್ಯಥಾ ಯದಿ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ಯದಿ (freg + 1 >= NFREGARG)
	    ಅನುವರ್ತ;
	  ಯದಿ (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      ಅನ್ಯಥಾ
	{
	  ಪೂರ್ಣಾಙ್ಕ n = (z + ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - 1) / ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
#ಯದಿ defined(__SH4__)
	  ಯದಿ (greg + n - 1 >= NGREGARG)
	    ಅನುವರ್ತ;
#ಅನ್ಯಥಾ
	  ಯದಿ (greg >= NGREGARG)
	    ಅನುವರ್ತ;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#ಯದಿ defined(__SH4__)
  freg = 0;
#endif

  ಪ್ರತಿ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      ಯದಿ (z < ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
	{
	  ಯದಿ (greg++ < NGREGARG)
	    ಅನುವರ್ತ;

	  z = ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ);
	  ವಿಕಲ್ಪ ((*p_arg)->type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	    ಪಕ್ಷ FFI_TYPE_UINT8:
	      avalue[i] = (((ವರ್ಣ *)pst) + OFS_INT8);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_SINT16:
	    ಪಕ್ಷ FFI_TYPE_UINT16:
	      avalue[i] = (((ವರ್ಣ *)pst) + OFS_INT16);
	      ಭಙ್ಗ;
  
	    ಪಕ್ಷ FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      ಭಙ್ಗ;

	    ಸ್ವಭಾವ:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
	{
#ಯದಿ defined(__SH4__)
	  ಯದಿ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ಯದಿ (freg++ < NFREGARG)
		ಅನುವರ್ತ;
	    }
	  ಅನ್ಯಥಾ
#endif
	    {
	      ಯದಿ (greg++ < NGREGARG)
		ಅನುವರ್ತ;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#ಯದಿ defined(__SH4__)
      ಅನ್ಯಥಾ ಯದಿ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ಯದಿ (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ಅನುವರ್ತ;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      ಅನ್ಯಥಾ
	{
	  ಪೂರ್ಣಾಙ್ಕ n = (z + ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - 1) / ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	  ಯದಿ (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ಅನುವರ್ತ;
	    }
#ಯದಿ (! defined(__SH4__))
	  ಅನ್ಯಥಾ ಯದಿ (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      ಅನುವರ್ತ;
	    }
#endif
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ಪ್ರತ್ಯಾಗಮ return_type (cif->rtype);
}
