/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2015 Michael Knyszek <mknyszek@berkeley.edu>
                         2015 Andrew Waterman <waterman@cs.berkeley.edu>
                         2018 Stef O'Rear <sorear2@gmail.com>
   Based on MIPS N32/64 port

   RISC-V Foreign Function Interface

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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>

#ਜੇ __riscv_float_abi_double
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ABI_FLEN 64
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ {
    uint64_t i;
    ਦੁੱਗਣਾ d;
} float_reg;
#ਜਾਂਜੇ __riscv_float_abi_single
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ABI_FLEN 32
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ {
    ਦਸ਼ਮਲਵ f;
} float_reg;
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NARGREG 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STKALIGN 16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXCOPYARG (2 * ਆਕਾਰ(ਦੁੱਗਣਾ))

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ call_context
{
#ਜੇ ABI_FLEN
    float_reg fa[8];
#ਜੇ_ਅੰਤ
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    ਅੱਖਰ ਚੌਖਟਾ[16];
} call_context;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ call_builder
{
    call_context *aregs;
    ਪੂਰਨ ਵਰਤਿਆ_integer;
    ਪੂਰਨ ਵਰਤਿਆ_float;
    size_t *ਵਰਤਿਆ_ਢੇਰ;
    ਖਾਲੀ *struct_ਢੇਰ;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ਜੇ __SIZEOF_POINTER__ == 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#ਜੇ_ਅੰਤ

#ਜੇ ABI_FLEN
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    ਅੱਖਰ as_elements, type1, offset2, type2;
} float_struct_info;

#ਜੇ ABI_FLEN >= 64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#ਜੇ_ਅੰਤ

ਸਥਿਰ ffi_type **flatten_struct(ffi_type *ਵਿੱਚ_2, ffi_type **ਬਾਹਰਲਾ, ffi_type **ਬਾਹਰਲਾ_ਸਿਰਾ) {
    ਪੂਰਨ i;
    ਜੇ (ਬਾਹਰਲਾ == ਬਾਹਰਲਾ_ਸਿਰਾ) ਵਾਪਸ ਬਾਹਰਲਾ;
    ਜੇ (ਵਿੱਚ_2->type != FFI_TYPE_STRUCT) {
        *(ਬਾਹਰਲਾ++) = ਵਿੱਚ_2;
    } ਨਹੀਂਤਾਂ {
        ਲਈ (i = 0; ਵਿੱਚ_2->elements[i]; i++)
            ਬਾਹਰਲਾ = flatten_struct(ਵਿੱਚ_2->elements[i], ਬਾਹਰਲਾ, ਬਾਹਰਲਾ_ਸਿਰਾ);
    }
    ਵਾਪਸ ਬਾਹਰਲਾ;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
ਸਥਿਰ float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *ਸਿਖਰ) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    ਪੂਰਨ ਅੰਕ_floats, ਅੰਕ_ints;
    ਪੂਰਨ ਅੰਕ_fields = flatten_struct(ਸਿਖਰ, fields, fields + 3) - fields;

    ਜੇ (ਅੰਕ_fields == 1) {
        ਜੇ (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } ਨਹੀਂਤਾਂ ਜੇ (ਅੰਕ_fields == 2) {
        ਅੰਕ_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        ਅੰਕ_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        ਜੇ (ਅੰਕ_floats == 0 || ਅੰਕ_floats + ਅੰਕ_ints != 2)
            ਵਾਪਸ ret;
        ਜੇ (cb->ਵਰਤਿਆ_float + ਅੰਕ_floats > NARGREG || cb->ਵਰਤਿਆ_integer + (2 - ਅੰਕ_floats) > NARGREG)
            ਵਾਪਸ ret;
        ਜੇ (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            ਵਾਪਸ ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->ਮਾਪ, fields[1]->alignment);
        ret.as_elements = 1;
    }

    ਵਾਪਸ ret;
}

#ਜੇ ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
ਸਥਿਰ ਖਾਲੀ marshal_float(call_builder *cb, ਖਾਲੀ *ਅੰਕੜੇ) {
    ਸੰਘ {
        uint32_t i;
        ਦਸ਼ਮਲਵ f;
    } ਮੁੱਲ;

    ਮੁੱਲ.f = *(ਦਸ਼ਮਲਵ *)ਅੰਕੜੇ;
    cb->aregs->fa[cb->ਵਰਤਿਆ_float++].i =
        UINT64_C(0xffffffff00000000) | ਮੁੱਲ.i;
}

