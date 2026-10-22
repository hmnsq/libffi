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

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_مكدس_ALIGNMENT (32)
#define KVX_ABI_مكدس_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_اطبع(...) افعل{ fprintf( stderr, __VA_ARGS__ ); } طالما(0)
#وإلا
#define DEBUG_اطبع(...)
#endif

بنية ret_قيمة {
	unsigned long صحيح r0;
	unsigned long صحيح r1;
	unsigned long صحيح r2;
	unsigned long صحيح r3;
};

خارجي بنية ret_قيمة ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      فراغ *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  أرجع FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

فراغ *ffi_prep_وسائط(حرف *مكدس, unsigned صحيح arg_slots_size, extended_cif *ecif)
{
  حرف *stacktemp = مكدس;
  حرف *current_arg_passed_by_قيمة = مكدس + arg_slots_size;
  صحيح i, s;
  ffi_type **arg;
  صحيح عدد = 0;
  ffi_cif *cif = ecif->cif;
  فراغ **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_اطبع("stack: %p\n", مكدس);
  DEBUG_اطبع("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_اطبع("current_arg_passed_by_value: %p\n", current_arg_passed_by_قيمة);
  DEBUG_اطبع("ecif: %p\n", ecif);
  DEBUG_اطبع("ecif->avalue: %p\n", ecif->avalue);

  لكل (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
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
      حالة FFI_TYPE_POINTER:
        DEBUG_اطبع("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", مكدس);
        *(uint64_t *) مكدس = *(uint64_t *)(* argv);
        اكسر;

      حالة FFI_TYPE_COMPLEX:
        إذا ((*arg)->size == 8)
          *(_Complex عائم *) مكدس = *(_Complex عائم *)(* argv);
        وإلا إذا ((*arg)->size == 16) {
          *(_Complex مزدوج *) مكدس = *(_Complex مزدوج *)(* argv);
          s = 16;
        } وإلا
          abort();
        اكسر;
      حالة FFI_TYPE_STRUCT: {
        حرف *قيمة;
        unsigned صحيح written_size = 0;
        DEBUG_اطبع("struct by value @%p\n", مكدس);
        إذا ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_اطبع("big struct\n");
          *(uint64_t *) مكدس = (uintptr_t)current_arg_passed_by_قيمة;
          قيمة = current_arg_passed_by_قيمة;
          current_arg_passed_by_قيمة += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } وإلا {
          قيمة = مكدس;
          written_size = (*arg)->size;
        }
        memcpy(قيمة, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_مكدس_ARG_ALIGNMENT);
        اكسر;
      }
      افتراضي:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        اكسر;

    }
    مكدس += s;
    عدد += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_وسائط_SIZE) & (KVX_ABI_مكدس_ALIGNMENT-1)) == 0);
#endif
  أرجع stacktemp + REG_وسائط_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned صحيح nfixedargs,
                                    unsigned صحيح ntotalargs)
{
  cif->flags = cif->rtype->size;
  أرجع FFI_OK;
}

ساكن unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          ثابت ffi_type *rtype)
{
  بدل (rtype->type) {
    حالة FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      أرجع KVX_REGISTER_SIZE;

    حالة FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      أرجع KVX_REGISTER_SIZE;

    حالة FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      أرجع KVX_REGISTER_SIZE;

    حالة FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      أرجع KVX_REGISTER_SIZE;

    حالة FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      أرجع KVX_REGISTER_SIZE;

    حالة FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      أرجع KVX_REGISTER_SIZE;

    افتراضي:
      *int_ext_method = KVX_RET_NONE;
      أرجع rtype->size;
  }
}

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  صحيح i;
  unsigned long صحيح slot_fitting_وسائط_size = 0;
  unsigned long صحيح total_size = 0;
  unsigned long صحيح big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  بنية ret_قيمة local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  لكل (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_اطبع("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    إذا (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      إذا ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_وسائط_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } وإلا {
        slot_fitting_وسائط_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } وإلا إذا ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_وسائط_size += KVX_ABI_SLOT_SIZE;
    } وإلا {
      printf("Error: unsupported arg size %ld arg type %d\n", (*arg)->size, (*arg)->type);
      abort(); /* should never happen? */
    }
  }

  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  /* This implementation allocates anyway for all register based args */
  slot_fitting_وسائط_size = max(slot_fitting_وسائط_size, REG_وسائط_SIZE);
  total_size = slot_fitting_وسائط_size + big_struct_size;
  total_size = ALIGN(total_size, KVX_ABI_مكدس_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_size = handle_small_int_ext(&int_extension_method, cif->rtype);

  بدل (cif->abi) {
    حالة FFI_SYSV:
      DEBUG_اطبع("total_size: %lu\n", total_size);
      DEBUG_اطبع("slot fitting args size: %lu\n", slot_fitting_وسائط_size);
      DEBUG_اطبع("rvalue: %p\n", rvalue);
      DEBUG_اطبع("fn: %p\n", fn);
      DEBUG_اطبع("rsize: %u\n", cif->flags);
      DEBUG_اطبع("wb_size: %u\n", wb_size);
      DEBUG_اطبع("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_وسائط_size,
                                   &ecif, rvalue, fn, int_extension_method);
      إذا ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      اكسر;
    افتراضي:
      abort();
      اكسر;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_data,
                      فراغ *codeloc)
{
  أرجع FFI_BAD_ABI;
}

#endif /* (__kvx__) */
