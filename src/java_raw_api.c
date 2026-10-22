/* -----------------------------------------------------------------------
   java_raw_api.c - Copyright (c) 1999, 2007, 2008  Red Hat, Inc.

   Cloned from raw_api.c

   Raw_api.c author: Kresten Krab Thorup <krab@gnu.org>
   Java_raw_api.c author: Hans-J. Boehm <hboehm@hpl.hp.com>

   $Id $

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

/* This defines a Java- and 64-bit specific variant of the raw API.	*/
/* It assumes that "raw" argument blocks look like Java stacks on a	*/
/* 64-bit machine.  Arguments that can be stored in a single stack	*/
/* stack slots (longs, doubles) occupy 128 bits, but only the first	*/
/* 64 bits are actually used.						*/

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <stdlib.h>

#എങ്കിൽ !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  പൂർണ്ണസംഖ്യ i;

  ffi_type **at = cif->arg_types;

  വേണ്ടി (i = cif->nargs-1; i >= 0; i--, at++)
    {
      തിരഞ്ഞെടുപ്പ്((*at) -> type) {
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	  result += 2 * FFI_SIZEOF_JAVA_RAW;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_STRUCT:
	അവസ്ഥ FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	അവസ്ഥ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	സ്ഥിരസ്ഥിതി:
	  result += FFI_SIZEOF_JAVA_RAW;
      }
    }

  തിരികെ result;
}


ശൂന്യം
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, ശൂന്യം **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#എങ്കിൽ വാക്കുകൾ_BIGENDIAN

  വേണ്ടി (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      തിരഞ്ഞെടുപ്പ് ((*tp)->type)
	{
	അവസ്ഥ FFI_TYPE_UINT8:
	അവസ്ഥ FFI_TYPE_SINT8:
	  *args = (ശൂന്യം*) ((അക്ഷരം*)(raw++) + 3);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT16:
	  *args = (ശൂന്യം*) ((അക്ഷരം*)(raw++) + 2);
	  നിർത്തുക;

#എങ്കിൽ FFI_SIZEOF_JAVA_RAW == 8
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	  *args = (ശൂന്യം *)raw;
	  raw += 2;
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_POINTER:
	  *args = (ശൂന്യം*) &(raw++)->ptr;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	സ്ഥിരസ്ഥിതി:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->size, വലുപ്പം(ffi_java_raw)) / വലുപ്പം(ffi_java_raw);
	}
    }

#അല്ലെങ്കിൽ /* WORDS_BIGENDIAN */

