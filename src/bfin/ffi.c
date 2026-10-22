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
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>

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
शून्य ffi_prep_args(अचिन्हित वर्ण *, extended_cif *);

/*====================================================================*/
/*                          Externals                                 */
/*                          (Assembly)                                */
/*====================================================================*/

बाह्य शून्य ffi_call_SYSV(अचिन्हित, extended_cif *, शून्य(*)(अचिन्हित वर्ण *, extended_cif *), अचिन्हित, शून्य *, शून्य(*fn)(शून्य));

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
   विकल्प (cif->rtype->type) {
      पक्ष FFI_TYPE_VOID:
         cif->flags = FFIBFIN_RET_VOID;
         रोक;
      पक्ष FFI_TYPE_UINT16:
      पक्ष FFI_TYPE_SINT16:
         cif->flags = FFIBFIN_RET_HALFWORD;
         रोक;
      पक्ष FFI_TYPE_UINT8:
         cif->flags = FFIBFIN_RET_BYTE;
         रोक;
      पक्ष FFI_TYPE_INT:
      पक्ष FFI_TYPE_UINT32:
      पक्ष FFI_TYPE_SINT32:
      पक्ष FFI_TYPE_FLOAT:
      पक्ष FFI_TYPE_POINTER:
      पक्ष FFI_TYPE_SINT8:
         cif->flags = FFIBFIN_RET_INT32;
         रोक;
      पक्ष FFI_TYPE_SINT64:
      पक्ष FFI_TYPE_UINT64:
      पक्ष FFI_TYPE_DOUBLE:
          cif->flags = FFIBFIN_RET_INT64;
          रोक;
      पक्ष FFI_TYPE_STRUCT:
         यदि (cif->rtype->size <= 4){
        	 cif->flags = FFIBFIN_RET_INT32;
         }अन्यथा यदि (cif->rtype->size == 8){
        	 cif->flags = FFIBFIN_RET_INT64;
         }अन्यथा{
        	 //it will return via a hidden pointer in P0
        	 cif->flags = FFIBFIN_RET_VOID;
         }
         रोक;
      स्वभाव:
         FFI_ASSERT(0);
         रोक;
   }
   फर्काऊ FFI_OK;
}

/*
 * This will prepare the arguments and will call the assembly routine
 * cif = the call interface
 * fn = the function to be called
 * rvalue = the return value
 * avalue = the arguments
 */
शून्य ffi_call(ffi_cif *cif, शून्य(*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
   पूर्णाङ्क ret_type = cif->flags;
   extended_cif ecif;
   ecif.cif = cif;
   ecif.avalue = avalue;
   ecif.rvalue = rvalue;

   विकल्प (cif->abi) {
      पक्ष FFI_SYSV:
         ffi_call_SYSV(cif->bytes, &ecif, ffi_prep_args, ret_type, ecif.rvalue, fn);
         रोक;
      स्वभाव:
         FFI_ASSERT(0);
         रोक;
   }
}


/*
* This function prepares the parameters (copies them from the ecif to the stack)
*  to call the function (ffi_prep_args is called by the assembly routine in file
*  sysv.S, which also calls the actual function)
*/
शून्य ffi_prep_args(अचिन्हित वर्ण *थाक, extended_cif *ecif)
{
   पञ्जिका अचिन्हित पूर्णाङ्क i = 0;
   शून्य **p_argv;
   अचिन्हित वर्ण *argp;
   ffi_type **p_arg;
   argp = थाक;
   p_argv = ecif->avalue;
   लागि (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
        (i != 0);
        i--, p_arg++) {
      माप_प्रकार z;
      z = (*p_arg)->size;
      यदि (z < परिमाण(पूर्णाङ्क)) {
         z = परिमाण(पूर्णाङ्क);
         विकल्प ((*p_arg)->type) {
            पक्ष FFI_TYPE_SINT8: {
                  चिन्हित वर्ण v = *(SINT8 *)(* p_argv);
                  चिन्हित पूर्णाङ्क t = v;
                  *(चिन्हित पूर्णाङ्क *) argp = t;
               }
               रोक;
            पक्ष FFI_TYPE_UINT8: {
                  अचिन्हित वर्ण v = *(UINT8 *)(* p_argv);
                  अचिन्हित पूर्णाङ्क t = v;
                  *(अचिन्हित पूर्णाङ्क *) argp = t;
               }
               रोक;
            पक्ष FFI_TYPE_SINT16:
               *(चिन्हित पूर्णाङ्क *) argp = (चिन्हित पूर्णाङ्क) * (SINT16 *)(* p_argv);
               रोक;
            पक्ष FFI_TYPE_UINT16:
               *(अचिन्हित पूर्णाङ्क *) argp = (अचिन्हित पूर्णाङ्क) * (UINT16 *)(* p_argv);
               रोक;
            पक्ष FFI_TYPE_STRUCT:
               स_नकल(argp, *p_argv, (*p_arg)->size);
               रोक;
            स्वभाव:
               FFI_ASSERT(0);
               रोक;
         }
      } अन्यथा यदि (z == परिमाण(पूर्णाङ्क)) {
         *(अचिन्हित पूर्णाङ्क *) argp = (अचिन्हित पूर्णाङ्क) * (UINT32 *)(* p_argv);
      } अन्यथा {
         स_नकल(argp, *p_argv, z);
      }
      p_argv++;
      argp += z;
   }
}



