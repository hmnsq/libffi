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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <tramp.h>

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdint.h>

#എങ്കിൽ defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#അല്ലെങ്കിലെങ്കിൽ defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT ദശാംശം
#അല്ലെങ്കിലെങ്കിൽ defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT ഇരട്ട
#അല്ലെങ്കിൽ
#error unsupported LoongArch floating-point ABI
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * വലുപ്പം (ഇരട്ട))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
തരനിർവചനം ഘടന call_context
{
#എങ്കിൽ !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#endif
  size_t a[10];
} call_context;

തരനിർവചനം ഘടന call_builder
{
  call_context *aregs;
  പൂർണ്ണസംഖ്യ used_integer;
  പൂർണ്ണസംഖ്യ used_float;
  size_t *used_അട്ടി;	    /* Stack pointer.  */
  size_t *അട്ടി;	    /* Place for arguments pass by reference.  */
  size_t next_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#എങ്കിൽ __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#അല്ലെങ്കിൽ
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#എങ്കിൽ ABI_FRLEN
തരനിർവചനം ഘടന float_struct_info
{
  അക്ഷരം as_elements;
  അക്ഷരം type1;
  അക്ഷരം offset2;
  അക്ഷരം type2;
} float_struct_info;

#എങ്കിൽ ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#അല്ലെങ്കിൽ
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

സ്ഥിതം ffi_type **
flatten_struct (ffi_type *in, ffi_type **out, ffi_type **out_end)
{
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ (out == out_end)
    തിരികെ out;
  എങ്കിൽ (in->type != FFI_TYPE_STRUCT)
    *(out++) = in;
  അല്ലെങ്കിൽ
    വേണ്ടി (i = 0; in->elements[i]; i++)
      out = flatten_struct (in->elements[i], out, out_end);
  തിരികെ out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
സ്ഥിതം float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *മുകൾ)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  പൂർണ്ണസംഖ്യ അക്കം_floats, അക്കം_ints;
  പൂർണ്ണസംഖ്യ അക്കം_fields = flatten_struct (മുകൾ, fields, fields + 3) - fields;

  എങ്കിൽ (അക്കം_fields == 1)
    {
      എങ്കിൽ (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  അല്ലെങ്കിൽ എങ്കിൽ (അക്കം_fields == 2)
    {
      അക്കം_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      അക്കം_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      എങ്കിൽ (അക്കം_floats == 0 || അക്കം_floats + അക്കം_ints != 2)
	തിരികെ ret;
      എങ്കിൽ (cb->used_float + അക്കം_floats > NARGREG
	  || cb->used_integer + (2 - അക്കം_floats) > NARGREG)
	തിരികെ ret;
      എങ്കിൽ (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	തിരികെ ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->size, fields[1]->alignment);
      ret.as_elements = 1;
    }
  തിരികെ ret;
}
#endif

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
സ്ഥിതം ശൂന്യം
marshal_atom (call_builder *cb, പൂർണ്ണസംഖ്യ type, ശൂന്യം *data)
{
  size_t മൂല്യം = 0;
  തിരഞ്ഞെടുപ്പ് (type)
    {
    അവസ്ഥ FFI_TYPE_UINT8:
      മൂല്യം = *(uint8_t *) data;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT8:
      മൂല്യം = *(int8_t *) data;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT16:
      മൂല്യം = *(uint16_t *) data;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16:
      മൂല്യം = *(int16_t *) data;
      നിർത്തുക;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    അവസ്ഥ FFI_TYPE_UINT32:
      മൂല്യം = *(int32_t *) data;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT32:
      മൂല്യം = *(int32_t *) data;
      നിർത്തുക;
#എങ്കിൽ __SIZEOF_POINTER__ == 8
    അവസ്ഥ FFI_TYPE_UINT64:
      മൂല്യം = *(uint64_t *) data;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT64:
      മൂല്യം = *(int64_t *) data;
      നിർത്തുക;
#endif
    അവസ്ഥ FFI_TYPE_POINTER:
      മൂല്യം = *(size_t *) data;
      നിർത്തുക;

#എങ്കിൽ ABI_FRLEN >= 32
    അവസ്ഥ FFI_TYPE_FLOAT:
      *(ദശാംശം *)(cb->aregs->fa + cb->used_float++) = *(ദശാംശം *) data;
      തിരികെ;
#endif
#എങ്കിൽ ABI_FRLEN >= 64
    അവസ്ഥ FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->used_float++]) = *(ഇരട്ട *) data;
      തിരികെ;
#endif
    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT (0);
      നിർത്തുക;
    }

  എങ്കിൽ (cb->used_integer == NARGREG)
    *cb->used_അട്ടി++ = മൂല്യം;
  അല്ലെങ്കിൽ
    cb->aregs->a[cb->used_integer++] = മൂല്യം;
}

