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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಆದರೆ !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_ಗಾತ್ರ (ffi_cif *cif)
{
  size_t ಫಲಿತಾಂಶ = 0;
  ಪೂರ್ಣಾಂಕ i;

  ffi_type **at = cif->arg_types;

  ಗಾಗಿ (i = cif->nargs-1; i >= 0; i--, at++)
    {
      ಆಯ್ಕೆ((*at) -> type) {
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಫಲಿತಾಂಶ += 2 * FFI_SIZEOF_JAVA_RAW;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	ಸ್ಥಿತಿ FFI_TYPE_ಸದಿಶ:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	ಪೂರ್ವನಿಯೋಜಿತ:
	  ಫಲಿತಾಂಶ += FFI_SIZEOF_JAVA_RAW;
      }
    }

  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}


ಶೂನ್ಯ
ffi_java_raw_ಗಮ್ಯ_ptrarray (ffi_cif *cif, ffi_java_raw *raw, ಶೂನ್ಯ **ವಾದಗಳು)
{
  ಚಿಹ್ನೆರಹಿತ i;
  ffi_type **tp = cif->arg_types;

#ಆದರೆ ಪದಗಳು_BIGENDIAN

  ಗಾಗಿ (i = 0; i < cif->nargs; i++, tp++, ವಾದಗಳು++)
    {
      ಆಯ್ಕೆ ((*tp)->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  *ವಾದಗಳು = (ಶೂನ್ಯ*) ((ಅಕ್ಷರ*)(raw++) + 3);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  *ವಾದಗಳು = (ಶೂನ್ಯ*) ((ಅಕ್ಷರ*)(raw++) + 2);
	  ನಿಲ್ಲಿಸು;

#ಆದರೆ FFI_SIZEOF_JAVA_RAW == 8
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  *ವಾದಗಳು = (ಶೂನ್ಯ *)raw;
	  raw += 2;
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  *ವಾದಗಳು = (ಶೂನ್ಯ*) &(raw++)->ptr;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	ಪೂರ್ವನಿಯೋಜಿತ:
	  *ವಾದಗಳು = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->ಗಾತ್ರ_2, ಗಾತ್ರ(ffi_java_raw)) / ಗಾತ್ರ(ffi_java_raw);
	}
    }

#ಇಲ್ಲದಿದ್ದರೆ /* WORDS_BIGENDIAN */

#ಆದರೆ !PDP

  /* then assume little endian */
  ಗಾಗಿ (i = 0; i < cif->nargs; i++, tp++, ವಾದಗಳು++)
    {
#ಆದರೆ FFI_SIZEOF_JAVA_RAW == 8
      ಆಯ್ಕೆ((*tp)->type) {
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  *ವಾದಗಳು = (ಶೂನ್ಯ*) raw;
	  raw += 2;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	ಪೂರ್ವನಿಯೋಜಿತ:
	  *ವಾದಗಳು = (ಶೂನ್ಯ*) raw++;
      }
#ಇಲ್ಲದಿದ್ದರೆ /* FFI_SIZEOF_JAVA_RAW != 8 */
	*ವಾದಗಳು = (ಶೂನ್ಯ*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->ಗಾತ್ರ_2, ಗಾತ್ರ(ffi_java_raw)) / ಗಾತ್ರ(ffi_java_raw);
#ಆದರೆ_ಅಂತ್ಯ /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ "pdp endian not supported"
#ಆದರೆ_ಅಂತ್ಯ /* ! PDP */

#ಆದರೆ_ಅಂತ್ಯ /* WORDS_BIGENDIAN */
}

