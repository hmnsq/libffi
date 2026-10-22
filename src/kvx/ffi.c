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

#જો defined(__kvx__)
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <fficonfig.h>
#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ "ffi_common.h"
#અન્તર્ભાવ "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_ઢગલો_ALIGNMENT (32)
#define KVX_ABI_ઢગલો_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_PRINT(...) કરો{ fprintf( stderr, __VA_ARGS__ ); } જ્યારે(0)
#નહીંતર
#define DEBUG_PRINT(...)
#endif

સંરચના ret_કિંમત {
	unsigned long પૂર્ણાંક r0;
	unsigned long પૂર્ણાંક r1;
	unsigned long પૂર્ણાંક r2;
	unsigned long પૂર્ણાંક r3;
};

બાહ્ય સંરચના ret_કિંમત ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      શૂન્ય *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  પાછા FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

શૂન્ય *ffi_prep_args(અક્ષર *ઢગલો, unsigned પૂર્ણાંક arg_slots_size, extended_cif *ecif)
{
  અક્ષર *stacktemp = ઢગલો;
  અક્ષર *current_arg_passed_by_કિંમત = ઢગલો + arg_slots_size;
  પૂર્ણાંક i, s;
  ffi_type **arg;
  પૂર્ણાંક ગણતરી = 0;
  ffi_cif *cif = ecif->cif;
  શૂન્ય **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", ઢગલો);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_કિંમત);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  માટે (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
    પસંદગી((*arg)->type) {
      સ્થિતિ FFI_TYPE_SINT8:
      સ્થિતિ FFI_TYPE_UINT8:
      સ્થિતિ FFI_TYPE_SINT16:
      સ્થિતિ FFI_TYPE_UINT16:
      સ્થિતિ FFI_TYPE_SINT32:
      સ્થિતિ FFI_TYPE_UINT32:
      સ્થિતિ FFI_TYPE_FLOAT:
      સ્થિતિ FFI_TYPE_DOUBLE:
      સ્થિતિ FFI_TYPE_UINT64:
      સ્થિતિ FFI_TYPE_SINT64:
      સ્થિતિ FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", ઢગલો);
        *(uint64_t *) ઢગલો = *(uint64_t *)(* argv);
        તોડો;

      સ્થિતિ FFI_TYPE_COMPLEX:
        જો ((*arg)->size == 8)
          *(_Complex દશાંશ *) ઢગલો = *(_Complex દશાંશ *)(* argv);
        નહીંતર જો ((*arg)->size == 16) {
          *(_Complex દ્વિગુણ *) ઢગલો = *(_Complex દ્વિગુણ *)(* argv);
          s = 16;
        } નહીંતર
          abort();
        તોડો;
      સ્થિતિ FFI_TYPE_STRUCT: {
        અક્ષર *કિંમત;
        unsigned પૂર્ણાંક written_size = 0;
        DEBUG_PRINT("struct by value @%p\n", ઢગલો);
        જો ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) ઢગલો = (uintptr_t)current_arg_passed_by_કિંમત;
          કિંમત = current_arg_passed_by_કિંમત;
          current_arg_passed_by_કિંમત += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } નહીંતર {
          કિંમત = ઢગલો;
          written_size = (*arg)->size;
        }
        memcpy(કિંમત, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_ઢગલો_ARG_ALIGNMENT);
        તોડો;
      }
      મૂળભૂત:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        તોડો;

    }
    ઢગલો += s;
    ગણતરી += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_SIZE) & (KVX_ABI_ઢગલો_ALIGNMENT-1)) == 0);
#endif
  પાછા stacktemp + REG_ARGS_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned પૂર્ણાંક nfixedargs,
                                    unsigned પૂર્ણાંક ntotalargs)
{
  cif->flags = cif->rtype->size;
  પાછા FFI_OK;
}

સ્થિર unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          અચલ ffi_type *rtype)
{
  પસંદગી (rtype->type) {
    સ્થિતિ FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      પાછા KVX_REGISTER_SIZE;

    સ્થિતિ FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      પાછા KVX_REGISTER_SIZE;

    સ્થિતિ FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      પાછા KVX_REGISTER_SIZE;

    સ્થિતિ FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      પાછા KVX_REGISTER_SIZE;

    સ્થિતિ FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      પાછા KVX_REGISTER_SIZE;

    સ્થિતિ FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      પાછા KVX_REGISTER_SIZE;

    મૂળભૂત:
      *int_ext_method = KVX_RET_NONE;
      પાછા rtype->size;
  }
}

શૂન્ય ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  પૂર્ણાંક i;
  unsigned long પૂર્ણાંક slot_fitting_args_size = 0;
  unsigned long પૂર્ણાંક total_size = 0;
  unsigned long પૂર્ણાંક big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  સંરચના ret_કિંમત local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  માટે (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    જો (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      જો ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_args_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } નહીંતર {
        slot_fitting_args_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } નહીંતર જો ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_args_size += KVX_ABI_SLOT_SIZE;
    } નહીંતર {
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
  total_size = ALIGN(total_size, KVX_ABI_ઢગલો_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_size = handle_small_int_ext(&int_extension_method, cif->rtype);

  પસંદગી (cif->abi) {
    સ્થિતિ FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", total_size);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_size);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_size);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_args_size,
                                   &ecif, rvalue, fn, int_extension_method);
      જો ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      તોડો;
    મૂળભૂત:
      abort();
      તોડો;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
                      શૂન્ય *user_data,
                      શૂન્ય *codeloc)
{
  પાછા FFI_BAD_ABI;
}

#endif /* (__kvx__) */
