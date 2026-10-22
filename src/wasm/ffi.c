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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdint.h>

#অন্তর্ভাব <emscripten/emscripten.h>

#ifdef DEBUG_F
#define LOG_DEBUG(args...)  \
  console.warn(`====LIBFFI(সারি __LINE__)`, args)
#অন্যথা
#define LOG_DEBUG(args...) 0
#endif

#define EM_JS_MACROS(ret, name, args, body...) EM_JS(ret, name, args, body)

EM_JS_DEPS(libffi, "$getWasmTableEntry,$setWasmTableEntry,$getEmptyTableSlot,$wasmTable,$stackSave,$stackAlloc,$stackRestore");

#define DEREF_U8(addr, offset) HEAPU8[addr + offset]
#define DEREF_S8(addr, offset) HEAP8[addr + offset]
#define DEREF_U16(addr, offset) HEAPU16[(addr >> 1) + offset]
#define DEREF_S16(addr, offset) HEAP16[(addr >> 1) + offset]
#define DEREF_U32(addr, offset) HEAPU32[(addr >> 2) + offset]
#define DEREF_S32(addr, offset) HEAP32[(addr >> 2) + offset]

#define DEREF_F32(addr, offset) HEAPF32[(addr >> 2) + offset]
#define DEREF_F64(addr, offset) HEAPF64[(addr >> 3) + offset]
#define DEREF_U64(addr, offset) HEAPU64[(addr >> 3) + offset]

