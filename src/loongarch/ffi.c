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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <tramp.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>

#जर defined(__loongarch_soft_float)
# define ABI_FRLEN 0
#किंवाजर defined(__loongarch_single_float)
# define ABI_FRLEN 32
# define ABI_FLOAT दशांश
#किंवाजर defined(__loongarch_double_float)
# define ABI_FRLEN 64
# define ABI_FLOAT द्विगुण
#अन्यथा
#error unsupported LoongArch floating-point ABI
#endif

#define NARGREG 8
#define STKALIGN 16
#define MAXCOPYARG (2 * आकार (द्विगुण))

/* call_context registers
   - 8 floating point parameter/result registers.
   - 8 integer parameter/result registers.
   - 2 registers used by the assembly code to in-place construct its own
     stack frame
     - frame register
     - return register
*/
प्रकारव्याख्या संरचना call_context
{
#जर !defined(__loongarch_soft_float)
  ABI_FLOAT fa[8];
#endif
  माप_प्रकार a[10];
} call_context;

प्रकारव्याख्या संरचना call_builder
{
  call_context *aregs;
  पूर्णांक used_integer;
  पूर्णांक used_float;
  माप_प्रकार *used_चळत;	    /* Stack pointer.  */
  माप_प्रकार *चळत;	    /* Place for arguments pass by reference.  */
  माप_प्रकार next_struct_area;  /* Offset for arguments pass by reference.  */
} call_builder;

/* Integer (not pointer) less than ABI GRLEN.  */
/* FFI_TYPE_INT does not appear to be used.  */
#जर __SIZEOF_POINTER__ == 8
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT64)
#अन्यथा
# define IS_INT(type) ((type) >= FFI_TYPE_UINT8 && (type) <= FFI_TYPE_SINT32)
#endif

#जर ABI_FRLEN
प्रकारव्याख्या संरचना float_struct_info
{
  अक्षर as_elements;
  अक्षर type1;
  अक्षर offset2;
  अक्षर type2;
} float_struct_info;

#जर ABI_FRLEN >= 64
# define IS_FLOAT(type) ((type) >= FFI_TYPE_FLOAT && (type) <= FFI_TYPE_DOUBLE)
#अन्यथा
# define IS_FLOAT(type) ((type) == FFI_TYPE_FLOAT)
#endif

स्थिर ffi_type **
flatten_struct (ffi_type *in, ffi_type **out, ffi_type **out_end)
{
  पूर्णांक i;

  जर (out == out_end)
    परत out;
  जर (in->type != FFI_TYPE_STRUCT)
    *(out++) = in;
  अन्यथा
    साठी (i = 0; in->elements[i]; i++)
      out = flatten_struct (in->elements[i], out, out_end);
  परत out;
}

/* Structs with at most two fields after flattening, one of which is of
   floating point type, are passed in multiple registers if sufficient
   registers are available.  */
स्थिर float_struct_info
struct_passed_as_elements (call_builder *cb, ffi_type *शीर्ष)
{
  float_struct_info ret = {0, 0, 0, 0};
  ffi_type *fields[3];
  पूर्णांक अंक_floats, अंक_ints;
  पूर्णांक अंक_fields = flatten_struct (शीर्ष, fields, fields + 3) - fields;

  जर (अंक_fields == 1)
    {
      जर (IS_FLOAT (fields[0]->type))
	{
	  ret.as_elements = 1;
	  ret.type1 = fields[0]->type;
	}
    }
  अन्यथा जर (अंक_fields == 2)
    {
      अंक_floats = IS_FLOAT (fields[0]->type) + IS_FLOAT (fields[1]->type);
      अंक_ints = IS_INT (fields[0]->type) + IS_INT (fields[1]->type);
      जर (अंक_floats == 0 || अंक_floats + अंक_ints != 2)
	परत ret;
      जर (cb->used_float + अंक_floats > NARGREG
	  || cb->used_integer + (2 - अंक_floats) > NARGREG)
	परत ret;
      जर (!IS_FLOAT (fields[0]->type) && !IS_FLOAT (fields[1]->type))
	परत ret;

      ret.type1 = fields[0]->type;
      ret.type2 = fields[1]->type;
      ret.offset2 = FFI_ALIGN (fields[0]->size, fields[1]->alignment);
      ret.as_elements = 1;
    }
  परत ret;
}
#endif

/* Allocates a single register, float register, or GRLEN-sized stack slot to a
   datum.  */