സ്ഥിതം ശൂന്യം
unmarshal_atom (call_builder *cb, പൂർണ്ണസംഖ്യ type, ശൂന്യം *data)
{
  size_t മൂല്യം;
  തിരഞ്ഞെടുപ്പ് (type)
    {
#എങ്കിൽ ABI_FRLEN >= 32
    അവസ്ഥ FFI_TYPE_FLOAT:
      *(ദശാംശം *) data = *(ദശാംശം *)(cb->aregs->fa + cb->used_float++);
      തിരികെ;
#endif
#എങ്കിൽ ABI_FRLEN >= 64
    അവസ്ഥ FFI_TYPE_DOUBLE:
      *(ഇരട്ട *) data = cb->aregs->fa[cb->used_float++];
      തിരികെ;
#endif
    }

  എങ്കിൽ (cb->used_integer == NARGREG)
    മൂല്യം = *cb->used_അട്ടി++;
  അല്ലെങ്കിൽ
    മൂല്യം = cb->aregs->a[cb->used_integer++];

  തിരഞ്ഞെടുപ്പ് (type)
    {
    അവസ്ഥ FFI_TYPE_UINT8:
    അവസ്ഥ FFI_TYPE_SINT8:
    അവസ്ഥ FFI_TYPE_UINT16:
    അവസ്ഥ FFI_TYPE_SINT16:
    അവസ്ഥ FFI_TYPE_UINT32:
    അവസ്ഥ FFI_TYPE_SINT32:
#എങ്കിൽ __SIZEOF_POINTER__ == 8
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
#endif
    അവസ്ഥ FFI_TYPE_POINTER:
      *(ffi_arg *)data = മൂല്യം;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT (0);
      നിർത്തുക;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
സ്ഥിതം ശൂന്യം *
allocate_and_copy_struct_to_അട്ടി (call_builder *cb, ശൂന്യം *data,
				   ffi_type *type)
{
  size_t dest = cb->next_struct_area - type->size;

  dest = FFI_ALIGN_DOWN (dest, type->alignment);
  cb->next_struct_area = dest;

  തിരികെ memcpy ((അക്ഷരം *)cb->അട്ടി + dest, data, type->size);
}

/* Adds an argument to a call, or a not by reference return value.  */
സ്ഥിതം ശൂന്യം
marshal (call_builder *cb, ffi_type *type, പൂർണ്ണസംഖ്യ var, ശൂന്യം *data)
{
  size_t realign[2];

#എങ്കിൽ ABI_FRLEN
  എങ്കിൽ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      എങ്കിൽ (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, data);
	  എങ്കിൽ (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((അക്ഷരം *) data) + fsi.offset2);
	  തിരികെ;
	}
    }

  എങ്കിൽ (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, data);
      തിരികെ;
    }

  ഇരട്ട promoted;
  എങ്കിൽ (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(ദശാംശം *) data;
      type = &ffi_type_double;
      data = &promoted;
    }
