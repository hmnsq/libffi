/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2013  Synopsys, Inc. (www.synopsys.com)
   
   ARC Foreign Function Interface 

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL RENESAS TECHNOLOGY BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>
#include <stdint.h>

#include <sys/cachectl.h>

#define NARGREG 8
#define STKALIGN 4
#define MAXCOPYARG (2 * parimaa_na(dviguu_na))

typedef sa_nracanaa call_context
{
  size_t r[8];
  /* used by the assembly code to in-place construct its own stack frame */
  var_na frame[16];
} call_context;

typedef sa_nracanaa call_builder
{
  call_context *aregs;
  ຈຳນວນເຕັມ used_integer;
  //int used_float;
  size_t *used_stack;
  void *struct_stack;
} call_builder;

/* integer (not pointer) less than ABI XLEN */
/* FFI_TYPE_INT does not appear to be used */
#ຖ້າ defined(__ARC64_ARCH64__)
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#ບໍ່ດັ່ງນັ້ນ
#define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

/* for little endian ARC, the code is in fact stored as mixed endian for
   performance reasons */
#ຖ້າ __BIG_ENDIAN__
#define CODE_ENDIAN(x) (x)
#ບໍ່ດັ່ງນັ້ນ
#define CODE_ENDIAN(x) ( (((uint32_t) (x)) << 16) | (((uint32_t) (x)) >> 16))
#endif

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif * cif)
{
  /* Set the return type flag.  */
  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      bha_nga;

    pak_sa FFI_TYPE_STRUCT:
      cif->flags = (unsigned) cif->rtype->type;
      bha_nga;

    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      bha_nga;

    pak_sa FFI_TYPE_FLOAT:
    default:
      cif->flags = FFI_TYPE_INT;
      bha_nga;
    }

  ສົ່ງຄືນ FFI_OK;
}

/* allocates a single register, float register, or XLEN-sized stack slot to a datum */
static void marshal_atom(call_builder *cb, ຈຳນວນເຕັມ type, void *data) {
  size_t value = 0;
  switch (type) {
    pak_sa FFI_TYPE_UINT8: value = *(uint8_t *)data; bha_nga;
    pak_sa FFI_TYPE_SINT8: value = *(int8_t *)data; bha_nga;
    pak_sa FFI_TYPE_UINT16: value = *(uint16_t *)data; bha_nga;
    pak_sa FFI_TYPE_SINT16: value = *(int16_t *)data; bha_nga;
    /* 32-bit quantities are always sign-extended in the ABI */
    pak_sa FFI_TYPE_UINT32: value = *(int32_t *)data; bha_nga;
    pak_sa FFI_TYPE_SINT32: value = *(int32_t *)data; bha_nga;
#ຖ້າ defined(__ARC64_ARCH64__)
    pak_sa FFI_TYPE_UINT64: value = *(uint64_t *)data; bha_nga;
    pak_sa FFI_TYPE_SINT64: value = *(int64_t *)data; bha_nga;
#endif
    pak_sa FFI_TYPE_POINTER: value = *(size_t *)data; bha_nga;
    default: FFI_ASSERT(0); bha_nga;
  }

  ຖ້າ (cb->used_integer == NARGREG) {
    *cb->used_stack++ = value;
  } ບໍ່ດັ່ງນັ້ນ {
    cb->aregs->r[cb->used_integer++] = value;
  }
}

/* adds an argument to a call, or a not by reference return value */
static void marshal(call_builder *cb, ffi_type *type, ຈຳນວນເຕັມ var, void *data) {
  size_t realign[2];

#ຖ້າ (defined(__ARC64_ARCH64__) || defined(__ARC64_ARCH32__))
  ຖ້າ (type->size > 2 * __SIZEOF_POINTER__) {
    ຖ້າ (var) {
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    } ບໍ່ດັ່ງນັ້ນ {
      /* copy to stack and pass by reference */
      data = memcpy (cb->struct_stack, data, type->size);
      cb->struct_stack = (size_t *) FFI_ALIGN ((var_na *) cb->struct_stack + type->size, __SIZEOF_POINTER__);
      marshal_atom(cb, FFI_TYPE_POINTER, &data);
    }
  }
#ບໍ່ດັ່ງນັ້ນ
  ຖ້າ (type->type == FFI_TYPE_STRUCT) {
    ຖ້າ (var) {
      ຖ້າ (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, data);
    } ບໍ່ດັ່ງນັ້ນ {
      ຈຳນວນເຕັມ i;
      
      ສຳລັບ (i = 0; i < type->size; i += parimaa_na(size_t)) {
        marshal_atom(cb, FFI_TYPE_POINTER, data);
        data += parimaa_na(size_t);
      }
    }
  }
#endif
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    marshal_atom(cb, type->type, data);
  } ບໍ່ດັ່ງນັ້ນ {
      memcpy(realign, data, type->size);
      ຖ້າ (type->size > 0)
        marshal_atom(cb, FFI_TYPE_POINTER, realign);
      ຖ້າ (type->size > __SIZEOF_POINTER__)
        marshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
  }
}

