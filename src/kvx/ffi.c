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

#যদি defined(__kvx__)
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <fficonfig.h>
#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ "ffi_common.h"
#অন্তর্ভাৱ "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_স্তূপ_ALIGNMENT (32)
#define KVX_ABI_স্তূপ_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_PRINT(...) কৰা{ fprintf( stderr, __VA_ARGS__ ); } যেতিয়ালৈ(0)
#অন্যথা
#define DEBUG_PRINT(...)
#endif

সংরচনা ret_মান {
	unsigned long পূর্ণাঙ্ক r0;
	unsigned long পূর্ণাঙ্ক r1;
	unsigned long পূর্ণাঙ্ক r2;
	unsigned long পূর্ণাঙ্ক r3;
};

বাহ্য সংরচনা ret_মান ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      শূন্য *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  ঘূৰাই FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

শূন্য *ffi_prep_args(ৱর্ণ *স্তূপ, unsigned পূর্ণাঙ্ক arg_slots_size, extended_cif *ecif)
{
  ৱর্ণ *stacktemp = স্তূপ;
  ৱর্ণ *current_arg_passed_by_মান = স্তূপ + arg_slots_size;
  পূর্ণাঙ্ক i, s;
  ffi_type **arg;
  পূর্ণাঙ্ক গুনতি = 0;
  ffi_cif *cif = ecif->cif;
  শূন্য **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", স্তূপ);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_মান);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  বাবে (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
    ৱিকল্প((*arg)->type) {
      পক্ষ FFI_TYPE_SINT8:
      পক্ষ FFI_TYPE_UINT8:
      পক্ষ FFI_TYPE_SINT16:
      পক্ষ FFI_TYPE_UINT16:
      পক্ষ FFI_TYPE_SINT32:
      পক্ষ FFI_TYPE_UINT32:
      পক্ষ FFI_TYPE_FLOAT:
      পক্ষ FFI_TYPE_DOUBLE:
      পক্ষ FFI_TYPE_UINT64:
      পক্ষ FFI_TYPE_SINT64:
      পক্ষ FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", স্তূপ);
        *(uint64_t *) স্তূপ = *(uint64_t *)(* argv);
        ভঙ্গ;

      পক্ষ FFI_TYPE_COMPLEX:
        যদি ((*arg)->size == 8)
          *(_Complex ভিন্ন *) স্তূপ = *(_Complex ভিন্ন *)(* argv);
        অন্যথা যদি ((*arg)->size == 16) {
          *(_Complex দ্ৱিগুণ *) স্তূপ = *(_Complex দ্ৱিগুণ *)(* argv);
          s = 16;
        } অন্যথা
          abort();
        ভঙ্গ;
      পক্ষ FFI_TYPE_STRUCT: {
        ৱর্ণ *মান;
        unsigned পূর্ণাঙ্ক written_size = 0;
        DEBUG_PRINT("struct by value @%p\n", স্তূপ);
        যদি ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) স্তূপ = (uintptr_t)current_arg_passed_by_মান;
          মান = current_arg_passed_by_মান;
          current_arg_passed_by_মান += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } অন্যথা {
          মান = স্তূপ;
          written_size = (*arg)->size;
        }
        memcpy(মান, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_স্তূপ_ARG_ALIGNMENT);
        ভঙ্গ;
      }
      স্ৱভাৱ:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        ভঙ্গ;

    }
    স্তূপ += s;
    গুনতি += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_SIZE) & (KVX_ABI_স্তূপ_ALIGNMENT-1)) == 0);
#endif
  ঘূৰাই stacktemp + REG_ARGS_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned পূর্ণাঙ্ক nfixedargs,
                                    unsigned পূর্ণাঙ্ক ntotalargs)
{
  cif->flags = cif->rtype->size;
  ঘূৰাই FFI_OK;
}

স্থির unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          ধ্রুৱ ffi_type *rtype)
{
  ৱিকল্প (rtype->type) {
    পক্ষ FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      ঘূৰাই KVX_REGISTER_SIZE;

    পক্ষ FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      ঘূৰাই KVX_REGISTER_SIZE;

    পক্ষ FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      ঘূৰাই KVX_REGISTER_SIZE;

    পক্ষ FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      ঘূৰাই KVX_REGISTER_SIZE;

    পক্ষ FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      ঘূৰাই KVX_REGISTER_SIZE;

    পক্ষ FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      ঘূৰাই KVX_REGISTER_SIZE;

    স্ৱভাৱ:
      *int_ext_method = KVX_RET_NONE;
      ঘূৰাই rtype->size;
  }
}

শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  পূর্ণাঙ্ক i;
  unsigned long পূর্ণাঙ্ক slot_fitting_args_size = 0;
  unsigned long পূর্ণাঙ্ক total_size = 0;
  unsigned long পূর্ণাঙ্ক big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  সংরচনা ret_মান local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  বাবে (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    যদি (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      যদি ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_args_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } অন্যথা {
        slot_fitting_args_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } অন্যথা যদি ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_args_size += KVX_ABI_SLOT_SIZE;
    } অন্যথা {
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
  total_size = ALIGN(total_size, KVX_ABI_স্তূপ_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_size = handle_small_int_ext(&int_extension_method, cif->rtype);

  ৱিকল্প (cif->abi) {
    পক্ষ FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", total_size);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_size);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_size);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_args_size,
                                   &ecif, rvalue, fn, int_extension_method);
      যদি ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      ভঙ্গ;
    স্ৱভাৱ:
      abort();
      ভঙ্গ;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
                      শূন্য *user_data,
                      শূন্য *codeloc)
{
  ঘূৰাই FFI_BAD_ABI;
}

#endif /* (__kvx__) */