#define CHECK_FIELD_OFFSET(সংরচনা, field, offset)                                  \
  _Static_assert(                                                                  \
    offsetof(সংরচনা, field) == offset,                                             \
    "Memory layout of '" #struct "' has changed: '" #field "' is in an unexpected location");

#যদি __SIZEOF_POINTER__ == 4

#define FFI_EMSCRIPTEN_ABI FFI_WASM32_EMSCRIPTEN
#define PTR_SIG 'i'
#define PTR_WASM_TYPE 0x7f

#define DEC_PTR(p) p
#define ENC_PTR(p) p

#define DEREF_PTR(addr, offset) DEREF_U32(addr, offset)
#define DEREF_PTR_সংখ্যা(addr, offset) DEREF_PTR(addr, offset)

// Store an integral return value widened to ffi_arg (32 bits on wasm32), as
// the libffi API promises for integral returns narrower than a register.
// The typed-array store wraps modulo 2^32, so a sign-extended negative
// Number is stored correctly by both variants.
#define STORE_ARG_WIDENED_UNSIGNED(rvalue, x) (DEREF_U32(rvalue, 0) = (x))
#define STORE_ARG_WIDENED_SIGNED(rvalue, x) (DEREF_U32(rvalue, 0) = (x))

CHECK_FIELD_OFFSET(ffi_cif, abi, 4*0);
CHECK_FIELD_OFFSET(ffi_cif, nargs, 4*1);
CHECK_FIELD_OFFSET(ffi_cif, arg_types, 4*2);
CHECK_FIELD_OFFSET(ffi_cif, rtype, 4*3);
CHECK_FIELD_OFFSET(ffi_cif, flags, 4*5);
CHECK_FIELD_OFFSET(ffi_cif, nfixedargs, 4*6);

#define CIF__ABI(addr) DEREF_U32(addr, 0)
#define CIF__NARGS(addr) DEREF_U32(addr, 1)
#define CIF__ARGTYPES(addr) DEREF_U32(addr, 2)
#define CIF__RTYPE(addr) DEREF_U32(addr, 3)
#define CIF__FLAGS(addr) DEREF_U32(addr, 5)
#define CIF__NFIXEDARGS(addr) DEREF_U32(addr, 6)

CHECK_FIELD_OFFSET(ffi_type, size, 0);
CHECK_FIELD_OFFSET(ffi_type, alignment, 4);
CHECK_FIELD_OFFSET(ffi_type, type, 6);
CHECK_FIELD_OFFSET(ffi_type, elements, 8);

#define FFI_TYPE__SIZE(addr) DEREF_U32(addr, 0)
#define FFI_TYPE__ALIGN(addr) DEREF_U16(addr + 4, 0)
#define FFI_TYPE__TYPEID(addr) DEREF_U16(addr + 6, 0)
#define FFI_TYPE__ELEMENTS(addr) DEREF_U32(addr + 8, 0)

#নোচেদ্যদি __SIZEOF_POINTER__ == 8

#define FFI_EMSCRIPTEN_ABI FFI_WASM64_EMSCRIPTEN
#define PTR_SIG 'j'
#define PTR_WASM_TYPE 0x7e

// DEC_PTR casts a pointer value (coming from Wasm) represented as BigInt (i64) to Number (i53).
// This should be used for a pointer that is expected to be within the i53 range. If the pointer
// value is outside the Number's range, the value will become NaN.
#define DEC_PTR(p) bigintToI53Checked(p)
// ENC_PTR casts a pointer value represented as Number to BigInt (i64)
#define ENC_PTR(p) BigInt(p)

#define DEREF_PTR(addr, offset) DEREF_U64(addr, offset)
#define DEREF_PTR_সংখ্যা(addr, offset) DEC_PTR(DEREF_PTR(addr, offset))

// Store an integral return value widened to ffi_arg (64 bits on wasm64), as
// the libffi API promises for integral returns narrower than a register.
// BigInt masking treats the value as infinite-precision two's complement, so
// the unsigned variant zero-extends a possibly-negative i32 Number, while
// the BigUint64Array store wraps the signed variant modulo 2^64, which is
// sign extension.
#define STORE_ARG_WIDENED_UNSIGNED(rvalue, x) \
  (DEREF_U64(rvalue, 0) = BigInt(x) & BigInt(4294967295))
#define STORE_ARG_WIDENED_SIGNED(rvalue, x) (DEREF_U64(rvalue, 0) = BigInt(x))

CHECK_FIELD_OFFSET(ffi_cif, abi, 0);
CHECK_FIELD_OFFSET(ffi_cif, nargs, 4);
CHECK_FIELD_OFFSET(ffi_cif, arg_types, 8);
CHECK_FIELD_OFFSET(ffi_cif, rtype, 16);
CHECK_FIELD_OFFSET(ffi_cif, flags, 28);
CHECK_FIELD_OFFSET(ffi_cif, nfixedargs, 32);

#define CIF__ABI(addr) DEREF_U32(addr, 0)
#define CIF__NARGS(addr) DEREF_U32(addr + 4,  0)
#define CIF__ARGTYPES(addr) DEREF_U64(addr + 8,  0)
#define CIF__RTYPE(addr) DEREF_U64(addr + 16, 0)
#define CIF__FLAGS(addr) DEREF_U32(addr + 28, 0)
#define CIF__NFIXEDARGS(addr) DEREF_U32(addr + 32, 0)

CHECK_FIELD_OFFSET(ffi_type, size, 0);
CHECK_FIELD_OFFSET(ffi_type, alignment, 8);
CHECK_FIELD_OFFSET(ffi_type, type, 10);
CHECK_FIELD_OFFSET(ffi_type, elements, 16);

#define FFI_TYPE__SIZE(addr) DEREF_U64(addr, 0)
#define FFI_TYPE__ALIGN(addr) DEREF_U16(addr + 8, 0)
#define FFI_TYPE__TYPEID(addr) DEREF_U16(addr + 10, 0)
#define FFI_TYPE__ELEMENTS(addr) DEREF_U64(addr + 16, 0)

#অন্যথা
#error "Unknown pointer size"
#endif

#define ALIGN_ADDRESS(addr, align) (addr &= (~((align) - 1)))
#define স্তূপ_ALLOC(স্তূপ, size, align) ((স্তূপ -= (size)), ALIGN_ADDRESS(স্তূপ, align))

// Most wasm runtimes support at most 1000 Js trampoline args.
#define MAX_ARGS 1000

#অন্তর্ভাব <stddef.h>

#define VARARGS_FLAG 1

#define FFI_OK_MACRO 0
_Static_assert(FFI_OK_MACRO == FFI_OK, "FFI_OK must be 0");

#define FFI_BAD_TYPEDEF_MACRO 1
_Static_assert(FFI_BAD_TYPEDEF_MACRO == FFI_BAD_TYPEDEF, "FFI_BAD_TYPEDEF must be 1");

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  যদি (cif->abi != FFI_EMSCRIPTEN_ABI)
    প্রত্যাগম FFI_BAD_ABI;
  // This is called after ffi_prep_cif_machdep_var so we need to avoid
  // overwriting cif->nfixedargs.
  যদি (!(cif->flags & VARARGS_FLAG))
    cif->nfixedargs = cif->nargs;
  যদি (cif->nargs > MAX_ARGS)
    প্রত্যাগম FFI_BAD_TYPEDEF;
  যদি (cif->rtype->type == FFI_TYPE_COMPLEX)
    প্রত্যাগম FFI_BAD_TYPEDEF;
  // If they put the COMPLEX type into a struct we won't notice, but whatever.
  প্রতি (পূর্ণাঙ্ক i = 0; i < cif->nargs; i++)
    যদি (cif->arg_types[i]->type == FFI_TYPE_COMPLEX)
      প্রত্যাগম FFI_BAD_TYPEDEF;
  প্রত্যাগম FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned nfixedargs, unsigned ntotalargs)
{
  cif->flags |= VARARGS_FLAG;
  cif->nfixedargs = nfixedargs;
  // The varargs takes up one extra argument
  যদি (cif->nfixedargs + 1 > MAX_ARGS)
    প্রত্যাগম FFI_BAD_TYPEDEF;
  প্রত্যাগম FFI_OK;
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
শূন্য,
unbox_small_structs, (ffi_type type_ptr), {
  type_ptr = DEC_PTR(type_ptr);
  var type_id = FFI_TYPE__TYPEID(type_ptr);
  যাবত্ (type_id === FFI_TYPE_STRUCT) {
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
    যদি (DEC_PTR(FFI_TYPE__SIZE(type_ptr)) > 16) {
      ভঙ্গ;
    }
    var elements = DEC_PTR(FFI_TYPE__ELEMENTS(type_ptr));
    var first_element = DEREF_PTR_সংখ্যা(elements, 0);
    যদি (first_element === 0) {
      type_id = FFI_TYPE_VOID;
      ভঙ্গ;
    } অন্যথা যদি (DEREF_PTR_সংখ্যা(elements, 1) === 0) {
      type_ptr = first_element;
      type_id = FFI_TYPE__TYPEID(first_element);
    } অন্যথা {
      ভঙ্গ;
    }
  }
  প্রত্যাগম [type_ptr, type_id];
})

