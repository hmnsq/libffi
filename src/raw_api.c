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

#شامل <ffi.h>
#شامل <ffi_common.h>

#اگر !FFI_NO_RAW_API

size_t
ffi_raw_سائز (ffi_cif *cif)
{
  size_t نتیجہ = 0;
  صحیح i;

  ffi_type **at = cif->arg_types;

  برائے (i = cif->nargs-1; i >= 0; i--, at++)
    {
#اگر !FFI_NO_STRUCTS
      اگر ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_سمتیہ)
	نتیجہ += FFI_ALIGN (حجم (خالی*), FFI_SIZEOF_ARG);
      ورنہ
#اگر_ختم
	نتیجہ += FFI_ALIGN ((*at)->سائز, FFI_SIZEOF_ARG);
    }

  واپس نتیجہ;
}


خالی
ffi_raw_مقصد_ptrarray (ffi_cif *cif, ffi_raw *raw, خالی **دلائل)
{
  غیرعلامتی i;
  ffi_type **tp = cif->arg_types;

#اگر الفاظ_BIGENDIAN

  برائے (i = 0; i < cif->nargs; i++, tp++, دلائل++)
    {	  
      انتخاب ((*tp)->type)
	{
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT8:
	  *دلائل = (خالی*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 1);
	  توڑو;
	  
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT16:
	  *دلائل = (خالی*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 2);
	  توڑو;

#اگر FFI_SIZEOF_ARG >= 4	  
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	  *دلائل = (خالی*) ((حرف*)(raw++) + FFI_SIZEOF_ARG - 4);
	  توڑو;
#اگر_ختم
	
#اگر !FFI_NO_STRUCTS
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_سمتیہ:
	  *دلائل = (raw++)->ptr;
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_COMPLEX:
	  *دلائل = (raw++)->ptr;
	  توڑو;

	حالت FFI_TYPE_اشارہ_گر:
	  *دلائل = (خالی*) &(raw++)->ptr;
	  توڑو;
	  
	طےشدہ:
	  *دلائل = raw;
	  raw += FFI_ALIGN ((*tp)->سائز, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#ورنہ /* WORDS_BIGENDIAN */

#اگر !PDP

  /* then assume little endian */
  برائے (i = 0; i < cif->nargs; i++, tp++, دلائل++)
    {	  
#اگر !FFI_NO_STRUCTS
      اگر ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_سمتیہ)
	{
	  *دلائل = (raw++)->ptr;
	}
      ورنہ
#اگر_ختم
      اگر ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *دلائل = (raw++)->ptr;
	}
      ورنہ
	{
	  *دلائل = (خالی*) raw;
	  raw += FFI_ALIGN ((*tp)->سائز, حجم (خالی*)) / حجم (خالی*);
	}
    }

#ورنہ
#غلطی "pdp endian not supported"
#اگر_ختم /* ! PDP */

#اگر_ختم /* WORDS_BIGENDIAN */
}

خالی
ffi_ptrarray_مقصد_raw (ffi_cif *cif, خالی **دلائل, ffi_raw *raw)
{
  غیرعلامتی i;
  ffi_type **tp = cif->arg_types;

  برائے (i = 0; i < cif->nargs; i++, tp++, دلائل++)
    {	  
      انتخاب ((*tp)->type)
	{
	حالت FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*دلائل);
	  توڑو;

	حالت FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*دلائل);
	  توڑو;

	حالت FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*دلائل);
	  توڑو;

	حالت FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*دلائل);
	  توڑو;

#اگر FFI_SIZEOF_ARG >= 4
	حالت FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*دلائل);
	  توڑو;

	حالت FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*دلائل);
	  توڑو;
#اگر_ختم

#اگر !FFI_NO_STRUCTS
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_سمتیہ:
	  (raw++)->ptr = *دلائل;
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *دلائل;
	  توڑو;

	حالت FFI_TYPE_اشارہ_گر:
	  (raw++)->ptr = **(خالی***) دلائل;
	  توڑو;

	طےشدہ:
	  حافظہ_نقل ((خالی*) raw->ڈیٹا, (خالی*)*دلائل, (*tp)->سائز);
	  raw += FFI_ALIGN ((*tp)->سائز, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#اگر !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

خالی ffi_raw_call (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, ffi_raw *raw)
{
  خالی **avalue = (خالی**) alloca (cif->nargs * حجم (خالی*));
  ffi_raw_مقصد_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#اگر FFI_CLOSURES		/* base system provides closures */

ساکن خالی
ffi_translate_دلائل (ffi_cif *cif, خالی *rvalue,
		    خالی **avalue, خالی *user_ڈیٹا)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_سائز (cif));
  ffi_raw_احاطہ *cl = (ffi_raw_احاطہ*)user_ڈیٹا;

  ffi_ptrarray_مقصد_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_ڈیٹا);
}

ffi_حالت
ffi_prep_raw_احاطہ_loc (ffi_raw_احاطہ* cl,
			  ffi_cif *cif,
			  خالی (*fun)(ffi_cif*,خالی*,ffi_raw*,خالی*),
			  خالی *user_ڈیٹا,
			  خالی *codeloc)
{
  ffi_حالت status;

  status = ffi_prep_احاطہ_loc ((ffi_احاطہ*) cl,
				 cif,
				 &ffi_translate_دلائل,
				 codeloc,
				 codeloc);
  اگر (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_ڈیٹا = user_ڈیٹا;
    }

  واپس status;
}

#اگر_ختم /* FFI_CLOSURES */
#اگر_ختم /* !FFI_NATIVE_RAW_API */

#اگر FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_حالت
ffi_prep_raw_احاطہ (ffi_raw_احاطہ* cl,
		      ffi_cif *cif,
		      خالی (*fun)(ffi_cif*,خالی*,ffi_raw*,خالی*),
		      خالی *user_ڈیٹا)
{
  واپس ffi_prep_raw_احاطہ_loc (cl, cif, fun, user_ڈیٹا, cl);
}

#اگر_ختم /* FFI_CLOSURES */

#اگر_ختم /* !FFI_NO_RAW_API */