#endif

  എങ്കിൽ (type->size > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_to_അട്ടി (cb, data, type);
      data = (അക്ഷരം *)cb->അട്ടി + cb->next_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &data);
    }
  അല്ലെങ്കിൽ എങ്കിൽ (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, data);
  അല്ലെങ്കിൽ
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      എങ്കിൽ (type->alignment > __SIZEOF_POINTER__)
	{
	  എങ്കിൽ (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_അട്ടി
	    = (size_t *) FFI_ALIGN (cb->used_അട്ടി, 2 * __SIZEOF_POINTER__);
	}

      memcpy (realign, data, type->size);
      എങ്കിൽ (type->size > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      എങ്കിൽ (type->size > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
സ്ഥിതം ശൂന്യം *
unmarshal (call_builder *cb, ffi_type *type, പൂർണ്ണസംഖ്യ var, ശൂന്യം *data)
{
  size_t realign[2];
  ശൂന്യം *pointer;

#എങ്കിൽ ABI_FRLEN
  എങ്കിൽ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      എങ്കിൽ (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, data);
	  എങ്കിൽ (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((അക്ഷരം *) data) + fsi.offset2);
	  തിരികെ data;
	}
    }

  എങ്കിൽ (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, data);
      തിരികെ data;
    }

  എങ്കിൽ (var && type->type == FFI_TYPE_FLOAT)
    {
      പൂർണ്ണസംഖ്യ m = cb->used_integer;
      ശൂന്യം *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->used_അട്ടി + m - NARGREG + 1;
      *(ദശാംശം *) promoted = *(ഇരട്ട *) promoted;
    }
#endif

  എങ്കിൽ (type->size > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (അക്ഷരം *) &pointer);
      തിരികെ pointer;
    }
  അല്ലെങ്കിൽ എങ്കിൽ (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, data);
      തിരികെ data;
    }
  അല്ലെങ്കിൽ
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      എങ്കിൽ (type->alignment > __SIZEOF_POINTER__)
	{
	  എങ്കിൽ (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_അട്ടി
	    = (size_t *) FFI_ALIGN (cb->used_അട്ടി, 2 * __SIZEOF_POINTER__);
	}

      എങ്കിൽ (type->size > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      എങ്കിൽ (type->size > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      memcpy (data, realign, type->size);
      തിരികെ data;
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
passed_by_ref (call_builder *cb, ffi_type *type, പൂർണ്ണസംഖ്യ var)
{
#എങ്കിൽ ABI_FRLEN
  എങ്കിൽ (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      എങ്കിൽ (fsi.as_elements)
	തിരികെ 0;
    }
#endif

  തിരികെ type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  തിരികെ FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_status
ffi_prep_cif_machdep_var (ffi_cif *cif, unsigned പൂർണ്ണസംഖ്യ nfixedargs,
			  unsigned പൂർണ്ണസംഖ്യ ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  തിരികെ FFI_OK;
}

/* Low level routine for calling functions.  */
ബാഹ്യ ശൂന്യം ffi_call_asm (ശൂന്യം *അട്ടി, ഘടന call_context *regs,
			  ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *closure) FFI_HIDDEN;

സ്ഥിതം ശൂന്യം
ffi_call_int (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue,
	      ശൂന്യം *closure)
{
  size_t arg_bytes = FFI_ALIGN (cif->bytes, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  size_t extra_bytes = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * വലുപ്പം(size_t) * (cif->nargs - 3), STKALIGN);

  size_t rval_bytes = 0;
  എങ്കിൽ (rvalue == ശൂന്യ && cif->rtype->size > 2 * __SIZEOF_POINTER__)
    rval_bytes = FFI_ALIGN (cif->rtype->size, STKALIGN);
  size_t alloc_size = arg_bytes + extra_bytes + rval_bytes + വലുപ്പം (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  size_t alloc_base;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  എങ്കിൽ (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_base = (size_t) alloca (alloc_size);
  അല്ലെങ്കിൽ
    alloc_base = FFI_ALIGN (alloca (alloc_size + STKALIGN - 1), STKALIGN);

  എങ്കിൽ (rval_bytes)
    rvalue = (ശൂന്യം *) (alloc_base + arg_bytes);

  call_builder cb;
  cb.used_float = cb.used_integer = 0;
  cb.aregs = (call_context *) (alloc_base + arg_bytes + extra_bytes + rval_bytes);
  cb.used_അട്ടി = (size_t *)((അക്ഷരം *) alloc_base);
  cb.അട്ടി = (size_t *)((അക്ഷരം *) alloc_base + extra_bytes + rval_bytes);
  cb.next_struct_area = arg_bytes;

  പൂർണ്ണസംഖ്യ return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  എങ്കിൽ (return_by_ref)
    cb.aregs->a[cb.used_integer++] = (size_t)rvalue;

  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((ശൂന്യം *) alloc_base, cb.aregs, fn, closure);

  cb.used_float = cb.used_integer = 0;
  എങ്കിൽ (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ശൂന്യ);
}

ശൂന്യം
ffi_call_go (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue,
	     ശൂന്യം *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

ബാഹ്യ ശൂന്യം ffi_closure_asm (ശൂന്യം) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
		      ശൂന്യം *user_data, ശൂന്യം *codeloc)
{
  uint32_t *tramp = (uint32_t *) &closure->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

  എങ്കിൽ (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    തിരികെ FFI_BAD_ABI;

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
  എങ്കിൽ (ffi_tramp_is_present(closure))
    {
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_asm, closure);
      പോകുക out;
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

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
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
   registers will be reloaded from aregs.  */
ശൂന്യം FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
		   ശൂന്യം *user_data, size_t *അട്ടി, call_context *aregs)
{
  ശൂന്യം **avalue = alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  അക്ഷരം *astorage = alloca (cif->nargs * MAXCOPYARG);
  ശൂന്യം *rvalue;
  call_builder cb;
  പൂർണ്ണസംഖ്യ return_by_ref;
  പൂർണ്ണസംഖ്യ i;

  cb.aregs = aregs;
  cb.used_integer = cb.used_float = 0;
  cb.used_അട്ടി = അട്ടി;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  എങ്കിൽ (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  അല്ലെങ്കിൽ
    rvalue = alloca (cif->rtype->size);

  വേണ്ടി (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_data);

  എങ്കിൽ (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.used_integer = cb.used_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
ശൂന്യം *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ബാഹ്യ ശൂന്യം *trampoline_code_table;

  *tramp_size = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_size = 1 << 16;
  തിരികെ &trampoline_code_table;
}
#endif
