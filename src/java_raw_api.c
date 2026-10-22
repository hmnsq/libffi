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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdlib.h>

#यदि !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_आकार (ffi_cif *cif)
{
  size_t परिणाम = 0;
  पूर्णाङ्क i;

  ffi_type **at = cif->arg_types;

  प्रति (i = cif->nargs-1; i >= 0; i--, at++)
    {
      विकल्प((*at) -> type) {
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_DOUBLE:
	  परिणाम += 2 * FFI_SIZEOF_JAVA_RAW;
	  भङ्ग;
	पक्ष FFI_TYPE_STRUCT:
	पक्ष FFI_TYPE_सदिशम्:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	पक्ष FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	स्वभाव:
	  परिणाम += FFI_SIZEOF_JAVA_RAW;
      }
    }

  प्रत्यागम परिणाम;
}


शून्य
ffi_java_raw_गन्तव्यम्_ptrarray (ffi_cif *cif, ffi_java_raw *raw, शून्य **तर्काः)
{
  अचिह्नित i;
  ffi_type **tp = cif->arg_types;

#यदि शब्दाः_BIGENDIAN

  प्रति (i = 0; i < cif->nargs; i++, tp++, तर्काः++)
    {
      विकल्प ((*tp)->type)
	{
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT8:
	  *तर्काः = (शून्य*) ((वर्ण*)(raw++) + 3);
	  भङ्ग;

	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT16:
	  *तर्काः = (शून्य*) ((वर्ण*)(raw++) + 2);
	  भङ्ग;

#यदि FFI_SIZEOF_JAVA_RAW == 8
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_DOUBLE:
	  *तर्काः = (शून्य *)raw;
	  raw += 2;
	  भङ्ग;
#यदि_अन्तः

	पक्ष FFI_TYPE_POINTER:
	  *तर्काः = (शून्य*) &(raw++)->ptr;
	  भङ्ग;

	पक्ष FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	स्वभाव:
	  *तर्काः = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->आकार, परिमाण(ffi_java_raw)) / परिमाण(ffi_java_raw);
	}
    }

#अन्यथा /* WORDS_BIGENDIAN */

#यदि !PDP

  /* then assume little endian */
  प्रति (i = 0; i < cif->nargs; i++, tp++, तर्काः++)
    {
#यदि FFI_SIZEOF_JAVA_RAW == 8
      विकल्प((*tp)->type) {
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_DOUBLE:
	  *तर्काः = (शून्य*) raw;
	  raw += 2;
	  भङ्ग;
	पक्ष FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	स्वभाव:
	  *तर्काः = (शून्य*) raw++;
      }
#अन्यथा /* FFI_SIZEOF_JAVA_RAW != 8 */
	*तर्काः = (शून्य*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->आकार, परिमाण(ffi_java_raw)) / परिमाण(ffi_java_raw);
#यदि_अन्तः /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#अन्यथा
#दोषः "pdp endian not supported"
#यदि_अन्तः /* ! PDP */

#यदि_अन्तः /* WORDS_BIGENDIAN */
}

शून्य
ffi_java_ptrarray_गन्तव्यम्_raw (ffi_cif *cif, शून्य **तर्काः, ffi_java_raw *raw)
{
  अचिह्नित i;
  ffi_type **tp = cif->arg_types;

  प्रति (i = 0; i < cif->nargs; i++, tp++, तर्काः++)
    {
      विकल्प ((*tp)->type)
	{
	पक्ष FFI_TYPE_UINT8:
#यदि शब्दाः_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*तर्काः);
#अन्यथा
	  (raw++)->uint = *(UINT8*) (*तर्काः);
#यदि_अन्तः
	  भङ्ग;

	पक्ष FFI_TYPE_SINT8:
#यदि शब्दाः_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*तर्काः);
#अन्यथा
	  (raw++)->sint = *(SINT8*) (*तर्काः);
#यदि_अन्तः
	  भङ्ग;

	पक्ष FFI_TYPE_UINT16:
#यदि शब्दाः_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*तर्काः);
#अन्यथा
	  (raw++)->uint = *(UINT16*) (*तर्काः);
#यदि_अन्तः
	  भङ्ग;

	पक्ष FFI_TYPE_SINT16:
#यदि शब्दाः_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*तर्काः);
#अन्यथा
	  (raw++)->sint = *(SINT16*) (*तर्काः);
#यदि_अन्तः
	  भङ्ग;

	पक्ष FFI_TYPE_UINT32:
#यदि शब्दाः_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*तर्काः);
#अन्यथा
	  (raw++)->uint = *(UINT32*) (*तर्काः);
#यदि_अन्तः
	  भङ्ग;

	पक्ष FFI_TYPE_SINT32:
#यदि शब्दाः_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*तर्काः);
#अन्यथा
	  (raw++)->sint = *(SINT32*) (*तर्काः);
#यदि_अन्तः
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*तर्काः);
	  भङ्ग;

#यदि FFI_SIZEOF_JAVA_RAW == 8
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*तर्काः);
	  raw += 2;
	  भङ्ग;
#यदि_अन्तः

	पक्ष FFI_TYPE_POINTER:
	  (raw++)->ptr = **(शून्य***) तर्काः;
	  भङ्ग;

	स्वभाव:
#यदि FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#अन्यथा
	  स्मृतिं_प्रतिलिख ((शून्य*) raw->दत्त, (शून्य*)*तर्काः, (*tp)->आकार);
	  raw +=
	    FFI_ALIGN ((*tp)->आकार, परिमाण(ffi_java_raw)) / परिमाण(ffi_java_raw);
#यदि_अन्तः
	}
    }
}

