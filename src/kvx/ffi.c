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

#وصف_ڪريو ALIGN(x, a) ALIGN_نقاب(x, (typeof(x))(a) - 1)
#وصف_ڪريو ALIGN_نقاب(x, نقاب) (((x) + (نقاب)) & ~(نقاب))
#وصف_ڪريو KVX_ABI_ڍير_ALIGNMENT (32)
#وصف_ڪريو KVX_ABI_ڍير_ARG_ALIGNMENT (8)
#وصف_ڪريو max(a,b) ((a) > (b) ? (a) : (b))

#جيڪڏهن_وصف_ٿيل FFI_DEBUG
#وصف_ڪريو DEBUG_PRINT(...) افعل{ fprintf( stderr, __VA_ARGS__ ); } طالما(0)
#وإلا
#وصف_ڪريو DEBUG_PRINT(...)
#جيڪڏهن_پورو

بنية ret_قدر {
	بنانشان ڊگهو صحيح r0;
	بنانشان ڊگهو صحيح r1;
	بنانشان ڊگهو صحيح r2;
	بنانشان ڊگهو صحيح r3;
};

خارجي بنية ret_قدر ffi_call_SYSV(بنانشان ڪل_سائيز,
                                      بنانشان سائيز,
                                      extended_cif *ecif,
                                      بنانشان *rvalue_addr,
                                      فراغ *fn,
                                      بنانشان int_ext_طريقو);

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->سائيز;
  أرجع FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

فراغ *ffi_prep_دليلون(حرف *ڍير, بنانشان صحيح arg_slots_سائيز, extended_cif *ecif)
{
  حرف *stacktemp = ڍير;
  حرف *current_arg_passed_by_قدر = ڍير + arg_slots_سائيز;
  صحيح i, s;
  ffi_type **arg;
  صحيح ڳڻپ = 0;
  ffi_cif *cif = ecif->cif;
  فراغ **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", ڍير);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_سائيز);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_قدر);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  لكل (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_جڳهه_سائيز;
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
      حالة FFI_TYPE_اشارو:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", ڍير);
        *(uint64_t *) ڍير = *(uint64_t *)(* argv);
        اكسر;

      حالة FFI_TYPE_COMPLEX:
        إذا ((*arg)->سائيز == 8)
          *(_Complex عائم *) ڍير = *(_Complex عائم *)(* argv);
        وإلا إذا ((*arg)->سائيز == 16) {
          *(_Complex مزدوج *) ڍير = *(_Complex مزدوج *)(* argv);
          s = 16;
        } وإلا
          abort();
        اكسر;
      حالة FFI_TYPE_STRUCT: {
        حرف *قدر;
        بنانشان صحيح written_سائيز = 0;
        DEBUG_PRINT("struct by value @%p\n", ڍير);
        إذا ((*arg)->سائيز > KVX_ABI_MAX_AGGREGATE_اندريون_REG_سائيز) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) ڍير = (uintptr_t)current_arg_passed_by_قدر;
          قدر = current_arg_passed_by_قدر;
          current_arg_passed_by_قدر += (*arg)->سائيز;
          written_سائيز = KVX_ABI_جڳهه_سائيز;
        } وإلا {
          قدر = ڍير;
          written_سائيز = (*arg)->سائيز;
        }
        يادگيري_نقل(قدر, *argv, (*arg)->سائيز);
        s = ALIGN(written_سائيز, KVX_ABI_ڍير_ARG_ALIGNMENT);
        اكسر;
      }
      افتراضي:
        لکو("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        اكسر;

    }
    ڍير += s;
    ڳڻپ += s;
    argv++;
    arg++;
  }
#جيڪڏهن_وصف_ٿيل FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_دليلون_سائيز) & (KVX_ABI_ڍير_ALIGNMENT-1)) == 0);
#جيڪڏهن_پورو
  أرجع stacktemp + REG_دليلون_سائيز;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif, بنانشان صحيح nfixedargs,
                                    بنانشان صحيح ntotalargs)
{
  cif->flags = cif->rtype->سائيز;
  أرجع FFI_OK;
}

