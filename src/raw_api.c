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

#شامل <ffi.h>
#شامل <ffi_common.h>

#اگر !FFI_NO_RAW_API

size_t
ffi_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  صحیح i;

  ffi_type **at = cif->arg_types;

  برای (i = cif->nargs-1; i >= 0; i--, at++)
    {
#اگر !FFI_NO_STRUCTS
      اگر ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (اندازه (تهی*), FFI_SIZEOF_ARG);
      وگرنه
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  بازگشت result;
}


تهی
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, تهی **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#اگر واژگان_BIGENDIAN

  برای (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      انتخاب ((*tp)->type)
	{
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT8:
	  *args = (تهی*) ((نویسه*)(raw++) + FFI_SIZEOF_ARG - 1);
	  بشکن;
	  
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT16:
	  *args = (تهی*) ((نویسه*)(raw++) + FFI_SIZEOF_ARG - 2);
	  بشکن;

#اگر FFI_SIZEOF_ARG >= 4	  
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	  *args = (تهی*) ((نویسه*)(raw++) + FFI_SIZEOF_ARG - 4);
	  بشکن;
#endif
	
#اگر !FFI_NO_STRUCTS
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  بشکن;
#endif

	حالت FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  بشکن;

	حالت FFI_TYPE_POINTER:
	  *args = (تهی*) &(raw++)->ptr;
	  بشکن;
	  
	پیشفرض:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#وگرنه /* WORDS_BIGENDIAN */

#اگر !PDP

  /* then assume little endian */
  برای (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#اگر !FFI_NO_STRUCTS
      اگر ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      وگرنه
#endif
      اگر ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      وگرنه
	{
	  *args = (تهی*) raw;
	  raw += FFI_ALIGN ((*tp)->size, اندازه (تهی*)) / اندازه (تهی*);
	}
    }

#وگرنه
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

تهی
ffi_ptrarray_to_raw (ffi_cif *cif, تهی **args, ffi_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  برای (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      انتخاب ((*tp)->type)
	{
	حالت FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  بشکن;

	حالت FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  بشکن;

	حالت FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  بشکن;

	حالت FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  بشکن;

#اگر FFI_SIZEOF_ARG >= 4
	حالت FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  بشکن;

	حالت FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  بشکن;
#endif

#اگر !FFI_NO_STRUCTS
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  بشکن;
#endif

	حالت FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  بشکن;

	حالت FFI_TYPE_POINTER:
	  (raw++)->ptr = **(تهی***) args;
	  بشکن;

	پیشفرض:
	  memcpy ((تهی*) raw->data, (تهی*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#اگر !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

تهی ffi_raw_call (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, ffi_raw *raw)
{
  تهی **avalue = (تهی**) alloca (cif->nargs * اندازه (تهی*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#اگر FFI_CLOSURES		/* base system provides closures */

ایستا تهی
ffi_translate_args (ffi_cif *cif, تهی *rvalue,
		    تهی **avalue, تهی *user_data)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_data);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  تهی (*fun)(ffi_cif*,تهی*,ffi_raw*,تهی*),
			  تهی *user_data,
			  تهی *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  اگر (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  بازگشت status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#اگر FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      تهی (*fun)(ffi_cif*,تهی*,ffi_raw*,تهی*),
		      تهی *user_data)
{
  بازگشت ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
