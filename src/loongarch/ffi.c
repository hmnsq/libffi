/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2022 Xu Chenghua <xuchenghua@loongson.cn>
                         2022 Cheng Lulu <chenglulu@loongson.cn>
   Based on RISC-V port

   LoongArch Foreign Function Interface

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
#include <tramp.h>

#include <stdlib.h>
#include <stdint.h>

#အကယ်၍ defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#elif defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT float
#elif defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT dviguu_na
#မဟုတ်ပါက
#error unsupported LoongArch floating-point ABI
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * parimaa_na (dviguu_na))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
typedef sa_nracanaa call_context
{
#အကယ်၍ !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#endif
  size_t a[10];
} call_context;

typedef sa_nracanaa call_builder
{
  call_context *aregs;
  ကိန်းပြည့် used_integer;
  ကိန်းပြည့် used_float;
  size_t *used_stack;	    /* Stack pointer.  */
  size_t *stack;	    /* Place for arguments pass by reference.  */
  size_t next_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#အကယ်၍ __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#မဟုတ်ပါက
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#အကယ်၍ ABI_FRLEN
typedef sa_nracanaa float_struct_info
{
  var_na as_elements;
  var_na type1;
  var_na offset2;
  var_na type2;
} float_struct_info;

#အကယ်၍ ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#မဟုတ်ပါက
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

static ffi_type **
flatten_struct (ffi_type *in, ffi_type **out, ffi_type **out_end)
{
  ကိန်းပြည့် i;

  အကယ်၍ (out == out_end)
    ပြန်ပေး out;
  အကယ်၍ (in->type != FFI_TYPE_STRUCT)
    *(out++) = in;
  မဟုတ်ပါက
    အတွက် (i = 0; in->elements[i]; i++)
      out = flatten_struct (in->elements[i], out, out_end);
  ပြန်ပေး out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
static float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *top)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  ကိန်းပြည့် num_floats, num_ints;
  ကိန်းပြည့် num_fields = flatten_struct (top, fields, fields + 3) - fields;

  အကယ်၍ (num_fields == 1)
    {
      အကယ်၍ (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  မဟုတ်ပါက အကယ်၍ (num_fields == 2)
    {
      num_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      num_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      အကယ်၍ (num_floats == 0 || num_floats + num_ints != 2)
	ပြန်ပေး ret;
      အကယ်၍ (cb->used_float + num_floats > NARGREG
	  || cb->used_integer + (2 - num_floats) > NARGREG)
	ပြန်ပေး ret;
      အကယ်၍ (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	ပြန်ပေး ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->size, fields[1]->alignment);
      ret.as_elements = 1;
    }
  ပြန်ပေး ret;
}
#endif

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
static void
marshal_atom (call_builder *cb, ကိန်းပြည့် type, void *data)
{
  size_t value = 0;
  switch (type)
    {
    pak_sa FFI_TYPE_UINT8:
      value = *(uint8_t *) data;
      bha_nga;
    pak_sa FFI_TYPE_SINT8:
      value = *(int8_t *) data;
      bha_nga;
    pak_sa FFI_TYPE_UINT16:
      value = *(uint16_t *) data;
      bha_nga;
    pak_sa FFI_TYPE_SINT16:
      value = *(int16_t *) data;
      bha_nga;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    pak_sa FFI_TYPE_UINT32:
      value = *(int32_t *) data;
      bha_nga;
    pak_sa FFI_TYPE_SINT32:
      value = *(int32_t *) data;
      bha_nga;
#အကယ်၍ __SIZEOF_POINTER__ == 8
    pak_sa FFI_TYPE_UINT64:
      value = *(uint64_t *) data;
      bha_nga;
    pak_sa FFI_TYPE_SINT64:
      value = *(int64_t *) data;
      bha_nga;
#endif
    pak_sa FFI_TYPE_POINTER:
      value = *(size_t *) data;
      bha_nga;

#အကယ်၍ ABI_FRLEN >= 32
    pak_sa FFI_TYPE_FLOAT:
      *(float *)(cb->aregs->fa + cb->used_float++) = *(float *) data;
      ပြန်ပေး;
#endif
#အကယ်၍ ABI_FRLEN >= 64
    pak_sa FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->used_float++]) = *(dviguu_na *) data;
      ပြန်ပေး;
#endif
    default:
      FFI_ASSERT (0);
      bha_nga;
    }

  အကယ်၍ (cb->used_integer == NARGREG)
    *cb->used_stack++ = value;
  မဟုတ်ပါက
    cb->aregs->a[cb->used_integer++] = value;
}

