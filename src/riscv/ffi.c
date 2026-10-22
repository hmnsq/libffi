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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ "internal.h"
#ଅନ୍ତର୍ଭାଵ <tramp.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>

#ଯଦି __riscv_float_abi_double
#define ABI_FLEN 64
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂଘ {
    uint64_t i;
    ଦ୍ୱିଗୁଣ d;
} float_reg;
#ନଚେତଯଦି __riscv_float_abi_single
#define ABI_FLEN 32
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂଘ {
    ଦଶମିକ f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * ଆକାର(ଦ୍ୱିଗୁଣ))

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା call_context
{
#ଯଦି ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    ଅକ୍ଷର frame[16];
} call_context;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା call_builder
{
    call_context *aregs;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା used_integer;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା used_float;
    size_t *used_ଗଦା;
    ଶୂନ୍ୟ *struct_ଗଦା;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ଯଦି __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ଅନ୍ୟଥା
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#ଯଦି ABI_FLEN
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    ଅକ୍ଷର as_elements, type1, offset2, type2;
} float_struct_info;

#ଯଦି ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#ଅନ୍ୟଥା
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

ସ୍ଥିର ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ଯଦି (out == out_end) ଫେରନ୍ତୁ out;
    ଯଦି (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } ଅନ୍ୟଥା {
        ପାଇଁ (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    ଫେରନ୍ତୁ out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
ସ୍ଥିର float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *ଶୀର୍ଷ) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_floats, ଅଙ୍କ_ints;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_fields = flatten_struct(ଶୀର୍ଷ, fields, fields + 3) - fields;

    ଯଦି (ଅଙ୍କ_fields == 1) {
        ଯଦି (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } ଅନ୍ୟଥା ଯଦି (ଅଙ୍କ_fields == 2) {
        ଅଙ୍କ_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        ଅଙ୍କ_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        ଯଦି (ଅଙ୍କ_floats == 0 || ଅଙ୍କ_floats + ଅଙ୍କ_ints != 2)
            ଫେରନ୍ତୁ ret;
        ଯଦି (cb->used_float + ଅଙ୍କ_floats > NARGREG || cb->used_integer + (2 - ଅଙ୍କ_floats) > NARGREG)
            ଫେରନ୍ତୁ ret;
        ଯଦି (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            ଫେରନ୍ତୁ ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    ଫେରନ୍ତୁ ret;
}

#ଯଦି ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
ସ୍ଥିର ଶୂନ୍ୟ marshal_float(call_builder *cb, ଶୂନ୍ୟ *data) {
    ସଂଘ {
        uint32_t i;
        ଦଶମିକ f;
    } ମୂଲ୍ୟ;

    ମୂଲ୍ୟ.f = *(ଦଶମିକ *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | ମୂଲ୍ୟ.i;
}

ସ୍ଥିର ଶୂନ୍ୟ unmarshal_float(call_builder *cb, ଶୂନ୍ୟ *data) {
    ସଂଘ {
        uint32_t i;
        ଦଶମିକ f;
    } ମୂଲ୍ୟ;

    ମୂଲ୍ୟ.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(ଦଶମିକ *)data = ମୂଲ୍ୟ.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
ସ୍ଥିର ଶୂନ୍ୟ marshal_atom(call_builder *cb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ଶୂନ୍ୟ *data) {
    size_t ମୂଲ୍ୟ = 0;
    ବାଛ (type) {
        ସ୍ଥିତି FFI_TYPE_UINT8: ମୂଲ୍ୟ = *(uint8_t *)data; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_SINT8: ମୂଲ୍ୟ = *(int8_t *)data; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_UINT16: ମୂଲ୍ୟ = *(uint16_t *)data; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_SINT16: ମୂଲ୍ୟ = *(int16_t *)data; ଭାଙ୍ଗ;
        /* 32-bit quantities are always sign-extended in the ABI */
        ସ୍ଥିତି FFI_TYPE_UINT32: ମୂଲ୍ୟ = *(int32_t *)data; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_SINT32: ମୂଲ୍ୟ = *(int32_t *)data; ଭାଙ୍ଗ;
#ଯଦି __SIZEOF_POINTER__ == 8
        ସ୍ଥିତି FFI_TYPE_UINT64: ମୂଲ୍ୟ = *(uint64_t *)data; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_SINT64: ମୂଲ୍ୟ = *(int64_t *)data; ଭାଙ୍ଗ;
#endif
        ସ୍ଥିତି FFI_TYPE_POINTER: ମୂଲ୍ୟ = *(size_t *)data; ଭାଙ୍ଗ;

#ଯଦି ABI_FLEN >= 64
        ସ୍ଥିତି FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            ଫେରନ୍ତୁ;
#ନଚେତଯଦି ABI_FLEN >= 32
        ସ୍ଥିତି FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(ଦଶମିକ *)data));
            ଫେରନ୍ତୁ;
#endif
#ଯଦି ABI_FLEN >= 64
        ସ୍ଥିତି FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(ଦ୍ୱିଗୁଣ *)data));
            ଫେରନ୍ତୁ;
#endif
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ: FFI_ASSERT(0); ଭାଙ୍ଗ;
    }

    ଯଦି (cb->used_integer == NARGREG) {
        *cb->used_ଗଦା++ = ମୂଲ୍ୟ;
    } ଅନ୍ୟଥା {
        cb->aregs->a[cb->used_integer++] = ମୂଲ୍ୟ;
    }
}

ସ୍ଥିର ଶୂନ୍ୟ unmarshal_atom(call_builder *cb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ଶୂନ୍ୟ *data) {
    size_t ମୂଲ୍ୟ;
    ବାଛ (type) {
#ଯଦି ABI_FLEN >= 64
        ସ୍ଥିତି FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            ଫେରନ୍ତୁ;
#ନଚେତଯଦି ABI_FLEN >= 32
        ସ୍ଥିତି FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(ଦଶମିକ *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            ଫେରନ୍ତୁ;
#endif
#ଯଦି ABI_FLEN >= 64
        ସ୍ଥିତି FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(ଦ୍ୱିଗୁଣ *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            ଫେରନ୍ତୁ;
#endif
    }

    ଯଦି (cb->used_integer == NARGREG) {
        ମୂଲ୍ୟ = *cb->used_ଗଦା++;
    } ଅନ୍ୟଥା {
        ମୂଲ୍ୟ = cb->aregs->a[cb->used_integer++];
    }

    ବାଛ (type) {
        ସ୍ଥିତି FFI_TYPE_UINT8: *(uint8_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_SINT8: *(uint8_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_UINT16: *(uint16_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_SINT16: *(uint16_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_UINT32: *(uint32_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_SINT32: *(uint32_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
#ଯଦି __SIZEOF_POINTER__ == 8
        ସ୍ଥିତି FFI_TYPE_UINT64: *(uint64_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_SINT64: *(uint64_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
#endif
        ସ୍ଥିତି FFI_TYPE_POINTER: *(size_t *)data = ମୂଲ୍ୟ; ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ: FFI_ASSERT(0); ଭାଙ୍ଗ;
    }
}

/* adds an argument to a call, or a not by reference return value */
ସ୍ଥିର ଶୂନ୍ୟ marshal(call_builder *cb, ffi_type *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା var, ଶୂନ୍ୟ *data) {
    size_t realign[2];

#ଯଦି ABI_FLEN
    ଯଦି (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ଯଦି (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            ଯଦି (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((ଅକ୍ଷର*)data) + fsi.offset2);
            ଫେରନ୍ତୁ;
        }
    }

    ଯଦି (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        ଫେରନ୍ତୁ;
    }
#endif

    ଯଦି (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = memcpy (cb->struct_ଗଦା, data, type->size);
        cb->struct_ଗଦା = (size_t *) FFI_ALIGN ((ଅକ୍ଷର *) cb->struct_ଗଦା + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } ଅନ୍ୟଥା ଯଦି (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } ଅନ୍ୟଥା {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        ଯଦି (type->alignment > __SIZEOF_POINTER__) {
            ଯଦି (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_ଗଦା = (size_t *)FFI_ALIGN(cb->used_ଗଦା, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, data, type->size);
        ଯଦି (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        ଯଦି (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
ସ୍ଥିର ଶୂନ୍ୟ *unmarshal(call_builder *cb, ffi_type *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା var, ଶୂନ୍ୟ *data) {
    size_t realign[2];
    ଶୂନ୍ୟ *pointer;

#ଯଦି ABI_FLEN
    ଯଦି (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ଯଦି (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            ଯଦି (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((ଅକ୍ଷର*)data) + fsi.offset2);
            ଫେରନ୍ତୁ data;
        }
    }

    ଯଦି (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        ଫେରନ୍ତୁ data;
    }
#endif

    ଯଦି (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (ଅକ୍ଷର*)&pointer);
        ଫେରନ୍ତୁ pointer;
    } ଅନ୍ୟଥା ଯଦି (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        ଫେରନ୍ତୁ data;
    } ଅନ୍ୟଥା {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        ଯଦି (type->alignment > __SIZEOF_POINTER__) {
            ଯଦି (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_ଗଦା = (size_t *)FFI_ALIGN(cb->used_ଗଦା, 2*__SIZEOF_POINTER__);
        }

        ଯଦି (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        ଯଦି (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(data, realign, type->size);
        ଫେରନ୍ତୁ data;
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା passed_by_ref(call_builder *cb, ffi_type *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା var) {
#ଯଦି ABI_FLEN
    ଯଦି (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ଯଦି (fsi.as_elements) ଫେରନ୍ତୁ 0;
    }
#endif

    ଫେରନ୍ତୁ type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    ଫେରନ୍ତୁ FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    ଫେରନ୍ତୁ FFI_OK;
}

/* Low level routine for calling functions */
ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_asm (ଶୂନ୍ୟ *ଗଦା, ସଂରଚନା call_context *regs,
			  ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *closure) FFI_HIDDEN;

ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue,
	      ଶୂନ୍ୟ *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * ଆକାର(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    ଯଦି (rvalue == ଖାଲି && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    size_t alloc_size = arg_bytes + rval_bytes + struct_bytes + ଆକାର(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    ଯଦି (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_size);
    } ଅନ୍ୟଥା {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    ଯଦି (rval_bytes)
        rvalue = (ଶୂନ୍ୟ*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_ଗଦା = (ଶୂନ୍ୟ*)alloc_base;
    cb.struct_ଗଦା = (ଶୂନ୍ୟ *) (alloc_base + arg_bytes + rval_bytes);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    ଯଦି (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((ଶୂନ୍ୟ *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    ଯଦି (!return_by_ref && rvalue)
      {
	ଯଦି (IS_INT(cif->rtype->type)
	    && cif->rtype->size < ଆକାର (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    ବାଛ (cif->rtype->type)
	      {
	      ସ୍ଥିତି FFI_TYPE_SINT8:
	      ସ୍ଥିତି FFI_TYPE_SINT16:
	      ସ୍ଥିତି FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (ଆକାର (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		ଭାଙ୍ଗ;
	      ସ୍ଥିତି FFI_TYPE_UINT8:
	      ସ୍ଥିତି FFI_TYPE_UINT16:
	      ସ୍ଥିତି FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (ଆକାର (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		ଭାଙ୍ଗ;
	      }
	  }
	ଅନ୍ୟଥା
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ଖାଲି);
}

ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_asm(ଶୂନ୍ୟ) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*), ଶୂନ୍ୟ *user_data, ଶୂନ୍ୟ *codeloc)
{
    ଯଦି (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        ଫେରନ୍ତୁ FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  ଯଦି (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      ଶୂନ୍ୟ (*dest)(ଶୂନ୍ୟ) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  ଅନ୍ୟଥା
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#ଯଦି __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#ଅନ୍ୟଥା
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#ଯଦି !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_asm (ଶୂନ୍ୟ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *))
{
  ଯଦି (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  closure->tramp = (ଶୂନ୍ୟ *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
ଶୂନ୍ୟ FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
		   ଶୂନ୍ୟ *user_data,
		   size_t *ଗଦା, call_context *aregs)
{
    ଶୂନ୍ୟ **avalue = alloca(cif->nargs * ଆକାର(ଶୂନ୍ୟ*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    ଅକ୍ଷର *astorage = alloca(cif->nargs * MAXCOPYARG);
    ଶୂନ୍ୟ *rvalue;
    call_builder cb;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା return_by_ref;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_ଗଦା = ଗଦା;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    ଯଦି (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    ଅନ୍ୟଥା
        rvalue = alloca(cif->rtype->size);

    ପାଇଁ (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    ଯଦି (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
ଶୂନ୍ୟ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ବାହ୍ୟ ଶୂନ୍ୟ *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  ଫେରନ୍ତୁ &trampoline_code_table;
}
#endif
