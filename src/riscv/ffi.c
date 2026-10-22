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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ "internal.h"
#ಅನ್ತರ್ಭಾವ <tramp.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>

#ಯದಿ __riscv_float_abi_double
#define ABI_FLEN 64
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ {
    uint64_t i;
    ದ್ವಿಗುಣ d;
} float_reg;
#ನೋಚೇದ್ಯದಿ __riscv_float_abi_single
#define ABI_FLEN 32
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ {
    ಭಿನ್ನ f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * ಪರಿಮಾಣ(ದ್ವಿಗುಣ))

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ call_context
{
#ಯದಿ ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    ವರ್ಣ frame[16];
} call_context;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ call_builder
{
    call_context *aregs;
    ಪೂರ್ಣಾಙ್ಕ used_integer;
    ಪೂರ್ಣಾಙ್ಕ used_float;
    size_t *used_ರಾಶಿ;
    ಶೂನ್ಯ *struct_ರಾಶಿ;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ಯದಿ __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ಅನ್ಯಥಾ
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#ಯದಿ ABI_FLEN
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
    ವರ್ಣ as_elements, type1, offset2, type2;
} float_struct_info;

#ಯದಿ ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#ಅನ್ಯಥಾ
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

ಸ್ಥಿರ ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    ಪೂರ್ಣಾಙ್ಕ i;
    ಯದಿ (out == out_end) ಪ್ರತ್ಯಾಗಮ out;
    ಯದಿ (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } ಅನ್ಯಥಾ {
        ಪ್ರತಿ (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    ಪ್ರತ್ಯಾಗಮ out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
ಸ್ಥಿರ float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *ತುದಿ) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_floats, ಅಂಕೆ_ints;
    ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_fields = flatten_struct(ತುದಿ, fields, fields + 3) - fields;

    ಯದಿ (ಅಂಕೆ_fields == 1) {
        ಯದಿ (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } ಅನ್ಯಥಾ ಯದಿ (ಅಂಕೆ_fields == 2) {
        ಅಂಕೆ_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        ಅಂಕೆ_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        ಯದಿ (ಅಂಕೆ_floats == 0 || ಅಂಕೆ_floats + ಅಂಕೆ_ints != 2)
            ಪ್ರತ್ಯಾಗಮ ret;
        ಯದಿ (cb->used_float + ಅಂಕೆ_floats > NARGREG || cb->used_integer + (2 - ಅಂಕೆ_floats) > NARGREG)
            ಪ್ರತ್ಯಾಗಮ ret;
        ಯದಿ (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            ಪ್ರತ್ಯಾಗಮ ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    ಪ್ರತ್ಯಾಗಮ ret;
}

#ಯದಿ ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
ಸ್ಥಿರ ಶೂನ್ಯ marshal_float(call_builder *cb, ಶೂನ್ಯ *data) {
    ಸಙ್ಘ {
        uint32_t i;
        ಭಿನ್ನ f;
    } ಮೌಲ್ಯ;

    ಮೌಲ್ಯ.f = *(ಭಿನ್ನ *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | ಮೌಲ್ಯ.i;
}

ಸ್ಥಿರ ಶೂನ್ಯ unmarshal_float(call_builder *cb, ಶೂನ್ಯ *data) {
    ಸಙ್ಘ {
        uint32_t i;
        ಭಿನ್ನ f;
    } ಮೌಲ್ಯ;

    ಮೌಲ್ಯ.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(ಭಿನ್ನ *)data = ಮೌಲ್ಯ.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ಸ್ಥಿರ ಶೂನ್ಯ marshal_atom(call_builder *cb, ಪೂರ್ಣಾಙ್ಕ type, ಶೂನ್ಯ *data) {
    size_t ಮೌಲ್ಯ = 0;
    ವಿಕಲ್ಪ (type) {
        ಪಕ್ಷ FFI_TYPE_UINT8: ಮೌಲ್ಯ = *(uint8_t *)data; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_SINT8: ಮೌಲ್ಯ = *(int8_t *)data; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_UINT16: ಮೌಲ್ಯ = *(uint16_t *)data; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_SINT16: ಮೌಲ್ಯ = *(int16_t *)data; ಭಙ್ಗ;
        /* 32-bit quantities are always sign-extended in the ABI */
        ಪಕ್ಷ FFI_TYPE_UINT32: ಮೌಲ್ಯ = *(int32_t *)data; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_SINT32: ಮೌಲ್ಯ = *(int32_t *)data; ಭಙ್ಗ;
#ಯದಿ __SIZEOF_POINTER__ == 8
        ಪಕ್ಷ FFI_TYPE_UINT64: ಮೌಲ್ಯ = *(uint64_t *)data; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_SINT64: ಮೌಲ್ಯ = *(int64_t *)data; ಭಙ್ಗ;
#endif
        ಪಕ್ಷ FFI_TYPE_POINTER: ಮೌಲ್ಯ = *(size_t *)data; ಭಙ್ಗ;

#ಯದಿ ABI_FLEN >= 64
        ಪಕ್ಷ FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            ಪ್ರತ್ಯಾಗಮ;
#ನೋಚೇದ್ಯದಿ ABI_FLEN >= 32
        ಪಕ್ಷ FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(ಭಿನ್ನ *)data));
            ಪ್ರತ್ಯಾಗಮ;
#endif
#ಯದಿ ABI_FLEN >= 64
        ಪಕ್ಷ FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(ದ್ವಿಗುಣ *)data));
            ಪ್ರತ್ಯಾಗಮ;
#endif
        ಸ್ವಭಾವ: FFI_ASSERT(0); ಭಙ್ಗ;
    }

    ಯದಿ (cb->used_integer == NARGREG) {
        *cb->used_ರಾಶಿ++ = ಮೌಲ್ಯ;
    } ಅನ್ಯಥಾ {
        cb->aregs->a[cb->used_integer++] = ಮೌಲ್ಯ;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ unmarshal_atom(call_builder *cb, ಪೂರ್ಣಾಙ್ಕ type, ಶೂನ್ಯ *data) {
    size_t ಮೌಲ್ಯ;
    ವಿಕಲ್ಪ (type) {
#ಯದಿ ABI_FLEN >= 64
        ಪಕ್ಷ FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            ಪ್ರತ್ಯಾಗಮ;
#ನೋಚೇದ್ಯದಿ ABI_FLEN >= 32
        ಪಕ್ಷ FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(ಭಿನ್ನ *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            ಪ್ರತ್ಯಾಗಮ;
#endif
#ಯದಿ ABI_FLEN >= 64
        ಪಕ್ಷ FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(ದ್ವಿಗುಣ *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            ಪ್ರತ್ಯಾಗಮ;
#endif
    }

    ಯದಿ (cb->used_integer == NARGREG) {
        ಮೌಲ್ಯ = *cb->used_ರಾಶಿ++;
    } ಅನ್ಯಥಾ {
        ಮೌಲ್ಯ = cb->aregs->a[cb->used_integer++];
    }

    ವಿಕಲ್ಪ (type) {
        ಪಕ್ಷ FFI_TYPE_UINT8: *(uint8_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_SINT8: *(uint8_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_UINT16: *(uint16_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_SINT16: *(uint16_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_UINT32: *(uint32_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_SINT32: *(uint32_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
#ಯದಿ __SIZEOF_POINTER__ == 8
        ಪಕ್ಷ FFI_TYPE_UINT64: *(uint64_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_SINT64: *(uint64_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
#endif
        ಪಕ್ಷ FFI_TYPE_POINTER: *(size_t *)data = ಮೌಲ್ಯ; ಭಙ್ಗ;
        ಸ್ವಭಾವ: FFI_ASSERT(0); ಭಙ್ಗ;
    }
}

/* adds an argument to a call, or a not by reference return value */
ಸ್ಥಿರ ಶೂನ್ಯ marshal(call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಙ್ಕ var, ಶೂನ್ಯ *data) {
    size_t realign[2];

#ಯದಿ ABI_FLEN
    ಯದಿ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ಯದಿ (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            ಯದಿ (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((ವರ್ಣ*)data) + fsi.offset2);
            ಪ್ರತ್ಯಾಗಮ;
        }
    }

    ಯದಿ (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        ಪ್ರತ್ಯಾಗಮ;
    }
#endif

    ಯದಿ (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = memcpy (cb->struct_ರಾಶಿ, data, type->size);
        cb->struct_ರಾಶಿ = (size_t *) FFI_ALIGN ((ವರ್ಣ *) cb->struct_ರಾಶಿ + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } ಅನ್ಯಥಾ ಯದಿ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } ಅನ್ಯಥಾ {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        ಯದಿ (type->alignment > __SIZEOF_POINTER__) {
            ಯದಿ (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_ರಾಶಿ = (size_t *)FFI_ALIGN(cb->used_ರಾಶಿ, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, data, type->size);
        ಯದಿ (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        ಯದಿ (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ಸ್ಥಿರ ಶೂನ್ಯ *unmarshal(call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಙ್ಕ var, ಶೂನ್ಯ *data) {
    size_t realign[2];
    ಶೂನ್ಯ *pointer;

#ಯದಿ ABI_FLEN
    ಯದಿ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ಯದಿ (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            ಯದಿ (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((ವರ್ಣ*)data) + fsi.offset2);
            ಪ್ರತ್ಯಾಗಮ data;
        }
    }

    ಯದಿ (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        ಪ್ರತ್ಯಾಗಮ data;
    }
#endif

    ಯದಿ (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (ವರ್ಣ*)&pointer);
        ಪ್ರತ್ಯಾಗಮ pointer;
    } ಅನ್ಯಥಾ ಯದಿ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        ಪ್ರತ್ಯಾಗಮ data;
    } ಅನ್ಯಥಾ {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        ಯದಿ (type->alignment > __SIZEOF_POINTER__) {
            ಯದಿ (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_ರಾಶಿ = (size_t *)FFI_ALIGN(cb->used_ರಾಶಿ, 2*__SIZEOF_POINTER__);
        }

        ಯದಿ (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        ಯದಿ (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(data, realign, type->size);
        ಪ್ರತ್ಯಾಗಮ data;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ passed_by_ref(call_builder *cb, ffi_type *type, ಪೂರ್ಣಾಙ್ಕ var) {
#ಯದಿ ABI_FLEN
    ಯದಿ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ಯದಿ (fsi.as_elements) ಪ್ರತ್ಯಾಗಮ 0;
    }
#endif

    ಪ್ರತ್ಯಾಗಮ type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs, unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Low level routine for calling functions */
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_asm (ಶೂನ್ಯ *ರಾಶಿ, ಸಂರಚನಾ call_context *regs,
			  ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *closure) FFI_HIDDEN;

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue,
	      ಶೂನ್ಯ *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * ಪರಿಮಾಣ(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    ಯದಿ (rvalue == ರಿಕ್ತ && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    size_t alloc_size = arg_bytes + rval_bytes + struct_bytes + ಪರಿಮಾಣ(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    ಯದಿ (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_size);
    } ಅನ್ಯಥಾ {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    ಯದಿ (rval_bytes)
        rvalue = (ಶೂನ್ಯ*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_ರಾಶಿ = (ಶೂನ್ಯ*)alloc_base;
    cb.struct_ರಾಶಿ = (ಶೂನ್ಯ *) (alloc_base + arg_bytes + rval_bytes);

    ಪೂರ್ಣಾಙ್ಕ return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    ಯದಿ (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((ಶೂನ್ಯ *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    ಯದಿ (!return_by_ref && rvalue)
      {
	ಯದಿ (IS_INT(cif->rtype->type)
	    && cif->rtype->size < ಪರಿಮಾಣ (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    ವಿಕಲ್ಪ (cif->rtype->type)
	      {
	      ಪಕ್ಷ FFI_TYPE_SINT8:
	      ಪಕ್ಷ FFI_TYPE_SINT16:
	      ಪಕ್ಷ FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (ಪರಿಮಾಣ (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		ಭಙ್ಗ;
	      ಪಕ್ಷ FFI_TYPE_UINT8:
	      ಪಕ್ಷ FFI_TYPE_UINT16:
	      ಪಕ್ಷ FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (ಪರಿಮಾಣ (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		ಭಙ್ಗ;
	      }
	  }
	ಅನ್ಯಥಾ
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ರಿಕ್ತ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_asm(ಶೂನ್ಯ) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*), ಶೂನ್ಯ *user_data, ಶೂನ್ಯ *codeloc)
{
    ಯದಿ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  ಯದಿ (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  ಅನ್ಯಥಾ
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#ಯದಿ __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#ಅನ್ಯಥಾ
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#ಯದಿ !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_asm (ಶೂನ್ಯ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *))
{
  ಯದಿ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  closure->tramp = (ಶೂನ್ಯ *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
ಶೂನ್ಯ FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		   ಶೂನ್ಯ *user_data,
		   size_t *ರಾಶಿ, call_context *aregs)
{
    ಶೂನ್ಯ **avalue = alloca(cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    ವರ್ಣ *astorage = alloca(cif->nargs * MAXCOPYARG);
    ಶೂನ್ಯ *rvalue;
    call_builder cb;
    ಪೂರ್ಣಾಙ್ಕ return_by_ref;
    ಪೂರ್ಣಾಙ್ಕ i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_ರಾಶಿ = ರಾಶಿ;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    ಯದಿ (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    ಅನ್ಯಥಾ
        rvalue = alloca(cif->rtype->size);

    ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    ಯದಿ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  ಪ್ರತ್ಯಾಗಮ &trampoline_code_table;
}
#endif
