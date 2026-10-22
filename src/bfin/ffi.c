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
#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>

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
ଶୂନ୍ୟ ffi_prep_args(unsigned ଅକ୍ଷର *, extended_cif *);

/*====================================================================*/
/*                          Externals                                 */
/*                          (Assembly)                                */
/*====================================================================*/

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV(unsigned, extended_cif *, ଶୂନ୍ୟ(*)(unsigned ଅକ୍ଷର *, extended_cif *), unsigned, ଶୂନ୍ୟ *, ଶୂନ୍ୟ(*fn)(ଶୂନ୍ୟ));

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
   ବାଛ (cif->rtype->type) {
      ସ୍ଥିତି FFI_TYPE_VOID:
         cif->flags = FFIBFIN_RET_VOID;
         ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_UINT16:
      ସ୍ଥିତି FFI_TYPE_SINT16:
         cif->flags = FFIBFIN_RET_HALFWORD;
         ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_UINT8:
         cif->flags = FFIBFIN_RET_BYTE;
         ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_INT:
      ସ୍ଥିତି FFI_TYPE_UINT32:
      ସ୍ଥିତି FFI_TYPE_SINT32:
      ସ୍ଥିତି FFI_TYPE_FLOAT:
      ସ୍ଥିତି FFI_TYPE_POINTER:
      ସ୍ଥିତି FFI_TYPE_SINT8:
         cif->flags = FFIBFIN_RET_INT32;
         ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_SINT64:
      ସ୍ଥିତି FFI_TYPE_UINT64:
      ସ୍ଥିତି FFI_TYPE_DOUBLE:
          cif->flags = FFIBFIN_RET_INT64;
          ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_STRUCT:
         ଯଦି (cif->rtype->size <= 4){
        	 cif->flags = FFIBFIN_RET_INT32;
         }ଅନ୍ୟଥା ଯଦି (cif->rtype->size == 8){
        	 cif->flags = FFIBFIN_RET_INT64;
         }ଅନ୍ୟଥା{
        	 //it will return via a hidden pointer in P0
        	 cif->flags = FFIBFIN_RET_VOID;
         }
         ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
         FFI_ASSERT(0);
         ଭାଙ୍ଗ;
   }
   ଫେରନ୍ତୁ FFI_OK;
}

/*
 * This will prepare the arguments and will call the assembly routine
 * cif = the call interface
 * fn = the function to be called
 * rvalue = the return value
 * avalue = the arguments
 */
ଶୂନ୍ୟ ffi_call(ffi_cif *cif, ଶୂନ୍ୟ(*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
   ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret_type = cif->flags;
   extended_cif ecif;
   ecif.cif = cif;
   ecif.avalue = avalue;
   ecif.rvalue = rvalue;

   ବାଛ (cif->abi) {
      ସ୍ଥିତି FFI_SYSV:
         ffi_call_SYSV(cif->bytes, &ecif, ffi_prep_args, ret_type, ecif.rvalue, fn);
         ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
         FFI_ASSERT(0);
         ଭାଙ୍ଗ;
   }
}


/*
* This function prepares the parameters (copies them from the ecif to the stack)
*  to call the function (ffi_prep_args is called by the assembly routine in file
*  sysv.S, which also calls the actual function)
*/
ଶୂନ୍ୟ ffi_prep_args(unsigned ଅକ୍ଷର *ଗଦା, extended_cif *ecif)
{
   ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
   ଶୂନ୍ୟ **p_argv;
   unsigned ଅକ୍ଷର *argp;
   ffi_type **p_arg;
   argp = ଗଦା;
   p_argv = ecif->avalue;
   ପାଇଁ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
        (i != 0);
        i--, p_arg++) {
      size_t z;
      z = (*p_arg)->size;
      ଯଦି (z < ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)) {
         z = ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
         ବାଛ ((*p_arg)->type) {
            ସ୍ଥିତି FFI_TYPE_SINT8: {
                  signed ଅକ୍ଷର v = *(SINT8 *)(* p_argv);
                  signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = v;
                  *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = t;
               }
               ଭାଙ୍ଗ;
            ସ୍ଥିତି FFI_TYPE_UINT8: {
                  unsigned ଅକ୍ଷର v = *(UINT8 *)(* p_argv);
                  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = v;
                  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = t;
               }
               ଭାଙ୍ଗ;
            ସ୍ଥିତି FFI_TYPE_SINT16:
               *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା) * (SINT16 *)(* p_argv);
               ଭାଙ୍ଗ;
            ସ୍ଥିତି FFI_TYPE_UINT16:
               *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) * (UINT16 *)(* p_argv);
               ଭାଙ୍ଗ;
            ସ୍ଥିତି FFI_TYPE_STRUCT:
               memcpy(argp, *p_argv, (*p_arg)->size);
               ଭାଙ୍ଗ;
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
               FFI_ASSERT(0);
               ଭାଙ୍ଗ;
         }
      } ଅନ୍ୟଥା ଯଦି (z == ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)) {
         *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) * (UINT32 *)(* p_argv);
      } ଅନ୍ୟଥା {
         memcpy(argp, *p_argv, z);
      }
      p_argv++;
      argp += z;
   }
}



