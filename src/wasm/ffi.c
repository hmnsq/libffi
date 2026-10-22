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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <stdint.h>

#ضمن <emscripten/emscripten.h>

#جے_تعریف_شدہ DEBUG_F
#تعریف_کرو LOG_DEBUG(دلیلاں...)  \
  console.warn(`====LIBFFI(سطر __LINE__)`, دلیلاں)
#وإلا
#تعریف_کرو LOG_DEBUG(دلیلاں...) 0
#جے_انت

#تعریف_کرو EM_JS_MACROS(ret, name, دلیلاں, body...) EM_JS(ret, name, دلیلاں, body)

EM_JS_DEPS(libffi, "$getWasmTableEntry,$setWasmTableEntry,$getEmptyTableSlot,$wasmTable,$stackSave,$stackAlloc,$stackRestore");

#تعریف_کرو DEREF_U8(addr, offset) HEAPU8[addr + offset]
#تعریف_کرو DEREF_S8(addr, offset) HEAP8[addr + offset]
#تعریف_کرو DEREF_U16(addr, offset) HEAPU16[(addr >> 1) + offset]
#تعریف_کرو DEREF_S16(addr, offset) HEAP16[(addr >> 1) + offset]
#تعریف_کرو DEREF_U32(addr, offset) HEAPU32[(addr >> 2) + offset]
#تعریف_کرو DEREF_S32(addr, offset) HEAP32[(addr >> 2) + offset]

#تعریف_کرو DEREF_F32(addr, offset) HEAPF32[(addr >> 2) + offset]
#تعریف_کرو DEREF_F64(addr, offset) HEAPF64[(addr >> 3) + offset]
#تعریف_کرو DEREF_U64(addr, offset) HEAPU64[(addr >> 3) + offset]

