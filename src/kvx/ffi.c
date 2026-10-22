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

#اگر defined(__kvx__)
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <stdint.h>
#شامل <fficonfig.h>
#شامل <ffi.h>
#شامل "ffi_common.h"
#شامل "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_پشته_ALIGNMENT (32)
#define KVX_ABI_پشته_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_PRINT(...) انجامبده{ fprintf( stderr, __VA_ARGS__ ); } وقتی(0)
#وگرنه
#define DEBUG_PRINT(...)
#endif

ساختار ret_مقدار {
	unsigned long صحیح r0;
	unsigned long صحیح r1;
	unsigned long صحیح r2;
	unsigned long صحیح r3;
};

بیرونی ساختار ret_مقدار ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      تهی *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  بازگشت FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

تهی *ffi_prep_args(نویسه *پشته, unsigned صحیح arg_slots_size, extended_cif *ecif)
{
  نویسه *stacktemp = پشته;
  نویسه *current_arg_passed_by_مقدار = پشته + arg_slots_size;
  صحیح i, s;
  ffi_type **arg;
  صحیح شمار = 0;
  ffi_cif *cif = ecif->cif;
  تهی **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", پشته);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_مقدار);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  برای (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
    انتخاب((*arg)->type) {
      حالت FFI_TYPE_SINT8:
      حالت FFI_TYPE_UINT8:
      حالت FFI_TYPE_SINT16:
      حالت FFI_TYPE_UINT16:
      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_FLOAT:
      حالت FFI_TYPE_DOUBLE:
      حالت FFI_TYPE_UINT64:
      حالت FFI_TYPE_SINT64:
      حالت FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", پشته);
        *(uint64_t *) پشته = *(uint64_t *)(* argv);
        بشکن;

      حالت FFI_TYPE_COMPLEX:
        اگر ((*arg)->size == 8)
          *(_Complex اعشاری *) پشته = *(_Complex اعشاری *)(* argv);
        وگرنه اگر ((*arg)->size == 16) {
          *(_Complex دوگانه *) پشته = *(_Complex دوگانه *)(* argv);
          s = 16;
        } وگرنه
          abort();
        بشکن;
      حالت FFI_TYPE_STRUCT: {
        نویسه *مقدار;
        unsigned صحیح written_size = 0;
        DEBUG_PRINT("struct by value @%p\n", پشته);
        اگر ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) پشته = (uintptr_t)current_arg_passed_by_مقدار;
          مقدار = current_arg_passed_by_مقدار;
          current_arg_passed_by_مقدار += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } وگرنه {
          مقدار = پشته;
          written_size = (*arg)->size;
        }
        memcpy(مقدار, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_پشته_ARG_ALIGNMENT);
        بشکن;
      }
      پیشفرض:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        بشکن;

    }
    پشته += s;
    شمار += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_SIZE) & (KVX_ABI_پشته_ALIGNMENT-1)) == 0);
#endif
  بازگشت stacktemp + REG_ARGS_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned صحیح nfixedargs,
                                    unsigned صحیح ntotalargs)
{
  cif->flags = cif->rtype->size;
  بازگشت FFI_OK;
}

ایستا unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          ثابت ffi_type *rtype)
{
  انتخاب (rtype->type) {
    حالت FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      بازگشت KVX_REGISTER_SIZE;

    حالت FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      بازگشت KVX_REGISTER_SIZE;

    حالت FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      بازگشت KVX_REGISTER_SIZE;

    حالت FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      بازگشت KVX_REGISTER_SIZE;

    حالت FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      بازگشت KVX_REGISTER_SIZE;

    حالت FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      بازگشت KVX_REGISTER_SIZE;

    پیشفرض:
      *int_ext_method = KVX_RET_NONE;
      بازگشت rtype->size;
  }
}

تهی ffi_call(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  صحیح i;
  unsigned long صحیح slot_fitting_args_size = 0;
  unsigned long صحیح total_size = 0;
  unsigned long صحیح big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  ساختار ret_مقدار local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  برای (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    اگر (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      اگر ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_args_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } وگرنه {
        slot_fitting_args_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } وگرنه اگر ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_args_size += KVX_ABI_SLOT_SIZE;
    } وگرنه {
      printf("Error: unsupported arg size %ld arg type %d\n", (*arg)->size, (*arg)->type);
      abort(); /* should never happen? */
    }
  }

  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  /* This implementation allocates anyway for all register based args */
  slot_fitting_args_size = max(slot_fitting_args_size, REG_ARGS_SIZE);
  total_size = slot_fitting_args_size + big_struct_size;
  total_size = ALIGN(total_size, KVX_ABI_پشته_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_size = handle_small_int_ext(&int_extension_method, cif->rtype);

  انتخاب (cif->abi) {
    حالت FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", total_size);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_size);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_size);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_args_size,
                                   &ecif, rvalue, fn, int_extension_method);
      اگر ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      بشکن;
    پیشفرض:
      abort();
      بشکن;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*),
                      تهی *user_data,
                      تهی *codeloc)
{
  بازگشت FFI_BAD_ABI;
}

#endif /* (__kvx__) */
