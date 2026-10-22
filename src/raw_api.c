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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಆದರೆ !FFI_NO_RAW_API

size_t
ffi_raw_ಗಾತ್ರ (ffi_cif *cif)
{
  size_t ಫಲಿತಾಂಶ = 0;
  ಪೂರ್ಣಾಂಕ i;

  ffi_type **at = cif->arg_types;

  ಗಾಗಿ (i = cif->nargs-1; i >= 0; i--, at++)
    {
#ಆದರೆ !FFI_NO_STRUCTS
      ಆದರೆ ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_ಸದಿಶ)
	ಫಲಿತಾಂಶ += FFI_ALIGN (ಗಾತ್ರ (ಶೂನ್ಯ*), FFI_SIZEOF_ARG);
      ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	ಫಲಿತಾಂಶ += FFI_ALIGN ((*at)->ಗಾತ್ರ_2, FFI_SIZEOF_ARG);
    }

  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}


ಶೂನ್ಯ
ffi_raw_ಗಮ್ಯ_ptrarray (ffi_cif *cif, ffi_raw *raw, ಶೂನ್ಯ **ವಾದಗಳು)
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
	  *ವಾದಗಳು = (ಶೂನ್ಯ*) ((ಅಕ್ಷರ*)(raw++) + FFI_SIZEOF_ARG - 1);
	  ನಿಲ್ಲಿಸು;
	  
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  *ವಾದಗಳು = (ಶೂನ್ಯ*) ((ಅಕ್ಷರ*)(raw++) + FFI_SIZEOF_ARG - 2);
	  ನಿಲ್ಲಿಸು;

#ಆದರೆ FFI_SIZEOF_ARG >= 4	  
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	  *ವಾದಗಳು = (ಶೂನ್ಯ*) ((ಅಕ್ಷರ*)(raw++) + FFI_SIZEOF_ARG - 4);
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
	
#ಆದರೆ !FFI_NO_STRUCTS
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	ಸ್ಥಿತಿ FFI_TYPE_ಸದಿಶ:
	  *ವಾದಗಳು = (raw++)->ptr;
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  *ವಾದಗಳು = (raw++)->ptr;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  *ವಾದಗಳು = (ಶೂನ್ಯ*) &(raw++)->ptr;
	  ನಿಲ್ಲಿಸು;
	  
	ಪೂರ್ವನಿಯೋಜಿತ:
	  *ವಾದಗಳು = raw;
	  raw += FFI_ALIGN ((*tp)->ಗಾತ್ರ_2, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#ಇಲ್ಲದಿದ್ದರೆ /* WORDS_BIGENDIAN */

#ಆದರೆ !PDP

  /* then assume little endian */
  ಗಾಗಿ (i = 0; i < cif->nargs; i++, tp++, ವಾದಗಳು++)
    {	  
#ಆದರೆ !FFI_NO_STRUCTS
      ಆದರೆ ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_ಸದಿಶ)
	{
	  *ವಾದಗಳು = (raw++)->ptr;
	}
      ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *ವಾದಗಳು = (raw++)->ptr;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  *ವಾದಗಳು = (ಶೂನ್ಯ*) raw;
	  raw += FFI_ALIGN ((*tp)->ಗಾತ್ರ_2, ಗಾತ್ರ (ಶೂನ್ಯ*)) / ಗಾತ್ರ (ಶೂನ್ಯ*);
	}
    }

#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ "pdp endian not supported"
#ಆದರೆ_ಅಂತ್ಯ /* ! PDP */

#ಆದರೆ_ಅಂತ್ಯ /* WORDS_BIGENDIAN */
}

ಶೂನ್ಯ
ffi_ptrarray_ಗಮ್ಯ_raw (ffi_cif *cif, ಶೂನ್ಯ **ವಾದಗಳು, ffi_raw *raw)
{
  ಚಿಹ್ನೆರಹಿತ i;
  ffi_type **tp = cif->arg_types;

  ಗಾಗಿ (i = 0; i < cif->nargs; i++, tp++, ವಾದಗಳು++)
    {	  
      ಆಯ್ಕೆ ((*tp)->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*ವಾದಗಳು);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*ವಾದಗಳು);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*ವಾದಗಳು);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*ವಾದಗಳು);
	  ನಿಲ್ಲಿಸು;

#ಆದರೆ FFI_SIZEOF_ARG >= 4
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*ವಾದಗಳು);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*ವಾದಗಳು);
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !FFI_NO_STRUCTS
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	ಸ್ಥಿತಿ FFI_TYPE_ಸದಿಶ:
	  (raw++)->ptr = *ವಾದಗಳು;
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *ವಾದಗಳು;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  (raw++)->ptr = **(ಶೂನ್ಯ***) ವಾದಗಳು;
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  ಸ್ಮರಣೆ_ನಕಲು ((ಶೂನ್ಯ*) raw->ದತ್ತಾಂಶ, (ಶೂನ್ಯ*)*ವಾದಗಳು, (*tp)->ಗಾತ್ರ_2);
	  raw += FFI_ALIGN ((*tp)->ಗಾತ್ರ_2, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#ಆದರೆ !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

ಶೂನ್ಯ ffi_raw_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ffi_raw *raw)
{
  ಶೂನ್ಯ **avalue = (ಶೂನ್ಯ**) alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ*));
  ffi_raw_ಗಮ್ಯ_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#ಆದರೆ FFI_CLOSURES		/* base system provides closures */

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_translate_ವಾದಗಳು (ffi_cif *cif, ಶೂನ್ಯ *rvalue,
		    ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_ಗಾತ್ರ (cif));
  ffi_raw_ಆವರಣ *cl = (ffi_raw_ಆವರಣ*)user_ದತ್ತಾಂಶ;

  ffi_ptrarray_ಗಮ್ಯ_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_ದತ್ತಾಂಶ);
}

ffi_ಸ್ಥಿತಿ
ffi_prep_raw_ಆವರಣ_loc (ffi_raw_ಆವರಣ* cl,
			  ffi_cif *cif,
			  ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ffi_raw*,ಶೂನ್ಯ*),
			  ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			  ಶೂನ್ಯ *codeloc)
{
  ffi_ಸ್ಥಿತಿ status;

  status = ffi_prep_ಆವರಣ_loc ((ffi_ಆವರಣ*) cl,
				 cif,
				 &ffi_translate_ವಾದಗಳು,
				 codeloc,
				 codeloc);
  ಆದರೆ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
    }

  ಹಿಂತಿರುಗಿ status;
}

#ಆದರೆ_ಅಂತ್ಯ /* FFI_CLOSURES */
#ಆದರೆ_ಅಂತ್ಯ /* !FFI_NATIVE_RAW_API */

#ಆದರೆ FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_ಸ್ಥಿತಿ
ffi_prep_raw_ಆವರಣ (ffi_raw_ಆವರಣ* cl,
		      ffi_cif *cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ffi_raw*,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
  ಹಿಂತಿರುಗಿ ffi_prep_raw_ಆವರಣ_loc (cl, cif, fun, user_ದತ್ತಾಂಶ, cl);
}

#ಆದರೆ_ಅಂತ್ಯ /* FFI_CLOSURES */

#ಆದರೆ_ಅಂತ್ಯ /* !FFI_NO_RAW_API */
