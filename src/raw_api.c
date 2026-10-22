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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਜੇ !FFI_NO_RAW_API

size_t
ffi_raw_ਮਾਪ (ffi_cif *cif)
{
  size_t ਨਤੀਜਾ = 0;
  ਪੂਰਨ i;

  ffi_type **at = cif->arg_types;

  ਲਈ (i = cif->nargs-1; i >= 0; i--, at++)
    {
#ਜੇ !FFI_NO_STRUCTS
      ਜੇ ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_ਸਦਿਸ਼)
	ਨਤੀਜਾ += FFI_ALIGN (ਆਕਾਰ (ਖਾਲੀ*), FFI_SIZEOF_ARG);
      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	ਨਤੀਜਾ += FFI_ALIGN ((*at)->ਮਾਪ, FFI_SIZEOF_ARG);
    }

  ਵਾਪਸ ਨਤੀਜਾ;
}


ਖਾਲੀ
ffi_raw_ਮੰਜ਼ਿਲ_ptrarray (ffi_cif *cif, ffi_raw *raw, ਖਾਲੀ **ਦਲੀਲਾਂ)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ i;
  ffi_type **tp = cif->arg_types;

#ਜੇ ਸ਼ਬਦ_BIGENDIAN

  ਲਈ (i = 0; i < cif->nargs; i++, tp++, ਦਲੀਲਾਂ++)
    {	  
      ਚੋਣ ((*tp)->type)
	{
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT8:
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ*) ((ਅੱਖਰ*)(raw++) + FFI_SIZEOF_ARG - 1);
	  ਰੋਕੋ;
	  
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT16:
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ*) ((ਅੱਖਰ*)(raw++) + FFI_SIZEOF_ARG - 2);
	  ਰੋਕੋ;

#ਜੇ FFI_SIZEOF_ARG >= 4	  
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ*) ((ਅੱਖਰ*)(raw++) + FFI_SIZEOF_ARG - 4);
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ
	
#ਜੇ !FFI_NO_STRUCTS
	ਹਾਲਤ FFI_TYPE_STRUCT:
	ਹਾਲਤ FFI_TYPE_ਸਦਿਸ਼:
	  *ਦਲੀਲਾਂ = (raw++)->ptr;
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  *ਦਲੀਲਾਂ = (raw++)->ptr;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ*) &(raw++)->ptr;
	  ਰੋਕੋ;
	  
	ਮੂਲ:
	  *ਦਲੀਲਾਂ = raw;
	  raw += FFI_ALIGN ((*tp)->ਮਾਪ, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#ਨਹੀਂਤਾਂ /* WORDS_BIGENDIAN */

#ਜੇ !PDP

  /* then assume little endian */
  ਲਈ (i = 0; i < cif->nargs; i++, tp++, ਦਲੀਲਾਂ++)
    {	  
#ਜੇ !FFI_NO_STRUCTS
      ਜੇ ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_ਸਦਿਸ਼)
	{
	  *ਦਲੀਲਾਂ = (raw++)->ptr;
	}
      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
      ਜੇ ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *ਦਲੀਲਾਂ = (raw++)->ptr;
	}
      ਨਹੀਂਤਾਂ
	{
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ*) raw;
	  raw += FFI_ALIGN ((*tp)->ਮਾਪ, ਆਕਾਰ (ਖਾਲੀ*)) / ਆਕਾਰ (ਖਾਲੀ*);
	}
    }

#ਨਹੀਂਤਾਂ
#ਗਲਤੀ "pdp endian not supported"
#ਜੇ_ਅੰਤ /* ! PDP */

#ਜੇ_ਅੰਤ /* WORDS_BIGENDIAN */
}

ਖਾਲੀ
ffi_ptrarray_ਮੰਜ਼ਿਲ_raw (ffi_cif *cif, ਖਾਲੀ **ਦਲੀਲਾਂ, ffi_raw *raw)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ i;
  ffi_type **tp = cif->arg_types;

  ਲਈ (i = 0; i < cif->nargs; i++, tp++, ਦਲੀਲਾਂ++)
    {	  
      ਚੋਣ ((*tp)->type)
	{
	ਹਾਲਤ FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*ਦਲੀਲਾਂ);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*ਦਲੀਲਾਂ);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*ਦਲੀਲਾਂ);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*ਦਲੀਲਾਂ);
	  ਰੋਕੋ;

#ਜੇ FFI_SIZEOF_ARG >= 4
	ਹਾਲਤ FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*ਦਲੀਲਾਂ);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*ਦਲੀਲਾਂ);
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

#ਜੇ !FFI_NO_STRUCTS
	ਹਾਲਤ FFI_TYPE_STRUCT:
	ਹਾਲਤ FFI_TYPE_ਸਦਿਸ਼:
	  (raw++)->ptr = *ਦਲੀਲਾਂ;
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *ਦਲੀਲਾਂ;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  (raw++)->ptr = **(ਖਾਲੀ***) ਦਲੀਲਾਂ;
	  ਰੋਕੋ;

	ਮੂਲ:
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਖਾਲੀ*) raw->ਅੰਕੜੇ, (ਖਾਲੀ*)*ਦਲੀਲਾਂ, (*tp)->ਮਾਪ);
	  raw += FFI_ALIGN ((*tp)->ਮਾਪ, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#ਜੇ !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

ਖਾਲੀ ffi_raw_call (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ffi_raw *raw)
{
  ਖਾਲੀ **avalue = (ਖਾਲੀ**) alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ*));
  ffi_raw_ਮੰਜ਼ਿਲ_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#ਜੇ FFI_CLOSURES		/* base system provides closures */

ਸਥਿਰ ਖਾਲੀ
ffi_translate_ਦਲੀਲਾਂ (ffi_cif *cif, ਖਾਲੀ *rvalue,
		    ਖਾਲੀ **avalue, ਖਾਲੀ *user_ਅੰਕੜੇ)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_ਮਾਪ (cif));
  ffi_raw_ਘੇਰਾ *cl = (ffi_raw_ਘੇਰਾ*)user_ਅੰਕੜੇ;

  ffi_ptrarray_ਮੰਜ਼ਿਲ_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_ਅੰਕੜੇ);
}

ffi_ਸਥਿਤੀ
ffi_prep_raw_ਘੇਰਾ_loc (ffi_raw_ਘੇਰਾ* cl,
			  ffi_cif *cif,
			  ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ffi_raw*,ਖਾਲੀ*),
			  ਖਾਲੀ *user_ਅੰਕੜੇ,
			  ਖਾਲੀ *codeloc)
{
  ffi_ਸਥਿਤੀ status;

  status = ffi_prep_ਘੇਰਾ_loc ((ffi_ਘੇਰਾ*) cl,
				 cif,
				 &ffi_translate_ਦਲੀਲਾਂ,
				 codeloc,
				 codeloc);
  ਜੇ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
    }

  ਵਾਪਸ status;
}

#ਜੇ_ਅੰਤ /* FFI_CLOSURES */
#ਜੇ_ਅੰਤ /* !FFI_NATIVE_RAW_API */

#ਜੇ FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_ਸਥਿਤੀ
ffi_prep_raw_ਘੇਰਾ (ffi_raw_ਘੇਰਾ* cl,
		      ffi_cif *cif,
		      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ffi_raw*,ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ)
{
  ਵਾਪਸ ffi_prep_raw_ਘੇਰਾ_loc (cl, cif, fun, user_ਅੰਕੜੇ, cl);
}

#ਜੇ_ਅੰਤ /* FFI_CLOSURES */

#ਜੇ_ਅੰਤ /* !FFI_NO_RAW_API */
