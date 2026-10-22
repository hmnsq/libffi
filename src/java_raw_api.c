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
  size_t پایله = 0;
  صحيح i;

  ffi_type **at = cif->arg_types;

  لكل (i = cif->nargs-1; i >= 0; i--, at++)
    {
      بدل((*at) -> type) {
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  پایله += 2 * FFI_SIZEOF_JAVA_RAW;
	  اكسر;
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_ویکټور:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	افتراضي:
	  پایله += FFI_SIZEOF_JAVA_RAW;
      }
    }

  أرجع پایله;
}


فراغ
ffi_java_raw_منزل_ptrarray (ffi_cif *cif, ffi_java_raw *raw, فراغ **دلیلونه)
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
	  *دلیلونه = (فراغ*) ((حرف*)(raw++) + 3);
	  اكسر;

	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	  *دلیلونه = (فراغ*) ((حرف*)(raw++) + 2);
	  اكسر;

#إذا FFI_SIZEOF_JAVA_RAW == 8
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  *دلیلونه = (فراغ *)raw;
	  raw += 2;
	  اكسر;
#د_که_پای

	حالة FFI_TYPE_اشاره_ګر:
	  *دلیلونه = (فراغ*) &(raw++)->ptr;
	  اكسر;

	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	افتراضي:
	  *دلیلونه = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->اندازه, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
	}
    }

#وإلا /* WORDS_BIGENDIAN */

#إذا !PDP

  /* then assume little endian */
  لكل (i = 0; i < cif->nargs; i++, tp++, دلیلونه++)
    {
#إذا FFI_SIZEOF_JAVA_RAW == 8
      بدل((*tp)->type) {
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  *دلیلونه = (فراغ*) raw;
	  raw += 2;
	  اكسر;
	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	افتراضي:
	  *دلیلونه = (فراغ*) raw++;
      }
#وإلا /* FFI_SIZEOF_JAVA_RAW != 8 */
	*دلیلونه = (فراغ*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->اندازه, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
#د_که_پای /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#وإلا
#تېروتنه "pdp endian not supported"
#د_که_پای /* ! PDP */

#د_که_پای /* WORDS_BIGENDIAN */
}

فراغ
ffi_java_ptrarray_منزل_raw (ffi_cif *cif, فراغ **دلیلونه, ffi_java_raw *raw)
{
  بینښه i;
  ffi_type **tp = cif->arg_types;

  لكل (i = 0; i < cif->nargs; i++, tp++, دلیلونه++)
    {
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
#إذا ټکي_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*دلیلونه);
#وإلا
	  (raw++)->uint = *(UINT8*) (*دلیلونه);
#د_که_پای
	  اكسر;

	حالة FFI_TYPE_SINT8:
#إذا ټکي_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*دلیلونه);
#وإلا
	  (raw++)->sint = *(SINT8*) (*دلیلونه);
#د_که_پای
	  اكسر;

	حالة FFI_TYPE_UINT16:
#إذا ټکي_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*دلیلونه);
#وإلا
	  (raw++)->uint = *(UINT16*) (*دلیلونه);
#د_که_پای
	  اكسر;

	حالة FFI_TYPE_SINT16:
#إذا ټکي_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*دلیلونه);
#وإلا
	  (raw++)->sint = *(SINT16*) (*دلیلونه);
#د_که_پای
	  اكسر;

	حالة FFI_TYPE_UINT32:
#إذا ټکي_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*دلیلونه);
#وإلا
	  (raw++)->uint = *(UINT32*) (*دلیلونه);
#د_که_پای
	  اكسر;

	حالة FFI_TYPE_SINT32:
#إذا ټکي_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*دلیلونه);
#وإلا
	  (raw++)->sint = *(SINT32*) (*دلیلونه);
#د_که_پای
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*دلیلونه);
	  اكسر;

#إذا FFI_SIZEOF_JAVA_RAW == 8
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*دلیلونه);
	  raw += 2;
	  اكسر;
#د_که_پای

	حالة FFI_TYPE_اشاره_ګر:
	  (raw++)->ptr = **(فراغ***) دلیلونه;
	  اكسر;

	افتراضي:
#إذا FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#وإلا
	  حافظه_نقل ((فراغ*) raw->معلومات, (فراغ*)*دلیلونه, (*tp)->اندازه);
	  raw +=
	    FFI_ALIGN ((*tp)->اندازه, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
#د_که_پای
	}
    }
}

#إذا !FFI_NATIVE_RAW_API

ساكن فراغ
ffi_java_rvalue_منزل_raw (ffi_cif *cif, فراغ *rvalue)
{
#إذا ټکي_BIGENDIAN && FFI_SIZEOF_ARG == 8
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
    حالة FFI_TYPE_اشاره_ګر:
#د_که_پای
      *(SINT64 *)rvalue <<= 32;
      اكسر;

    حالة FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    افتراضي:
      اكسر;
    }
#د_که_پای
}

ساكن فراغ
ffi_java_raw_منزل_rvalue (ffi_cif *cif, فراغ *rvalue)
{
#إذا ټکي_BIGENDIAN && FFI_SIZEOF_ARG == 8
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
#د_که_پای
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
ffi_java_translate_دلیلونه (ffi_cif *cif, فراغ *rvalue,
		    فراغ **avalue, فراغ *user_معلومات)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_اندازه (cif));
  ffi_raw_تړون *cl = (ffi_raw_تړون*)user_معلومات;

  ffi_java_ptrarray_منزل_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_معلومات);
  ffi_java_raw_منزل_rvalue (cif, rvalue);
}

ffi_حالت
ffi_prep_java_raw_تړون_loc (ffi_java_raw_تړون* cl,
			       ffi_cif *cif,
			       فراغ (*fun)(ffi_cif*,فراغ*,ffi_java_raw*,فراغ*),
			       فراغ *user_معلومات,
			       فراغ *codeloc)
{
  ffi_حالت status;

  status = ffi_prep_تړون_loc ((ffi_تړون*) cl,
				 cif,
				 &ffi_java_translate_دلیلونه,
				 codeloc,
				 codeloc);
  إذا (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_معلومات = user_معلومات;
    }

  أرجع status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_حالت
ffi_prep_java_raw_تړون (ffi_java_raw_تړون* cl,
			   ffi_cif *cif,
			   فراغ (*fun)(ffi_cif*,فراغ*,ffi_java_raw*,فراغ*),
			   فراغ *user_معلومات)
{
  أرجع ffi_prep_java_raw_تړون_loc (cl, cif, fun, user_معلومات, cl);
}

#د_که_پای /* FFI_CLOSURES */
#د_که_پای /* !FFI_NATIVE_RAW_API */
#د_که_پای /* !NO_JAVA_RAW_API */
