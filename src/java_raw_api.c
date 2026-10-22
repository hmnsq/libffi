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

#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <stdlib.h>

#إذا !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_ناپ (ffi_cif *cif)
{
  size_t نتیجہ = 0;
  صحيح i;

  ffi_type **at = cif->arg_types;

  لكل (i = cif->nargs-1; i >= 0; i--, at++)
    {
      بدل((*at) -> type) {
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  نتیجہ += 2 * FFI_SIZEOF_JAVA_RAW;
	  اكسر;
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_سمتیہ:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	افتراضي:
	  نتیجہ += FFI_SIZEOF_JAVA_RAW;
      }
    }

  أرجع نتیجہ;
}


فراغ
ffi_java_raw_منزل_ptrarray (ffi_cif *cif, ffi_java_raw *raw, فراغ **دلیلاں)
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
	  *دلیلاں = (فراغ*) ((حرف*)(raw++) + 3);
	  اكسر;

	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	  *دلیلاں = (فراغ*) ((حرف*)(raw++) + 2);
	  اكسر;

#إذا FFI_SIZEOF_JAVA_RAW == 8
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  *دلیلاں = (فراغ *)raw;
	  raw += 2;
	  اكسر;
#جے_انت

	حالة FFI_TYPE_اشارہ_گر:
	  *دلیلاں = (فراغ*) &(raw++)->ptr;
	  اكسر;

	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	افتراضي:
	  *دلیلاں = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->ناپ, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
	}
    }

#وإلا /* WORDS_BIGENDIAN */

#إذا !PDP

  /* then assume little endian */
  لكل (i = 0; i < cif->nargs; i++, tp++, دلیلاں++)
    {
#إذا FFI_SIZEOF_JAVA_RAW == 8
      بدل((*tp)->type) {
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  *دلیلاں = (فراغ*) raw;
	  raw += 2;
	  اكسر;
	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	افتراضي:
	  *دلیلاں = (فراغ*) raw++;
      }
#وإلا /* FFI_SIZEOF_JAVA_RAW != 8 */
	*دلیلاں = (فراغ*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->ناپ, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
#جے_انت /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#وإلا
#غلطی "pdp endian not supported"
#جے_انت /* ! PDP */

#جے_انت /* WORDS_BIGENDIAN */
}

فراغ
ffi_java_ptrarray_منزل_raw (ffi_cif *cif, فراغ **دلیلاں, ffi_java_raw *raw)
{
  بنانشانی i;
  ffi_type **tp = cif->arg_types;

  لكل (i = 0; i < cif->nargs; i++, tp++, دلیلاں++)
    {
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
#إذا الفاظ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*دلیلاں);
#وإلا
	  (raw++)->uint = *(UINT8*) (*دلیلاں);
#جے_انت
	  اكسر;

	حالة FFI_TYPE_SINT8:
#إذا الفاظ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*دلیلاں);
#وإلا
	  (raw++)->sint = *(SINT8*) (*دلیلاں);
#جے_انت
	  اكسر;

	حالة FFI_TYPE_UINT16:
#إذا الفاظ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*دلیلاں);
#وإلا
	  (raw++)->uint = *(UINT16*) (*دلیلاں);
#جے_انت
	  اكسر;

	حالة FFI_TYPE_SINT16:
#إذا الفاظ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*دلیلاں);
#وإلا
	  (raw++)->sint = *(SINT16*) (*دلیلاں);
#جے_انت
	  اكسر;

	حالة FFI_TYPE_UINT32:
#إذا الفاظ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*دلیلاں);
#وإلا
	  (raw++)->uint = *(UINT32*) (*دلیلاں);
#جے_انت
	  اكسر;

	حالة FFI_TYPE_SINT32:
#إذا الفاظ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*دلیلاں);
#وإلا
	  (raw++)->sint = *(SINT32*) (*دلیلاں);
#جے_انت
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*دلیلاں);
	  اكسر;

#إذا FFI_SIZEOF_JAVA_RAW == 8
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*دلیلاں);
	  raw += 2;
	  اكسر;
#جے_انت

	حالة FFI_TYPE_اشارہ_گر:
	  (raw++)->ptr = **(فراغ***) دلیلاں;
	  اكسر;

	افتراضي:
#إذا FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#وإلا
	  حافظہ_نقل ((فراغ*) raw->انکڑے, (فراغ*)*دلیلاں, (*tp)->ناپ);
	  raw +=
	    FFI_ALIGN ((*tp)->ناپ, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
#جے_انت
	}
    }
}

#إذا !FFI_NATIVE_RAW_API

ساكن فراغ
ffi_java_rvalue_منزل_raw (ffi_cif *cif, فراغ *rvalue)
{
#إذا الفاظ_BIGENDIAN && FFI_SIZEOF_ARG == 8
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_UINT8:
    حالة FFI_TYPE_UINT16:
    حالة FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      اكسر;

    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_INT:
#إذا FFI_SIZEOF_JAVA_RAW == 4
    حالة FFI_TYPE_اشارہ_گر:
#جے_انت
      *(SINT64 *)rvalue <<= 32;
      اكسر;

    حالة FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    افتراضي:
      اكسر;
    }
#جے_انت
}

ساكن فراغ
ffi_java_raw_منزل_rvalue (ffi_cif *cif, فراغ *rvalue)
{
#إذا الفاظ_BIGENDIAN && FFI_SIZEOF_ARG == 8
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_UINT8:
    حالة FFI_TYPE_UINT16:
    حالة FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      اكسر;

    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      اكسر;

    حالة FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    افتراضي:
      اكسر;
    }
#جے_انت
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

فراغ ffi_java_raw_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
			ffi_java_raw *raw)
{
  فراغ **avalue = (فراغ**) alloca (cif->nargs * حجم (فراغ*));
  ffi_java_raw_منزل_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_منزل_raw (cif, rvalue);
}

#إذا FFI_CLOSURES		/* base system provides closures */

ساكن فراغ
ffi_java_translate_دلیلاں (ffi_cif *cif, فراغ *rvalue,
		    فراغ **avalue, فراغ *user_انکڑے)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_ناپ (cif));
  ffi_raw_گھیرا *cl = (ffi_raw_گھیرا*)user_انکڑے;

  ffi_java_ptrarray_منزل_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_انکڑے);
  ffi_java_raw_منزل_rvalue (cif, rvalue);
}

ffi_حالت
ffi_prep_java_raw_گھیرا_loc (ffi_java_raw_گھیرا* cl,
			       ffi_cif *cif,
			       فراغ (*fun)(ffi_cif*,فراغ*,ffi_java_raw*,فراغ*),
			       فراغ *user_انکڑے,
			       فراغ *codeloc)
{
  ffi_حالت status;

  status = ffi_prep_گھیرا_loc ((ffi_گھیرا*) cl,
				 cif,
				 &ffi_java_translate_دلیلاں,
				 codeloc,
				 codeloc);
  إذا (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_انکڑے = user_انکڑے;
    }

  أرجع status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_حالت
ffi_prep_java_raw_گھیرا (ffi_java_raw_گھیرا* cl,
			   ffi_cif *cif,
			   فراغ (*fun)(ffi_cif*,فراغ*,ffi_java_raw*,فراغ*),
			   فراغ *user_انکڑے)
{
  أرجع ffi_prep_java_raw_گھیرا_loc (cl, cif, fun, user_انکڑے, cl);
}

#جے_انت /* FFI_CLOSURES */
#جے_انت /* !FFI_NATIVE_RAW_API */
#جے_انت /* !NO_JAVA_RAW_API */