ਸਥਿਰ ਖਾਲੀ unmarshal_float(call_builder *cb, ਖਾਲੀ *ਅੰਕੜੇ) {
    ਸੰਘ {
        uint32_t i;
        ਦਸ਼ਮਲਵ f;
    } ਮੁੱਲ;

    ਮੁੱਲ.i = (uint32_t)cb->aregs->fa[cb->ਵਰਤਿਆ_float++].i;
    *(ਦਸ਼ਮਲਵ *)ਅੰਕੜੇ = ਮੁੱਲ.f;
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ਸਥਿਰ ਖਾਲੀ marshal_atom(call_builder *cb, ਪੂਰਨ type, ਖਾਲੀ *ਅੰਕੜੇ) {
    size_t ਮੁੱਲ = 0;
    ਚੋਣ (type) {
        ਹਾਲਤ FFI_TYPE_UINT8: ਮੁੱਲ = *(uint8_t *)ਅੰਕੜੇ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_SINT8: ਮੁੱਲ = *(int8_t *)ਅੰਕੜੇ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_UINT16: ਮੁੱਲ = *(uint16_t *)ਅੰਕੜੇ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_SINT16: ਮੁੱਲ = *(int16_t *)ਅੰਕੜੇ; ਰੋਕੋ;
        /* 32-bit quantities are always sign-extended in the ABI */
        ਹਾਲਤ FFI_TYPE_UINT32: ਮੁੱਲ = *(int32_t *)ਅੰਕੜੇ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_SINT32: ਮੁੱਲ = *(int32_t *)ਅੰਕੜੇ; ਰੋਕੋ;
#ਜੇ __SIZEOF_POINTER__ == 8
        ਹਾਲਤ FFI_TYPE_UINT64: ਮੁੱਲ = *(uint64_t *)ਅੰਕੜੇ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_SINT64: ਮੁੱਲ = *(int64_t *)ਅੰਕੜੇ; ਰੋਕੋ;
#ਜੇ_ਅੰਤ
        ਹਾਲਤ FFI_TYPE_ਸੂਚਕ: ਮੁੱਲ = *(size_t *)ਅੰਕੜੇ; ਰੋਕੋ;

#ਜੇ ABI_FLEN >= 64
        ਹਾਲਤ FFI_TYPE_FLOAT:
            marshal_float(cb, ਅੰਕੜੇ);
            ਵਾਪਸ;
#ਜਾਂਜੇ ABI_FLEN >= 32
        ਹਾਲਤ FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->ਵਰਤਿਆ_float++].f) : "0"(*(ਦਸ਼ਮਲਵ *)ਅੰਕੜੇ));
            ਵਾਪਸ;
#ਜੇ_ਅੰਤ
#ਜੇ ABI_FLEN >= 64
        ਹਾਲਤ FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->ਵਰਤਿਆ_float++].d) : "0"(*(ਦੁੱਗਣਾ *)ਅੰਕੜੇ));
            ਵਾਪਸ;
#ਜੇ_ਅੰਤ
        ਮੂਲ: FFI_ASSERT(0); ਰੋਕੋ;
    }

    ਜੇ (cb->ਵਰਤਿਆ_integer == NARGREG) {
        *cb->ਵਰਤਿਆ_ਢੇਰ++ = ਮੁੱਲ;
    } ਨਹੀਂਤਾਂ {
        cb->aregs->a[cb->ਵਰਤਿਆ_integer++] = ਮੁੱਲ;
    }
}

ਸਥਿਰ ਖਾਲੀ unmarshal_atom(call_builder *cb, ਪੂਰਨ type, ਖਾਲੀ *ਅੰਕੜੇ) {
    size_t ਮੁੱਲ;
    ਚੋਣ (type) {
#ਜੇ ABI_FLEN >= 64
        ਹਾਲਤ FFI_TYPE_FLOAT:
            unmarshal_float(cb, ਅੰਕੜੇ);
            ਵਾਪਸ;
#ਜਾਂਜੇ ABI_FLEN >= 32
        ਹਾਲਤ FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(ਦਸ਼ਮਲਵ *)ਅੰਕੜੇ) : "0"(cb->aregs->fa[cb->ਵਰਤਿਆ_float++].f));
            ਵਾਪਸ;
#ਜੇ_ਅੰਤ
#ਜੇ ABI_FLEN >= 64
        ਹਾਲਤ FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(ਦੁੱਗਣਾ *)ਅੰਕੜੇ) : "0"(cb->aregs->fa[cb->ਵਰਤਿਆ_float++].d));
            ਵਾਪਸ;
