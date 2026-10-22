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
ffi_raw_ناپ (ffi_cif *cif)
{
  size_t نتیجہ = 0;
  صحيح i;

  ffi_type **at = cif->arg_types;

  لكل (i = cif->nargs-1; i >= 0; i--, at++)
    {
#إذا !FFI_NO_STRUCTS
      إذا ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_سمتیہ)
	نتیجہ += FFI_ALIGN (حجم (فراغ*), FFI_SIZEOF_ARG);
      وإلا
#جے_انت
	نتیجہ += FFI_ALIGN ((*at)->ناپ, FFI_SIZEOF_ARG);
    }

  أرجع نتیجہ;
}


فراغ
ffi_raw_منزل_ptrarray (ffi_cif *cif, ffi_raw *raw, فراغ **دلیلاں)
{
  بنانشانی i;
  ffi_type **tp = cif->arg_types;

#إذا الفاظ_BIGENDIAN

  لكل (i = 0; i < cif->nargs; i++, tp++, دلیلاں++)
    {	  
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	  *دلیلاں = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 1);
	  اكسر;
	  
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	  *دلیلاں = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 2);
	  اكسر;

#إذا FFI_SIZEOF_ARG >= 4	  
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	  *دلیلاں = (فراغ*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 4);
	  اكسر;
#جے_انت
	
#إذا !FFI_NO_STRUCTS
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_سمتیہ:
	  *دلیلاں = (raw++)->ptr;
	  اكسر;
#جے_انت

	حالة FFI_TYPE_COMPLEX:
	  *دلیلاں = (raw++)->ptr;
	  اكسر;

	حالة FFI_TYPE_اشارہ_گر:
	  *دلیلاں = (فراغ*) &(raw++)->ptr;
	  اكسر;
	  
	افتراضي:
	  *دلیلاں = raw;
	  raw += FFI_ALIGN ((*tp)->ناپ, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#وإلا /* WORDS_BIGENDIAN */

#إذا !PDP

  /* then assume little endian */
  لكل (i = 0; i < cif->nargs; i++, tp++, دلیلاں++)
    {	  
#إذا !FFI_NO_STRUCTS
      إذا ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_سمتیہ)
	{
	  *دلیلاں = (raw++)->ptr;
	}
      وإلا
#جے_انت
      إذا ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *دلیلاں = (raw++)->ptr;
	}
      وإلا
	{
	  *دلیلاں = (فراغ*) raw;
	  raw += FFI_ALIGN ((*tp)->ناپ, حجم (فراغ*)) / حجم (فراغ*);
	}
    }

#وإلا
#غلطی "pdp endian not supported"
#جے_انت /* ! PDP */

#جے_انت /* WORDS_BIGENDIAN */
}

فراغ
ffi_ptrarray_منزل_raw (ffi_cif *cif, فراغ **دلیلاں, ffi_raw *raw)
{
  بنانشانی i;
  ffi_type **tp = cif->arg_types;

  لكل (i = 0; i < cif->nargs; i++, tp++, دلیلاں++)
    {	  
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*دلیلاں);
	  اكسر;

	حالة FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*دلیلاں);
	  اكسر;

	حالة FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*دلیلاں);
	  اكسر;

	حالة FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*دلیلاں);
	  اكسر;

#إذا FFI_SIZEOF_ARG >= 4
	حالة FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*دلیلاں);
	  اكسر;

	حالة FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*دلیلاں);
	  اكسر;
#جے_انت

#إذا !FFI_NO_STRUCTS
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_سمتیہ:
	  (raw++)->ptr = *دلیلاں;
	  اكسر;
#جے_انت

	حالة FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *دلیلاں;
	  اكسر;

	حالة FFI_TYPE_اشارہ_گر:
	  (raw++)->ptr = **(فراغ***) دلیلاں;
	  اكسر;

	افتراضي:
	  حافظہ_نقل ((فراغ*) raw->انکڑے, (فراغ*)*دلیلاں, (*tp)->ناپ);
	  raw += FFI_ALIGN ((*tp)->ناپ, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
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
ffi_translate_دلیلاں (ffi_cif *cif, فراغ *rvalue,
		    فراغ **avalue, فراغ *user_انکڑے)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_ناپ (cif));
  ffi_raw_گھیرا *cl = (ffi_raw_گھیرا*)user_انکڑے;

  ffi_ptrarray_منزل_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_انکڑے);
}

ffi_حالت
ffi_prep_raw_گھیرا_loc (ffi_raw_گھیرا* cl,
			  ffi_cif *cif,
			  فراغ (*fun)(ffi_cif*,فراغ*,ffi_raw*,فراغ*),
			  فراغ *user_انکڑے,
			  فراغ *codeloc)
{
  ffi_حالت status;

  status = ffi_prep_گھیرا_loc ((ffi_گھیرا*) cl,
				 cif,
				 &ffi_translate_دلیلاں,
				 codeloc,
				 codeloc);
  إذا (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_انکڑے = user_انکڑے;
    }

  أرجع status;
}

#جے_انت /* FFI_CLOSURES */
#جے_انت /* !FFI_NATIVE_RAW_API */

#إذا FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_حالت
ffi_prep_raw_گھیرا (ffi_raw_گھیرا* cl,
		      ffi_cif *cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,ffi_raw*,فراغ*),
		      فراغ *user_انکڑے)
{
  أرجع ffi_prep_raw_گھیرا_loc (cl, cif, fun, user_انکڑے, cl);
}

#جے_انت /* FFI_CLOSURES */

#جے_انت /* !FFI_NO_RAW_API */
