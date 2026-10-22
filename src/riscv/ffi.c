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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>
#অন্তর্ভাৱ "internal.h"
#অন্তর্ভাৱ <tramp.h>

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdint.h>

#যদি __riscv_float_abi_double
#define ABI_FLEN 64
প্রকারনির্দেশ সঙ্ঘ {
    uint64_t i;
    দ্ৱিগুণ d;
} float_reg;
#নোচেদ্যদি __riscv_float_abi_single
#define ABI_FLEN 32
প্রকারনির্দেশ সঙ্ঘ {
    ভিন্ন f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * পরিমাণ(দ্ৱিগুণ))

প্রকারনির্দেশ সংরচনা call_context
{
#যদি ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    ৱর্ণ frame[16];
} call_context;

প্রকারনির্দেশ সংরচনা call_builder
{
    call_context *aregs;
    পূর্ণাঙ্ক used_integer;
    পূর্ণাঙ্ক used_float;
    size_t *used_স্তূপ;
    শূন্য *struct_স্তূপ;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#যদি __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#অন্যথা
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#যদি ABI_FLEN
প্রকারনির্দেশ সংরচনা {
    ৱর্ণ as_elements, type1, offset2, type2;
} float_struct_info;

#যদি ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#অন্যথা
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

স্থির ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    পূর্ণাঙ্ক i;
    যদি (out == out_end) ঘূৰাই out;
    যদি (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } অন্যথা {
        বাবে (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    ঘূৰাই out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
স্থির float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *শীৰ্ষ) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    পূর্ণাঙ্ক অঙ্ক_floats, অঙ্ক_ints;
    পূর্ণাঙ্ক অঙ্ক_fields = flatten_struct(শীৰ্ষ, fields, fields + 3) - fields;

    যদি (অঙ্ক_fields == 1) {
        যদি (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } অন্যথা যদি (অঙ্ক_fields == 2) {
        অঙ্ক_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        অঙ্ক_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        যদি (অঙ্ক_floats == 0 || অঙ্ক_floats + অঙ্ক_ints != 2)
            ঘূৰাই ret;
        যদি (cb->used_float + অঙ্ক_floats > NARGREG || cb->used_integer + (2 - অঙ্ক_floats) > NARGREG)
            ঘূৰাই ret;
        যদি (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            ঘূৰাই ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    ঘূৰাই ret;
}

#যদি ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
স্থির শূন্য marshal_float(call_builder *cb, শূন্য *data) {
    সঙ্ঘ {
        uint32_t i;
        ভিন্ন f;
    } মান;

    মান.f = *(ভিন্ন *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | মান.i;
}

স্থির শূন্য unmarshal_float(call_builder *cb, শূন্য *data) {
    সঙ্ঘ {
        uint32_t i;
        ভিন্ন f;
    } মান;

    মান.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(ভিন্ন *)data = মান.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
স্থির শূন্য marshal_atom(call_builder *cb, পূর্ণাঙ্ক type, শূন্য *data) {
    size_t মান = 0;
    ৱিকল্প (type) {
        পক্ষ FFI_TYPE_UINT8: মান = *(uint8_t *)data; ভঙ্গ;
        পক্ষ FFI_TYPE_SINT8: মান = *(int8_t *)data; ভঙ্গ;
        পক্ষ FFI_TYPE_UINT16: মান = *(uint16_t *)data; ভঙ্গ;
        পক্ষ FFI_TYPE_SINT16: মান = *(int16_t *)data; ভঙ্গ;
        /* 32-bit quantities are always sign-extended in the ABI */
        পক্ষ FFI_TYPE_UINT32: মান = *(int32_t *)data; ভঙ্গ;
        পক্ষ FFI_TYPE_SINT32: মান = *(int32_t *)data; ভঙ্গ;
#যদি __SIZEOF_POINTER__ == 8
        পক্ষ FFI_TYPE_UINT64: মান = *(uint64_t *)data; ভঙ্গ;
        পক্ষ FFI_TYPE_SINT64: মান = *(int64_t *)data; ভঙ্গ;
#endif
        পক্ষ FFI_TYPE_POINTER: মান = *(size_t *)data; ভঙ্গ;

#যদি ABI_FLEN >= 64
        পক্ষ FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            ঘূৰাই;
#নোচেদ্যদি ABI_FLEN >= 32
        পক্ষ FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(ভিন্ন *)data));
            ঘূৰাই;
#endif
#যদি ABI_FLEN >= 64
        পক্ষ FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(দ্ৱিগুণ *)data));
            ঘূৰাই;
#endif
        স্ৱভাৱ: FFI_ASSERT(0); ভঙ্গ;
    }

    যদি (cb->used_integer == NARGREG) {
        *cb->used_স্তূপ++ = মান;
    } অন্যথা {
        cb->aregs->a[cb->used_integer++] = মান;
    }
}

স্থির শূন্য unmarshal_atom(call_builder *cb, পূর্ণাঙ্ক type, শূন্য *data) {
    size_t মান;
    ৱিকল্প (type) {
#যদি ABI_FLEN >= 64
        পক্ষ FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            ঘূৰাই;
#নোচেদ্যদি ABI_FLEN >= 32
        পক্ষ FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(ভিন্ন *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            ঘূৰাই;
#endif
#যদি ABI_FLEN >= 64
        পক্ষ FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(দ্ৱিগুণ *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            ঘূৰাই;
#endif
    }

    যদি (cb->used_integer == NARGREG) {
        মান = *cb->used_স্তূপ++;
    } অন্যথা {
        মান = cb->aregs->a[cb->used_integer++];
    }

    ৱিকল্প (type) {
        পক্ষ FFI_TYPE_UINT8: *(uint8_t *)data = মান; ভঙ্গ;
        পক্ষ FFI_TYPE_SINT8: *(uint8_t *)data = মান; ভঙ্গ;
        পক্ষ FFI_TYPE_UINT16: *(uint16_t *)data = মান; ভঙ্গ;
        পক্ষ FFI_TYPE_SINT16: *(uint16_t *)data = মান; ভঙ্গ;
        পক্ষ FFI_TYPE_UINT32: *(uint32_t *)data = মান; ভঙ্গ;
        পক্ষ FFI_TYPE_SINT32: *(uint32_t *)data = মান; ভঙ্গ;
#যদি __SIZEOF_POINTER__ == 8
        পক্ষ FFI_TYPE_UINT64: *(uint64_t *)data = মান; ভঙ্গ;
        পক্ষ FFI_TYPE_SINT64: *(uint64_t *)data = মান; ভঙ্গ;
#endif
        পক্ষ FFI_TYPE_POINTER: *(size_t *)data = মান; ভঙ্গ;
        স্ৱভাৱ: FFI_ASSERT(0); ভঙ্গ;
    }
}

/* adds an argument to a call, or a not by reference return value */
স্থির শূন্য marshal(call_builder *cb, ffi_type *type, পূর্ণাঙ্ক var, শূন্য *data) {
    size_t realign[2];

#যদি ABI_FLEN
    যদি (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        যদি (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            যদি (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((ৱর্ণ*)data) + fsi.offset2);
            ঘূৰাই;
        }
    }

    যদি (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        ঘূৰাই;
    }
#endif

    যদি (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = memcpy (cb->struct_স্তূপ, data, type->size);
        cb->struct_স্তূপ = (size_t *) FFI_ALIGN ((ৱর্ণ *) cb->struct_স্তূপ + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } অন্যথা যদি (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } অন্যথা {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        যদি (type->alignment > __SIZEOF_POINTER__) {
            যদি (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_স্তূপ = (size_t *)FFI_ALIGN(cb->used_স্তূপ, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, data, type->size);
        যদি (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        যদি (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
স্থির শূন্য *unmarshal(call_builder *cb, ffi_type *type, পূর্ণাঙ্ক var, শূন্য *data) {
    size_t realign[2];
    শূন্য *pointer;

#যদি ABI_FLEN
    যদি (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        যদি (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            যদি (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((ৱর্ণ*)data) + fsi.offset2);
            ঘূৰাই data;
        }
    }

    যদি (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        ঘূৰাই data;
    }
#endif

    যদি (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (ৱর্ণ*)&pointer);
        ঘূৰাই pointer;
    } অন্যথা যদি (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        ঘূৰাই data;
    } অন্যথা {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        যদি (type->alignment > __SIZEOF_POINTER__) {
            যদি (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_স্তূপ = (size_t *)FFI_ALIGN(cb->used_স্তূপ, 2*__SIZEOF_POINTER__);
        }

        যদি (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        যদি (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(data, realign, type->size);
        ঘূৰাই data;
    }
}

স্থির পূর্ণাঙ্ক passed_by_ref(call_builder *cb, ffi_type *type, পূর্ণাঙ্ক var) {
#যদি ABI_FLEN
    যদি (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        যদি (fsi.as_elements) ঘূৰাই 0;
    }
#endif

    ঘূৰাই type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    ঘূৰাই FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned পূর্ণাঙ্ক nfixedargs, unsigned পূর্ণাঙ্ক ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    ঘূৰাই FFI_OK;
}

/* Low level routine for calling functions */
বাহ্য শূন্য ffi_call_asm (শূন্য *স্তূপ, সংরচনা call_context *regs,
			  শূন্য (*fn) (শূন্য), শূন্য *closure) FFI_HIDDEN;

স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue,
	      শূন্য *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * পরিমাণ(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    যদি (rvalue == রিক্ত && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    size_t alloc_size = arg_bytes + rval_bytes + struct_bytes + পরিমাণ(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    যদি (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_size);
    } অন্যথা {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    যদি (rval_bytes)
        rvalue = (শূন্য*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_স্তূপ = (শূন্য*)alloc_base;
    cb.struct_স্তূপ = (শূন্য *) (alloc_base + arg_bytes + rval_bytes);

    পূর্ণাঙ্ক return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    যদি (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    পূর্ণাঙ্ক i;
    বাবে (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((শূন্য *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    যদি (!return_by_ref && rvalue)
      {
	যদি (IS_INT(cif->rtype->type)
	    && cif->rtype->size < পরিমাণ (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    ৱিকল্প (cif->rtype->type)
	      {
	      পক্ষ FFI_TYPE_SINT8:
	      পক্ষ FFI_TYPE_SINT16:
	      পক্ষ FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (পরিমাণ (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		ভঙ্গ;
	      পক্ষ FFI_TYPE_UINT8:
	      পক্ষ FFI_TYPE_UINT16:
	      পক্ষ FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (পরিমাণ (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		ভঙ্গ;
	      }
	  }
	অন্যথা
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, রিক্ত);
}

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

বাহ্য শূন্য ffi_closure_asm(শূন্য) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*), শূন্য *user_data, শূন্য *codeloc)
{
    যদি (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        ঘূৰাই FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  যদি (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      শূন্য (*dest)(শূন্য) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  অন্যথা
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#যদি __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#অন্যথা
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#যদি !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    ঘূৰাই FFI_OK;
}

বাহ্য শূন্য ffi_go_closure_asm (শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *))
{
  যদি (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ঘূৰাই FFI_BAD_ABI;

  closure->tramp = (শূন্য *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ঘূৰাই FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
শূন্য FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		   শূন্য *user_data,
		   size_t *স্তূপ, call_context *aregs)
{
    শূন্য **avalue = alloca(cif->nargs * পরিমাণ(শূন্য*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    ৱর্ণ *astorage = alloca(cif->nargs * MAXCOPYARG);
    শূন্য *rvalue;
    call_builder cb;
    পূর্ণাঙ্ক return_by_ref;
    পূর্ণাঙ্ক i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_স্তূপ = স্তূপ;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    যদি (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    অন্যথা
        rvalue = alloca(cif->rtype->size);

    বাবে (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    যদি (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
শূন্য *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  বাহ্য শূন্য *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  ঘূৰাই &trampoline_code_table;
}
#endif
