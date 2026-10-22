/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2018-2023  Hood Chatham, Brion Vibber, Kleis Auke Wolthuizen, and others.
           Copyright (c) 2026  Anthony Green

   wasm32/emscripten Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>

#ಅನ್ತರ್ಭಾವ <emscripten/emscripten.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG_F
#ವ್ಯಾಖ್ಯಾನಿಸು LOG_DEBUG(ವಾದಗಳು...)  \
  console.warn(`====LIBFFI(ಸಾಲು __LINE__)`, ವಾದಗಳು)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು LOG_DEBUG(ವಾದಗಳು...) 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು EM_JS_MACROS(ret, name, ವಾದಗಳು, body...) EM_JS(ret, name, ವಾದಗಳು, body)

EM_JS_DEPS(libffi, "$getWasmTableEntry,$setWasmTableEntry,$getEmptyTableSlot,$wasmTable,$stackSave,$stackAlloc,$stackRestore");

#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_U8(addr, offset) HEAPU8[addr + offset]
#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_S8(addr, offset) HEAP8[addr + offset]
#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_U16(addr, offset) HEAPU16[(addr >> 1) + offset]
#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_S16(addr, offset) HEAP16[(addr >> 1) + offset]
#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_U32(addr, offset) HEAPU32[(addr >> 2) + offset]
#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_S32(addr, offset) HEAP32[(addr >> 2) + offset]

#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_F32(addr, offset) HEAPF32[(addr >> 2) + offset]
#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_F64(addr, offset) HEAPF64[(addr >> 3) + offset]
#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_U64(addr, offset) HEAPU64[(addr >> 3) + offset]

#ವ್ಯಾಖ್ಯಾನಿಸು CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ರಚನೆ, ಕ್ಷೇತ್ರ, offset)                                  \
  _Static_assert(                                                                  \
    offsetof(ರಚನೆ, ಕ್ಷೇತ್ರ) == offset,                                             \
    "Memory layout of '" #struct "' has changed: '" #field "' is in an unexpected location");

#ಆದರೆ __SIZEOF_POINTER__ == 4

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_EMSCRIPTEN_ABI FFI_WASM32_EMSCRIPTEN
#ವ್ಯಾಖ್ಯಾನಿಸು PTR_SIG 'i'
#ವ್ಯಾಖ್ಯಾನಿಸು PTR_WASM_TYPE 0x7f

#ವ್ಯಾಖ್ಯಾನಿಸು DEC_PTR(p) p
#ವ್ಯಾಖ್ಯಾನಿಸು ENC_PTR(p) p

#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_PTR(addr, offset) DEREF_U32(addr, offset)
#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_PTR_ಸಂಖ್ಯೆ(addr, offset) DEREF_PTR(addr, offset)

// Store an integral return value widened to ffi_arg (32 bits on wasm32), as
// the libffi API promises for integral returns narrower than a register.
// The typed-array store wraps modulo 2^32, so a sign-extended negative
// Number is stored correctly by both variants.
#ವ್ಯಾಖ್ಯಾನಿಸು STORE_ARG_WIDENED_UNSIGNED(rvalue, x) (DEREF_U32(rvalue, 0) = (x))
#ವ್ಯಾಖ್ಯಾನಿಸು STORE_ARG_WIDENED_SIGNED(rvalue, x) (DEREF_U32(rvalue, 0) = (x))

CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, abi, 4*0);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, nargs, 4*1);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, arg_types, 4*2);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, rtype, 4*3);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, flags, 4*5);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, nfixedargs, 4*6);

#ವ್ಯಾಖ್ಯಾನಿಸು CIF__ABI(addr) DEREF_U32(addr, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF__NARGS(addr) DEREF_U32(addr, 1)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF__ARGTYPES(addr) DEREF_U32(addr, 2)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF__RTYPE(addr) DEREF_U32(addr, 3)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF__FLAGS(addr) DEREF_U32(addr, 5)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF__NFIXEDARGS(addr) DEREF_U32(addr, 6)

CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_type, ಗಾತ್ರ_2, 0);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_type, alignment, 4);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_type, type, 6);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_type, elements, 8);

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_ಗಾತ್ರ(addr) DEREF_U32(addr, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE__ALIGN(addr) DEREF_U16(addr + 4, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE__TYPEID(addr) DEREF_U16(addr + 6, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE__ELEMENTS(addr) DEREF_U32(addr + 8, 0)

#ಅಥವಾಆದರೆ __SIZEOF_POINTER__ == 8

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_EMSCRIPTEN_ABI FFI_WASM64_EMSCRIPTEN
#ವ್ಯಾಖ್ಯಾನಿಸು PTR_SIG 'j'
#ವ್ಯಾಖ್ಯಾನಿಸು PTR_WASM_TYPE 0x7e

// DEC_PTR casts a pointer value (coming from Wasm) represented as BigInt (i64) to Number (i53).
// This should be used for a pointer that is expected to be within the i53 range. If the pointer
// value is outside the Number's range, the value will become NaN.
#ವ್ಯಾಖ್ಯಾನಿಸು DEC_PTR(p) bigint_ಗಮ್ಯ_I53_Checked(p)
// ENC_PTR casts a pointer value represented as Number to BigInt (i64)
#ವ್ಯಾಖ್ಯಾನಿಸು ENC_PTR(p) BigInt(p)

#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_PTR(addr, offset) DEREF_U64(addr, offset)
#ವ್ಯಾಖ್ಯಾನಿಸು DEREF_PTR_ಸಂಖ್ಯೆ(addr, offset) DEC_PTR(DEREF_PTR(addr, offset))

// Store an integral return value widened to ffi_arg (64 bits on wasm64), as
// the libffi API promises for integral returns narrower than a register.
// BigInt masking treats the value as infinite-precision two's complement, so
// the unsigned variant zero-extends a possibly-negative i32 Number, while
// the BigUint64Array store wraps the signed variant modulo 2^64, which is
// sign extension.
#ವ್ಯಾಖ್ಯಾನಿಸು STORE_ARG_WIDENED_UNSIGNED(rvalue, x) \
  (DEREF_U64(rvalue, 0) = BigInt(x) & BigInt(4294967295))
#ವ್ಯಾಖ್ಯಾನಿಸು STORE_ARG_WIDENED_SIGNED(rvalue, x) (DEREF_U64(rvalue, 0) = BigInt(x))

CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, abi, 0);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, nargs, 4);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, arg_types, 8);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, rtype, 16);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, flags, 28);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_cif, nfixedargs, 32);