EM_JS_MACROS(
শূন্য,
ffi_call_js, (ffi_cif *cif, ffi_fp fn, শূন্য *rvalue, শূন্য **avalue),
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
  var orig_স্তূপ_ptr = স্তূপ_Save();
  var cur_স্তূপ_ptr = orig_স্তূপ_ptr;

  var args = [];
  // Does our onwards call return by argument or normally? We return by argument
  // no matter what.
  var ret_by_arg = false;

  যদি (rtype_id === FFI_TYPE_COMPLEX) {
    throw new Error('complex ret marshalling nyi');
  }
  যদি (rtype_id < 0 || rtype_id > FFI_TYPE_LAST) {
    throw new Error('Unexpected rtype ' + rtype_id);
  }
  // If the return type is a struct with multiple entries or a long double, the
  // function takes an extra first argument which is a pointer to return value.
  // Conveniently, we've already received a pointer to return value, so we can
  // just use this. We also mark a flag that we don't need to convert the return
  // value of the dynamic call back to C.
  যদি (rtype_id === FFI_TYPE_LONGDOUBLE || rtype_id === FFI_TYPE_STRUCT) {
    যদি (rvalue === 0) {
      // NULL rvalue: the caller discards the result, but the callee still
      // needs somewhere to write it. Allocate scratch space on the stack;
      // it is released by the stackRestore after the call.
      var rsize = DEC_PTR(FFI_TYPE__SIZE(rtype_ptr));
      var ralign = FFI_TYPE__ALIGN(rtype_ptr);
      স্তূপ_ALLOC(cur_স্তূপ_ptr, rsize, ralign);
      rvalue = cur_স্তূপ_ptr;
    }
    args.ঠেলো(ENC_PTR(rvalue));
    ret_by_arg = true;
  }

  // Accumulate a Javascript list of arguments for the Javascript wrapper for
  // the wasm function. The Javascript wrapper does a type conversion from
  // Javascript to C automatically, here we manually do the inverse conversion
  // from C to Javascript.
  প্রতি (var i = 0; i < nfixedargs; i++) {
    var arg_ptr = DEREF_PTR_সংখ্যা(avalue, i);
    var arg_unboxed = unbox_small_structs(DEREF_PTR(arg_types_ptr, i));
    var arg_type_ptr = arg_unboxed[0];
    var arg_type_id = arg_unboxed[1];

    // It's okay here to always use unsigned integers as long as the size is 32
    // or 64 bits. Smaller sizes get extended to 32 bits differently according
    // to whether they are signed or unsigned.
    বিকল্প (arg_type_id) {
    পক্ষ FFI_TYPE_INT:
    পক্ষ FFI_TYPE_SINT32:
    পক্ষ FFI_TYPE_UINT32:
      args.ঠেলো(DEREF_U32(arg_ptr, 0));
      ভঙ্গ;
    পক্ষ FFI_TYPE_FLOAT:
      args.ঠেলো(DEREF_F32(arg_ptr, 0));
      ভঙ্গ;
    পক্ষ FFI_TYPE_DOUBLE:
      args.ঠেলো(DEREF_F64(arg_ptr, 0));
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT8:
      args.ঠেলো(DEREF_U8(arg_ptr, 0));
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT8:
      args.ঠেলো(DEREF_S8(arg_ptr, 0));
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT16:
      args.ঠেলো(DEREF_U16(arg_ptr, 0));
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT16:
      args.ঠেলো(DEREF_S16(arg_ptr, 0));
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_SINT64:
      args.ঠেলো(DEREF_U64(arg_ptr, 0));
      ভঙ্গ;
    পক্ষ FFI_TYPE_LONGDOUBLE:
      // long double is passed as a pair of BigInts.
      args.ঠেলো(DEREF_U64(arg_ptr, 0));
      args.ঠেলো(DEREF_U64(arg_ptr, 1));
      ভঙ্গ;
    পক্ষ FFI_TYPE_STRUCT:
      // Nontrivial structs are passed by pointer.
      // Have to copy the struct onto the stack though because C ABI says it's
      // call by value.
      var size = DEC_PTR(FFI_TYPE__SIZE(arg_type_ptr));
      var align = FFI_TYPE__ALIGN(arg_type_ptr);
      স্তূপ_ALLOC(cur_স্তূপ_ptr, size, align);
      HEAP8.subarray(cur_স্তূপ_ptr, cur_স্তূপ_ptr+size).set(HEAP8.subarray(arg_ptr, arg_ptr + size));
      args.ঠেলো(ENC_PTR(cur_স্তূপ_ptr));
      ভঙ্গ;
    পক্ষ FFI_TYPE_POINTER:
      args.ঠেলো(DEREF_PTR(arg_ptr, 0));
      ভঙ্গ;
    পক্ষ FFI_TYPE_COMPLEX:
      throw new Error('complex marshalling nyi');
    স্বভাব:
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
  যদি (flags & VARARGS_FLAG) {
    var struct_arg_info = [];
    প্রতি (var i = nargs - 1;  i >= nfixedargs; i--) {
      var arg_ptr = DEREF_PTR_সংখ্যা(avalue, i);
      var arg_unboxed = unbox_small_structs(DEREF_PTR(arg_types_ptr, i));
      var arg_type_ptr = arg_unboxed[0];
      var arg_type_id = arg_unboxed[1];
      বিকল্প (arg_type_id) {
      পক্ষ FFI_TYPE_UINT8:
      পক্ষ FFI_TYPE_SINT8:
        স্তূপ_ALLOC(cur_স্তূপ_ptr, 1, 1);
        DEREF_U8(cur_স্তূপ_ptr, 0) = DEREF_U8(arg_ptr, 0);
        ভঙ্গ;
      পক্ষ FFI_TYPE_UINT16:
      পক্ষ FFI_TYPE_SINT16:
        স্তূপ_ALLOC(cur_স্তূপ_ptr, 2, 2);
        DEREF_U16(cur_স্তূপ_ptr, 0) = DEREF_U16(arg_ptr, 0);
        ভঙ্গ;
      পক্ষ FFI_TYPE_INT:
      পক্ষ FFI_TYPE_UINT32:
      পক্ষ FFI_TYPE_SINT32:
      পক্ষ FFI_TYPE_FLOAT:
        স্তূপ_ALLOC(cur_স্তূপ_ptr, 4, 4);
        DEREF_U32(cur_স্তূপ_ptr, 0) = DEREF_U32(arg_ptr, 0);
        ভঙ্গ;
      পক্ষ FFI_TYPE_DOUBLE:
      পক্ষ FFI_TYPE_UINT64:
      পক্ষ FFI_TYPE_SINT64:
        স্তূপ_ALLOC(cur_স্তূপ_ptr, 8, 8);
        DEREF_U32(cur_স্তূপ_ptr, 0) = DEREF_U32(arg_ptr, 0);
        DEREF_U32(cur_স্তূপ_ptr, 1) = DEREF_U32(arg_ptr, 1);
        ভঙ্গ;
      পক্ষ FFI_TYPE_LONGDOUBLE:
        স্তূপ_ALLOC(cur_স্তূপ_ptr, 16, 8);
        DEREF_U32(cur_স্তূপ_ptr, 0) = DEREF_U32(arg_ptr, 0);
        DEREF_U32(cur_স্তূপ_ptr, 1) = DEREF_U32(arg_ptr, 1);
        DEREF_U32(cur_স্তূপ_ptr, 2) = DEREF_U32(arg_ptr, 2);
        DEREF_U32(cur_স্তূপ_ptr, 3) = DEREF_U32(arg_ptr, 3);
        ভঙ্গ;
      পক্ষ FFI_TYPE_STRUCT:
        // Again, struct must be passed by pointer.
        // But ABI is by value, so have to copy struct onto stack.
        // Currently arguments are going onto stack so we can't put it there now. Come back for this.
        স্তূপ_ALLOC(cur_স্তূপ_ptr, __SIZEOF_POINTER__, __SIZEOF_POINTER__);
        struct_arg_info.ঠেলো([cur_স্তূপ_ptr, arg_ptr, DEC_PTR(FFI_TYPE__SIZE(arg_type_ptr)), FFI_TYPE__ALIGN(arg_type_ptr)]);
        ভঙ্গ;
      পক্ষ FFI_TYPE_POINTER:
        স্তূপ_ALLOC(cur_স্তূপ_ptr, __SIZEOF_POINTER__, __SIZEOF_POINTER__);
        DEREF_PTR(cur_স্তূপ_ptr, 0) = DEREF_PTR(arg_ptr, 0);
        ভঙ্গ;
      পক্ষ FFI_TYPE_COMPLEX:
        throw new Error('complex arg marshalling nyi');
      স্বভাব:
        throw new Error('Unexpected argtype ' + arg_type_id);
      }
    }
    // extra normal argument which is the pointer to the varargs.
    args.ঠেলো(ENC_PTR(cur_স্তূপ_ptr));
    // Now allocate variable struct args on stack too.
    প্রতি (var i = 0; i < struct_arg_info.length; i++) {
      var struct_info = struct_arg_info[i];
      var arg_target = struct_info[0];
      var arg_ptr = struct_info[1];
      var size = struct_info[2];
      var align = struct_info[3];
      স্তূপ_ALLOC(cur_স্তূপ_ptr, size, align);
      HEAP8.subarray(cur_স্তূপ_ptr, cur_স্তূপ_ptr+size).set(HEAP8.subarray(arg_ptr, arg_ptr + size));
      DEREF_PTR(arg_target, 0) = ENC_PTR(cur_স্তূপ_ptr);
    }
  }
  স্তূপ_Restore(cur_স্তূপ_ptr);
  স্তূপ_Alloc(0); // stackAlloc enforces alignment invariants on the stack pointer
  LOG_DEBUG("CALL_FUNC_PTR", "fn:", fn, "args:", args);
  var result = getWasmTableEntry(fn).apply(null, args);
  // Put the stack pointer back (we moved it if there were any struct args or we
  // made a varargs call)
  স্তূপ_Restore(orig_স্তূপ_ptr);

  // We need to return by argument. If return value was a nontrivial struct or
  // long double, the onwards call already put the return value in rvalue
  যদি (ret_by_arg) {
    প্রত্যাগম;
  }

  // A NULL rvalue means the caller discards the return value.
  যদি (rvalue === 0) {
    প্রত্যাগম;
  }

  // Otherwise the result was automatically converted from C into Javascript and
  // we need to manually convert it back to C.
  // Integral returns narrower than a register are widened to ffi_arg, as
  // documented and as every native port does -- but only genuine scalar
  // returns: an unboxed small-struct return keeps its natural width.
  বিকল্প (rtype_id) {
  পক্ষ FFI_TYPE_VOID:
    ভঙ্গ;
  পক্ষ FFI_TYPE_INT:
  পক্ষ FFI_TYPE_SINT32:
    যদি (rtype_widen)
      STORE_ARG_WIDENED_SIGNED(rvalue, result | 0);
    অন্যথা
      DEREF_U32(rvalue, 0) = result;
    ভঙ্গ;
  পক্ষ FFI_TYPE_UINT32:
    যদি (rtype_widen)
      STORE_ARG_WIDENED_UNSIGNED(rvalue, result | 0);
    অন্যথা
      DEREF_U32(rvalue, 0) = result;
    ভঙ্গ;
  পক্ষ FFI_TYPE_FLOAT:
    DEREF_F32(rvalue, 0) = result;
    ভঙ্গ;
  পক্ষ FFI_TYPE_DOUBLE:
    DEREF_F64(rvalue, 0) = result;
    ভঙ্গ;
  পক্ষ FFI_TYPE_UINT8:
    যদি (rtype_widen)
      STORE_ARG_WIDENED_UNSIGNED(rvalue, result & 0xff);
    অন্যথা
      DEREF_U8(rvalue, 0) = result;
    ভঙ্গ;
  পক্ষ FFI_TYPE_SINT8:
    যদি (rtype_widen)
      STORE_ARG_WIDENED_SIGNED(rvalue, (result << 24) >> 24);
    অন্যথা
      DEREF_U8(rvalue, 0) = result;
    ভঙ্গ;
  পক্ষ FFI_TYPE_UINT16:
    যদি (rtype_widen)
      STORE_ARG_WIDENED_UNSIGNED(rvalue, result & 0xffff);
    অন্যথা
      DEREF_U16(rvalue, 0) = result;
    ভঙ্গ;
  পক্ষ FFI_TYPE_SINT16:
    যদি (rtype_widen)
      STORE_ARG_WIDENED_SIGNED(rvalue, (result << 16) >> 16);
    অন্যথা
      DEREF_U16(rvalue, 0) = result;
    ভঙ্গ;
  পক্ষ FFI_TYPE_UINT64:
  পক্ষ FFI_TYPE_SINT64:
    DEREF_U64(rvalue, 0) = result;
    ভঙ্গ;
  পক্ষ FFI_TYPE_POINTER:
    DEREF_PTR(rvalue, 0) = result;
    ভঙ্গ;
  পক্ষ FFI_TYPE_COMPLEX:
    throw new Error('complex ret marshalling nyi');
  স্বভাব:
    throw new Error('Unexpected rtype ' + rtype_id);
  }
});

শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue) {
  ffi_call_js(cif, fn, rvalue, avalue);
}

