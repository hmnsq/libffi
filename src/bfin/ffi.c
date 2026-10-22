/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2012  Alexandre K. I. de Mendonca <alexandre.keunecke@gmail.com>,
							   Paulo Pizarro <paulo.pizarro@gmail.com>

   Blackfin Foreign Function Interface

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
#ضمن <stdio.h>

/* Maximum number of GPRs available for argument passing.  */
#define MAX_GPRARGS 3

/*
 * Return types
 */
#define FFIBFIN_RET_VOID 0
#define FFIBFIN_RET_BYTE 1
#define FFIBFIN_RET_HALFWORD 2
#define FFIBFIN_RET_INT64 3
#define FFIBFIN_RET_INT32 4

/*====================================================================*/
/*                          PROTOTYPE          *
 /*====================================================================*/
فراغ ffi_prep_حجوجي(unsigned حرف *, extended_cif *);

/*====================================================================*/
/*                          Externals                                 */
/*                          (Assembly)                                */
/*====================================================================*/

خارجي فراغ ffi_call_SYSV(unsigned, extended_cif *, فراغ(*)(unsigned حرف *, extended_cif *), unsigned, فراغ *, فراغ(*fn)(فراغ));

/*====================================================================*/
/*                          Implementation                            */
/*                                                            */
/*====================================================================*/


/*
 * This function calculates the return type (size) based on type.
 */

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
   /* --------------------------------------*
    *   Return handling                *
    * --------------------------------------*/
   بدل (cif->rtype->type) {
      حالة FFI_TYPE_VOID:
         cif->flags = FFIBFIN_RET_VOID;
         اكسر;
      حالة FFI_TYPE_UINT16:
      حالة FFI_TYPE_SINT16:
         cif->flags = FFIBFIN_RET_HALFWORD;
         اكسر;
      حالة FFI_TYPE_UINT8:
         cif->flags = FFIBFIN_RET_BYTE;
         اكسر;
      حالة FFI_TYPE_INT:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_FLOAT:
      حالة FFI_TYPE_POINTER:
      حالة FFI_TYPE_SINT8:
         cif->flags = FFIBFIN_RET_INT32;
         اكسر;
      حالة FFI_TYPE_SINT64:
      حالة FFI_TYPE_UINT64:
      حالة FFI_TYPE_DOUBLE:
          cif->flags = FFIBFIN_RET_INT64;
          اكسر;
      حالة FFI_TYPE_STRUCT:
         إذا (cif->rtype->size <= 4){
        	 cif->flags = FFIBFIN_RET_INT32;
         }وإلا إذا (cif->rtype->size == 8){
        	 cif->flags = FFIBFIN_RET_INT64;
         }وإلا{
        	 //it will return via a hidden pointer in P0
        	 cif->flags = FFIBFIN_RET_VOID;
         }
         اكسر;
      افتراضي:
         FFI_ASSERT(0);
         اكسر;
   }
   أرجع FFI_OK;
}

/*
 * This will prepare the arguments and will call the assembly routine
 * cif = the call interface
 * fn = the function to be called
 * rvalue = the return value
 * avalue = the arguments
 */
فراغ ffi_call(ffi_cif *cif, فراغ(*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
   صحيح ret_type = cif->flags;
   extended_cif ecif;
   ecif.cif = cif;
   ecif.avalue = avalue;
   ecif.rvalue = rvalue;

   بدل (cif->abi) {
      حالة FFI_SYSV:
         ffi_call_SYSV(cif->bytes, &ecif, ffi_prep_حجوجي, ret_type, ecif.rvalue, fn);
         اكسر;
      افتراضي:
         FFI_ASSERT(0);
         اكسر;
   }
}


/*
* This function prepares the parameters (copies them from the ecif to the stack)
*  to call the function (ffi_prep_args is called by the assembly routine in file
*  sysv.S, which also calls the actual function)
*/
فراغ ffi_prep_حجوجي(unsigned حرف *تر, extended_cif *ecif)
{
   سجل unsigned صحيح i = 0;
   فراغ **p_argv;
   unsigned حرف *argp;
   ffi_type **p_arg;
   argp = تر;
   p_argv = ecif->avalue;
   لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
        (i != 0);
        i--, p_arg++) {
      size_t z;
      z = (*p_arg)->size;
      إذا (z < حجم(صحيح)) {
         z = حجم(صحيح);
         بدل ((*p_arg)->type) {
            حالة FFI_TYPE_SINT8: {
                  signed حرف v = *(SINT8 *)(* p_argv);
                  signed صحيح t = v;
                  *(signed صحيح *) argp = t;
               }
               اكسر;
            حالة FFI_TYPE_UINT8: {
                  unsigned حرف v = *(UINT8 *)(* p_argv);
                  unsigned صحيح t = v;
                  *(unsigned صحيح *) argp = t;
               }
               اكسر;
            حالة FFI_TYPE_SINT16:
               *(signed صحيح *) argp = (signed صحيح) * (SINT16 *)(* p_argv);
               اكسر;
            حالة FFI_TYPE_UINT16:
               *(unsigned صحيح *) argp = (unsigned صحيح) * (UINT16 *)(* p_argv);
               اكسر;
            حالة FFI_TYPE_STRUCT:
               memcpy(argp, *p_argv, (*p_arg)->size);
               اكسر;
            افتراضي:
               FFI_ASSERT(0);
               اكسر;
         }
      } وإلا إذا (z == حجم(صحيح)) {
         *(unsigned صحيح *) argp = (unsigned صحيح) * (UINT32 *)(* p_argv);
      } وإلا {
         memcpy(argp, *p_argv, z);
      }
      p_argv++;
      argp += z;
   }
}



