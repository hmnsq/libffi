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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

فراغ *ffi_prep_دلیلونه(حرف *ډېری, extended_cif *ecif)
{
  سجل بینښه صحيح i;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;
  سجل صحيح شمېر = 0;

  p_argv = ecif->avalue;
  argp = ډېری;

  لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->اندازه;

      إذا ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = حجم(فراغ*);
	  *(فراغ **) argp = *p_argv;
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
      وإلا إذا (z < حجم(صحيح))
	{
	  z = حجم(صحيح);
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(نښهدار صحيح *) argp = (نښهدار صحيح)*(SINT8 *)(* p_argv);
	      اكسر;
	      
	    حالة FFI_TYPE_UINT8:
	      *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT8 *)(* p_argv);
	      اكسر;
	      
	    حالة FFI_TYPE_SINT16:
	      *(نښهدار صحيح *) argp = (نښهدار صحيح)*(SINT16 *)(* p_argv);
	      اكسر;
		  
	    حالة FFI_TYPE_UINT16:
	      *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT16 *)(* p_argv);
	      اكسر;
		  
	    افتراضي:
	      FFI_ASSERT(0);
	    }
	}
      وإلا إذا (z == حجم(صحيح))
	{
	  *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT32 *)(* p_argv);
	}
      وإلا
	{
	  حافظه_نقل(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      شمېر += z;
    }

  أرجع (ډېری + ((شمېر > 24) ? 24 : FFI_ALIGN_ښکته(شمېر, 8)));
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  وإلا
    cif->flags = cif->rtype->اندازه;

  cif->بایټونه = FFI_ALIGN (cif->بایټونه, 8);

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_EABI(فراغ *(*)(حرف *, extended_cif *), 
			  extended_cif *, 
			  بینښه, بینښه, 
			  بینښه *, 
			  فراغ (*fn)(فراغ));

فراغ ffi_call(ffi_cif *cif, 
	      فراغ (*fn)(فراغ), 
	      فراغ *rvalue, 
	      فراغ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  إذا ((rvalue == عدم) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->اندازه);
    }
  وإلا
    ecif.rvalue = rvalue;
    
  
  بدل (cif->abi) 
    {
    حالة FFI_EABI:
      ffi_call_EABI(ffi_prep_دلیلونه, &ecif, cif->بایټونه, 
		    cif->flags, ecif.rvalue, fn);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}

فراغ ffi_تړون_eabi (بینښه arg1, بینښه arg2, بینښه arg3,
		       بینښه arg4, بینښه arg5, بینښه arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  سجل ffi_تړون *creg __asm__ ("gr7");
  ffi_تړون *تړون = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  سجل حرف *چوکاټ_اشاره_ګر __asm__ ("fp");
  حرف *ډېری_دلیلونه = چوکاټ_اشاره_ګر + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  بینښه ثبتوونکی_دلیلونه[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = تړون->cif;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue = alloca (cif->nargs * حجم(فراغ *));
  حرف *ptr = (حرف *) ثبتوونکی_دلیلونه;
  صحيح i;

  /* Find the address of each argument.  */
  لكل (i = 0; i < cif->nargs; i++)
    {
      بدل (arg_types[i]->type)
	{
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  اكسر;
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  اكسر;
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  اكسر;
	حالة FFI_TYPE_STRUCT:
	  avalue[i] = *(فراغ**)ptr;
	  اكسر;
	افتراضي:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  اكسر;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      إذا (ptr == ((حرف *)ثبتوونکی_دلیلونه + (6*4)))
	ptr = ډېری_دلیلونه;
    }

  /* Invoke the closure.  */
  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      سجل فراغ *return_struct_ptr __asm__("gr3");
      (تړون->fun) (cif, return_struct_ptr, avalue, تړون->user_معلومات);
    }
  وإلا
    {
      /* Allocate space for the return value and call the function.  */
      اوږد اوږد rvalue;
      (تړون->fun) (cif, &rvalue, avalue, تړون->user_معلومات);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((صحيح *) &rvalue)[1]));
    }
}

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون* تړون,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  بینښه صحيح *tramp = (بینښه صحيح *) &تړون->tramp[0];
  بینښه اوږد fn = (اوږد) ffi_تړون_eabi;
  بینښه اوږد cls = (اوږد) codeloc;
#که_تعریف_شوی __FRV_FDPIC__
  سجل فراغ *got __asm__("gr15");
#د_که_پای
  صحيح i;

  fn = (بینښه اوږد) ffi_تړون_eabi;

#که_تعریف_شوی __FRV_FDPIC__
  tramp[0] = &((بینښه صحيح *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#وإلا
  tramp[0] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[1] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[2] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[3] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[4] = 0x80300006;                  /* jmpl @(gr0, gr6)      */
#د_که_پای

  تړون->cif = cif;
  تړون->fun = fun;
  تړون->user_معلومات = user_معلومات;

  /* Cache flushing.  */
  لكل (i = 0; i < FFI_TRAMPOLINE_اندازه; i++)
    __asm__ متطاير ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  أرجع FFI_OK;
}
