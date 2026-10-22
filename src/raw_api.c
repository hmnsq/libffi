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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#अगर !FFI_NO_RAW_API

माप_प्रकार
ffi_raw_आकार (ffi_cif *cif)
{
  माप_प्रकार परिणाम = 0;
  पूर्णांक i;

  ffi_type **at = cif->arg_types;

  क्रम (i = cif->nargs-1; i >= 0; i--, at++)
    {
#अगर !FFI_NO_STRUCTS
      अगर ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	परिणाम += FFI_ALIGN (माप (व्योम*), FFI_SIZEOF_ARG);
      अन्यथा
#पूर्ण_अगर
	परिणाम += FFI_ALIGN ((*at)->आकार, FFI_SIZEOF_ARG);
    }

  वापस परिणाम;
}


व्योम
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, व्योम **args)
{
  अचिन्हित i;
  ffi_type **tp = cif->arg_types;

#अगर शब्दगण_BIGENDIAN

  क्रम (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      चयन ((*tp)->type)
	{
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT8:
	  *args = (व्योम*) ((अक्षर*)(raw++) + FFI_SIZEOF_ARG - 1);
	  अवरोध;
	  
	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT16:
	  *args = (व्योम*) ((अक्षर*)(raw++) + FFI_SIZEOF_ARG - 2);
	  अवरोध;

#अगर FFI_SIZEOF_ARG >= 4	  
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT32:
	  *args = (व्योम*) ((अक्षर*)(raw++) + FFI_SIZEOF_ARG - 4);
	  अवरोध;
#पूर्ण_अगर
	
#अगर !FFI_NO_STRUCTS
	हाल FFI_TYPE_STRUCT:
	हाल FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  अवरोध;

	हाल FFI_TYPE_POINTER:
	  *args = (व्योम*) &(raw++)->ptr;
	  अवरोध;
	  
	शेष:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->आकार, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#अन्यथा /* WORDS_BIGENDIAN */

#अगर !PDP

  /* then assume little endian */
  क्रम (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#अगर !FFI_NO_STRUCTS
      अगर ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      अन्यथा
#पूर्ण_अगर
      अगर ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      अन्यथा
	{
	  *args = (व्योम*) raw;
	  raw += FFI_ALIGN ((*tp)->आकार, माप (व्योम*)) / माप (व्योम*);
	}
    }

#अन्यथा
#त्रुटि "pdp endian not supported"
#पूर्ण_अगर /* ! PDP */

#पूर्ण_अगर /* WORDS_BIGENDIAN */
}

व्योम
ffi_ptrarray_to_raw (ffi_cif *cif, व्योम **args, ffi_raw *raw)
{
  अचिन्हित i;
  ffi_type **tp = cif->arg_types;

  क्रम (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      चयन ((*tp)->type)
	{
	हाल FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  अवरोध;

	हाल FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  अवरोध;

	हाल FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  अवरोध;

	हाल FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  अवरोध;

#अगर FFI_SIZEOF_ARG >= 4
	हाल FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  अवरोध;

	हाल FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  अवरोध;
#पूर्ण_अगर

#अगर !FFI_NO_STRUCTS
	हाल FFI_TYPE_STRUCT:
	हाल FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  अवरोध;

	हाल FFI_TYPE_POINTER:
	  (raw++)->ptr = **(व्योम***) args;
	  अवरोध;

	शेष:
	  स_नकल ((व्योम*) raw->तथ्य, (व्योम*)*args, (*tp)->आकार);
	  raw += FFI_ALIGN ((*tp)->आकार, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#अगर !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

व्योम ffi_raw_call (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, ffi_raw *raw)
{
  व्योम **avalue = (व्योम**) alloca (cif->nargs * माप (व्योम*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#अगर FFI_CLOSURES		/* base system provides closures */

अटल व्योम
ffi_translate_args (ffi_cif *cif, व्योम *rvalue,
		    व्योम **avalue, व्योम *user_तथ्य)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_आकार (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_तथ्य;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_तथ्य);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  व्योम (*fun)(ffi_cif*,व्योम*,ffi_raw*,व्योम*),
			  व्योम *user_तथ्य,
			  व्योम *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  अगर (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_तथ्य = user_तथ्य;
    }

  वापस status;
}

#पूर्ण_अगर /* FFI_CLOSURES */
#पूर्ण_अगर /* !FFI_NATIVE_RAW_API */

#अगर FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      व्योम (*fun)(ffi_cif*,व्योम*,ffi_raw*,व्योम*),
		      व्योम *user_तथ्य)
{
  वापस ffi_prep_raw_closure_loc (cl, cif, fun, user_तथ्य, cl);
}

#पूर्ण_अगर /* FFI_CLOSURES */

#पूर्ण_अगर /* !FFI_NO_RAW_API */
