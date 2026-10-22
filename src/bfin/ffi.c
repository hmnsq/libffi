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
#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdio.h>

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
শূন্য ffi_prep_args(unsigned বর্ণ *, extended_cif *);

/*====================================================================*/
/*                          Externals                                 */
/*                          (Assembly)                                */
/*====================================================================*/

বাহ্য শূন্য ffi_call_SYSV(unsigned, extended_cif *, শূন্য(*)(unsigned বর্ণ *, extended_cif *), unsigned, শূন্য *, শূন্য(*fn)(শূন্য));

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
   বিকল্প (cif->rtype->type) {
      পক্ষ FFI_TYPE_VOID:
         cif->flags = FFIBFIN_RET_VOID;
         ভঙ্গ;
      পক্ষ FFI_TYPE_UINT16:
      পক্ষ FFI_TYPE_SINT16:
         cif->flags = FFIBFIN_RET_HALFWORD;
         ভঙ্গ;
      পক্ষ FFI_TYPE_UINT8:
         cif->flags = FFIBFIN_RET_BYTE;
         ভঙ্গ;
      পক্ষ FFI_TYPE_INT:
      পক্ষ FFI_TYPE_UINT32:
      পক্ষ FFI_TYPE_SINT32:
      পক্ষ FFI_TYPE_FLOAT:
      পক্ষ FFI_TYPE_POINTER:
      পক্ষ FFI_TYPE_SINT8:
         cif->flags = FFIBFIN_RET_INT32;
         ভঙ্গ;
      পক্ষ FFI_TYPE_SINT64:
      পক্ষ FFI_TYPE_UINT64:
      পক্ষ FFI_TYPE_DOUBLE:
          cif->flags = FFIBFIN_RET_INT64;
          ভঙ্গ;
      পক্ষ FFI_TYPE_STRUCT:
         যদি (cif->rtype->size <= 4){
        	 cif->flags = FFIBFIN_RET_INT32;
         }অন্যথা যদি (cif->rtype->size == 8){
        	 cif->flags = FFIBFIN_RET_INT64;
         }অন্যথা{
        	 //it will return via a hidden pointer in P0
        	 cif->flags = FFIBFIN_RET_VOID;
         }
         ভঙ্গ;
      স্বভাব:
         FFI_ASSERT(0);
         ভঙ্গ;
   }
   প্রত্যাগম FFI_OK;
}

/*
 * This will prepare the arguments and will call the assembly routine
 * cif = the call interface
 * fn = the function to be called
 * rvalue = the return value
 * avalue = the arguments
 */
শূন্য ffi_call(ffi_cif *cif, শূন্য(*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
   পূর্ণাঙ্ক ret_type = cif->flags;
   extended_cif ecif;
   ecif.cif = cif;
   ecif.avalue = avalue;
   ecif.rvalue = rvalue;

   বিকল্প (cif->abi) {
      পক্ষ FFI_SYSV:
         ffi_call_SYSV(cif->bytes, &ecif, ffi_prep_args, ret_type, ecif.rvalue, fn);
         ভঙ্গ;
      স্বভাব:
         FFI_ASSERT(0);
         ভঙ্গ;
   }
}


/*
* This function prepares the parameters (copies them from the ecif to the stack)
*  to call the function (ffi_prep_args is called by the assembly routine in file
*  sysv.S, which also calls the actual function)
*/
শূন্য ffi_prep_args(unsigned বর্ণ *স্তূপ, extended_cif *ecif)
{
   পঞ্জিকা unsigned পূর্ণাঙ্ক i = 0;
   শূন্য **p_argv;
   unsigned বর্ণ *argp;
   ffi_type **p_arg;
   argp = স্তূপ;
   p_argv = ecif->avalue;
   প্রতি (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
        (i != 0);
        i--, p_arg++) {
      size_t z;
      z = (*p_arg)->size;
      যদি (z < পরিমাণ(পূর্ণাঙ্ক)) {
         z = পরিমাণ(পূর্ণাঙ্ক);
         বিকল্প ((*p_arg)->type) {
            পক্ষ FFI_TYPE_SINT8: {
                  signed বর্ণ v = *(SINT8 *)(* p_argv);
                  signed পূর্ণাঙ্ক t = v;
                  *(signed পূর্ণাঙ্ক *) argp = t;
               }
               ভঙ্গ;
            পক্ষ FFI_TYPE_UINT8: {
                  unsigned বর্ণ v = *(UINT8 *)(* p_argv);
                  unsigned পূর্ণাঙ্ক t = v;
                  *(unsigned পূর্ণাঙ্ক *) argp = t;
               }
               ভঙ্গ;
            পক্ষ FFI_TYPE_SINT16:
               *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক) * (SINT16 *)(* p_argv);
               ভঙ্গ;
            পক্ষ FFI_TYPE_UINT16:
               *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক) * (UINT16 *)(* p_argv);
               ভঙ্গ;
            পক্ষ FFI_TYPE_STRUCT:
               memcpy(argp, *p_argv, (*p_arg)->size);
               ভঙ্গ;
            স্বভাব:
               FFI_ASSERT(0);
               ভঙ্গ;
         }
      } অন্যথা যদি (z == পরিমাণ(পূর্ণাঙ্ক)) {
         *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক) * (UINT32 *)(* p_argv);
      } অন্যথা {
         memcpy(argp, *p_argv, z);
      }
      p_argv++;
      argp += z;
   }
}



