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

#include <ffi.h>
#include <ffi_common.h>
#include "internal.h"
#include <tramp.h>

#include <stdlib.h>
#include <stdint.h>

#ᬬᬾᬦ᭄ __riscv_float_abi_double
#define ABI_FLEN 64
typedef sa_ngha {
    uint64_t i;
    dviguu_na d;
} float_reg;
#elif __riscv_float_abi_single
#define ABI_FLEN 32
typedef sa_ngha {
    float f;
} float_reg;
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * parimaa_na(dviguu_na))

typedef sa_nracanaa call_context
{
#ᬬᬾᬦ᭄ ABI_FLEN
    float_reg fa[8];
#endif
    size_t a[8];
    /* used by the assembly code to in-place construct its own stack frame */
    var_na frame[16];
} call_context;

typedef sa_nracanaa call_builder
{
    call_context *aregs;
    puur_naa_nka used_integer;
    puur_naa_nka used_float;
    size_t *used_stack;
    void *struct_stack;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ᬬᬾᬦ᭄ __SIZEOF_POINTER__ == 8
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ᬮᬶᬬ
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#ᬬᬾᬦ᭄ ABI_FLEN
typedef sa_nracanaa {
    var_na as_elements, type1, offset2, type2;
} float_struct_info;

#ᬬᬾᬦ᭄ ABI_FLEN >= 64
#define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#ᬮᬶᬬ
#define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

static ffi_type **flatten_struct(ffi_type *in, ffi_type **out, ffi_type **out_end) {
    puur_naa_nka i;
    ᬬᬾᬦ᭄ (out == out_end) ᬯᬮᬶ out;
    ᬬᬾᬦ᭄ (in->type != FFI_TYPE_STRUCT) {
        *(out++) = in;
    } ᬮᬶᬬ {
        for (i = 0; in->elements[i]; i++)
            out = flatten_struct(in->elements[i], out, out_end);
    }
    ᬯᬮᬶ out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available. */
static float_struct_info struct_passed_as_elements(call_builder *cb, ffi_type *top) {
    float_struct_info ret = {0, 0, 0, 0};
    ffi_type *fields[3];
    puur_naa_nka num_floats, num_ints;
    puur_naa_nka num_fields = flatten_struct(top, fields, fields + 3) - fields;

    ᬬᬾᬦ᭄ (num_fields == 1) {
        ᬬᬾᬦ᭄ (IS_FLOAT(fields[0]->type)) {
            ret.as_elements = 1;
            ret.type1 = fields[0]->type;
        }
    } ᬮᬶᬬ ᬬᬾᬦ᭄ (num_fields == 2) {
        num_floats = IS_FLOAT(fields[0]->type) + IS_FLOAT(fields[1]->type);
        num_ints = IS_INT(fields[0]->type) + IS_INT(fields[1]->type);
        ᬬᬾᬦ᭄ (num_floats == 0 || num_floats + num_ints != 2)
            ᬯᬮᬶ ret;
        ᬬᬾᬦ᭄ (cb->used_float + num_floats > NARGREG || cb->used_integer + (2 - num_floats) > NARGREG)
            ᬯᬮᬶ ret;
        ᬬᬾᬦ᭄ (!IS_FLOAT(fields[0]->type) && !IS_FLOAT(fields[1]->type))
            ᬯᬮᬶ ret;

        ret.type1 = fields[0]->type;
        ret.type2 = fields[1]->type;
        ret.offset2 = FFI_ALIGN(fields[0]->size, fields[1]->alignment);
        ret.as_elements = 1;
    }

    ᬯᬮᬶ ret;
}

#ᬬᬾᬦ᭄ ABI_FLEN >= 64
/* Float values in wider RISC-V FP registers are NaN-boxed */
static void marshal_float(call_builder *cb, void *data) {
    sa_ngha {
        uint32_t i;
        float f;
    } value;

    value.f = *(float *)data;
    cb->aregs->fa[cb->used_float++].i =
        UINT64_C(0xffffffff00000000) | value.i;
}

static void unmarshal_float(call_builder *cb, void *data) {
    sa_ngha {
        uint32_t i;
        float f;
    } value;

    value.i = (uint32_t)cb->aregs->fa[cb->used_float++].i;
    *(float *)data = value.f;
}
#endif
#endif

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
static void marshal_atom(call_builder *cb, puur_naa_nka type, void *data) {
    size_t value = 0;
    switch (type) {
        pak_sa FFI_TYPE_UINT8: value = *(uint8_t *)data; bha_nga;
        pak_sa FFI_TYPE_SINT8: value = *(int8_t *)data; bha_nga;
        pak_sa FFI_TYPE_UINT16: value = *(uint16_t *)data; bha_nga;
        pak_sa FFI_TYPE_SINT16: value = *(int16_t *)data; bha_nga;
        /* 32-bit quantities are always sign-extended in the ABI */
        pak_sa FFI_TYPE_UINT32: value = *(int32_t *)data; bha_nga;
        pak_sa FFI_TYPE_SINT32: value = *(int32_t *)data; bha_nga;
#ᬬᬾᬦ᭄ __SIZEOF_POINTER__ == 8
        pak_sa FFI_TYPE_UINT64: value = *(uint64_t *)data; bha_nga;
        pak_sa FFI_TYPE_SINT64: value = *(int64_t *)data; bha_nga;
#endif
        pak_sa FFI_TYPE_POINTER: value = *(size_t *)data; bha_nga;

#ᬬᬾᬦ᭄ ABI_FLEN >= 64
        pak_sa FFI_TYPE_FLOAT:
            marshal_float(cb, data);
            ᬯᬮᬶ;
#elif ABI_FLEN >= 32
        pak_sa FFI_TYPE_FLOAT:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].f) : "0"(*(float *)data));
            ᬯᬮᬶ;
#endif
#ᬬᬾᬦ᭄ ABI_FLEN >= 64
        pak_sa FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(cb->aregs->fa[cb->used_float++].d) : "0"(*(dviguu_na *)data));
            ᬯᬮᬶ;
