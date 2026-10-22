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
#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>
#شامل <stdio.h>

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
تهی ffi_prep_args(unsigned نویسه *, extended_cif *);

/*====================================================================*/
/*                          Externals                                 */
/*                          (Assembly)                                */
/*====================================================================*/

بیرونی تهی ffi_call_SYSV(unsigned, extended_cif *, تهی(*)(unsigned نویسه *, extended_cif *), unsigned, تهی *, تهی(*fn)(تهی));

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
   انتخاب (cif->rtype->type) {
      حالت FFI_TYPE_VOID:
         cif->flags = FFIBFIN_RET_VOID;
         بشکن;
      حالت FFI_TYPE_UINT16:
      حالت FFI_TYPE_SINT16:
         cif->flags = FFIBFIN_RET_HALFWORD;
         بشکن;
      حالت FFI_TYPE_UINT8:
         cif->flags = FFIBFIN_RET_BYTE;
         بشکن;
      حالت FFI_TYPE_INT:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_FLOAT:
      حالت FFI_TYPE_POINTER:
      حالت FFI_TYPE_SINT8:
         cif->flags = FFIBFIN_RET_INT32;
         بشکن;
      حالت FFI_TYPE_SINT64:
      حالت FFI_TYPE_UINT64:
      حالت FFI_TYPE_DOUBLE:
          cif->flags = FFIBFIN_RET_INT64;
          بشکن;
      حالت FFI_TYPE_STRUCT:
         اگر (cif->rtype->size <= 4){
        	 cif->flags = FFIBFIN_RET_INT32;
         }وگرنه اگر (cif->rtype->size == 8){
        	 cif->flags = FFIBFIN_RET_INT64;
         }وگرنه{
        	 //it will return via a hidden pointer in P0
        	 cif->flags = FFIBFIN_RET_VOID;
         }
         بشکن;
      پیشفرض:
         FFI_ASSERT(0);
         بشکن;
   }
   بازگشت FFI_OK;
}

/*
 * This will prepare the arguments and will call the assembly routine
 * cif = the call interface
 * fn = the function to be called
 * rvalue = the return value
 * avalue = the arguments
 */
تهی ffi_call(ffi_cif *cif, تهی(*fn)(تهی), تهی *rvalue, تهی **avalue)
{
   صحیح ret_type = cif->flags;
   extended_cif ecif;
   ecif.cif = cif;
   ecif.avalue = avalue;
   ecif.rvalue = rvalue;

   انتخاب (cif->abi) {
      حالت FFI_SYSV:
         ffi_call_SYSV(cif->bytes, &ecif, ffi_prep_args, ret_type, ecif.rvalue, fn);
         بشکن;
      پیشفرض:
         FFI_ASSERT(0);
         بشکن;
   }
}


/*
* This function prepares the parameters (copies them from the ecif to the stack)
*  to call the function (ffi_prep_args is called by the assembly routine in file
*  sysv.S, which also calls the actual function)
*/
تهی ffi_prep_args(unsigned نویسه *پشته, extended_cif *ecif)
{
   ثبات unsigned صحیح i = 0;
   تهی **p_argv;
   unsigned نویسه *argp;
   ffi_type **p_arg;
   argp = پشته;
   p_argv = ecif->avalue;
   برای (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
        (i != 0);
        i--, p_arg++) {
      size_t z;
      z = (*p_arg)->size;
      اگر (z < اندازه(صحیح)) {
         z = اندازه(صحیح);
         انتخاب ((*p_arg)->type) {
            حالت FFI_TYPE_SINT8: {
                  signed نویسه v = *(SINT8 *)(* p_argv);
                  signed صحیح t = v;
                  *(signed صحیح *) argp = t;
               }
               بشکن;
            حالت FFI_TYPE_UINT8: {
                  unsigned نویسه v = *(UINT8 *)(* p_argv);
                  unsigned صحیح t = v;
                  *(unsigned صحیح *) argp = t;
               }
               بشکن;
            حالت FFI_TYPE_SINT16:
               *(signed صحیح *) argp = (signed صحیح) * (SINT16 *)(* p_argv);
               بشکن;
            حالت FFI_TYPE_UINT16:
               *(unsigned صحیح *) argp = (unsigned صحیح) * (UINT16 *)(* p_argv);
               بشکن;
            حالت FFI_TYPE_STRUCT:
               memcpy(argp, *p_argv, (*p_arg)->size);
               بشکن;
            پیشفرض:
               FFI_ASSERT(0);
               بشکن;
         }
      } وگرنه اگر (z == اندازه(صحیح)) {
         *(unsigned صحیح *) argp = (unsigned صحیح) * (UINT32 *)(* p_argv);
      } وگرنه {
         memcpy(argp, *p_argv, z);
      }
      p_argv++;
      argp += z;
   }
}



