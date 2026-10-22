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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ <tramp.h>

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdint.h>

#જો defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#અથવાજો defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT દશાંશ
#અથવાજો defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT દ્વિગુણ
#નહીંતર
#error unsupported LoongArch floating-point ABI
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * કદ (દ્વિગુણ))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
પ્રકારવ્યાખ્યા સંરચના call_context
{
#જો !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#endif
  size_t a[10];
} call_context;

પ્રકારવ્યાખ્યા સંરચના call_builder
{
  call_context *aregs;
  પૂર્ણાંક used_integer;
  પૂર્ણાંક used_float;
  size_t *used_ઢગલો;	    /* Stack pointer.  */
  size_t *ઢગલો;	    /* Place for arguments pass by reference.  */
  size_t next_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#જો __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#નહીંતર
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#જો ABI_FRLEN
પ્રકારવ્યાખ્યા સંરચના float_struct_info
{
  અક્ષર as_elements;
  અક્ષર type1;
  અક્ષર offset2;
  અક્ષર type2;
} float_struct_info;

#જો ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#નહીંતર
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

સ્થિર ffi_type **
flatten_struct (ffi_type *in, ffi_type **out, ffi_type **out_end)
{
  પૂર્ણાંક i;

  જો (out == out_end)
    પાછા out;
  જો (in->type != FFI_TYPE_STRUCT)
    *(out++) = in;
  નહીંતર
    માટે (i = 0; in->elements[i]; i++)
      out = flatten_struct (in->elements[i], out, out_end);
  પાછા out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
સ્થિર float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *ટોચ)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  પૂર્ણાંક અંક_floats, અંક_ints;
  પૂર્ણાંક અંક_fields = flatten_struct (ટોચ, fields, fields + 3) - fields;

  જો (અંક_fields == 1)
    {
      જો (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  નહીંતર જો (અંક_fields == 2)
    {
      અંક_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      અંક_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      જો (અંક_floats == 0 || અંક_floats + અંક_ints != 2)
	પાછા ret;
      જો (cb->used_float + અંક_floats > NARGREG
	  || cb->used_integer + (2 - અંક_floats) > NARGREG)
	પાછા ret;
      જો (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	પાછા ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->size, fields[1]->alignment);
      ret.as_elements = 1;
    }
  પાછા ret;
}
#endif

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
સ્થિર શૂન્ય
marshal_atom (call_builder *cb, પૂર્ણાંક type, શૂન્ય *data)
{
  size_t કિંમત = 0;
  પસંદગી (type)
    {
    સ્થિતિ FFI_TYPE_UINT8:
      કિંમત = *(uint8_t *) data;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT8:
      કિંમત = *(int8_t *) data;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT16:
      કિંમત = *(uint16_t *) data;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT16:
      કિંમત = *(int16_t *) data;
      તોડો;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    સ્થિતિ FFI_TYPE_UINT32:
      કિંમત = *(int32_t *) data;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT32:
      કિંમત = *(int32_t *) data;
      તોડો;
#જો __SIZEOF_POINTER__ == 8
    સ્થિતિ FFI_TYPE_UINT64:
      કિંમત = *(uint64_t *) data;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT64:
      કિંમત = *(int64_t *) data;
      તોડો;
#endif
    સ્થિતિ FFI_TYPE_POINTER:
      કિંમત = *(size_t *) data;
      તોડો;

#જો ABI_FRLEN >= 32
    સ્થિતિ FFI_TYPE_FLOAT:
      *(દશાંશ *)(cb->aregs->fa + cb->used_float++) = *(દશાંશ *) data;
      પાછા;
#endif
#જો ABI_FRLEN >= 64
    સ્થિતિ FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->used_float++]) = *(દ્વિગુણ *) data;
      પાછા;
#endif
    મૂળભૂત:
      FFI_ASSERT (0);
      તોડો;
    }

  જો (cb->used_integer == NARGREG)
    *cb->used_ઢગલો++ = કિંમત;
  નહીંતર
    cb->aregs->a[cb->used_integer++] = કિંમત;
}

