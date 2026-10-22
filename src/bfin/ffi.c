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
#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdio.h>

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
ശൂന്യം ffi_prep_args(unsigned അക്ഷരം *, extended_cif *);

/*====================================================================*/
/*                          Externals                                 */
/*                          (Assembly)                                */
/*====================================================================*/

ബാഹ്യ ശൂന്യം ffi_call_SYSV(unsigned, extended_cif *, ശൂന്യം(*)(unsigned അക്ഷരം *, extended_cif *), unsigned, ശൂന്യം *, ശൂന്യം(*fn)(ശൂന്യം));

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
   തിരഞ്ഞെടുപ്പ് (cif->rtype->type) {
      അവസ്ഥ FFI_TYPE_VOID:
         cif->flags = FFIBFIN_RET_VOID;
         നിർത്തുക;
      അവസ്ഥ FFI_TYPE_UINT16:
      അവസ്ഥ FFI_TYPE_SINT16:
         cif->flags = FFIBFIN_RET_HALFWORD;
         നിർത്തുക;
      അവസ്ഥ FFI_TYPE_UINT8:
         cif->flags = FFIBFIN_RET_BYTE;
         നിർത്തുക;
      അവസ്ഥ FFI_TYPE_INT:
      അവസ്ഥ FFI_TYPE_UINT32:
      അവസ്ഥ FFI_TYPE_SINT32:
      അവസ്ഥ FFI_TYPE_FLOAT:
      അവസ്ഥ FFI_TYPE_POINTER:
      അവസ്ഥ FFI_TYPE_SINT8:
         cif->flags = FFIBFIN_RET_INT32;
         നിർത്തുക;
      അവസ്ഥ FFI_TYPE_SINT64:
      അവസ്ഥ FFI_TYPE_UINT64:
      അവസ്ഥ FFI_TYPE_DOUBLE:
          cif->flags = FFIBFIN_RET_INT64;
          നിർത്തുക;
      അവസ്ഥ FFI_TYPE_STRUCT:
         എങ്കിൽ (cif->rtype->size <= 4){
        	 cif->flags = FFIBFIN_RET_INT32;
         }അല്ലെങ്കിൽ എങ്കിൽ (cif->rtype->size == 8){
        	 cif->flags = FFIBFIN_RET_INT64;
         }അല്ലെങ്കിൽ{
        	 //it will return via a hidden pointer in P0
        	 cif->flags = FFIBFIN_RET_VOID;
         }
         നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
         FFI_ASSERT(0);
         നിർത്തുക;
   }
   തിരികെ FFI_OK;
}

/*
 * This will prepare the arguments and will call the assembly routine
 * cif = the call interface
 * fn = the function to be called
 * rvalue = the return value
 * avalue = the arguments
 */
ശൂന്യം ffi_call(ffi_cif *cif, ശൂന്യം(*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
   പൂർണ്ണസംഖ്യ ret_type = cif->flags;
   extended_cif ecif;
   ecif.cif = cif;
   ecif.avalue = avalue;
   ecif.rvalue = rvalue;

   തിരഞ്ഞെടുപ്പ് (cif->abi) {
      അവസ്ഥ FFI_SYSV:
         ffi_call_SYSV(cif->bytes, &ecif, ffi_prep_args, ret_type, ecif.rvalue, fn);
         നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
         FFI_ASSERT(0);
         നിർത്തുക;
   }
}


/*
* This function prepares the parameters (copies them from the ecif to the stack)
*  to call the function (ffi_prep_args is called by the assembly routine in file
*  sysv.S, which also calls the actual function)
*/
ശൂന്യം ffi_prep_args(unsigned അക്ഷരം *അട്ടി, extended_cif *ecif)
{
   രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ i = 0;
   ശൂന്യം **p_argv;
   unsigned അക്ഷരം *argp;
   ffi_type **p_arg;
   argp = അട്ടി;
   p_argv = ecif->avalue;
   വേണ്ടി (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
        (i != 0);
        i--, p_arg++) {
      size_t z;
      z = (*p_arg)->size;
      എങ്കിൽ (z < വലുപ്പം(പൂർണ്ണസംഖ്യ)) {
         z = വലുപ്പം(പൂർണ്ണസംഖ്യ);
         തിരഞ്ഞെടുപ്പ് ((*p_arg)->type) {
            അവസ്ഥ FFI_TYPE_SINT8: {
                  signed അക്ഷരം v = *(SINT8 *)(* p_argv);
                  signed പൂർണ്ണസംഖ്യ t = v;
                  *(signed പൂർണ്ണസംഖ്യ *) argp = t;
               }
               നിർത്തുക;
            അവസ്ഥ FFI_TYPE_UINT8: {
                  unsigned അക്ഷരം v = *(UINT8 *)(* p_argv);
                  unsigned പൂർണ്ണസംഖ്യ t = v;
                  *(unsigned പൂർണ്ണസംഖ്യ *) argp = t;
               }
               നിർത്തുക;
            അവസ്ഥ FFI_TYPE_SINT16:
               *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ) * (SINT16 *)(* p_argv);
               നിർത്തുക;
            അവസ്ഥ FFI_TYPE_UINT16:
               *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ) * (UINT16 *)(* p_argv);
               നിർത്തുക;
            അവസ്ഥ FFI_TYPE_STRUCT:
               memcpy(argp, *p_argv, (*p_arg)->size);
               നിർത്തുക;
            സ്ഥിരസ്ഥിതി:
               FFI_ASSERT(0);
               നിർത്തുക;
         }
      } അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം(പൂർണ്ണസംഖ്യ)) {
         *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ) * (UINT32 *)(* p_argv);
      } അല്ലെങ്കിൽ {
         memcpy(argp, *p_argv, z);
      }
      p_argv++;
      argp += z;
   }
}