#تعریف_کرو CHECK_میدان_وتھ(بنية, میدان, offset)                                  \
  _Static_assert(                                                                  \
    offsetof(بنية, میدان) == offset,                                             \
    "Memory layout of '" #struct "' has changed: '" #field "' is in an unexpected location");

#إذا __SIZEOF_POINTER__ == 4

#تعریف_کرو FFI_EMSCRIPTEN_ABI FFI_WASM32_EMSCRIPTEN
#تعریف_کرو PTR_SIG 'i'
#تعریف_کرو PTR_WASM_TYPE 0x7f

#تعریف_کرو DEC_PTR(p) p
#تعریف_کرو ENC_PTR(p) p

#تعریف_کرو DEREF_PTR(addr, offset) DEREF_U32(addr, offset)
#تعریف_کرو DEREF_PTR_نمبر(addr, offset) DEREF_PTR(addr, offset)

// Store an integral return value widened to ffi_arg (32 bits on wasm32), as
// the libffi API promises for integral returns narrower than a register.
// The typed-array store wraps modulo 2^32, so a sign-extended negative
// Number is stored correctly by both variants.
#تعریف_کرو STORE_ARG_WIDENED_UNSIGNED(rvalue, x) (DEREF_U32(rvalue, 0) = (x))
#تعریف_کرو STORE_ARG_WIDENED_SIGNED(rvalue, x) (DEREF_U32(rvalue, 0) = (x))

CHECK_میدان_وتھ(ffi_cif, abi, 4*0);
CHECK_میدان_وتھ(ffi_cif, nargs, 4*1);
CHECK_میدان_وتھ(ffi_cif, arg_types, 4*2);
CHECK_میدان_وتھ(ffi_cif, rtype, 4*3);
CHECK_میدان_وتھ(ffi_cif, flags, 4*5);
CHECK_میدان_وتھ(ffi_cif, nfixedargs, 4*6);

#تعریف_کرو CIF__ABI(addr) DEREF_U32(addr, 0)
#تعریف_کرو CIF__NARGS(addr) DEREF_U32(addr, 1)
#تعریف_کرو CIF__ARGTYPES(addr) DEREF_U32(addr, 2)
#تعریف_کرو CIF__RTYPE(addr) DEREF_U32(addr, 3)
#تعریف_کرو CIF__FLAGS(addr) DEREF_U32(addr, 5)
#تعریف_کرو CIF__NFIXEDARGS(addr) DEREF_U32(addr, 6)

CHECK_میدان_وتھ(ffi_type, ناپ, 0);
CHECK_میدان_وتھ(ffi_type, alignment, 4);
CHECK_میدان_وتھ(ffi_type, type, 6);
CHECK_میدان_وتھ(ffi_type, elements, 8);

#تعریف_کرو FFI_TYPE_ناپ(addr) DEREF_U32(addr, 0)
#تعریف_کرو FFI_TYPE__ALIGN(addr) DEREF_U16(addr + 4, 0)
#تعریف_کرو FFI_TYPE__TYPEID(addr) DEREF_U16(addr + 6, 0)
#تعریف_کرو FFI_TYPE__ELEMENTS(addr) DEREF_U32(addr + 8, 0)

#وإلاإذا __SIZEOF_POINTER__ == 8

#تعریف_کرو FFI_EMSCRIPTEN_ABI FFI_WASM64_EMSCRIPTEN
#تعریف_کرو PTR_SIG 'j'
#تعریف_کرو PTR_WASM_TYPE 0x7e

// DEC_PTR casts a pointer value (coming from Wasm) represented as BigInt (i64) to Number (i53).
// This should be used for a pointer that is expected to be within the i53 range. If the pointer
// value is outside the Number's range, the value will become NaN.
#تعریف_کرو DEC_PTR(p) bigint_منزل_I53_Checked(p)
// ENC_PTR casts a pointer value represented as Number to BigInt (i64)
#تعریف_کرو ENC_PTR(p) BigInt(p)

#تعریف_کرو DEREF_PTR(addr, offset) DEREF_U64(addr, offset)
#تعریف_کرو DEREF_PTR_نمبر(addr, offset) DEC_PTR(DEREF_PTR(addr, offset))

// Store an integral return value widened to ffi_arg (64 bits on wasm64), as
// the libffi API promises for integral returns narrower than a register.
// BigInt masking treats the value as infinite-precision two's complement, so
// the unsigned variant zero-extends a possibly-negative i32 Number, while
// the BigUint64Array store wraps the signed variant modulo 2^64, which is
// sign extension.
#تعریف_کرو STORE_ARG_WIDENED_UNSIGNED(rvalue, x) \
  (DEREF_U64(rvalue, 0) = BigInt(x) & BigInt(4294967295))
#تعریف_کرو STORE_ARG_WIDENED_SIGNED(rvalue, x) (DEREF_U64(rvalue, 0) = BigInt(x))

CHECK_میدان_وتھ(ffi_cif, abi, 0);
CHECK_میدان_وتھ(ffi_cif, nargs, 4);
CHECK_میدان_وتھ(ffi_cif, arg_types, 8);
CHECK_میدان_وتھ(ffi_cif, rtype, 16);
CHECK_میدان_وتھ(ffi_cif, flags, 28);
CHECK_میدان_وتھ(ffi_cif, nfixedargs, 32);

#تعریف_کرو CIF__ABI(addr) DEREF_U32(addr, 0)
#تعریف_کرو CIF__NARGS(addr) DEREF_U32(addr + 4,  0)
#تعریف_کرو CIF__ARGTYPES(addr) DEREF_U64(addr + 8,  0)
#تعریف_کرو CIF__RTYPE(addr) DEREF_U64(addr + 16, 0)
#تعریف_کرو CIF__FLAGS(addr) DEREF_U32(addr + 28, 0)
#تعریف_کرو CIF__NFIXEDARGS(addr) DEREF_U32(addr + 32, 0)

CHECK_میدان_وتھ(ffi_type, ناپ, 0);
CHECK_میدان_وتھ(ffi_type, alignment, 8);
CHECK_میدان_وتھ(ffi_type, type, 10);
CHECK_میدان_وتھ(ffi_type, elements, 16);

#تعریف_کرو FFI_TYPE_ناپ(addr) DEREF_U64(addr, 0)
#تعریف_کرو FFI_TYPE__ALIGN(addr) DEREF_U16(addr + 8, 0)
#تعریف_کرو FFI_TYPE__TYPEID(addr) DEREF_U16(addr + 10, 0)
#تعریف_کرو FFI_TYPE__ELEMENTS(addr) DEREF_U64(addr + 16, 0)

#وإلا
#غلطی "Unknown pointer size"
#جے_انت

#تعریف_کرو ALIGN_پتہ(addr, align) (addr &= (~((align) - 1)))
#تعریف_کرو ڈھیر_ALLOC(ڈھیر, ناپ, align) ((ڈھیر -= (ناپ)), ALIGN_پتہ(ڈھیر, align))

// Most wasm runtimes support at most 1000 Js trampoline args.
#تعریف_کرو MAX_دلیلاں 1000

#ضمن <stddef.h>

#تعریف_کرو VARARGS_FLAG 1

#تعریف_کرو FFI_OK_MACRO 0
_Static_assert(FFI_OK_MACRO == FFI_OK, "FFI_OK must be 0");

#تعریف_کرو FFI_BAD_TYPEDEF_MACRO 1
_Static_assert(FFI_BAD_TYPEDEF_MACRO == FFI_BAD_TYPEDEF, "FFI_BAD_TYPEDEF must be 1");

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  إذا (cif->abi != FFI_EMSCRIPTEN_ABI)
    أرجع FFI_BAD_ABI;
  // This is called after ffi_prep_cif_machdep_var so we need to avoid
  // overwriting cif->nfixedargs.
  إذا (!(cif->flags & VARARGS_FLAG))
    cif->nfixedargs = cif->nargs;
  إذا (cif->nargs > MAX_دلیلاں)
    أرجع FFI_BAD_TYPEDEF;
  إذا (cif->rtype->type == FFI_TYPE_COMPLEX)
    أرجع FFI_BAD_TYPEDEF;
  // If they put the COMPLEX type into a struct we won't notice, but whatever.
  لكل (صحيح i = 0; i < cif->nargs; i++)
    إذا (cif->arg_types[i]->type == FFI_TYPE_COMPLEX)
      أرجع FFI_BAD_TYPEDEF;
  أرجع FFI_OK;
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, بنانشانی nfixedargs, بنانشانی ntotalargs)
{
  cif->flags |= VARARGS_FLAG;
  cif->nfixedargs = nfixedargs;
  // The varargs takes up one extra argument
  إذا (cif->nfixedargs + 1 > MAX_دلیلاں)
    أرجع FFI_BAD_TYPEDEF;
  أرجع FFI_OK;
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
فراغ,
unbox_small_structs, (ffi_type type_ptr), {
  type_ptr = DEC_PTR(type_ptr);
  var type_id = FFI_TYPE__TYPEID(type_ptr);
  طالما (type_id === FFI_TYPE_STRUCT) {
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
    إذا (DEC_PTR(FFI_TYPE_ناپ(type_ptr)) > 16) {
      اكسر;
    }
    var elements = DEC_PTR(FFI_TYPE__ELEMENTS(type_ptr));
    var پہلا_element = DEREF_PTR_نمبر(elements, 0);
    إذا (پہلا_element === 0) {
      type_id = FFI_TYPE_VOID;
      اكسر;
    } وإلا إذا (DEREF_PTR_نمبر(elements, 1) === 0) {
      type_ptr = پہلا_element;
      type_id = FFI_TYPE__TYPEID(پہلا_element);
    } وإلا {
      اكسر;
    }
  }
  أرجع [type_ptr, type_id];
})

