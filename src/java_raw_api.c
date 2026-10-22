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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਜੇ !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_ਮਾਪ (ffi_cif *cif)
{
  size_t ਨਤੀਜਾ = 0;
  ਪੂਰਨ i;

  ffi_type **at = cif->arg_types;

  ਲਈ (i = cif->nargs-1; i >= 0; i--, at++)
    {
      ਚੋਣ((*at) -> type) {
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਨਤੀਜਾ += 2 * FFI_SIZEOF_JAVA_RAW;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_STRUCT:
	ਹਾਲਤ FFI_TYPE_ਸਦਿਸ਼:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	ਮੂਲ:
	  ਨਤੀਜਾ += FFI_SIZEOF_JAVA_RAW;
      }
    }

  ਵਾਪਸ ਨਤੀਜਾ;
}


ਖਾਲੀ
ffi_java_raw_ਮੰਜ਼ਿਲ_ptrarray (ffi_cif *cif, ffi_java_raw *raw, ਖਾਲੀ **ਦਲੀਲਾਂ)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ i;
  ffi_type **tp = cif->arg_types;

#ਜੇ ਸ਼ਬਦ_BIGENDIAN

  ਲਈ (i = 0; i < cif->nargs; i++, tp++, ਦਲੀਲਾਂ++)
    {
      ਚੋਣ ((*tp)->type)
	{
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT8:
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ*) ((ਅੱਖਰ*)(raw++) + 3);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT16:
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ*) ((ਅੱਖਰ*)(raw++) + 2);
	  ਰੋਕੋ;

#ਜੇ FFI_SIZEOF_JAVA_RAW == 8
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ *)raw;
	  raw += 2;
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ*) &(raw++)->ptr;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	ਮੂਲ:
	  *ਦਲੀਲਾਂ = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->ਮਾਪ, ਆਕਾਰ(ffi_java_raw)) / ਆਕਾਰ(ffi_java_raw);
	}
    }

#ਨਹੀਂਤਾਂ /* WORDS_BIGENDIAN */

#ਜੇ !PDP

  /* then assume little endian */
  ਲਈ (i = 0; i < cif->nargs; i++, tp++, ਦਲੀਲਾਂ++)
    {
#ਜੇ FFI_SIZEOF_JAVA_RAW == 8
      ਚੋਣ((*tp)->type) {
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ*) raw;
	  raw += 2;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	ਮੂਲ:
	  *ਦਲੀਲਾਂ = (ਖਾਲੀ*) raw++;
      }
#ਨਹੀਂਤਾਂ /* FFI_SIZEOF_JAVA_RAW != 8 */
	*ਦਲੀਲਾਂ = (ਖਾਲੀ*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->ਮਾਪ, ਆਕਾਰ(ffi_java_raw)) / ਆਕਾਰ(ffi_java_raw);
#ਜੇ_ਅੰਤ /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#ਨਹੀਂਤਾਂ
#ਗਲਤੀ "pdp endian not supported"
#ਜੇ_ਅੰਤ /* ! PDP */

#ਜੇ_ਅੰਤ /* WORDS_BIGENDIAN */
}

ਖਾਲੀ
ffi_java_ptrarray_ਮੰਜ਼ਿਲ_raw (ffi_cif *cif, ਖਾਲੀ **ਦਲੀਲਾਂ, ffi_java_raw *raw)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ i;
  ffi_type **tp = cif->arg_types;

  ਲਈ (i = 0; i < cif->nargs; i++, tp++, ਦਲੀਲਾਂ++)
    {
      ਚੋਣ ((*tp)->type)
	{
	ਹਾਲਤ FFI_TYPE_UINT8:
#ਜੇ ਸ਼ਬਦ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*ਦਲੀਲਾਂ);
#ਨਹੀਂਤਾਂ
	  (raw++)->uint = *(UINT8*) (*ਦਲੀਲਾਂ);
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT8:
#ਜੇ ਸ਼ਬਦ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*ਦਲੀਲਾਂ);
#ਨਹੀਂਤਾਂ
	  (raw++)->sint = *(SINT8*) (*ਦਲੀਲਾਂ);
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT16:
#ਜੇ ਸ਼ਬਦ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*ਦਲੀਲਾਂ);
#ਨਹੀਂਤਾਂ
	  (raw++)->uint = *(UINT16*) (*ਦਲੀਲਾਂ);
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT16:
#ਜੇ ਸ਼ਬਦ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*ਦਲੀਲਾਂ);
#ਨਹੀਂਤਾਂ
	  (raw++)->sint = *(SINT16*) (*ਦਲੀਲਾਂ);
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT32:
#ਜੇ ਸ਼ਬਦ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*ਦਲੀਲਾਂ);
#ਨਹੀਂਤਾਂ
	  (raw++)->uint = *(UINT32*) (*ਦਲੀਲਾਂ);
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT32:
#ਜੇ ਸ਼ਬਦ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*ਦਲੀਲਾਂ);
#ਨਹੀਂਤਾਂ
	  (raw++)->sint = *(SINT32*) (*ਦਲੀਲਾਂ);
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*ਦਲੀਲਾਂ);
	  ਰੋਕੋ;

#ਜੇ FFI_SIZEOF_JAVA_RAW == 8
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*ਦਲੀਲਾਂ);
	  raw += 2;
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  (raw++)->ptr = **(ਖਾਲੀ***) ਦਲੀਲਾਂ;
	  ਰੋਕੋ;

	ਮੂਲ:
