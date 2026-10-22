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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <tramp.h>

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdint.h>

#எனில் defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#அல்லதெனில் defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT தசம
#அல்லதெனில் defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT இரட்டை
#இல்லையெனில்
#error unsupported LoongArch floating-point ABI
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * அளவு (இரட்டை))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
வகைவரை கட்டமைப்பு call_context
{
#எனில் !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#endif
  size_t a[10];
} call_context;

வகைவரை கட்டமைப்பு call_builder
{
  call_context *aregs;
  முழு used_integer;
  முழு used_float;
  size_t *used_அடுக்கு;	    /* Stack pointer.  */
  size_t *அடுக்கு;	    /* Place for arguments pass by reference.  */
  size_t next_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#எனில் __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#இல்லையெனில்
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#எனில் ABI_FRLEN
வகைவரை கட்டமைப்பு float_struct_info
{
  எழுத்து as_elements;
  எழுத்து type1;
  எழுத்து offset2;
  எழுத்து type2;
} float_struct_info;

#எனில் ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#இல்லையெனில்
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

நிலையான ffi_type **
flatten_struct (ffi_type *in, ffi_type **out, ffi_type **out_end)
{
  முழு i;

  எனில் (out == out_end)
    திரும்பு out;
  எனில் (in->type != FFI_TYPE_STRUCT)
    *(out++) = in;
  இல்லையெனில்
    க்கு (i = 0; in->elements[i]; i++)
      out = flatten_struct (in->elements[i], out, out_end);
  திரும்பு out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
நிலையான float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *உச்சி)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  முழு இலக்கம்_floats, இலக்கம்_ints;
  முழு இலக்கம்_fields = flatten_struct (உச்சி, fields, fields + 3) - fields;

  எனில் (இலக்கம்_fields == 1)
    {
      எனில் (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  இல்லையெனில் எனில் (இலக்கம்_fields == 2)
    {
      இலக்கம்_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      இலக்கம்_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      எனில் (இலக்கம்_floats == 0 || இலக்கம்_floats + இலக்கம்_ints != 2)
	திரும்பு ret;
      எனில் (cb->used_float + இலக்கம்_floats > NARGREG
	  || cb->used_integer + (2 - இலக்கம்_floats) > NARGREG)
	திரும்பு ret;
      எனில் (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	திரும்பு ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->size, fields[1]->alignment);
      ret.as_elements = 1;
    }
  திரும்பு ret;
}
#endif

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
நிலையான வெறுமை
marshal_atom (call_builder *cb, முழு type, வெறுமை *data)
{
  size_t மதிப்பு = 0;
  தேர்வு (type)
    {
    நிலை FFI_TYPE_UINT8:
      மதிப்பு = *(uint8_t *) data;
      நிறுத்து;
    நிலை FFI_TYPE_SINT8:
      மதிப்பு = *(int8_t *) data;
      நிறுத்து;
    நிலை FFI_TYPE_UINT16:
      மதிப்பு = *(uint16_t *) data;
      நிறுத்து;
    நிலை FFI_TYPE_SINT16:
      மதிப்பு = *(int16_t *) data;
      நிறுத்து;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    நிலை FFI_TYPE_UINT32:
      மதிப்பு = *(int32_t *) data;
      நிறுத்து;
    நிலை FFI_TYPE_SINT32:
      மதிப்பு = *(int32_t *) data;
      நிறுத்து;
#எனில் __SIZEOF_POINTER__ == 8
    நிலை FFI_TYPE_UINT64:
      மதிப்பு = *(uint64_t *) data;
      நிறுத்து;
    நிலை FFI_TYPE_SINT64:
      மதிப்பு = *(int64_t *) data;
      நிறுத்து;
#endif
    நிலை FFI_TYPE_POINTER:
      மதிப்பு = *(size_t *) data;
      நிறுத்து;

#எனில் ABI_FRLEN >= 32
    நிலை FFI_TYPE_FLOAT:
      *(தசம *)(cb->aregs->fa + cb->used_float++) = *(தசம *) data;
      திரும்பு;
#endif
#எனில் ABI_FRLEN >= 64
    நிலை FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->used_float++]) = *(இரட்டை *) data;
      திரும்பு;
#endif
    இயல்பு:
      FFI_ASSERT (0);
      நிறுத்து;
    }

  எனில் (cb->used_integer == NARGREG)
    *cb->used_அடுக்கு++ = மதிப்பு;
  இல்லையெனில்
    cb->aregs->a[cb->used_integer++] = மதிப்பு;
}