स्थिर शून्य
marshal_atom (call_builder *cb, पूर्णांक type, शून्य *data)
{
  माप_प्रकार मूल्य = 0;
  निवड (type)
    {
    स्थिती FFI_TYPE_UINT8:
      मूल्य = *(uint8_t *) data;
      थांबा;
    स्थिती FFI_TYPE_SINT8:
      मूल्य = *(int8_t *) data;
      थांबा;
    स्थिती FFI_TYPE_UINT16:
      मूल्य = *(uint16_t *) data;
      थांबा;
    स्थिती FFI_TYPE_SINT16:
      मूल्य = *(int16_t *) data;
      थांबा;
    /* 32-bit quantities are always sign-extended in the ABI.  */
    स्थिती FFI_TYPE_UINT32:
      मूल्य = *(int32_t *) data;
      थांबा;
    स्थिती FFI_TYPE_SINT32:
      मूल्य = *(int32_t *) data;
      थांबा;
#जर __SIZEOF_POINTER__ == 8
    स्थिती FFI_TYPE_UINT64:
      मूल्य = *(uint64_t *) data;
      थांबा;
    स्थिती FFI_TYPE_SINT64:
      मूल्य = *(int64_t *) data;
      थांबा;
#endif
    स्थिती FFI_TYPE_POINTER:
      मूल्य = *(माप_प्रकार *) data;
      थांबा;

#जर ABI_FRLEN >= 32
    स्थिती FFI_TYPE_FLOAT:
      *(दशांश *)(cb->aregs->fa + cb->used_float++) = *(दशांश *) data;
      परत;
#endif
#जर ABI_FRLEN >= 64
    स्थिती FFI_TYPE_DOUBLE:
      (cb->aregs->fa[cb->used_float++]) = *(द्विगुण *) data;
      परत;
#endif
    पूर्वनिर्धारित:
      FFI_ASSERT (0);
      थांबा;
    }

  जर (cb->used_integer == NARGREG)
    *cb->used_चळत++ = मूल्य;
  अन्यथा
    cb->aregs->a[cb->used_integer++] = मूल्य;
}