ಶೂನ್ಯ
ffi_java_ptrarray_ಗಮ್ಯ_raw (ffi_cif *cif, ಶೂನ್ಯ **ವಾದಗಳು, ffi_java_raw *raw)
{
  ಚಿಹ್ನೆರಹಿತ i;
  ffi_type **tp = cif->arg_types;

  ಗಾಗಿ (i = 0; i < cif->nargs; i++, tp++, ವಾದಗಳು++)
    {
      ಆಯ್ಕೆ ((*tp)->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
#ಆದರೆ ಪದಗಳು_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*ವಾದಗಳು);
#ಇಲ್ಲದಿದ್ದರೆ
	  (raw++)->uint = *(UINT8*) (*ವಾದಗಳು);
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
#ಆದರೆ ಪದಗಳು_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*ವಾದಗಳು);
#ಇಲ್ಲದಿದ್ದರೆ
	  (raw++)->sint = *(SINT8*) (*ವಾದಗಳು);
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
#ಆದರೆ ಪದಗಳು_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*ವಾದಗಳು);
#ಇಲ್ಲದಿದ್ದರೆ
	  (raw++)->uint = *(UINT16*) (*ವಾದಗಳು);
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
#ಆದರೆ ಪದಗಳು_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*ವಾದಗಳು);
#ಇಲ್ಲದಿದ್ದರೆ
	  (raw++)->sint = *(SINT16*) (*ವಾದಗಳು);
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
#ಆದರೆ ಪದಗಳು_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*ವಾದಗಳು);
#ಇಲ್ಲದಿದ್ದರೆ
	  (raw++)->uint = *(UINT32*) (*ವಾದಗಳು);
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
#ಆದರೆ ಪದಗಳು_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*ವಾದಗಳು);
#ಇಲ್ಲದಿದ್ದರೆ
	  (raw++)->sint = *(SINT32*) (*ವಾದಗಳು);
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*ವಾದಗಳು);
	  ನಿಲ್ಲಿಸು;

#ಆದರೆ FFI_SIZEOF_JAVA_RAW == 8
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*ವಾದಗಳು);
	  raw += 2;
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  (raw++)->ptr = **(ಶೂನ್ಯ***) ವಾದಗಳು;
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
#ಆದರೆ FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#ಇಲ್ಲದಿದ್ದರೆ
	  ಸ್ಮರಣೆ_ನಕಲು ((ಶೂನ್ಯ*) raw->ದತ್ತಾಂಶ, (ಶೂನ್ಯ*)*ವಾದಗಳು, (*tp)->ಗಾತ್ರ_2);
	  raw +=
	    FFI_ALIGN ((*tp)->ಗಾತ್ರ_2, ಗಾತ್ರ(ffi_java_raw)) / ಗಾತ್ರ(ffi_java_raw);
#ಆದರೆ_ಅಂತ್ಯ
	}
    }
}

#ಆದರೆ !FFI_NATIVE_RAW_API

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_java_rvalue_ಗಮ್ಯ_raw (ffi_cif *cif, ಶೂನ್ಯ *rvalue)
{
#ಆದರೆ ಪದಗಳು_BIGENDIAN && FFI_SIZEOF_ARG == 8
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_INT:
#ಆದರೆ FFI_SIZEOF_JAVA_RAW == 4
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
#ಆದರೆ_ಅಂತ್ಯ
      *(SINT64 *)rvalue <<= 32;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    ಪೂರ್ವನಿಯೋಜಿತ:
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_java_raw_ಗಮ್ಯ_rvalue (ffi_cif *cif, ಶೂನ್ಯ *rvalue)
{
#ಆದರೆ ಪದಗಳು_BIGENDIAN && FFI_SIZEOF_ARG == 8
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    ಪೂರ್ವನಿಯೋಜಿತ:
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

ಶೂನ್ಯ ffi_java_raw_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
			ffi_java_raw *raw)
{
  ಶೂನ್ಯ **avalue = (ಶೂನ್ಯ**) alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ*));
  ffi_java_raw_ಗಮ್ಯ_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_ಗಮ್ಯ_raw (cif, rvalue);
}

#ಆದರೆ FFI_CLOSURES		/* base system provides closures */

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_java_translate_ವಾದಗಳು (ffi_cif *cif, ಶೂನ್ಯ *rvalue,
		    ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_ಗಾತ್ರ (cif));
  ffi_raw_ಆವರಣ *cl = (ffi_raw_ಆವರಣ*)user_ದತ್ತಾಂಶ;

  ffi_java_ptrarray_ಗಮ್ಯ_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_ದತ್ತಾಂಶ);
  ffi_java_raw_ಗಮ್ಯ_rvalue (cif, rvalue);
}

ffi_ಸ್ಥಿತಿ
ffi_prep_java_raw_ಆವರಣ_loc (ffi_java_raw_ಆವರಣ* cl,
			       ffi_cif *cif,
			       ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ffi_java_raw*,ಶೂನ್ಯ*),
			       ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			       ಶೂನ್ಯ *codeloc)
{
  ffi_ಸ್ಥಿತಿ status;

  status = ffi_prep_ಆವರಣ_loc ((ffi_ಆವರಣ*) cl,
				 cif,
				 &ffi_java_translate_ವಾದಗಳು,
				 codeloc,
				 codeloc);
  ಆದರೆ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
    }

  ಹಿಂತಿರುಗಿ status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_ಸ್ಥಿತಿ
ffi_prep_java_raw_ಆವರಣ (ffi_java_raw_ಆವರಣ* cl,
			   ffi_cif *cif,
			   ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ffi_java_raw*,ಶೂನ್ಯ*),
			   ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
  ಹಿಂತಿರುಗಿ ffi_prep_java_raw_ಆವರಣ_loc (cl, cif, fun, user_ದತ್ತಾಂಶ, cl);
}

#ಆದರೆ_ಅಂತ್ಯ /* FFI_CLOSURES */
#ಆದರೆ_ಅಂತ್ಯ /* !FFI_NATIVE_RAW_API */
#ಆದರೆ_ಅಂತ್ಯ /* !NO_JAVA_RAW_API */
