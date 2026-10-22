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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଯଦି !FFI_NO_RAW_API

size_t
ffi_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ffi_type **at = cif->arg_types;

  ପାଇଁ (i = cif->nargs-1; i >= 0; i--, at++)
    {
#ଯଦି !FFI_NO_STRUCTS
      ଯଦି ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (ଆକାର (ଶୂନ୍ୟ*), FFI_SIZEOF_ARG);
      ଅନ୍ୟଥା
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  ଫେରନ୍ତୁ result;
}


ଶୂନ୍ୟ
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, ଶୂନ୍ୟ **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#ଯଦି ଶବ୍ଦଗୁଡ଼ିକ_BIGENDIAN

  ପାଇଁ (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      ବାଛ ((*tp)->type)
	{
	ସ୍ଥିତି FFI_TYPE_UINT8:
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  *args = (ଶୂନ୍ୟ*) ((ଅକ୍ଷର*)(raw++) + FFI_SIZEOF_ARG - 1);
	  ଭାଙ୍ଗ;
	  
	ସ୍ଥିତି FFI_TYPE_UINT16:
	ସ୍ଥିତି FFI_TYPE_SINT16:
	  *args = (ଶୂନ୍ୟ*) ((ଅକ୍ଷର*)(raw++) + FFI_SIZEOF_ARG - 2);
	  ଭାଙ୍ଗ;

#ଯଦି FFI_SIZEOF_ARG >= 4	  
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	  *args = (ଶୂନ୍ୟ*) ((ଅକ୍ଷର*)(raw++) + FFI_SIZEOF_ARG - 4);
	  ଭାଙ୍ଗ;
#endif
	
#ଯଦି !FFI_NO_STRUCTS
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	ସ୍ଥିତି FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_POINTER:
	  *args = (ଶୂନ୍ୟ*) &(raw++)->ptr;
	  ଭାଙ୍ଗ;
	  
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#ଅନ୍ୟଥା /* WORDS_BIGENDIAN */

#ଯଦି !PDP

  /* then assume little endian */
  ପାଇଁ (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#ଯଦି !FFI_NO_STRUCTS
      ଯଦି ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      ଅନ୍ୟଥା
#endif
      ଯଦି ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      ଅନ୍ୟଥା
	{
	  *args = (ଶୂନ୍ୟ*) raw;
	  raw += FFI_ALIGN ((*tp)->size, ଆକାର (ଶୂନ୍ୟ*)) / ଆକାର (ଶୂନ୍ୟ*);
	}
    }

#ଅନ୍ୟଥା
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

ଶୂନ୍ୟ
ffi_ptrarray_to_raw (ffi_cif *cif, ଶୂନ୍ୟ **args, ffi_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  ପାଇଁ (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      ବାଛ ((*tp)->type)
	{
	ସ୍ଥିତି FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  ଭାଙ୍ଗ;

#ଯଦି FFI_SIZEOF_ARG >= 4
	ସ୍ଥିତି FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  ଭାଙ୍ଗ;
#endif

#ଯଦି !FFI_NO_STRUCTS
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	ସ୍ଥିତି FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_POINTER:
	  (raw++)->ptr = **(ଶୂନ୍ୟ***) args;
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  memcpy ((ଶୂନ୍ୟ*) raw->data, (ଶୂନ୍ୟ*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#ଯଦି !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

ଶୂନ୍ୟ ffi_raw_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ffi_raw *raw)
{
  ଶୂନ୍ୟ **avalue = (ଶୂନ୍ୟ**) alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#ଯଦି FFI_CLOSURES		/* base system provides closures */

ସ୍ଥିର ଶୂନ୍ୟ
ffi_translate_args (ffi_cif *cif, ଶୂନ୍ୟ *rvalue,
		    ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *user_data)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_data);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ffi_raw*,ଶୂନ୍ୟ*),
			  ଶୂନ୍ୟ *user_data,
			  ଶୂନ୍ୟ *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  ଯଦି (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  ଫେରନ୍ତୁ status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#ଯଦି FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ffi_raw*,ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data)
{
  ଫେରନ୍ତୁ ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