स्थिर शून्य
unmarshal_atom (call_builder *cb, पूर्णांक type, शून्य *data)
{
  माप_प्रकार मूल्य;
  निवड (type)
    {
#जर ABI_FRLEN >= 32
    स्थिती FFI_TYPE_FLOAT:
      *(दशांश *) data = *(दशांश *)(cb->aregs->fa + cb->used_float++);
      परत;
#endif
#जर ABI_FRLEN >= 64
    स्थिती FFI_TYPE_DOUBLE:
      *(द्विगुण *) data = cb->aregs->fa[cb->used_float++];
      परत;
#endif
    }

  जर (cb->used_integer == NARGREG)
    मूल्य = *cb->used_चळत++;
  अन्यथा
    मूल्य = cb->aregs->a[cb->used_integer++];

  निवड (type)
    {
    स्थिती FFI_TYPE_UINT8:
    स्थिती FFI_TYPE_SINT8:
    स्थिती FFI_TYPE_UINT16:
    स्थिती FFI_TYPE_SINT16:
    स्थिती FFI_TYPE_UINT32:
    स्थिती FFI_TYPE_SINT32:
#जर __SIZEOF_POINTER__ == 8
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
#endif
    स्थिती FFI_TYPE_POINTER:
      *(ffi_arg *)data = मूल्य;
      थांबा;
    पूर्वनिर्धारित:
      FFI_ASSERT (0);
      थांबा;
    }
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
स्थिर शून्य *
allocate_and_copy_struct_to_चळत (call_builder *cb, शून्य *data,
				   ffi_type *type)
{
  माप_प्रकार dest = cb->next_struct_area - type->size;

  dest = FFI_ALIGN_DOWN (dest, type->alignment);
  cb->next_struct_area = dest;

  परत स_नकल ((अक्षर *)cb->चळत + dest, data, type->size);
}

/* Adds an argument to a call, or a not by reference return value.  */
स्थिर शून्य
marshal (call_builder *cb, ffi_type *type, पूर्णांक var, शून्य *data)
{
  माप_प्रकार realign[2];

#जर ABI_FRLEN
  जर (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      जर (fsi.as_elements)
	{
	  marshal_atom (cb, fsi.type1, data);
	  जर (fsi.offset2)
	    marshal_atom (cb, fsi.type2, ((अक्षर *) data) + fsi.offset2);
	  परत;
	}
    }

  जर (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      marshal_atom (cb, type->type, data);
      परत;
    }

  द्विगुण promoted;
  जर (var && type->type == FFI_TYPE_FLOAT)
    {
      /* C standard requires promoting float -> double for variable arg.  */
      promoted = *(दशांश *) data;
      type = &ffi_type_double;
      data = &promoted;
    }
#endif

  जर (type->size > 2 * __SIZEOF_POINTER__)
    /* Pass by reference.  */
    {
      allocate_and_copy_struct_to_चळत (cb, data, type);
      data = (अक्षर *)cb->चळत + cb->next_struct_area;
      marshal_atom (cb, FFI_TYPE_POINTER, &data);
    }
  अन्यथा जर (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    marshal_atom (cb, type->type, data);
  अन्यथा
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      जर (type->alignment > __SIZEOF_POINTER__)
	{
	  जर (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_चळत
	    = (माप_प्रकार *) FFI_ALIGN (cb->used_चळत, 2 * __SIZEOF_POINTER__);
	}

      स_नकल (realign, data, type->size);
      जर (type->size > 0)
	marshal_atom (cb, FFI_TYPE_POINTER, realign);
      जर (type->size > __SIZEOF_POINTER__)
	marshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
    }
}

/* For arguments passed by reference returns the pointer, otherwise the arg
   is copied (up to MAXCOPYARG bytes).  */
स्थिर शून्य *
unmarshal (call_builder *cb, ffi_type *type, पूर्णांक var, शून्य *data)
{
  माप_प्रकार realign[2];
  शून्य *pointer;

#जर ABI_FRLEN
  जर (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      जर (fsi.as_elements)
	{
	  unmarshal_atom (cb, fsi.type1, data);
	  जर (fsi.offset2)
	    unmarshal_atom (cb, fsi.type2, ((अक्षर *) data) + fsi.offset2);
	  परत data;
	}
    }

  जर (!var && cb->used_float < NARGREG
      && IS_FLOAT (type->type))
    {
      unmarshal_atom (cb, type->type, data);
      परत data;
    }

  जर (var && type->type == FFI_TYPE_FLOAT)
    {
      पूर्णांक m = cb->used_integer;
      शून्य *promoted
	= m < NARGREG ? cb->aregs->a + m : cb->used_चळत + m - NARGREG + 1;
      *(दशांश *) promoted = *(द्विगुण *) promoted;
    }
#endif

  जर (type->size > 2 * __SIZEOF_POINTER__)
    {
      /* Pass by reference.  */
      unmarshal_atom (cb, FFI_TYPE_POINTER, (अक्षर *) &pointer);
      परत pointer;
    }
  अन्यथा जर (IS_INT (type->type) || type->type == FFI_TYPE_POINTER)
    {
      unmarshal_atom (cb, type->type, data);
      परत data;
    }
  अन्यथा
    {
      /* Overlong integers, soft-float floats, and structs without special
	 float handling are treated identically from this point on.  */

      /* Variadics are aligned even in registers.  */
      जर (type->alignment > __SIZEOF_POINTER__)
	{
	  जर (var)
	    cb->used_integer = FFI_ALIGN (cb->used_integer, 2);
	  cb->used_चळत
	    = (माप_प्रकार *) FFI_ALIGN (cb->used_चळत, 2 * __SIZEOF_POINTER__);
	}

      जर (type->size > 0)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign);
      जर (type->size > __SIZEOF_POINTER__)
	unmarshal_atom (cb, FFI_TYPE_POINTER, realign + 1);
      स_नकल (data, realign, type->size);
      परत data;
    }
}

स्थिर पूर्णांक
passed_by_ref (call_builder *cb, ffi_type *type, पूर्णांक var)
{
#जर ABI_FRLEN
  जर (!var && type->type == FFI_TYPE_STRUCT)
    {
      float_struct_info fsi = struct_passed_as_elements (cb, type);
      जर (fsi.as_elements)
	परत 0;
    }
#endif

  परत type->size > 2 * __SIZEOF_POINTER__;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->loongarch_nfixedargs = cif->nargs;
  परत FFI_OK;
}

/* Perform machine dependent cif processing when we have a variadic
   function.  */
ffi_status
ffi_prep_cif_machdep_var (ffi_cif *cif, अचिन्हित पूर्णांक nfixedargs,
			  अचिन्हित पूर्णांक ntotalargs)
{
  cif->loongarch_nfixedargs = nfixedargs;
  परत FFI_OK;
}

/* Low level routine for calling functions.  */
बाह्य शून्य ffi_call_asm (शून्य *चळत, संरचना call_context *regs,
			  शून्य (*fn) (शून्य), शून्य *closure) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	      शून्य *closure)
{
  माप_प्रकार arg_bytes = FFI_ALIGN (cif->bytes, STKALIGN);

  /* This is a conservative estimate, assuming a complex return value and
     that all remaining arguments are long long / __int128 */
  माप_प्रकार extra_bytes = cif->nargs <= 3 ? 0 :
      FFI_ALIGN(2 * आकार(माप_प्रकार) * (cif->nargs - 3), STKALIGN);

  माप_प्रकार rval_bytes = 0;
  जर (rvalue == रिक्त && cif->rtype->size > 2 * __SIZEOF_POINTER__)
    rval_bytes = FFI_ALIGN (cif->rtype->size, STKALIGN);
  माप_प्रकार alloc_size = arg_bytes + extra_bytes + rval_bytes + आकार (call_context);

  /* The assembly code will deallocate all stack data at lower addresses
     than the argument region, so we need to allocate the frame and the
     return value after the arguments in a single allocation.  */
  माप_प्रकार alloc_base;
  /* Argument region must be 16-byte aligned in LP64 ABIs.  */
  जर (_Alignof(max_align_t) >= STKALIGN)
    /* Since sizeof long double is normally 16, the compiler will
       guarantee alloca alignment to at least that much.  */
    alloc_base = (माप_प्रकार) alloca (alloc_size);
  अन्यथा
    alloc_base = FFI_ALIGN (alloca (alloc_size + STKALIGN - 1), STKALIGN);

  जर (rval_bytes)
    rvalue = (शून्य *) (alloc_base + arg_bytes);

  call_builder cb;
  cb.used_float = cb.used_integer = 0;
  cb.aregs = (call_context *) (alloc_base + arg_bytes + extra_bytes + rval_bytes);
  cb.used_चळत = (माप_प्रकार *)((अक्षर *) alloc_base);
  cb.चळत = (माप_प्रकार *)((अक्षर *) alloc_base + extra_bytes + rval_bytes);
  cb.next_struct_area = arg_bytes;

  पूर्णांक return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  जर (return_by_ref)
    cb.aregs->a[cb.used_integer++] = (माप_प्रकार)rvalue;

  पूर्णांक i;
  साठी (i = 0; i < cif->nargs; i++)
    marshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
	     avalue[i]);

  ffi_call_asm ((शून्य *) alloc_base, cb.aregs, fn, closure);

  cb.used_float = cb.used_integer = 0;
  जर (!return_by_ref && rvalue)
    unmarshal (&cb, cif->rtype, 0, rvalue);
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	     शून्य *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