#endif
        default: FFI_ASSERT(0); bha_nga;
    }

    ᬬᬾᬦ᭄ (cb->used_integer == NARGREG) {
        *cb->used_stack++ = value;
    } ᬮᬶᬬ {
        cb->aregs->a[cb->used_integer++] = value;
    }
}

static void unmarshal_atom(call_builder *cb, puur_naa_nka type, void *data) {
    size_t value;
    switch (type) {
#ᬬᬾᬦ᭄ ABI_FLEN >= 64
        pak_sa FFI_TYPE_FLOAT:
            unmarshal_float(cb, data);
            ᬯᬮᬶ;
#elif ABI_FLEN >= 32
        pak_sa FFI_TYPE_FLOAT:
            __asm__("" : "=f"(*(float *)data) : "0"(cb->aregs->fa[cb->used_float++].f));
            ᬯᬮᬶ;
#endif
#ᬬᬾᬦ᭄ ABI_FLEN >= 64
        pak_sa FFI_TYPE_DOUBLE:
            __asm__("" : "=f"(*(dviguu_na *)data) : "0"(cb->aregs->fa[cb->used_float++].d));
            ᬯᬮᬶ;
#endif
    }

    ᬬᬾᬦ᭄ (cb->used_integer == NARGREG) {
        value = *cb->used_stack++;
    } ᬮᬶᬬ {
        value = cb->aregs->a[cb->used_integer++];
    }

    switch (type) {
        pak_sa FFI_TYPE_UINT8: *(uint8_t *)data = value; bha_nga;
        pak_sa FFI_TYPE_SINT8: *(uint8_t *)data = value; bha_nga;
        pak_sa FFI_TYPE_UINT16: *(uint16_t *)data = value; bha_nga;
        pak_sa FFI_TYPE_SINT16: *(uint16_t *)data = value; bha_nga;
        pak_sa FFI_TYPE_UINT32: *(uint32_t *)data = value; bha_nga;
        pak_sa FFI_TYPE_SINT32: *(uint32_t *)data = value; bha_nga;
#ᬬᬾᬦ᭄ __SIZEOF_POINTER__ == 8
        pak_sa FFI_TYPE_UINT64: *(uint64_t *)data = value; bha_nga;
        pak_sa FFI_TYPE_SINT64: *(uint64_t *)data = value; bha_nga;
#endif
        pak_sa FFI_TYPE_POINTER: *(size_t *)data = value; bha_nga;
        default: FFI_ASSERT(0); bha_nga;
    }
}