#ವ್ಯಾಖ್ಯಾನಿಸು CIF__ABI(addr) DEREF_U32(addr, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF__NARGS(addr) DEREF_U32(addr + 4,  0)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF__ARGTYPES(addr) DEREF_U64(addr + 8,  0)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF__RTYPE(addr) DEREF_U64(addr + 16, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF__FLAGS(addr) DEREF_U32(addr + 28, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು CIF__NFIXEDARGS(addr) DEREF_U32(addr + 32, 0)

CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_type, ಗಾತ್ರ_2, 0);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_type, alignment, 8);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_type, type, 10);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_type, elements, 16);

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_ಗಾತ್ರ(addr) DEREF_U64(addr, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE__ALIGN(addr) DEREF_U16(addr + 8, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE__TYPEID(addr) DEREF_U16(addr + 10, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE__ELEMENTS(addr) DEREF_U64(addr + 16, 0)

#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ "Unknown pointer size"
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು ALIGN_ವಿಳಾಸ(addr, align) (addr &= (~((align) - 1)))
#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_ALLOC(ರಾಶಿ, ಗಾತ್ರ_2, align) ((ರಾಶಿ -= (ಗಾತ್ರ_2)), ALIGN_ವಿಳಾಸ(ರಾಶಿ, align))

// Most wasm runtimes support at most 1000 Js trampoline args.
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_ವಾದಗಳು 1000

#ಅನ್ತರ್ಭಾವ <stddef.h>

#ವ್ಯಾಖ್ಯಾನಿಸು VARARGS_FLAG 1

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_OK_MACRO 0
_Static_assert(FFI_OK_MACRO == FFI_OK, "FFI_OK must be 0");

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_BAD_TYPEDEF_MACRO 1
_Static_assert(FFI_BAD_TYPEDEF_MACRO == FFI_BAD_TYPEDEF, "FFI_BAD_TYPEDEF must be 1");

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  ಆದರೆ (cif->abi != FFI_EMSCRIPTEN_ABI)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
  // This is called after ffi_prep_cif_machdep_var so we need to avoid
  // overwriting cif->nfixedargs.
  ಆದರೆ (!(cif->flags & VARARGS_FLAG))
    cif->nfixedargs = cif->nargs;
  ಆದರೆ (cif->nargs > MAX_ವಾದಗಳು)
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
  ಆದರೆ (cif->rtype->type == FFI_TYPE_COMPLEX)
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
  // If they put the COMPLEX type into a struct we won't notice, but whatever.
  ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < cif->nargs; i++)
    ಆದರೆ (cif->arg_types[i]->type == FFI_TYPE_COMPLEX)
      ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
  ಹಿಂತಿರುಗಿ FFI_OK;
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, ಚಿಹ್ನೆರಹಿತ nfixedargs, ಚಿಹ್ನೆರಹಿತ ntotalargs)
{
  cif->flags |= VARARGS_FLAG;
  cif->nfixedargs = nfixedargs;
  // The varargs takes up one extra argument
  ಆದರೆ (cif->nfixedargs + 1 > MAX_ವಾದಗಳು)
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
  ಹಿಂತಿರುಗಿ FFI_OK;
}

/**
 * A Javascript helper function. This takes an argument typ which is a wasm
 * pointer to an ffi_type object. It returns a pair a type and a type id.
 *
 *    - If it is not a struct, return its type and its typeid field.
 *    - If it is a struct of size >= 2, return the type and its typeid (which
 *      will be FFI_TYPE_STRUCT)
 *    - If it is a struct of size 0, return FFI_TYPE_VOID (????? this is broken)
 *    - If it is a struct of size 1, replace it with the single field and apply
 *      the same logic again to that.
 *
 * By always unboxing structs up front, we can avoid messy casework later.
 */
EM_JS_MACROS(
ಶೂನ್ಯ,
unbox_small_structs, (ffi_type type_ptr), {
  type_ptr = DEC_PTR(type_ptr);
  var type_id = FFI_TYPE__TYPEID(type_ptr);
  ಆಗ (type_id === FFI_TYPE_STRUCT) {
    // Don't unbox single element structs if they are bigger than 16 bytes. This
    // is a work around for the fact that Python will give incorrect values for
    // the size of the field in these cases: it says that the struct has pointer
    // size and alignment and are of type pointer, even though it is more
    // accurately a struct and has a larger size. Keeping it as a struct here
    // will let us get the ABI right (which is in fact that the true argument is
    // a pointer to the stack... so maybe Python issn't so wrong??)
    //
    // See the Python comment here:
    // https://github.com/python/cpython/blob/a16a9f978f42b8a09297c1efbf33877f6388c403/Modules/_ctypes/stgdict.c#L718-L779
    ಆದರೆ (DEC_PTR(FFI_TYPE_ಗಾತ್ರ(type_ptr)) > 16) {
      ನಿಲ್ಲಿಸು;
    }
    var elements = DEC_PTR(FFI_TYPE__ELEMENTS(type_ptr));
    var ಮೊದಲ_element = DEREF_PTR_ಸಂಖ್ಯೆ(elements, 0);
    ಆದರೆ (ಮೊದಲ_element === 0) {
      type_id = FFI_TYPE_VOID;
      ನಿಲ್ಲಿಸು;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (DEREF_PTR_ಸಂಖ್ಯೆ(elements, 1) === 0) {
      type_ptr = ಮೊದಲ_element;
      type_id = FFI_TYPE__TYPEID(ಮೊದಲ_element);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ [type_ptr, type_id];
})

EM_JS_MACROS(
ಶೂನ್ಯ,
ffi_call_js, (ffi_cif *cif, ffi_fp fn, ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue),
{
  cif = DEC_PTR(cif);
  fn = DEC_PTR(fn);
  rvalue = DEC_PTR(rvalue);
  avalue = DEC_PTR(avalue);
  var abi = CIF__ABI(cif);
  var nargs = CIF__NARGS(cif);
  var nfixedargs = CIF__NFIXEDARGS(cif);
  var arg_types_ptr = DEC_PTR(CIF__ARGTYPES(cif));
  var flags = CIF__FLAGS(cif);
  var rtype_unboxed = unbox_small_structs(CIF__RTYPE(cif));
  var rtype_ptr = rtype_unboxed[0];
  var rtype_id = rtype_unboxed[1];
  // A small struct return is unboxed to a scalar type id above, but it is
  // still a struct to the caller: its buffer is exactly the struct's size,
  // so it must not get the integral widening to ffi_arg.
  var rtype_widen = FFI_TYPE__TYPEID(DEC_PTR(CIF__RTYPE(cif))) !== FFI_TYPE_STRUCT;
  var orig_ರಾಶಿ_ptr = ರಾಶಿ_Save();
  var cur_ರಾಶಿ_ptr = orig_ರಾಶಿ_ptr;

  var ವಾದಗಳು = [];
  // Does our onwards call return by argument or normally? We return by argument
  // no matter what.
  var ret_by_arg = false;

  ಆದರೆ (rtype_id === FFI_TYPE_COMPLEX) {
    throw new Error('complex ret marshalling nyi');
  }
  ಆದರೆ (rtype_id < 0 || rtype_id > FFI_TYPE_ಕೊನೆಯ) {
    throw new Error('Unexpected rtype ' + rtype_id);
  }
  // If the return type is a struct with multiple entries or a long double, the
  // function takes an extra first argument which is a pointer to return value.
  // Conveniently, we've already received a pointer to return value, so we can
  // just use this. We also mark a flag that we don't need to convert the return
  // value of the dynamic call back to C.
  ಆದರೆ (rtype_id === FFI_TYPE_LONGDOUBLE || rtype_id === FFI_TYPE_STRUCT) {
    ಆದರೆ (rvalue === 0) {
      // NULL rvalue: the caller discards the result, but the callee still
      // needs somewhere to write it. Allocate scratch space on the stack;
      // it is released by the stackRestore after the call.
      var rsize = DEC_PTR(FFI_TYPE_ಗಾತ್ರ(rtype_ptr));
      var ralign = FFI_TYPE__ALIGN(rtype_ptr);
      ರಾಶಿ_ALLOC(cur_ರಾಶಿ_ptr, rsize, ralign);
      rvalue = cur_ರಾಶಿ_ptr;
    }
    ವಾದಗಳು.ತಳ್ಳು(ENC_PTR(rvalue));
    ret_by_arg = true;
  }

  // Accumulate a Javascript list of arguments for the Javascript wrapper for
  // the wasm function. The Javascript wrapper does a type conversion from
  // Javascript to C automatically, here we manually do the inverse conversion
  // from C to Javascript.
  ಗಾಗಿ (var i = 0; i < nfixedargs; i++) {
    var arg_ptr = DEREF_PTR_ಸಂಖ್ಯೆ(avalue, i);
    var arg_unboxed = unbox_small_structs(DEREF_PTR(arg_types_ptr, i));
    var arg_type_ptr = arg_unboxed[0];
    var arg_type_id = arg_unboxed[1];

    // It's okay here to always use unsigned integers as long as the size is 32
    // or 64 bits. Smaller sizes get extended to 32 bits differently according
    // to whether they are signed or unsigned.
    ಆಯ್ಕೆ (arg_type_id) {
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ವಾದಗಳು.ತಳ್ಳು(DEREF_U32(arg_ptr, 0));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      ವಾದಗಳು.ತಳ್ಳು(DEREF_F32(arg_ptr, 0));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      ವಾದಗಳು.ತಳ್ಳು(DEREF_F64(arg_ptr, 0));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      ವಾದಗಳು.ತಳ್ಳು(DEREF_U8(arg_ptr, 0));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      ವಾದಗಳು.ತಳ್ಳು(DEREF_S8(arg_ptr, 0));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ವಾದಗಳು.ತಳ್ಳು(DEREF_U16(arg_ptr, 0));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      ವಾದಗಳು.ತಳ್ಳು(DEREF_S16(arg_ptr, 0));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      ವಾದಗಳು.ತಳ್ಳು(DEREF_U64(arg_ptr, 0));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      // long double is passed as a pair of BigInts.
      ವಾದಗಳು.ತಳ್ಳು(DEREF_U64(arg_ptr, 0));
      ವಾದಗಳು.ತಳ್ಳು(DEREF_U64(arg_ptr, 1));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      // Nontrivial structs are passed by pointer.
      // Have to copy the struct onto the stack though because C ABI says it's
      // call by value.
      var ಗಾತ್ರ_2 = DEC_PTR(FFI_TYPE_ಗಾತ್ರ(arg_type_ptr));
      var align = FFI_TYPE__ALIGN(arg_type_ptr);
      ರಾಶಿ_ALLOC(cur_ರಾಶಿ_ptr, ಗಾತ್ರ_2, align);
      HEAP8.subarray(cur_ರಾಶಿ_ptr, cur_ರಾಶಿ_ptr+ಗಾತ್ರ_2).ಗಣ(HEAP8.subarray(arg_ptr, arg_ptr + ಗಾತ್ರ_2));
      ವಾದಗಳು.ತಳ್ಳು(ENC_PTR(cur_ರಾಶಿ_ptr));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      ವಾದಗಳು.ತಳ್ಳು(DEREF_PTR(arg_ptr, 0));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      throw new Error('complex marshalling nyi');
    ಪೂರ್ವನಿಯೋಜಿತ:
      throw new Error('Unexpected type ' + arg_type_id);
    }
  }

  // Wasm functions can't directly manipulate the callstack, so varargs
  // arguments have to go on a separate stack. A varags function takes one extra
  // argument which is a pointer to where on the separate stack the args are
  // located. Because stacks are allocated backwards, we have to loop over the
  // varargs backwards.
  //
  // We don't have any way of knowing how many args were actually passed, so we
  // just always copy extra nonsense past the end. The ownwards call will know
  // not to look at it.
  ಆದರೆ (flags & VARARGS_FLAG) {
    var struct_arg_info = [];
    ಗಾಗಿ (var i = nargs - 1;  i >= nfixedargs; i--) {
      var arg_ptr = DEREF_PTR_ಸಂಖ್ಯೆ(avalue, i);
      var arg_unboxed = unbox_small_structs(DEREF_PTR(arg_types_ptr, i));
      var arg_type_ptr = arg_unboxed[0];
      var arg_type_id = arg_unboxed[1];
      ಆಯ್ಕೆ (arg_type_id) {
      ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      ಸ್ಥಿತಿ FFI_TYPE_SINT8:
        ರಾಶಿ_ALLOC(cur_ರಾಶಿ_ptr, 1, 1);
        DEREF_U8(cur_ರಾಶಿ_ptr, 0) = DEREF_U8(arg_ptr, 0);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ಸ್ಥಿತಿ FFI_TYPE_SINT16:
        ರಾಶಿ_ALLOC(cur_ರಾಶಿ_ptr, 2, 2);
        DEREF_U16(cur_ರಾಶಿ_ptr, 0) = DEREF_U16(arg_ptr, 0);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_INT:
      ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
        ರಾಶಿ_ALLOC(cur_ರಾಶಿ_ptr, 4, 4);
        DEREF_U32(cur_ರಾಶಿ_ptr, 0) = DEREF_U32(arg_ptr, 0);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      ಸ್ಥಿತಿ FFI_TYPE_SINT64:
        ರಾಶಿ_ALLOC(cur_ರಾಶಿ_ptr, 8, 8);
        DEREF_U32(cur_ರಾಶಿ_ptr, 0) = DEREF_U32(arg_ptr, 0);
        DEREF_U32(cur_ರಾಶಿ_ptr, 1) = DEREF_U32(arg_ptr, 1);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
        ರಾಶಿ_ALLOC(cur_ರಾಶಿ_ptr, 16, 8);
        DEREF_U32(cur_ರಾಶಿ_ptr, 0) = DEREF_U32(arg_ptr, 0);
        DEREF_U32(cur_ರಾಶಿ_ptr, 1) = DEREF_U32(arg_ptr, 1);
        DEREF_U32(cur_ರಾಶಿ_ptr, 2) = DEREF_U32(arg_ptr, 2);
        DEREF_U32(cur_ರಾಶಿ_ptr, 3) = DEREF_U32(arg_ptr, 3);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
        // Again, struct must be passed by pointer.
        // But ABI is by value, so have to copy struct onto stack.
        // Currently arguments are going onto stack so we can't put it there now. Come back for this.
        ರಾಶಿ_ALLOC(cur_ರಾಶಿ_ptr, __SIZEOF_POINTER__, __SIZEOF_POINTER__);
        struct_arg_info.ತಳ್ಳು([cur_ರಾಶಿ_ptr, arg_ptr, DEC_PTR(FFI_TYPE_ಗಾತ್ರ(arg_type_ptr)), FFI_TYPE__ALIGN(arg_type_ptr)]);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
        ರಾಶಿ_ALLOC(cur_ರಾಶಿ_ptr, __SIZEOF_POINTER__, __SIZEOF_POINTER__);
        DEREF_PTR(cur_ರಾಶಿ_ptr, 0) = DEREF_PTR(arg_ptr, 0);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
        throw new Error('complex arg marshalling nyi');
      ಪೂರ್ವನಿಯೋಜಿತ:
        throw new Error('Unexpected argtype ' + arg_type_id);
      }
    }
    // extra normal argument which is the pointer to the varargs.
    ವಾದಗಳು.ತಳ್ಳು(ENC_PTR(cur_ರಾಶಿ_ptr));
    // Now allocate variable struct args on stack too.
    ಗಾಗಿ (var i = 0; i < struct_arg_info.ಉದ್ದಳತೆ; i++) {
      var struct_info = struct_arg_info[i];
      var arg_ಗುರಿ = struct_info[0];
      var arg_ptr = struct_info[1];
      var ಗಾತ್ರ_2 = struct_info[2];
      var align = struct_info[3];
      ರಾಶಿ_ALLOC(cur_ರಾಶಿ_ptr, ಗಾತ್ರ_2, align);
      HEAP8.subarray(cur_ರಾಶಿ_ptr, cur_ರಾಶಿ_ptr+ಗಾತ್ರ_2).ಗಣ(HEAP8.subarray(arg_ptr, arg_ptr + ಗಾತ್ರ_2));
      DEREF_PTR(arg_ಗುರಿ, 0) = ENC_PTR(cur_ರಾಶಿ_ptr);
    }
  }
  ರಾಶಿ_Restore(cur_ರಾಶಿ_ptr);
  ರಾಶಿ_Alloc(0); // stackAlloc enforces alignment invariants on the stack pointer
  LOG_DEBUG("CALL_FUNC_PTR", "fn:", fn, "args:", ವಾದಗಳು);
  var ಫಲಿತಾಂಶ = get_Wasm_ಕೋಷ್ಟಕ_ನಮೂದು(fn).apply(null, ವಾದಗಳು);
  // Put the stack pointer back (we moved it if there were any struct args or we
  // made a varargs call)
  ರಾಶಿ_Restore(orig_ರಾಶಿ_ptr);

  // We need to return by argument. If return value was a nontrivial struct or
  // long double, the onwards call already put the return value in rvalue
  ಆದರೆ (ret_by_arg) {
    ಹಿಂತಿರುಗಿ;
  }

  // A NULL rvalue means the caller discards the return value.
  ಆದರೆ (rvalue === 0) {
    ಹಿಂತಿರುಗಿ;
  }

  // Otherwise the result was automatically converted from C into Javascript and
  // we need to manually convert it back to C.
  // Integral returns narrower than a register are widened to ffi_arg, as
  // documented and as every native port does -- but only genuine scalar
  // returns: an unboxed small-struct return keeps its natural width.
  ಆಯ್ಕೆ (rtype_id) {
  ಸ್ಥಿತಿ FFI_TYPE_VOID:
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_INT:
  ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಆದರೆ (rtype_widen)
      STORE_ARG_WIDENED_SIGNED(rvalue, ಫಲಿತಾಂಶ | 0);
    ಇಲ್ಲದಿದ್ದರೆ
      DEREF_U32(rvalue, 0) = ಫಲಿತಾಂಶ;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಆದರೆ (rtype_widen)
      STORE_ARG_WIDENED_UNSIGNED(rvalue, ಫಲಿತಾಂಶ | 0);
    ಇಲ್ಲದಿದ್ದರೆ
      DEREF_U32(rvalue, 0) = ಫಲಿತಾಂಶ;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    DEREF_F32(rvalue, 0) = ಫಲಿತಾಂಶ;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
    DEREF_F64(rvalue, 0) = ಫಲಿತಾಂಶ;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_UINT8:
    ಆದರೆ (rtype_widen)
      STORE_ARG_WIDENED_UNSIGNED(rvalue, ಫಲಿತಾಂಶ & 0xff);
    ಇಲ್ಲದಿದ್ದರೆ
      DEREF_U8(rvalue, 0) = ಫಲಿತಾಂಶ;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_SINT8:
    ಆದರೆ (rtype_widen)
      STORE_ARG_WIDENED_SIGNED(rvalue, (ಫಲಿತಾಂಶ << 24) >> 24);
    ಇಲ್ಲದಿದ್ದರೆ
      DEREF_U8(rvalue, 0) = ಫಲಿತಾಂಶ;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_UINT16:
    ಆದರೆ (rtype_widen)
      STORE_ARG_WIDENED_UNSIGNED(rvalue, ಫಲಿತಾಂಶ & 0xffff);
    ಇಲ್ಲದಿದ್ದರೆ
      DEREF_U16(rvalue, 0) = ಫಲಿತಾಂಶ;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_SINT16:
    ಆದರೆ (rtype_widen)
      STORE_ARG_WIDENED_SIGNED(rvalue, (ಫಲಿತಾಂಶ << 16) >> 16);
    ಇಲ್ಲದಿದ್ದರೆ
      DEREF_U16(rvalue, 0) = ಫಲಿತಾಂಶ;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_UINT64:
  ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    DEREF_U64(rvalue, 0) = ಫಲಿತಾಂಶ;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
    DEREF_PTR(rvalue, 0) = ಫಲಿತಾಂಶ;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
    throw new Error('complex ret marshalling nyi');
  ಪೂರ್ವನಿಯೋಜಿತ:
    throw new Error('Unexpected rtype ' + rtype_id);
  }
});

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue) {
  ffi_call_js(cif, fn, rvalue, avalue);
}

#ಆದರೆ __SIZEOF_POINTER__ == 4

CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_ಆವರಣ, ftramp, 4*0);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_ಆವರಣ, cif, 4*1);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_ಆವರಣ, fun, 4*2);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_ಆವರಣ, user_ದತ್ತಾಂಶ, 4*3);

