/* -----------------------------------------------------------------------
   ffi.c - Copyright (C) 2004, 2026  Anthony Green
   Copyright (C) 2007  Free Software Foundation, Inc.
	   Copyright (C) 2008  Red Hat, Inc.
   
   FR-V Foreign Function Interface 

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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

خالی *ffi_prep_دلائل(حرف *ڈھیر, extended_cif *ecif)
{
  اندراج غیرعلامتی صحیح i;
  اندراج خالی **p_argv;
  اندراج حرف *argp;
  اندراج ffi_type **p_arg;
  اندراج صحیح گنتی = 0;

  p_argv = ecif->avalue;
  argp = ڈھیر;

  برائے (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->سائز;

      اگر ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = حجم(خالی*);
	  *(خالی **) argp = *p_argv;
	} 
      /*      if ((*p_arg)->type == FFI_TYPE_FLOAT)
	{
	  if (count > 24)
	    {
	      // This is going on the stack.  Turn it into a double.  
	      *(double *) argp = (double) *(float*)(* p_argv);
	      z = sizeof(double);
	    }
	  else
	    *(void **) argp = *(void **)(* p_argv);
	}  */
      ورنہ اگر (z < حجم(صحیح))
	{
	  z = حجم(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(علامتی صحیح *) argp = (علامتی صحیح)*(SINT8 *)(* p_argv);
	      توڑو;
	      
	    حالت FFI_TYPE_UINT8:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT8 *)(* p_argv);
	      توڑو;
	      
	    حالت FFI_TYPE_SINT16:
	      *(علامتی صحیح *) argp = (علامتی صحیح)*(SINT16 *)(* p_argv);
	      توڑو;
		  
	    حالت FFI_TYPE_UINT16:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT16 *)(* p_argv);
	      توڑو;
		  
	    طےشدہ:
	      FFI_ASSERT(0);
	    }
	}
      ورنہ اگر (z == حجم(صحیح))
	{
	  *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT32 *)(* p_argv);
	}
      ورنہ
	{
	  حافظہ_نقل(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      گنتی += z;
    }

  واپس (ڈھیر + ((گنتی > 24) ? 24 : FFI_ALIGN_نیچے_کو(گنتی, 8)));
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  ورنہ
    cif->flags = cif->rtype->سائز;

  cif->بائٹیں = FFI_ALIGN (cif->بائٹیں, 8);

  واپس FFI_OK;
}

خارجی خالی ffi_call_EABI(خالی *(*)(حرف *, extended_cif *), 
			  extended_cif *, 
			  غیرعلامتی, غیرعلامتی, 
			  غیرعلامتی *, 
			  خالی (*fn)(خالی));

خالی ffi_call(ffi_cif *cif, 
	      خالی (*fn)(خالی), 
	      خالی *rvalue, 
	      خالی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  اگر ((rvalue == تہی) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->سائز);
    }
  ورنہ
    ecif.rvalue = rvalue;
    
  
  انتخاب (cif->abi) 
    {
    حالت FFI_EABI:
      ffi_call_EABI(ffi_prep_دلائل, &ecif, cif->بائٹیں, 
		    cif->flags, ecif.rvalue, fn);
      توڑو;
    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }
}

خالی ffi_احاطہ_eabi (غیرعلامتی arg1, غیرعلامتی arg2, غیرعلامتی arg3,
		       غیرعلامتی arg4, غیرعلامتی arg5, غیرعلامتی arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  اندراج ffi_احاطہ *creg __asm__ ("gr7");
  ffi_احاطہ *احاطہ = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  اندراج حرف *چوکھٹا_اشارہ_گر __asm__ ("fp");
  حرف *ڈھیر_دلائل = چوکھٹا_اشارہ_گر + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  غیرعلامتی رجسٹر_دلائل[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = احاطہ->cif;
  ffi_type **arg_types = cif->arg_types;
  خالی **avalue = alloca (cif->nargs * حجم(خالی *));
  حرف *ptr = (حرف *) رجسٹر_دلائل;
  صحیح i;

  /* Find the address of each argument.  */
  برائے (i = 0; i < cif->nargs; i++)
    {
      انتخاب (arg_types[i]->type)
	{
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  توڑو;
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  توڑو;
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  توڑو;
	حالت FFI_TYPE_STRUCT:
	  avalue[i] = *(خالی**)ptr;
	  توڑو;
	طےشدہ:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  توڑو;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      اگر (ptr == ((حرف *)رجسٹر_دلائل + (6*4)))
	ptr = ڈھیر_دلائل;
    }

  /* Invoke the closure.  */
  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      اندراج خالی *return_struct_ptr __asm__("gr3");
      (احاطہ->fun) (cif, return_struct_ptr, avalue, احاطہ->user_ڈیٹا);
    }
  ورنہ
    {
      /* Allocate space for the return value and call the function.  */
      طویل طویل rvalue;
      (احاطہ->fun) (cif, &rvalue, avalue, احاطہ->user_ڈیٹا);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((صحیح *) &rvalue)[1]));
    }
}

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  غیرعلامتی صحیح *tramp = (غیرعلامتی صحیح *) &احاطہ->tramp[0];
  غیرعلامتی طویل fn = (طویل) ffi_احاطہ_eabi;
  غیرعلامتی طویل cls = (طویل) codeloc;
#اگر_تعریف_شدہ __FRV_FDPIC__
  اندراج خالی *got __asm__("gr15");
#اگر_ختم
  صحیح i;

  fn = (غیرعلامتی طویل) ffi_احاطہ_eabi;

#اگر_تعریف_شدہ __FRV_FDPIC__
  tramp[0] = &((غیرعلامتی صحیح *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#ورنہ
  tramp[0] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[1] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[2] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[3] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[4] = 0x80300006;                  /* jmpl @(gr0, gr6)      */
#اگر_ختم

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  /* Cache flushing.  */
  برائے (i = 0; i < FFI_TRAMPOLINE_سائز; i++)
    __asm__ متلون ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  واپس FFI_OK;
}
