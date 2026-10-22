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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ "internal.h"
#અન્તર્ભાવ <tramp.h>

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdint.h>

#જો __riscv_float_abi_double
#define ABI_FLEN 64
પ્રકારવ્યાખ્યા સંઘ {
    uint64_t i;
    દ્વિગુણ d;
} float_reg;
#અથવાજો __riscv_float_abi_single
#define ABI_FLEN 32
પ્રકારવ્યાખ્યા સંઘ {
    દશાંશ f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * કદ(દ્વિગુણ))

પ્રકારવ્યાખ્યા સંરચના call_context
{
#જો ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    અક્ષર frame[16];
} call_context;

પ્રકારવ્યાખ્યા સંરચના call_builder
{
    call_context *aregs;
    પૂર્ણાંક used_integer;
    પૂર્ણાંક used_float;
    size_t *used_ઢગલો;
    શૂન્ય *struct_ઢગલો;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#જો __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#નહીંતર
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#જો ABI_FLEN
પ્રકારવ્યાખ્યા સંરચના {
    અક્ષર as_elements, type1, offset2, type2;
} float_struct_info;

#જો ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#નહીંતર
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

સ્થિર ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    પૂર્ણાંક i;
    જો (out == out_end) પાછા out;
    જો (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } નહીંતર {
        માટે (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    પાછા out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
સ્થિર float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *ટોચ) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    પૂર્ણાંક અંક_floats, અંક_ints;
    પૂર્ણાંક અંક_fields = flatten_struct(ટોચ, fields, fields + 3) - fields;

    જો (અંક_fields == 1) {
        જો (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } નહીંતર જો (અંક_fields == 2) {
        અંક_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        અંક_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        જો (અંક_floats == 0 || અંક_floats + અંક_ints != 2)
            પાછા ret;
        જો (cb->used_float + અંક_floats > NARGREG || cb->used_integer + (2 - અંક_floats) > NARGREG)
            પાછા ret;
        જો (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            પાછા ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    પાછા ret;
}

#જો ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
સ્થિર શૂન્ય marshal_float(call_builder *cb, શૂન્ય *data) {
    સંઘ {
        uint32_t i;
        દશાંશ f;
    } કિંમત;

    કિંમત.f = *(દશાંશ *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | કિંમત.i;
}

સ્થિર શૂન્ય unmarshal_float(call_builder *cb, શૂન્ય *data) {
    સંઘ {
        uint32_t i;
        દશાંશ f;
    } કિંમત;

    કિંમત.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(દશાંશ *)data = કિંમત.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
સ્થિર શૂન્ય marshal_atom(call_builder *cb, પૂર્ણાંક type, શૂન્ય *data) {
    size_t કિંમત = 0;
    પસંદગી (type) {
        સ્થિતિ FFI_TYPE_UINT8: કિંમત = *(uint8_t *)data; તોડો;
        સ્થિતિ FFI_TYPE_SINT8: કિંમત = *(int8_t *)data; તોડો;
        સ્થિતિ FFI_TYPE_UINT16: કિંમત = *(uint16_t *)data; તોડો;
        સ્થિતિ FFI_TYPE_SINT16: કિંમત = *(int16_t *)data; તોડો;
        /* 32-bit quantities are always sign-extended in the ABI */
        સ્થિતિ FFI_TYPE_UINT32: કિંમત = *(int32_t *)data; તોડો;
        સ્થિતિ FFI_TYPE_SINT32: કિંમત = *(int32_t *)data; તોડો;
#જો __SIZEOF_POINTER__ == 8
        સ્થિતિ FFI_TYPE_UINT64: કિંમત = *(uint64_t *)data; તોડો;
        સ્થિતિ FFI_TYPE_SINT64: કિંમત = *(int64_t *)data; તોડો;
#endif
        સ્થિતિ FFI_TYPE_POINTER: કિંમત = *(size_t *)data; તોડો;

#જો ABI_FLEN >= 64
        સ્થિતિ FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            પાછા;
#અથવાજો ABI_FLEN >= 32
        સ્થિતિ FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(દશાંશ *)data));
            પાછા;
#endif
#જો ABI_FLEN >= 64
        સ્થિતિ FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(દ્વિગુણ *)data));
            પાછા;
#endif
        મૂળભૂત: FFI_ASSERT(0); તોડો;
    }

    જો (cb->used_integer == NARGREG) {
        *cb->used_ઢગલો++ = કિંમત;
    } નહીંતર {
        cb->aregs->a[cb->used_integer++] = કિંમત;
    }
}

સ્થિર શૂન્ય unmarshal_atom(call_builder *cb, પૂર્ણાંક type, શૂન્ય *data) {
    size_t કિંમત;
    પસંદગી (type) {
#જો ABI_FLEN >= 64
        સ્થિતિ FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            પાછા;
#અથવાજો ABI_FLEN >= 32
        સ્થિતિ FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(દશાંશ *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            પાછા;
#endif
#જો ABI_FLEN >= 64
        સ્થિતિ FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(દ્વિગુણ *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            પાછા;
#endif
    }

    જો (cb->used_integer == NARGREG) {
        કિંમત = *cb->used_ઢગલો++;
    } નહીંતર {
        કિંમત = cb->aregs->a[cb->used_integer++];
    }

    પસંદગી (type) {
        સ્થિતિ FFI_TYPE_UINT8: *(uint8_t *)data = કિંમત; તોડો;
        સ્થિતિ FFI_TYPE_SINT8: *(uint8_t *)data = કિંમત; તોડો;
        સ્થિતિ FFI_TYPE_UINT16: *(uint16_t *)data = કિંમત; તોડો;
        સ્થિતિ FFI_TYPE_SINT16: *(uint16_t *)data = કિંમત; તોડો;
        સ્થિતિ FFI_TYPE_UINT32: *(uint32_t *)data = કિંમત; તોડો;
        સ્થિતિ FFI_TYPE_SINT32: *(uint32_t *)data = કિંમત; તોડો;
#જો __SIZEOF_POINTER__ == 8
        સ્થિતિ FFI_TYPE_UINT64: *(uint64_t *)data = કિંમત; તોડો;
        સ્થિતિ FFI_TYPE_SINT64: *(uint64_t *)data = કિંમત; તોડો;
#endif
        સ્થિતિ FFI_TYPE_POINTER: *(size_t *)data = કિંમત; તોડો;
        મૂળભૂત: FFI_ASSERT(0); તોડો;
    }
}

/* adds an argument to a call, or a not by reference return value */
સ્થિર શૂન્ય marshal(call_builder *cb, ffi_type *type, પૂર્ણાંક var, શૂન્ય *data) {
    size_t realign[2];

#જો ABI_FLEN
    જો (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        જો (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            જો (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((અક્ષર*)data) + fsi.offset2);
            પાછા;
        }
    }

    જો (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        પાછા;
    }
#endif

    જો (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = memcpy (cb->struct_ઢગલો, data, type->size);
        cb->struct_ઢગલો = (size_t *) FFI_ALIGN ((અક્ષર *) cb->struct_ઢગલો + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } નહીંતર જો (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } નહીંતર {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        જો (type->alignment > __SIZEOF_POINTER__) {
            જો (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_ઢગલો = (size_t *)FFI_ALIGN(cb->used_ઢગલો, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, data, type->size);
        જો (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        જો (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
સ્થિર શૂન્ય *unmarshal(call_builder *cb, ffi_type *type, પૂર્ણાંક var, શૂન્ય *data) {
    size_t realign[2];
    શૂન્ય *pointer;

#જો ABI_FLEN
    જો (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        જો (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            જો (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((અક્ષર*)data) + fsi.offset2);
            પાછા data;
        }
    }

    જો (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        પાછા data;
    }
#endif

    જો (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (અક્ષર*)&pointer);
        પાછા pointer;
    } નહીંતર જો (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        પાછા data;
    } નહીંતર {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        જો (type->alignment > __SIZEOF_POINTER__) {
            જો (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_ઢગલો = (size_t *)FFI_ALIGN(cb->used_ઢગલો, 2*__SIZEOF_POINTER__);
        }

        જો (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        જો (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(data, realign, type->size);
        પાછા data;
    }
}

સ્થિર પૂર્ણાંક passed_by_ref(call_builder *cb, ffi_type *type, પૂર્ણાંક var) {
#જો ABI_FLEN
    જો (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        જો (fsi.as_elements) પાછા 0;
    }
#endif

    પાછા type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    પાછા FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned પૂર્ણાંક nfixedargs, unsigned પૂર્ણાંક ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    પાછા FFI_OK;
}

/* Low level routine for calling functions */
બાહ્ય શૂન્ય ffi_call_asm (શૂન્ય *ઢગલો, સંરચના call_context *regs,
			  શૂન્ય (*fn) (શૂન્ય), શૂન્ય *closure) FFI_HIDDEN;

સ્થિર શૂન્ય
ffi_call_int (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue,
	      શૂન્ય *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * કદ(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    જો (rvalue == ખાલી && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    size_t alloc_size = arg_bytes + rval_bytes + struct_bytes + કદ(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    જો (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_size);
    } નહીંતર {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    જો (rval_bytes)
        rvalue = (શૂન્ય*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_ઢગલો = (શૂન્ય*)alloc_base;
    cb.struct_ઢગલો = (શૂન્ય *) (alloc_base + arg_bytes + rval_bytes);

    પૂર્ણાંક return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    જો (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    પૂર્ણાંક i;
    માટે (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((શૂન્ય *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    જો (!return_by_ref && rvalue)
      {
	જો (IS_INT(cif->rtype->type)
	    && cif->rtype->size < કદ (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    પસંદગી (cif->rtype->type)
	      {
	      સ્થિતિ FFI_TYPE_SINT8:
	      સ્થિતિ FFI_TYPE_SINT16:
	      સ્થિતિ FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (કદ (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		તોડો;
	      સ્થિતિ FFI_TYPE_UINT8:
	      સ્થિતિ FFI_TYPE_UINT16:
	      સ્થિતિ FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (કદ (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		તોડો;
	      }
	  }
	નહીંતર
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ખાલી);
}

શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

બાહ્ય શૂન્ય ffi_closure_asm(શૂન્ય) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*), શૂન્ય *user_data, શૂન્ય *codeloc)
{
    જો (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        પાછા FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  જો (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      શૂન્ય (*dest)(શૂન્ય) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  નહીંતર
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#જો __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#નહીંતર
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#જો !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    પાછા FFI_OK;
}

બાહ્ય શૂન્ય ffi_go_closure_asm (શૂન્ય) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *))
{
  જો (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    પાછા FFI_BAD_ABI;

  closure->tramp = (શૂન્ય *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
શૂન્ય FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
		   શૂન્ય *user_data,
		   size_t *ઢગલો, call_context *aregs)
{
    શૂન્ય **avalue = alloca(cif->nargs * કદ(શૂન્ય*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    અક્ષર *astorage = alloca(cif->nargs * MAXCOPYARG);
    શૂન્ય *rvalue;
    call_builder cb;
    પૂર્ણાંક return_by_ref;
    પૂર્ણાંક i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_ઢગલો = ઢગલો;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    જો (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    નહીંતર
        rvalue = alloca(cif->rtype->size);

    માટે (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    જો (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
શૂન્ય *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  બાહ્ય શૂન્ય *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  પાછા &trampoline_code_table;
}
#endif