/* adds an argument to a call, or a not by reference return value */
static void marshal(call_builder *cb, ffi_type *type, puur_naa_nka var, void *data) {
    size_t realign[2];

#ᬬᬾᬦ᭄ ABI_FLEN
    ᬬᬾᬦ᭄ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ᬬᬾᬦ᭄ (fsi.as_elements) {
            marshal_atom(cb, fsi.type1, data);
            ᬬᬾᬦ᭄ (fsi.offset2)
                marshal_atom(cb, fsi.type2, ((var_na*)data) + fsi.offset2);
            ᬯᬮᬶ;
        }
    }

    ᬬᬾᬦ᭄ (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        marshal_atom(cb, type->type, data);
        ᬯᬮᬶ;
    }
#endif

    ᬬᬾᬦ᭄ (type->size > 2 * __SIZEOF_POINTER__) {
        /* copy to stack and pass by reference */
        data = memcpy (cb->struct_stack, data, type->size);
        cb->struct_stack = (size_t *) FFI_ALIGN ((var_na *) cb->struct_stack + type->size, __SIZEOF_POINTER__);
        marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } ᬮᬶᬬ ᬬᬾᬦ᭄ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        marshal_atom(cb, type->type, data);
    } ᬮᬶᬬ {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        ᬬᬾᬦ᭄ (type->alignment > __SIZEOF_POINTER__) {
            ᬬᬾᬦ᭄ (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_stack = (size_t *)FFI_ALIGN(cb->used_stack, 2*__SIZEOF_POINTER__);
        }

        memcpy(realign, data, type->size);
        ᬬᬾᬦ᭄ (type->size > 0)
            marshal_atom(cb, FFI_TYPE_POINTER, realign);
        ᬬᬾᬦ᭄ (type->size > __SIZEOF_POINTER__)
            marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
static void *unmarshal(call_builder *cb, ffi_type *type, puur_naa_nka var, void *data) {
    size_t realign[2];
    void *pointer;

#ᬬᬾᬦ᭄ ABI_FLEN
    ᬬᬾᬦ᭄ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ᬬᬾᬦ᭄ (fsi.as_elements) {
            unmarshal_atom(cb, fsi.type1, data);
            ᬬᬾᬦ᭄ (fsi.offset2)
                unmarshal_atom(cb, fsi.type2, ((var_na*)data) + fsi.offset2);
            ᬯᬮᬶ data;
        }
    }

    ᬬᬾᬦ᭄ (!var && cb->used_float < NARGREG && IS_FLOAT(type->type)) {
        unmarshal_atom(cb, type->type, data);
        ᬯᬮᬶ data;
    }
#endif

    ᬬᬾᬦ᭄ (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (var_na*)&pointer);
        ᬯᬮᬶ pointer;
    } ᬮᬶᬬ ᬬᬾᬦ᭄ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
        unmarshal_atom(cb, type->type, data);
        ᬯᬮᬶ data;
    } ᬮᬶᬬ {
        /* overlong integers, soft-float floats, and structs without special
           float handling are treated identically from this point on */

        /* variadics are aligned even in registers */
        ᬬᬾᬦ᭄ (type->alignment > __SIZEOF_POINTER__) {
            ᬬᬾᬦ᭄ (var)
                cb->used_integer = FFI_ALIGN(cb->used_integer, 2);
            cb->used_stack = (size_t *)FFI_ALIGN(cb->used_stack, 2*__SIZEOF_POINTER__);
        }

        ᬬᬾᬦ᭄ (type->size > 0)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
        ᬬᬾᬦ᭄ (type->size > __SIZEOF_POINTER__)
            unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
        memcpy(data, realign, type->size);
        ᬯᬮᬶ data;
    }
}

