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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>

#define NGREGARG 4
#എങ്കിൽ defined(__SH4__)
#define NFREGARG 8
#endif

#എങ്കിൽ defined(__HITACHI__)
#define STRUCT_മൂല്യം_ADDRESS_WITH_ARG 1
#അല്ലെങ്കിൽ
#define STRUCT_മൂല്യം_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
simple_type (ffi_type *arg)
{
  എങ്കിൽ (arg->type != FFI_TYPE_STRUCT)
    തിരികെ arg->type;
  അല്ലെങ്കിൽ എങ്കിൽ (arg->elements[1])
    തിരികെ FFI_TYPE_STRUCT;

  തിരികെ simple_type (arg->elements[0]);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
return_type (ffi_type *arg)
{
  unsigned short type;

  എങ്കിൽ (arg->type != FFI_TYPE_STRUCT)
    തിരികെ arg->type;

  type = simple_type (arg->elements[0]);
  എങ്കിൽ (! arg->elements[1])
    {
      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT8:
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT32:
	  തിരികെ FFI_TYPE_INT;

	സ്ഥിരസ്ഥിതി:
	  തിരികെ type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  എങ്കിൽ (arg->size <= 2 * വലുപ്പം (പൂർണ്ണസംഖ്യ))
    {
      പൂർണ്ണസംഖ്യ i = 0;
      ffi_type *e;

      ആകുമ്പോൾ ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  തിരഞ്ഞെടുപ്പ് (type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT32:
	    അവസ്ഥ FFI_TYPE_UINT32:
	    അവസ്ഥ FFI_TYPE_INT:
	    അവസ്ഥ FFI_TYPE_FLOAT:
	      തിരികെ FFI_TYPE_UINT64;

	    സ്ഥിരസ്ഥിതി:
	      നിർത്തുക;
	    }
	}
    }

  തിരികെ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ശൂന്യം ffi_prep_args(അക്ഷരം *അട്ടി, extended_cif *ecif)
{
  രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ i;
  രജിസ്റ്റർ പൂർണ്ണസംഖ്യ tmp;
  രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ avn;
  രജിസ്റ്റർ ശൂന്യം **p_argv;
  രജിസ്റ്റർ അക്ഷരം *argp;
  രജിസ്റ്റർ ffi_type **p_arg;
  പൂർണ്ണസംഖ്യ greg, ireg;
#എങ്കിൽ defined(__SH4__)
  പൂർണ്ണസംഖ്യ freg = 0;
#endif

  tmp = 0;
  argp = അട്ടി;

  എങ്കിൽ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(ശൂന്യം **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_മൂല്യം_ADDRESS_WITH_ARG ? 1 : 0;
    }
  അല്ലെങ്കിൽ
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  വേണ്ടി (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      എങ്കിൽ (z < വലുപ്പം(പൂർണ്ണസംഖ്യ))
	{
	  എങ്കിൽ (greg++ >= NGREGARG)
	    തുടരുക;

	  z = വലുപ്പം(പൂർണ്ണസംഖ്യ);
	  തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT8:
	      *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ)*(SINT8 *)(* p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_UINT8:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT8 *)(* p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_SINT16:
	      *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ)*(SINT16 *)(* p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_UINT16:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT16 *)(* p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_STRUCT:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT32 *)(* p_argv);
	      നിർത്തുക;

	    സ്ഥിരസ്ഥിതി:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം(പൂർണ്ണസംഖ്യ))
	{
#എങ്കിൽ defined(__SH4__)
	  എങ്കിൽ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      എങ്കിൽ (freg++ >= NFREGARG)
		തുടരുക;
	    }
	  അല്ലെങ്കിൽ
#endif
	    {
	      എങ്കിൽ (greg++ >= NGREGARG)
		തുടരുക;
	    }
	  *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#എങ്കിൽ defined(__SH4__)
      അല്ലെങ്കിൽ എങ്കിൽ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  എങ്കിൽ (freg + 1 >= NFREGARG)
	    തുടരുക;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      അല്ലെങ്കിൽ
	{
	  പൂർണ്ണസംഖ്യ n = (z + വലുപ്പം (പൂർണ്ണസംഖ്യ) - 1) / വലുപ്പം (പൂർണ്ണസംഖ്യ);
#എങ്കിൽ defined(__SH4__)
	  എങ്കിൽ (greg + n - 1 >= NGREGARG)
	    തുടരുക;
#അല്ലെങ്കിൽ
	  എങ്കിൽ (greg >= NGREGARG)
	    തുടരുക;
#endif
	  greg += n;
	  memcpy (argp, *p_argv, z);
	  argp += n * വലുപ്പം (പൂർണ്ണസംഖ്യ);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#എങ്കിൽ defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  വേണ്ടി (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->size;
      എങ്കിൽ (z < വലുപ്പം(പൂർണ്ണസംഖ്യ))
	{
	  എങ്കിൽ (greg++ < NGREGARG)
	    തുടരുക;

	  z = വലുപ്പം(പൂർണ്ണസംഖ്യ);
	  തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT8:
	      *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ)*(SINT8 *)(* p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_UINT8:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT8 *)(* p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_SINT16:
	      *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ)*(SINT16 *)(* p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_UINT16:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT16 *)(* p_argv);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_STRUCT:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT32 *)(* p_argv);
	      നിർത്തുക;

	    സ്ഥിരസ്ഥിതി:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം(പൂർണ്ണസംഖ്യ))
	{
#എങ്കിൽ defined(__SH4__)
	  എങ്കിൽ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      എങ്കിൽ (freg++ < NFREGARG)
		തുടരുക;
	    }
	  അല്ലെങ്കിൽ
#endif
	    {
	      എങ്കിൽ (greg++ < NGREGARG)
		തുടരുക;
	    }
	  *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#എങ്കിൽ defined(__SH4__)
      അല്ലെങ്കിൽ എങ്കിൽ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  എങ്കിൽ (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      തുടരുക;
	    }
	  memcpy (argp, *p_argv, z);
	  argp += z;
	}
#endif
      അല്ലെങ്കിൽ
	{
	  പൂർണ്ണസംഖ്യ n = (z + വലുപ്പം (പൂർണ്ണസംഖ്യ) - 1) / വലുപ്പം (പൂർണ്ണസംഖ്യ);
	  എങ്കിൽ (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      തുടരുക;
	    }
#എങ്കിൽ (! defined(__SH4__))
	  അല്ലെങ്കിൽ എങ്കിൽ (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      തുടരുക;
	    }
#endif
	  memcpy (argp, *p_argv, z);
	  argp += n * വലുപ്പം (പൂർണ്ണസംഖ്യ);
	}
    }

  തിരികെ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  പൂർണ്ണസംഖ്യ i, j;
  പൂർണ്ണസംഖ്യ size, type;
  പൂർണ്ണസംഖ്യ n, m;
  പൂർണ്ണസംഖ്യ greg;
