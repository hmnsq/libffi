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

#എങ്കിൽ defined(__kvx__)
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <fficonfig.h>
#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ "ffi_common.h"
#അന്തര്ഭാവ "asm.h"

#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define KVX_ABI_അട്ടി_ALIGNMENT (32)
#define KVX_ABI_അട്ടി_ARG_ALIGNMENT (8)
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifdef FFI_DEBUG
#define DEBUG_PRINT(...) ചെയ്യുക{ fprintf( stderr, __VA_ARGS__ ); } ആകുമ്പോൾ(0)
#അല്ലെങ്കിൽ
#define DEBUG_PRINT(...)
#endif

ഘടന ret_മൂല്യം {
	unsigned long പൂർണ്ണസംഖ്യ r0;
	unsigned long പൂർണ്ണസംഖ്യ r1;
	unsigned long പൂർണ്ണസംഖ്യ r2;
	unsigned long പൂർണ്ണസംഖ്യ r3;
};

ബാഹ്യ ഘടന ret_മൂല്യം ffi_call_SYSV(unsigned total_size,
                                      unsigned size,
                                      extended_cif *ecif,
                                      unsigned *rvalue_addr,
                                      ശൂന്യം *fn,
                                      unsigned int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->size;
  തിരികെ FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ശൂന്യം *ffi_prep_args(അക്ഷരം *അട്ടി, unsigned പൂർണ്ണസംഖ്യ arg_slots_size, extended_cif *ecif)
{
  അക്ഷരം *stacktemp = അട്ടി;
  അക്ഷരം *current_arg_passed_by_മൂല്യം = അട്ടി + arg_slots_size;
  പൂർണ്ണസംഖ്യ i, s;
  ffi_type **arg;
  പൂർണ്ണസംഖ്യ എണ്ണം = 0;
  ffi_cif *cif = ecif->cif;
  ശൂന്യം **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", അട്ടി);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_size);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_മൂല്യം);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  വേണ്ടി (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_SIZE;
    തിരഞ്ഞെടുപ്പ്((*arg)->type) {
      അവസ്ഥ FFI_TYPE_SINT8:
      അവസ്ഥ FFI_TYPE_UINT8:
      അവസ്ഥ FFI_TYPE_SINT16:
      അവസ്ഥ FFI_TYPE_UINT16:
      അവസ്ഥ FFI_TYPE_SINT32:
      അവസ്ഥ FFI_TYPE_UINT32:
      അവസ്ഥ FFI_TYPE_FLOAT:
      അവസ്ഥ FFI_TYPE_DOUBLE:
      അവസ്ഥ FFI_TYPE_UINT64:
      അവസ്ഥ FFI_TYPE_SINT64:
      അവസ്ഥ FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", അട്ടി);
        *(uint64_t *) അട്ടി = *(uint64_t *)(* argv);
        നിർത്തുക;

      അവസ്ഥ FFI_TYPE_COMPLEX:
        എങ്കിൽ ((*arg)->size == 8)
          *(_Complex ദശാംശം *) അട്ടി = *(_Complex ദശാംശം *)(* argv);
        അല്ലെങ്കിൽ എങ്കിൽ ((*arg)->size == 16) {
          *(_Complex ഇരട്ട *) അട്ടി = *(_Complex ഇരട്ട *)(* argv);
          s = 16;
        } അല്ലെങ്കിൽ
          abort();
        നിർത്തുക;
      അവസ്ഥ FFI_TYPE_STRUCT: {
        അക്ഷരം *മൂല്യം;
        unsigned പൂർണ്ണസംഖ്യ written_size = 0;
        DEBUG_PRINT("struct by value @%p\n", അട്ടി);
        എങ്കിൽ ((*arg)->size > KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) അട്ടി = (uintptr_t)current_arg_passed_by_മൂല്യം;
          മൂല്യം = current_arg_passed_by_മൂല്യം;
          current_arg_passed_by_മൂല്യം += (*arg)->size;
          written_size = KVX_ABI_SLOT_SIZE;
        } അല്ലെങ്കിൽ {
          മൂല്യം = അട്ടി;
          written_size = (*arg)->size;
        }
        memcpy(മൂല്യം, *argv, (*arg)->size);
        s = ALIGN(written_size, KVX_ABI_അട്ടി_ARG_ALIGNMENT);
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി:
        printf("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        നിർത്തുക;

    }
    അട്ടി += s;
    എണ്ണം += s;
    argv++;
    arg++;
  }
#ifdef FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_SIZE) & (KVX_ABI_അട്ടി_ALIGNMENT-1)) == 0);
#endif
  തിരികെ stacktemp + REG_ARGS_SIZE;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned പൂർണ്ണസംഖ്യ nfixedargs,
                                    unsigned പൂർണ്ണസംഖ്യ ntotalargs)
{
  cif->flags = cif->rtype->size;
  തിരികെ FFI_OK;
}

സ്ഥിതം unsigned long handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          സ്ഥിരം ffi_type *rtype)
{
  തിരഞ്ഞെടുപ്പ് (rtype->type) {
    അവസ്ഥ FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      തിരികെ KVX_REGISTER_SIZE;

    അവസ്ഥ FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      തിരികെ KVX_REGISTER_SIZE;

    അവസ്ഥ FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      തിരികെ KVX_REGISTER_SIZE;

    അവസ്ഥ FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      തിരികെ KVX_REGISTER_SIZE;

    അവസ്ഥ FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      തിരികെ KVX_REGISTER_SIZE;

    അവസ്ഥ FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      തിരികെ KVX_REGISTER_SIZE;

    സ്ഥിരസ്ഥിതി:
      *int_ext_method = KVX_RET_NONE;
      തിരികെ rtype->size;
  }
}

ശൂന്യം ffi_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  പൂർണ്ണസംഖ്യ i;
  unsigned long പൂർണ്ണസംഖ്യ slot_fitting_args_size = 0;
  unsigned long പൂർണ്ണസംഖ്യ total_size = 0;
  unsigned long പൂർണ്ണസംഖ്യ big_struct_size = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  ഘടന ret_മൂല്യം local_rvalue = {0};
  size_t wb_size;


  /* Calculate size to allocate on stack */
  വേണ്ടി (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->size);
    എങ്കിൽ (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      എങ്കിൽ ((*arg)->size <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE) {
        slot_fitting_args_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      } അല്ലെങ്കിൽ {
        slot_fitting_args_size += KVX_ABI_SLOT_SIZE; /* aggregate passed by reference */
        big_struct_size += ALIGN((*arg)->size, KVX_ABI_SLOT_SIZE);
      }
    } അല്ലെങ്കിൽ എങ്കിൽ ((*arg)->size <= KVX_ABI_SLOT_SIZE) {
      slot_fitting_args_size += KVX_ABI_SLOT_SIZE;
    } അല്ലെങ്കിൽ {
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
  total_size = ALIGN(total_size, KVX_ABI_അട്ടി_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_size = handle_small_int_ext(&int_extension_method, cif->rtype);

  തിരഞ്ഞെടുപ്പ് (cif->abi) {
    അവസ്ഥ FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", total_size);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_size);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_size);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(total_size, slot_fitting_args_size,
                                   &ecif, rvalue, fn, int_extension_method);
      എങ്കിൽ ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_SIZE)
          && (cif->rtype->type != FFI_TYPE_VOID))
        memcpy(rvalue, &local_rvalue, wb_size);
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      abort();
      നിർത്തുക;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
                      ശൂന്യം *user_data,
                      ശൂന്യം *codeloc)
{
  തിരികെ FFI_BAD_ABI;
}

#endif /* (__kvx__) */