EM_JS_MACROS(
فراغ,
ffi_call_js, (ffi_cif *cif, ffi_fp fn, فراغ *rvalue, فراغ **avalue),
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
  var orig_ڈھیر_ptr = ڈھیر_Save();
  var cur_ڈھیر_ptr = orig_ڈھیر_ptr;

  var دلیلاں = [];
  // Does our onwards call return by argument or normally? We return by argument
  // no matter what.
  var ret_by_arg = false;

  إذا (rtype_id === FFI_TYPE_COMPLEX) {
    throw new Error('complex ret marshalling nyi');
  }
  إذا (rtype_id < 0 || rtype_id > FFI_TYPE_آخری) {
    throw new Error('Unexpected rtype ' + rtype_id);
  }
  // If the return type is a struct with multiple entries or a long double, the
  // function takes an extra first argument which is a pointer to return value.
  // Conveniently, we've already received a pointer to return value, so we can
  // just use this. We also mark a flag that we don't need to convert the return
  // value of the dynamic call back to C.
  إذا (rtype_id === FFI_TYPE_LONGDOUBLE || rtype_id === FFI_TYPE_STRUCT) {
    إذا (rvalue === 0) {
      // NULL rvalue: the caller discards the result, but the callee still
      // needs somewhere to write it. Allocate scratch space on the stack;
      // it is released by the stackRestore after the call.
      var rsize = DEC_PTR(FFI_TYPE_ناپ(rtype_ptr));
      var ralign = FFI_TYPE__ALIGN(rtype_ptr);
      ڈھیر_ALLOC(cur_ڈھیر_ptr, rsize, ralign);
      rvalue = cur_ڈھیر_ptr;
    }
    دلیلاں.ڈالو(ENC_PTR(rvalue));
    ret_by_arg = true;
  }

  // Accumulate a Javascript list of arguments for the Javascript wrapper for
  // the wasm function. The Javascript wrapper does a type conversion from
  // Javascript to C automatically, here we manually do the inverse conversion
  // from C to Javascript.
  لكل (var i = 0; i < nfixedargs; i++) {
    var arg_ptr = DEREF_PTR_نمبر(avalue, i);
    var arg_unboxed = unbox_small_structs(DEREF_PTR(arg_types_ptr, i));
    var arg_type_ptr = arg_unboxed[0];
    var arg_type_id = arg_unboxed[1];

    // It's okay here to always use unsigned integers as long as the size is 32
    // or 64 bits. Smaller sizes get extended to 32 bits differently according
    // to whether they are signed or unsigned.
    بدل (arg_type_id) {
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
      دلیلاں.ڈالو(DEREF_U32(arg_ptr, 0));
      اكسر;
    حالة FFI_TYPE_FLOAT:
      دلیلاں.ڈالو(DEREF_F32(arg_ptr, 0));
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      دلیلاں.ڈالو(DEREF_F64(arg_ptr, 0));
      اكسر;
    حالة FFI_TYPE_UINT8:
      دلیلاں.ڈالو(DEREF_U8(arg_ptr, 0));
      اكسر;
    حالة FFI_TYPE_SINT8:
      دلیلاں.ڈالو(DEREF_S8(arg_ptr, 0));
      اكسر;
    حالة FFI_TYPE_UINT16:
      دلیلاں.ڈالو(DEREF_U16(arg_ptr, 0));
      اكسر;
    حالة FFI_TYPE_SINT16:
      دلیلاں.ڈالو(DEREF_S16(arg_ptr, 0));
      اكسر;
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      دلیلاں.ڈالو(DEREF_U64(arg_ptr, 0));
      اكسر;
    حالة FFI_TYPE_LONGDOUBLE:
      // long double is passed as a pair of BigInts.
      دلیلاں.ڈالو(DEREF_U64(arg_ptr, 0));
      دلیلاں.ڈالو(DEREF_U64(arg_ptr, 1));
      اكسر;
    حالة FFI_TYPE_STRUCT:
      // Nontrivial structs are passed by pointer.
      // Have to copy the struct onto the stack though because C ABI says it's
      // call by value.
      var ناپ = DEC_PTR(FFI_TYPE_ناپ(arg_type_ptr));
      var align = FFI_TYPE__ALIGN(arg_type_ptr);
      ڈھیر_ALLOC(cur_ڈھیر_ptr, ناپ, align);
      HEAP8.subarray(cur_ڈھیر_ptr, cur_ڈھیر_ptr+ناپ).سیٹ(HEAP8.subarray(arg_ptr, arg_ptr + ناپ));
      دلیلاں.ڈالو(ENC_PTR(cur_ڈھیر_ptr));
      اكسر;
    حالة FFI_TYPE_اشارہ_گر:
      دلیلاں.ڈالو(DEREF_PTR(arg_ptr, 0));
      اكسر;
    حالة FFI_TYPE_COMPLEX:
      throw new Error('complex marshalling nyi');
    افتراضي:
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
  إذا (flags & VARARGS_FLAG) {
    var struct_arg_info = [];
    لكل (var i = nargs - 1;  i >= nfixedargs; i--) {
      var arg_ptr = DEREF_PTR_نمبر(avalue, i);
      var arg_unboxed = unbox_small_structs(DEREF_PTR(arg_types_ptr, i));
      var arg_type_ptr = arg_unboxed[0];
      var arg_type_id = arg_unboxed[1];
      بدل (arg_type_id) {
      حالة FFI_TYPE_UINT8:
      حالة FFI_TYPE_SINT8:
        ڈھیر_ALLOC(cur_ڈھیر_ptr, 1, 1);
        DEREF_U8(cur_ڈھیر_ptr, 0) = DEREF_U8(arg_ptr, 0);
        اكسر;
      حالة FFI_TYPE_UINT16:
      حالة FFI_TYPE_SINT16:
        ڈھیر_ALLOC(cur_ڈھیر_ptr, 2, 2);
        DEREF_U16(cur_ڈھیر_ptr, 0) = DEREF_U16(arg_ptr, 0);
        اكسر;
      حالة FFI_TYPE_INT:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_FLOAT:
        ڈھیر_ALLOC(cur_ڈھیر_ptr, 4, 4);
        DEREF_U32(cur_ڈھیر_ptr, 0) = DEREF_U32(arg_ptr, 0);
        اكسر;
      حالة FFI_TYPE_DOUBLE:
      حالة FFI_TYPE_UINT64:
      حالة FFI_TYPE_SINT64:
        ڈھیر_ALLOC(cur_ڈھیر_ptr, 8, 8);
        DEREF_U32(cur_ڈھیر_ptr, 0) = DEREF_U32(arg_ptr, 0);
        DEREF_U32(cur_ڈھیر_ptr, 1) = DEREF_U32(arg_ptr, 1);
        اكسر;
      حالة FFI_TYPE_LONGDOUBLE:
        ڈھیر_ALLOC(cur_ڈھیر_ptr, 16, 8);
        DEREF_U32(cur_ڈھیر_ptr, 0) = DEREF_U32(arg_ptr, 0);
        DEREF_U32(cur_ڈھیر_ptr, 1) = DEREF_U32(arg_ptr, 1);
        DEREF_U32(cur_ڈھیر_ptr, 2) = DEREF_U32(arg_ptr, 2);
        DEREF_U32(cur_ڈھیر_ptr, 3) = DEREF_U32(arg_ptr, 3);
        اكسر;
      حالة FFI_TYPE_STRUCT:
        // Again, struct must be passed by pointer.
        // But ABI is by value, so have to copy struct onto stack.
        // Currently arguments are going onto stack so we can't put it there now. Come back for this.
        ڈھیر_ALLOC(cur_ڈھیر_ptr, __SIZEOF_POINTER__, __SIZEOF_POINTER__);
        struct_arg_info.ڈالو([cur_ڈھیر_ptr, arg_ptr, DEC_PTR(FFI_TYPE_ناپ(arg_type_ptr)), FFI_TYPE__ALIGN(arg_type_ptr)]);
        اكسر;
      حالة FFI_TYPE_اشارہ_گر:
        ڈھیر_ALLOC(cur_ڈھیر_ptr, __SIZEOF_POINTER__, __SIZEOF_POINTER__);
        DEREF_PTR(cur_ڈھیر_ptr, 0) = DEREF_PTR(arg_ptr, 0);
        اكسر;
      حالة FFI_TYPE_COMPLEX:
        throw new Error('complex arg marshalling nyi');
      افتراضي:
        throw new Error('Unexpected argtype ' + arg_type_id);
      }
    }
    // extra normal argument which is the pointer to the varargs.
    دلیلاں.ڈالو(ENC_PTR(cur_ڈھیر_ptr));
    // Now allocate variable struct args on stack too.
    لكل (var i = 0; i < struct_arg_info.لمائی; i++) {
      var struct_info = struct_arg_info[i];
      var arg_نشانہ = struct_info[0];
      var arg_ptr = struct_info[1];
      var ناپ = struct_info[2];
      var align = struct_info[3];
      ڈھیر_ALLOC(cur_ڈھیر_ptr, ناپ, align);
      HEAP8.subarray(cur_ڈھیر_ptr, cur_ڈھیر_ptr+ناپ).سیٹ(HEAP8.subarray(arg_ptr, arg_ptr + ناپ));
      DEREF_PTR(arg_نشانہ, 0) = ENC_PTR(cur_ڈھیر_ptr);
    }
  }
  ڈھیر_Restore(cur_ڈھیر_ptr);
  ڈھیر_Alloc(0); // stackAlloc enforces alignment invariants on the stack pointer
  LOG_DEBUG("CALL_FUNC_PTR", "fn:", fn, "args:", دلیلاں);
  var نتیجہ = get_Wasm_جدول_اندراج(fn).apply(null, دلیلاں);
  // Put the stack pointer back (we moved it if there were any struct args or we
  // made a varargs call)
  ڈھیر_Restore(orig_ڈھیر_ptr);

  // We need to return by argument. If return value was a nontrivial struct or
  // long double, the onwards call already put the return value in rvalue
  إذا (ret_by_arg) {
    أرجع;
  }

  // A NULL rvalue means the caller discards the return value.
  إذا (rvalue === 0) {
    أرجع;
  }

  // Otherwise the result was automatically converted from C into Javascript and
  // we need to manually convert it back to C.
  // Integral returns narrower than a register are widened to ffi_arg, as
  // documented and as every native port does -- but only genuine scalar
  // returns: an unboxed small-struct return keeps its natural width.
  بدل (rtype_id) {
  حالة FFI_TYPE_VOID:
    اكسر;
  حالة FFI_TYPE_INT:
  حالة FFI_TYPE_SINT32:
    إذا (rtype_widen)
      STORE_ARG_WIDENED_SIGNED(rvalue, نتیجہ | 0);
    وإلا
      DEREF_U32(rvalue, 0) = نتیجہ;
    اكسر;
  حالة FFI_TYPE_UINT32:
    إذا (rtype_widen)
      STORE_ARG_WIDENED_UNSIGNED(rvalue, نتیجہ | 0);
    وإلا
      DEREF_U32(rvalue, 0) = نتیجہ;
    اكسر;
  حالة FFI_TYPE_FLOAT:
    DEREF_F32(rvalue, 0) = نتیجہ;
    اكسر;
  حالة FFI_TYPE_DOUBLE:
    DEREF_F64(rvalue, 0) = نتیجہ;
    اكسر;
  حالة FFI_TYPE_UINT8:
    إذا (rtype_widen)
      STORE_ARG_WIDENED_UNSIGNED(rvalue, نتیجہ & 0xff);
    وإلا
      DEREF_U8(rvalue, 0) = نتیجہ;
    اكسر;
  حالة FFI_TYPE_SINT8:
    إذا (rtype_widen)
      STORE_ARG_WIDENED_SIGNED(rvalue, (نتیجہ << 24) >> 24);
    وإلا
      DEREF_U8(rvalue, 0) = نتیجہ;
    اكسر;
  حالة FFI_TYPE_UINT16:
    إذا (rtype_widen)
      STORE_ARG_WIDENED_UNSIGNED(rvalue, نتیجہ & 0xffff);
    وإلا
      DEREF_U16(rvalue, 0) = نتیجہ;
    اكسر;
  حالة FFI_TYPE_SINT16:
    إذا (rtype_widen)
      STORE_ARG_WIDENED_SIGNED(rvalue, (نتیجہ << 16) >> 16);
    وإلا
      DEREF_U16(rvalue, 0) = نتیجہ;
    اكسر;
  حالة FFI_TYPE_UINT64:
  حالة FFI_TYPE_SINT64:
    DEREF_U64(rvalue, 0) = نتیجہ;
    اكسر;
  حالة FFI_TYPE_اشارہ_گر:
    DEREF_PTR(rvalue, 0) = نتیجہ;
    اكسر;
  حالة FFI_TYPE_COMPLEX:
    throw new Error('complex ret marshalling nyi');
  افتراضي:
    throw new Error('Unexpected rtype ' + rtype_id);
  }
});

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue) {
  ffi_call_js(cif, fn, rvalue, avalue);
}

