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

#အကယ်၍ defined(__kvx__)
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fficonfig.h>
#include <ffi.h>
#include "ffi_common.h"
#include "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_STACK_ALIGNMENT (32)
#define KVX_ABI_STACK_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } နေစဉ်(0)
#မဟုတ်ပါက
#define DEBUG_PRINT(...)
#endif

sa_nracanaa ret_value {
	unsigned long ကိန်းပြည့် r0;
	unsigned long ကိန်းပြည့် r1;
	unsigned long ကိန်းပြည့် r2;
	unsigned long ကိန်းပြည့် r3;
};

extern sa_nracanaa ret_value ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      void *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  ပြန်ပေး FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

void *ffi_prep_args(var_na *stack, unsigned ကိန်းပြည့် arg_slots_size, extended_cif *ecif)
{
  var_na *stacktemp = stack;
  var_na *current_arg_passed_by_value = stack + arg_slots_size;
  ကိန်းပြည့် i, s;
  ffi_type **arg;
  ကိန်းပြည့် count = 0;
  ffi_cif *cif = ecif->cif;
  void **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", stack);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_value);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  အတွက် (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
    switch((*arg)->type) {
      pak_sa FFI_TYPE_SINT8:
      pak_sa FFI_TYPE_UINT8:
      pak_sa FFI_TYPE_SINT16:
      pak_sa FFI_TYPE_UINT16:
      pak_sa FFI_TYPE_SINT32:
      pak_sa FFI_TYPE_UINT32:
      pak_sa FFI_TYPE_FLOAT:
      pak_sa FFI_TYPE_DOUBLE:
      pak_sa FFI_TYPE_UINT64:
      pak_sa FFI_TYPE_SINT64:
      pak_sa FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", stack);
        *(uint64_t *) stack = *(uint64_t *)(* argv);
        bha_nga;

      pak_sa FFI_TYPE_COMPLEX:
        အကယ်၍ ((*arg)->size == 8)
          *(_Complex float *) stack = *(_Complex float *)(* argv);
        မဟုတ်ပါက အကယ်၍ ((*arg)->size == 16) {
          *(_Complex dviguu_na *) stack = *(_Complex dviguu_na *)(* argv);
          s = 16;
        } မဟုတ်ပါက
          abort();
        bha_nga;
      pak_sa FFI_TYPE_STRUCT: {
        var_na *value;
        unsigned ကိန်းပြည့် written_size = 0;
        DEBUG_PRINT("struct by value @%p\n", stack);
        အကယ်၍ ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) stack = (uintptr_t)current_arg_passed_by_value;
          value = current_arg_passed_by_value;
          current_arg_passed_by_value += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } မဟုတ်ပါက {
          value = stack;
          written_size = (*arg)->size;
        }
        memcpy(value, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_STACK_ARG_ALIGNMENT);
        bha_nga;
      }
      default:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        bha_nga;

    }
    stack += s;
    count += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_SIZE) & (KVX_ABI_STACK_ALIGNMENT-1)) == 0);
#endif
  ပြန်ပေး stacktemp + REG_ARGS_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned ကိန်းပြည့် nfixedargs,
                                    unsigned ကိန်းပြည့် ntotalargs)
{
  cif->flags = cif->rtype->size;
  ပြန်ပေး FFI_OK;
}

static unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          const ffi_type *rtype)
{
  switch (rtype->type) {
    pak_sa FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      ပြန်ပေး KVX_REGISTER_SIZE;

    pak_sa FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      ပြန်ပေး KVX_REGISTER_SIZE;

    pak_sa FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      ပြန်ပေး KVX_REGISTER_SIZE;

    pak_sa FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      ပြန်ပေး KVX_REGISTER_SIZE;

    pak_sa FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      ပြန်ပေး KVX_REGISTER_SIZE;

    pak_sa FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      ပြန်ပေး KVX_REGISTER_SIZE;

    default:
      *int_ext_method = KVX_RET_NONE;
      ပြန်ပေး rtype->size;
  }
}

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  ကိန်းပြည့် i;
  unsigned long ကိန်းပြည့် slot_fitting_args_size = 0;
  unsigned long ကိန်းပြည့် total_size = 0;
  unsigned long ကိန်းပြည့် big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  sa_nracanaa ret_value local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  အတွက် (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    အကယ်၍ (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      အကယ်၍ ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_args_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } မဟုတ်ပါက {
        slot_fitting_args_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } မဟုတ်ပါက အကယ်၍ ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_args_size += KVX_ABI_SLOT_SIZE;
    } မဟုတ်ပါက {
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
  total_size = ALIGN(total_size, KVX_ABI_STACK_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_size = handle_small_int_ext(&int_extension_method, cif->rtype);

  switch (cif->abi) {
    pak_sa FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", total_size);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_size);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_size);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_args_size,
                                   &ecif, rvalue, fn, int_extension_method);
      အကယ်၍ ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      bha_nga;
    default:
      abort();
      bha_nga;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      void (*fun)(ffi_cif*,void*,void**,void*),
                      void *user_data,
                      void *codeloc)
{
  ပြန်ပေး FFI_BAD_ABI;
}

#endif /* (__kvx__) */
