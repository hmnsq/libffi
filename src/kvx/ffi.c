/* Copyright (c) 2020 Kalray

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
``Software''), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#إذا defined(__kvx__)
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <stdint.h>
#ضمن <fficonfig.h>
#ضمن <ffi.h>
#ضمن "ffi_common.h"
#ضمن "asm.h"

#تعریف_کن ALIGN(x, a) ALIGN_ماسک(x, (typeof(x))(a) - 1)
#تعریف_کن ALIGN_ماسک(x, ماسک) (((x) + (ماسک)) & ~(ماسک))
#تعریف_کن KVX_ABI_پشته_ALIGNMENT (32)
#تعریف_کن KVX_ABI_پشته_ARG_ALIGNMENT (8)
#تعریف_کن max(a,b) ((a) > (b) ? (a) : (b))

#اگر_تعریف_شده FFI_DEBUG
#تعریف_کن DEBUG_PRINT(...) افعل{ fprintf( stderr, __VA_ARGS__ ); } طالما(0)
#وإلا
#تعریف_کن DEBUG_PRINT(...)
#پایان_اگر

بنية ret_مقدار {
	بیعلامت دراز صحيح r0;
	بیعلامت دراز صحيح r1;
	بیعلامت دراز صحيح r2;
	بیعلامت دراز صحيح r3;
};

خارجي بنية ret_مقدار ffi_call_SYSV(بیعلامت مجموع_اندازه,
                                      بیعلامت اندازه,
                                      extended_cif *ecif,
                                      بیعلامت *rvalue_addr,
                                      فراغ *fn,
                                      بیعلامت int_ext_روش);

/* Perform machine dependent cif processing */
ffi_وضعیت ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->اندازه;
  أرجع FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

فراغ *ffi_prep_آرگومانها(حرف *پشته, بیعلامت صحيح arg_slots_اندازه, extended_cif *ecif)
{
  حرف *stacktemp = پشته;
  حرف *current_arg_passed_by_مقدار = پشته + arg_slots_اندازه;
  صحيح i, s;
  ffi_type **arg;
  صحيح شمار = 0;
  ffi_cif *cif = ecif->cif;
  فراغ **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", پشته);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_اندازه);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_مقدار);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  لكل (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_جایگاه_اندازه;
    بدل((*arg)->type) {
      حالة FFI_TYPE_SINT8:
      حالة FFI_TYPE_UINT8:
      حالة FFI_TYPE_SINT16:
      حالة FFI_TYPE_UINT16:
      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_FLOAT:
      حالة FFI_TYPE_DOUBLE:
      حالة FFI_TYPE_UINT64:
      حالة FFI_TYPE_SINT64:
      حالة FFI_TYPE_اشاره_گر:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", پشته);
        *(uint64_t *) پشته = *(uint64_t *)(* argv);
        اكسر;

      حالة FFI_TYPE_COMPLEX:
        إذا ((*arg)->اندازه == 8)
          *(_Complex عائم *) پشته = *(_Complex عائم *)(* argv);
        وإلا إذا ((*arg)->اندازه == 16) {
          *(_Complex مزدوج *) پشته = *(_Complex مزدوج *)(* argv);
          s = 16;
        } وإلا
          abort();
        اكسر;
      حالة FFI_TYPE_STRUCT: {
        حرف *مقدار;
        بیعلامت صحيح written_اندازه = 0;
        DEBUG_PRINT("struct by value @%p\n", پشته);
        إذا ((*arg)->اندازه > KVX_ABI_MAX_AGGREGATE_درون_REG_اندازه) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) پشته = (uintptr_t)current_arg_passed_by_مقدار;
          مقدار = current_arg_passed_by_مقدار;
          current_arg_passed_by_مقدار += (*arg)->اندازه;
          written_اندازه = KVX_ABI_جایگاه_اندازه;
        } وإلا {
          مقدار = پشته;
          written_اندازه = (*arg)->اندازه;
        }
        کپی_حافظه(مقدار, *argv, (*arg)->اندازه);
        s = ALIGN(written_اندازه, KVX_ABI_پشته_ARG_ALIGNMENT);
        اكسر;
      }
      افتراضي:
        چاپ_کن("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        اكسر;

    }
    پشته += s;
    شمار += s;
    argv++;
    arg++;
  }
