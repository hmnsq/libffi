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
ffi_java_raw_اندازه (ffi_cif *cif)
{
  size_t نتیجه = 0;
  صحيح i;

  ffi_type **at = cif->arg_types;

  لكل (i = cif->nargs-1; i >= 0; i--, at++)
    {
      بدل((*at) -> type) {
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  نتیجه += 2 * FFI_SIZEOF_JAVA_RAW;
	  اكسر;
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_وکتور:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	افتراضي:
	  نتیجه += FFI_SIZEOF_JAVA_RAW;
      }
    }

  أرجع نتیجه;
}


فراغ
ffi_java_raw_مقصد_ptrarray (ffi_cif *cif, ffi_java_raw *raw, فراغ **آرگومانها)
{
  بیعلامت i;
  ffi_type **tp = cif->arg_types;

#إذا واژگان_BIGENDIAN

  لكل (i = 0; i < cif->nargs; i++, tp++, آرگومانها++)
    {
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	  *آرگومانها = (فراغ*) ((حرف*)(raw++) + 3);
	  اكسر;

	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	  *آرگومانها = (فراغ*) ((حرف*)(raw++) + 2);
	  اكسر;

#إذا FFI_SIZEOF_JAVA_RAW == 8
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  *آرگومانها = (فراغ *)raw;
	  raw += 2;
	  اكسر;
#پایان_اگر

	حالة FFI_TYPE_اشاره_گر:
	  *آرگومانها = (فراغ*) &(raw++)->ptr;
	  اكسر;

	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	افتراضي:
	  *آرگومانها = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->اندازه, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
	}
    }

#وإلا /* WORDS_BIGENDIAN */

#إذا !PDP

  /* then assume little endian */
  لكل (i = 0; i < cif->nargs; i++, tp++, آرگومانها++)
    {
#إذا FFI_SIZEOF_JAVA_RAW == 8
      بدل((*tp)->type) {
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  *آرگومانها = (فراغ*) raw;
	  raw += 2;
	  اكسر;
	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	افتراضي:
	  *آرگومانها = (فراغ*) raw++;
      }
#وإلا /* FFI_SIZEOF_JAVA_RAW != 8 */
	*آرگومانها = (فراغ*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->اندازه, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
#پایان_اگر /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#وإلا
#خطا "pdp endian not supported"
#پایان_اگر /* ! PDP */

#پایان_اگر /* WORDS_BIGENDIAN */
}

فراغ
ffi_java_ptrarray_مقصد_raw (ffi_cif *cif, فراغ **آرگومانها, ffi_java_raw *raw)
{
  بیعلامت i;
  ffi_type **tp = cif->arg_types;

  لكل (i = 0; i < cif->nargs; i++, tp++, آرگومانها++)
    {
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
#إذا واژگان_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*آرگومانها);
#وإلا
	  (raw++)->uint = *(UINT8*) (*آرگومانها);
#پایان_اگر
	  اكسر;

	حالة FFI_TYPE_SINT8:
#إذا واژگان_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*آرگومانها);
#وإلا
	  (raw++)->sint = *(SINT8*) (*آرگومانها);
#پایان_اگر
	  اكسر;

	حالة FFI_TYPE_UINT16:
#إذا واژگان_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*آرگومانها);
#وإلا
	  (raw++)->uint = *(UINT16*) (*آرگومانها);
#پایان_اگر
	  اكسر;

	حالة FFI_TYPE_SINT16:
#إذا واژگان_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*آرگومانها);
#وإلا
	  (raw++)->sint = *(SINT16*) (*آرگومانها);
#پایان_اگر
	  اكسر;

	حالة FFI_TYPE_UINT32:
#إذا واژگان_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*آرگومانها);
#وإلا
	  (raw++)->uint = *(UINT32*) (*آرگومانها);
#پایان_اگر
	  اكسر;

	حالة FFI_TYPE_SINT32:
#إذا واژگان_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*آرگومانها);
#وإلا
	  (raw++)->sint = *(SINT32*) (*آرگومانها);
#پایان_اگر
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*آرگومانها);
	  اكسر;

#إذا FFI_SIZEOF_JAVA_RAW == 8
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*آرگومانها);
	  raw += 2;
	  اكسر;
#پایان_اگر

	حالة FFI_TYPE_اشاره_گر:
	  (raw++)->ptr = **(فراغ***) آرگومانها;
	  اكسر;

	افتراضي:
#إذا FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#وإلا
	  کپی_حافظه ((فراغ*) raw->داده, (فراغ*)*آرگومانها, (*tp)->اندازه);
	  raw +=
	    FFI_ALIGN ((*tp)->اندازه, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
#پایان_اگر
	}
    }
}

#إذا !FFI_NATIVE_RAW_API

ساكن فراغ
ffi_java_rvalue_مقصد_raw (ffi_cif *cif, فراغ *rvalue)
{
#إذا واژگان_BIGENDIAN && FFI_SIZEOF_ARG == 8
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
    حالة FFI_TYPE_اشاره_گر:
#پایان_اگر
      *(SINT64 *)rvalue <<= 32;
      اكسر;

    حالة FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    افتراضي:
      اكسر;
    }
#پایان_اگر
}

ساكن فراغ
ffi_java_raw_مقصد_rvalue (ffi_cif *cif, فراغ *rvalue)
{
#إذا واژگان_BIGENDIAN && FFI_SIZEOF_ARG == 8
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
#پایان_اگر
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
  ffi_java_raw_مقصد_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_مقصد_raw (cif, rvalue);
}

#إذا FFI_CLOSURES		/* base system provides closures */

ساكن فراغ
ffi_java_translate_آرگومانها (ffi_cif *cif, فراغ *rvalue,
		    فراغ **avalue, فراغ *user_داده)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_اندازه (cif));
  ffi_raw_بستگی *cl = (ffi_raw_بستگی*)user_داده;

  ffi_java_ptrarray_مقصد_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_داده);
  ffi_java_raw_مقصد_rvalue (cif, rvalue);
}

ffi_وضعیت
ffi_prep_java_raw_بستگی_loc (ffi_java_raw_بستگی* cl,
			       ffi_cif *cif,
			       فراغ (*fun)(ffi_cif*,فراغ*,ffi_java_raw*,فراغ*),
			       فراغ *user_داده,
			       فراغ *codeloc)
{
  ffi_وضعیت status;

  status = ffi_prep_بستگی_loc ((ffi_بستگی*) cl,
				 cif,
				 &ffi_java_translate_آرگومانها,
				 codeloc,
				 codeloc);
  إذا (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_داده = user_داده;
    }

  أرجع status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_وضعیت
ffi_prep_java_raw_بستگی (ffi_java_raw_بستگی* cl,
			   ffi_cif *cif,
			   فراغ (*fun)(ffi_cif*,فراغ*,ffi_java_raw*,فراغ*),
			   فراغ *user_داده)
{
  أرجع ffi_prep_java_raw_بستگی_loc (cl, cif, fun, user_داده, cl);
}

#پایان_اگر /* FFI_CLOSURES */
#پایان_اگر /* !FFI_NATIVE_RAW_API */
#پایان_اگر /* !NO_JAVA_RAW_API */
