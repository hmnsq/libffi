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
#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>
#समावेश <stdio.h>

/* Maximum number of GPRs available for argument passing.  */
#घोषणा MAX_GPRARGS 3

/*
 * Return types
 */
#घोषणा FFIBFIN_RET_VOID 0
#घोषणा FFIBFIN_RET_BYTE 1
#घोषणा FFIBFIN_RET_HALFWORD 2
#घोषणा FFIBFIN_RET_INT64 3
#घोषणा FFIBFIN_RET_INT32 4

/*====================================================================*/
/*                          PROTOTYPE          *
 /*====================================================================*/
व्योम ffi_prep_args(अचिन्हित अक्षर *, extended_cif *);

/*====================================================================*/
/*                          Externals                                 */
/*                          (Assembly)                                */
/*====================================================================*/

बाह्य व्योम ffi_call_SYSV(अचिन्हित, extended_cif *, व्योम(*)(अचिन्हित अक्षर *, extended_cif *), अचिन्हित, व्योम *, व्योम(*fn)(व्योम));

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
   चयन (cif->rtype->type) {
      हाल FFI_TYPE_VOID:
         cif->flags = FFIBFIN_RET_VOID;
         अवरोध;
      हाल FFI_TYPE_UINT16:
      हाल FFI_TYPE_SINT16:
         cif->flags = FFIBFIN_RET_HALFWORD;
         अवरोध;
      हाल FFI_TYPE_UINT8:
         cif->flags = FFIBFIN_RET_BYTE;
         अवरोध;
      हाल FFI_TYPE_INT:
      हाल FFI_TYPE_UINT32:
      हाल FFI_TYPE_SINT32:
      हाल FFI_TYPE_FLOAT:
      हाल FFI_TYPE_POINTER:
      हाल FFI_TYPE_SINT8:
         cif->flags = FFIBFIN_RET_INT32;
         अवरोध;
      हाल FFI_TYPE_SINT64:
      हाल FFI_TYPE_UINT64:
      हाल FFI_TYPE_DOUBLE:
          cif->flags = FFIBFIN_RET_INT64;
          अवरोध;
      हाल FFI_TYPE_STRUCT:
         अगर (cif->rtype->आकार <= 4){
        	 cif->flags = FFIBFIN_RET_INT32;
         }अन्यथा अगर (cif->rtype->आकार == 8){
        	 cif->flags = FFIBFIN_RET_INT64;
         }अन्यथा{
        	 //it will return via a hidden pointer in P0
        	 cif->flags = FFIBFIN_RET_VOID;
         }
         अवरोध;
      शेष:
         FFI_ASSERT(0);
         अवरोध;
   }
   वापस FFI_OK;
}

/*
 * This will prepare the arguments and will call the assembly routine
 * cif = the call interface
 * fn = the function to be called
 * rvalue = the return value
 * avalue = the arguments
 */
व्योम ffi_call(ffi_cif *cif, व्योम(*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
   पूर्णांक ret_type = cif->flags;
   extended_cif ecif;
   ecif.cif = cif;
   ecif.avalue = avalue;
   ecif.rvalue = rvalue;

   चयन (cif->abi) {
      हाल FFI_SYSV:
         ffi_call_SYSV(cif->bytes, &ecif, ffi_prep_args, ret_type, ecif.rvalue, fn);
         अवरोध;
      शेष:
         FFI_ASSERT(0);
         अवरोध;
   }
}


/*
* This function prepares the parameters (copies them from the ecif to the stack)
*  to call the function (ffi_prep_args is called by the assembly routine in file
*  sysv.S, which also calls the actual function)
*/
व्योम ffi_prep_args(अचिन्हित अक्षर *ढेर, extended_cif *ecif)
{
   रेजिस्टर अचिन्हित पूर्णांक i = 0;
   व्योम **p_argv;
   अचिन्हित अक्षर *argp;
   ffi_type **p_arg;
   argp = ढेर;
   p_argv = ecif->avalue;
   क्रम (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
        (i != 0);
        i--, p_arg++) {
      माप_प्रकार z;
      z = (*p_arg)->आकार;
      अगर (z < माप(पूर्णांक)) {
         z = माप(पूर्णांक);
         चयन ((*p_arg)->type) {
            हाल FFI_TYPE_SINT8: {
                  चिन्हित अक्षर v = *(SINT8 *)(* p_argv);
                  चिन्हित पूर्णांक t = v;
                  *(चिन्हित पूर्णांक *) argp = t;
               }
               अवरोध;
            हाल FFI_TYPE_UINT8: {
                  अचिन्हित अक्षर v = *(UINT8 *)(* p_argv);
                  अचिन्हित पूर्णांक t = v;
                  *(अचिन्हित पूर्णांक *) argp = t;
               }
               अवरोध;
            हाल FFI_TYPE_SINT16:
               *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक) * (SINT16 *)(* p_argv);
               अवरोध;
            हाल FFI_TYPE_UINT16:
               *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक) * (UINT16 *)(* p_argv);
               अवरोध;
            हाल FFI_TYPE_STRUCT:
               स_नकल(argp, *p_argv, (*p_arg)->आकार);
               अवरोध;
            शेष:
               FFI_ASSERT(0);
               अवरोध;
         }
      } अन्यथा अगर (z == माप(पूर्णांक)) {
         *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक) * (UINT32 *)(* p_argv);
      } अन्यथा {
         स_नकल(argp, *p_argv, z);
      }
      p_argv++;
      argp += z;
   }
}



