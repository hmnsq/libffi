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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಯದಿ !FFI_NO_RAW_API

size_t
ffi_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  ಪೂರ್ಣಾಙ್ಕ i;

  ffi_type **at = cif->arg_types;

  ಪ್ರತಿ (i = cif->nargs-1; i >= 0; i--, at++)
    {
#ಯದಿ !FFI_NO_STRUCTS
      ಯದಿ ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (ಪರಿಮಾಣ (ಶೂನ್ಯ*), FFI_SIZEOF_ARG);
      ಅನ್ಯಥಾ
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  ಪ್ರತ್ಯಾಗಮ result;
}


ಶೂನ್ಯ
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, ಶೂನ್ಯ **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#ಯದಿ ಪದಗಳು_BIGENDIAN

  ಪ್ರತಿ (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      ವಿಕಲ್ಪ ((*tp)->type)
	{
	ಪಕ್ಷ FFI_TYPE_UINT8:
	ಪಕ್ಷ FFI_TYPE_SINT8:
	  *args = (ಶೂನ್ಯ*) ((ವರ್ಣ*)(raw++) + FFI_SIZEOF_ARG - 1);
	  ಭಙ್ಗ;
	  
	ಪಕ್ಷ FFI_TYPE_UINT16:
	ಪಕ್ಷ FFI_TYPE_SINT16:
	  *args = (ಶೂನ್ಯ*) ((ವರ್ಣ*)(raw++) + FFI_SIZEOF_ARG - 2);
	  ಭಙ್ಗ;

#ಯದಿ FFI_SIZEOF_ARG >= 4	  
	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	  *args = (ಶೂನ್ಯ*) ((ವರ್ಣ*)(raw++) + FFI_SIZEOF_ARG - 4);
	  ಭಙ್ಗ;
#endif
	
#ಯದಿ !FFI_NO_STRUCTS
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	ಪಕ್ಷ FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_POINTER:
	  *args = (ಶೂನ್ಯ*) &(raw++)->ptr;
	  ಭಙ್ಗ;
	  
	ಸ್ವಭಾವ:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#ಅನ್ಯಥಾ /* WORDS_BIGENDIAN */

#ಯದಿ !PDP

  /* then assume little endian */
  ಪ್ರತಿ (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#ಯದಿ !FFI_NO_STRUCTS
      ಯದಿ ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      ಅನ್ಯಥಾ
#endif
      ಯದಿ ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      ಅನ್ಯಥಾ
	{
	  *args = (ಶೂನ್ಯ*) raw;
	  raw += FFI_ALIGN ((*tp)->size, ಪರಿಮಾಣ (ಶೂನ್ಯ*)) / ಪರಿಮಾಣ (ಶೂನ್ಯ*);
	}
    }

#ಅನ್ಯಥಾ
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

ಶೂನ್ಯ
ffi_ptrarray_to_raw (ffi_cif *cif, ಶೂನ್ಯ **args, ffi_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  ಪ್ರತಿ (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      ವಿಕಲ್ಪ ((*tp)->type)
	{
	ಪಕ್ಷ FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  ಭಙ್ಗ;

#ಯದಿ FFI_SIZEOF_ARG >= 4
	ಪಕ್ಷ FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  ಭಙ್ಗ;
#endif

#ಯದಿ !FFI_NO_STRUCTS
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	ಪಕ್ಷ FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_POINTER:
	  (raw++)->ptr = **(ಶೂನ್ಯ***) args;
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  memcpy ((ಶೂನ್ಯ*) raw->data, (ಶೂನ್ಯ*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#ಯದಿ !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

ಶೂನ್ಯ ffi_raw_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ffi_raw *raw)
{
  ಶೂನ್ಯ **avalue = (ಶೂನ್ಯ**) alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#ಯದಿ FFI_CLOSURES		/* base system provides closures */

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_translate_args (ffi_cif *cif, ಶೂನ್ಯ *rvalue,
		    ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *user_data)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_data);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ffi_raw*,ಶೂನ್ಯ*),
			  ಶೂನ್ಯ *user_data,
			  ಶೂನ್ಯ *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  ಯದಿ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  ಪ್ರತ್ಯಾಗಮ status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#ಯದಿ FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ffi_raw*,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data)
{
  ಪ್ರತ್ಯಾಗಮ ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
