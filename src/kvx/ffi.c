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

#जइ defined(__kvx__)
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <fficonfig.h>
#अन्तर्भाव <ffi.h>
#अन्तर्भाव "ffi_common.h"
#अन्तर्भाव "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_राशिः_ALIGNMENT (32)
#define KVX_ABI_राशिः_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_PRINT(...) करेहि{ fprintf( stderr, __VA_ARGS__ ); } जाव(0)
#अण्णहा
#define DEBUG_PRINT(...)
#endif

संरचना ret_मानम् {
	unsigned long पूर्णाङ्क r0;
	unsigned long पूर्णाङ्क r1;
	unsigned long पूर्णाङ्क r2;
	unsigned long पूर्णाङ्क r3;
};

बाह्य संरचना ret_मानम् ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      शून्य *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  प्रत्यागम FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

शून्य *ffi_prep_args(वर्ण *राशिः, unsigned पूर्णाङ्क arg_slots_size, extended_cif *ecif)
{
  वर्ण *stacktemp = राशिः;
  वर्ण *current_arg_passed_by_मानम् = राशिः + arg_slots_size;
  पूर्णाङ्क i, s;
  ffi_type **arg;
  पूर्णाङ्क गणनम् = 0;
  ffi_cif *cif = ecif->cif;
  शून्य **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", राशिः);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_मानम्);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  प्रति (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
    विकल्प((*arg)->type) {
      पक्ष FFI_TYPE_SINT8:
      पक्ष FFI_TYPE_UINT8:
      पक्ष FFI_TYPE_SINT16:
      पक्ष FFI_TYPE_UINT16:
      पक्ष FFI_TYPE_SINT32:
      पक्ष FFI_TYPE_UINT32:
      पक्ष FFI_TYPE_FLOAT:
      पक्ष FFI_TYPE_DOUBLE:
      पक्ष FFI_TYPE_UINT64:
      पक्ष FFI_TYPE_SINT64:
      पक्ष FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", राशिः);
        *(uint64_t *) राशिः = *(uint64_t *)(* argv);
        भङ्ग;

      पक्ष FFI_TYPE_COMPLEX:
        जइ ((*arg)->size == 8)
          *(_Complex भिन्न *) राशिः = *(_Complex भिन्न *)(* argv);
        अण्णहा जइ ((*arg)->size == 16) {
          *(_Complex द्विगुण *) राशिः = *(_Complex द्विगुण *)(* argv);
          s = 16;
        } अण्णहा
          abort();
        भङ्ग;
      पक्ष FFI_TYPE_STRUCT: {
        वर्ण *मानम्;
        unsigned पूर्णाङ्क written_size = 0;
        DEBUG_PRINT("struct by value @%p\n", राशिः);
        जइ ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) राशिः = (uintptr_t)current_arg_passed_by_मानम्;
          मानम् = current_arg_passed_by_मानम्;
          current_arg_passed_by_मानम् += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } अण्णहा {
          मानम् = राशिः;
          written_size = (*arg)->size;
        }
        memcpy(मानम्, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_राशिः_ARG_ALIGNMENT);
        भङ्ग;
      }
      स्वभाव:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        भङ्ग;

    }
    राशिः += s;
    गणनम् += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_SIZE) & (KVX_ABI_राशिः_ALIGNMENT-1)) == 0);
#endif
  प्रत्यागम stacktemp + REG_ARGS_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned पूर्णाङ्क nfixedargs,
                                    unsigned पूर्णाङ्क ntotalargs)
{
  cif->flags = cif->rtype->size;
  प्रत्यागम FFI_OK;
}

स्थिर unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          ध्रुव ffi_type *rtype)
{
  विकल्प (rtype->type) {
    पक्ष FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      प्रत्यागम KVX_REGISTER_SIZE;

    पक्ष FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      प्रत्यागम KVX_REGISTER_SIZE;

    पक्ष FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      प्रत्यागम KVX_REGISTER_SIZE;

    पक्ष FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      प्रत्यागम KVX_REGISTER_SIZE;

    पक्ष FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      प्रत्यागम KVX_REGISTER_SIZE;

    पक्ष FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      प्रत्यागम KVX_REGISTER_SIZE;

    स्वभाव:
      *int_ext_method = KVX_RET_NONE;
      प्रत्यागम rtype->size;
  }
}

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  पूर्णाङ्क i;
  unsigned long पूर्णाङ्क slot_fitting_args_size = 0;
  unsigned long पूर्णाङ्क total_size = 0;
  unsigned long पूर्णाङ्क big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  संरचना ret_मानम् local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  प्रति (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    जइ (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      जइ ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_args_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } अण्णहा {
        slot_fitting_args_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } अण्णहा जइ ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_args_size += KVX_ABI_SLOT_SIZE;
    } अण्णहा {
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
  total_size = ALIGN(total_size, KVX_ABI_राशिः_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_size = handle_small_int_ext(&int_extension_method, cif->rtype);

  विकल्प (cif->abi) {
    पक्ष FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", total_size);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_size);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_size);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_args_size,
                                   &ecif, rvalue, fn, int_extension_method);
      जइ ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      भङ्ग;
    स्वभाव:
      abort();
      भङ्ग;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
                      शून्य *user_data,
                      शून्य *codeloc)
{
  प्रत्यागम FFI_BAD_ABI;
}

#endif /* (__kvx__) */