#ವ್ಯಾಖ್ಯಾನಿಸು ಆವರಣ_wrapper(addr) DEREF_U32(addr, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಆವರಣ_cif(addr) DEREF_U32(addr, 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ಆವರಣ_fun(addr) DEREF_U32(addr, 2)
#ವ್ಯಾಖ್ಯಾನಿಸು ಆವರಣ_user_ದತ್ತಾಂಶ(addr) DEREF_U32(addr, 3)

#ಅಥವಾಆದರೆ __SIZEOF_POINTER__ == 8

CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_ಆವರಣ, ftramp, 0);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_ಆವರಣ, cif, 8);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_ಆವರಣ, fun, 16);
CHECK_ಕ್ಷೇತ್ರ_ಸ್ಥಾನಾಂತರ(ffi_ಆವರಣ, user_ದತ್ತಾಂಶ, 24);

#ವ್ಯಾಖ್ಯಾನಿಸು ಆವರಣ_wrapper(addr) DEREF_U64(addr, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಆವರಣ_cif(addr) DEREF_U64(addr, 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ಆವರಣ_fun(addr) DEREF_U64(addr, 2)
#ವ್ಯಾಖ್ಯಾನಿಸು ಆವರಣ_user_ದತ್ತಾಂಶ(addr) DEREF_U64(addr, 3)