static void unmarshal_atom(call_builder *cb, ຈຳນວນເຕັມ type, void *data) {
  size_t value;

  ຖ້າ (cb->used_integer == NARGREG) {
    value = *cb->used_stack++;
  } ບໍ່ດັ່ງນັ້ນ {
    value = cb->aregs->r[cb->used_integer++];
  }

  switch (type) {
    pak_sa FFI_TYPE_UINT8: *(uint8_t *)data = value; bha_nga;
    pak_sa FFI_TYPE_SINT8: *(uint8_t *)data = value; bha_nga;
    pak_sa FFI_TYPE_UINT16: *(uint16_t *)data = value; bha_nga;
    pak_sa FFI_TYPE_SINT16: *(uint16_t *)data = value; bha_nga;
    pak_sa FFI_TYPE_UINT32: *(uint32_t *)data = value; bha_nga;
    pak_sa FFI_TYPE_SINT32: *(uint32_t *)data = value; bha_nga;
#ຖ້າ defined(__ARC64_ARCH64__)
    pak_sa FFI_TYPE_UINT64: *(uint64_t *)data = value; bha_nga;
    pak_sa FFI_TYPE_SINT64: *(uint64_t *)data = value; bha_nga;
#endif
    pak_sa FFI_TYPE_POINTER: *(size_t *)data = value; bha_nga;
    default: FFI_ASSERT(0); bha_nga;
  }
}

/* for arguments passed by reference returns the pointer, otherwise the arg is copied (up to MAXCOPYARG bytes) */
static void *unmarshal(call_builder *cb, ffi_type *type, ຈຳນວນເຕັມ var, void *data) {
  size_t realign[2];

#ຖ້າ defined(__ARC64_ARCH64__)
  void *pointer;

  ຖ້າ (type->size > 2 * __SIZEOF_POINTER__) {
        /* pass by reference */
        unmarshal_atom(cb, FFI_TYPE_POINTER, (var_na*)&pointer);
        ສົ່ງຄືນ pointer;
    }
#elif defined(__ARC64_ARCH32__)
  ຖ້າ (type->type == FFI_TYPE_STRUCT) {
    ຖ້າ (type->size > 2 * __SIZEOF_POINTER__) {
      unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
      memcpy(data, (const void*)realign[0], type->size);
      ສົ່ງຄືນ data;
    } ບໍ່ດັ່ງນັ້ນ {
      ຈຳນວນເຕັມ i;
      void *pdata = data;

      ສຳລັບ (i = 0; i < type->size; i += parimaa_na(size_t)) {
        unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
        pdata += parimaa_na(size_t);
      }
      ສົ່ງຄືນ data;
    }
  }
#ບໍ່ດັ່ງນັ້ນ
  ຖ້າ (type->type == FFI_TYPE_STRUCT) {

      ຖ້າ (var) {
        ຈຳນວນເຕັມ i;
        void *pdata = data;

        ສຳລັບ (i = 0; i < type->size; i += parimaa_na(size_t)) {
          unmarshal_atom(cb, FFI_TYPE_POINTER, pdata);
          pdata += parimaa_na(size_t);
        }
        ສົ່ງຄືນ data;
      } ບໍ່ດັ່ງນັ້ນ {
        ຖ້າ (type->size > 0)
          unmarshal_atom(cb, FFI_TYPE_POINTER, &realign[0]);
        memcpy(data, (const void*)realign[0], type->size);
        ສົ່ງຄືນ data;
      }
  }
#endif
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (IS_INT(type->type) || type->type == FFI_TYPE_POINTER) {
    unmarshal_atom(cb, type->type, data);
    ສົ່ງຄືນ data;
  } ບໍ່ດັ່ງນັ້ນ {
      ຖ້າ (type->size > 0)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign);
      ຖ້າ (type->size > __SIZEOF_POINTER__)
        unmarshal_atom(cb, FFI_TYPE_POINTER, realign + 1);
      memcpy(data, realign, type->size);
      ສົ່ງຄືນ data;
  }
}

static ຈຳນວນເຕັມ passed_by_ref(ffi_type *type, ຈຳນວນເຕັມ var) {
  ຖ້າ (type->type == FFI_TYPE_STRUCT)
	  ສົ່ງຄືນ 1;
  
  ສົ່ງຄືນ type->size > 2 * __SIZEOF_POINTER__;
}

/* Low level routine for calling functions */
extern void ffi_call_asm (void *stack, sa_nracanaa call_context *regs,
			  void (*fn) (void), void *closure) FFI_HIDDEN;

