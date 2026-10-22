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

#if defined(__kvx__)
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fficonfig.h>
#include <ffi.h>
#include "ffi_common.h"
#include "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_सालानसान_ALIGNMENT (32)
#define KVX_ABI_सालानसान_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_इलिम्बाग(...) do{ fprintf( stderr, __VA_ARGS__ ); } while(0)
#else
#define DEBUG_इलिम्बाग(...)
#endif

struct ret_हालागा {
	unsigned long int r0;
	unsigned long int r1;
	unsigned long int r2;
	unsigned long int r3;
};

extern struct ret_हालागा ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      void *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  return FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

void *ffi_prep_मगा_कातविरान(char *सालानसान, unsigned int arg_slots_size, extended_cif *ecif)
{
  char *stacktemp = सालानसान;
  char *current_arg_passed_by_हालागा = सालानसान + arg_slots_size;
  int i, s;
  ffi_type **arg;
  int बिलांग = 0;
  ffi_cif *cif = ecif->cif;
  void **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_इलिम्बाग("stack: %p\n", सालानसान);
  DEBUG_इलिम्बाग("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_इलिम्बाग("current_arg_passed_by_value: %p\n", current_arg_passed_by_हालागा);
  DEBUG_इलिम्बाग("ecif: %p\n", ecif);
  DEBUG_इलिम्बाग("ecif->avalue: %p\n", ecif->avalue);

  for (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
    switch((*arg)->type) {
      case FFI_TYPE_SINT8:
      case FFI_TYPE_UINT8:
      case FFI_TYPE_SINT16:
      case FFI_TYPE_UINT16:
      case FFI_TYPE_SINT32:
      case FFI_TYPE_UINT32:
      case FFI_TYPE_FLOAT:
      case FFI_TYPE_DOUBLE:
      case FFI_TYPE_UINT64:
      case FFI_TYPE_SINT64:
      case FFI_TYPE_POINTER:
        DEBUG_इलिम्बाग("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", सालानसान);
        *(uint64_t *) सालानसान = *(uint64_t *)(* argv);
        break;

      case FFI_TYPE_COMPLEX:
        if ((*arg)->size == 8)
          *(_Complex float *) सालानसान = *(_Complex float *)(* argv);
        else if ((*arg)->size == 16) {
          *(_Complex double *) सालानसान = *(_Complex double *)(* argv);
          s = 16;
        } else
          abort();
        break;
      case FFI_TYPE_STRUCT: {
        char *हालागा;
        unsigned int written_size = 0;
        DEBUG_इलिम्बाग("struct by value @%p\n", सालानसान);
        if ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_इलिम्बाग("big struct\n");
          *(uint64_t *) सालानसान = (uintptr_t)current_arg_passed_by_हालागा;
          हालागा = current_arg_passed_by_हालागा;
          current_arg_passed_by_हालागा += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } else {
          हालागा = सालानसान;
          written_size = (*arg)->size;
        }
        memcpy(हालागा, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_सालानसान_ARG_ALIGNMENT);
        break;
      }
      default:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        break;

    }
    सालानसान += s;
    बिलांग += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_मगा_कातविरान_SIZE) & (KVX_ABI_सालानसान_ALIGNMENT-1)) == 0);
#endif
  return stacktemp + REG_मगा_कातविरान_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned int nfixedargs,
                                    unsigned int ntotalargs)
{
  cif->flags = cif->rtype->size;
  return FFI_OK;
}

static unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          const ffi_type *rtype)
{
  switch (rtype->type) {
    case FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      return KVX_REGISTER_SIZE;

    case FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      return KVX_REGISTER_SIZE;

    case FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      return KVX_REGISTER_SIZE;

    case FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      return KVX_REGISTER_SIZE;

    case FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      return KVX_REGISTER_SIZE;

    case FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      return KVX_REGISTER_SIZE;

    default:
      *int_ext_method = KVX_RET_NONE;
      return rtype->size;
  }
}

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  int i;
  unsigned long int slot_fitting_मगा_कातविरान_size = 0;
  unsigned long int total_size = 0;
  unsigned long int big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  struct ret_हालागा local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  for (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_इलिम्बाग("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    if (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      if ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_मगा_कातविरान_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } else {
        slot_fitting_मगा_कातविरान_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } else if ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_मगा_कातविरान_size += KVX_ABI_SLOT_SIZE;
    } else {
      printf("Error: unsupported arg size %ld arg type %d\n", (*arg)->size, (*arg)->type);
      abort(); /* should never happen? */
    }
  }

  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  /* This implementation allocates anyway for all register based args */
  slot_fitting_मगा_कातविरान_size = max(slot_fitting_मगा_कातविरान_size, REG_मगा_कातविरान_SIZE);
  total_size = slot_fitting_मगा_कातविरान_size + big_struct_size;
  total_size = ALIGN(total_size, KVX_ABI_सालानसान_ALIGNMENT);

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
    case FFI_SYSV:
      DEBUG_इलिम्बाग("total_size: %lu\n", total_size);
      DEBUG_इलिम्बाग("slot fitting args size: %lu\n", slot_fitting_मगा_कातविरान_size);
      DEBUG_इलिम्बाग("rvalue: %p\n", rvalue);
      DEBUG_इलिम्बाग("fn: %p\n", fn);
      DEBUG_इलिम्बाग("rsize: %u\n", cif->flags);
      DEBUG_इलिम्बाग("wb_size: %u\n", wb_size);
      DEBUG_इलिम्बाग("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_मगा_कातविरान_size,
                                   &ecif, rvalue, fn, int_extension_method);
      if ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      break;
    default:
      abort();
      break;
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
  return FFI_BAD_ABI;
}

#endif /* (__kvx__) */