#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ "Unknown pointer size"
#ಆದರೆ_ಅಂತ್ಯ

EM_JS_MACROS(ಶೂನ್ಯ *, ffi_ಆವರಣ_alloc_js, (size_t ಗಾತ್ರ_2, ಶೂನ್ಯ **ಸಂಕೇತ_2), {
  ಗಾತ್ರ_2 = DEC_PTR(ಗಾತ್ರ_2);
  ಸಂಕೇತ_2 = DEC_PTR(ಸಂಕೇತ_2);
  var ಆವರಣ = _malloc(ಗಾತ್ರ_2);
  var ಸೂಚ್ಯಂಕ = get_ಖಾಲಿಗಳು_ಕೋಷ್ಟಕ_ಸ್ಥಾನ();
  DEREF_PTR(ಸಂಕೇತ_2, 0) = ENC_PTR(ಸೂಚ್ಯಂಕ);
  ಆವರಣ_wrapper(ಆವರಣ) = ENC_PTR(ಸೂಚ್ಯಂಕ);
  ಹಿಂತಿರುಗಿ ENC_PTR(ಆವರಣ);
})

ಶೂನ್ಯ * __attribute__ ((visibility ("default")))
ffi_ಆವರಣ_alloc(size_t ಗಾತ್ರ_2, ಶೂನ್ಯ **ಸಂಕೇತ_2) {
  ಹಿಂತಿರುಗಿ ffi_ಆವರಣ_alloc_js(ಗಾತ್ರ_2, ಸಂಕೇತ_2);
}

