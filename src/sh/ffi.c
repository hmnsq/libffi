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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>

#घोषणा NGREGARG 4
#अगर defined(__SH4__)
#घोषणा NFREGARG 8
#पूर्ण_अगर

#अगर defined(__HITACHI__)
#घोषणा STRUCT_मूल्य_ADDRESS_WITH_ARG 1
#अन्यथा
#घोषणा STRUCT_मूल्य_ADDRESS_WITH_ARG 0
#पूर्ण_अगर

/* If the structure has essentially an unique element, return its type.  */
अटल पूर्णांक
simple_type (ffi_type *arg)
{
  अगर (arg->type != FFI_TYPE_STRUCT)
    वापस arg->type;
  अन्यथा अगर (arg->elements[1])
    वापस FFI_TYPE_STRUCT;

  वापस simple_type (arg->elements[0]);
}

अटल पूर्णांक
return_type (ffi_type *arg)
{
  अचिन्हित लघु type;

  अगर (arg->type != FFI_TYPE_STRUCT)
    वापस arg->type;

  type = simple_type (arg->elements[0]);
  अगर (! arg->elements[1])
    {
      चयन (type)
	{
	हाल FFI_TYPE_SINT8:
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT32:
	  वापस FFI_TYPE_INT;

	शेष:
	  वापस type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  अगर (arg->आकार <= 2 * माप (पूर्णांक))
    {
      पूर्णांक i = 0;
      ffi_type *e;

      जबतक ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  चयन (type)
	    {
	    हाल FFI_TYPE_SINT32:
	    हाल FFI_TYPE_UINT32:
	    हाल FFI_TYPE_INT:
	    हाल FFI_TYPE_FLOAT:
	      वापस FFI_TYPE_UINT64;

	    शेष:
	      अवरोध;
	    }
	}
    }

  वापस FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

व्योम ffi_prep_args(अक्षर *ढेर, extended_cif *ecif)
{
  रेजिस्टर अचिन्हित पूर्णांक i;
  रेजिस्टर पूर्णांक tmp;
  रेजिस्टर अचिन्हित पूर्णांक avn;
  रेजिस्टर व्योम **p_argv;
  रेजिस्टर अक्षर *argp;
  रेजिस्टर ffi_type **p_arg;
  पूर्णांक greg, ireg;
#अगर defined(__SH4__)
  पूर्णांक freg = 0;
#पूर्ण_अगर

  tmp = 0;
  argp = ढेर;

  अगर (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(व्योम **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_मूल्य_ADDRESS_WITH_ARG ? 1 : 0;
    }
  अन्यथा
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  क्रम (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      माप_प्रकार z;

      z = (*p_arg)->आकार;
      अगर (z < माप(पूर्णांक))
	{
	  अगर (greg++ >= NGREGARG)
	    जारी;

	  z = माप(पूर्णांक);
	  चयन ((*p_arg)->type)
	    {
	    हाल FFI_TYPE_SINT8:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_UINT8:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_SINT16:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_UINT16:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_STRUCT:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	      अवरोध;

	    शेष:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      अन्यथा अगर (z == माप(पूर्णांक))
	{
#अगर defined(__SH4__)
	  अगर ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      अगर (freg++ >= NFREGARG)
		जारी;
	    }
	  अन्यथा
#पूर्ण_अगर
	    {
	      अगर (greg++ >= NGREGARG)
		जारी;
	    }
	  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#अगर defined(__SH4__)
      अन्यथा अगर ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  अगर (freg + 1 >= NFREGARG)
	    जारी;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  स_नकल (argp, *p_argv, z);
	  argp += z;
	}
#पूर्ण_अगर
      अन्यथा
	{
	  पूर्णांक n = (z + माप (पूर्णांक) - 1) / माप (पूर्णांक);
#अगर defined(__SH4__)
	  अगर (greg + n - 1 >= NGREGARG)
	    जारी;
#अन्यथा
	  अगर (greg >= NGREGARG)
	    जारी;
#पूर्ण_अगर
	  greg += n;
	  स_नकल (argp, *p_argv, z);
	  argp += n * माप (पूर्णांक);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#अगर defined(__SH4__)
  freg = 0;
#पूर्ण_अगर
  p_argv = ecif->avalue;

  क्रम (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      माप_प्रकार z;

      z = (*p_arg)->आकार;
      अगर (z < माप(पूर्णांक))
	{
	  अगर (greg++ < NGREGARG)
	    जारी;

	  z = माप(पूर्णांक);
	  चयन ((*p_arg)->type)
	    {
	    हाल FFI_TYPE_SINT8:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_UINT8:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_SINT16:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_UINT16:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
	      अवरोध;
  
	    हाल FFI_TYPE_STRUCT:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	      अवरोध;

	    शेष:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      अन्यथा अगर (z == माप(पूर्णांक))
	{
#अगर defined(__SH4__)
	  अगर ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      अगर (freg++ < NFREGARG)
		जारी;
	    }
	  अन्यथा
#पूर्ण_अगर
	    {
	      अगर (greg++ < NGREGARG)
		जारी;
	    }
	  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#अगर defined(__SH4__)
      अन्यथा अगर ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  अगर (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      जारी;
	    }
	  स_नकल (argp, *p_argv, z);
	  argp += z;
	}
#पूर्ण_अगर
      अन्यथा
	{
	  पूर्णांक n = (z + माप (पूर्णांक) - 1) / माप (पूर्णांक);
	  अगर (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      जारी;
	    }
#अगर (! defined(__SH4__))
	  अन्यथा अगर (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      जारी;
	    }
#पूर्ण_अगर
	  स_नकल (argp, *p_argv, z);
	  argp += n * माप (पूर्णांक);
	}
    }

  वापस;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  पूर्णांक i, j;
  पूर्णांक आकार, type;
  पूर्णांक n, m;
  पूर्णांक greg;
#अगर defined(__SH4__)
  पूर्णांक freg = 0;
#पूर्ण_अगर

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_मूल्य_ADDRESS_WITH_ARG) ? 1 : 0;

#अगर defined(__SH4__)
  क्रम (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      चयन (type)
	{
	हाल FFI_TYPE_FLOAT:
	  अगर (freg >= NFREGARG)
	    जारी;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  अवरोध;

	हाल FFI_TYPE_DOUBLE:
	  अगर ((freg + 1) >= NFREGARG)
	    जारी;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  अवरोध;
	      
	शेष:
	  आकार = (cif->arg_types)[i]->आकार;
	  n = (आकार + माप (पूर्णांक) - 1) / माप (पूर्णांक);
	  अगर (greg + n - 1 >= NGREGARG)
		जारी;
	  greg += n;
	  क्रम (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  अवरोध;
	}
    }
#अन्यथा
  क्रम (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      आकार = (cif->arg_types)[i]->आकार;
      n = (आकार + माप (पूर्णांक) - 1) / माप (पूर्णांक);
      अगर (greg >= NGREGARG)
	जारी;
      अन्यथा अगर (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      क्रम (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#पूर्ण_अगर

  /* Set the return type flag */
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_STRUCT:
      cif->flags += (अचिन्हित) (return_type (cif->rtype)) << 24;
      अवरोध;

    हाल FFI_TYPE_VOID:
    हाल FFI_TYPE_FLOAT:
    हाल FFI_TYPE_DOUBLE:
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
      cif->flags += (अचिन्हित) cif->rtype->type << 24;
      अवरोध;

    शेष:
      cif->flags += FFI_TYPE_INT << 24;
      अवरोध;
    }

  वापस FFI_OK;
}

बाह्य व्योम ffi_call_SYSV(व्योम (*)(अक्षर *, extended_cif *), extended_cif *,
			  अचिन्हित, अचिन्हित, अचिन्हित *, व्योम (*fn)(व्योम));

व्योम ffi_call(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  अगर (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  अन्यथा अगर ((rvalue == शून्य) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->आकार);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  चयन (cif->abi) 
    {
    हाल FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes, cif->flags, ecif.rvalue,
		    fn);
      अवरोध;
    शेष:
      FFI_ASSERT(0);
      अवरोध;
    }

  अगर (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    स_नकल (rvalue, &trvalue, cif->rtype->आकार);
}

बाह्य व्योम ffi_closure_SYSV (व्योम);
#अगर defined(__SH4__)
बाह्य व्योम __ic_invalidate (व्योम *पंक्ति_2);
#पूर्ण_अगर

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  अचिन्हित पूर्णांक *tramp;
  अचिन्हित पूर्णांक insn;

  अगर (cif->abi != FFI_SYSV)
    वापस FFI_BAD_ABI;

  tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#अगर_घोषित __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#अन्यथा
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#पूर्ण_अगर
  *(व्योम **) &tramp[2] = (व्योम *)codeloc;          /* ctx */
  *(व्योम **) &tramp[3] = (व्योम *)ffi_closure_SYSV; /* funaddr */

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

#अगर defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#पूर्ण_अगर

  वापस FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

#अगर_घोषित __LITTLE_ENDIAN__
#घोषणा OFS_INT8	0
#घोषणा OFS_INT16	0
#अन्यथा
#घोषणा OFS_INT8	3
#घोषणा OFS_INT16	2
#पूर्ण_अगर

पूर्णांक
ffi_closure_helper_SYSV (ffi_closure *closure, व्योम *rvalue, 
			 अचिन्हित दीर्घ *pgr, अचिन्हित दीर्घ *pfr, 
			 अचिन्हित दीर्घ *pst)
{
  व्योम **avalue;
  ffi_type **p_arg;
  पूर्णांक i, avn;
  पूर्णांक ireg, greg = 0;
#अगर defined(__SH4__)
  पूर्णांक freg = 0;
#पूर्ण_अगर
  ffi_cif *cif; 

  cif = closure->cif;
  avalue = alloca(cif->nargs * माप(व्योम *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  अगर (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_मूल्य_ADDRESS_WITH_ARG)
    {
      rvalue = (व्योम *) *pgr++;
      ireg = 1;
    }
  अन्यथा
    ireg = 0;

  cif = closure->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  क्रम (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->आकार;
      अगर (z < माप(पूर्णांक))
	{
	  अगर (greg++ >= NGREGARG)
	    जारी;

	  z = माप(पूर्णांक);
	  चयन ((*p_arg)->type)
	    {
	    हाल FFI_TYPE_SINT8:
	    हाल FFI_TYPE_UINT8:
	      avalue[i] = (((अक्षर *)pgr) + OFS_INT8);
	      अवरोध;
  
	    हाल FFI_TYPE_SINT16:
	    हाल FFI_TYPE_UINT16:
	      avalue[i] = (((अक्षर *)pgr) + OFS_INT16);
	      अवरोध;
  
	    हाल FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      अवरोध;

	    शेष:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      अन्यथा अगर (z == माप(पूर्णांक))
	{
#अगर defined(__SH4__)
	  अगर ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      अगर (freg++ >= NFREGARG)
		जारी;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
#पूर्ण_अगर
	    {
	      अगर (greg++ >= NGREGARG)
		जारी;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#अगर defined(__SH4__)
      अन्यथा अगर ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  अगर (freg + 1 >= NFREGARG)
	    जारी;
	  अगर (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#पूर्ण_अगर
      अन्यथा
	{
	  पूर्णांक n = (z + माप (पूर्णांक) - 1) / माप (पूर्णांक);
#अगर defined(__SH4__)
	  अगर (greg + n - 1 >= NGREGARG)
	    जारी;
#अन्यथा
	  अगर (greg >= NGREGARG)
	    जारी;
#पूर्ण_अगर
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#अगर defined(__SH4__)
  freg = 0;
#पूर्ण_अगर

  क्रम (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->आकार;
      अगर (z < माप(पूर्णांक))
	{
	  अगर (greg++ < NGREGARG)
	    जारी;

	  z = माप(पूर्णांक);
	  चयन ((*p_arg)->type)
	    {
	    हाल FFI_TYPE_SINT8:
	    हाल FFI_TYPE_UINT8:
	      avalue[i] = (((अक्षर *)pst) + OFS_INT8);
	      अवरोध;
  
	    हाल FFI_TYPE_SINT16:
	    हाल FFI_TYPE_UINT16:
	      avalue[i] = (((अक्षर *)pst) + OFS_INT16);
	      अवरोध;
  
	    हाल FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      अवरोध;

	    शेष:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      अन्यथा अगर (z == माप(पूर्णांक))
	{
#अगर defined(__SH4__)
	  अगर ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      अगर (freg++ < NFREGARG)
		जारी;
	    }
	  अन्यथा
#पूर्ण_अगर
	    {
	      अगर (greg++ < NGREGARG)
		जारी;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#अगर defined(__SH4__)
      अन्यथा अगर ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  अगर (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      जारी;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#पूर्ण_अगर
      अन्यथा
	{
	  पूर्णांक n = (z + माप (पूर्णांक) - 1) / माप (पूर्णांक);
	  अगर (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      जारी;
	    }
#अगर (! defined(__SH4__))
	  अन्यथा अगर (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      जारी;
	    }
#पूर्ण_अगर
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (closure->fun) (cif, rvalue, avalue, closure->user_तथ्य);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  वापस return_type (cif->rtype);
}