static puur_naa_nka passed_by_ref(call_builder *cb, ffi_type *type, puur_naa_nka var) {
#ᬬᬾᬦ᭄ ABI_FLEN
    ᬬᬾᬦ᭄ (!var && type->type == FFI_TYPE_STRUCT) {
        float_struct_info fsi = struct_passed_as_elements(cb, type);
        ᬬᬾᬦ᭄ (fsi.as_elements) ᬯᬮᬶ 0;
    }
#endif

    ᬯᬮᬶ type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif) {
    cif->riscv_nfixedargs = cif->nargs;
    ᬯᬮᬶ FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned puur_naa_nka nfixedargs, unsigned puur_naa_nka ntotalargs) {
    cif->riscv_nfixedargs = nfixedargs;
    ᬯᬮᬶ FFI_OK;
}

/* Low level routine for calling functions */
extern void ffi_call_asm (void *stack, sa_nracanaa call_context *regs,
			  void (*fn) (void), void *closure) FFI_HIDDEN;

static void
ffi_call_int (ffi_cif *cif, void (*fn) (void), void *rvalue, void **avalue,
	      void *closure)
{
    /* this is a conservative estimate, assuming a complex return value and
       that all remaining arguments are long long / __int128 */
    size_t arg_bytes = cif->nargs <= 3 ? 0 :
        FFI_ALIGN(2 * parimaa_na(size_t) * (cif->nargs - 3), STKALIGN);
    /* Allocate space for copies of big structures.  */
    size_t struct_bytes = FFI_ALIGN (cif->bytes, STKALIGN);
    size_t rval_bytes = 0;
    ᬬᬾᬦ᭄ (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
        rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
    size_t alloc_size = arg_bytes + rval_bytes + struct_bytes + parimaa_na(call_context);

    /* the assembly code will deallocate all stack data at lower addresses
       than the argument region, so we need to allocate the frame and the
       return value after the arguments in a single allocation */
    size_t alloc_base;
    /* Argument region must be 16-byte aligned */
    ᬬᬾᬦ᭄ (_Alignof(max_align_t) >= STKALIGN) {
        /* since sizeof long double is normally 16, the compiler will
           guarantee alloca alignment to at least that much */
        alloc_base = (size_t)alloca(alloc_size);
    } ᬮᬶᬬ {
        alloc_base = FFI_ALIGN(alloca(alloc_size + STKALIGN - 1), STKALIGN);
    }

    ᬬᬾᬦ᭄ (rval_bytes)
        rvalue = (void*)(alloc_base + arg_bytes);

    call_builder cb;
    cb.used_float = cb.used_integer = 0;
    cb.aregs = (call_context*)(alloc_base + arg_bytes + rval_bytes + struct_bytes);
    cb.used_stack = (void*)alloc_base;
    cb.struct_stack = (void *) (alloc_base + arg_bytes + rval_bytes);

    puur_naa_nka return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    ᬬᬾᬦ᭄ (return_by_ref)
        marshal(&cb, &ffi_type_pointer, 0, &rvalue);

    puur_naa_nka i;
    for (i = 0; i < cif->nargs; i++)
        marshal(&cb, cif->arg_types[i], i >= cif->riscv_nfixedargs, avalue[i]);

    ffi_call_asm ((void *) alloc_base, cb.aregs, fn, closure);

    cb.used_float = cb.used_integer = 0;
    ᬬᬾᬦ᭄ (!return_by_ref && rvalue)
      {
	ᬬᬾᬦ᭄ (IS_INT(cif->rtype->type)
	    && cif->rtype->size < parimaa_na (ffi_arg))
	  {
	    /* Integer types smaller than ffi_arg need to be extended.  */
	    switch (cif->rtype->type)
	      {
	      pak_sa FFI_TYPE_SINT8:
	      pak_sa FFI_TYPE_SINT16:
	      pak_sa FFI_TYPE_SINT32:
		unmarshal_atom (&cb, (parimaa_na (ffi_arg) > 4
				      ? FFI_TYPE_SINT64 : FFI_TYPE_SINT32),
				rvalue);
		bha_nga;
	      pak_sa FFI_TYPE_UINT8:
	      pak_sa FFI_TYPE_UINT16:
	      pak_sa FFI_TYPE_UINT32:
		unmarshal_atom (&cb, (parimaa_na (ffi_arg) > 4
				      ? FFI_TYPE_UINT64 : FFI_TYPE_UINT32),
				rvalue);
		bha_nga;
	      }
	  }
	ᬮᬶᬬ
	  unmarshal(&cb, cif->rtype, 0, rvalue);
      }
}

void
ffi_call (ffi_cif *cif, void (*fn) (void), void *rvalue, void **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, NULL);
}

