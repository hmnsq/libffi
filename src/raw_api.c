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
ffi_raw_سائيز (ffi_cif *cif)
{
  size_t نتيجو = 0;
  صحيح i;

  ffi_type **at = cif->arg_types;

  لكل (i = cif->nargs-1; i >= 0; i--, at++)
    {
#إذا !FFI_NO_STRUCTS
      إذا ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_ويڪٽر)
	نتيجو += FFI_ALIGN (حجم (فراغ*), FFI_SIZEOF_ARG);
      وإلا
#جيڪڏهن_پورو
	نتيجو += FFI_ALIGN ((*at)->سائيز, FFI_SIZEOF_ARG);
    }

  أرجع نتيجو;
}


فراغ
ffi_raw_منزل_ptrarray (ffi_cif *cif, ffi_raw *raw, فراغ **دليلون)
{
  بنانشان i;
  ffi_type **tp = cif->arg_types;

#إذا لفظ_BIGENDIAN

  لكل (i = 0; i < cif->nargs; i++, tp++, دليلون++)
    {	  
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	  *دليلون = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 1);
	  اكسر;
	  
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	  *دليلون = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 2);
	  اكسر;

#إذا FFI_SIZEOF_ARG >= 4	  
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	  *دليلون = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 4);
	  اكسر;
#جيڪڏهن_پورو
	
#إذا !FFI_NO_STRUCTS
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_ويڪٽر:
	  *دليلون = (raw++)->ptr;
	  اكسر;
#جيڪڏهن_پورو

	حالة FFI_TYPE_COMPLEX:
	  *دليلون = (raw++)->ptr;
	  اكسر;

	حالة FFI_TYPE_اشارو:
	  *دليلون = (فراغ*) &(raw++)->ptr;
	  اكسر;
	  
	افتراضي:
	  *دليلون = raw;
	  raw += FFI_ALIGN ((*tp)->سائيز, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#وإلا /* WORDS_BIGENDIAN */

#إذا !PDP

  /* then assume little endian */
  لكل (i = 0; i < cif->nargs; i++, tp++, دليلون++)
    {	  
#إذا !FFI_NO_STRUCTS
      إذا ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_ويڪٽر)
	{
	  *دليلون = (raw++)->ptr;
	}
      وإلا
#جيڪڏهن_پورو
      إذا ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *دليلون = (raw++)->ptr;
	}
      وإلا
	{
	  *دليلون = (فراغ*) raw;
	  raw += FFI_ALIGN ((*tp)->سائيز, حجم (فراغ*)) / حجم (فراغ*);
	}
    }

#وإلا
#غلطي "pdp endian not supported"
#جيڪڏهن_پورو /* ! PDP */

#جيڪڏهن_پورو /* WORDS_BIGENDIAN */
}

فراغ
ffi_ptrarray_منزل_raw (ffi_cif *cif, فراغ **دليلون, ffi_raw *raw)
{
  بنانشان i;
  ffi_type **tp = cif->arg_types;

  لكل (i = 0; i < cif->nargs; i++, tp++, دليلون++)
    {	  
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*دليلون);
	  اكسر;

	حالة FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*دليلون);
	  اكسر;

	حالة FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*دليلون);
	  اكسر;

	حالة FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*دليلون);
	  اكسر;

#إذا FFI_SIZEOF_ARG >= 4
	حالة FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*دليلون);
	  اكسر;

	حالة FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*دليلون);
	  اكسر;
#جيڪڏهن_پورو

#إذا !FFI_NO_STRUCTS
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_ويڪٽر:
	  (raw++)->ptr = *دليلون;
	  اكسر;
#جيڪڏهن_پورو

	حالة FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *دليلون;
	  اكسر;

	حالة FFI_TYPE_اشارو:
	  (raw++)->ptr = **(فراغ***) دليلون;
	  اكسر;

	افتراضي:
	  يادگيري_نقل ((فراغ*) raw->معلومات, (فراغ*)*دليلون, (*tp)->سائيز);
	  raw += FFI_ALIGN ((*tp)->سائيز, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
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
ffi_translate_دليلون (ffi_cif *cif, فراغ *rvalue,
		    فراغ **avalue, فراغ *user_معلومات)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_سائيز (cif));
  ffi_raw_گهيرو *cl = (ffi_raw_گهيرو*)user_معلومات;

  ffi_ptrarray_منزل_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_معلومات);
}

ffi_حالت
ffi_prep_raw_گهيرو_loc (ffi_raw_گهيرو* cl,
			  ffi_cif *cif,
			  فراغ (*fun)(ffi_cif*,فراغ*,ffi_raw*,فراغ*),
			  فراغ *user_معلومات,
			  فراغ *codeloc)
{
  ffi_حالت status;

  status = ffi_prep_گهيرو_loc ((ffi_گهيرو*) cl,
				 cif,
				 &ffi_translate_دليلون,
				 codeloc,
				 codeloc);
  إذا (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_معلومات = user_معلومات;
    }

  أرجع status;
}

#جيڪڏهن_پورو /* FFI_CLOSURES */
#جيڪڏهن_پورو /* !FFI_NATIVE_RAW_API */

#إذا FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_حالت
ffi_prep_raw_گهيرو (ffi_raw_گهيرو* cl,
		      ffi_cif *cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,ffi_raw*,فراغ*),
		      فراغ *user_معلومات)
{
  أرجع ffi_prep_raw_گهيرو_loc (cl, cif, fun, user_معلومات, cl);
}

#جيڪڏهن_پورو /* FFI_CLOSURES */

#جيڪڏهن_پورو /* !FFI_NO_RAW_API */