#যদি __SIZEOF_POINTER__ == 4

CHECK_FIELD_OFFSET(ffi_closure, ftramp, 4*0);
CHECK_FIELD_OFFSET(ffi_closure, cif, 4*1);
CHECK_FIELD_OFFSET(ffi_closure, fun, 4*2);
CHECK_FIELD_OFFSET(ffi_closure, user_data, 4*3);

#define CLOSURE__wrapper(addr) DEREF_U32(addr, 0)
#define CLOSURE__cif(addr) DEREF_U32(addr, 1)
#define CLOSURE__fun(addr) DEREF_U32(addr, 2)
#define CLOSURE__user_data(addr) DEREF_U32(addr, 3)

#নোচেদ্যদি __SIZEOF_POINTER__ == 8

CHECK_FIELD_OFFSET(ffi_closure, ftramp, 0);
CHECK_FIELD_OFFSET(ffi_closure, cif, 8);
CHECK_FIELD_OFFSET(ffi_closure, fun, 16);
CHECK_FIELD_OFFSET(ffi_closure, user_data, 24);

#define CLOSURE__wrapper(addr) DEREF_U64(addr, 0)
#define CLOSURE__cif(addr) DEREF_U64(addr, 1)
#define CLOSURE__fun(addr) DEREF_U64(addr, 2)
#define CLOSURE__user_data(addr) DEREF_U64(addr, 3)

