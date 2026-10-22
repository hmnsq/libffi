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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#यदि !FFI_NO_RAW_API

size_t
ffi_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  पूर्णाङ्क i;

  ffi_type **at = cif->arg_types;

  प्रति (i = cif->nargs-1; i >= 0; i--, at++)
    {
#यदि !FFI_NO_STRUCTS
      यदि ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (परिमाण (शून्य*), FFI_SIZEOF_ARG);
      अन्यथा
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  प्रत्यागम result;
}


शून्य
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, शून्य **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#यदि शब्द_BIGENDIAN

  प्रति (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      विकल्प ((*tp)->type)
	{
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT8:
	  *args = (शून्य*) ((वर्ण*)(raw++) + FFI_SIZEOF_ARG - 1);
	  भङ्ग;
	  
	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT16:
	  *args = (शून्य*) ((वर्ण*)(raw++) + FFI_SIZEOF_ARG - 2);
	  भङ्ग;

#यदि FFI_SIZEOF_ARG >= 4	  
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT32:
	  *args = (शून्य*) ((वर्ण*)(raw++) + FFI_SIZEOF_ARG - 4);
	  भङ्ग;
#endif
	
#यदि !FFI_NO_STRUCTS
	पक्ष FFI_TYPE_STRUCT:
	पक्ष FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  भङ्ग;

	पक्ष FFI_TYPE_POINTER:
	  *args = (शून्य*) &(raw++)->ptr;
	  भङ्ग;
	  
	स्वभाव:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#अन्यथा /* WORDS_BIGENDIAN */

#यदि !PDP

  /* then assume little endian */
  प्रति (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#यदि !FFI_NO_STRUCTS
      यदि ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      अन्यथा
#endif
      यदि ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      अन्यथा
	{
	  *args = (शून्य*) raw;
	  raw += FFI_ALIGN ((*tp)->size, परिमाण (शून्य*)) / परिमाण (शून्य*);
	}
    }

#अन्यथा
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

शून्य
ffi_ptrarray_to_raw (ffi_cif *cif, शून्य **args, ffi_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  प्रति (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      विकल्प ((*tp)->type)
	{
	पक्ष FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  भङ्ग;

	पक्ष FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  भङ्ग;

	पक्ष FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  भङ्ग;

	पक्ष FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  भङ्ग;

#यदि FFI_SIZEOF_ARG >= 4
	पक्ष FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  भङ्ग;

	पक्ष FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  भङ्ग;
#endif

#यदि !FFI_NO_STRUCTS
	पक्ष FFI_TYPE_STRUCT:
	पक्ष FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  भङ्ग;

	पक्ष FFI_TYPE_POINTER:
	  (raw++)->ptr = **(शून्य***) args;
	  भङ्ग;

	स्वभाव:
	  memcpy ((शून्य*) raw->data, (शून्य*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#यदि !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

शून्य ffi_raw_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, ffi_raw *raw)
{
  शून्य **avalue = (शून्य**) alloca (cif->nargs * परिमाण (शून्य*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#यदि FFI_CLOSURES		/* base system provides closures */

स्थिर शून्य
ffi_translate_args (ffi_cif *cif, शून्य *rvalue,
		    शून्य **avalue, शून्य *user_data)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_data);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  शून्य (*fun)(ffi_cif*,शून्य*,ffi_raw*,शून्य*),
			  शून्य *user_data,
			  शून्य *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  यदि (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  प्रत्यागम status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#यदि FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,ffi_raw*,शून्य*),
		      शून्य *user_data)
{
  प्रत्यागम ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