સ્થિર શૂન્ય
unmarshal_atom (call_builder *cb, પૂર્ણાંક type, શૂન્ય *data)
{
  size_t કિંમત;
  પસંદગી (type)
    {
#જો ABI_FRLEN >= 32
    સ્થિતિ FFI_TYPE_FLOAT:
      *(દશાંશ *) data = *(દશાંશ *)(cb->aregs->fa + cb->used_float++);
      પાછા;
#endif
#જો ABI_FRLEN >= 64
    સ્થિતિ FFI_TYPE_DOUBLE:
      *(દ્વિગુણ *) data = cb->aregs->fa[cb->used_float++];
      પાછા;
#endif
    }

  જો (cb->used_integer == NARGREG)
    કિંમત = *cb->used_ઢગલો++;
  નહીંતર
    કિંમત = cb->aregs->a[cb->used_integer++];

  પસંદગી (type)
    {
    સ્થિતિ FFI_TYPE_UINT8:
    સ્થિતિ FFI_TYPE_SINT8:
    સ્થિતિ FFI_TYPE_UINT16:
    સ્થિતિ FFI_TYPE_SINT16:
    સ્થિતિ FFI_TYPE_UINT32:
    સ્થિતિ FFI_TYPE_SINT32:
#જો __SIZEOF_POINTER__ == 8
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_SINT64:
#endif
    સ્થિતિ FFI_TYPE_POINTER:
      *(ffi_arg *)data = કિંમત;
      તોડો;
    મૂળભૂત:
      FFI_ASSERT (0);
      તોડો;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
સ્થિર શૂન્ય *
allocate_and_copy_struct_to_ઢગલો (call_builder *cb, શૂન્ય *data,
				   ffi_type *type)
{
  size_t dest = cb->next_struct_area - type->size;

  dest = FFI_ALIGN_DOWN (dest, type->alignment);
  cb->next_struct_area = dest;

  પાછા memcpy ((અક્ષર *)cb->ઢગલો + dest, data, type->size);
}

/* Adds an argument to a call, or a not by reference return value.  */
સ્થિર શૂન્ય
marshal (call_builder *cb, ffi_type *type, પૂર્ણાંક var, શૂન્ય *data)
{
  size_t realign[2];

#જો ABI_FRLEN
  જો (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      જો (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, data);
	  જો (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((અક્ષર *) data) + fsi.offset2);
	  પાછા;
	}
    }

  જો (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, data);
      પાછા;
    }

  દ્વિગુણ promoted;
  જો (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(દશાંશ *) data;
      type = &ffi_type_double;
      data = &promoted;
    }
#endif

  જો (type->size > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_to_ઢગલો (cb, data, type);
      data = (અક્ષર *)cb->ઢગલો + cb->next_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &data);
    }
  નહીંતર જો (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, data);
  નહીંતર
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      જો (type->alignment > __SIZEOF_POINTER__)
	{
	  જો (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_ઢગલો
	    = (size_t *) FFI_ALIGN (cb->used_ઢગલો, 2 * __SIZEOF_POINTER__);
	}

      memcpy (realign, data, type->size);
      જો (type->size > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      જો (type->size > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
સ્થિર શૂન્ય *
unmarshal (call_builder *cb, ffi_type *type, પૂર્ણાંક var, શૂન્ય *data)
{
  size_t realign[2];
  શૂન્ય *pointer;

#જો ABI_FRLEN
  જો (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      જો (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, data);
	  જો (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((અક્ષર *) data) + fsi.offset2);
	  પાછા data;
	}
    }

  જો (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, data);
      પાછા data;
    }

  જો (var && type->type == FFI_TYPE_FLOAT)
    {
      પૂર્ણાંક m = cb->used_integer;
      શૂન્ય *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->used_ઢગલો + m - NARGREG + 1;
      *(દશાંશ *) promoted = *(દ્વિગુણ *) promoted;
    }
#endif

  જો (type->size > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (અક્ષર *) &pointer);
      પાછા pointer;
    }
  નહીંતર જો (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, data);
      પાછા data;
    }
  નહીંતર
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      જો (type->alignment > __SIZEOF_POINTER__)
	{
	  જો (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_ઢગલો
	    = (size_t *) FFI_ALIGN (cb->used_ઢગલો, 2 * __SIZEOF_POINTER__);
	}

      જો (type->size > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      જો (type->size > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      memcpy (data, realign, type->size);
      પાછા data;
    }
}

સ્થિર પૂર્ણાંક
passed_by_ref (call_builder *cb, ffi_type *type, પૂર્ણાંક var)
{
#જો ABI_FRLEN
  જો (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      જો (fsi.as_elements)
	પાછા 0;
    }
#endif

  પાછા type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  પાછા FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_status
ffi_prep_cif_machdep_var (ffi_cif *cif, unsigned પૂર્ણાંક nfixedargs,
			  unsigned પૂર્ણાંક ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  પાછા FFI_OK;
}

/* Low level routine for calling functions.  */
બાહ્ય શૂન્ય ffi_call_asm (શૂન્ય *ઢગલો, સંરચના call_context *regs,
			  શૂન્ય (*fn) (શૂન્ય), શૂન્ય *closure) FFI_HIDDEN;

સ્થિર શૂન્ય
ffi_call_int (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue,
	      શૂન્ય *closure)
{
  size_t arg_bytes = FFI_ALIGN (cif->bytes, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_bytes = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * કદ(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_bytes = 0;
  જો (rvalue == ખાલી && cif->rtype->size > 2 * __SIZEOF_POINTER__)
    rval_bytes = FFI_ALIGN (cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + extra_bytes + rval_bytes + કદ (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_base;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  જો (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_base = (size_t) alloca (alloc_size);
  નહીંતર
    alloc_base = FFI_ALIGN (alloca (alloc_size + STKALIGN - 1), STKALIGN);

  જો (rval_bytes)
    rvalue = (શૂન્ય *) (alloc_base + arg_bytes);

  call_builder cb;
  cb.used_float = cb.used_integer = 0;
  cb.aregs = (call_context *) (alloc_base + arg_bytes + extra_bytes + rval_bytes);
  cb.used_ઢગલો = (size_t *)((અક્ષર *) alloc_base);
  cb.ઢગલો = (size_t *)((અક્ષર *) alloc_base + extra_bytes + rval_bytes);
  cb.next_struct_area = arg_bytes;

  પૂર્ણાંક return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  જો (return_by_ref)
    cb.aregs->a[cb.used_integer++] = (size_t)rvalue;

  પૂર્ણાંક i;
  માટે (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((શૂન્ય *) alloc_base, cb.aregs, fn, closure);

  cb.used_float = cb.used_integer = 0;
  જો (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ખાલી);
}

શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue,
	     શૂન્ય *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

બાહ્ય શૂન્ય ffi_closure_asm (શૂન્ય) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
		      શૂન્ય *user_data, શૂન્ય *codeloc)
{
  uint32_t *tramp = (uint32_t *) &closure->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

  જો (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    પાછા FFI_BAD_ABI;

#જો defined(FFI_EXEC_STATIC_TRAMP)
  જો (ffi_tramp_is_present(closure))
    {
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_asm, closure);
      જાઓ out;
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

#જો defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
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
   registers will be reloaded from aregs.  */
શૂન્ય FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
		   શૂન્ય *user_data, size_t *ઢગલો, call_context *aregs)
{
  શૂન્ય **avalue = alloca (cif->nargs * કદ (શૂન્ય *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  અક્ષર *astorage = alloca (cif->nargs * MAXCOPYARG);
  શૂન્ય *rvalue;
  call_builder cb;
  પૂર્ણાંક return_by_ref;
  પૂર્ણાંક i;

  cb.aregs = aregs;
  cb.used_integer = cb.used_float = 0;
  cb.used_ઢગલો = ઢગલો;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  જો (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  નહીંતર
    rvalue = alloca (cif->rtype->size);

  માટે (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_data);

  જો (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.used_integer = cb.used_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#જો defined(FFI_EXEC_STATIC_TRAMP)
શૂન્ય *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  બાહ્ય શૂન્ય *trampoline_code_table;

  *tramp_size = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_size = 1 << 16;
  પાછા &trampoline_code_table;
}
#endif
