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

#تعریف_کرو ALIGN(x, a) ALIGN_نقاب(x, (typeof(x))(a) - 1)
#تعریف_کرو ALIGN_نقاب(x, نقاب) (((x) + (نقاب)) & ~(نقاب))
#تعریف_کرو KVX_ABI_ڈھیر_ALIGNMENT (32)
#تعریف_کرو KVX_ABI_ڈھیر_ARG_ALIGNMENT (8)
#تعریف_کرو max(a,b) ((a) > (b) ? (a) : (b))

#جے_تعریف_شدہ FFI_DEBUG
#تعریف_کرو DEBUG_PRINT(...) افعل{ fprintf( stderr, __VA_ARGS__ ); } طالما(0)
#وإلا
#تعریف_کرو DEBUG_PRINT(...)
#جے_انت

بنية ret_قدر {
	بنانشانی لما صحيح r0;
	بنانشانی لما صحيح r1;
	بنانشانی لما صحيح r2;
	بنانشانی لما صحيح r3;
};

خارجي بنية ret_قدر ffi_call_SYSV(بنانشانی کل_ناپ,
                                      بنانشانی ناپ,
                                      extended_cif *ecif,
                                      بنانشانی *rvalue_addr,
                                      فراغ *fn,
                                      بنانشانی int_ext_طریقہ);

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->ناپ;
  أرجع FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

فراغ *ffi_prep_دلیلاں(حرف *ڈھیر, بنانشانی صحيح arg_slots_ناپ, extended_cif *ecif)
{
  حرف *stacktemp = ڈھیر;
  حرف *current_arg_passed_by_قدر = ڈھیر + arg_slots_ناپ;
  صحيح i, s;
  ffi_type **arg;
  صحيح گنتی = 0;
  ffi_cif *cif = ecif->cif;
  فراغ **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", ڈھیر);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_ناپ);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_قدر);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  لكل (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_تھاں_ناپ;
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
      حالة FFI_TYPE_اشارہ_گر:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", ڈھیر);
        *(uint64_t *) ڈھیر = *(uint64_t *)(* argv);
        اكسر;

      حالة FFI_TYPE_COMPLEX:
        إذا ((*arg)->ناپ == 8)
          *(_Complex عائم *) ڈھیر = *(_Complex عائم *)(* argv);
        وإلا إذا ((*arg)->ناپ == 16) {
          *(_Complex مزدوج *) ڈھیر = *(_Complex مزدوج *)(* argv);
          s = 16;
        } وإلا
          abort();
        اكسر;
      حالة FFI_TYPE_STRUCT: {
        حرف *قدر;
        بنانشانی صحيح written_ناپ = 0;
        DEBUG_PRINT("struct by value @%p\n", ڈھیر);
        إذا ((*arg)->ناپ > KVX_ABI_MAX_AGGREGATE_وچ_REG_ناپ) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) ڈھیر = (uintptr_t)current_arg_passed_by_قدر;
          قدر = current_arg_passed_by_قدر;
          current_arg_passed_by_قدر += (*arg)->ناپ;
          written_ناپ = KVX_ABI_تھاں_ناپ;
        } وإلا {
          قدر = ڈھیر;
          written_ناپ = (*arg)->ناپ;
        }
        حافظہ_نقل(قدر, *argv, (*arg)->ناپ);
        s = ALIGN(written_ناپ, KVX_ABI_ڈھیر_ARG_ALIGNMENT);
        اكسر;
      }
      افتراضي:
        لکھو("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        اكسر;

    }
    ڈھیر += s;
    گنتی += s;
    argv++;
    arg++;
  }
#جے_تعریف_شدہ FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_دلیلاں_ناپ) & (KVX_ABI_ڈھیر_ALIGNMENT-1)) == 0);
#جے_انت
  أرجع stacktemp + REG_دلیلاں_ناپ;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif, بنانشانی صحيح nfixedargs,
                                    بنانشانی صحيح ntotalargs)
{
  cif->flags = cif->rtype->ناپ;
  أرجع FFI_OK;
}