நிலையான வெறுமை
unmarshal_atom (call_builder *cb, முழு type, வெறுமை *data)
{
  size_t மதிப்பு;
  தேர்வு (type)
    {
#எனில் ABI_FRLEN >= 32
    நிலை FFI_TYPE_FLOAT:
      *(தசம *) data = *(தசம *)(cb->aregs->fa + cb->used_float++);
      திரும்பு;
#endif
#எனில் ABI_FRLEN >= 64
    நிலை FFI_TYPE_DOUBLE:
      *(இரட்டை *) data = cb->aregs->fa[cb->used_float++];
      திரும்பு;
#endif
    }

  எனில் (cb->used_integer == NARGREG)
    மதிப்பு = *cb->used_அடுக்கு++;
  இல்லையெனில்
    மதிப்பு = cb->aregs->a[cb->used_integer++];

  தேர்வு (type)
    {
    நிலை FFI_TYPE_UINT8:
    நிலை FFI_TYPE_SINT8:
    நிலை FFI_TYPE_UINT16:
    நிலை FFI_TYPE_SINT16:
    நிலை FFI_TYPE_UINT32:
    நிலை FFI_TYPE_SINT32:
#எனில் __SIZEOF_POINTER__ == 8
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_SINT64:
#endif
    நிலை FFI_TYPE_POINTER:
      *(ffi_arg *)data = மதிப்பு;
      நிறுத்து;
    இயல்பு:
      FFI_ASSERT (0);
      நிறுத்து;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
நிலையான வெறுமை *
allocate_and_copy_struct_to_அடுக்கு (call_builder *cb, வெறுமை *data,
				   ffi_type *type)
{
  size_t dest = cb->next_struct_area - type->size;

  dest = FFI_ALIGN_DOWN (dest, type->alignment);
  cb->next_struct_area = dest;

  திரும்பு memcpy ((எழுத்து *)cb->அடுக்கு + dest, data, type->size);
}

/* Adds an argument to a call, or a not by reference return value.  */
நிலையான வெறுமை
marshal (call_builder *cb, ffi_type *type, முழு var, வெறுமை *data)
{
  size_t realign[2];

#எனில் ABI_FRLEN
  எனில் (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      எனில் (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, data);
	  எனில் (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((எழுத்து *) data) + fsi.offset2);
	  திரும்பு;
	}
    }

  எனில் (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, data);
      திரும்பு;
    }

  இரட்டை promoted;
  எனில் (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(தசம *) data;
      type = &ffi_type_double;
      data = &promoted;
    }
#endif

  எனில் (type->size > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_to_அடுக்கு (cb, data, type);
      data = (எழுத்து *)cb->அடுக்கு + cb->next_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &data);
    }
  இல்லையெனில் எனில் (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, data);
  இல்லையெனில்
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      எனில் (type->alignment > __SIZEOF_POINTER__)
	{
	  எனில் (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_அடுக்கு
	    = (size_t *) FFI_ALIGN (cb->used_அடுக்கு, 2 * __SIZEOF_POINTER__);
	}

      memcpy (realign, data, type->size);
      எனில் (type->size > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      எனில் (type->size > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
நிலையான வெறுமை *
unmarshal (call_builder *cb, ffi_type *type, முழு var, வெறுமை *data)
{
  size_t realign[2];
  வெறுமை *pointer;

#எனில் ABI_FRLEN
  எனில் (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      எனில் (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, data);
	  எனில் (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((எழுத்து *) data) + fsi.offset2);
	  திரும்பு data;
	}
    }

  எனில் (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, data);
      திரும்பு data;
    }

  எனில் (var && type->type == FFI_TYPE_FLOAT)
    {
      முழு m = cb->used_integer;
      வெறுமை *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->used_அடுக்கு + m - NARGREG + 1;
      *(தசம *) promoted = *(இரட்டை *) promoted;
    }
#endif

  எனில் (type->size > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (எழுத்து *) &pointer);
      திரும்பு pointer;
    }
  இல்லையெனில் எனில் (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, data);
      திரும்பு data;
    }
  இல்லையெனில்
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      எனில் (type->alignment > __SIZEOF_POINTER__)
	{
	  எனில் (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_அடுக்கு
	    = (size_t *) FFI_ALIGN (cb->used_அடுக்கு, 2 * __SIZEOF_POINTER__);
	}

      எனில் (type->size > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      எனில் (type->size > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      memcpy (data, realign, type->size);
      திரும்பு data;
    }
}

நிலையான முழு
passed_by_ref (call_builder *cb, ffi_type *type, முழு var)
{
#எனில் ABI_FRLEN
  எனில் (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      எனில் (fsi.as_elements)
	திரும்பு 0;
    }
#endif

  திரும்பு type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  திரும்பு FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_status
ffi_prep_cif_machdep_var (ffi_cif *cif, unsigned முழு nfixedargs,
			  unsigned முழு ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  திரும்பு FFI_OK;
}

/* Low level routine for calling functions.  */
வெளி வெறுமை ffi_call_asm (வெறுமை *அடுக்கு, கட்டமைப்பு call_context *regs,
			  வெறுமை (*fn) (வெறுமை), வெறுமை *closure) FFI_HIDDEN;

நிலையான வெறுமை
ffi_call_int (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue,
	      வெறுமை *closure)
{
  size_t arg_bytes = FFI_ALIGN (cif->bytes, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_bytes = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * அளவு(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_bytes = 0;
  எனில் (rvalue == வெற்று && cif->rtype->size > 2 * __SIZEOF_POINTER__)
    rval_bytes = FFI_ALIGN (cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + extra_bytes + rval_bytes + அளவு (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_base;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  எனில் (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_base = (size_t) alloca (alloc_size);
  இல்லையெனில்
    alloc_base = FFI_ALIGN (alloca (alloc_size + STKALIGN - 1), STKALIGN);

  எனில் (rval_bytes)
    rvalue = (வெறுமை *) (alloc_base + arg_bytes);

  call_builder cb;
  cb.used_float = cb.used_integer = 0;
  cb.aregs = (call_context *) (alloc_base + arg_bytes + extra_bytes + rval_bytes);
  cb.used_அடுக்கு = (size_t *)((எழுத்து *) alloc_base);
  cb.அடுக்கு = (size_t *)((எழுத்து *) alloc_base + extra_bytes + rval_bytes);
  cb.next_struct_area = arg_bytes;

  முழு return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  எனில் (return_by_ref)
    cb.aregs->a[cb.used_integer++] = (size_t)rvalue;

  முழு i;
  க்கு (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((வெறுமை *) alloc_base, cb.aregs, fn, closure);

  cb.used_float = cb.used_integer = 0;
  எனில் (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, வெற்று);
}

வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue,
	     வெறுமை *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

வெளி வெறுமை ffi_closure_asm (வெறுமை) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		      வெறுமை *user_data, வெறுமை *codeloc)
{
  uint32_t *tramp = (uint32_t *) &closure->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

  எனில் (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    திரும்பு FFI_BAD_ABI;

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
  எனில் (ffi_tramp_is_present(closure))
    {
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_asm, closure);
      செல் out;
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

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
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
   registers will be reloaded from aregs.  */
வெறுமை FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		   வெறுமை *user_data, size_t *அடுக்கு, call_context *aregs)
{
  வெறுமை **avalue = alloca (cif->nargs * அளவு (வெறுமை *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  எழுத்து *astorage = alloca (cif->nargs * MAXCOPYARG);
  வெறுமை *rvalue;
  call_builder cb;
  முழு return_by_ref;
  முழு i;

  cb.aregs = aregs;
  cb.used_integer = cb.used_float = 0;
  cb.used_அடுக்கு = அடுக்கு;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  எனில் (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  இல்லையெனில்
    rvalue = alloca (cif->rtype->size);

  க்கு (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_data);

  எனில் (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.used_integer = cb.used_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெறுமை *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  வெளி வெறுமை *trampoline_code_table;

  *tramp_size = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_size = 1 << 16;
  திரும்பு &trampoline_code_table;
}
#endif
