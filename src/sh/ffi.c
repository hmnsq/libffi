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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>

#define NGREGARG 4
#जर defined(__SH4__)
#define NFREGARG 8
#endif

#जर defined(__HITACHI__)
#define STRUCT_मूल्य_ADDRESS_WITH_ARG 1
#अन्यथा
#define STRUCT_मूल्य_ADDRESS_WITH_ARG 0
#endif

/* If the structure has essentially an unique element, return its type.  */
स्थिर पूर्णांक
simple_type (ffi_type *arg)
{
  जर (arg->type != FFI_TYPE_STRUCT)
    परत arg->type;
  अन्यथा जर (arg->elements[1])
    परत FFI_TYPE_STRUCT;

  परत simple_type (arg->elements[0]);
}

स्थिर पूर्णांक
return_type (ffi_type *arg)
{
  अचिन्हित लघु type;

  जर (arg->type != FFI_TYPE_STRUCT)
    परत arg->type;

  type = simple_type (arg->elements[0]);
  जर (! arg->elements[1])
    {
      निवड (type)
	{
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT32:
	  परत FFI_TYPE_INT;

	पूर्वनिर्धारित:
	  परत type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  जर (arg->size <= 2 * आकार (पूर्णांक))
    {
      पूर्णांक i = 0;
      ffi_type *e;

      पर्यंत ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  निवड (type)
	    {
	    स्थिती FFI_TYPE_SINT32:
	    स्थिती FFI_TYPE_UINT32:
	    स्थिती FFI_TYPE_INT:
	    स्थिती FFI_TYPE_FLOAT:
	      परत FFI_TYPE_UINT64;

	    पूर्वनिर्धारित:
	      थांबा;
	    }
	}
    }

  परत FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

शून्य ffi_prep_args(अक्षर *चळत, extended_cif *ecif)
{
  नोंदणी अचिन्हित पूर्णांक i;
  नोंदणी पूर्णांक tmp;
  नोंदणी अचिन्हित पूर्णांक avn;
  नोंदणी शून्य **p_argv;
  नोंदणी अक्षर *argp;
  नोंदणी ffi_type **p_arg;
  पूर्णांक greg, ireg;
#जर defined(__SH4__)
  पूर्णांक freg = 0;
#endif

  tmp = 0;
  argp = चळत;

  जर (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(शून्य **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_मूल्य_ADDRESS_WITH_ARG ? 1 : 0;
    }
  अन्यथा
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  साठी (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      माप_प्रकार z;

      z = (*p_arg)->size;
      जर (z < आकार(पूर्णांक))
	{
	  जर (greg++ >= NGREGARG)
	    सुरूठेवा;

	  z = आकार(पूर्णांक);
	  निवड ((*p_arg)->type)
	    {
	    स्थिती FFI_TYPE_SINT8:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
	      थांबा;
  
	    स्थिती FFI_TYPE_UINT8:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
	      थांबा;
  
	    स्थिती FFI_TYPE_SINT16:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
	      थांबा;
  
	    स्थिती FFI_TYPE_UINT16:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
	      थांबा;
  
	    स्थिती FFI_TYPE_STRUCT:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	      थांबा;

	    पूर्वनिर्धारित:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      अन्यथा जर (z == आकार(पूर्णांक))
	{
#जर defined(__SH4__)
	  जर ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      जर (freg++ >= NFREGARG)
		सुरूठेवा;
	    }
	  अन्यथा
#endif
	    {
	      जर (greg++ >= NGREGARG)
		सुरूठेवा;
	    }
	  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#जर defined(__SH4__)
      अन्यथा जर ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  जर (freg + 1 >= NFREGARG)
	    सुरूठेवा;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  स_नकल (argp, *p_argv, z);
	  argp += z;
	}
#endif
      अन्यथा
	{
	  पूर्णांक n = (z + आकार (पूर्णांक) - 1) / आकार (पूर्णांक);
#जर defined(__SH4__)
	  जर (greg + n - 1 >= NGREGARG)
	    सुरूठेवा;
#अन्यथा
	  जर (greg >= NGREGARG)
	    सुरूठेवा;
#endif
	  greg += n;
	  स_नकल (argp, *p_argv, z);
	  argp += n * आकार (पूर्णांक);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#जर defined(__SH4__)
  freg = 0;
#endif
  p_argv = ecif->avalue;

  साठी (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      माप_प्रकार z;

      z = (*p_arg)->size;
      जर (z < आकार(पूर्णांक))
	{
	  जर (greg++ < NGREGARG)
	    सुरूठेवा;

	  z = आकार(पूर्णांक);
	  निवड ((*p_arg)->type)
	    {
	    स्थिती FFI_TYPE_SINT8:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
	      थांबा;
  
	    स्थिती FFI_TYPE_UINT8:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
	      थांबा;
  
	    स्थिती FFI_TYPE_SINT16:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
	      थांबा;
  
	    स्थिती FFI_TYPE_UINT16:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
	      थांबा;
  
	    स्थिती FFI_TYPE_STRUCT:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	      थांबा;

	    पूर्वनिर्धारित:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      अन्यथा जर (z == आकार(पूर्णांक))
	{
#जर defined(__SH4__)
	  जर ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      जर (freg++ < NFREGARG)
		सुरूठेवा;
	    }
	  अन्यथा
#endif
	    {
	      जर (greg++ < NGREGARG)
		सुरूठेवा;
	    }
	  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#जर defined(__SH4__)
      अन्यथा जर ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  जर (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      सुरूठेवा;
	    }
	  स_नकल (argp, *p_argv, z);
	  argp += z;
	}
#endif
      अन्यथा
	{
	  पूर्णांक n = (z + आकार (पूर्णांक) - 1) / आकार (पूर्णांक);
	  जर (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      सुरूठेवा;
	    }
#जर (! defined(__SH4__))
	  अन्यथा जर (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      सुरूठेवा;
	    }
#endif
	  स_नकल (argp, *p_argv, z);
	  argp += n * आकार (पूर्णांक);
	}
    }

  परत;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  पूर्णांक i, j;
  पूर्णांक size, type;
  पूर्णांक n, m;
  पूर्णांक greg;
#जर defined(__SH4__)
  पूर्णांक freg = 0;
#endif

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_मूल्य_ADDRESS_WITH_ARG) ? 1 : 0;

#जर defined(__SH4__)
  साठी (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      निवड (type)
	{
	स्थिती FFI_TYPE_FLOAT:
	  जर (freg >= NFREGARG)
	    सुरूठेवा;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	  जर ((freg + 1) >= NFREGARG)
	    सुरूठेवा;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  थांबा;
	      
	पूर्वनिर्धारित:
	  size = (cif->arg_types)[i]->size;
	  n = (size + आकार (पूर्णांक) - 1) / आकार (पूर्णांक);
	  जर (greg + n - 1 >= NGREGARG)
		सुरूठेवा;
	  greg += n;
	  साठी (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  थांबा;
	}
    }
#अन्यथा
  साठी (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      size = (cif->arg_types)[i]->size;
      n = (size + आकार (पूर्णांक) - 1) / आकार (पूर्णांक);
      जर (greg >= NGREGARG)
	सुरूठेवा;
      अन्यथा जर (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      साठी (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#endif

  /* Set the return type flag */
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_STRUCT:
      cif->flags += (अचिन्हित) (return_type (cif->rtype)) << 24;
      थांबा;

    स्थिती FFI_TYPE_VOID:
    स्थिती FFI_TYPE_FLOAT:
    स्थिती FFI_TYPE_DOUBLE:
    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
      cif->flags += (अचिन्हित) cif->rtype->type << 24;
      थांबा;

    पूर्वनिर्धारित:
      cif->flags += FFI_TYPE_INT << 24;
      थांबा;
    }

  परत FFI_OK;
}

बाह्य शून्य ffi_call_SYSV(शून्य (*)(अक्षर *, extended_cif *), extended_cif *,
			  अचिन्हित, अचिन्हित, अचिन्हित *, शून्य (*fn)(शून्य));

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  जर (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  अन्यथा जर ((rvalue == रिक्त) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  निवड (cif->abi) 
    {
    स्थिती FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      थांबा;
    पूर्वनिर्धारित:
      FFI_ASSERT(0);
      थांबा;
    }

  जर (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    स_नकल (rvalue, &trvalue, cif->rtype->size);
}

बाह्य शून्य ffi_closure_SYSV (शून्य);
#जर defined(__SH4__)
बाह्य शून्य __ic_invalidate (शून्य *ओळ);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  अचिन्हित पूर्णांक *tramp;
  अचिन्हित पूर्णांक insn;

  जर (cif->abi != FFI_SYSV)
    परत FFI_BAD_ABI;

  tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ifdef __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#अन्यथा
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#endif
  *(शून्य **) &tramp[2] = (शून्य *)codeloc;          /* ctx */
  *(शून्य **) &tramp[3] = (शून्य *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#जर defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#endif

  परत FFI_OK;
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
#अन्यथा
#define OFS_INT8	3
#define OFS_INT16	2
#endif

पूर्णांक
ffi_closure_helper_SYSV (ffi_closure *closure, शून्य *rvalue, 
			 अचिन्हित दीर्घ *pgr, अचिन्हित दीर्घ *pfr, 
			 अचिन्हित दीर्घ *pst)
{
  शून्य **avalue;
  ffi_type **p_arg;
  पूर्णांक i, avn;
  पूर्णांक ireg, greg = 0;
#जर defined(__SH4__)
  पूर्णांक freg = 0;
#endif
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * आकार(शून्य *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  जर (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_मूल्य_ADDRESS_WITH_ARG)
    {
      rvalue = (शून्य *) *pgr++;
      ireg = 1;
    }
  अन्यथा
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  साठी (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->size;
      जर (z < आकार(पूर्णांक))
	{
	  जर (greg++ >= NGREGARG)
	    सुरूठेवा;

	  z = आकार(पूर्णांक);
	  निवड ((*p_arg)->type)
	    {
	    स्थिती FFI_TYPE_SINT8:
	    स्थिती FFI_TYPE_UINT8:
	      avalue[i] = (((अक्षर *)pgr) + OFS_INT8);
	      थांबा;
  
	    स्थिती FFI_TYPE_SINT16:
	    स्थिती FFI_TYPE_UINT16:
	      avalue[i] = (((अक्षर *)pgr) + OFS_INT16);
	      थांबा;
  
	    स्थिती FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      थांबा;

	    पूर्वनिर्धारित:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      अन्यथा जर (z == आकार(पूर्णांक))
	{
#जर defined(__SH4__)
	  जर ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      जर (freg++ >= NFREGARG)
		सुरूठेवा;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
#endif
	    {
	      जर (greg++ >= NGREGARG)
		सुरूठेवा;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#जर defined(__SH4__)
      अन्यथा जर ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  जर (freg + 1 >= NFREGARG)
	    सुरूठेवा;
	  जर (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#endif
      अन्यथा
	{
	  पूर्णांक n = (z + आकार (पूर्णांक) - 1) / आकार (पूर्णांक);
#जर defined(__SH4__)
	  जर (greg + n - 1 >= NGREGARG)
	    सुरूठेवा;
#अन्यथा
	  जर (greg >= NGREGARG)
	    सुरूठेवा;
#endif
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#जर defined(__SH4__)
  freg = 0;
#endif

  साठी (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->size;
      जर (z < आकार(पूर्णांक))
	{
	  जर (greg++ < NGREGARG)
	    सुरूठेवा;

	  z = आकार(पूर्णांक);
	  निवड ((*p_arg)->type)
	    {
	    स्थिती FFI_TYPE_SINT8:
	    स्थिती FFI_TYPE_UINT8:
	      avalue[i] = (((अक्षर *)pst) + OFS_INT8);
	      थांबा;
  
	    स्थिती FFI_TYPE_SINT16:
	    स्थिती FFI_TYPE_UINT16:
	      avalue[i] = (((अक्षर *)pst) + OFS_INT16);
	      थांबा;
  
	    स्थिती FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      थांबा;

	    पूर्वनिर्धारित:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      अन्यथा जर (z == आकार(पूर्णांक))
	{
#जर defined(__SH4__)
	  जर ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      जर (freg++ < NFREGARG)
		सुरूठेवा;
	    }
	  अन्यथा
#endif
	    {
	      जर (greg++ < NGREGARG)
		सुरूठेवा;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#जर defined(__SH4__)
      अन्यथा जर ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  जर (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      सुरूठेवा;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#endif
      अन्यथा
	{
	  पूर्णांक n = (z + आकार (पूर्णांक) - 1) / आकार (पूर्णांक);
	  जर (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      सुरूठेवा;
	    }
#जर (! defined(__SH4__))
	  अन्यथा जर (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      सुरूठेवा;
	    }
#endif
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  परत return_type (cif->rtype);
}