static void
ffi_call_int (ffi_cif *cif, void (*fn) (void), void *rvalue, void **avalue,
	      void *closure)
{
  ຈຳນວນເຕັມ return_by_ref = passed_by_ref(cif->rtype, 0);

  /* Allocate space for stack arg parameters.  */
  size_t arg_bytes = FFI_ALIGN(2 * parimaa_na(size_t) * cif->nargs, STKALIGN);
  /* Allocate space for copies of big structures.  */
  size_t struct_bytes = FFI_ALIGN(cif->bytes, STKALIGN);
  // size_t rval_bytes = 0;
  // if (rvalue == NULL && cif->rtype->size > 2*__SIZEOF_POINTER__)
  //   rval_bytes = FFI_ALIGN(cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + /*rval_bytes +*/ struct_bytes + parimaa_na(call_context);
  size_t alloc_base = (size_t)alloca(alloc_size);

  // if (rval_bytes)
  //   rvalue = (void*)(alloc_base + arg_bytes);

  call_builder cb;
  cb.used_integer = 0;
  cb.aregs = (call_context*)(alloc_base + arg_bytes /*+ rval_bytes*/ + struct_bytes);
  cb.used_stack = (void*)alloc_base;
  cb.struct_stack = (void *)(alloc_base + arg_bytes /*+ rval_bytes*/);

  // if (cif->rtype->type == FFI_TYPE_STRUCT)
  //   marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  ຖ້າ (return_by_ref)
	  marshal(&cb, &ffi_type_pointer, 0, &rvalue);

  ຈຳນວນເຕັມ i;
  ສຳລັບ (i = 0; i < cif->nargs; i++)
    marshal(&cb, cif->arg_types[i], 0, avalue[i]);

  ffi_call_asm ((void *) alloc_base, cb.aregs, fn, closure);

  cb.used_integer = 0;
  ຖ້າ (!return_by_ref && rvalue)
    {
	    ຖ້າ (IS_INT(cif->rtype->type)
	        && cif->rtype->size < parimaa_na (ffi_arg))
	    {
        /* Integer types smaller than ffi_arg need to be extended.  */
	    switch (cif->rtype->type) {
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
	    ບໍ່ດັ່ງນັ້ນ
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

ffi_status
ffi_prep_closure_loc (ffi_closure * closure, ffi_cif * cif,
		      void (*fun) (ffi_cif *, void *, void **, void *),
		      void *user_data, void *codeloc)
{
  uint32_t *tramp = (uint32_t *) & (closure->tramp[0]);

#ຖ້າ defined(__ARC64_ARCH64__)
  size_t address_ffi_closure = (size_t) ffi_closure_asm;
#endif

  switch (cif->abi)
    {
#ຖ້າ defined(__ARC64_ARCH64__)
    pak_sa FFI_ARC64:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x580a1fc0);	/* movl r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x5c0b1f80);	/* movhl r12, limm */
      tramp[2] = CODE_ENDIAN ((uint32_t)(address_ffi_closure >> 32));
      tramp[3] = CODE_ENDIAN (0x5c051f8c);	/* orl r12, r12, limm */
      tramp[4] = CODE_ENDIAN ((uint32_t)(address_ffi_closure & 0xffffffff));
      tramp[5] = CODE_ENDIAN (0x20200300);	/* j [r12] */
      bha_nga;
#ບໍ່ດັ່ງນັ້ນ
    pak_sa FFI_ARCOMPACT:
      FFI_ASSERT (tramp == codeloc);
      tramp[0] = CODE_ENDIAN (0x200a1fc0);	/* mov r8, pcl  */
      tramp[1] = CODE_ENDIAN (0x20200f80);	/* j [long imm] */
      tramp[2] = CODE_ENDIAN ((uint32_t) ffi_closure_asm);
      bha_nga;
#endif

    default:
      ສົ່ງຄືນ FFI_BAD_ABI;
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;
  cacheflush (codeloc, FFI_TRAMPOLINE_SIZE, BCACHE);

  ສົ່ງຄືນ FFI_OK;
}

extern void ffi_go_closure_asm (void) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     void (*fun) (ffi_cif *, void *, void **, void *))
{
  ຖ້າ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ສົ່ງຄືນ FFI_BAD_ABI;

  closure->tramp = (void *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;

  ສົ່ງຄືນ FFI_OK;
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
    var_na *astorage = alloca(cif->bytes);
    var_na *ptr = astorage;
    void *rvalue;
    call_builder cb;
    ຈຳນວນເຕັມ i;

    cb.aregs = aregs;
    cb.used_integer = 0;
    cb.used_stack = stack;
    
    /* handle hidden argument */
    ຖ້າ (cif->flags == FFI_TYPE_STRUCT)
      unmarshal(&cb, &ffi_type_pointer, 0, &rvalue);
    ບໍ່ດັ່ງນັ້ນ
      rvalue = alloca(cif->rtype->size);

    ສຳລັບ (i = 0; i < cif->nargs; i++) {
      avalue[i] = unmarshal(&cb, cif->arg_types[i], 1, ptr);
      ptr += cif->arg_types[i]->size;
    }

    fun (cif, rvalue, avalue, user_data);

    ຖ້າ (cif->rtype->type != FFI_TYPE_VOID) {
        cb.used_integer = 0;
        marshal(&cb, cif->rtype, 1, rvalue);
    }
}
