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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#எனில் !FFI_NO_RAW_API

size_t
ffi_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  முழு i;

  ffi_type **at = cif->arg_types;

  க்கு (i = cif->nargs-1; i >= 0; i--, at++)
    {
#எனில் !FFI_NO_STRUCTS
      எனில் ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (அளவு (வெறுமை*), FFI_SIZEOF_ARG);
      இல்லையெனில்
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  திரும்பு result;
}


வெறுமை
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, வெறுமை **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#எனில் சொற்கள்_BIGENDIAN

  க்கு (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      தேர்வு ((*tp)->type)
	{
	நிலை FFI_TYPE_UINT8:
	நிலை FFI_TYPE_SINT8:
	  *args = (வெறுமை*) ((எழுத்து*)(raw++) + FFI_SIZEOF_ARG - 1);
	  நிறுத்து;
	  
	நிலை FFI_TYPE_UINT16:
	நிலை FFI_TYPE_SINT16:
	  *args = (வெறுமை*) ((எழுத்து*)(raw++) + FFI_SIZEOF_ARG - 2);
	  நிறுத்து;

#எனில் FFI_SIZEOF_ARG >= 4	  
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_SINT32:
	  *args = (வெறுமை*) ((எழுத்து*)(raw++) + FFI_SIZEOF_ARG - 4);
	  நிறுத்து;
#endif
	
#எனில் !FFI_NO_STRUCTS
	நிலை FFI_TYPE_STRUCT:
	நிலை FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  நிறுத்து;

	நிலை FFI_TYPE_POINTER:
	  *args = (வெறுமை*) &(raw++)->ptr;
	  நிறுத்து;
	  
	இயல்பு:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#இல்லையெனில் /* WORDS_BIGENDIAN */

#எனில் !PDP

  /* then assume little endian */
  க்கு (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#எனில் !FFI_NO_STRUCTS
      எனில் ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      இல்லையெனில்
#endif
      எனில் ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      இல்லையெனில்
	{
	  *args = (வெறுமை*) raw;
	  raw += FFI_ALIGN ((*tp)->size, அளவு (வெறுமை*)) / அளவு (வெறுமை*);
	}
    }

#இல்லையெனில்
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

வெறுமை
ffi_ptrarray_to_raw (ffi_cif *cif, வெறுமை **args, ffi_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  க்கு (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      தேர்வு ((*tp)->type)
	{
	நிலை FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  நிறுத்து;

	நிலை FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  நிறுத்து;

	நிலை FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  நிறுத்து;

	நிலை FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  நிறுத்து;

#எனில் FFI_SIZEOF_ARG >= 4
	நிலை FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  நிறுத்து;

	நிலை FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  நிறுத்து;
#endif

#எனில் !FFI_NO_STRUCTS
	நிலை FFI_TYPE_STRUCT:
	நிலை FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  நிறுத்து;

	நிலை FFI_TYPE_POINTER:
	  (raw++)->ptr = **(வெறுமை***) args;
	  நிறுத்து;

	இயல்பு:
	  memcpy ((வெறுமை*) raw->data, (வெறுமை*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#எனில் !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

வெறுமை ffi_raw_call (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, ffi_raw *raw)
{
  வெறுமை **avalue = (வெறுமை**) alloca (cif->nargs * அளவு (வெறுமை*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#எனில் FFI_CLOSURES		/* base system provides closures */

நிலையான வெறுமை
ffi_translate_args (ffi_cif *cif, வெறுமை *rvalue,
		    வெறுமை **avalue, வெறுமை *user_data)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_data);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  வெறுமை (*fun)(ffi_cif*,வெறுமை*,ffi_raw*,வெறுமை*),
			  வெறுமை *user_data,
			  வெறுமை *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  எனில் (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  திரும்பு status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#எனில் FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      வெறுமை (*fun)(ffi_cif*,வெறுமை*,ffi_raw*,வெறுமை*),
		      வெறுமை *user_data)
{
  திரும்பு ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