#ਜੇ FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#ਨਹੀਂਤਾਂ
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਖਾਲੀ*) raw->ਅੰਕੜੇ, (ਖਾਲੀ*)*ਦਲੀਲਾਂ, (*tp)->ਮਾਪ);
	  raw +=
	    FFI_ALIGN ((*tp)->ਮਾਪ, ਆਕਾਰ(ffi_java_raw)) / ਆਕਾਰ(ffi_java_raw);
#ਜੇ_ਅੰਤ
	}
    }
}

#ਜੇ !FFI_NATIVE_RAW_API

ਸਥਿਰ ਖਾਲੀ
ffi_java_rvalue_ਮੰਜ਼ਿਲ_raw (ffi_cif *cif, ਖਾਲੀ *rvalue)
{
#ਜੇ ਸ਼ਬਦ_BIGENDIAN && FFI_SIZEOF_ARG == 8
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_UINT8:
    ਹਾਲਤ FFI_TYPE_UINT16:
    ਹਾਲਤ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SINT8:
    ਹਾਲਤ FFI_TYPE_SINT16:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_INT:
#ਜੇ FFI_SIZEOF_JAVA_RAW == 4
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
#ਜੇ_ਅੰਤ
      *(SINT64 *)rvalue <<= 32;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    ਮੂਲ:
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਖਾਲੀ
ffi_java_raw_ਮੰਜ਼ਿਲ_rvalue (ffi_cif *cif, ਖਾਲੀ *rvalue)
{
#ਜੇ ਸ਼ਬਦ_BIGENDIAN && FFI_SIZEOF_ARG == 8
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_UINT8:
    ਹਾਲਤ FFI_TYPE_UINT16:
    ਹਾਲਤ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SINT8:
    ਹਾਲਤ FFI_TYPE_SINT16:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    ਮੂਲ:
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

ਖਾਲੀ ffi_java_raw_call (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
			ffi_java_raw *raw)
{
  ਖਾਲੀ **avalue = (ਖਾਲੀ**) alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ*));
  ffi_java_raw_ਮੰਜ਼ਿਲ_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_ਮੰਜ਼ਿਲ_raw (cif, rvalue);
}

#ਜੇ FFI_CLOSURES		/* base system provides closures */

ਸਥਿਰ ਖਾਲੀ
ffi_java_translate_ਦਲੀਲਾਂ (ffi_cif *cif, ਖਾਲੀ *rvalue,
		    ਖਾਲੀ **avalue, ਖਾਲੀ *user_ਅੰਕੜੇ)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_ਮਾਪ (cif));
  ffi_raw_ਘੇਰਾ *cl = (ffi_raw_ਘੇਰਾ*)user_ਅੰਕੜੇ;

  ffi_java_ptrarray_ਮੰਜ਼ਿਲ_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_ਅੰਕੜੇ);
  ffi_java_raw_ਮੰਜ਼ਿਲ_rvalue (cif, rvalue);
}

ffi_ਸਥਿਤੀ
ffi_prep_java_raw_ਘੇਰਾ_loc (ffi_java_raw_ਘੇਰਾ* cl,
			       ffi_cif *cif,
			       ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ffi_java_raw*,ਖਾਲੀ*),
			       ਖਾਲੀ *user_ਅੰਕੜੇ,
			       ਖਾਲੀ *codeloc)
{
  ffi_ਸਥਿਤੀ status;

  status = ffi_prep_ਘੇਰਾ_loc ((ffi_ਘੇਰਾ*) cl,
				 cif,
				 &ffi_java_translate_ਦਲੀਲਾਂ,
				 codeloc,
				 codeloc);
  ਜੇ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
    }

  ਵਾਪਸ status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_ਸਥਿਤੀ
ffi_prep_java_raw_ਘੇਰਾ (ffi_java_raw_ਘੇਰਾ* cl,
			   ffi_cif *cif,
			   ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ffi_java_raw*,ਖਾਲੀ*),
			   ਖਾਲੀ *user_ਅੰਕੜੇ)
{
  ਵਾਪਸ ffi_prep_java_raw_ਘੇਰਾ_loc (cl, cif, fun, user_ਅੰਕੜੇ, cl);
}

#ਜੇ_ਅੰਤ /* FFI_CLOSURES */
#ਜੇ_ਅੰਤ /* !FFI_NATIVE_RAW_API */
#ਜੇ_ਅੰਤ /* !NO_JAVA_RAW_API */