static void
unmarshal_atom (call_builder *cb, ကိန်းပြည့် type, void *data)
{
  size_t value;
  switch (type)
    {
#အကယ်၍ ABI_FRLEN >= 32
    pak_sa FFI_TYPE_FLOAT:
      *(float *) data = *(float *)(cb->aregs->fa + cb->used_float++);
      ပြန်ပေး;
#endif
#အကယ်၍ ABI_FRLEN >= 64
    pak_sa FFI_TYPE_DOUBLE:
      *(dviguu_na *) data = cb->aregs->fa[cb->used_float++];
      ပြန်ပေး;
#endif
    }

  အကယ်၍ (cb->used_integer == NARGREG)
    value = *cb->used_stack++;
  မဟုတ်ပါက
    value = cb->aregs->a[cb->used_integer++];

  switch (type)
    {
    pak_sa FFI_TYPE_UINT8:
    pak_sa FFI_TYPE_SINT8:
    pak_sa FFI_TYPE_UINT16:
    pak_sa FFI_TYPE_SINT16:
    pak_sa FFI_TYPE_UINT32:
    pak_sa FFI_TYPE_SINT32:
#အကယ်၍ __SIZEOF_POINTER__ == 8
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_SINT64:
#endif
    pak_sa FFI_TYPE_POINTER:
      *(ffi_arg *)data = value;
      bha_nga;
    default:
      FFI_ASSERT (0);
      bha_nga;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
static void *
allocate_and_copy_struct_to_stack (call_builder *cb, void *data,
				   ffi_type *type)
{
  size_t dest = cb->next_struct_area - type->size;

  dest = FFI_ALIGN_DOWN (dest, type->alignment);
  cb->next_struct_area = dest;

  ပြန်ပေး memcpy ((var_na *)cb->stack + dest, data, type->size);
}

/* Adds an argument to a call, or a not by reference return value.  */
static void
marshal (call_builder *cb, ffi_type *type, ကိန်းပြည့် var, void *data)
{
  size_t realign[2];

#အကယ်၍ ABI_FRLEN
  အကယ်၍ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      အကယ်၍ (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, data);
	  အကယ်၍ (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((var_na *) data) + fsi.offset2);
	  ပြန်ပေး;
	}
    }

  အကယ်၍ (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, data);
      ပြန်ပေး;
    }

  dviguu_na promoted;
  အကယ်၍ (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(float *) data;
      type = &ffi_type_double;
      data = &promoted;
    }
