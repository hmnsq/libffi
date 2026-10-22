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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#যদি !FFI_NO_RAW_API

size_t
ffi_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  পূর্ণাঙ্ক i;

  ffi_type **at = cif->arg_types;

  প্রতি (i = cif->nargs-1; i >= 0; i--, at++)
    {
#যদি !FFI_NO_STRUCTS
      যদি ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (পরিমাণ (শূন্য*), FFI_SIZEOF_ARG);
      অন্যথা
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  প্রত্যাগম result;
}


শূন্য
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, শূন্য **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#যদি শব্দগুলি_BIGENDIAN

  প্রতি (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      বিকল্প ((*tp)->type)
	{
	পক্ষ FFI_TYPE_UINT8:
	পক্ষ FFI_TYPE_SINT8:
	  *args = (শূন্য*) ((বর্ণ*)(raw++) + FFI_SIZEOF_ARG - 1);
	  ভঙ্গ;
	  
	পক্ষ FFI_TYPE_UINT16:
	পক্ষ FFI_TYPE_SINT16:
	  *args = (শূন্য*) ((বর্ণ*)(raw++) + FFI_SIZEOF_ARG - 2);
	  ভঙ্গ;

#যদি FFI_SIZEOF_ARG >= 4	  
	পক্ষ FFI_TYPE_UINT32:
	পক্ষ FFI_TYPE_SINT32:
	  *args = (শূন্য*) ((বর্ণ*)(raw++) + FFI_SIZEOF_ARG - 4);
	  ভঙ্গ;
#endif
	
#যদি !FFI_NO_STRUCTS
	পক্ষ FFI_TYPE_STRUCT:
	পক্ষ FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_POINTER:
	  *args = (শূন্য*) &(raw++)->ptr;
	  ভঙ্গ;
	  
	স্বভাব:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#অন্যথা /* WORDS_BIGENDIAN */

#যদি !PDP

  /* then assume little endian */
  প্রতি (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#যদি !FFI_NO_STRUCTS
      যদি ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      অন্যথা
#endif
      যদি ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      অন্যথা
	{
	  *args = (শূন্য*) raw;
	  raw += FFI_ALIGN ((*tp)->size, পরিমাণ (শূন্য*)) / পরিমাণ (শূন্য*);
	}
    }

#অন্যথা
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

শূন্য
ffi_ptrarray_to_raw (ffi_cif *cif, শূন্য **args, ffi_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  প্রতি (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      বিকল্প ((*tp)->type)
	{
	পক্ষ FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  ভঙ্গ;

#যদি FFI_SIZEOF_ARG >= 4
	পক্ষ FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  ভঙ্গ;
#endif

#যদি !FFI_NO_STRUCTS
	পক্ষ FFI_TYPE_STRUCT:
	পক্ষ FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_POINTER:
	  (raw++)->ptr = **(শূন্য***) args;
	  ভঙ্গ;

	স্বভাব:
	  memcpy ((শূন্য*) raw->data, (শূন্য*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#যদি !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

শূন্য ffi_raw_call (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, ffi_raw *raw)
{
  শূন্য **avalue = (শূন্য**) alloca (cif->nargs * পরিমাণ (শূন্য*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#যদি FFI_CLOSURES		/* base system provides closures */

স্থির শূন্য
ffi_translate_args (ffi_cif *cif, শূন্য *rvalue,
		    শূন্য **avalue, শূন্য *user_data)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_data);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  শূন্য (*fun)(ffi_cif*,শূন্য*,ffi_raw*,শূন্য*),
			  শূন্য *user_data,
			  শূন্য *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  যদি (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  প্রত্যাগম status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#যদি FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      শূন্য (*fun)(ffi_cif*,শূন্য*,ffi_raw*,শূন্য*),
		      শূন্য *user_data)
{
  প্রত্যাগম ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