#إذا __SIZEOF_POINTER__ == 4

CHECK_میدان_وتھ(ffi_گھیرا, ftramp, 4*0);
CHECK_میدان_وتھ(ffi_گھیرا, cif, 4*1);
CHECK_میدان_وتھ(ffi_گھیرا, fun, 4*2);
CHECK_میدان_وتھ(ffi_گھیرا, user_انکڑے, 4*3);

#تعریف_کرو گھیرا_wrapper(addr) DEREF_U32(addr, 0)
#تعریف_کرو گھیرا_cif(addr) DEREF_U32(addr, 1)
#تعریف_کرو گھیرا_fun(addr) DEREF_U32(addr, 2)
#تعریف_کرو گھیرا_user_انکڑے(addr) DEREF_U32(addr, 3)

#وإلاإذا __SIZEOF_POINTER__ == 8

CHECK_میدان_وتھ(ffi_گھیرا, ftramp, 0);
CHECK_میدان_وتھ(ffi_گھیرا, cif, 8);
CHECK_میدان_وتھ(ffi_گھیرا, fun, 16);
CHECK_میدان_وتھ(ffi_گھیرا, user_انکڑے, 24);

#تعریف_کرو گھیرا_wrapper(addr) DEREF_U64(addr, 0)
#تعریف_کرو گھیرا_cif(addr) DEREF_U64(addr, 1)
#تعریف_کرو گھیرا_fun(addr) DEREF_U64(addr, 2)
#تعریف_کرو گھیرا_user_انکڑے(addr) DEREF_U64(addr, 3)