#എങ്കിൽ !PDP

  /* then assume little endian */
  വേണ്ടി (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#എങ്കിൽ FFI_SIZEOF_JAVA_RAW == 8
      തിരഞ്ഞെടുപ്പ്((*tp)->type) {
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	  *args = (ശൂന്യം*) raw;
	  raw += 2;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	സ്ഥിരസ്ഥിതി:
	  *args = (ശൂന്യം*) raw++;
      }
#അല്ലെങ്കിൽ /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (ശൂന്യം*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->size, വലുപ്പം(ffi_java_raw)) / വലുപ്പം(ffi_java_raw);
#endif /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#അല്ലെങ്കിൽ
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

ശൂന്യം
ffi_java_ptrarray_to_raw (ffi_cif *cif, ശൂന്യം **args, ffi_java_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  വേണ്ടി (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      തിരഞ്ഞെടുപ്പ് ((*tp)->type)
	{
	അവസ്ഥ FFI_TYPE_UINT8:
#എങ്കിൽ വാക്കുകൾ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#അല്ലെങ്കിൽ
	  (raw++)->uint = *(UINT8*) (*args);
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT8:
#എങ്കിൽ വാക്കുകൾ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#അല്ലെങ്കിൽ
	  (raw++)->sint = *(SINT8*) (*args);
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT16:
#എങ്കിൽ വാക്കുകൾ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#അല്ലെങ്കിൽ
	  (raw++)->uint = *(UINT16*) (*args);
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT16:
#എങ്കിൽ വാക്കുകൾ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#അല്ലെങ്കിൽ
	  (raw++)->sint = *(SINT16*) (*args);
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT32:
#എങ്കിൽ വാക്കുകൾ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#അല്ലെങ്കിൽ
	  (raw++)->uint = *(UINT32*) (*args);
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT32:
#എങ്കിൽ വാക്കുകൾ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#അല്ലെങ്കിൽ
	  (raw++)->sint = *(SINT32*) (*args);
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  നിർത്തുക;

#എങ്കിൽ FFI_SIZEOF_JAVA_RAW == 8
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_POINTER:
	  (raw++)->ptr = **(ശൂന്യം***) args;
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
#എങ്കിൽ FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#അല്ലെങ്കിൽ
	  memcpy ((ശൂന്യം*) raw->data, (ശൂന്യം*)*args, (*tp)->size);
	  raw +=
	    FFI_ALIGN ((*tp)->size, വലുപ്പം(ffi_java_raw)) / വലുപ്പം(ffi_java_raw);
#endif
	}
    }
}

#എങ്കിൽ !FFI_NATIVE_RAW_API

സ്ഥിതം ശൂന്യം
ffi_java_rvalue_to_raw (ffi_cif *cif, ശൂന്യം *rvalue)
{
#എങ്കിൽ വാക്കുകൾ_BIGENDIAN && FFI_SIZEOF_ARG == 8
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_UINT8:
    അവസ്ഥ FFI_TYPE_UINT16:
    അവസ്ഥ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SINT8:
    അവസ്ഥ FFI_TYPE_SINT16:
    അവസ്ഥ FFI_TYPE_SINT32:
    അവസ്ഥ FFI_TYPE_INT:
#എങ്കിൽ FFI_SIZEOF_JAVA_RAW == 4
    അവസ്ഥ FFI_TYPE_POINTER:
#endif
      *(SINT64 *)rvalue <<= 32;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    സ്ഥിരസ്ഥിതി:
      നിർത്തുക;
    }
#endif
}

സ്ഥിതം ശൂന്യം
ffi_java_raw_to_rvalue (ffi_cif *cif, ശൂന്യം *rvalue)
{
#എങ്കിൽ വാക്കുകൾ_BIGENDIAN && FFI_SIZEOF_ARG == 8
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_UINT8:
    അവസ്ഥ FFI_TYPE_UINT16:
    അവസ്ഥ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SINT8:
    അവസ്ഥ FFI_TYPE_SINT16:
    അവസ്ഥ FFI_TYPE_SINT32:
    അവസ്ഥ FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    സ്ഥിരസ്ഥിതി:
      നിർത്തുക;
    }
#endif
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

ശൂന്യം ffi_java_raw_call (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
			ffi_java_raw *raw)
{
  ശൂന്യം **avalue = (ശൂന്യം**) alloca (cif->nargs * വലുപ്പം (ശൂന്യം*));
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#എങ്കിൽ FFI_CLOSURES		/* base system provides closures */

സ്ഥിതം ശൂന്യം
ffi_java_translate_args (ffi_cif *cif, ശൂന്യം *rvalue,
		    ശൂന്യം **avalue, ശൂന്യം *user_data)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_java_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_data);
  ffi_java_raw_to_rvalue (cif, rvalue);
}

ffi_status
ffi_prep_java_raw_closure_loc (ffi_java_raw_closure* cl,
			       ffi_cif *cif,
			       ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ffi_java_raw*,ശൂന്യം*),
			       ശൂന്യം *user_data,
			       ശൂന്യം *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_java_translate_args,
				 codeloc,
				 codeloc);
  എങ്കിൽ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  തിരികെ status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_java_raw_closure (ffi_java_raw_closure* cl,
			   ffi_cif *cif,
			   ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ffi_java_raw*,ശൂന്യം*),
			   ശൂന്യം *user_data)
{
  തിരികെ ffi_prep_java_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */
#endif /* !NO_JAVA_RAW_API */
