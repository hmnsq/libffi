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

#जर !FFI_NO_RAW_API

माप_प्रकार
ffi_raw_size (ffi_cif *cif)
{
  माप_प्रकार result = 0;
  पूर्णांक i;

  ffi_type **at = cif->arg_types;

  साठी (i = cif->nargs-1; i >= 0; i--, at++)
    {
#जर !FFI_NO_STRUCTS
      जर ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (आकार (शून्य*), FFI_SIZEOF_ARG);
      अन्यथा
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  परत result;
}


शून्य
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, शून्य **args)
{
  अचिन्हित i;
  ffi_type **tp = cif->arg_types;

#जर शब्द_BIGENDIAN

  साठी (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      निवड ((*tp)->type)
	{
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT8:
	  *args = (शून्य*) ((अक्षर*)(raw++) + FFI_SIZEOF_ARG - 1);
	  थांबा;
	  
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT16:
	  *args = (शून्य*) ((अक्षर*)(raw++) + FFI_SIZEOF_ARG - 2);
	  थांबा;

#जर FFI_SIZEOF_ARG >= 4	  
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT32:
	  *args = (शून्य*) ((अक्षर*)(raw++) + FFI_SIZEOF_ARG - 4);
	  थांबा;
#endif
	
#जर !FFI_NO_STRUCTS
	स्थिती FFI_TYPE_STRUCT:
	स्थिती FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  थांबा;
#endif

	स्थिती FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  थांबा;

	स्थिती FFI_TYPE_POINTER:
	  *args = (शून्य*) &(raw++)->ptr;
	  थांबा;
	  
	पूर्वनिर्धारित:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#अन्यथा /* WORDS_BIGENDIAN */

#जर !PDP

  /* then assume little endian */
  साठी (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#जर !FFI_NO_STRUCTS
      जर ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      अन्यथा
#endif
      जर ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      अन्यथा
	{
	  *args = (शून्य*) raw;
	  raw += FFI_ALIGN ((*tp)->size, आकार (शून्य*)) / आकार (शून्य*);
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
  अचिन्हित i;
  ffi_type **tp = cif->arg_types;

  साठी (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      निवड ((*tp)->type)
	{
	स्थिती FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  थांबा;

	स्थिती FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  थांबा;

	स्थिती FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  थांबा;

	स्थिती FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  थांबा;

#जर FFI_SIZEOF_ARG >= 4
	स्थिती FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  थांबा;

	स्थिती FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  थांबा;
#endif

#जर !FFI_NO_STRUCTS
	स्थिती FFI_TYPE_STRUCT:
	स्थिती FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  थांबा;
#endif

	स्थिती FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  थांबा;

	स्थिती FFI_TYPE_POINTER:
	  (raw++)->ptr = **(शून्य***) args;
	  थांबा;

	पूर्वनिर्धारित:
	  स_नकल ((शून्य*) raw->data, (शून्य*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#जर !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

शून्य ffi_raw_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, ffi_raw *raw)
{
  शून्य **avalue = (शून्य**) alloca (cif->nargs * आकार (शून्य*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#जर FFI_CLOSURES		/* base system provides closures */

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
  जर (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  परत status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#जर FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,ffi_raw*,शून्य*),
		      शून्य *user_data)
{
  परत ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