void
ffi_call_go (ffi_cif *cif, void (*fn) (void), void *rvalue,
	     void **avalue, void *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

extern void ffi_closure_asm(void) FFI_HIDDEN;

ffi_status ffi_prep_closure_loc(ffi_closure *closure, ffi_cif *cif, void (*fun)(ffi_cif*,void*,void**,void*), void *user_data, void *codeloc)
{
    ᬬᬾᬦ᭄ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
        ᬯᬮᬶ FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  ᬬᬾᬦ᭄ (ffi_tramp_is_present (closure))
    {
      /* Initialize the static trampoline's parameters. */
      void (*dest)(void) = ffi_closure_asm;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  ᬮᬶᬬ
#endif
    {
      uint32_t *tramp = (uint32_t *) &closure->tramp[0];
      uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

      /* we will call ffi_closure_inner with codeloc, not closure, but as long
	 as the memory is readable it should work */

      tramp[0] = 0x00000317; /* auipc t1, 0 (i.e. t0 <- codeloc) */
#ᬬᬾᬦ᭄ __SIZEOF_POINTER__ == 8
      tramp[1] = 0x01033383; /* ld t2, 16(t1) */
#ᬮᬶᬬ
      tramp[1] = 0x01032383; /* lw t2, 16(t1) */
#endif
      tramp[2] = 0x00038067; /* jr t2 */
      tramp[3] = 0x00000013; /* nop */
      tramp[4] = fn;
      tramp[5] = fn >> 32;
#ᬬᬾᬦ᭄ !defined(__FreeBSD__)
    __builtin___clear_cache(codeloc, codeloc + FFI_TRAMPOLINE_SIZE);
#endif
    }

    closure->cif = cif;
    closure->fun = fun;
    closure->user_data = user_data;

    ᬯᬮᬶ FFI_OK;
}

extern void ffi_go_closure_asm (void) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     void (*fun) (ffi_cif *, void *, void **, void *))
{
  ᬬᬾᬦ᭄ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ᬯᬮᬶ FFI_BAD_ABI;

  closure->tramp = (void *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ᬯᬮᬶ FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs. */
void FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   void (*fun) (ffi_cif *, void *, void **, void *),
		   void *user_data,
		   size_t *stack, call_context *aregs)
{
    void **avalue = alloca(cif->nargs * parimaa_na(void*));
    /* storage for arguments which will be copied by unmarshal().  We could
       theoretically avoid the copies in many cases and use at most 128 bytes
       of memory, but allocating disjoint storage for each argument is
       simpler. */
    var_na *astorage = alloca(cif->nargs * MAXCOPYARG);
    void *rvalue;
    call_builder cb;
    puur_naa_nka return_by_ref;
    puur_naa_nka i;

    cb.aregs = aregs;
    cb.used_integer = cb.used_float = 0;
    cb.used_stack = stack;

    return_by_ref = passed_by_ref(&cb, cif->rtype, 0);
    ᬬᬾᬦ᭄ (return_by_ref)
        unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    ᬮᬶᬬ
        rvalue = alloca(cif->rtype->size);

    for (i = 0; i < cif->nargs; i++)
        avalue[i] = unmarshal(&cb, cif->arg_types[i],
            i >= cif->riscv_nfixedargs, astorage + i*MAXCOPYARG);

    fun (cif, rvalue, avalue, user_data);

    ᬬᬾᬦ᭄ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = cb.used_float = 0;
        marshal(&cb, cif->rtype, 0, rvalue);
    }
}

#ifdef FFI_EXEC_STATIC_TRAMP
void *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  extern void *trampoline_code_table;
  *tramp_size = RISCV_TRAMP_SIZE;
  *map_size = RISCV_TRAMP_MAP_SIZE;
  ᬯᬮᬶ &trampoline_code_table;
}
#endif
