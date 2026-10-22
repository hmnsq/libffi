/* -----------------------------------------------------------------------
   java_raw_api.c - Copyright (c) 1999, 2007, 2008  Red Hat, Inc.

   Cloned from raw_api.c

   Raw_api.c author: Kresten Krab Thorup <krab@gnu.org>
   Java_raw_api.c author: Hans-J. Boehm <hboehm@hpl.hp.com>

   $Id $

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

/* This defines a Java- and 64-bit specific variant of the raw API.	*/
/* It assumes that "raw" argument blocks look like Java stacks on a	*/
/* 64-bit machine.  Arguments that can be stored in a single stack	*/
/* stack slots (longs, doubles) occupy 128 bits, but only the first	*/
/* 64 bits are actually used.						*/

#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdlib.h>

#اگر !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_سائز (ffi_cif *cif)
{
  size_t نتیجہ = 0;
  صحیح i;

  ffi_type **at = cif->arg_types;

  برائے (i = cif->nargs-1; i >= 0; i--, at++)
    {
      انتخاب((*at) -> type) {
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_DOUBLE:
	  نتیجہ += 2 * FFI_SIZEOF_JAVA_RAW;
	  توڑو;
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_سمتیہ:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	حالت FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	طےشدہ:
	  نتیجہ += FFI_SIZEOF_JAVA_RAW;
      }
    }

  واپس نتیجہ;
}


خالی
ffi_java_raw_مقصد_ptrarray (ffi_cif *cif, ffi_java_raw *raw, خالی **دلائل)
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
	  *دلائل = (خالی*) ((حرف*)(raw++) + 3);
	  توڑو;

	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT16:
	  *دلائل = (خالی*) ((حرف*)(raw++) + 2);
	  توڑو;

#اگر FFI_SIZEOF_JAVA_RAW == 8
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_DOUBLE:
	  *دلائل = (خالی *)raw;
	  raw += 2;
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_اشارہ_گر:
	  *دلائل = (خالی*) &(raw++)->ptr;
	  توڑو;

	حالت FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	طےشدہ:
	  *دلائل = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->سائز, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
	}
    }

#ورنہ /* WORDS_BIGENDIAN */

#اگر !PDP

  /* then assume little endian */
  برائے (i = 0; i < cif->nargs; i++, tp++, دلائل++)
    {
#اگر FFI_SIZEOF_JAVA_RAW == 8
      انتخاب((*tp)->type) {
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_DOUBLE:
	  *دلائل = (خالی*) raw;
	  raw += 2;
	  توڑو;
	حالت FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	طےشدہ:
	  *دلائل = (خالی*) raw++;
      }
#ورنہ /* FFI_SIZEOF_JAVA_RAW != 8 */
	*دلائل = (خالی*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->سائز, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
#اگر_ختم /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#ورنہ
#غلطی "pdp endian not supported"
#اگر_ختم /* ! PDP */

#اگر_ختم /* WORDS_BIGENDIAN */
}

خالی
ffi_java_ptrarray_مقصد_raw (ffi_cif *cif, خالی **دلائل, ffi_java_raw *raw)
{
  غیرعلامتی i;
  ffi_type **tp = cif->arg_types;

  برائے (i = 0; i < cif->nargs; i++, tp++, دلائل++)
    {
      انتخاب ((*tp)->type)
	{
	حالت FFI_TYPE_UINT8:
#اگر الفاظ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*دلائل);
#ورنہ
	  (raw++)->uint = *(UINT8*) (*دلائل);
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_SINT8:
#اگر الفاظ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*دلائل);
#ورنہ
	  (raw++)->sint = *(SINT8*) (*دلائل);
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_UINT16:
#اگر الفاظ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*دلائل);
#ورنہ
	  (raw++)->uint = *(UINT16*) (*دلائل);
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_SINT16:
#اگر الفاظ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*دلائل);
#ورنہ
	  (raw++)->sint = *(SINT16*) (*دلائل);
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_UINT32:
#اگر الفاظ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*دلائل);
#ورنہ
	  (raw++)->uint = *(UINT32*) (*دلائل);
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_SINT32:
#اگر الفاظ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*دلائل);
#ورنہ
	  (raw++)->sint = *(SINT32*) (*دلائل);
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*دلائل);
	  توڑو;

#اگر FFI_SIZEOF_JAVA_RAW == 8
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*دلائل);
	  raw += 2;
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_اشارہ_گر:
	  (raw++)->ptr = **(خالی***) دلائل;
	  توڑو;

	طےشدہ:
#اگر FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#ورنہ
	  حافظہ_نقل ((خالی*) raw->ڈیٹا, (خالی*)*دلائل, (*tp)->سائز);
	  raw +=
	    FFI_ALIGN ((*tp)->سائز, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
#اگر_ختم
	}
    }
}