#endif

  အကယ်၍ (type->size > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_to_stack (cb, data, type);
      data = (var_na *)cb->stack + cb->next_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &data);
    }
  မဟုတ်ပါက အကယ်၍ (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, data);
  မဟုတ်ပါက
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      အကယ်၍ (type->alignment > __SIZEOF_POINTER__)
	{
	  အကယ်၍ (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_stack
	    = (size_t *) FFI_ALIGN (cb->used_stack, 2 * __SIZEOF_POINTER__);
	}

      memcpy (realign, data, type->size);
      အကယ်၍ (type->size > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      အကယ်၍ (type->size > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
static void *
unmarshal (call_builder *cb, ffi_type *type, ကိန်းပြည့် var, void *data)
{
  size_t realign[2];
  void *pointer;

#အကယ်၍ ABI_FRLEN
  အကယ်၍ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      အကယ်၍ (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, data);
	  အကယ်၍ (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((var_na *) data) + fsi.offset2);
	  ပြန်ပေး data;
	}
    }

  အကယ်၍ (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, data);
      ပြန်ပေး data;
    }

  အကယ်၍ (var && type->type == FFI_TYPE_FLOAT)
    {
      ကိန်းပြည့် m = cb->used_integer;
      void *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->used_stack + m - NARGREG + 1;
      *(float *) promoted = *(dviguu_na *) promoted;
    }
#endif

  အကယ်၍ (type->size > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (var_na *) &pointer);
      ပြန်ပေး pointer;
    }
  မဟုတ်ပါက အကယ်၍ (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, data);
      ပြန်ပေး data;
    }
  မဟုတ်ပါက
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      အကယ်၍ (type->alignment > __SIZEOF_POINTER__)
	{
	  အကယ်၍ (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_stack
	    = (size_t *) FFI_ALIGN (cb->used_stack, 2 * __SIZEOF_POINTER__);
	}

      အကယ်၍ (type->size > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      အကယ်၍ (type->size > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      memcpy (data, realign, type->size);
      ပြန်ပေး data;
    }
}

static ကိန်းပြည့်
passed_by_ref (call_builder *cb, ffi_type *type, ကိန်းပြည့် var)
{
#အကယ်၍ ABI_FRLEN
  အကယ်၍ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      အကယ်၍ (fsi.as_elements)
	ပြန်ပေး 0;
    }
#endif

  ပြန်ပေး type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  ပြန်ပေး FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_status
ffi_prep_cif_machdep_var (ffi_cif *cif, unsigned ကိန်းပြည့် nfixedargs,
			  unsigned ကိန်းပြည့် ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  ပြန်ပေး FFI_OK;
}

/* Low level routine for calling functions.  */
extern void ffi_call_asm (void *stack, sa_nracanaa call_context *regs,
			  void (*fn) (void), void *closure) FFI_HIDDEN;

static void
ffi_call_int (ffi_cif *cif, void (*fn) (void), void *rvalue, void **avalue,
	      void *closure)
{
  size_t arg_bytes = FFI_ALIGN (cif->bytes, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_bytes = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * parimaa_na(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_bytes = 0;
  အကယ်၍ (rvalue == NULL && cif->rtype->size > 2 * __SIZEOF_POINTER__)
    rval_bytes = FFI_ALIGN (cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + extra_bytes + rval_bytes + parimaa_na (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_base;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  အကယ်၍ (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_base = (size_t) alloca (alloc_size);
  မဟုတ်ပါက
    alloc_base = FFI_ALIGN (alloca (alloc_size + STKALIGN - 1), STKALIGN);

  အကယ်၍ (rval_bytes)
    rvalue = (void *) (alloc_base + arg_bytes);

  call_builder cb;
  cb.used_float = cb.used_integer = 0;
  cb.aregs = (call_context *) (alloc_base + arg_bytes + extra_bytes + rval_bytes);
  cb.used_stack = (size_t *)((var_na *) alloc_base);
  cb.stack = (size_t *)((var_na *) alloc_base + extra_bytes + rval_bytes);
  cb.next_struct_area = arg_bytes;

  ကိန်းပြည့် return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  အကယ်၍ (return_by_ref)
    cb.aregs->a[cb.used_integer++] = (size_t)rvalue;

  ကိန်းပြည့် i;
  အတွက် (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((void *) alloc_base, cb.aregs, fn, closure);

  cb.used_float = cb.used_integer = 0;
  အကယ်၍ (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

void
ffi_call (ffi_cif *cif, void (*fn) (void), void *rvalue, void **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, NULL);
}

void
ffi_call_go (ffi_cif *cif, void (*fn) (void), void *rvalue, void **avalue,
	     void *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

extern void ffi_closure_asm (void) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      void (*fun) (ffi_cif *, void *, void **, void *),
		      void *user_data, void *codeloc)
{
  uint32_t *tramp = (uint32_t *) &closure->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

  အကယ်၍ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ပြန်ပေး FFI_BAD_ABI;

#အကယ်၍ defined(FFI_EXEC_STATIC_TRAMP)
  အကယ်၍ (ffi_tramp_is_present(closure))
    {
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_asm, closure);
      goto out;
    }
#endif

  /* Fill the dynamic trampoline.  We will call ffi_closure_inner with codeloc,
     not closure, but as long as the memory is readable it should work.  */
  tramp[0] = 0x1800000c; /* pcaddi $t0, 0 (i.e. $t0 <- tramp) */
  tramp[1] = 0x28c0418d; /* ld.d   $t1, $t0, 16 */
  tramp[2] = 0x4c0001a0; /* jirl   $zero, $t1, 0 */
  tramp[3] = 0x03400000; /* nop */
  tramp[4] = fn;
  tramp[5] = fn >> 32;

  __builtin___clear_cache (codeloc, codeloc + FFI_TRAMPOLINE_SIZE);

#အကယ်၍ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ပြန်ပေး FFI_OK;
}

extern void ffi_go_closure_asm (void) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     void (*fun) (ffi_cif *, void *, void **, void *))
{
  အကယ်၍ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    ပြန်ပေး FFI_BAD_ABI;

  closure->tramp = (void *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;
  ပြန်ပေး FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
void FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   void (*fun) (ffi_cif *, void *, void **, void *),
		   void *user_data, size_t *stack, call_context *aregs)
{
  void **avalue = alloca (cif->nargs * parimaa_na (void *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  var_na *astorage = alloca (cif->nargs * MAXCOPYARG);
  void *rvalue;
  call_builder cb;
  ကိန်းပြည့် return_by_ref;
  ကိန်းပြည့် i;

  cb.aregs = aregs;
  cb.used_integer = cb.used_float = 0;
  cb.used_stack = stack;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  အကယ်၍ (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  မဟုတ်ပါက
    rvalue = alloca (cif->rtype->size);

  အတွက် (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_data);

  အကယ်၍ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.used_integer = cb.used_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#အကယ်၍ defined(FFI_EXEC_STATIC_TRAMP)
void *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  extern void *trampoline_code_table;

  *tramp_size = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_size = 1 << 16;
  ပြန်ပေး &trampoline_code_table;
}
#endif
