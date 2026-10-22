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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ "internal.h"
#அந்தர்பாவ <tramp.h>

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdint.h>

#எனில் __riscv_float_abi_double
#define ABI_FLEN 64
வகைவரை ஒன்றிப்பு {
    uint64_t i;
    இரட்டை d;
} float_reg;
#அல்லதெனில் __riscv_float_abi_single
#define ABI_FLEN 32
வகைவரை ஒன்றிப்பு {
    தசம f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * அளவு(இரட்டை))

வகைவரை கட்டமைப்பு call_context
{
#எனில் ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    எழுத்து frame[16];
} call_context;

வகைவரை கட்டமைப்பு call_builder
{
    call_context *aregs;
    முழு used_integer;
    முழு used_float;
    size_t *used_அடுக்கு;
    வெறுமை *struct_அடுக்கு;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#எனில் __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#இல்லையெனில்
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#எனில் ABI_FLEN
வகைவரை கட்டமைப்பு {
    எழுத்து as_elements, type1, offset2, type2;
} float_struct_info;

#எனில் ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#இல்லையெனில்
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

நிலையான ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    முழு i;
    எனில் (out == out_end) திரும்பு out;
    எனில் (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } இல்லையெனில் {
        க்கு (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    திரும்பு out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
நிலையான float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *உச்சி) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    முழு இலக்கம்_floats, இலக்கம்_ints;
    முழு இலக்கம்_fields = flatten_struct(உச்சி, fields, fields + 3) - fields;

    எனில் (இலக்கம்_fields == 1) {
        எனில் (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } இல்லையெனில் எனில் (இலக்கம்_fields == 2) {
        இலக்கம்_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        இலக்கம்_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        எனில் (இலக்கம்_floats == 0 || இலக்கம்_floats + இலக்கம்_ints != 2)
            திரும்பு ret;
        எனில் (cb->used_float + இலக்கம்_floats > NARGREG || cb->used_integer + (2 - இலக்கம்_floats) > NARGREG)
            திரும்பு ret;
        எனில் (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            திரும்பு ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    திரும்பு ret;
}

#எனில் ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
நிலையான வெறுமை marshal_float(call_builder *cb, வெறுமை *data) {
    ஒன்றிப்பு {
        uint32_t i;
        தசம f;
    } மதிப்பு;

    மதிப்பு.f = *(தசம *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | மதிப்பு.i;
}

நிலையான வெறுமை unmarshal_float(call_builder *cb, வெறுமை *data) {
    ஒன்றிப்பு {
        uint32_t i;
        தசம f;
    } மதிப்பு;

    மதிப்பு.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(தசம *)data = மதிப்பு.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
நிலையான வெறுமை marshal_atom(call_builder *cb, முழு type, வெறுமை *data) {
    size_t மதிப்பு = 0;
    தேர்வு (type) {
        நிலை FFI_TYPE_UINT8: மதிப்பு = *(uint8_t *)data; நிறுத்து;
        நிலை FFI_TYPE_SINT8: மதிப்பு = *(int8_t *)data; நிறுத்து;
        நிலை FFI_TYPE_UINT16: மதிப்பு = *(uint16_t *)data; நிறுத்து;
        நிலை FFI_TYPE_SINT16: மதிப்பு = *(int16_t *)data; நிறுத்து;
        /* 32-bit quantities are always sign-extended in the ABI */
        நிலை FFI_TYPE_UINT32: மதிப்பு = *(int32_t *)data; நிறுத்து;
        நிலை FFI_TYPE_SINT32: மதிப்பு = *(int32_t *)data; நிறுத்து;
#எனில் __SIZEOF_POINTER__ == 8
        நிலை FFI_TYPE_UINT64: மதிப்பு = *(uint64_t *)data; நிறுத்து;
        நிலை FFI_TYPE_SINT64: மதிப்பு = *(int64_t *)data; நிறுத்து;
#endif
        நிலை FFI_TYPE_POINTER: மதிப்பு = *(size_t *)data; நிறுத்து;

#எனில் ABI_FLEN >= 64
        நிலை FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            திரும்பு;
#அல்லதெனில் ABI_FLEN >= 32
        நிலை FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(தசம *)data));
            திரும்பு;
#endif
#எனில் ABI_FLEN >= 64
        நிலை FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(இரட்டை *)data));
            திரும்பு;
#endif
        இயல்பு: FFI_ASSERT(0); நிறுத்து;
    }

    எனில் (cb->used_integer == NARGREG) {
        *cb->used_அடுக்கு++ = மதிப்பு;
    } இல்லையெனில் {
        cb->aregs->a[cb->used_integer++] = மதிப்பு;
    }
}

நிலையான வெறுமை unmarshal_atom(call_builder *cb, முழு type, வெறுமை *data) {
    size_t மதிப்பு;
    தேர்வு (type) {
#எனில் ABI_FLEN >= 64
        நிலை FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            திரும்பு;
#அல்லதெனில் ABI_FLEN >= 32
        நிலை FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(தசம *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            திரும்பு;
#endif
#எனில் ABI_FLEN >= 64
        நிலை FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(இரட்டை *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            திரும்பு;
#endif
    }

    எனில் (cb->used_integer == NARGREG) {
        மதிப்பு = *cb->used_அடுக்கு++;
    } இல்லையெனில் {
        மதிப்பு = cb->aregs->a[cb->used_integer++];
    }

    தேர்வு (type) {
        நிலை FFI_TYPE_UINT8: *(uint8_t *)data = மதிப்பு; நிறுத்து;
        நிலை FFI_TYPE_SINT8: *(uint8_t *)data = மதிப்பு; நிறுத்து;
        நிலை FFI_TYPE_UINT16: *(uint16_t *)data = மதிப்பு; நிறுத்து;
        நிலை FFI_TYPE_SINT16: *(uint16_t *)data = மதிப்பு; நிறுத்து;
        நிலை FFI_TYPE_UINT32: *(uint32_t *)data = மதிப்பு; நிறுத்து;
        நிலை FFI_TYPE_SINT32: *(uint32_t *)data = மதிப்பு; நிறுத்து;
#எனில் __SIZEOF_POINTER__ == 8
        நிலை FFI_TYPE_UINT64: *(uint64_t *)data = மதிப்பு; நிறுத்து;
        நிலை FFI_TYPE_SINT64: *(uint64_t *)data = மதிப்பு; நிறுத்து;
#endif
        நிலை FFI_TYPE_POINTER: *(size_t *)data = மதிப்பு; நிறுத்து;
        இயல்பு: FFI_ASSERT(0); நிறுத்து;
    }
}

/* adds an argument to a call, or a not by reference return value */
நிலையான வெறுமை marshal(call_builder *cb, ffi_type *type, முழு var, வெறுமை *data) {
    size_t realign[2];

#எனில் ABI_FLEN
    எனில் (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        எனில் (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            எனில் (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((எழுத்து*)data) + fsi.offset2);
            திரும்பு;
        }
    }

    எனில் (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        திரும்பு;
    }
#endif

    எனில் (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = memcpy (cb->struct_அடுக்கு, data, type->size);
        cb->struct_அடுக்கு = (size_t *) FFI_ALIGN ((எழுத்து *) cb->struct_அடுக்கு + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } இல்லையெனில் எனில் (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } இல்லையெனில் {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        எனில் (type->alignment > __SIZEOF_POINTER__) {
            எனில் (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_அடுக்கு = (size_t *)FFI_ALIGN(cb->used_அடுக்கு, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, data, type->size);
        எனில் (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        எனில் (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
நிலையான வெறுமை *unmarshal(call_builder *cb, ffi_type *type, முழு var, வெறுமை *data) {
    size_t realign[2];
    வெறுமை *pointer;

#எனில் ABI_FLEN
    எனில் (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        எனில் (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            எனில் (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((எழுத்து*)data) + fsi.offset2);
            திரும்பு data;
        }
    }

    எனில் (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        திரும்பு data;
    }
#endif

    எனில் (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (எழுத்து*)&pointer);
        திரும்பு pointer;
    } இல்லையெனில் எனில் (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        திரும்பு data;
    } இல்லையெனில் {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        எனில் (type->alignment > __SIZEOF_POINTER__) {
            எனில் (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_அடுக்கு = (size_t *)FFI_ALIGN(cb->used_அடுக்கு, 2*__SIZEOF_POINTER__);
        }

        எனில் (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        எனில் (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(data, realign, type->size);
        திரும்பு data;
    }
}

நிலையான முழு passed_by_ref(call_builder *cb, ffi_type *type, முழு var) {
#எனில் ABI_FLEN
    எனில் (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        எனில் (fsi.as_elements) திரும்பு 0;
    }
#endif

    திரும்பு type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    திரும்பு FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned முழு nfixedargs, unsigned முழு ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    திரும்பு FFI_OK;
}

/* Low level routine for calling functions */
வெளி வெறுமை ffi_call_asm (வெறுமை *அடுக்கு, கட்டமைப்பு call_context *regs,
			  வெறுமை (*fn) (வெறுமை), வெறுமை *closure) FFI_HIDDEN;

நிலையான வெறுமை
ffi_call_int (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue,
	      வெறுமை *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * அளவு(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    எனில் (rvalue == வெற்று && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    size_t alloc_size = arg_bytes + rval_bytes + struct_bytes + அளவு(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    எனில் (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_size);
    } இல்லையெனில் {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    எனில் (rval_bytes)
        rvalue = (வெறுமை*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_அடுக்கு = (வெறுமை*)alloc_base;
    cb.struct_அடுக்கு = (வெறுமை *) (alloc_base + arg_bytes + rval_bytes);

    முழு return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    எனில் (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    முழு i;
    க்கு (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((வெறுமை *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    எனில் (!return_by_ref && rvalue)
      {
	எனில் (IS_INT(cif->rtype->type)
	    && cif->rtype->size < அளவு (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    தேர்வு (cif->rtype->type)
	      {
	      நிலை FFI_TYPE_SINT8:
	      நிலை FFI_TYPE_SINT16:
	      நிலை FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (அளவு (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		நிறுத்து;
	      நிலை FFI_TYPE_UINT8:
	      நிலை FFI_TYPE_UINT16:
	      நிலை FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (அளவு (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		நிறுத்து;
	      }
	  }
	இல்லையெனில்
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, வெற்று);
}

வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

வெளி வெறுமை ffi_closure_asm(வெறுமை) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*), வெறுமை *user_data, வெறுமை *codeloc)
{
    எனில் (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        திரும்பு FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  எனில் (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      வெறுமை (*dest)(வெறுமை) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  இல்லையெனில்
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#எனில் __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#இல்லையெனில்
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#எனில் !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    திரும்பு FFI_OK;
}

வெளி வெறுமை ffi_go_closure_asm (வெறுமை) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *))
{
  எனில் (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    திரும்பு FFI_BAD_ABI;

  closure->tramp = (வெறுமை *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
வெறுமை FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		   வெறுமை *user_data,
		   size_t *அடுக்கு, call_context *aregs)
{
    வெறுமை **avalue = alloca(cif->nargs * அளவு(வெறுமை*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    எழுத்து *astorage = alloca(cif->nargs * MAXCOPYARG);
    வெறுமை *rvalue;
    call_builder cb;
    முழு return_by_ref;
    முழு i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_அடுக்கு = அடுக்கு;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    எனில் (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    இல்லையெனில்
        rvalue = alloca(cif->rtype->size);

    க்கு (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    எனில் (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
வெறுமை *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  வெளி வெறுமை *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  திரும்பு &trampoline_code_table;
}
#endif