#اگر !FFI_NATIVE_RAW_API

ساکن خالی
ffi_java_rvalue_مقصد_raw (ffi_cif *cif, خالی *rvalue)
{
#اگر الفاظ_BIGENDIAN && FFI_SIZEOF_ARG == 8
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_UINT16:
    حالت FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      توڑو;

    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_INT:
#اگر FFI_SIZEOF_JAVA_RAW == 4
    حالت FFI_TYPE_اشارہ_گر:
#اگر_ختم
      *(SINT64 *)rvalue <<= 32;
      توڑو;

    حالت FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    طےشدہ:
      توڑو;
    }
#اگر_ختم
}

ساکن خالی
ffi_java_raw_مقصد_rvalue (ffi_cif *cif, خالی *rvalue)
{
#اگر الفاظ_BIGENDIAN && FFI_SIZEOF_ARG == 8
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_UINT16:
    حالت FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      توڑو;

    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      توڑو;

    حالت FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    طےشدہ:
      توڑو;
    }
#اگر_ختم
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

خالی ffi_java_raw_call (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
			ffi_java_raw *raw)
{
  خالی **avalue = (خالی**) alloca (cif->nargs * حجم (خالی*));
  ffi_java_raw_مقصد_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_مقصد_raw (cif, rvalue);
}

#اگر FFI_CLOSURES		/* base system provides closures */

ساکن خالی
ffi_java_translate_دلائل (ffi_cif *cif, خالی *rvalue,
		    خالی **avalue, خالی *user_ڈیٹا)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_سائز (cif));
  ffi_raw_احاطہ *cl = (ffi_raw_احاطہ*)user_ڈیٹا;

  ffi_java_ptrarray_مقصد_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_ڈیٹا);
  ffi_java_raw_مقصد_rvalue (cif, rvalue);
}

ffi_حالت
ffi_prep_java_raw_احاطہ_loc (ffi_java_raw_احاطہ* cl,
			       ffi_cif *cif,
			       خالی (*fun)(ffi_cif*,خالی*,ffi_java_raw*,خالی*),
			       خالی *user_ڈیٹا,
			       خالی *codeloc)
{
  ffi_حالت status;

  status = ffi_prep_احاطہ_loc ((ffi_احاطہ*) cl,
				 cif,
				 &ffi_java_translate_دلائل,
				 codeloc,
				 codeloc);
  اگر (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_ڈیٹا = user_ڈیٹا;
    }

  واپس status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_حالت
ffi_prep_java_raw_احاطہ (ffi_java_raw_احاطہ* cl,
			   ffi_cif *cif,
			   خالی (*fun)(ffi_cif*,خالی*,ffi_java_raw*,خالی*),
			   خالی *user_ڈیٹا)
{
  واپس ffi_prep_java_raw_احاطہ_loc (cl, cif, fun, user_ڈیٹا, cl);
}

#اگر_ختم /* FFI_CLOSURES */
#اگر_ختم /* !FFI_NATIVE_RAW_API */
#اگر_ختم /* !NO_JAVA_RAW_API */
