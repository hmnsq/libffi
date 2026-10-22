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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#𐡄𐡍 !FFI_NO_RAW_API

size_t
ffi_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  صحيح i;

  ffi_type **at = cif->arg_types;

  لكل (i = cif->nargs-1; i >= 0; i--, at++)
    {
#𐡄𐡍 !FFI_NO_STRUCTS
      𐡄𐡍 ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_VECTOR)
	result += FFI_ALIGN (حجم (فراغ*), FFI_SIZEOF_ARG);
      وإلا
#endif
	result += FFI_ALIGN ((*at)->size, FFI_SIZEOF_ARG);
    }

  أرجع result;
}


فراغ
ffi_raw_to_ptrarray (ffi_cif *cif, ffi_raw *raw, فراغ **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#𐡄𐡍 WORDS_BIGENDIAN

  لكل (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	  *args = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 1);
	  اكسر;
	  
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	  *args = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 2);
	  اكسر;

#𐡄𐡍 FFI_SIZEOF_ARG >= 4	  
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	  *args = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 4);
	  اكسر;
#endif
	
#𐡄𐡍 !FFI_NO_STRUCTS
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_VECTOR:
	  *args = (raw++)->ptr;
	  اكسر;
#endif

	حالة FFI_TYPE_COMPLEX:
	  *args = (raw++)->ptr;
	  اكسر;

	حالة FFI_TYPE_POINTER:
	  *args = (فراغ*) &(raw++)->ptr;
	  اكسر;
	  
	افتراضي:
	  *args = raw;
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#وإلا /* WORDS_BIGENDIAN */

#𐡄𐡍 !PDP

  /* then assume little endian */
  لكل (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
#𐡄𐡍 !FFI_NO_STRUCTS
      𐡄𐡍 ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_VECTOR)
	{
	  *args = (raw++)->ptr;
	}
      وإلا
#endif
      𐡄𐡍 ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *args = (raw++)->ptr;
	}
      وإلا
	{
	  *args = (فراغ*) raw;
	  raw += FFI_ALIGN ((*tp)->size, حجم (فراغ*)) / حجم (فراغ*);
	}
    }

#وإلا
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

فراغ
ffi_ptrarray_to_raw (ffi_cif *cif, فراغ **args, ffi_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  لكل (i = 0; i < cif->nargs; i++, tp++, args++)
    {	  
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*args);
	  اكسر;

	حالة FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*args);
	  اكسر;

	حالة FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*args);
	  اكسر;

	حالة FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*args);
	  اكسر;

#𐡄𐡍 FFI_SIZEOF_ARG >= 4
	حالة FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*args);
	  اكسر;

	حالة FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*args);
	  اكسر;
#endif

#𐡄𐡍 !FFI_NO_STRUCTS
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_VECTOR:
	  (raw++)->ptr = *args;
	  اكسر;
#endif

	حالة FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *args;
	  اكسر;

	حالة FFI_TYPE_POINTER:
	  (raw++)->ptr = **(فراغ***) args;
	  اكسر;

	افتراضي:
	  memcpy ((فراغ*) raw->data, (فراغ*)*args, (*tp)->size);
	  raw += FFI_ALIGN ((*tp)->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#𐡄𐡍 !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

فراغ ffi_raw_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, ffi_raw *raw)
{
  فراغ **avalue = (فراغ**) alloca (cif->nargs * حجم (فراغ*));
  ffi_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#𐡄𐡍 FFI_CLOSURES		/* base system provides closures */

ساكن فراغ
ffi_translate_args (ffi_cif *cif, فراغ *rvalue,
		    فراغ **avalue, فراغ *user_data)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_data);
}

ffi_status
ffi_prep_raw_closure_loc (ffi_raw_closure* cl,
			  ffi_cif *cif,
			  فراغ (*fun)(ffi_cif*,فراغ*,ffi_raw*,فراغ*),
			  فراغ *user_data,
			  فراغ *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_translate_args,
				 codeloc,
				 codeloc);
  𐡄𐡍 (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  أرجع status;
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */

#𐡄𐡍 FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_raw_closure (ffi_raw_closure* cl,
		      ffi_cif *cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,ffi_raw*,فراغ*),
		      فراغ *user_data)
{
  أرجع ffi_prep_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */

#endif /* !FFI_NO_RAW_API */