#എങ്കിൽ defined(__SH4__)
  പൂർണ്ണസംഖ്യ freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_മൂല്യം_ADDRESS_WITH_ARG) ? 1 : 0;

#എങ്കിൽ defined(__SH4__)
  വേണ്ടി (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_FLOAT:
	  എങ്കിൽ (freg >= NFREGARG)
	    തുടരുക;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  എങ്കിൽ ((freg + 1) >= NFREGARG)
	    തുടരുക;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  നിർത്തുക;
	      
	സ്ഥിരസ്ഥിതി:
	  size = (cif->arg_types)[i]->size;
	  n = (size + വലുപ്പം (പൂർണ്ണസംഖ്യ) - 1) / വലുപ്പം (പൂർണ്ണസംഖ്യ);
	  എങ്കിൽ (greg + n - 1 >= NGREGARG)
		തുടരുക;
	  greg += n;
	  വേണ്ടി (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  നിർത്തുക;
	}
    }
#അല്ലെങ്കിൽ
  വേണ്ടി (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + വലുപ്പം (പൂർണ്ണസംഖ്യ) - 1) / വലുപ്പം (പൂർണ്ണസംഖ്യ);
      എങ്കിൽ (greg >= NGREGARG)
	തുടരുക;
      അല്ലെങ്കിൽ എങ്കിൽ (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      വേണ്ടി (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_STRUCT:
      cif->flags += (unsigned) (return_type (cif->rtype)) << 24;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_VOID:
    അവസ്ഥ FFI_TYPE_FLOAT:
    അവസ്ഥ FFI_TYPE_DOUBLE:
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
      cif->flags += (unsigned) cif->rtype->type << 24;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      cif->flags += FFI_TYPE_INT << 24;
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

ബാഹ്യ ശൂന്യം ffi_call_SYSV(ശൂന്യം (*)(അക്ഷരം *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, ശൂന്യം (*fn)(ശൂന്യം));

ശൂന്യം ffi_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  അല്ലെങ്കിൽ എങ്കിൽ ((rvalue == ശൂന്യ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi) 
    {
    അവസ്ഥ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }

  എങ്കിൽ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    memcpy (rvalue, &trvalue, cif->rtype->size);
}

ബാഹ്യ ശൂന്യം ffi_closure_SYSV (ശൂന്യം);
#എങ്കിൽ defined(__SH4__)
ബാഹ്യ ശൂന്യം __ic_invalidate (ശൂന്യം *വരി);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  unsigned പൂർണ്ണസംഖ്യ *tramp;
  unsigned പൂർണ്ണസംഖ്യ insn;

  എങ്കിൽ (cif->abi != FFI_SYSV)
    തിരികെ FFI_BAD_ABI;

  tramp = (unsigned പൂർണ്ണസംഖ്യ *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#അല്ലെങ്കിൽ
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(ശൂന്യം **) &tramp[2] = (ശൂന്യം *)codeloc;          /* ctx */
  *(ശൂന്യം **) &tramp[3] = (ശൂന്യം *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#എങ്കിൽ defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  തിരികെ FFI_OK;
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
#അല്ലെങ്കിൽ
#define OFS_INT8	3
#define OFS_INT16	2
#endif

പൂർണ്ണസംഖ്യ
ffi_closure_helper_SYSV (ffi_closure *closure, ശൂന്യം *rvalue, 
			 unsigned long *pgr, unsigned long *pfr, 
			 unsigned long *pst)
{
  ശൂന്യം **avalue;
  ffi_type **p_arg;
  പൂർണ്ണസംഖ്യ i, avn;
  പൂർണ്ണസംഖ്യ ireg, greg = 0;
#എങ്കിൽ defined(__SH4__)
  പൂർണ്ണസംഖ്യ freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * വലുപ്പം(ശൂന്യം *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_മൂല്യം_ADDRESS_WITH_ARG)
    {
      rvalue = (ശൂന്യം *) *pgr++;
      ireg = 1;
    }
  അല്ലെങ്കിൽ
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  വേണ്ടി (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      എങ്കിൽ (z < വലുപ്പം(പൂർണ്ണസംഖ്യ))
	{
	  എങ്കിൽ (greg++ >= NGREGARG)
	    തുടരുക;

	  z = വലുപ്പം(പൂർണ്ണസംഖ്യ);
	  തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT8:
	    അവസ്ഥ FFI_TYPE_UINT8:
	      avalue[i] = (((അക്ഷരം *)pgr) + OFS_INT8);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_SINT16:
	    അവസ്ഥ FFI_TYPE_UINT16:
	      avalue[i] = (((അക്ഷരം *)pgr) + OFS_INT16);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      നിർത്തുക;

	    സ്ഥിരസ്ഥിതി:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം(പൂർണ്ണസംഖ്യ))
	{
#എങ്കിൽ defined(__SH4__)
	  എങ്കിൽ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      എങ്കിൽ (freg++ >= NFREGARG)
		തുടരുക;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  അല്ലെങ്കിൽ
#endif
	    {
	      എങ്കിൽ (greg++ >= NGREGARG)
		തുടരുക;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#എങ്കിൽ defined(__SH4__)
      അല്ലെങ്കിൽ എങ്കിൽ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  എങ്കിൽ (freg + 1 >= NFREGARG)
	    തുടരുക;
	  എങ്കിൽ (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      അല്ലെങ്കിൽ
	{
	  പൂർണ്ണസംഖ്യ n = (z + വലുപ്പം (പൂർണ്ണസംഖ്യ) - 1) / വലുപ്പം (പൂർണ്ണസംഖ്യ);
#എങ്കിൽ defined(__SH4__)
	  എങ്കിൽ (greg + n - 1 >= NGREGARG)
	    തുടരുക;
#അല്ലെങ്കിൽ
	  എങ്കിൽ (greg >= NGREGARG)
	    തുടരുക;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#എങ്കിൽ defined(__SH4__)
  freg = 0;
#endif

  വേണ്ടി (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      എങ്കിൽ (z < വലുപ്പം(പൂർണ്ണസംഖ്യ))
	{
	  എങ്കിൽ (greg++ < NGREGARG)
	    തുടരുക;

	  z = വലുപ്പം(പൂർണ്ണസംഖ്യ);
	  തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT8:
	    അവസ്ഥ FFI_TYPE_UINT8:
	      avalue[i] = (((അക്ഷരം *)pst) + OFS_INT8);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_SINT16:
	    അവസ്ഥ FFI_TYPE_UINT16:
	      avalue[i] = (((അക്ഷരം *)pst) + OFS_INT16);
	      നിർത്തുക;
  
	    അവസ്ഥ FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      നിർത്തുക;

	    സ്ഥിരസ്ഥിതി:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം(പൂർണ്ണസംഖ്യ))
	{
#എങ്കിൽ defined(__SH4__)
	  എങ്കിൽ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      എങ്കിൽ (freg++ < NFREGARG)
		തുടരുക;
	    }
	  അല്ലെങ്കിൽ
#endif
	    {
	      എങ്കിൽ (greg++ < NGREGARG)
		തുടരുക;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#എങ്കിൽ defined(__SH4__)
      അല്ലെങ്കിൽ എങ്കിൽ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  എങ്കിൽ (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      തുടരുക;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      അല്ലെങ്കിൽ
	{
	  പൂർണ്ണസംഖ്യ n = (z + വലുപ്പം (പൂർണ്ണസംഖ്യ) - 1) / വലുപ്പം (പൂർണ്ണസംഖ്യ);
	  എങ്കിൽ (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      തുടരുക;
	    }
#എങ്കിൽ (! defined(__SH4__))
	  അല്ലെങ്കിൽ എങ്കിൽ (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      തുടരുക;
	    }
#endif
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  തിരികെ return_type (cif->rtype);
}