#وإلا
#غلطی "Unknown pointer size"
#جے_انت

EM_JS_MACROS(فراغ *, ffi_گھیرا_alloc_js, (size_t ناپ, فراغ **کوڈ), {
  ناپ = DEC_PTR(ناپ);
  کوڈ = DEC_PTR(کوڈ);
  var گھیرا = _malloc(ناپ);
  var اشاریہ = get_خالی_تھاواں_جدول_تھاں();
  DEREF_PTR(کوڈ, 0) = ENC_PTR(اشاریہ);
  گھیرا_wrapper(گھیرا) = ENC_PTR(اشاریہ);
  أرجع ENC_PTR(گھیرا);
})

فراغ * __attribute__ ((visibility ("default")))
ffi_گھیرا_alloc(size_t ناپ, فراغ **کوڈ) {
  أرجع ffi_گھیرا_alloc_js(ناپ, کوڈ);
}

EM_JS_MACROS(فراغ, ffi_گھیرا_free_js, (فراغ *گھیرا), {
  گھیرا = DEC_PTR(گھیرا);
  var اشاریہ = DEC_PTR(گھیرا_wrapper(گھیرا));
  free_جدول_Indexes.ڈالو(اشاریہ);
  _free(گھیرا);
})

فراغ __attribute__ ((visibility ("default")))
ffi_گھیرا_free(فراغ *گھیرا) {
  أرجع ffi_گھیرا_free_js(گھیرا);
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
ffi_حالت,
ffi_prep_گھیرا_loc_js,
(ffi_گھیرا *گھیرا, ffi_cif *cif, فراغ *fun, فراغ *user_انکڑے, فراغ *codeloc),
{
  گھیرا = DEC_PTR(گھیرا);
  cif = DEC_PTR(cif);
  fun = DEC_PTR(fun);
  user_انکڑے = DEC_PTR(user_انکڑے);
  codeloc = DEC_PTR(codeloc);
  var { sig, call_info } = ffi_cif_منزل_call_info(cif);

  LOG_DEBUG("CREATE_CLOSURE", "sig:", sig);
  try {
    var instance = new WebAssembly.Instance(
      ffi_گھیرا_trampoline_module(sig),
      {
        e: {
          p: ffi_گھیرا_prelude(گھیرا, call_info),
          e: ffi_گھیرا_epilogue(sig),
          t: wasm_جدول,
        },
      },
    );
  } catch (e) {
    LOG_DEBUG("CREATE_CLOSURE_FAILED", e);
    أرجع FFI_BAD_TYPEDEF_MACRO;
  }
  var wasm_trampoline = instance.exports["f"];
  سیٹ_Wasm_جدول_اندراج(codeloc, wasm_trampoline);
  گھیرا_cif(گھیرا) = ENC_PTR(cif);
  گھیرا_fun(گھیرا) = ENC_PTR(fun);
  گھیرا_user_انکڑے(گھیرا) = ENC_PTR(user_انکڑے);
  أرجع FFI_OK_MACRO;
}

// Walk the CIF and compute the metadata we need for ffi_closure_prelude
// and ffi_closure_trampoline_module.
function ffi_cif_منزل_call_info(cif) {
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
  بدل (rtype_id) {
  حالة FFI_TYPE_VOID:
    sig = 'v';
    اكسر;
  حالة FFI_TYPE_STRUCT:
  حالة FFI_TYPE_LONGDOUBLE:
    // Return via a first pointer argument.
    sig = 'v' + PTR_SIG;
    ret_by_arg = true;
    اكسر;
  حالة FFI_TYPE_INT:
  حالة FFI_TYPE_UINT8:
  حالة FFI_TYPE_SINT8:
  حالة FFI_TYPE_UINT16:
  حالة FFI_TYPE_SINT16:
  حالة FFI_TYPE_UINT32:
  حالة FFI_TYPE_SINT32:
    sig = 'i';
    اكسر;
  حالة FFI_TYPE_FLOAT:
    sig = 'f';
    اكسر;
  حالة FFI_TYPE_DOUBLE:
    sig = 'd';
    اكسر;
  حالة FFI_TYPE_UINT64:
  حالة FFI_TYPE_SINT64:
    sig = 'j';
    اكسر;
  حالة FFI_TYPE_اشارہ_گر:
    sig = PTR_SIG;
    اكسر;
  حالة FFI_TYPE_COMPLEX:
    throw new Error('complex ret marshalling nyi');
  افتراضي:
    throw new Error('Unexpected rtype ' + rtype_id);
  }
  var unboxed_arg_type_id_list = [];
  var unboxed_arg_type_info_list = [];
  لكل (var i = 0; i < nargs; i++) {
    var arg_unboxed = unbox_small_structs(DEREF_PTR(arg_types_ptr, i));
    var arg_type_ptr = arg_unboxed[0];
    var arg_type_id = arg_unboxed[1];
    unboxed_arg_type_id_list.ڈالو(arg_type_id);
    unboxed_arg_type_info_list.ڈالو([DEC_PTR(FFI_TYPE_ناپ(arg_type_ptr)), FFI_TYPE__ALIGN(arg_type_ptr)]);
  }
  لكل (var i = 0; i < nfixedargs; i++) {
    بدل (unboxed_arg_type_id_list[i]) {
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_UINT8:
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT16:
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_SINT32:
      sig += 'i';
      اكسر;
    حالة FFI_TYPE_FLOAT:
      sig += 'f';
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      sig += 'd';
      اكسر;
    حالة FFI_TYPE_LONGDOUBLE:
      sig += 'jj';
      اكسر;
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      sig += 'j';
      اكسر;
    حالة FFI_TYPE_STRUCT:
    حالة FFI_TYPE_اشارہ_گر:
      sig += PTR_SIG;
      اكسر;
    حالة FFI_TYPE_COMPLEX:
      throw new Error('complex marshalling nyi');
    افتراضي:
      throw new Error('Unexpected argtype ' + arg_type_id);
    }
  }
  إذا (nfixedargs < nargs) {
    // extra pointer to varargs stack
    sig += PTR_SIG;
  }
  أرجع {
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
function ffi_گھیرا_prelude(
  گھیرا,
  {
    nfixedargs,
    nargs,
    ret_by_arg,
    unboxed_arg_type_info_list,
    unboxed_arg_type_id_list,
  },
) {
  أرجع function prelude() {
    var دلیلاں = arguments;
    var orig_ڈھیر_ptr = ڈھیر_Save();
    var cur_ptr = orig_ڈھیر_ptr;
    var ret_ptr;
    var jsarg_idx = 0;
    // Should we return by argument or not? The onwards call returns by argument
    // no matter what. (Warning: ret_by_arg means the opposite in ffi_call)
    إذا (ret_by_arg) {
      ret_ptr = DEC_PTR(دلیلاں[jsarg_idx++]);
    } وإلا {
      // We might return 4 bytes or 8 bytes, allocate 8 just in case.
      ڈھیر_ALLOC(cur_ptr, 8, 8);
      ret_ptr = cur_ptr;
    }
    cur_ptr -= __SIZEOF_POINTER__ * nargs;
    var دلیلاں_ptr = cur_ptr;
    var carg_idx = 0;
    // Here we either have the actual argument, or a pair of BigInts for long
    // double, or a pointer to struct. We have to store into args_ptr[i] a
    // pointer to the ith argument. If the argument is a struct, just store the
    // pointer. Otherwise allocate stack space and copy the js argument onto the
    // stack.
    لكل (; carg_idx < nfixedargs; carg_idx++) {
      // jsarg_idx might start out as 0 or 1 depending on ret_by_arg
      // it advances an extra time for long double
      var cur_arg = دلیلاں[jsarg_idx++];
      var arg_type_info = unboxed_arg_type_info_list[carg_idx];
      var arg_ناپ = arg_type_info[0];
      var arg_align = arg_type_info[1];
      var arg_type_id = unboxed_arg_type_id_list[carg_idx];
      بدل (arg_type_id) {
      حالة FFI_TYPE_UINT8:
      حالة FFI_TYPE_SINT8:
        // Bad things happen if we don't align to 4 here
        ڈھیر_ALLOC(cur_ptr, 1, 4);
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U8(cur_ptr, 0) = cur_arg;
        اكسر;
      حالة FFI_TYPE_UINT16:
      حالة FFI_TYPE_SINT16:
        // Bad things happen if we don't align to 4 here
        ڈھیر_ALLOC(cur_ptr, 2, 4);
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U16(cur_ptr, 0) = cur_arg;
        اكسر;
      حالة FFI_TYPE_INT:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_SINT32:
        ڈھیر_ALLOC(cur_ptr, 4, 4);
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U32(cur_ptr, 0) = cur_arg;
        اكسر;
      حالة FFI_TYPE_STRUCT:
        // cur_arg is already a pointer to struct
        // copy it onto stack to pass by value
        ڈھیر_ALLOC(cur_ptr, arg_ناپ, arg_align);
        HEAP8.subarray(cur_ptr, cur_ptr + arg_ناپ).سیٹ(HEAP8.subarray(DEC_PTR(cur_arg), DEC_PTR(cur_arg) + arg_ناپ));
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(cur_ptr);
        اكسر;
      حالة FFI_TYPE_FLOAT:
        ڈھیر_ALLOC(cur_ptr, 4, 4);
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_F32(cur_ptr, 0) = cur_arg;
        اكسر;
      حالة FFI_TYPE_DOUBLE:
        ڈھیر_ALLOC(cur_ptr, 8, 8);
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_F64(cur_ptr, 0) = cur_arg;
        اكسر;
      حالة FFI_TYPE_UINT64:
      حالة FFI_TYPE_SINT64:
        ڈھیر_ALLOC(cur_ptr, 8, 8);
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U64(cur_ptr, 0) = cur_arg;
        اكسر;
      حالة FFI_TYPE_LONGDOUBLE:
        ڈھیر_ALLOC(cur_ptr, 16, 8);
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U64(cur_ptr, 0) = cur_arg;
        cur_arg = دلیلاں[jsarg_idx++];
        DEREF_U64(cur_ptr, 1) = cur_arg;
        اكسر;
      حالة FFI_TYPE_اشارہ_گر:
        ڈھیر_ALLOC(cur_ptr, __SIZEOF_POINTER__, __SIZEOF_POINTER__);
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_PTR(cur_ptr, 0) = cur_arg;
        اكسر;
      }
    }
    // If its a varargs call, last js argument is a pointer to the varargs.
    var varargs = DEC_PTR(دلیلاں[دلیلاں.لمائی - 1]);
    // We have no way of knowing how many varargs were actually provided, this
    // fills the rest of the stack space allocated with nonsense. The onward
    // call will know to ignore the nonsense.

    // We either have a pointer to the argument if the argument is not a struct
    // or a pointer to pointer to struct. We need to store a pointer to the
    // argument into args_ptr[i]
    لكل (; carg_idx < nargs; carg_idx++) {
      var arg_type_id = unboxed_arg_type_id_list[carg_idx];
      var arg_type_info = unboxed_arg_type_info_list[carg_idx];
      var arg_ناپ = arg_type_info[0];
      var arg_align = arg_type_info[1];
      إذا (arg_type_id === FFI_TYPE_STRUCT) {
        // In this case varargs is a pointer to pointer to struct so we need to
        // deref once
        var struct_ptr = DEREF_PTR_نمبر(varargs, 0);
        ڈھیر_ALLOC(cur_ptr, arg_ناپ, arg_align);
        HEAP8.subarray(cur_ptr, cur_ptr + arg_ناپ).سیٹ(HEAP8.subarray(struct_ptr, struct_ptr + arg_ناپ));
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(cur_ptr);
      } وإلا {
        DEREF_PTR(دلیلاں_ptr, carg_idx) = ENC_PTR(varargs);
      }
      varargs += __SIZEOF_POINTER__;
    }
    ڈھیر_Restore(cur_ptr);
    ڈھیر_Alloc(0); // stackAlloc enforces alignment invariants on the stack pointer
    LOG_DEBUG("CALL_CLOSURE", "closure:", گھیرا, "fptr", گھیرا_fun(گھیرا), "cif", گھیرا_cif(گھیرا));
    // The trampoline now does
    //    fun(cif, ret_ptr, args_ptr, user_data);
    //    return epilogue(ret_ptr, orig_stack_ptr);
    //
    // We return these arguments in reverse order because stack is LIFO.
    أرجع [
      ENC_PTR(ret_ptr),          // epilogue args (in correct order)
      ENC_PTR(orig_ڈھیر_ptr),

      گھیرا_cif(گھیرا),     // call_indirect args (in correct order)
      ENC_PTR(ret_ptr),
      ENC_PTR(دلیلاں_ptr),
      گھیرا_user_انکڑے(گھیرا),

      گھیرا_fun(گھیرا),     // ...and the table index for call_indirect
    ];
  }
}

// Returns function which restores the stack pointer and loads the closure's
// return value from ret_ptr.
function ffi_گھیرا_epilogue(sig) {
  var لدو;
  بدل (sig[0]) {
  حالة 'i': لدو = (ptr) => DEREF_S32(ptr, 0); اكسر;
  حالة 'j': لدو = (ptr) => DEREF_U64(ptr, 0); اكسر;
  حالة 'f': لدو = (ptr) => DEREF_F32(ptr, 0); اكسر;
  حالة 'd': لدو = (ptr) => DEREF_F64(ptr, 0); اكسر;
  افتراضي:  لدو = (ptr) => undefined; اكسر;
  }
  أرجع function epilogue(ret_ptr, orig_ڈھیر_ptr) {
    ڈھیر_Restore(DEC_PTR(orig_ڈھیر_ptr));
    أرجع لدو(DEC_PTR(ret_ptr));
  };
}

var ffi_گھیرا_trampoline_module_cache = new Map();
function ffi_گھیرا_trampoline_module(sig) {
  var cache = ffi_گھیرا_trampoline_module_cache;
  var module = cache.get(sig);
  إذا (module) {
    أرجع module;
  }

  function uleb128(n) {
    var نتیجہ = [];
    افعل {
      var byte = n & 0x7f;
      n >>>= 7;
      إذا (n) {
        byte |= 0x80;
      }
      نتیجہ.ڈالو(byte);
    } طالما (n);
    أرجع نتیجہ;
  }
  // A vector of single-byte items
  function vec(بائٹاں) {
    أرجع [...uleb128(بائٹاں.لمائی), ...بائٹاں];
  }
  function section(id, بائٹاں) {
    أرجع [id, ...vec(بائٹاں)];
  }
  function funcType(params, results) {
    أرجع [0x60, ...vec(params), ...vec(results)];
  }

  var typeCodes = { 'i': 0x7f, 'j': 0x7e, 'f': 0x7d, 'd': 0x7c };
  var P = PTR_WASM_TYPE;
  var params = Array.from(sig.ٹکڑا(1), (c) => typeCodes[c]);
  var results = sig[0] === 'v' ? [] : [typeCodes[sig[0]]];
  var nparams = params.لمائی;

  // Is the function table 64-bit indexed?
  // -sMEMORY64=2 compiles with 64-bit pointers but uses a 32-bit memory and
  // table, so we have to i32.wrap_i64 pointers.
  var idx64 = typeof wasm_جدول.لمائی === 'bigint';
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
  لكل (var i = 0; i < nparams; i++) {
    body.ڈالو(0x20, ...uleb128(i)); // local.get i
  }
  body.ڈالو(0x10, 0x00); // call prelude
  // Operand stack is now: ret_ptr, orig_sp, cif, ret_ptr, args_ptr, user_data, fun
  body.ڈالو(...wrapPtr); // fun is used as a table index
  body.ڈالو(0x11, 0x02, 0x00); // call_indirect (type 2) (table 0), consumes the top 5
  // Operand stack: ret_ptr, orig_sp
  body.ڈالو(0x10, 0x01); // call epilogue, consumes both and leaves the result (if any)
  body.ڈالو(0x0b); //سرا
  var کوڈ_Section = section(0x0a, [...uleb128(1), ...vec(body)]);

  var بائٹاں = new Uint8Array([
    0x00, 0x61, 0x73, 0x6d, // magic
    0x01, 0x00, 0x00, 0x00, //نسخہ
    ...typeSection,
    ...importSection,
    ...functionSection,
    ...exportSection,
    ...کوڈ_Section,
  ]);
  LOG_DEBUG("CLOSURE_MODULE", "sig:", sig, "bytes:", بائٹاں);
  // The module is tiny so compiling synchronously is fine.
  module = new WebAssembly.Module(بائٹاں);
  cache.سیٹ(sig, module);
  أرجع module;
}
)

// EM_JS does not correctly handle function pointer arguments, so we need a
// helper
ffi_حالت ffi_prep_گھیرا_loc(ffi_گھیرا *گھیرا, ffi_cif *cif,
                                فراغ (*fun)(ffi_cif *, فراغ *, فراغ **, فراغ *),
                                فراغ *user_انکڑے, فراغ *codeloc) {
  إذا (cif->abi != FFI_EMSCRIPTEN_ABI)
    أرجع FFI_BAD_ABI;
  أرجع ffi_prep_گھیرا_loc_js(گھیرا, cif, (فراغ *)fun, user_انکڑے,
                                     codeloc);
}
