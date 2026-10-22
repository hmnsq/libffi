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

#ਜੇ defined(__kvx__)
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fficonfig.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "ffi_common.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "asm.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALIGN(x, a) ALIGN_ਨਕਾਬ(x, (typeof(x))(a) - 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALIGN_ਨਕਾਬ(x, ਨਕਾਬ) (((x) + (ਨਕਾਬ)) & ~(ਨਕਾਬ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KVX_ABI_ਢੇਰ_ALIGNMENT (32)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KVX_ABI_ਢੇਰ_ARG_ALIGNMENT (8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ max(a,b) ((a) > (b) ? (a) : (b))

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_DEBUG
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_PRINT(...) ਕਰੋ{ fprintf( stderr, __VA_ARGS__ ); } ਜਦੋਂ(0)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_PRINT(...)
#ਜੇ_ਅੰਤ

ਬਣਤਰ ret_ਮੁੱਲ {
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ r0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ r1;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ r2;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ r3;
};

ਬਾਹਰੀ ਬਣਤਰ ret_ਮੁੱਲ ffi_call_SYSV(ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੁੱਲ_ਮਾਪ,
                                      ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ,
                                      extended_cif *ecif,
                                      ਬਿਨਾਨਿਸ਼ਾਨੀ *rvalue_addr,
                                      ਖਾਲੀ *fn,
                                      ਬਿਨਾਨਿਸ਼ਾਨੀ int_ext_ਵਿਧੀ);

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->ਮਾਪ;
  ਵਾਪਸ FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ਖਾਲੀ *ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ arg_slots_ਮਾਪ, extended_cif *ecif)
{
  ਅੱਖਰ *stacktemp = ਢੇਰ;
  ਅੱਖਰ *current_arg_passed_by_ਮੁੱਲ = ਢੇਰ + arg_slots_ਮਾਪ;
  ਪੂਰਨ i, s;
  ffi_type **arg;
  ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
  ffi_cif *cif = ecif->cif;
  ਖਾਲੀ **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", ਢੇਰ);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_ਮਾਪ);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_ਮੁੱਲ);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  ਲਈ (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_ਥਾਂ_ਮਾਪ;
    ਚੋਣ((*arg)->type) {
      ਹਾਲਤ FFI_TYPE_SINT8:
      ਹਾਲਤ FFI_TYPE_UINT8:
      ਹਾਲਤ FFI_TYPE_SINT16:
      ਹਾਲਤ FFI_TYPE_UINT16:
      ਹਾਲਤ FFI_TYPE_SINT32:
      ਹਾਲਤ FFI_TYPE_UINT32:
      ਹਾਲਤ FFI_TYPE_FLOAT:
      ਹਾਲਤ FFI_TYPE_DOUBLE:
      ਹਾਲਤ FFI_TYPE_UINT64:
      ਹਾਲਤ FFI_TYPE_SINT64:
      ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", ਢੇਰ);
        *(uint64_t *) ਢੇਰ = *(uint64_t *)(* argv);
        ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_COMPLEX:
        ਜੇ ((*arg)->ਮਾਪ == 8)
          *(_Complex ਦਸ਼ਮਲਵ *) ਢੇਰ = *(_Complex ਦਸ਼ਮਲਵ *)(* argv);
        ਨਹੀਂਤਾਂ ਜੇ ((*arg)->ਮਾਪ == 16) {
          *(_Complex ਦੁੱਗਣਾ *) ਢੇਰ = *(_Complex ਦੁੱਗਣਾ *)(* argv);
          s = 16;
        } ਨਹੀਂਤਾਂ
          abort();
        ਰੋਕੋ;
      ਹਾਲਤ FFI_TYPE_STRUCT: {
        ਅੱਖਰ *ਮੁੱਲ;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ written_ਮਾਪ = 0;
        DEBUG_PRINT("struct by value @%p\n", ਢੇਰ);
        ਜੇ ((*arg)->ਮਾਪ > KVX_ABI_MAX_AGGREGATE_ਵਿੱਚ_REG_ਮਾਪ) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) ਢੇਰ = (uintptr_t)current_arg_passed_by_ਮੁੱਲ;
          ਮੁੱਲ = current_arg_passed_by_ਮੁੱਲ;
          current_arg_passed_by_ਮੁੱਲ += (*arg)->ਮਾਪ;
          written_ਮਾਪ = KVX_ABI_ਥਾਂ_ਮਾਪ;
        } ਨਹੀਂਤਾਂ {
          ਮੁੱਲ = ਢੇਰ;
          written_ਮਾਪ = (*arg)->ਮਾਪ;
        }
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਮੁੱਲ, *argv, (*arg)->ਮਾਪ);
        s = ALIGN(written_ਮਾਪ, KVX_ABI_ਢੇਰ_ARG_ALIGNMENT);
        ਰੋਕੋ;
      }
      ਮੂਲ:
        ਲਿਖੋ("Error: unsupported arg type %d\n", (*arg)->type);
        abort();
        ਰੋਕੋ;

    }
    ਢੇਰ += s;
    ਗਿਣਤੀ_2 += s;
    argv++;
    arg++;
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ਦਲੀਲਾਂ_ਮਾਪ) & (KVX_ABI_ਢੇਰ_ALIGNMENT-1)) == 0);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ stacktemp + REG_ਦਲੀਲਾਂ_ਮਾਪ;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_ਸਥਿਤੀ ffi_prep_cif_machdep_var(ffi_cif *cif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs,
                                    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs)
{
  cif->flags = cif->rtype->ਮਾਪ;
  ਵਾਪਸ FFI_OK;
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ handle_small_int_ext(kvx_intext_ਵਿਧੀ *int_ext_ਵਿਧੀ,
                                          ਅਚਲ ffi_type *rtype)
{
  ਚੋਣ (rtype->type) {
    ਹਾਲਤ FFI_TYPE_SINT8:
      *int_ext_ਵਿਧੀ = KVX_RET_SXBD;
      ਵਾਪਸ KVX_ਰਜਿਸਟਰ_ਮਾਪ;

    ਹਾਲਤ FFI_TYPE_SINT16:
      *int_ext_ਵਿਧੀ = KVX_RET_SXHD;
      ਵਾਪਸ KVX_ਰਜਿਸਟਰ_ਮਾਪ;

    ਹਾਲਤ FFI_TYPE_SINT32:
      *int_ext_ਵਿਧੀ = KVX_RET_SXWD;
      ਵਾਪਸ KVX_ਰਜਿਸਟਰ_ਮਾਪ;

    ਹਾਲਤ FFI_TYPE_UINT8:
      *int_ext_ਵਿਧੀ = KVX_RET_ZXBD;
      ਵਾਪਸ KVX_ਰਜਿਸਟਰ_ਮਾਪ;

    ਹਾਲਤ FFI_TYPE_UINT16:
      *int_ext_ਵਿਧੀ = KVX_RET_ZXHD;
      ਵਾਪਸ KVX_ਰਜਿਸਟਰ_ਮਾਪ;

    ਹਾਲਤ FFI_TYPE_UINT32:
      *int_ext_ਵਿਧੀ = KVX_RET_ZXWD;
      ਵਾਪਸ KVX_ਰਜਿਸਟਰ_ਮਾਪ;

    ਮੂਲ:
      *int_ext_ਵਿਧੀ = KVX_RET_NONE;
      ਵਾਪਸ rtype->ਮਾਪ;
  }
}

ਖਾਲੀ ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ਪੂਰਨ i;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ ਥਾਂ_fitting_ਦਲੀਲਾਂ_ਮਾਪ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ ਕੁੱਲ_ਮਾਪ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ big_struct_ਮਾਪ = 0;
  kvx_intext_ਵਿਧੀ int_extension_ਵਿਧੀ;
  ffi_type **arg;
  ਬਣਤਰ ret_ਮੁੱਲ local_rvalue = {0};
  size_t wb_ਮਾਪ;


  /* Calculate size to allocate on stack */
  ਲਈ (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->ਮਾਪ);
    ਜੇ (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      ਜੇ ((*arg)->ਮਾਪ <= KVX_ABI_MAX_AGGREGATE_ਵਿੱਚ_REG_ਮਾਪ) {
        ਥਾਂ_fitting_ਦਲੀਲਾਂ_ਮਾਪ += ALIGN((*arg)->ਮਾਪ, KVX_ABI_ਥਾਂ_ਮਾਪ);
      } ਨਹੀਂਤਾਂ {
        ਥਾਂ_fitting_ਦਲੀਲਾਂ_ਮਾਪ += KVX_ABI_ਥਾਂ_ਮਾਪ; /* aggregate passed by reference */
        big_struct_ਮਾਪ += ALIGN((*arg)->ਮਾਪ, KVX_ABI_ਥਾਂ_ਮਾਪ);
      }
    } ਨਹੀਂਤਾਂ ਜੇ ((*arg)->ਮਾਪ <= KVX_ABI_ਥਾਂ_ਮਾਪ) {
      ਥਾਂ_fitting_ਦਲੀਲਾਂ_ਮਾਪ += KVX_ABI_ਥਾਂ_ਮਾਪ;
    } ਨਹੀਂਤਾਂ {
      ਲਿਖੋ("Error: unsupported arg size %ld arg type %d\n", (*arg)->ਮਾਪ, (*arg)->type);
      abort(); /* should never happen? */
    }
  }

  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  /* This implementation allocates anyway for all register based args */
  ਥਾਂ_fitting_ਦਲੀਲਾਂ_ਮਾਪ = max(ਥਾਂ_fitting_ਦਲੀਲਾਂ_ਮਾਪ, REG_ਦਲੀਲਾਂ_ਮਾਪ);
  ਕੁੱਲ_ਮਾਪ = ਥਾਂ_fitting_ਦਲੀਲਾਂ_ਮਾਪ + big_struct_ਮਾਪ;
  ਕੁੱਲ_ਮਾਪ = ALIGN(ਕੁੱਲ_ਮਾਪ, KVX_ABI_ਢੇਰ_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_ਮਾਪ = handle_small_int_ext(&int_extension_ਵਿਧੀ, cif->rtype);

  ਚੋਣ (cif->abi) {
    ਹਾਲਤ FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", ਕੁੱਲ_ਮਾਪ);
      DEBUG_PRINT("slot fitting args size: %lu\n", ਥਾਂ_fitting_ਦਲੀਲਾਂ_ਮਾਪ);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_ਮਾਪ);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_ਵਿਧੀ);
      local_rvalue = ffi_call_SYSV(ਕੁੱਲ_ਮਾਪ, ਥਾਂ_fitting_ਦਲੀਲਾਂ_ਮਾਪ,
                                   &ecif, rvalue, fn, int_extension_ਵਿਧੀ);
      ਜੇ ((cif->flags <= KVX_ABI_MAX_AGGREGATE_ਵਿੱਚ_REG_ਮਾਪ)
          && (cif->rtype->type != FFI_TYPE_VOID))
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(rvalue, &local_rvalue, wb_ਮਾਪ);
      ਰੋਕੋ;
    ਮੂਲ:
      abort();
      ਰੋਕੋ;
  }
}

/* Closures not supported yet */
ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
                      ffi_cif* cif,
                      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
                      ਖਾਲੀ *user_ਅੰਕੜੇ,
                      ਖਾਲੀ *codeloc)
{
  ਵਾਪਸ FFI_BAD_ABI;
}

#ਜੇ_ਅੰਤ /* (__kvx__) */