#اگر_تعریف_شده FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_آرگومانها_اندازه) & (KVX_ABI_پشته_ALIGNMENT-1)) == 0);
#پایان_اگر
  أرجع stacktemp + REG_آرگومانها_اندازه;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_وضعیت ffi_prep_cif_machdep_var(ffi_cif *cif, بیعلامت صحيح nfixedargs,
                                    بیعلامت صحيح ntotalargs)
{
  cif->flags = cif->rtype->اندازه;
  أرجع FFI_OK;
}

ساكن بیعلامت دراز handle_small_int_ext(kvx_intext_روش *int_ext_روش,
                                          ثابت ffi_type *rtype)
{
  بدل (rtype->type) {
    حالة FFI_TYPE_SINT8:
      *int_ext_روش = KVX_RET_SXBD;
      أرجع KVX_ثبات_اندازه;

    حالة FFI_TYPE_SINT16:
      *int_ext_روش = KVX_RET_SXHD;
      أرجع KVX_ثبات_اندازه;

    حالة FFI_TYPE_SINT32:
      *int_ext_روش = KVX_RET_SXWD;
      أرجع KVX_ثبات_اندازه;

    حالة FFI_TYPE_UINT8:
      *int_ext_روش = KVX_RET_ZXBD;
      أرجع KVX_ثبات_اندازه;

    حالة FFI_TYPE_UINT16:
      *int_ext_روش = KVX_RET_ZXHD;
      أرجع KVX_ثبات_اندازه;

    حالة FFI_TYPE_UINT32:
      *int_ext_روش = KVX_RET_ZXWD;
      أرجع KVX_ثبات_اندازه;

    افتراضي:
      *int_ext_روش = KVX_RET_NONE;
      أرجع rtype->اندازه;
  }
}

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  صحيح i;
  بیعلامت دراز صحيح جایگاه_fitting_آرگومانها_اندازه = 0;
  بیعلامت دراز صحيح مجموع_اندازه = 0;
  بیعلامت دراز صحيح big_struct_اندازه = 0;
  kvx_intext_روش int_extension_روش;
  ffi_type **arg;
  بنية ret_مقدار local_rvalue = {0};
  size_t wb_اندازه;


  /* Calculate size to allocate on stack */
  لكل (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->اندازه);
    إذا (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      إذا ((*arg)->اندازه <= KVX_ABI_MAX_AGGREGATE_درون_REG_اندازه) {
        جایگاه_fitting_آرگومانها_اندازه += ALIGN((*arg)->اندازه, KVX_ABI_جایگاه_اندازه);
      } وإلا {
        جایگاه_fitting_آرگومانها_اندازه += KVX_ABI_جایگاه_اندازه; /* aggregate passed by reference */
        big_struct_اندازه += ALIGN((*arg)->اندازه, KVX_ABI_جایگاه_اندازه);
      }
    } وإلا إذا ((*arg)->اندازه <= KVX_ABI_جایگاه_اندازه) {
      جایگاه_fitting_آرگومانها_اندازه += KVX_ABI_جایگاه_اندازه;
    } وإلا {
      چاپ_کن("Error: unsupported arg size %ld arg type %d\n", (*arg)->اندازه, (*arg)->type);
      abort(); /* should never happen? */
    }
  }

  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  /* This implementation allocates anyway for all register based args */
  جایگاه_fitting_آرگومانها_اندازه = max(جایگاه_fitting_آرگومانها_اندازه, REG_آرگومانها_اندازه);
  مجموع_اندازه = جایگاه_fitting_آرگومانها_اندازه + big_struct_اندازه;
  مجموع_اندازه = ALIGN(مجموع_اندازه, KVX_ABI_پشته_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_اندازه = handle_small_int_ext(&int_extension_روش, cif->rtype);

  بدل (cif->abi) {
    حالة FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", مجموع_اندازه);
      DEBUG_PRINT("slot fitting args size: %lu\n", جایگاه_fitting_آرگومانها_اندازه);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_اندازه);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_روش);
      local_rvalue = ffi_call_SYSV(مجموع_اندازه, جایگاه_fitting_آرگومانها_اندازه,
                                   &ecif, rvalue, fn, int_extension_روش);
      إذا ((cif->flags <= KVX_ABI_MAX_AGGREGATE_درون_REG_اندازه)
          && (cif->rtype->type != FFI_TYPE_VOID))
        کپی_حافظه(rvalue, &local_rvalue, wb_اندازه);
      اكسر;
    افتراضي:
      abort();
      اكسر;
  }
}

/* Closures not supported yet */
ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی* بستگی,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_داده,
                      فراغ *codeloc)
{
  أرجع FFI_BAD_ABI;
}

#پایان_اگر /* (__kvx__) */
