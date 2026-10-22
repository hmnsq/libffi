/* -----------------------------------------------------------------------
   raw_api.c - Copyright (c) 1999, 2008  Red Hat, Inc.

   Author: Kresten Krab Thorup <krab@gnu.org>

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

/* This file defines generic functions for use with the raw api. */

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#എങ്കിൽ !FFI_NO_RAW_API

size_t
ffi_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  പൂർണ്ണസംഖ്യ i;

  ffi_type **at = cif->arg_types;

  വേണ്ടി (i = cif->nargs-1; i >= 0; i--, at++)
    {
#എങ്കിൽ !FFI_NO_STRUCTS
      എങ്കിൽ ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (വലുപ്പം (ശൂന്യം*), FFI_SIZEOF_ARG);
      അല്ലെങ്കിൽ
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  തിരികെ result;
}


ശൂന്യം
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, ശൂന്യം **args)
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
	  *args = (ശൂന്യം*) ((അക്ഷരം*)(raw++) + FFI_SIZEOF_ARG - 1);
	  നിർത്തുക;
	  
	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT16:
	  *args = (ശൂന്യം*) ((അക്ഷരം*)(raw++) + FFI_SIZEOF_ARG - 2);
	  നിർത്തുക;

#എങ്കിൽ FFI_SIZEOF_ARG >= 4	  
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT32:
	  *args = (ശൂന്യം*) ((അക്ഷരം*)(raw++) + FFI_SIZEOF_ARG - 4);
	  നിർത്തുക;
#endif
	
#എങ്കിൽ !FFI_NO_STRUCTS
	അവസ്ഥ FFI_TYPE_STRUCT:
	അവസ്ഥ FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_POINTER:
	  *args = (ശൂന്യം*) &(raw++)->ptr;
	  നിർത്തുക;
	  
	സ്ഥിരസ്ഥിതി:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#അല്ലെങ്കിൽ /* WORDS_BIGENDIAN */

#എങ്കിൽ !PDP

  /* then assume little endian */
  വേണ്ടി (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#എങ്കിൽ !FFI_NO_STRUCTS
      എങ്കിൽ ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      അല്ലെങ്കിൽ
#endif
      എങ്കിൽ ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      അല്ലെങ്കിൽ
	{
	  *args = (ശൂന്യം*) raw;
	  raw += FFI_ALIGN ((*tp)->size, വലുപ്പം (ശൂന്യം*)) / വലുപ്പം (ശൂന്യം*);
	}
    }

#അല്ലെങ്കിൽ
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

ശൂന്യം
ffi_ptrarray_to_raw (ffi_cif *cif, ശൂന്യം **args, ffi_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  വേണ്ടി (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      തിരഞ്ഞെടുപ്പ് ((*tp)->type)
	{
	അവസ്ഥ FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  നിർത്തുക;

#എങ്കിൽ FFI_SIZEOF_ARG >= 4
	അവസ്ഥ FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  നിർത്തുക;
#endif

#എങ്കിൽ !FFI_NO_STRUCTS
	അവസ്ഥ FFI_TYPE_STRUCT:
	അവസ്ഥ FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_POINTER:
	  (raw++)->ptr = **(ശൂന്യം***) args;
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  memcpy ((ശൂന്യം*) raw->data, (ശൂന്യം*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#എങ്കിൽ !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

ശൂന്യം ffi_raw_call (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ffi_raw *raw)
{
  ശൂന്യം **avalue = (ശൂന്യം**) alloca (cif->nargs * വലുപ്പം (ശൂന്യം*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#എങ്കിൽ FFI_CLOSURES		/* base system provides closures */

സ്ഥിതം ശൂന്യം
ffi_translate_args (ffi_cif *cif, ശൂന്യം *rvalue,
		    ശൂന്യം **avalue, ശൂന്യം *user_data)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_data);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ffi_raw*,ശൂന്യം*),
			  ശൂന്യം *user_data,
			  ശൂന്യം *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  എങ്കിൽ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  തിരികെ status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#എങ്കിൽ FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ffi_raw*,ശൂന്യം*),
		      ശൂന്യം *user_data)
{
  തിരികെ ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