EM_JS_MACROS(ಶೂನ್ಯ, ffi_ಆವರಣ_free_js, (ಶೂನ್ಯ *ಆವರಣ), {
  ಆವರಣ = DEC_PTR(ಆವರಣ);
  var ಸೂಚ್ಯಂಕ = DEC_PTR(ಆವರಣ_wrapper(ಆವರಣ));
  free_ಕೋಷ್ಟಕ_Indexes.ತಳ್ಳು(ಸೂಚ್ಯಂಕ);
  _free(ಆವರಣ);
})

ಶೂನ್ಯ __attribute__ ((visibility ("default")))
ffi_ಆವರಣ_free(ಶೂನ್ಯ *ಆವರಣ) {
  ಹಿಂತಿರುಗಿ ffi_ಆವರಣ_free_js(ಆವರಣ);
}



/**
 * A closure has to appear to its caller as an ordinary wasm function with the
 * signature of the cif. In order for it to work with JSPI, the trampoline has
 * to be implemented in WebAssembly -- otherwise attempting to suspend inside of
 * the closure will trap with "trying to suspend JS frames".
 *
 * It's a bit annoying to do things in generated WebAssembly so we put most of
 * the logic in JS prelude/epilogue.
 *
 *    call prelude(... all closure arguments)
 *    call_indirect closure->fun(cif, ret_ptr, args_ptr, user_data)
 *    call epilogue(ret_ptr, orig_stack_ptr)
 *
 * prelude() which copies the closure arguments into the appropriate places on
 * the C stack, builds the argument pointer array, and returns 7 values: The
 * closure->fun pointer and its four arguments, and the two arguments for epilogue.

 * epilogue() is a JavaScript helper that restores the stack pointer and loads
 * the return value from ret_ptr. Doing the load in JavaScript rather than in
 * the trampoline avoids needing to specify the memory size in the imports
 * section.
 *
 * The module bytes are a pure function of the signature string. We cache the
 * compiled WebAssembly.Module per signature and instantiate it once per closure
 * with that closure's prelude.
 */
