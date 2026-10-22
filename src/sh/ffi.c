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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>

#నిర్వచించు NGREGARG 4
#ఒకవేళ defined(__SH4__)
#నిర్వచించు NFREGARG 8
#అయితే_ముగింపు

#ఒకవేళ defined(__HITACHI__)
#నిర్వచించు STRUCT_విలువ_చిరునామా_WITH_ARG 1
#లేకపోతే
#నిర్వచించు STRUCT_విలువ_చిరునామా_WITH_ARG 0
#అయితే_ముగింపు

/* If the structure has essentially an unique element, return its type.  */
స్థిర పూర్ణాంకం
simple_type (ffi_type *arg)
{
  ఒకవేళ (arg->type != FFI_TYPE_STRUCT)
    తిరిగి arg->type;
  లేకపోతే ఒకవేళ (arg->elements[1])
    తిరిగి FFI_TYPE_STRUCT;

  తిరిగి simple_type (arg->elements[0]);
}

స్థిర పూర్ణాంకం
return_type (ffi_type *arg)
{
  గుర్తులేని పొట్టి type;

  ఒకవేళ (arg->type != FFI_TYPE_STRUCT)
    తిరిగి arg->type;

  type = simple_type (arg->elements[0]);
  ఒకవేళ (! arg->elements[1])
    {
      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
	  తిరిగి FFI_TYPE_INT;

	అప్రమేయ:
	  తిరిగి type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  ఒకవేళ (arg->కొలత <= 2 * పరిమాణం (పూర్ణాంకం))
    {
      పూర్ణాంకం i = 0;
      ffi_type *e;

      అయితే ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  ఎంపిక (type)
	    {
	    స్థితి FFI_TYPE_SINT32:
	    స్థితి FFI_TYPE_UINT32:
	    స్థితి FFI_TYPE_INT:
	    స్థితి FFI_TYPE_FLOAT:
	      తిరిగి FFI_TYPE_UINT64;

	    అప్రమేయ:
	      ఆపు;
	    }
	}
    }

  తిరిగి FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

శూన్య ffi_prep_వాదనలు(అక్షరం *దొంతర, extended_cif *ecif)
{
  నమోదు గుర్తులేని పూర్ణాంకం i;
  నమోదు పూర్ణాంకం tmp;
  నమోదు గుర్తులేని పూర్ణాంకం avn;
  నమోదు శూన్య **p_argv;
  నమోదు అక్షరం *argp;
  నమోదు ffi_type **p_arg;
  పూర్ణాంకం greg, ireg;
#ఒకవేళ defined(__SH4__)
  పూర్ణాంకం freg = 0;
#అయితే_ముగింపు

  tmp = 0;
  argp = దొంతర;

  ఒకవేళ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(శూన్య **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_విలువ_చిరునామా_WITH_ARG ? 1 : 0;
    }
  లేకపోతే
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  కొరకు (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->కొలత;
      ఒకవేళ (z < పరిమాణం(పూర్ణాంకం))
	{
	  ఒకవేళ (greg++ >= NGREGARG)
	    కొనసాగు;

	  z = పరిమాణం(పూర్ణాంకం);
	  ఎంపిక ((*p_arg)->type)
	    {
	    స్థితి FFI_TYPE_SINT8:
	      *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT8 *)(* p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_UINT8:
	      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT8 *)(* p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_SINT16:
	      *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT16 *)(* p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_UINT16:
	      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT16 *)(* p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_STRUCT:
	      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT32 *)(* p_argv);
	      ఆపు;

	    అప్రమేయ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      లేకపోతే ఒకవేళ (z == పరిమాణం(పూర్ణాంకం))
	{
#ఒకవేళ defined(__SH4__)
	  ఒకవేళ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ఒకవేళ (freg++ >= NFREGARG)
		కొనసాగు;
	    }
	  లేకపోతే
#అయితే_ముగింపు
	    {
	      ఒకవేళ (greg++ >= NGREGARG)
		కొనసాగు;
	    }
	  *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ఒకవేళ defined(__SH4__)
      లేకపోతే ఒకవేళ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ఒకవేళ (freg + 1 >= NFREGARG)
	    కొనసాగు;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  జ్ఞాపకం_నకలు (argp, *p_argv, z);
	  argp += z;
	}
#అయితే_ముగింపు
      లేకపోతే
	{
	  పూర్ణాంకం n = (z + పరిమాణం (పూర్ణాంకం) - 1) / పరిమాణం (పూర్ణాంకం);
#ఒకవేళ defined(__SH4__)
	  ఒకవేళ (greg + n - 1 >= NGREGARG)
	    కొనసాగు;
#లేకపోతే
	  ఒకవేళ (greg >= NGREGARG)
	    కొనసాగు;
#అయితే_ముగింపు
	  greg += n;
	  జ్ఞాపకం_నకలు (argp, *p_argv, z);
	  argp += n * పరిమాణం (పూర్ణాంకం);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#ఒకవేళ defined(__SH4__)
  freg = 0;
#అయితే_ముగింపు
  p_argv = ecif->avalue;

  కొరకు (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->కొలత;
      ఒకవేళ (z < పరిమాణం(పూర్ణాంకం))
	{
	  ఒకవేళ (greg++ < NGREGARG)
	    కొనసాగు;

	  z = పరిమాణం(పూర్ణాంకం);
	  ఎంపిక ((*p_arg)->type)
	    {
	    స్థితి FFI_TYPE_SINT8:
	      *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT8 *)(* p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_UINT8:
	      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT8 *)(* p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_SINT16:
	      *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT16 *)(* p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_UINT16:
	      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT16 *)(* p_argv);
	      ఆపు;
  
	    స్థితి FFI_TYPE_STRUCT:
	      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT32 *)(* p_argv);
	      ఆపు;

	    అప్రమేయ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      లేకపోతే ఒకవేళ (z == పరిమాణం(పూర్ణాంకం))
	{
#ఒకవేళ defined(__SH4__)
	  ఒకవేళ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ఒకవేళ (freg++ < NFREGARG)
		కొనసాగు;
	    }
	  లేకపోతే
#అయితే_ముగింపు
	    {
	      ఒకవేళ (greg++ < NGREGARG)
		కొనసాగు;
	    }
	  *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ఒకవేళ defined(__SH4__)
      లేకపోతే ఒకవేళ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ఒకవేళ (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      కొనసాగు;
	    }
	  జ్ఞాపకం_నకలు (argp, *p_argv, z);
	  argp += z;
	}
#అయితే_ముగింపు
      లేకపోతే
	{
	  పూర్ణాంకం n = (z + పరిమాణం (పూర్ణాంకం) - 1) / పరిమాణం (పూర్ణాంకం);
	  ఒకవేళ (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      కొనసాగు;
	    }
#ఒకవేళ (! defined(__SH4__))
	  లేకపోతే ఒకవేళ (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      కొనసాగు;
	    }
#అయితే_ముగింపు
	  జ్ఞాపకం_నకలు (argp, *p_argv, z);
	  argp += n * పరిమాణం (పూర్ణాంకం);
	}
    }

  తిరిగి;
}

/* Perform machine dependent cif processing */
ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif)
{
  పూర్ణాంకం i, j;
  పూర్ణాంకం కొలత, type;
  పూర్ణాంకం n, m;
  పూర్ణాంకం greg;
#ఒకవేళ defined(__SH4__)
  పూర్ణాంకం freg = 0;
#అయితే_ముగింపు

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_విలువ_చిరునామా_WITH_ARG) ? 1 : 0;

#ఒకవేళ defined(__SH4__)
  కొరకు (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_FLOAT:
	  ఒకవేళ (freg >= NFREGARG)
	    కొనసాగు;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  ఒకవేళ ((freg + 1) >= NFREGARG)
	    కొనసాగు;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ఆపు;
	      
	అప్రమేయ:
	  కొలత = (cif->arg_types)[i]->కొలత;
	  n = (కొలత + పరిమాణం (పూర్ణాంకం) - 1) / పరిమాణం (పూర్ణాంకం);
	  ఒకవేళ (greg + n - 1 >= NGREGARG)
		కొనసాగు;
	  greg += n;
	  కొరకు (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  ఆపు;
	}
    }
#లేకపోతే
  కొరకు (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      కొలత = (cif->arg_types)[i]->కొలత;
      n = (కొలత + పరిమాణం (పూర్ణాంకం) - 1) / పరిమాణం (పూర్ణాంకం);
      ఒకవేళ (greg >= NGREGARG)
	కొనసాగు;
      లేకపోతే ఒకవేళ (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      కొరకు (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#అయితే_ముగింపు

  /* Set the return type flag */
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_STRUCT:
      cif->flags += (గుర్తులేని) (return_type (cif->rtype)) << 24;
      ఆపు;

    స్థితి FFI_TYPE_VOID:
    స్థితి FFI_TYPE_FLOAT:
    స్థితి FFI_TYPE_DOUBLE:
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      cif->flags += (గుర్తులేని) cif->rtype->type << 24;
      ఆపు;

    అప్రమేయ:
      cif->flags += FFI_TYPE_INT << 24;
      ఆపు;
    }

  తిరిగి FFI_OK;
}

బాహ్య శూన్య ffi_call_SYSV(శూన్య (*)(అక్షరం *, extended_cif *), extended_cif *,
			  గుర్తులేని, గుర్తులేని, గుర్తులేని *, శూన్య (*fn)(శూన్య));

శూన్య ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  లేకపోతే ఒకవేళ ((rvalue == ఖాళీ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->కొలత);
    }
  లేకపోతే
    ecif.rvalue = rvalue;

  ఎంపిక (cif->abi) 
    {
    స్థితి FFI_SYSV:
      ffi_call_SYSV(ffi_prep_వాదనలు, &ecif, cif->బైట్లు, cif->flags, ecif.rvalue,
		    fn);
      ఆపు;
    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }

  ఒకవేళ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    జ్ఞాపకం_నకలు (rvalue, &trvalue, cif->rtype->కొలత);
}

బాహ్య శూన్య ffi_సంవృతం_SYSV (శూన్య);
#ఒకవేళ defined(__SH4__)
బాహ్య శూన్య __ic_invalidate (శూన్య *పంక్తి);
#అయితే_ముగింపు

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  గుర్తులేని పూర్ణాంకం *tramp;
  గుర్తులేని పూర్ణాంకం insn;

  ఒకవేళ (cif->abi != FFI_SYSV)
    తిరిగి FFI_BAD_ABI;

  tramp = (గుర్తులేని పూర్ణాంకం *) &సంవృతం->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#నిర్వచితమైతే __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#లేకపోతే
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#అయితే_ముగింపు
  *(శూన్య **) &tramp[2] = (శూన్య *)codeloc;          /* ctx */
  *(శూన్య **) &tramp[3] = (శూన్య *)ffi_సంవృతం_SYSV; /* funaddr */

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

#ఒకవేళ defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#అయితే_ముగింపు

  తిరిగి FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

#నిర్వచితమైతే __LITTLE_ENDIAN__
#నిర్వచించు OFS_INT8	0
#నిర్వచించు OFS_INT16	0
#లేకపోతే
#నిర్వచించు OFS_INT8	3
#నిర్వచించు OFS_INT16	2
#అయితే_ముగింపు

పూర్ణాంకం
ffi_సంవృతం_helper_SYSV (ffi_సంవృతం *సంవృతం, శూన్య *rvalue, 
			 గుర్తులేని దీర్ఘ *pgr, గుర్తులేని దీర్ఘ *pfr, 
			 గుర్తులేని దీర్ఘ *pst)
{
  శూన్య **avalue;
  ffi_type **p_arg;
  పూర్ణాంకం i, avn;
  పూర్ణాంకం ireg, greg = 0;
#ఒకవేళ defined(__SH4__)
  పూర్ణాంకం freg = 0;
#అయితే_ముగింపు
  ffi_cif *cif; 

  cif = సంవృతం->cif;
  avalue = alloca(cif->nargs * పరిమాణం(శూన్య *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_విలువ_చిరునామా_WITH_ARG)
    {
      rvalue = (శూన్య *) *pgr++;
      ireg = 1;
    }
  లేకపోతే
    ireg = 0;

  cif = సంవృతం->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  కొరకు (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->కొలత;
      ఒకవేళ (z < పరిమాణం(పూర్ణాంకం))
	{
	  ఒకవేళ (greg++ >= NGREGARG)
	    కొనసాగు;

	  z = పరిమాణం(పూర్ణాంకం);
	  ఎంపిక ((*p_arg)->type)
	    {
	    స్థితి FFI_TYPE_SINT8:
	    స్థితి FFI_TYPE_UINT8:
	      avalue[i] = (((అక్షరం *)pgr) + OFS_INT8);
	      ఆపు;
  
	    స్థితి FFI_TYPE_SINT16:
	    స్థితి FFI_TYPE_UINT16:
	      avalue[i] = (((అక్షరం *)pgr) + OFS_INT16);
	      ఆపు;
  
	    స్థితి FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      ఆపు;

	    అప్రమేయ:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      లేకపోతే ఒకవేళ (z == పరిమాణం(పూర్ణాంకం))
	{
#ఒకవేళ defined(__SH4__)
	  ఒకవేళ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ఒకవేళ (freg++ >= NFREGARG)
		కొనసాగు;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  లేకపోతే
#అయితే_ముగింపు
	    {
	      ఒకవేళ (greg++ >= NGREGARG)
		కొనసాగు;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#ఒకవేళ defined(__SH4__)
      లేకపోతే ఒకవేళ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ఒకవేళ (freg + 1 >= NFREGARG)
	    కొనసాగు;
	  ఒకవేళ (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#అయితే_ముగింపు
      లేకపోతే
	{
	  పూర్ణాంకం n = (z + పరిమాణం (పూర్ణాంకం) - 1) / పరిమాణం (పూర్ణాంకం);
#ఒకవేళ defined(__SH4__)
	  ఒకవేళ (greg + n - 1 >= NGREGARG)
	    కొనసాగు;
#లేకపోతే
	  ఒకవేళ (greg >= NGREGARG)
	    కొనసాగు;
#అయితే_ముగింపు
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#ఒకవేళ defined(__SH4__)
  freg = 0;
#అయితే_ముగింపు

  కొరకు (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->కొలత;
      ఒకవేళ (z < పరిమాణం(పూర్ణాంకం))
	{
	  ఒకవేళ (greg++ < NGREGARG)
	    కొనసాగు;

	  z = పరిమాణం(పూర్ణాంకం);
	  ఎంపిక ((*p_arg)->type)
	    {
	    స్థితి FFI_TYPE_SINT8:
	    స్థితి FFI_TYPE_UINT8:
	      avalue[i] = (((అక్షరం *)pst) + OFS_INT8);
	      ఆపు;
  
	    స్థితి FFI_TYPE_SINT16:
	    స్థితి FFI_TYPE_UINT16:
	      avalue[i] = (((అక్షరం *)pst) + OFS_INT16);
	      ఆపు;
  
	    స్థితి FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      ఆపు;

	    అప్రమేయ:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      లేకపోతే ఒకవేళ (z == పరిమాణం(పూర్ణాంకం))
	{
#ఒకవేళ defined(__SH4__)
	  ఒకవేళ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ఒకవేళ (freg++ < NFREGARG)
		కొనసాగు;
	    }
	  లేకపోతే
#అయితే_ముగింపు
	    {
	      ఒకవేళ (greg++ < NGREGARG)
		కొనసాగు;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#ఒకవేళ defined(__SH4__)
      లేకపోతే ఒకవేళ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ఒకవేళ (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      కొనసాగు;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#అయితే_ముగింపు
      లేకపోతే
	{
	  పూర్ణాంకం n = (z + పరిమాణం (పూర్ణాంకం) - 1) / పరిమాణం (పూర్ణాంకం);
	  ఒకవేళ (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      కొనసాగు;
	    }
#ఒకవేళ (! defined(__SH4__))
	  లేకపోతే ఒకవేళ (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      కొనసాగు;
	    }
#అయితే_ముగింపు
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (సంవృతం->fun) (cif, rvalue, avalue, సంవృతం->user_దత్తాంశం);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  తిరిగి return_type (cif->rtype);
}
