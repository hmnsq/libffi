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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#જો !FFI_NO_RAW_API

size_t
ffi_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  પૂર્ણાંક i;

  ffi_type **at = cif->arg_types;

  માટે (i = cif->nargs-1; i >= 0; i--, at++)
    {
#જો !FFI_NO_STRUCTS
      જો ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (કદ (શૂન્ય*), FFI_SIZEOF_ARG);
      નહીંતર
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  પાછા result;
}


શૂન્ય
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, શૂન્ય **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#જો શબ્દો_BIGENDIAN

  માટે (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      પસંદગી ((*tp)->type)
	{
	સ્થિતિ FFI_TYPE_UINT8:
	સ્થિતિ FFI_TYPE_SINT8:
	  *args = (શૂન્ય*) ((અક્ષર*)(raw++) + FFI_SIZEOF_ARG - 1);
	  તોડો;
	  
	સ્થિતિ FFI_TYPE_UINT16:
	સ્થિતિ FFI_TYPE_SINT16:
	  *args = (શૂન્ય*) ((અક્ષર*)(raw++) + FFI_SIZEOF_ARG - 2);
	  તોડો;

#જો FFI_SIZEOF_ARG >= 4	  
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT32:
	  *args = (શૂન્ય*) ((અક્ષર*)(raw++) + FFI_SIZEOF_ARG - 4);
	  તોડો;
#endif
	
#જો !FFI_NO_STRUCTS
	સ્થિતિ FFI_TYPE_STRUCT:
	સ્થિતિ FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  તોડો;

	સ્થિતિ FFI_TYPE_POINTER:
	  *args = (શૂન્ય*) &(raw++)->ptr;
	  તોડો;
	  
	મૂળભૂત:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#નહીંતર /* WORDS_BIGENDIAN */

#જો !PDP

  /* then assume little endian */
  માટે (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#જો !FFI_NO_STRUCTS
      જો ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      નહીંતર
#endif
      જો ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      નહીંતર
	{
	  *args = (શૂન્ય*) raw;
	  raw += FFI_ALIGN ((*tp)->size, કદ (શૂન્ય*)) / કદ (શૂન્ય*);
	}
    }

#નહીંતર
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

શૂન્ય
ffi_ptrarray_to_raw (ffi_cif *cif, શૂન્ય **args, ffi_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  માટે (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      પસંદગી ((*tp)->type)
	{
	સ્થિતિ FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  તોડો;

#જો FFI_SIZEOF_ARG >= 4
	સ્થિતિ FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  તોડો;
#endif

#જો !FFI_NO_STRUCTS
	સ્થિતિ FFI_TYPE_STRUCT:
	સ્થિતિ FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  તોડો;

	સ્થિતિ FFI_TYPE_POINTER:
	  (raw++)->ptr = **(શૂન્ય***) args;
	  તોડો;

	મૂળભૂત:
	  memcpy ((શૂન્ય*) raw->data, (શૂન્ય*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#જો !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

શૂન્ય ffi_raw_call (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, ffi_raw *raw)
{
  શૂન્ય **avalue = (શૂન્ય**) alloca (cif->nargs * કદ (શૂન્ય*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#જો FFI_CLOSURES		/* base system provides closures */

સ્થિર શૂન્ય
ffi_translate_args (ffi_cif *cif, શૂન્ય *rvalue,
		    શૂન્ય **avalue, શૂન્ય *user_data)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_data);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,ffi_raw*,શૂન્ય*),
			  શૂન્ય *user_data,
			  શૂન્ય *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  જો (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  પાછા status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#જો FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,ffi_raw*,શૂન્ય*),
		      શૂન્ય *user_data)
{
  પાછા ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