#ਜੇ_ਅੰਤ
    }

    ਜੇ (cb->ਵਰਤਿਆ_integer == NARGREG) {
        ਮੁੱਲ = *cb->ਵਰਤਿਆ_ਢੇਰ++;
    } ਨਹੀਂਤਾਂ {
        ਮੁੱਲ = cb->aregs->a[cb->ਵਰਤਿਆ_integer++];
    }

    ਚੋਣ (type) {
        ਹਾਲਤ FFI_TYPE_UINT8: *(uint8_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_SINT8: *(uint8_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_UINT16: *(uint16_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_SINT16: *(uint16_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_UINT32: *(uint32_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_SINT32: *(uint32_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
#ਜੇ __SIZEOF_POINTER__ == 8
        ਹਾਲਤ FFI_TYPE_UINT64: *(uint64_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_SINT64: *(uint64_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
#ਜੇ_ਅੰਤ
        ਹਾਲਤ FFI_TYPE_ਸੂਚਕ: *(size_t *)ਅੰਕੜੇ = ਮੁੱਲ; ਰੋਕੋ;
        ਮੂਲ: FFI_ASSERT(0); ਰੋਕੋ;
    }
}

/* adds an argument to a call, or a not by reference return value */
ਸਥਿਰ ਖਾਲੀ marshal(call_builder *cb, ffi_type *type, ਪੂਰਨ var, ਖਾਲੀ *ਅੰਕੜੇ) {
    size_t realign[2];

#ਜੇ ABI_FLEN
    ਜੇ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ਜੇ (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, ਅੰਕੜੇ);
            ਜੇ (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((ਅੱਖਰ*)ਅੰਕੜੇ) + fsi.offset2);
            ਵਾਪਸ;
        }
    }

    ਜੇ (!var && cb->ਵਰਤਿਆ_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, ਅੰਕੜੇ);
        ਵਾਪਸ;
    }
#ਜੇ_ਅੰਤ

    ਜੇ (type->ਮਾਪ > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        ਅੰਕੜੇ = ਸਮ੍ਰਿਤੀ_ਨਕਲ (cb->struct_ਢੇਰ, ਅੰਕੜੇ, type->ਮਾਪ);
        cb->struct_ਢੇਰ = (size_t *) FFI_ALIGN ((ਅੱਖਰ *) cb->struct_ਢੇਰ + type->ਮਾਪ, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_ਸੂਚਕ, &ਅੰਕੜੇ);
    } ਨਹੀਂਤਾਂ ਜੇ (IS_INT(type->type) || type->type == FFI_TYPE_ਸੂਚਕ) {
        marshal_atom(cb, type->type, ਅੰਕੜੇ);
    } ਨਹੀਂਤਾਂ {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        ਜੇ (type->alignment > __SIZEOF_POINTER__) {
            ਜੇ (var)
                cb->ਵਰਤਿਆ_integer = FFI_ALIGN(cb->ਵਰਤਿਆ_integer, 2);
            cb->ਵਰਤਿਆ_ਢੇਰ = (size_t *)FFI_ALIGN(cb->ਵਰਤਿਆ_ਢੇਰ, 2*__SIZEOF_POINTER__);
        }

        ਸਮ੍ਰਿਤੀ_ਨਕਲ(realign, ਅੰਕੜੇ, type->ਮਾਪ);
        ਜੇ (type->ਮਾਪ > 0)
            marshal_atom(cb, FFI_TYPE_ਸੂਚਕ, realign);
        ਜੇ (type->ਮਾਪ > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_ਸੂਚਕ, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ਸਥਿਰ ਖਾਲੀ *unmarshal(call_builder *cb, ffi_type *type, ਪੂਰਨ var, ਖਾਲੀ *ਅੰਕੜੇ) {
    size_t realign[2];
    ਖਾਲੀ *ਸੂਚਕ;

#ਜੇ ABI_FLEN
    ਜੇ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ਜੇ (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, ਅੰਕੜੇ);
            ਜੇ (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((ਅੱਖਰ*)ਅੰਕੜੇ) + fsi.offset2);
            ਵਾਪਸ ਅੰਕੜੇ;
        }
    }

    ਜੇ (!var && cb->ਵਰਤਿਆ_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, ਅੰਕੜੇ);
        ਵਾਪਸ ਅੰਕੜੇ;
    }
#ਜੇ_ਅੰਤ

    ਜੇ (type->ਮਾਪ > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_ਸੂਚਕ, (ਅੱਖਰ*)&ਸੂਚਕ);
        ਵਾਪਸ ਸੂਚਕ;
    } ਨਹੀਂਤਾਂ ਜੇ (IS_INT(type->type) || type->type == FFI_TYPE_ਸੂਚਕ) {
        unmarshal_atom(cb, type->type, ਅੰਕੜੇ);
        ਵਾਪਸ ਅੰਕੜੇ;
    } ਨਹੀਂਤਾਂ {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        ਜੇ (type->alignment > __SIZEOF_POINTER__) {
            ਜੇ (var)
                cb->ਵਰਤਿਆ_integer = FFI_ALIGN(cb->ਵਰਤਿਆ_integer, 2);
            cb->ਵਰਤਿਆ_ਢੇਰ = (size_t *)FFI_ALIGN(cb->ਵਰਤਿਆ_ਢੇਰ, 2*__SIZEOF_POINTER__);
        }

        ਜੇ (type->ਮਾਪ > 0)
            unmarshal_atom(cb, FFI_TYPE_ਸੂਚਕ, realign);
        ਜੇ (type->ਮਾਪ > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_ਸੂਚਕ, realign + 1);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ, realign, type->ਮਾਪ);
        ਵਾਪਸ ਅੰਕੜੇ;
    }
}

ਸਥਿਰ ਪੂਰਨ passed_by_ref(call_builder *cb, ffi_type *type, ਪੂਰਨ var) {
#ਜੇ ABI_FLEN
    ਜੇ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ਜੇ (fsi.as_elements) ਵਾਪਸ 0;
    }
#ਜੇ_ਅੰਤ

    ਵਾਪਸ type->ਮਾਪ > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    ਵਾਪਸ FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_ਸਥਿਤੀ ffi_prep_cif_machdep_var(ffi_cif *cif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    ਵਾਪਸ FFI_OK;
}

/* Low level routine for calling functions */
ਬਾਹਰੀ ਖਾਲੀ ffi_call_asm (ਖਾਲੀ *ਢੇਰ, ਬਣਤਰ call_context *regs,
			  ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *ਘੇਰਾ) FFI_HIDDEN;

ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue,
	      ਖਾਲੀ *ਘੇਰਾ)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_ਬਾਈਟਾਂ = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * ਆਕਾਰ(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_ਬਾਈਟਾਂ = FFI_ALIGN (cif->ਬਾਈਟਾਂ, STKALIGN);
    size_t rval_ਬਾਈਟਾਂ = 0;
    ਜੇ (rvalue == ਨਿਰਾ && cif->rtype->ਮਾਪ > 2*__SIZEOF_POINTER__)
        rval_ਬਾਈਟਾਂ = FFI_ALIGN(cif->rtype->ਮਾਪ, STKALIGN);
    size_t alloc_ਮਾਪ = arg_ਬਾਈਟਾਂ + rval_ਬਾਈਟਾਂ + struct_ਬਾਈਟਾਂ + ਆਕਾਰ(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_ਅਧਾਰ;
    /* Argument region must be 16-byte aligned */
    ਜੇ (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_ਅਧਾਰ = (size_t)alloca(alloc_ਮਾਪ);
    } ਨਹੀਂਤਾਂ {
        alloc_ਅਧਾਰ = FFI_ALIGN(alloca(alloc_ਮਾਪ + STKALIGN - 1), STKALIGN);
    }

    ਜੇ (rval_ਬਾਈਟਾਂ)
        rvalue = (ਖਾਲੀ*)(alloc_ਅਧਾਰ + arg_ਬਾਈਟਾਂ);

    call_builder cb;
    cb.ਵਰਤਿਆ_float = cb.ਵਰਤਿਆ_integer = 0;
    cb.aregs = (call_context*)(alloc_ਅਧਾਰ + arg_ਬਾਈਟਾਂ + rval_ਬਾਈਟਾਂ + struct_ਬਾਈਟਾਂ);
    cb.ਵਰਤਿਆ_ਢੇਰ = (ਖਾਲੀ*)alloc_ਅਧਾਰ;
    cb.struct_ਢੇਰ = (ਖਾਲੀ *) (alloc_ਅਧਾਰ + arg_ਬਾਈਟਾਂ + rval_ਬਾਈਟਾਂ);

    ਪੂਰਨ return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    ਜੇ (return_by_ref)
        marshal(&cb, &ffi_type_ਸੂਚਕ, 0, &rvalue);

    ਪੂਰਨ i;
    ਲਈ (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((ਖਾਲੀ *) alloc_ਅਧਾਰ, cb.aregs, fn, ਘੇਰਾ);

    cb.ਵਰਤਿਆ_float = cb.ਵਰਤਿਆ_integer = 0;
    ਜੇ (!return_by_ref && rvalue)
      {
	ਜੇ (IS_INT(cif->rtype->type)
	    && cif->rtype->ਮਾਪ < ਆਕਾਰ (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    ਚੋਣ (cif->rtype->type)
	      {
	      ਹਾਲਤ FFI_TYPE_SINT8:
	      ਹਾਲਤ FFI_TYPE_SINT16:
	      ਹਾਲਤ FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (ਆਕਾਰ (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		ਰੋਕੋ;
	      ਹਾਲਤ FFI_TYPE_UINT8:
	      ਹਾਲਤ FFI_TYPE_UINT16:
	      ਹਾਲਤ FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (ਆਕਾਰ (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		ਰੋਕੋ;
	      }
	  }
	ਨਹੀਂਤਾਂ
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ਨਿਰਾ);
}

ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int(cif, fn, rvalue, avalue, ਘੇਰਾ);
}

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_asm(ਖਾਲੀ) FFI_HIDDEN;

ffi_ਸਥਿਤੀ ffi_prep_ਘੇਰਾ_loc(ffi_ਘੇਰਾ *ਘੇਰਾ, ffi_cif *cif, ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*), ਖਾਲੀ *user_ਅੰਕੜੇ, ਖਾਲੀ *codeloc)
{
    ਜੇ (cif->abi <= FFI_ਪਹਿਲਾ_ABI || cif->abi >= FFI_ਆਖਰੀ_ABI)
        ਵਾਪਸ FFI_BAD_ABI;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_EXEC_STATIC_TRAMP
  ਜੇ (ffi_tramp_is_present (ਘੇਰਾ))
    {
      /* Initialize the static trampoline's parameters. */
      ਖਾਲੀ (*dest)(ਖਾਲੀ) = ffi_ਘੇਰਾ_asm;
      ffi_tramp_ਸੈੱਟ_parms (ਘੇਰਾ->ftramp, dest, ਘੇਰਾ);
    }
  ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      uint32_t *tramp = (uint32_t *) &ਘੇਰਾ->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_ਘੇਰਾ_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#ਜੇ __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#ਨਹੀਂਤਾਂ
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#ਜੇ_ਅੰਤ
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#ਜੇ !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_ਮਾਪ);
#ਜੇ_ਅੰਤ
    }

    ਘੇਰਾ->cif = cif;
    ਘੇਰਾ->fun = fun;
    ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

    ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_asm (ਖਾਲੀ) FFI_HIDDEN;

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ *ਘੇਰਾ, ffi_cif *cif,
		     ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *))
{
  ਜੇ (cif->abi <= FFI_ਪਹਿਲਾ_ABI || cif->abi >= FFI_ਆਖਰੀ_ABI)
    ਵਾਪਸ FFI_BAD_ABI;

  ਘੇਰਾ->tramp = (ਖਾਲੀ *) ffi_go_ਘੇਰਾ_asm;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
ਖਾਲੀ FFI_HIDDEN
ffi_ਘੇਰਾ_inner (ffi_cif *cif,
		   ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		   ਖਾਲੀ *user_ਅੰਕੜੇ,
		   size_t *ਢੇਰ, call_context *aregs)
{
    ਖਾਲੀ **avalue = alloca(cif->nargs * ਆਕਾਰ(ਖਾਲੀ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    ਅੱਖਰ *astorage = alloca(cif->nargs * MAXCOPYARG);
    ਖਾਲੀ *rvalue;
    call_builder cb;
    ਪੂਰਨ return_by_ref;
    ਪੂਰਨ i;

    cb.aregs = aregs;
    cb.ਵਰਤਿਆ_integer = cb.ਵਰਤਿਆ_float = 0;
    cb.ਵਰਤਿਆ_ਢੇਰ = ਢੇਰ;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    ਜੇ (return_by_ref)
        unmarshal(&cb, &ffi_type_ਸੂਚਕ, 0, &rvalue);
    ਨਹੀਂਤਾਂ
        rvalue = alloca(cif->rtype->ਮਾਪ);

    ਲਈ (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);

    ਜੇ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.ਵਰਤਿਆ_integer = cb.ਵਰਤਿਆ_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_EXEC_STATIC_TRAMP
ਖਾਲੀ *
ffi_tramp_arch (size_t *tramp_ਮਾਪ, size_t *map_ਮਾਪ)
{
  ਬਾਹਰੀ ਖਾਲੀ *trampoline_ਕੋਡ_ਸਾਰਣੀ;
  *tramp_ਮਾਪ = RISCV_TRAMP_ਮਾਪ;
  *map_ਮਾਪ = RISCV_TRAMP_MAP_ਮਾਪ;
  ਵਾਪਸ &trampoline_ਕੋਡ_ਸਾਰਣੀ;
}
#ਜੇ_ਅੰਤ