#অন্যথা
#error "Unknown pointer size"
#endif

EM_JS_MACROS(শূন্য *, ffi_closure_alloc_js, (size_t size, শূন্য **code), {
  size = DEC_PTR(size);
  code = DEC_PTR(code);
  var closure = _malloc(size);
  var index = getEmptyTableSlot();
  DEREF_PTR(code, 0) = ENC_PTR(index);
  CLOSURE__wrapper(closure) = ENC_PTR(index);
  প্রত্যাগম ENC_PTR(closure);
})

শূন্য * __attribute__ ((visibility ("default")))
ffi_closure_alloc(size_t size, শূন্য **code) {
  প্রত্যাগম ffi_closure_alloc_js(size, code);
}

EM_JS_MACROS(শূন্য, ffi_closure_free_js, (শূন্য *closure), {
  closure = DEC_PTR(closure);
  var index = DEC_PTR(CLOSURE__wrapper(closure));
  freeTableIndexes.ঠেলো(index);
  _free(closure);
})

শূন্য __attribute__ ((visibility ("default")))
ffi_closure_free(শূন্য *closure) {
  প্রত্যাগম ffi_closure_free_js(closure);
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
ffi_status,
ffi_prep_closure_loc_js,
(ffi_closure *closure, ffi_cif *cif, শূন্য *fun, শূন্য *user_data, শূন্য *codeloc),
{
  closure = DEC_PTR(closure);
  cif = DEC_PTR(cif);
  fun = DEC_PTR(fun);
  user_data = DEC_PTR(user_data);
  codeloc = DEC_PTR(codeloc);
  var { sig, call_info } = ffi_cif_to_call_info(cif);

  LOG_DEBUG("CREATE_CLOSURE", "sig:", sig);
  try {
    var instance = new WebAssembly.Instance(
      ffi_closure_trampoline_module(sig),
      {
        e: {
          p: ffi_closure_prelude(closure, call_info),
          e: ffi_closure_epilogue(sig),
          t: wasmTable,
        },
      },
    );
  } catch (e) {
    LOG_DEBUG("CREATE_CLOSURE_FAILED", e);
    প্রত্যাগম FFI_BAD_TYPEDEF_MACRO;
  }
  var wasm_trampoline = instance.exports["f"];
  setWasmTableEntry(codeloc, wasm_trampoline);
  CLOSURE__cif(closure) = ENC_PTR(cif);
  CLOSURE__fun(closure) = ENC_PTR(fun);
  CLOSURE__user_data(closure) = ENC_PTR(user_data);
  প্রত্যাগম FFI_OK_MACRO;
}

// Walk the CIF and compute the metadata we need for ffi_closure_prelude
// and ffi_closure_trampoline_module.
function ffi_cif_to_call_info(cif) {
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
  বিকল্প (rtype_id) {
  পক্ষ FFI_TYPE_VOID:
    sig = 'v';
    ভঙ্গ;
  পক্ষ FFI_TYPE_STRUCT:
  পক্ষ FFI_TYPE_LONGDOUBLE:
    // Return via a first pointer argument.
    sig = 'v' + PTR_SIG;
    ret_by_arg = true;
    ভঙ্গ;
  পক্ষ FFI_TYPE_INT:
  পক্ষ FFI_TYPE_UINT8:
  পক্ষ FFI_TYPE_SINT8:
  পক্ষ FFI_TYPE_UINT16:
  পক্ষ FFI_TYPE_SINT16:
  পক্ষ FFI_TYPE_UINT32:
  পক্ষ FFI_TYPE_SINT32:
    sig = 'i';
    ভঙ্গ;
  পক্ষ FFI_TYPE_FLOAT:
    sig = 'f';
    ভঙ্গ;
  পক্ষ FFI_TYPE_DOUBLE:
    sig = 'd';
    ভঙ্গ;
  পক্ষ FFI_TYPE_UINT64:
  পক্ষ FFI_TYPE_SINT64:
    sig = 'j';
    ভঙ্গ;
  পক্ষ FFI_TYPE_POINTER:
    sig = PTR_SIG;
    ভঙ্গ;
  পক্ষ FFI_TYPE_COMPLEX:
    throw new Error('complex ret marshalling nyi');
  স্বভাব:
    throw new Error('Unexpected rtype ' + rtype_id);
  }
  var unboxed_arg_type_id_list = [];
  var unboxed_arg_type_info_list = [];
  প্রতি (var i = 0; i < nargs; i++) {
    var arg_unboxed = unbox_small_structs(DEREF_PTR(arg_types_ptr, i));
    var arg_type_ptr = arg_unboxed[0];
    var arg_type_id = arg_unboxed[1];
    unboxed_arg_type_id_list.ঠেলো(arg_type_id);
    unboxed_arg_type_info_list.ঠেলো([DEC_PTR(FFI_TYPE__SIZE(arg_type_ptr)), FFI_TYPE__ALIGN(arg_type_ptr)]);
  }
  প্রতি (var i = 0; i < nfixedargs; i++) {
    বিকল্প (unboxed_arg_type_id_list[i]) {
    পক্ষ FFI_TYPE_INT:
    পক্ষ FFI_TYPE_UINT8:
    পক্ষ FFI_TYPE_SINT8:
    পক্ষ FFI_TYPE_UINT16:
    পক্ষ FFI_TYPE_SINT16:
    পক্ষ FFI_TYPE_UINT32:
    পক্ষ FFI_TYPE_SINT32:
      sig += 'i';
      ভঙ্গ;
    পক্ষ FFI_TYPE_FLOAT:
      sig += 'f';
      ভঙ্গ;
    পক্ষ FFI_TYPE_DOUBLE:
      sig += 'd';
      ভঙ্গ;
    পক্ষ FFI_TYPE_LONGDOUBLE:
      sig += 'jj';
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_SINT64:
      sig += 'j';
      ভঙ্গ;
    পক্ষ FFI_TYPE_STRUCT:
    পক্ষ FFI_TYPE_POINTER:
      sig += PTR_SIG;
      ভঙ্গ;
    পক্ষ FFI_TYPE_COMPLEX:
      throw new Error('complex marshalling nyi');
    স্বভাব:
      throw new Error('Unexpected argtype ' + arg_type_id);
    }
  }
  যদি (nfixedargs < nargs) {
    // extra pointer to varargs stack
    sig += PTR_SIG;
  }
  প্রত্যাগম {
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
function ffi_closure_prelude(
  closure,
  {
    nfixedargs,
    nargs,
    ret_by_arg,
    unboxed_arg_type_info_list,
    unboxed_arg_type_id_list,
  },
) {
  প্রত্যাগম function prelude() {
    var args = arguments;
    var orig_স্তূপ_ptr = স্তূপ_Save();
    var cur_ptr = orig_স্তূপ_ptr;
    var ret_ptr;
    var jsarg_idx = 0;
    // Should we return by argument or not? The onwards call returns by argument
    // no matter what. (Warning: ret_by_arg means the opposite in ffi_call)
    যদি (ret_by_arg) {
      ret_ptr = DEC_PTR(args[jsarg_idx++]);
    } অন্যথা {
      // We might return 4 bytes or 8 bytes, allocate 8 just in case.
      স্তূপ_ALLOC(cur_ptr, 8, 8);
      ret_ptr = cur_ptr;
    }
    cur_ptr -= __SIZEOF_POINTER__ * nargs;
    var args_ptr = cur_ptr;
    var carg_idx = 0;
    // Here we either have the actual argument, or a pair of BigInts for long
    // double, or a pointer to struct. We have to store into args_ptr[i] a
    // pointer to the ith argument. If the argument is a struct, just store the
    // pointer. Otherwise allocate stack space and copy the js argument onto the
    // stack.
    প্রতি (; carg_idx < nfixedargs; carg_idx++) {
      // jsarg_idx might start out as 0 or 1 depending on ret_by_arg
      // it advances an extra time for long double
      var cur_arg = args[jsarg_idx++];
      var arg_type_info = unboxed_arg_type_info_list[carg_idx];
      var arg_size = arg_type_info[0];
      var arg_align = arg_type_info[1];
      var arg_type_id = unboxed_arg_type_id_list[carg_idx];
      বিকল্প (arg_type_id) {
      পক্ষ FFI_TYPE_UINT8:
      পক্ষ FFI_TYPE_SINT8:
        // Bad things happen if we don't align to 4 here
        স্তূপ_ALLOC(cur_ptr, 1, 4);
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U8(cur_ptr, 0) = cur_arg;
        ভঙ্গ;
      পক্ষ FFI_TYPE_UINT16:
      পক্ষ FFI_TYPE_SINT16:
        // Bad things happen if we don't align to 4 here
        স্তূপ_ALLOC(cur_ptr, 2, 4);
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U16(cur_ptr, 0) = cur_arg;
        ভঙ্গ;
      পক্ষ FFI_TYPE_INT:
      পক্ষ FFI_TYPE_UINT32:
      পক্ষ FFI_TYPE_SINT32:
        স্তূপ_ALLOC(cur_ptr, 4, 4);
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U32(cur_ptr, 0) = cur_arg;
        ভঙ্গ;
      পক্ষ FFI_TYPE_STRUCT:
        // cur_arg is already a pointer to struct
        // copy it onto stack to pass by value
        স্তূপ_ALLOC(cur_ptr, arg_size, arg_align);
        HEAP8.subarray(cur_ptr, cur_ptr + arg_size).set(HEAP8.subarray(DEC_PTR(cur_arg), DEC_PTR(cur_arg) + arg_size));
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(cur_ptr);
        ভঙ্গ;
      পক্ষ FFI_TYPE_FLOAT:
        স্তূপ_ALLOC(cur_ptr, 4, 4);
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_F32(cur_ptr, 0) = cur_arg;
        ভঙ্গ;
      পক্ষ FFI_TYPE_DOUBLE:
        স্তূপ_ALLOC(cur_ptr, 8, 8);
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_F64(cur_ptr, 0) = cur_arg;
        ভঙ্গ;
      পক্ষ FFI_TYPE_UINT64:
      পক্ষ FFI_TYPE_SINT64:
        স্তূপ_ALLOC(cur_ptr, 8, 8);
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U64(cur_ptr, 0) = cur_arg;
        ভঙ্গ;
      পক্ষ FFI_TYPE_LONGDOUBLE:
        স্তূপ_ALLOC(cur_ptr, 16, 8);
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_U64(cur_ptr, 0) = cur_arg;
        cur_arg = args[jsarg_idx++];
        DEREF_U64(cur_ptr, 1) = cur_arg;
        ভঙ্গ;
      পক্ষ FFI_TYPE_POINTER:
        স্তূপ_ALLOC(cur_ptr, __SIZEOF_POINTER__, __SIZEOF_POINTER__);
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(cur_ptr);
        DEREF_PTR(cur_ptr, 0) = cur_arg;
        ভঙ্গ;
      }
    }
    // If its a varargs call, last js argument is a pointer to the varargs.
    var varargs = DEC_PTR(args[args.length - 1]);
    // We have no way of knowing how many varargs were actually provided, this
    // fills the rest of the stack space allocated with nonsense. The onward
    // call will know to ignore the nonsense.

    // We either have a pointer to the argument if the argument is not a struct
    // or a pointer to pointer to struct. We need to store a pointer to the
    // argument into args_ptr[i]
    প্রতি (; carg_idx < nargs; carg_idx++) {
      var arg_type_id = unboxed_arg_type_id_list[carg_idx];
      var arg_type_info = unboxed_arg_type_info_list[carg_idx];
      var arg_size = arg_type_info[0];
      var arg_align = arg_type_info[1];
      যদি (arg_type_id === FFI_TYPE_STRUCT) {
        // In this case varargs is a pointer to pointer to struct so we need to
        // deref once
        var struct_ptr = DEREF_PTR_সংখ্যা(varargs, 0);
        স্তূপ_ALLOC(cur_ptr, arg_size, arg_align);
        HEAP8.subarray(cur_ptr, cur_ptr + arg_size).set(HEAP8.subarray(struct_ptr, struct_ptr + arg_size));
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(cur_ptr);
      } অন্যথা {
        DEREF_PTR(args_ptr, carg_idx) = ENC_PTR(varargs);
      }
      varargs += __SIZEOF_POINTER__;
    }
    স্তূপ_Restore(cur_ptr);
    স্তূপ_Alloc(0); // stackAlloc enforces alignment invariants on the stack pointer
    LOG_DEBUG("CALL_CLOSURE", "closure:", closure, "fptr", CLOSURE__fun(closure), "cif", CLOSURE__cif(closure));
    // The trampoline now does
    //    fun(cif, ret_ptr, args_ptr, user_data);
    //    return epilogue(ret_ptr, orig_stack_ptr);
    //
    // We return these arguments in reverse order because stack is LIFO.
    প্রত্যাগম [
      ENC_PTR(ret_ptr),          // epilogue args (in correct order)
      ENC_PTR(orig_স্তূপ_ptr),

      CLOSURE__cif(closure),     // call_indirect args (in correct order)
      ENC_PTR(ret_ptr),
      ENC_PTR(args_ptr),
      CLOSURE__user_data(closure),

      CLOSURE__fun(closure),     // ...and the table index for call_indirect
    ];
  }
}

// Returns function which restores the stack pointer and loads the closure's
// return value from ret_ptr.
function ffi_closure_epilogue(sig) {
  var load;
  বিকল্প (sig[0]) {
  পক্ষ 'i': load = (ptr) => DEREF_S32(ptr, 0); ভঙ্গ;
  পক্ষ 'j': load = (ptr) => DEREF_U64(ptr, 0); ভঙ্গ;
  পক্ষ 'f': load = (ptr) => DEREF_F32(ptr, 0); ভঙ্গ;
  পক্ষ 'd': load = (ptr) => DEREF_F64(ptr, 0); ভঙ্গ;
  স্বভাব:  load = (ptr) => undefined; ভঙ্গ;
  }
  প্রত্যাগম function epilogue(ret_ptr, orig_স্তূপ_ptr) {
    স্তূপ_Restore(DEC_PTR(orig_স্তূপ_ptr));
    প্রত্যাগম load(DEC_PTR(ret_ptr));
  };
}

var ffi_closure_trampoline_module_cache = new Map();
function ffi_closure_trampoline_module(sig) {
  var cache = ffi_closure_trampoline_module_cache;
  var module = cache.get(sig);
  যদি (module) {
    প্রত্যাগম module;
  }

  function uleb128(n) {
    var result = [];
    কুরু {
      var byte = n & 0x7f;
      n >>>= 7;
      যদি (n) {
        byte |= 0x80;
      }
      result.ঠেলো(byte);
    } যাবত্ (n);
    প্রত্যাগম result;
  }
  // A vector of single-byte items
  function vec(bytes) {
    প্রত্যাগম [...uleb128(bytes.length), ...bytes];
  }
  function section(id, bytes) {
    প্রত্যাগম [id, ...vec(bytes)];
  }
  function funcType(params, results) {
    প্রত্যাগম [0x60, ...vec(params), ...vec(results)];
  }

  var typeCodes = { 'i': 0x7f, 'j': 0x7e, 'f': 0x7d, 'd': 0x7c };
  var P = PTR_WASM_TYPE;
  var params = Array.from(sig.slice(1), (c) => typeCodes[c]);
  var results = sig[0] === 'v' ? [] : [typeCodes[sig[0]]];
  var nparams = params.length;

  // Is the function table 64-bit indexed?
  // -sMEMORY64=2 compiles with 64-bit pointers but uses a 32-bit memory and
  // table, so we have to i32.wrap_i64 pointers.
  var idx64 = typeof wasmTable.length === 'bigint';
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
  প্রতি (var i = 0; i < nparams; i++) {
    body.ঠেলো(0x20, ...uleb128(i)); // local.get i
  }
  body.ঠেলো(0x10, 0x00); // call prelude
  // Operand stack is now: ret_ptr, orig_sp, cif, ret_ptr, args_ptr, user_data, fun
  body.ঠেলো(...wrapPtr); // fun is used as a table index
  body.ঠেলো(0x11, 0x02, 0x00); // call_indirect (type 2) (table 0), consumes the top 5
  // Operand stack: ret_ptr, orig_sp
  body.ঠেলো(0x10, 0x01); // call epilogue, consumes both and leaves the result (if any)
  body.ঠেলো(0x0b); // end
  var codeSection = section(0x0a, [...uleb128(1), ...vec(body)]);

  var bytes = new Uint8Array([
    0x00, 0x61, 0x73, 0x6d, // magic
    0x01, 0x00, 0x00, 0x00, // version
    ...typeSection,
    ...importSection,
    ...functionSection,
    ...exportSection,
    ...codeSection,
  ]);
  LOG_DEBUG("CLOSURE_MODULE", "sig:", sig, "bytes:", bytes);
  // The module is tiny so compiling synchronously is fine.
  module = new WebAssembly.Module(bytes);
  cache.set(sig, module);
  প্রত্যাগম module;
}
)

// EM_JS does not correctly handle function pointer arguments, so we need a
// helper
ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif,
                                শূন্য (*fun)(ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
                                শূন্য *user_data, শূন্য *codeloc) {
  যদি (cif->abi != FFI_EMSCRIPTEN_ABI)
    প্রত্যাগম FFI_BAD_ABI;
  প্রত্যাগম ffi_prep_closure_loc_js(closure, cif, (শূন্য *)fun, user_data,
                                     codeloc);
}
