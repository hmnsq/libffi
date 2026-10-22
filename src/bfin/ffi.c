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
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

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
ಶೂನ್ಯ ffi_prep_args(unsigned ವರ್ಣ *, extended_cif *);

/*====================================================================*/
/*                          Externals                                 */
/*                          (Assembly)                                */
/*====================================================================*/

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(unsigned, extended_cif *, ಶೂನ್ಯ(*)(unsigned ವರ್ಣ *, extended_cif *), unsigned, ಶೂನ್ಯ *, ಶೂನ್ಯ(*fn)(ಶೂನ್ಯ));

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
   ವಿಕಲ್ಪ (cif->rtype->type) {
      ಪಕ್ಷ FFI_TYPE_VOID:
         cif->flags = FFIBFIN_RET_VOID;
         ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_UINT16:
      ಪಕ್ಷ FFI_TYPE_SINT16:
         cif->flags = FFIBFIN_RET_HALFWORD;
         ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_UINT8:
         cif->flags = FFIBFIN_RET_BYTE;
         ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_INT:
      ಪಕ್ಷ FFI_TYPE_UINT32:
      ಪಕ್ಷ FFI_TYPE_SINT32:
      ಪಕ್ಷ FFI_TYPE_FLOAT:
      ಪಕ್ಷ FFI_TYPE_POINTER:
      ಪಕ್ಷ FFI_TYPE_SINT8:
         cif->flags = FFIBFIN_RET_INT32;
         ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_SINT64:
      ಪಕ್ಷ FFI_TYPE_UINT64:
      ಪಕ್ಷ FFI_TYPE_DOUBLE:
          cif->flags = FFIBFIN_RET_INT64;
          ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_STRUCT:
         ಯದಿ (cif->rtype->size <= 4){
        	 cif->flags = FFIBFIN_RET_INT32;
         }ಅನ್ಯಥಾ ಯದಿ (cif->rtype->size == 8){
        	 cif->flags = FFIBFIN_RET_INT64;
         }ಅನ್ಯಥಾ{
        	 //it will return via a hidden pointer in P0
        	 cif->flags = FFIBFIN_RET_VOID;
         }
         ಭಙ್ಗ;
      ಸ್ವಭಾವ:
         FFI_ASSERT(0);
         ಭಙ್ಗ;
   }
   ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/*
 * This will prepare the arguments and will call the assembly routine
 * cif = the call interface
 * fn = the function to be called
 * rvalue = the return value
 * avalue = the arguments
 */
ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ(*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
   ಪೂರ್ಣಾಙ್ಕ ret_type = cif->flags;
   extended_cif ecif;
   ecif.cif = cif;
   ecif.avalue = avalue;
   ecif.rvalue = rvalue;

   ವಿಕಲ್ಪ (cif->abi) {
      ಪಕ್ಷ FFI_SYSV:
         ffi_call_SYSV(cif->bytes, &ecif, ffi_prep_args, ret_type, ecif.rvalue, fn);
         ಭಙ್ಗ;
      ಸ್ವಭಾವ:
         FFI_ASSERT(0);
         ಭಙ್ಗ;
   }
}


/*
* This function prepares the parameters (copies them from the ecif to the stack)
*  to call the function (ffi_prep_args is called by the assembly routine in file
*  sysv.S, which also calls the actual function)
*/
ಶೂನ್ಯ ffi_prep_args(unsigned ವರ್ಣ *ರಾಶಿ, extended_cif *ecif)
{
   ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i = 0;
   ಶೂನ್ಯ **p_argv;
   unsigned ವರ್ಣ *argp;
   ffi_type **p_arg;
   argp = ರಾಶಿ;
   p_argv = ecif->avalue;
   ಪ್ರತಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
        (i != 0);
        i--, p_arg++) {
      size_t z;
      z = (*p_arg)->size;
      ಯದಿ (z < ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)) {
         z = ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ);
         ವಿಕಲ್ಪ ((*p_arg)->type) {
            ಪಕ್ಷ FFI_TYPE_SINT8: {
                  signed ವರ್ಣ v = *(SINT8 *)(* p_argv);
                  signed ಪೂರ್ಣಾಙ್ಕ t = v;
                  *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = t;
               }
               ಭಙ್ಗ;
            ಪಕ್ಷ FFI_TYPE_UINT8: {
                  unsigned ವರ್ಣ v = *(UINT8 *)(* p_argv);
                  unsigned ಪೂರ್ಣಾಙ್ಕ t = v;
                  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = t;
               }
               ಭಙ್ಗ;
            ಪಕ್ಷ FFI_TYPE_SINT16:
               *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ) * (SINT16 *)(* p_argv);
               ಭಙ್ಗ;
            ಪಕ್ಷ FFI_TYPE_UINT16:
               *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ) * (UINT16 *)(* p_argv);
               ಭಙ್ಗ;
            ಪಕ್ಷ FFI_TYPE_STRUCT:
               memcpy(argp, *p_argv, (*p_arg)->size);
               ಭಙ್ಗ;
            ಸ್ವಭಾವ:
               FFI_ASSERT(0);
               ಭಙ್ಗ;
         }
      } ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)) {
         *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ) * (UINT32 *)(* p_argv);
      } ಅನ್ಯಥಾ {
         memcpy(argp, *p_argv, z);
      }
      p_argv++;
      argp += z;
   }
}