बाह्य शून्य ffi_closure_asm (शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_data, शून्य *codeloc)
{
  uint32_t *tramp = (uint32_t *) &closure->tramp[0];
  uint64_t fn = (uint64_t) (uintptr_t) ffi_closure_asm;

  जर (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    परत FFI_BAD_ABI;

#जर defined(FFI_EXEC_STATIC_TRAMP)
  जर (ffi_tramp_is_present(closure))
    {
      ffi_tramp_set_parms (closure->ftramp, ffi_closure_asm, closure);
      जा out;
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

#जर defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  परत FFI_OK;
}

बाह्य शून्य ffi_go_closure_asm (शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  जर (cif->abi <= FFI_FIRST_ABI || cif->abi >= FFI_LAST_ABI)
    परत FFI_BAD_ABI;

  closure->tramp = (शून्य *) ffi_go_closure_asm;
  closure->cif = cif;
  closure->fun = fun;
  परत FFI_OK;
}

/* Called by the assembly code with aregs pointing to saved argument registers
   and stack pointing to the stacked arguments.  Return values passed in
   registers will be reloaded from aregs.  */
शून्य FFI_HIDDEN
ffi_closure_inner (ffi_cif *cif,
		   शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		   शून्य *user_data, माप_प्रकार *चळत, call_context *aregs)
{
  शून्य **avalue = alloca (cif->nargs * आकार (शून्य *));
  /* Storage for arguments which will be copied by unmarshal().  We could
     theoretically avoid the copies in many cases and use at most 128 bytes
     of memory, but allocating disjoint storage for each argument is
     simpler.  */
  अक्षर *astorage = alloca (cif->nargs * MAXCOPYARG);
  शून्य *rvalue;
  call_builder cb;
  पूर्णांक return_by_ref;
  पूर्णांक i;

  cb.aregs = aregs;
  cb.used_integer = cb.used_float = 0;
  cb.used_चळत = चळत;

  return_by_ref = passed_by_ref (&cb, cif->rtype, 0);
  जर (return_by_ref)
    unmarshal (&cb, &ffi_type_pointer, 0, &rvalue);
  अन्यथा
    rvalue = alloca (cif->rtype->size);

  साठी (i = 0; i < cif->nargs; i++)
    avalue[i]
      = unmarshal (&cb, cif->arg_types[i], i >= cif->loongarch_nfixedargs,
		   astorage + i * MAXCOPYARG);

  fun (cif, rvalue, avalue, user_data);

  जर (!return_by_ref && cif->rtype->type != FFI_TYPE_VOID)
    {
      cb.used_integer = cb.used_float = 0;
      marshal (&cb, cif->rtype, 0, rvalue);
    }
}

#जर defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (माप_प्रकार *tramp_size, माप_प्रकार *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *tramp_size = 16;
  /* A mapping size of 64K is chosen to cover the page sizes of 4K, 16K, and
     64K.  */
  *map_size = 1 << 16;
  परत &trampoline_code_table;
}
#endif
