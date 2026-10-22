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

#ଯଦି defined(__kvx__)
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <fficonfig.h>
#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ "ffi_common.h"
#ଅନ୍ତର୍ଭାଵ "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_ଗଦା_ALIGNMENT (32)
#define KVX_ABI_ଗଦା_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_PRINT(...) କର{ fprintf( stderr, __VA_ARGS__ ); } ଯେତେବେଳେ(0)
#ଅନ୍ୟଥା
#define DEBUG_PRINT(...)
#endif

ସଂରଚନା ret_ମୂଲ୍ୟ {
	unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା r0;
	unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1;
	unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା r2;
	unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା r3;
};

ବାହ୍ୟ ସଂରଚନା ret_ମୂଲ୍ୟ ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      ଶୂନ୍ୟ *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  ଫେରନ୍ତୁ FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ଶୂନ୍ୟ *ffi_prep_args(ଅକ୍ଷର *ଗଦା, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg_slots_size, extended_cif *ecif)
{
  ଅକ୍ଷର *stacktemp = ଗଦା;
  ଅକ୍ଷର *current_arg_passed_by_ମୂଲ୍ୟ = ଗଦା + arg_slots_size;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, s;
  ffi_type **arg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
  ffi_cif *cif = ecif->cif;
  ଶୂନ୍ୟ **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", ଗଦା);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_ମୂଲ୍ୟ);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  ପାଇଁ (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
    ବାଛ((*arg)->type) {
      ସ୍ଥିତି FFI_TYPE_SINT8:
      ସ୍ଥିତି FFI_TYPE_UINT8:
      ସ୍ଥିତି FFI_TYPE_SINT16:
      ସ୍ଥିତି FFI_TYPE_UINT16:
      ସ୍ଥିତି FFI_TYPE_SINT32:
      ସ୍ଥିତି FFI_TYPE_UINT32:
      ସ୍ଥିତି FFI_TYPE_FLOAT:
      ସ୍ଥିତି FFI_TYPE_DOUBLE:
      ସ୍ଥିତି FFI_TYPE_UINT64:
      ସ୍ଥିତି FFI_TYPE_SINT64:
      ସ୍ଥିତି FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", ଗଦା);
        *(uint64_t *) ଗଦା = *(uint64_t *)(* argv);
        ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_COMPLEX:
        ଯଦି ((*arg)->size == 8)
          *(_Complex ଦଶମିକ *) ଗଦା = *(_Complex ଦଶମିକ *)(* argv);
        ଅନ୍ୟଥା ଯଦି ((*arg)->size == 16) {
          *(_Complex ଦ୍ୱିଗୁଣ *) ଗଦା = *(_Complex ଦ୍ୱିଗୁଣ *)(* argv);
          s = 16;
        } ଅନ୍ୟଥା
          abort();
        ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_STRUCT: {
        ଅକ୍ଷର *ମୂଲ୍ୟ;
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା written_size = 0;
        DEBUG_PRINT("struct by value @%p\n", ଗଦା);
        ଯଦି ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) ଗଦା = (uintptr_t)current_arg_passed_by_ମୂଲ୍ୟ;
          ମୂଲ୍ୟ = current_arg_passed_by_ମୂଲ୍ୟ;
          current_arg_passed_by_ମୂଲ୍ୟ += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } ଅନ୍ୟଥା {
          ମୂଲ୍ୟ = ଗଦା;
          written_size = (*arg)->size;
        }
        memcpy(ମୂଲ୍ୟ, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_ଗଦା_ARG_ALIGNMENT);
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        ଭାଙ୍ଗ;

    }
    ଗଦା += s;
    ଗଣନା_2 += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_SIZE) & (KVX_ABI_ଗଦା_ALIGNMENT-1)) == 0);
#endif
  ଫେରନ୍ତୁ stacktemp + REG_ARGS_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs,
                                    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs)
{
  cif->flags = cif->rtype->size;
  ଫେରନ୍ତୁ FFI_OK;
}

ସ୍ଥିର unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          ଅଚଳ ffi_type *rtype)
{
  ବାଛ (rtype->type) {
    ସ୍ଥିତି FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      ଫେରନ୍ତୁ KVX_REGISTER_SIZE;

    ସ୍ଥିତି FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      ଫେରନ୍ତୁ KVX_REGISTER_SIZE;

    ସ୍ଥିତି FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      ଫେରନ୍ତୁ KVX_REGISTER_SIZE;

    ସ୍ଥିତି FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      ଫେରନ୍ତୁ KVX_REGISTER_SIZE;

    ସ୍ଥିତି FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      ଫେରନ୍ତୁ KVX_REGISTER_SIZE;

    ସ୍ଥିତି FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      ଫେରନ୍ତୁ KVX_REGISTER_SIZE;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      *int_ext_method = KVX_RET_NONE;
      ଫେରନ୍ତୁ rtype->size;
  }
}

ଶୂନ୍ୟ ffi_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା slot_fitting_args_size = 0;
  unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା total_size = 0;
  unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  ସଂରଚନା ret_ମୂଲ୍ୟ local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  ପାଇଁ (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    ଯଦି (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      ଯଦି ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_args_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } ଅନ୍ୟଥା {
        slot_fitting_args_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } ଅନ୍ୟଥା ଯଦି ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_args_size += KVX_ABI_SLOT_SIZE;
    } ଅନ୍ୟଥା {
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
  total_size = ALIGN(total_size, KVX_ABI_ଗଦା_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_size = handle_small_int_ext(&int_extension_method, cif->rtype);

  ବାଛ (cif->abi) {
    ସ୍ଥିତି FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", total_size);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_size);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_size);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_args_size,
                                   &ecif, rvalue, fn, int_extension_method);
      ଯଦି ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort();
      ଭାଙ୍ଗ;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
                      ଶୂନ୍ୟ *user_data,
                      ଶୂନ୍ୟ *codeloc)
{
  ଫେରନ୍ତୁ FFI_BAD_ABI;
}

#endif /* (__kvx__) */