ساكن بنانشان ڊگهو handle_small_int_ext(kvx_intext_طريقو *int_ext_طريقو,
                                          ثابت ffi_type *rtype)
{
  بدل (rtype->type) {
    حالة FFI_TYPE_SINT8:
      *int_ext_طريقو = KVX_RET_SXBD;
      أرجع KVX_رجسٽر_سائيز;

    حالة FFI_TYPE_SINT16:
      *int_ext_طريقو = KVX_RET_SXHD;
      أرجع KVX_رجسٽر_سائيز;

    حالة FFI_TYPE_SINT32:
      *int_ext_طريقو = KVX_RET_SXWD;
      أرجع KVX_رجسٽر_سائيز;

    حالة FFI_TYPE_UINT8:
      *int_ext_طريقو = KVX_RET_ZXBD;
      أرجع KVX_رجسٽر_سائيز;

    حالة FFI_TYPE_UINT16:
      *int_ext_طريقو = KVX_RET_ZXHD;
      أرجع KVX_رجسٽر_سائيز;

    حالة FFI_TYPE_UINT32:
      *int_ext_طريقو = KVX_RET_ZXWD;
      أرجع KVX_رجسٽر_سائيز;

    افتراضي:
      *int_ext_طريقو = KVX_RET_NONE;
      أرجع rtype->سائيز;
  }
}

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  صحيح i;
  بنانشان ڊگهو صحيح جڳهه_fitting_دليلون_سائيز = 0;
  بنانشان ڊگهو صحيح ڪل_سائيز = 0;
  بنانشان ڊگهو صحيح big_struct_سائيز = 0;
  kvx_intext_طريقو int_extension_طريقو;
  ffi_type **arg;
  بنية ret_قدر local_rvalue = {0};
  size_t wb_سائيز;


  /* Calculate size to allocate on stack */
  لكل (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->سائيز);
    إذا (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      إذا ((*arg)->سائيز <= KVX_ABI_MAX_AGGREGATE_اندريون_REG_سائيز) {
        جڳهه_fitting_دليلون_سائيز += ALIGN((*arg)->سائيز, KVX_ABI_جڳهه_سائيز);
      } وإلا {
        جڳهه_fitting_دليلون_سائيز += KVX_ABI_جڳهه_سائيز; /* aggregate passed by reference */
        big_struct_سائيز += ALIGN((*arg)->سائيز, KVX_ABI_جڳهه_سائيز);
      }
    } وإلا إذا ((*arg)->سائيز <= KVX_ABI_جڳهه_سائيز) {
      جڳهه_fitting_دليلون_سائيز += KVX_ABI_جڳهه_سائيز;
    } وإلا {
      لکو("Error: unsupported arg size %ld arg type %d\n", (*arg)->سائيز, (*arg)->type);
      abort(); /* should never happen? */
    }
  }

  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  /* This implementation allocates anyway for all register based args */
  جڳهه_fitting_دليلون_سائيز = max(جڳهه_fitting_دليلون_سائيز, REG_دليلون_سائيز);
  ڪل_سائيز = جڳهه_fitting_دليلون_سائيز + big_struct_سائيز;
  ڪل_سائيز = ALIGN(ڪل_سائيز, KVX_ABI_ڍير_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_سائيز = handle_small_int_ext(&int_extension_طريقو, cif->rtype);

  بدل (cif->abi) {
    حالة FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", ڪل_سائيز);
      DEBUG_PRINT("slot fitting args size: %lu\n", جڳهه_fitting_دليلون_سائيز);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_سائيز);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_طريقو);
      local_rvalue = ffi_call_SYSV(ڪل_سائيز, جڳهه_fitting_دليلون_سائيز,
                                   &ecif, rvalue, fn, int_extension_طريقو);
      إذا ((cif->flags <= KVX_ABI_MAX_AGGREGATE_اندريون_REG_سائيز)
          && (cif->rtype->type != FFI_TYPE_VOID))
        يادگيري_نقل(rvalue, &local_rvalue, wb_سائيز);
      اكسر;
    افتراضي:
      abort();
      اكسر;
  }
}

/* Closures not supported yet */
ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو* گهيرو,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_معلومات,
                      فراغ *codeloc)
{
  أرجع FFI_BAD_ABI;
}

#جيڪڏهن_پورو /* (__kvx__) */
