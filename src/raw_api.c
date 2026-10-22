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

#إذا !FFI_NO_RAW_API

size_t
ffi_raw_اندازه (ffi_cif *cif)
{
  size_t پایله = 0;
  صحيح i;

  ffi_type **at = cif->arg_types;

  لكل (i = cif->nargs-1; i >= 0; i--, at++)
    {
#إذا !FFI_NO_STRUCTS
      إذا ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_ویکټور)
	پایله += FFI_ALIGN (حجم (فراغ*), FFI_SIZEOF_ARG);
      وإلا
#د_که_پای
	پایله += FFI_ALIGN ((*at)->اندازه, FFI_SIZEOF_ARG);
    }

  أرجع پایله;
}


فراغ
ffi_raw_منزل_ptrarray (ffi_cif *cif, ffi_raw *raw, فراغ **دلیلونه)
{
  بینښه i;
  ffi_type **tp = cif->arg_types;

#إذا ټکي_BIGENDIAN

  لكل (i = 0; i < cif->nargs; i++, tp++, دلیلونه++)
    {	  
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	  *دلیلونه = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 1);
	  اكسر;
	  
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	  *دلیلونه = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 2);
	  اكسر;

#إذا FFI_SIZEOF_ARG >= 4	  
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	  *دلیلونه = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 4);
	  اكسر;
#د_که_پای
	
#إذا !FFI_NO_STRUCTS
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_ویکټور:
	  *دلیلونه = (raw++)->ptr;
	  اكسر;
#د_که_پای

	حالة FFI_TYPE_COMPLEX:
	  *دلیلونه = (raw++)->ptr;
	  اكسر;

	حالة FFI_TYPE_اشاره_ګر:
	  *دلیلونه = (فراغ*) &(raw++)->ptr;
	  اكسر;
	  
	افتراضي:
	  *دلیلونه = raw;
	  raw += FFI_ALIGN ((*tp)->اندازه, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#وإلا /* WORDS_BIGENDIAN */

#إذا !PDP

  /* then assume little endian */
  لكل (i = 0; i < cif->nargs; i++, tp++, دلیلونه++)
    {	  
#إذا !FFI_NO_STRUCTS
      إذا ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_ویکټور)
	{
	  *دلیلونه = (raw++)->ptr;
	}
      وإلا
#د_که_پای
      إذا ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *دلیلونه = (raw++)->ptr;
	}
      وإلا
	{
	  *دلیلونه = (فراغ*) raw;
	  raw += FFI_ALIGN ((*tp)->اندازه, حجم (فراغ*)) / حجم (فراغ*);
	}
    }

#وإلا
#تېروتنه "pdp endian not supported"
#د_که_پای /* ! PDP */

#د_که_پای /* WORDS_BIGENDIAN */
}

فراغ
ffi_ptrarray_منزل_raw (ffi_cif *cif, فراغ **دلیلونه, ffi_raw *raw)
{
  بینښه i;
  ffi_type **tp = cif->arg_types;

  لكل (i = 0; i < cif->nargs; i++, tp++, دلیلونه++)
    {	  
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*دلیلونه);
	  اكسر;

	حالة FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*دلیلونه);
	  اكسر;

	حالة FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*دلیلونه);
	  اكسر;

	حالة FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*دلیلونه);
	  اكسر;

#إذا FFI_SIZEOF_ARG >= 4
	حالة FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*دلیلونه);
	  اكسر;

	حالة FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*دلیلونه);
	  اكسر;
#د_که_پای

#إذا !FFI_NO_STRUCTS
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_ویکټور:
	  (raw++)->ptr = *دلیلونه;
	  اكسر;
#د_که_پای

	حالة FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *دلیلونه;
	  اكسر;

	حالة FFI_TYPE_اشاره_ګر:
	  (raw++)->ptr = **(فراغ***) دلیلونه;
	  اكسر;

	افتراضي:
	  حافظه_نقل ((فراغ*) raw->معلومات, (فراغ*)*دلیلونه, (*tp)->اندازه);
	  raw += FFI_ALIGN ((*tp)->اندازه, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#إذا !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

فراغ ffi_raw_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, ffi_raw *raw)
{
  فراغ **avalue = (فراغ**) alloca (cif->nargs * حجم (فراغ*));
  ffi_raw_منزل_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#إذا FFI_CLOSURES		/* base system provides closures */

ساكن فراغ
ffi_translate_دلیلونه (ffi_cif *cif, فراغ *rvalue,
		    فراغ **avalue, فراغ *user_معلومات)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_اندازه (cif));
  ffi_raw_تړون *cl = (ffi_raw_تړون*)user_معلومات;

  ffi_ptrarray_منزل_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_معلومات);
}

ffi_حالت
ffi_prep_raw_تړون_loc (ffi_raw_تړون* cl,
			  ffi_cif *cif,
			  فراغ (*fun)(ffi_cif*,فراغ*,ffi_raw*,فراغ*),
			  فراغ *user_معلومات,
			  فراغ *codeloc)
{
  ffi_حالت status;

  status = ffi_prep_تړون_loc ((ffi_تړون*) cl,
				 cif,
				 &ffi_translate_دلیلونه,
				 codeloc,
				 codeloc);
  إذا (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_معلومات = user_معلومات;
    }

  أرجع status;
}

#د_که_پای /* FFI_CLOSURES */
#د_که_پای /* !FFI_NATIVE_RAW_API */

#إذا FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_حالت
ffi_prep_raw_تړون (ffi_raw_تړون* cl,
		      ffi_cif *cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,ffi_raw*,فراغ*),
		      فراغ *user_معلومات)
{
  أرجع ffi_prep_raw_تړون_loc (cl, cif, fun, user_معلومات, cl);
}

#د_که_پای /* FFI_CLOSURES */

#د_که_پای /* !FFI_NO_RAW_API */
