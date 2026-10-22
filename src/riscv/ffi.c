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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ "internal.h"
#അന്തര്ഭാവ <tramp.h>

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdint.h>

#എങ്കിൽ __riscv_float_abi_double
#define ABI_FLEN 64
തരനിർവചനം സംഘം {
    uint64_t i;
    ഇരട്ട d;
} float_reg;
#അല്ലെങ്കിലെങ്കിൽ __riscv_float_abi_single
#define ABI_FLEN 32
തരനിർവചനം സംഘം {
    ദശാംശം f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * വലുപ്പം(ഇരട്ട))

തരനിർവചനം ഘടന call_context
{
#എങ്കിൽ ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    അക്ഷരം frame[16];
} call_context;

തരനിർവചനം ഘടന call_builder
{
    call_context *aregs;
    പൂർണ്ണസംഖ്യ used_integer;
    പൂർണ്ണസംഖ്യ used_float;
    size_t *used_അട്ടി;
    ശൂന്യം *struct_അട്ടി;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#എങ്കിൽ __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#അല്ലെങ്കിൽ
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#എങ്കിൽ ABI_FLEN
തരനിർവചനം ഘടന {
    അക്ഷരം as_elements, type1, offset2, type2;
} float_struct_info;

#എങ്കിൽ ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#അല്ലെങ്കിൽ
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

സ്ഥിതം ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    പൂർണ്ണസംഖ്യ i;
    എങ്കിൽ (out == out_end) തിരികെ out;
    എങ്കിൽ (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } അല്ലെങ്കിൽ {
        വേണ്ടി (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    തിരികെ out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
സ്ഥിതം float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *മുകൾ) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    പൂർണ്ണസംഖ്യ അക്കം_floats, അക്കം_ints;
    പൂർണ്ണസംഖ്യ അക്കം_fields = flatten_struct(മുകൾ, fields, fields + 3) - fields;

    എങ്കിൽ (അക്കം_fields == 1) {
        എങ്കിൽ (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } അല്ലെങ്കിൽ എങ്കിൽ (അക്കം_fields == 2) {
        അക്കം_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        അക്കം_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        എങ്കിൽ (അക്കം_floats == 0 || അക്കം_floats + അക്കം_ints != 2)
            തിരികെ ret;
        എങ്കിൽ (cb->used_float + അക്കം_floats > NARGREG || cb->used_integer + (2 - അക്കം_floats) > NARGREG)
            തിരികെ ret;
        എങ്കിൽ (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            തിരികെ ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    തിരികെ ret;
}

#എങ്കിൽ ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
സ്ഥിതം ശൂന്യം marshal_float(call_builder *cb, ശൂന്യം *data) {
    സംഘം {
        uint32_t i;
        ദശാംശം f;
    } മൂല്യം;

    മൂല്യം.f = *(ദശാംശം *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | മൂല്യം.i;
}

സ്ഥിതം ശൂന്യം unmarshal_float(call_builder *cb, ശൂന്യം *data) {
    സംഘം {
        uint32_t i;
        ദശാംശം f;
    } മൂല്യം;

    മൂല്യം.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(ദശാംശം *)data = മൂല്യം.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
സ്ഥിതം ശൂന്യം marshal_atom(call_builder *cb, പൂർണ്ണസംഖ്യ type, ശൂന്യം *data) {
    size_t മൂല്യം = 0;
    തിരഞ്ഞെടുപ്പ് (type) {
        അവസ്ഥ FFI_TYPE_UINT8: മൂല്യം = *(uint8_t *)data; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_SINT8: മൂല്യം = *(int8_t *)data; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_UINT16: മൂല്യം = *(uint16_t *)data; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_SINT16: മൂല്യം = *(int16_t *)data; നിർത്തുക;
        /* 32-bit quantities are always sign-extended in the ABI */
        അവസ്ഥ FFI_TYPE_UINT32: മൂല്യം = *(int32_t *)data; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_SINT32: മൂല്യം = *(int32_t *)data; നിർത്തുക;
#എങ്കിൽ __SIZEOF_POINTER__ == 8
        അവസ്ഥ FFI_TYPE_UINT64: മൂല്യം = *(uint64_t *)data; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_SINT64: മൂല്യം = *(int64_t *)data; നിർത്തുക;
#endif
        അവസ്ഥ FFI_TYPE_POINTER: മൂല്യം = *(size_t *)data; നിർത്തുക;

#എങ്കിൽ ABI_FLEN >= 64
        അവസ്ഥ FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            തിരികെ;
#അല്ലെങ്കിലെങ്കിൽ ABI_FLEN >= 32
        അവസ്ഥ FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(ദശാംശം *)data));
            തിരികെ;
#endif
#എങ്കിൽ ABI_FLEN >= 64
        അവസ്ഥ FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(ഇരട്ട *)data));
            തിരികെ;
#endif
        സ്ഥിരസ്ഥിതി: FFI_ASSERT(0); നിർത്തുക;
    }

    എങ്കിൽ (cb->used_integer == NARGREG) {
        *cb->used_അട്ടി++ = മൂല്യം;
    } അല്ലെങ്കിൽ {
        cb->aregs->a[cb->used_integer++] = മൂല്യം;
    }
}

സ്ഥിതം ശൂന്യം unmarshal_atom(call_builder *cb, പൂർണ്ണസംഖ്യ type, ശൂന്യം *data) {
    size_t മൂല്യം;
    തിരഞ്ഞെടുപ്പ് (type) {
#എങ്കിൽ ABI_FLEN >= 64
        അവസ്ഥ FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            തിരികെ;
#അല്ലെങ്കിലെങ്കിൽ ABI_FLEN >= 32
        അവസ്ഥ FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(ദശാംശം *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            തിരികെ;
#endif
#എങ്കിൽ ABI_FLEN >= 64
        അവസ്ഥ FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(ഇരട്ട *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            തിരികെ;
#endif
    }

    എങ്കിൽ (cb->used_integer == NARGREG) {
        മൂല്യം = *cb->used_അട്ടി++;
    } അല്ലെങ്കിൽ {
        മൂല്യം = cb->aregs->a[cb->used_integer++];
    }

    തിരഞ്ഞെടുപ്പ് (type) {
        അവസ്ഥ FFI_TYPE_UINT8: *(uint8_t *)data = മൂല്യം; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_SINT8: *(uint8_t *)data = മൂല്യം; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_UINT16: *(uint16_t *)data = മൂല്യം; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_SINT16: *(uint16_t *)data = മൂല്യം; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_UINT32: *(uint32_t *)data = മൂല്യം; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_SINT32: *(uint32_t *)data = മൂല്യം; നിർത്തുക;
#എങ്കിൽ __SIZEOF_POINTER__ == 8
        അവസ്ഥ FFI_TYPE_UINT64: *(uint64_t *)data = മൂല്യം; നിർത്തുക;
        അവസ്ഥ FFI_TYPE_SINT64: *(uint64_t *)data = മൂല്യം; നിർത്തുക;
#endif
        അവസ്ഥ FFI_TYPE_POINTER: *(size_t *)data = മൂല്യം; നിർത്തുക;
        സ്ഥിരസ്ഥിതി: FFI_ASSERT(0); നിർത്തുക;
    }
}

/* adds an argument to a call, or a not by reference return value */
സ്ഥിതം ശൂന്യം marshal(call_builder *cb, ffi_type *type, പൂർണ്ണസംഖ്യ var, ശൂന്യം *data) {
    size_t realign[2];

#എങ്കിൽ ABI_FLEN
    എങ്കിൽ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        എങ്കിൽ (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            എങ്കിൽ (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((അക്ഷരം*)data) + fsi.offset2);
            തിരികെ;
        }
    }

    എങ്കിൽ (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        തിരികെ;
    }
#endif

    എങ്കിൽ (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = memcpy (cb->struct_അട്ടി, data, type->size);
        cb->struct_അട്ടി = (size_t *) FFI_ALIGN ((അക്ഷരം *) cb->struct_അട്ടി + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } അല്ലെങ്കിൽ എങ്കിൽ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } അല്ലെങ്കിൽ {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        എങ്കിൽ (type->alignment > __SIZEOF_POINTER__) {
            എങ്കിൽ (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_അട്ടി = (size_t *)FFI_ALIGN(cb->used_അട്ടി, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, data, type->size);
        എങ്കിൽ (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        എങ്കിൽ (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
സ്ഥിതം ശൂന്യം *unmarshal(call_builder *cb, ffi_type *type, പൂർണ്ണസംഖ്യ var, ശൂന്യം *data) {
    size_t realign[2];
    ശൂന്യം *pointer;

#എങ്കിൽ ABI_FLEN
    എങ്കിൽ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        എങ്കിൽ (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            എങ്കിൽ (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((അക്ഷരം*)data) + fsi.offset2);
            തിരികെ data;
        }
    }

    എങ്കിൽ (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        തിരികെ data;
    }
#endif

    എങ്കിൽ (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (അക്ഷരം*)&pointer);
        തിരികെ pointer;
    } അല്ലെങ്കിൽ എങ്കിൽ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        തിരികെ data;
    } അല്ലെങ്കിൽ {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        എങ്കിൽ (type->alignment > __SIZEOF_POINTER__) {
            എങ്കിൽ (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_അട്ടി = (size_t *)FFI_ALIGN(cb->used_അട്ടി, 2*__SIZEOF_POINTER__);
        }

        എങ്കിൽ (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        എങ്കിൽ (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(data, realign, type->size);
        തിരികെ data;
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ passed_by_ref(call_builder *cb, ffi_type *type, പൂർണ്ണസംഖ്യ var) {
#എങ്കിൽ ABI_FLEN
    എങ്കിൽ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        എങ്കിൽ (fsi.as_elements) തിരികെ 0;
    }
#endif

    തിരികെ type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    തിരികെ FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned പൂർണ്ണസംഖ്യ nfixedargs, unsigned പൂർണ്ണസംഖ്യ ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    തിരികെ FFI_OK;
}

/* Low level routine for calling functions */
ബാഹ്യ ശൂന്യം ffi_call_asm (ശൂന്യം *അട്ടി, ഘടന call_context *regs,
			  ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *closure) FFI_HIDDEN;

സ്ഥിതം ശൂന്യം
ffi_call_int (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue,
	      ശൂന്യം *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * വലുപ്പം(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    എങ്കിൽ (rvalue == ശൂന്യ && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    size_t alloc_size = arg_bytes + rval_bytes + struct_bytes + വലുപ്പം(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    എങ്കിൽ (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_size);
    } അല്ലെങ്കിൽ {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    എങ്കിൽ (rval_bytes)
        rvalue = (ശൂന്യം*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_അട്ടി = (ശൂന്യം*)alloc_base;
    cb.struct_അട്ടി = (ശൂന്യം *) (alloc_base + arg_bytes + rval_bytes);

    പൂർണ്ണസംഖ്യ return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    എങ്കിൽ (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((ശൂന്യം *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    എങ്കിൽ (!return_by_ref && rvalue)
      {
	എങ്കിൽ (IS_INT(cif->rtype->type)
	    && cif->rtype->size < വലുപ്പം (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
	      {
	      അവസ്ഥ FFI_TYPE_SINT8:
	      അവസ്ഥ FFI_TYPE_SINT16:
	      അവസ്ഥ FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (വലുപ്പം (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		നിർത്തുക;
	      അവസ്ഥ FFI_TYPE_UINT8:
	      അവസ്ഥ FFI_TYPE_UINT16:
	      അവസ്ഥ FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (വലുപ്പം (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		നിർത്തുക;
	      }
	  }
	അല്ലെങ്കിൽ
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ശൂന്യ);
}

ശൂന്യം
ffi_call_go (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue,
	     ശൂന്യം **avalue, ശൂന്യം *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ബാഹ്യ ശൂന്യം ffi_closure_asm(ശൂന്യം) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*), ശൂന്യം *user_data, ശൂന്യം *codeloc)
{
    എങ്കിൽ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        തിരികെ FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  എങ്കിൽ (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      ശൂന്യം (*dest)(ശൂന്യം) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  അല്ലെങ്കിൽ
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#എങ്കിൽ __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#അല്ലെങ്കിൽ
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#എങ്കിൽ !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    തിരികെ FFI_OK;
}

ബാഹ്യ ശൂന്യം ffi_go_closure_asm (ശൂന്യം) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *))
{
  എങ്കിൽ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    തിരികെ FFI_BAD_ABI;

  closure->tramp = (ശൂന്യം *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  തിരികെ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
ശൂന്യം FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
		   ശൂന്യം *user_data,
		   size_t *അട്ടി, call_context *aregs)
{
    ശൂന്യം **avalue = alloca(cif->nargs * വലുപ്പം(ശൂന്യം*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    അക്ഷരം *astorage = alloca(cif->nargs * MAXCOPYARG);
    ശൂന്യം *rvalue;
    call_builder cb;
    പൂർണ്ണസംഖ്യ return_by_ref;
    പൂർണ്ണസംഖ്യ i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_അട്ടി = അട്ടി;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    എങ്കിൽ (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    അല്ലെങ്കിൽ
        rvalue = alloca(cif->rtype->size);

    വേണ്ടി (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    എങ്കിൽ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
ശൂന്യം *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ബാഹ്യ ശൂന്യം *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  തിരികെ &trampoline_code_table;
}
#endif