EM_JS_MACROS(
ffi_ಸ್ಥಿತಿ,
ffi_prep_ಆವರಣ_loc_js,
(ffi_ಆವರಣ *ಆವರಣ, ffi_cif *cif, ಶೂನ್ಯ *fun, ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಶೂನ್ಯ *codeloc),
{
  ಆವರಣ = DEC_PTR(ಆವರಣ);
  cif = DEC_PTR(cif);
  fun = DEC_PTR(fun);
  user_ದತ್ತಾಂಶ = DEC_PTR(user_ದತ್ತಾಂಶ);
  codeloc = DEC_PTR(codeloc);
  var { sig, call_info } = ffi_cif_ಗಮ್ಯ_call_info(cif);

  LOG_DEBUG("CREATE_CLOSURE", "sig:", sig);
  try {
    var instance = new WebAssembly.Instance(
      ffi_ಆವರಣ_trampoline_module(sig),
      {
        e: {
          p: ffi_ಆವರಣ_prelude(ಆವರಣ, call_info),
          e: ffi_ಆವರಣ_epilogue(sig),
          t: wasm_ಕೋಷ್ಟಕ,
        },
      },
    );
  } catch (e) {
    LOG_DEBUG("CREATE_CLOSURE_FAILED", e);
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF_MACRO;
  }
  var wasm_trampoline = instance.exports["f"];
  ಗಣ_Wasm_ಕೋಷ್ಟಕ_ನಮೂದು(codeloc, wasm_trampoline);
  ಆವರಣ_cif(ಆವರಣ) = ENC_PTR(cif);
  ಆವರಣ_fun(ಆವರಣ) = ENC_PTR(fun);
  ಆವರಣ_user_ದತ್ತಾಂಶ(ಆವರಣ) = ENC_PTR(user_ದತ್ತಾಂಶ);
  ಹಿಂತಿರುಗಿ FFI_OK_MACRO;
}

// Walk the CIF and compute the metadata we need for ffi_closure_prelude
// and ffi_closure_trampoline_module.
function ffi_cif_ಗಮ್ಯ_call_info(cif) {
  var abi = CIF__ABI(cif);
  var nargs = CIF__NARGS(cif);
  var nfixedargs = CIF__NFIXEDARGS(cif);
  var arg_types_ptr = DEC_PTR(CIF__ARGTYPES(cif));
  var rtype_unboxed = unbox_small_structs(CIF__RTYPE(cif));
  var rtype_ptr = rtype_unboxed[0];
  var rtype_id = rtype_unboxed[1];

  // First construct the signature of the javascript trampoline we are going to create.
  // Important: this is the signature for calling us, the onward call always has sig viiii.
  var sig;
  var ret_by_arg = false;
  ಆಯ್ಕೆ (rtype_id) {
  ಸ್ಥಿತಿ FFI_TYPE_VOID:
    sig = 'v';
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
  ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
    // Return via a first pointer argument.
    sig = 'v' + PTR_SIG;
    ret_by_arg = true;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_INT:
  ಸ್ಥಿತಿ FFI_TYPE_UINT8:
  ಸ್ಥಿತಿ FFI_TYPE_SINT8:
  ಸ್ಥಿತಿ FFI_TYPE_UINT16:
  ಸ್ಥಿತಿ FFI_TYPE_SINT16:
  ಸ್ಥಿತಿ FFI_TYPE_UINT32:
  ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    sig = 'i';
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    sig = 'f';
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
    sig = 'd';
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_UINT64:
  ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    sig = 'j';
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
    sig = PTR_SIG;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
    throw new Error('complex ret marshalling nyi');
  ಪೂರ್ವನಿಯೋಜಿತ:
    throw new Error('Unexpected rtype ' + rtype_id);
  }
  var unboxed_arg_type_id_list = [];
  var unboxed_arg_type_info_list = [];
  ಗಾಗಿ (var i = 0; i < nargs; i++) {
    var arg_unboxed = unbox_small_structs(DEREF_PTR(arg_types_ptr, i));
    var arg_type_ptr = arg_unboxed[0];
    var arg_type_id = arg_unboxed[1];
    unboxed_arg_type_id_list.ತಳ್ಳು(arg_type_id);
    unboxed_arg_type_info_list.ತಳ್ಳು([DEC_PTR(FFI_TYPE_ಗಾತ್ರ(arg_type_ptr)), FFI_TYPE__ALIGN(arg_type_ptr)]);
  }
  ಗಾಗಿ (var i = 0; i < nfixedargs; i++) {
    ಆಯ್ಕೆ (unboxed_arg_type_id_list[i]) {
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      sig += 'i';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      sig += 'f';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      sig += 'd';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      sig += 'jj';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      sig += 'j';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      sig += PTR_SIG;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      throw new Error('complex marshalling nyi');
    ಪೂರ್ವನಿಯೋಜಿತ:
      throw new Error('Unexpected argtype ' + arg_type_id);
    }
  }
  ಆದರೆ (nfixedargs < nargs) {
    // extra pointer to varargs stack
    sig += PTR_SIG;
  }
  ಹಿಂತಿರುಗಿ {
    sig,
    call_info: {
      nfixedargs,
      nargs,
      ret_by_arg,
      unboxed_arg_type_info_list,
      unboxed_arg_type_id_list,
    },
  };
}

// The prelude() function is called by the wasm trampoline with the closure's
// arguments. It marshals them onto the C stack and returns what the trampoline
// needs to make the onward call.
function ffi_ಆವರಣ_prelude(
  ಆವರಣ,
  {
    nfixedargs,
    nargs,
    ret_by_arg,
    unboxed_arg_type_info_list,
    unboxed_arg_type_id_list,
  },
) {
  ಹಿಂತಿರುಗಿ function prelude() {
    var ವಾದಗಳು = arguments;
    var orig_ರಾಶಿ_ptr = ರಾಶಿ_Save();
    var cur_ptr = orig_ರಾಶಿ_ptr;
    var ret_ptr;
    var jsarg_idx = 0;
    // Should we return by argument or not? The onwards call returns by argument
    // no matter what. (Warning: ret_by_arg means the opposite in ffi_call)
    ಆದರೆ (ret_by_arg) {
      ret_ptr = DEC_PTR(ವಾದಗಳು[jsarg_idx++]);
    } ಇಲ್ಲದಿದ್ದರೆ {
      // We might return 4 bytes or 8 bytes, allocate 8 just in case.
      ರಾಶಿ_ALLOC(cur_ptr, 8, 8);
      ret_ptr = cur_ptr;
    }
    cur_ptr -= __SIZEOF_POINTER__ * nargs;
    var ವಾದಗಳು_ptr = cur_ptr;
    var carg_idx = 0;
    // Here we either have the actual argument, or a pair of BigInts for long
    // double, or a pointer to struct. We have to store into args_ptr[i] a
    // pointer to the ith argument. If the argument is a struct, just store the
    // pointer. Otherwise allocate stack space and copy the js argument onto the
    // stack.
    ಗಾಗಿ (; carg_idx < nfixedargs; carg_idx++) {
      // jsarg_idx might start out as 0 or 1 depending on ret_by_arg
      // it advances an extra time for long double
      var cur_arg = ವಾದಗಳು[jsarg_idx++];
      var arg_type_info = unboxed_arg_type_info_list[carg_idx];
      var arg_ಗಾತ್ರ = arg_type_info[0];
      var arg_align = arg_type_info[1];
      var arg_type_id = unboxed_arg_type_id_list[carg_idx];
      ಆಯ್ಕೆ (arg_type_id) {
      ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      ಸ್ಥಿತಿ FFI_TYPE_SINT8:
        // Bad things happen if we don't align to 4 here
        ರಾಶಿ_ALLOC(cur_ptr, 1, 4);
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U8(cur_ptr, 0) = cur_arg;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ಸ್ಥಿತಿ FFI_TYPE_SINT16:
        // Bad things happen if we don't align to 4 here
        ರಾಶಿ_ALLOC(cur_ptr, 2, 4);
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U16(cur_ptr, 0) = cur_arg;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_INT:
      ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ಸ್ಥಿತಿ FFI_TYPE_SINT32:
        ರಾಶಿ_ALLOC(cur_ptr, 4, 4);
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U32(cur_ptr, 0) = cur_arg;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
        // cur_arg is already a pointer to struct
        // copy it onto stack to pass by value
        ರಾಶಿ_ALLOC(cur_ptr, arg_ಗಾತ್ರ, arg_align);
        HEAP8.subarray(cur_ptr, cur_ptr + arg_ಗಾತ್ರ).ಗಣ(HEAP8.subarray(DEC_PTR(cur_arg), DEC_PTR(cur_arg) + arg_ಗಾತ್ರ));
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(cur_ptr);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
        ರಾಶಿ_ALLOC(cur_ptr, 4, 4);
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_F32(cur_ptr, 0) = cur_arg;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
        ರಾಶಿ_ALLOC(cur_ptr, 8, 8);
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_F64(cur_ptr, 0) = cur_arg;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      ಸ್ಥಿತಿ FFI_TYPE_SINT64:
        ರಾಶಿ_ALLOC(cur_ptr, 8, 8);
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U64(cur_ptr, 0) = cur_arg;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
        ರಾಶಿ_ALLOC(cur_ptr, 16, 8);
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U64(cur_ptr, 0) = cur_arg;
        cur_arg = ವಾದಗಳು[jsarg_idx++];
        DEREF_U64(cur_ptr, 1) = cur_arg;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
        ರಾಶಿ_ALLOC(cur_ptr, __SIZEOF_POINTER__, __SIZEOF_POINTER__);
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_PTR(cur_ptr, 0) = cur_arg;
        ನಿಲ್ಲಿಸು;
      }
    }
    // If its a varargs call, last js argument is a pointer to the varargs.
    var varargs = DEC_PTR(ವಾದಗಳು[ವಾದಗಳು.ಉದ್ದಳತೆ - 1]);
    // We have no way of knowing how many varargs were actually provided, this
    // fills the rest of the stack space allocated with nonsense. The onward
    // call will know to ignore the nonsense.

    // We either have a pointer to the argument if the argument is not a struct
    // or a pointer to pointer to struct. We need to store a pointer to the
    // argument into args_ptr[i]
    ಗಾಗಿ (; carg_idx < nargs; carg_idx++) {
      var arg_type_id = unboxed_arg_type_id_list[carg_idx];
      var arg_type_info = unboxed_arg_type_info_list[carg_idx];
      var arg_ಗಾತ್ರ = arg_type_info[0];
      var arg_align = arg_type_info[1];
      ಆದರೆ (arg_type_id === FFI_TYPE_STRUCT) {
        // In this case varargs is a pointer to pointer to struct so we need to
        // deref once
        var struct_ptr = DEREF_PTR_ಸಂಖ್ಯೆ(varargs, 0);
        ರಾಶಿ_ALLOC(cur_ptr, arg_ಗಾತ್ರ, arg_align);
        HEAP8.subarray(cur_ptr, cur_ptr + arg_ಗಾತ್ರ).ಗಣ(HEAP8.subarray(struct_ptr, struct_ptr + arg_ಗಾತ್ರ));
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(cur_ptr);
      } ಇಲ್ಲದಿದ್ದರೆ {
        DEREF_PTR(ವಾದಗಳು_ptr, carg_idx) = ENC_PTR(varargs);
      }
      varargs += __SIZEOF_POINTER__;
    }
    ರಾಶಿ_Restore(cur_ptr);
    ರಾಶಿ_Alloc(0); // stackAlloc enforces alignment invariants on the stack pointer
    LOG_DEBUG("CALL_CLOSURE", "closure:", ಆವರಣ, "fptr", ಆವರಣ_fun(ಆವರಣ), "cif", ಆವರಣ_cif(ಆವರಣ));
    // The trampoline now does
    //    fun(cif, ret_ptr, args_ptr, user_data);
    //    return epilogue(ret_ptr, orig_stack_ptr);
    //
    // We return these arguments in reverse order because stack is LIFO.
    ಹಿಂತಿರುಗಿ [
      ENC_PTR(ret_ptr),          // epilogue args (in correct order)
      ENC_PTR(orig_ರಾಶಿ_ptr),

      ಆವರಣ_cif(ಆವರಣ),     // call_indirect args (in correct order)
      ENC_PTR(ret_ptr),
      ENC_PTR(ವಾದಗಳು_ptr),
      ಆವರಣ_user_ದತ್ತಾಂಶ(ಆವರಣ),

      ಆವರಣ_fun(ಆವರಣ),     // ...and the table index for call_indirect
    ];
  }
}

// Returns function which restores the stack pointer and loads the closure's
// return value from ret_ptr.
function ffi_ಆವರಣ_epilogue(sig) {
  var ತುಂಬು;
  ಆಯ್ಕೆ (sig[0]) {
  ಸ್ಥಿತಿ 'i': ತುಂಬು = (ptr) => DEREF_S32(ptr, 0); ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ 'j': ತುಂಬು = (ptr) => DEREF_U64(ptr, 0); ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ 'f': ತುಂಬು = (ptr) => DEREF_F32(ptr, 0); ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ 'd': ತುಂಬು = (ptr) => DEREF_F64(ptr, 0); ನಿಲ್ಲಿಸು;
  ಪೂರ್ವನಿಯೋಜಿತ:  ತುಂಬು = (ptr) => undefined; ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ function epilogue(ret_ptr, orig_ರಾಶಿ_ptr) {
    ರಾಶಿ_Restore(DEC_PTR(orig_ರಾಶಿ_ptr));
    ಹಿಂತಿರುಗಿ ತುಂಬು(DEC_PTR(ret_ptr));
  };
}

var ffi_ಆವರಣ_trampoline_module_cache = new Map();
function ffi_ಆವರಣ_trampoline_module(sig) {
  var cache = ffi_ಆವರಣ_trampoline_module_cache;
  var module = cache.get(sig);
  ಆದರೆ (module) {
    ಹಿಂತಿರುಗಿ module;
  }

  function uleb128(n) {
    var ಫಲಿತಾಂಶ = [];
    ಮಾಡು {
      var byte = n & 0x7f;
      n >>>= 7;
      ಆದರೆ (n) {
        byte |= 0x80;
      }
      ಫಲಿತಾಂಶ.ತಳ್ಳು(byte);
    } ಆಗ (n);
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
  }
  // A vector of single-byte items
  function vec(ಬೈಟುಗಳು) {
    ಹಿಂತಿರುಗಿ [...uleb128(ಬೈಟುಗಳು.ಉದ್ದಳತೆ), ...ಬೈಟುಗಳು];
  }
  function section(id, ಬೈಟುಗಳು) {
    ಹಿಂತಿರುಗಿ [id, ...vec(ಬೈಟುಗಳು)];
  }
  function funcType(params, results) {
    ಹಿಂತಿರುಗಿ [0x60, ...vec(params), ...vec(results)];
  }

  var typeCodes = { 'i': 0x7f, 'j': 0x7e, 'f': 0x7d, 'd': 0x7c };
  var P = PTR_WASM_TYPE;
  var params = Array.from(sig.ತುಂಡು(1), (c) => typeCodes[c]);
  var results = sig[0] === 'v' ? [] : [typeCodes[sig[0]]];
  var nparams = params.ಉದ್ದಳತೆ;

  // Is the function table 64-bit indexed?
  // -sMEMORY64=2 compiles with 64-bit pointers but uses a 32-bit memory and
  // table, so we have to i32.wrap_i64 pointers.
  var idx64 = typeof wasm_ಕೋಷ್ಟಕ.ಉದ್ದಳತೆ === 'bigint';
  var idxFlag = idx64 ? 0x04 : 0x00;
  // Pointers are wrapped to i32 to be used as a table index.
  var wrapPtr = (P === 0x7e && !idx64) ? [0xa7] : []; // i32.wrap_i64

  var typeSection = section(0x01, [
    ...uleb128(4),
    ...funcType(params, results),               // type 0: the trampoline itself
    ...funcType(params, [P, P, P, P, P, P, P]), // type 1: prelude
    ...funcType([P, P, P, P], []),              // type 2: closure->fun(cif, ret, args, user_data)
    ...funcType([P, P], results),               // type 3: epilogue(ret_ptr, orig_sp)
  ]);
  var importSection = section(0x02, [
    ...uleb128(3),
    // prelude: (import "e" "p" (func (type 1)))
    0x01, 0x65, 0x01, 0x70, 0x00, 0x01,
    // epilogue: (import "e" "e" (func (type 3)))
    0x01, 0x65, 0x01, 0x65, 0x00, 0x03,
    // (import "e" "t" (table 0 funcref))
    0x01, 0x65, 0x01, 0x74, 0x01, 0x70, 0x00 | idxFlag, 0x00,
  ]);
  // One function, of type 0.
  var functionSection = section(0x03, [0x01, 0x00]);
  // (export "f" (func 2)) -- funcs 0 and 1 are the imports.
  var exportSection = section(0x07, [0x01, 0x01, 0x66, 0x00, 0x02]);

  // Function body
  var body = [0x00]; // no locals
  ಗಾಗಿ (var i = 0; i < nparams; i++) {
    body.ತಳ್ಳು(0x20, ...uleb128(i)); // local.get i
  }
  body.ತಳ್ಳು(0x10, 0x00); // call prelude
  // Operand stack is now: ret_ptr, orig_sp, cif, ret_ptr, args_ptr, user_data, fun
  body.ತಳ್ಳು(...wrapPtr); // fun is used as a table index
  body.ತಳ್ಳು(0x11, 0x02, 0x00); // call_indirect (type 2) (table 0), consumes the top 5
  // Operand stack: ret_ptr, orig_sp
  body.ತಳ್ಳು(0x10, 0x01); // call epilogue, consumes both and leaves the result (if any)
  body.ತಳ್ಳು(0x0b); //ಕೊನೆ
  var ಸಂಕೇತ_Section = section(0x0a, [...uleb128(1), ...vec(body)]);

  var ಬೈಟುಗಳು = new Uint8Array([
    0x00, 0x61, 0x73, 0x6d, // magic
    0x01, 0x00, 0x00, 0x00, //ಆವೃತ್ತಿ
    ...typeSection,
    ...importSection,
    ...functionSection,
    ...exportSection,
    ...ಸಂಕೇತ_Section,
  ]);
  LOG_DEBUG("CLOSURE_MODULE", "sig:", sig, "bytes:", ಬೈಟುಗಳು);
  // The module is tiny so compiling synchronously is fine.
  module = new WebAssembly.Module(ಬೈಟುಗಳು);
  cache.ಗಣ(sig, module);
  ಹಿಂತಿರುಗಿ module;
}
)

// EM_JS does not correctly handle function pointer arguments, so we need a
// helper
ffi_ಸ್ಥಿತಿ ffi_prep_ಆವರಣ_loc(ffi_ಆವರಣ *ಆವರಣ, ffi_cif *cif,
                                ಶೂನ್ಯ (*fun)(ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
                                ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಶೂನ್ಯ *codeloc) {
  ಆದರೆ (cif->abi != FFI_EMSCRIPTEN_ABI)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
  ಹಿಂತಿರುಗಿ ffi_prep_ಆವರಣ_loc_js(ಆವರಣ, cif, (ಶೂನ್ಯ *)fun, user_ದತ್ತಾಂಶ,
                                     codeloc);
}
