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

#ಯದಿ defined(__kvx__)
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <fficonfig.h>
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ "ffi_common.h"
#ಅನ್ತರ್ಭಾವ "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_ರಾಶಿ_ALIGNMENT (32)
#define KVX_ABI_ರಾಶಿ_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_PRINT(...) ಕುರು{ fprintf( stderr, __VA_ARGS__ ); } ಯಾವತ್(0)
#ಅನ್ಯಥಾ
#define DEBUG_PRINT(...)
#endif

ಸಂರಚನಾ ret_ಮೌಲ್ಯ {
	unsigned long ಪೂರ್ಣಾಙ್ಕ r0;
	unsigned long ಪೂರ್ಣಾಙ್ಕ r1;
	unsigned long ಪೂರ್ಣಾಙ್ಕ r2;
	unsigned long ಪೂರ್ಣಾಙ್ಕ r3;
};

ಬಾಹ್ಯ ಸಂರಚನಾ ret_ಮೌಲ್ಯ ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      ಶೂನ್ಯ *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಶೂನ್ಯ *ffi_prep_args(ವರ್ಣ *ರಾಶಿ, unsigned ಪೂರ್ಣಾಙ್ಕ arg_slots_size, extended_cif *ecif)
{
  ವರ್ಣ *stacktemp = ರಾಶಿ;
  ವರ್ಣ *current_arg_passed_by_ಮೌಲ್ಯ = ರಾಶಿ + arg_slots_size;
  ಪೂರ್ಣಾಙ್ಕ i, s;
  ffi_type **arg;
  ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
  ffi_cif *cif = ecif->cif;
  ಶೂನ್ಯ **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", ರಾಶಿ);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_ಮೌಲ್ಯ);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  ಪ್ರತಿ (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
    ವಿಕಲ್ಪ((*arg)->type) {
      ಪಕ್ಷ FFI_TYPE_SINT8:
      ಪಕ್ಷ FFI_TYPE_UINT8:
      ಪಕ್ಷ FFI_TYPE_SINT16:
      ಪಕ್ಷ FFI_TYPE_UINT16:
      ಪಕ್ಷ FFI_TYPE_SINT32:
      ಪಕ್ಷ FFI_TYPE_UINT32:
      ಪಕ್ಷ FFI_TYPE_FLOAT:
      ಪಕ್ಷ FFI_TYPE_DOUBLE:
      ಪಕ್ಷ FFI_TYPE_UINT64:
      ಪಕ್ಷ FFI_TYPE_SINT64:
      ಪಕ್ಷ FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", ರಾಶಿ);
        *(uint64_t *) ರಾಶಿ = *(uint64_t *)(* argv);
        ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_COMPLEX:
        ಯದಿ ((*arg)->size == 8)
          *(_Complex ಭಿನ್ನ *) ರಾಶಿ = *(_Complex ಭಿನ್ನ *)(* argv);
        ಅನ್ಯಥಾ ಯದಿ ((*arg)->size == 16) {
          *(_Complex ದ್ವಿಗುಣ *) ರಾಶಿ = *(_Complex ದ್ವಿಗುಣ *)(* argv);
          s = 16;
        } ಅನ್ಯಥಾ
          abort();
        ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_STRUCT: {
        ವರ್ಣ *ಮೌಲ್ಯ;
        unsigned ಪೂರ್ಣಾಙ್ಕ written_size = 0;
        DEBUG_PRINT("struct by value @%p\n", ರಾಶಿ);
        ಯದಿ ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) ರಾಶಿ = (uintptr_t)current_arg_passed_by_ಮೌಲ್ಯ;
          ಮೌಲ್ಯ = current_arg_passed_by_ಮೌಲ್ಯ;
          current_arg_passed_by_ಮೌಲ್ಯ += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } ಅನ್ಯಥಾ {
          ಮೌಲ್ಯ = ರಾಶಿ;
          written_size = (*arg)->size;
        }
        memcpy(ಮೌಲ್ಯ, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_ರಾಶಿ_ARG_ALIGNMENT);
        ಭಙ್ಗ;
      }
      ಸ್ವಭಾವ:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        ಭಙ್ಗ;

    }
    ರಾಶಿ += s;
    ಎಣಿಕೆ += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_SIZE) & (KVX_ABI_ರಾಶಿ_ALIGNMENT-1)) == 0);
#endif
  ಪ್ರತ್ಯಾಗಮ stacktemp + REG_ARGS_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs,
                                    unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs)
{
  cif->flags = cif->rtype->size;
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಸ್ಥಿರ unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          ಧ್ರುವ ffi_type *rtype)
{
  ವಿಕಲ್ಪ (rtype->type) {
    ಪಕ್ಷ FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      ಪ್ರತ್ಯಾಗಮ KVX_REGISTER_SIZE;

    ಪಕ್ಷ FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      ಪ್ರತ್ಯಾಗಮ KVX_REGISTER_SIZE;

    ಪಕ್ಷ FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      ಪ್ರತ್ಯಾಗಮ KVX_REGISTER_SIZE;

    ಪಕ್ಷ FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      ಪ್ರತ್ಯಾಗಮ KVX_REGISTER_SIZE;

    ಪಕ್ಷ FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      ಪ್ರತ್ಯಾಗಮ KVX_REGISTER_SIZE;

    ಪಕ್ಷ FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      ಪ್ರತ್ಯಾಗಮ KVX_REGISTER_SIZE;

    ಸ್ವಭಾವ:
      *int_ext_method = KVX_RET_NONE;
      ಪ್ರತ್ಯಾಗಮ rtype->size;
  }
}

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ಪೂರ್ಣಾಙ್ಕ i;
  unsigned long ಪೂರ್ಣಾಙ್ಕ slot_fitting_args_size = 0;
  unsigned long ಪೂರ್ಣಾಙ್ಕ total_size = 0;
  unsigned long ಪೂರ್ಣಾಙ್ಕ big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  ಸಂರಚನಾ ret_ಮೌಲ್ಯ local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  ಪ್ರತಿ (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    ಯದಿ (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      ಯದಿ ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_args_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } ಅನ್ಯಥಾ {
        slot_fitting_args_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } ಅನ್ಯಥಾ ಯದಿ ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_args_size += KVX_ABI_SLOT_SIZE;
    } ಅನ್ಯಥಾ {
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
  total_size = ALIGN(total_size, KVX_ABI_ರಾಶಿ_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_size = handle_small_int_ext(&int_extension_method, cif->rtype);

  ವಿಕಲ್ಪ (cif->abi) {
    ಪಕ್ಷ FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", total_size);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_size);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_size);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_args_size,
                                   &ecif, rvalue, fn, int_extension_method);
      ಯದಿ ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      abort();
      ಭಙ್ಗ;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
                      ಶೂನ್ಯ *user_data,
                      ಶೂನ್ಯ *codeloc)
{
  ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
}

#endif /* (__kvx__) */
