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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#define NGREGARG 4
#ଯଦି defined(__SH4__)
#define NFREGARG 8
#endif

#ଯଦି defined(__HITACHI__)
#define STRUCT_ମୂଲ୍ୟ_ADDRESS_WITH_ARG 1
#ଅନ୍ୟଥା
#define STRUCT_ମୂଲ୍ୟ_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
simple_type (ffi_type *arg)
{
  ଯଦି (arg->type != FFI_TYPE_STRUCT)
    ଫେରନ୍ତୁ arg->type;
  ଅନ୍ୟଥା ଯଦି (arg->elements[1])
    ଫେରନ୍ତୁ FFI_TYPE_STRUCT;

  ଫେରନ୍ତୁ simple_type (arg->elements[0]);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
return_type (ffi_type *arg)
{
  unsigned short type;

  ଯଦି (arg->type != FFI_TYPE_STRUCT)
    ଫେରନ୍ତୁ arg->type;

  type = simple_type (arg->elements[0]);
  ଯଦି (! arg->elements[1])
    {
      ବାଛ (type)
	{
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT8:
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	  ଫେରନ୍ତୁ FFI_TYPE_INT;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  ଫେରନ୍ତୁ type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  ଯଦି (arg->size <= 2 * ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
      ffi_type *e;

      ଯେତେବେଳେ ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  ବାଛ (type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT32:
	    ସ୍ଥିତି FFI_TYPE_UINT32:
	    ସ୍ଥିତି FFI_TYPE_INT:
	    ସ୍ଥିତି FFI_TYPE_FLOAT:
	      ଫେରନ୍ତୁ FFI_TYPE_UINT64;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      ଭାଙ୍ଗ;
	    }
	}
    }

  ଫେରନ୍ତୁ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ଶୂନ୍ୟ ffi_prep_args(ଅକ୍ଷର *ଗଦା, extended_cif *ecif)
{
  ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପଞ୍ଜିକରଣ ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmp;
  ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା avn;
  ପଞ୍ଜିକରଣ ଶୂନ୍ୟ **p_argv;
  ପଞ୍ଜିକରଣ ଅକ୍ଷର *argp;
  ପଞ୍ଜିକରଣ ffi_type **p_arg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା greg, ireg;
#ଯଦି defined(__SH4__)
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା freg = 0;
#endif

  tmp = 0;
  argp = ଗଦା;

  ଯଦି (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(ଶୂନ୍ୟ **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_ମୂଲ୍ୟ_ADDRESS_WITH_ARG ? 1 : 0;
    }
  ଅନ୍ୟଥା
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ପାଇଁ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      ଯଦି (z < ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  ଯଦି (greg++ >= NGREGARG)
	    ଜାରି;

	  z = ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  ବାଛ ((*p_arg)->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT8 *)(* p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_UINT8:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT8 *)(* p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_SINT16:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT16 *)(* p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_UINT16:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT16 *)(* p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_STRUCT:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT32 *)(* p_argv);
	      ଭାଙ୍ଗ;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      ଅନ୍ୟଥା ଯଦି (z == ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
#ଯଦି defined(__SH4__)
	  ଯଦି ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ଯଦି (freg++ >= NFREGARG)
		ଜାରି;
	    }
	  ଅନ୍ୟଥା
#endif
	    {
	      ଯଦି (greg++ >= NGREGARG)
		ଜାରି;
	    }
	  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ଯଦି defined(__SH4__)
      ଅନ୍ୟଥା ଯଦି ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ଯଦି (freg + 1 >= NFREGARG)
	    ଜାରି;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      ଅନ୍ୟଥା
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (z + ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) / ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
#ଯଦି defined(__SH4__)
	  ଯଦି (greg + n - 1 >= NGREGARG)
	    ଜାରି;
#ଅନ୍ୟଥା
	  ଯଦି (greg >= NGREGARG)
	    ଜାରି;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#ଯଦି defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  ପାଇଁ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      ଯଦି (z < ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  ଯଦି (greg++ < NGREGARG)
	    ଜାରି;

	  z = ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  ବାଛ ((*p_arg)->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT8 *)(* p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_UINT8:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT8 *)(* p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_SINT16:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT16 *)(* p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_UINT16:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT16 *)(* p_argv);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_STRUCT:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT32 *)(* p_argv);
	      ଭାଙ୍ଗ;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      ଅନ୍ୟଥା ଯଦି (z == ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
#ଯଦି defined(__SH4__)
	  ଯଦି ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ଯଦି (freg++ < NFREGARG)
		ଜାରି;
	    }
	  ଅନ୍ୟଥା
#endif
	    {
	      ଯଦି (greg++ < NGREGARG)
		ଜାରି;
	    }
	  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ଯଦି defined(__SH4__)
      ଅନ୍ୟଥା ଯଦି ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ଯଦି (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ଜାରି;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      ଅନ୍ୟଥା
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (z + ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) / ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  ଯଦି (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ଜାରି;
	    }
#ଯଦି (! defined(__SH4__))
	  ଅନ୍ୟଥା ଯଦି (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      ଜାରି;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	}
    }

  ଫେରନ୍ତୁ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, type;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, m;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା greg;
#ଯଦି defined(__SH4__)
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_ମୂଲ୍ୟ_ADDRESS_WITH_ARG) ? 1 : 0;

#ଯଦି defined(__SH4__)
  ପାଇଁ (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      ବାଛ (type)
	{
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  ଯଦି (freg >= NFREGARG)
	    ଜାରି;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  ଯଦି ((freg + 1) >= NFREGARG)
	    ଜାରି;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ଭାଙ୍ଗ;
	      
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  size = (cif->arg_types)[i]->size;
	  n = (size + ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) / ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  ଯଦି (greg + n - 1 >= NGREGARG)
		ଜାରି;
	  greg += n;
	  ପାଇଁ (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  ଭାଙ୍ଗ;
	}
    }
#ଅନ୍ୟଥା
  ପାଇଁ (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) / ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
      ଯଦି (greg >= NGREGARG)
	ଜାରି;
      ଅନ୍ୟଥା ଯଦି (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      ପାଇଁ (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      cif->flags += (unsigned) (return_type (cif->rtype)) << 24;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_VOID:
    ସ୍ଥିତି FFI_TYPE_FLOAT:
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
      cif->flags += (unsigned) cif->rtype->type << 24;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      cif->flags += FFI_TYPE_INT << 24;
      ଭାଙ୍ଗ;
    }

  ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV(ଶୂନ୍ୟ (*)(ଅକ୍ଷର *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ));

ଶୂନ୍ୟ ffi_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  ଅନ୍ୟଥା ଯଦି ((rvalue == ଖାଲି) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  ବାଛ (cif->abi) 
    {
    ସ୍ଥିତି FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT(0);
      ଭାଙ୍ଗ;
    }

  ଯଦି (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_SYSV (ଶୂନ୍ୟ);
#ଯଦି defined(__SH4__)
ବାହ୍ୟ ଶୂନ୍ୟ __ic_invalidate (ଶୂନ୍ୟ *ଧାଡ଼ି);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tramp;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା insn;

  ଯଦି (cif->abi != FFI_SYSV)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  tramp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#ଅନ୍ୟଥା
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(ଶୂନ୍ୟ **) &tramp[2] = (ଶୂନ୍ୟ *)codeloc;          /* ctx */
  *(ଶୂନ୍ୟ **) &tramp[3] = (ଶୂନ୍ୟ *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#ଯଦି defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  ଫେରନ୍ତୁ FFI_OK;
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
#ଅନ୍ୟଥା
#define OFS_INT8	3
#define OFS_INT16	2
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_helper_SYSV (ffi_closure *closure, ଶୂନ୍ୟ *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  ଶୂନ୍ୟ **avalue;
  ffi_type **p_arg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, avn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ireg, greg = 0;
#ଯଦି defined(__SH4__)
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * ଆକାର(ଶୂନ୍ୟ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_ମୂଲ୍ୟ_ADDRESS_WITH_ARG)
    {
      rvalue = (ଶୂନ୍ୟ *) *pgr++;
      ireg = 1;
    }
  ଅନ୍ୟଥା
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  ପାଇଁ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      ଯଦି (z < ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  ଯଦି (greg++ >= NGREGARG)
	    ଜାରି;

	  z = ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  ବାଛ ((*p_arg)->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	    ସ୍ଥିତି FFI_TYPE_UINT8:
	      avalue[i] = (((ଅକ୍ଷର *)pgr) + OFS_INT8);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_SINT16:
	    ସ୍ଥିତି FFI_TYPE_UINT16:
	      avalue[i] = (((ଅକ୍ଷର *)pgr) + OFS_INT16);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      ଭାଙ୍ଗ;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      ଅନ୍ୟଥା ଯଦି (z == ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
#ଯଦି defined(__SH4__)
	  ଯଦି ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ଯଦି (freg++ >= NFREGARG)
		ଜାରି;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ଅନ୍ୟଥା
#endif
	    {
	      ଯଦି (greg++ >= NGREGARG)
		ଜାରି;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#ଯଦି defined(__SH4__)
      ଅନ୍ୟଥା ଯଦି ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ଯଦି (freg + 1 >= NFREGARG)
	    ଜାରି;
	  ଯଦି (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      ଅନ୍ୟଥା
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (z + ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) / ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
#ଯଦି defined(__SH4__)
	  ଯଦି (greg + n - 1 >= NGREGARG)
	    ଜାରି;
#ଅନ୍ୟଥା
	  ଯଦି (greg >= NGREGARG)
	    ଜାରି;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#ଯଦି defined(__SH4__)
  freg = 0;
#endif

  ପାଇଁ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      ଯଦି (z < ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  ଯଦି (greg++ < NGREGARG)
	    ଜାରି;

	  z = ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  ବାଛ ((*p_arg)->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	    ସ୍ଥିତି FFI_TYPE_UINT8:
	      avalue[i] = (((ଅକ୍ଷର *)pst) + OFS_INT8);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_SINT16:
	    ସ୍ଥିତି FFI_TYPE_UINT16:
	      avalue[i] = (((ଅକ୍ଷର *)pst) + OFS_INT16);
	      ଭାଙ୍ଗ;
  
	    ସ୍ଥିତି FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      ଭାଙ୍ଗ;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      ଅନ୍ୟଥା ଯଦି (z == ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
#ଯଦି defined(__SH4__)
	  ଯଦି ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ଯଦି (freg++ < NFREGARG)
		ଜାରି;
	    }
	  ଅନ୍ୟଥା
#endif
	    {
	      ଯଦି (greg++ < NGREGARG)
		ଜାରି;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#ଯଦି defined(__SH4__)
      ଅନ୍ୟଥା ଯଦି ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ଯଦି (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ଜାରି;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      ଅନ୍ୟଥା
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (z + ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) / ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  ଯଦି (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ଜାରି;
	    }
#ଯଦି (! defined(__SH4__))
	  ଅନ୍ୟଥା ଯଦି (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      ଜାରି;
	    }
#endif
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ଫେରନ୍ତୁ return_type (cif->rtype);
}
