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
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <fficonfig.h>
#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব "ffi_common.h"
#অন্তর্ভাব "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_স্তূপ_ALIGNMENT (32)
#define KVX_ABI_স্তূপ_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_PRINT(...) করো{ fprintf( stderr, __VA_ARGS__ ); } যখন(0)
#অন্যথা
#define DEBUG_PRINT(...)
#endif

কাঠামো ret_মান {
	unsigned long পূর্ণসংখ্যা r0;
	unsigned long পূর্ণসংখ্যা r1;
	unsigned long পূর্ণসংখ্যা r2;
	unsigned long পূর্ণসংখ্যা r3;
};

বাহ্যিক কাঠামো ret_মান ffi_call_SYSV(unsigned যোগ_আকার,
                                      unsigned আকার_2,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      শূন্য *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->আকার_2;
  ফেরত FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

শূন্য *ffi_prep_args(অক্ষর *স্তূপ, unsigned পূর্ণসংখ্যা arg_slots_আকার, extended_cif *ecif)
{
  অক্ষর *stacktemp = স্তূপ;
  অক্ষর *current_arg_passed_by_মান = স্তূপ + arg_slots_আকার;
  পূর্ণসংখ্যা i, s;
  ffi_type **arg;
  পূর্ণসংখ্যা গুনতি = 0;
  ffi_cif *cif = ecif->cif;
  শূন্য **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", স্তূপ);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_আকার);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_মান);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  জন্য (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_আকার;
    নির্বাচন((*arg)->type) {
      ক্ষেত্র FFI_TYPE_SINT8:
      ক্ষেত্র FFI_TYPE_UINT8:
      ক্ষেত্র FFI_TYPE_SINT16:
      ক্ষেত্র FFI_TYPE_UINT16:
      ক্ষেত্র FFI_TYPE_SINT32:
      ক্ষেত্র FFI_TYPE_UINT32:
      ক্ষেত্র FFI_TYPE_FLOAT:
      ক্ষেত্র FFI_TYPE_DOUBLE:
      ক্ষেত্র FFI_TYPE_UINT64:
      ক্ষেত্র FFI_TYPE_SINT64:
      ক্ষেত্র FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", স্তূপ);
        *(uint64_t *) স্তূপ = *(uint64_t *)(* argv);
        বিরতি;

      ক্ষেত্র FFI_TYPE_COMPLEX:
        যদি ((*arg)->আকার_2 == 8)
          *(_Complex দশমিক *) স্তূপ = *(_Complex দশমিক *)(* argv);
        অন্যথা যদি ((*arg)->আকার_2 == 16) {
          *(_Complex দ্বিগুণ *) স্তূপ = *(_Complex দ্বিগুণ *)(* argv);
          s = 16;
        } অন্যথা
          abort();
        বিরতি;
      ক্ষেত্র FFI_TYPE_STRUCT: {
        অক্ষর *মান;
        unsigned পূর্ণসংখ্যা written_আকার = 0;
        DEBUG_PRINT("struct by value @%p\n", স্তূপ);
        যদি ((*arg)->আকার_2 > KVX_ABI_MAX_AGGREGATE_IN_REG_আকার) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) স্তূপ = (uintptr_t)current_arg_passed_by_মান;
          মান = current_arg_passed_by_মান;
          current_arg_passed_by_মান += (*arg)->আকার_2;
          written_আকার = KVX_ABI_SLOT_আকার;
        } অন্যথা {
          মান = স্তূপ;
          written_আকার = (*arg)->আকার_2;
        }
        memcpy(মান, *argv, (*arg)->আকার_2);
        s = ALIGN(written_আকার, KVX_ABI_স্তূপ_ARG_ALIGNMENT);
        বিরতি;
      }
      পূর্বনির্ধারিত:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        বিরতি;

    }
    স্তূপ += s;
    গুনতি += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_আকার) & (KVX_ABI_স্তূপ_ALIGNMENT-1)) == 0);
#endif
  ফেরত stacktemp + REG_ARGS_আকার;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned পূর্ণসংখ্যা nfixedargs,
                                    unsigned পূর্ণসংখ্যা ntotalargs)
{
  cif->flags = cif->rtype->আকার_2;
  ফেরত FFI_OK;
}

স্থির unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          ধ্রুবক ffi_type *rtype)
{
  নির্বাচন (rtype->type) {
    ক্ষেত্র FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      ফেরত KVX_REGISTER_আকার;

    ক্ষেত্র FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      ফেরত KVX_REGISTER_আকার;

    ক্ষেত্র FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      ফেরত KVX_REGISTER_আকার;

    ক্ষেত্র FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      ফেরত KVX_REGISTER_আকার;

    ক্ষেত্র FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      ফেরত KVX_REGISTER_আকার;

    ক্ষেত্র FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      ফেরত KVX_REGISTER_আকার;

    পূর্বনির্ধারিত:
      *int_ext_method = KVX_RET_NONE;
      ফেরত rtype->আকার_2;
  }
}

শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  পূর্ণসংখ্যা i;
  unsigned long পূর্ণসংখ্যা slot_fitting_args_আকার = 0;
  unsigned long পূর্ণসংখ্যা যোগ_আকার = 0;
  unsigned long পূর্ণসংখ্যা big_struct_আকার = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  কাঠামো ret_মান local_rvalue = {0};
  size_t wb_আকার;


  /* Calculate size to allocate on stack */
  জন্য (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->আকার_2);
    যদি (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      যদি ((*arg)->আকার_2 <= KVX_ABI_MAX_AGGREGATE_IN_REG_আকার) {
        slot_fitting_args_আকার += ALIGN((*arg)->আকার_2, KVX_ABI_SLOT_আকার);
      } অন্যথা {
        slot_fitting_args_আকার += KVX_ABI_SLOT_আকার; /* aggregate passed by reference */
        big_struct_আকার += ALIGN((*arg)->আকার_2, KVX_ABI_SLOT_আকার);
      }
    } অন্যথা যদি ((*arg)->আকার_2 <= KVX_ABI_SLOT_আকার) {
      slot_fitting_args_আকার += KVX_ABI_SLOT_আকার;
    } অন্যথা {
      printf("Error: unsupported arg size %ld arg type %d\n", (*arg)->আকার_2, (*arg)->type);
      abort(); /* should never happen? */
    }
  }

  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  /* This implementation allocates anyway for all register based args */
  slot_fitting_args_আকার = max(slot_fitting_args_আকার, REG_ARGS_আকার);
  যোগ_আকার = slot_fitting_args_আকার + big_struct_আকার;
  যোগ_আকার = ALIGN(যোগ_আকার, KVX_ABI_স্তূপ_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_আকার = handle_small_int_ext(&int_extension_method, cif->rtype);

  নির্বাচন (cif->abi) {
    ক্ষেত্র FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", যোগ_আকার);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_আকার);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_আকার);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(যোগ_আকার, slot_fitting_args_আকার,
                                   &ecif, rvalue, fn, int_extension_method);
      যদি ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_আকার)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_আকার);
      বিরতি;
    পূর্বনির্ধারিত:
      abort();
      বিরতি;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
                      শূন্য *user_তথ্য,
                      শূন্য *codeloc)
{
  ফেরত FFI_BAD_ABI;
}

#endif /* (__kvx__) */
