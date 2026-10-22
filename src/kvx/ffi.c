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

#எனில் defined(__kvx__)
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <fficonfig.h>
#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ "ffi_common.h"
#அந்தர்பாவ "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_அடுக்கு_ALIGNMENT (32)
#define KVX_ABI_அடுக்கு_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_PRINT(...) செய்{ fprintf( stderr, __VA_ARGS__ ); } போது(0)
#இல்லையெனில்
#define DEBUG_PRINT(...)
#endif

கட்டமைப்பு ret_மதிப்பு {
	unsigned long முழு r0;
	unsigned long முழு r1;
	unsigned long முழு r2;
	unsigned long முழு r3;
};

வெளி கட்டமைப்பு ret_மதிப்பு ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      வெறுமை *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  திரும்பு FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

வெறுமை *ffi_prep_args(எழுத்து *அடுக்கு, unsigned முழு arg_slots_size, extended_cif *ecif)
{
  எழுத்து *stacktemp = அடுக்கு;
  எழுத்து *current_arg_passed_by_மதிப்பு = அடுக்கு + arg_slots_size;
  முழு i, s;
  ffi_type **arg;
  முழு மொத்தம் = 0;
  ffi_cif *cif = ecif->cif;
  வெறுமை **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", அடுக்கு);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_மதிப்பு);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  க்கு (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
    தேர்வு((*arg)->type) {
      நிலை FFI_TYPE_SINT8:
      நிலை FFI_TYPE_UINT8:
      நிலை FFI_TYPE_SINT16:
      நிலை FFI_TYPE_UINT16:
      நிலை FFI_TYPE_SINT32:
      நிலை FFI_TYPE_UINT32:
      நிலை FFI_TYPE_FLOAT:
      நிலை FFI_TYPE_DOUBLE:
      நிலை FFI_TYPE_UINT64:
      நிலை FFI_TYPE_SINT64:
      நிலை FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", அடுக்கு);
        *(uint64_t *) அடுக்கு = *(uint64_t *)(* argv);
        நிறுத்து;

      நிலை FFI_TYPE_COMPLEX:
        எனில் ((*arg)->size == 8)
          *(_Complex தசம *) அடுக்கு = *(_Complex தசம *)(* argv);
        இல்லையெனில் எனில் ((*arg)->size == 16) {
          *(_Complex இரட்டை *) அடுக்கு = *(_Complex இரட்டை *)(* argv);
          s = 16;
        } இல்லையெனில்
          abort();
        நிறுத்து;
      நிலை FFI_TYPE_STRUCT: {
        எழுத்து *மதிப்பு;
        unsigned முழு written_size = 0;
        DEBUG_PRINT("struct by value @%p\n", அடுக்கு);
        எனில் ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) அடுக்கு = (uintptr_t)current_arg_passed_by_மதிப்பு;
          மதிப்பு = current_arg_passed_by_மதிப்பு;
          current_arg_passed_by_மதிப்பு += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } இல்லையெனில் {
          மதிப்பு = அடுக்கு;
          written_size = (*arg)->size;
        }
        memcpy(மதிப்பு, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_அடுக்கு_ARG_ALIGNMENT);
        நிறுத்து;
      }
      இயல்பு:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        நிறுத்து;

    }
    அடுக்கு += s;
    மொத்தம் += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_SIZE) & (KVX_ABI_அடுக்கு_ALIGNMENT-1)) == 0);
#endif
  திரும்பு stacktemp + REG_ARGS_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned முழு nfixedargs,
                                    unsigned முழு ntotalargs)
{
  cif->flags = cif->rtype->size;
  திரும்பு FFI_OK;
}

நிலையான unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          மாறிலி ffi_type *rtype)
{
  தேர்வு (rtype->type) {
    நிலை FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      திரும்பு KVX_REGISTER_SIZE;

    நிலை FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      திரும்பு KVX_REGISTER_SIZE;

    நிலை FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      திரும்பு KVX_REGISTER_SIZE;

    நிலை FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      திரும்பு KVX_REGISTER_SIZE;

    நிலை FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      திரும்பு KVX_REGISTER_SIZE;

    நிலை FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      திரும்பு KVX_REGISTER_SIZE;

    இயல்பு:
      *int_ext_method = KVX_RET_NONE;
      திரும்பு rtype->size;
  }
}

வெறுமை ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  முழு i;
  unsigned long முழு slot_fitting_args_size = 0;
  unsigned long முழு total_size = 0;
  unsigned long முழு big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  கட்டமைப்பு ret_மதிப்பு local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  க்கு (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    எனில் (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      எனில் ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_args_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } இல்லையெனில் {
        slot_fitting_args_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } இல்லையெனில் எனில் ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_args_size += KVX_ABI_SLOT_SIZE;
    } இல்லையெனில் {
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
  total_size = ALIGN(total_size, KVX_ABI_அடுக்கு_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_size = handle_small_int_ext(&int_extension_method, cif->rtype);

  தேர்வு (cif->abi) {
    நிலை FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", total_size);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_size);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_size);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_args_size,
                                   &ecif, rvalue, fn, int_extension_method);
      எனில் ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      நிறுத்து;
    இயல்பு:
      abort();
      நிறுத்து;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
                      வெறுமை *user_data,
                      வெறுமை *codeloc)
{
  திரும்பு FFI_BAD_ABI;
}

#endif /* (__kvx__) */