#यदि !FFI_NATIVE_RAW_API

स्थिर शून्य
ffi_java_rvalue_गन्तव्यम्_raw (ffi_cif *cif, शून्य *rvalue)
{
#यदि शब्दाः_BIGENDIAN && FFI_SIZEOF_ARG == 8
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_UINT8:
    पक्ष FFI_TYPE_UINT16:
    पक्ष FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      भङ्ग;

    पक्ष FFI_TYPE_SINT8:
    पक्ष FFI_TYPE_SINT16:
    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_INT:
#यदि FFI_SIZEOF_JAVA_RAW == 4
    पक्ष FFI_TYPE_POINTER:
#यदि_अन्तः
      *(SINT64 *)rvalue <<= 32;
      भङ्ग;

    पक्ष FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    स्वभाव:
      भङ्ग;
    }
#यदि_अन्तः
}

स्थिर शून्य
ffi_java_raw_गन्तव्यम्_rvalue (ffi_cif *cif, शून्य *rvalue)
{
#यदि शब्दाः_BIGENDIAN && FFI_SIZEOF_ARG == 8
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_UINT8:
    पक्ष FFI_TYPE_UINT16:
    पक्ष FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      भङ्ग;

    पक्ष FFI_TYPE_SINT8:
    पक्ष FFI_TYPE_SINT16:
    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      भङ्ग;

    पक्ष FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    स्वभाव:
      भङ्ग;
    }
#यदि_अन्तः
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

शून्य ffi_java_raw_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
			ffi_java_raw *raw)
{
  शून्य **avalue = (शून्य**) alloca (cif->nargs * परिमाण (शून्य*));
  ffi_java_raw_गन्तव्यम्_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_गन्तव्यम्_raw (cif, rvalue);
}

#यदि FFI_CLOSURES		/* base system provides closures */

स्थिर शून्य
ffi_java_translate_तर्काः (ffi_cif *cif, शून्य *rvalue,
		    शून्य **avalue, शून्य *user_दत्त)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_आकार (cif));
  ffi_raw_संवृतिः *cl = (ffi_raw_संवृतिः*)user_दत्त;

  ffi_java_ptrarray_गन्तव्यम्_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_दत्त);
  ffi_java_raw_गन्तव्यम्_rvalue (cif, rvalue);
}

ffi_स्थितिः
ffi_prep_java_raw_संवृतिः_loc (ffi_java_raw_संवृतिः* cl,
			       ffi_cif *cif,
			       शून्य (*fun)(ffi_cif*,शून्य*,ffi_java_raw*,शून्य*),
			       शून्य *user_दत्त,
			       शून्य *codeloc)
{
  ffi_स्थितिः status;

  status = ffi_prep_संवृतिः_loc ((ffi_संवृतिः*) cl,
				 cif,
				 &ffi_java_translate_तर्काः,
				 codeloc,
				 codeloc);
  यदि (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_दत्त = user_दत्त;
    }

  प्रत्यागम status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_स्थितिः
ffi_prep_java_raw_संवृतिः (ffi_java_raw_संवृतिः* cl,
			   ffi_cif *cif,
			   शून्य (*fun)(ffi_cif*,शून्य*,ffi_java_raw*,शून्य*),
			   शून्य *user_दत्त)
{
  प्रत्यागम ffi_prep_java_raw_संवृतिः_loc (cl, cif, fun, user_दत्त, cl);
}

#यदि_अन्तः /* FFI_CLOSURES */
#यदि_अन्तः /* !FFI_NATIVE_RAW_API */
#यदि_अन्तः /* !NO_JAVA_RAW_API */