ساكن بنانشانی لما handle_small_int_ext(kvx_intext_طریقہ *int_ext_طریقہ,
                                          ثابت ffi_type *rtype)
{
  بدل (rtype->type) {
    حالة FFI_TYPE_SINT8:
      *int_ext_طریقہ = KVX_RET_SXBD;
      أرجع KVX_رجسٹر_ناپ;

    حالة FFI_TYPE_SINT16:
      *int_ext_طریقہ = KVX_RET_SXHD;
      أرجع KVX_رجسٹر_ناپ;

    حالة FFI_TYPE_SINT32:
      *int_ext_طریقہ = KVX_RET_SXWD;
      أرجع KVX_رجسٹر_ناپ;

    حالة FFI_TYPE_UINT8:
      *int_ext_طریقہ = KVX_RET_ZXBD;
      أرجع KVX_رجسٹر_ناپ;

    حالة FFI_TYPE_UINT16:
      *int_ext_طریقہ = KVX_RET_ZXHD;
      أرجع KVX_رجسٹر_ناپ;

    حالة FFI_TYPE_UINT32:
      *int_ext_طریقہ = KVX_RET_ZXWD;
      أرجع KVX_رجسٹر_ناپ;

    افتراضي:
      *int_ext_طریقہ = KVX_RET_NONE;
      أرجع rtype->ناپ;
  }
}

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  صحيح i;
  بنانشانی لما صحيح تھاں_fitting_دلیلاں_ناپ = 0;
  بنانشانی لما صحيح کل_ناپ = 0;
  بنانشانی لما صحيح big_struct_ناپ = 0;
  kvx_intext_طریقہ int_extension_طریقہ;
  ffi_type **arg;
  بنية ret_قدر local_rvalue = {0};
  size_t wb_ناپ;


  /* Calculate size to allocate on stack */
  لكل (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->ناپ);
    إذا (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      إذا ((*arg)->ناپ <= KVX_ABI_MAX_AGGREGATE_وچ_REG_ناپ) {
        تھاں_fitting_دلیلاں_ناپ += ALIGN((*arg)->ناپ, KVX_ABI_تھاں_ناپ);
      } وإلا {
        تھاں_fitting_دلیلاں_ناپ += KVX_ABI_تھاں_ناپ; /* aggregate passed by reference */
        big_struct_ناپ += ALIGN((*arg)->ناپ, KVX_ABI_تھاں_ناپ);
      }
    } وإلا إذا ((*arg)->ناپ <= KVX_ABI_تھاں_ناپ) {
      تھاں_fitting_دلیلاں_ناپ += KVX_ABI_تھاں_ناپ;
    } وإلا {
      لکھو("Error: unsupported arg size %ld arg type %d\n", (*arg)->ناپ, (*arg)->type);
      abort(); /* should never happen? */
    }
  }

  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  /* This implementation allocates anyway for all register based args */
  تھاں_fitting_دلیلاں_ناپ = max(تھاں_fitting_دلیلاں_ناپ, REG_دلیلاں_ناپ);
  کل_ناپ = تھاں_fitting_دلیلاں_ناپ + big_struct_ناپ;
  کل_ناپ = ALIGN(کل_ناپ, KVX_ABI_ڈھیر_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_ناپ = handle_small_int_ext(&int_extension_طریقہ, cif->rtype);

  بدل (cif->abi) {
    حالة FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", کل_ناپ);
      DEBUG_PRINT("slot fitting args size: %lu\n", تھاں_fitting_دلیلاں_ناپ);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_ناپ);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_طریقہ);
      local_rvalue = ffi_call_SYSV(کل_ناپ, تھاں_fitting_دلیلاں_ناپ,
                                   &ecif, rvalue, fn, int_extension_طریقہ);
      إذا ((cif->flags <= KVX_ABI_MAX_AGGREGATE_وچ_REG_ناپ)
          && (cif->rtype->type != FFI_TYPE_VOID))
        حافظہ_نقل(rvalue, &local_rvalue, wb_ناپ);
      اكسر;
    افتراضي:
      abort();
      اكسر;
  }
}

/* Closures not supported yet */
ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا* گھیرا,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_انکڑے,
                      فراغ *codeloc)
{
  أرجع FFI_BAD_ABI;
}

#جے_انت /* (__kvx__) */
