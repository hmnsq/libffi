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

#include <ffi.h>
#include <ffi_common.h>

#ᤀᤡ᤺ !FFI_NO_RAW_API

size_t
ffi_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  puur_naa_nka i;

  ffi_type **at = cif->arg_types;

  for (i = cif->nargs-1; i >= 0; i--, at++)
    {
#ᤀᤡ᤺ !FFI_NO_STRUCTS
      ᤀᤡ᤺ ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (parimaa_na (void*), FFI_SIZEOF_ARG);
      ᤀᤡ᤺ᤆᤡ
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  ᤀᤡᤴ result;
}


void
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, void **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#ᤀᤡ᤺ WORDS_BIGENDIAN

  for (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      switch ((*tp)->type)
	{
	pak_sa FFI_TYPE_UINT8:
	pak_sa FFI_TYPE_SINT8:
	  *args = (void*) ((var_na*)(raw++) + FFI_SIZEOF_ARG - 1);
	  bha_nga;
	  
	pak_sa FFI_TYPE_UINT16:
	pak_sa FFI_TYPE_SINT16:
	  *args = (void*) ((var_na*)(raw++) + FFI_SIZEOF_ARG - 2);
	  bha_nga;

#ᤀᤡ᤺ FFI_SIZEOF_ARG >= 4	  
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_SINT32:
	  *args = (void*) ((var_na*)(raw++) + FFI_SIZEOF_ARG - 4);
	  bha_nga;
#endif
	
#ᤀᤡ᤺ !FFI_NO_STRUCTS
	pak_sa FFI_TYPE_STRUCT:
	pak_sa FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  bha_nga;
#endif

	pak_sa FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  bha_nga;

	pak_sa FFI_TYPE_POINTER:
	  *args = (void*) &(raw++)->ptr;
	  bha_nga;
	  
	default:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#ᤀᤡ᤺ᤆᤡ /* WORDS_BIGENDIAN */

#ᤀᤡ᤺ !PDP

  /* then assume little endian */
  for (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#ᤀᤡ᤺ !FFI_NO_STRUCTS
      ᤀᤡ᤺ ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      ᤀᤡ᤺ᤆᤡ
#endif
      ᤀᤡ᤺ ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      ᤀᤡ᤺ᤆᤡ
	{
	  *args = (void*) raw;
	  raw += FFI_ALIGN ((*tp)->size, parimaa_na (void*)) / parimaa_na (void*);
	}
    }

#ᤀᤡ᤺ᤆᤡ
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

void
ffi_ptrarray_to_raw (ffi_cif *cif, void **args, ffi_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  for (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      switch ((*tp)->type)
	{
	pak_sa FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  bha_nga;

	pak_sa FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  bha_nga;

	pak_sa FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  bha_nga;

	pak_sa FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  bha_nga;

#ᤀᤡ᤺ FFI_SIZEOF_ARG >= 4
	pak_sa FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  bha_nga;

	pak_sa FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  bha_nga;
#endif

#ᤀᤡ᤺ !FFI_NO_STRUCTS
	pak_sa FFI_TYPE_STRUCT:
	pak_sa FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  bha_nga;
#endif

	pak_sa FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  bha_nga;

	pak_sa FFI_TYPE_POINTER:
	  (raw++)->ptr = **(void***) args;
	  bha_nga;

	default:
	  memcpy ((void*) raw->data, (void*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#ᤀᤡ᤺ !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

void ffi_raw_call (ffi_cif *cif, void (*fn)(void), void *rvalue, ffi_raw *raw)
{
  void **avalue = (void**) alloca (cif->nargs * parimaa_na (void*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#ᤀᤡ᤺ FFI_CLOSURES		/* base system provides closures */

static void
ffi_translate_args (ffi_cif *cif, void *rvalue,
		    void **avalue, void *user_data)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_data);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  void (*fun)(ffi_cif*,void*,ffi_raw*,void*),
			  void *user_data,
			  void *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  ᤀᤡ᤺ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  ᤀᤡᤴ status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#ᤀᤡ᤺ FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      void (*fun)(ffi_cif*,void*,ffi_raw*,void*),
		      void *user_data)
{
  ᤀᤡᤴ ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
