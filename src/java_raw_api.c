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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଯଦି !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ffi_type **at = cif->arg_types;

  ପାଇଁ (i = cif->nargs-1; i >= 0; i--, at++)
    {
      ବାଛ((*at) -> type) {
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  result += 2 * FFI_SIZEOF_JAVA_RAW;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	ସ୍ଥିତି FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  result += FFI_SIZEOF_JAVA_RAW;
      }
    }

  ଫେରନ୍ତୁ result;
}


ଶୂନ୍ୟ
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, ଶୂନ୍ୟ **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#ଯଦି ଶବ୍ଦଗୁଡ଼ିକ_BIGENDIAN

  ପାଇଁ (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      ବାଛ ((*tp)->type)
	{
	ସ୍ଥିତି FFI_TYPE_UINT8:
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  *args = (ଶୂନ୍ୟ*) ((ଅକ୍ଷର*)(raw++) + 3);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT16:
	ସ୍ଥିତି FFI_TYPE_SINT16:
	  *args = (ଶୂନ୍ୟ*) ((ଅକ୍ଷର*)(raw++) + 2);
	  ଭାଙ୍ଗ;

#ଯଦି FFI_SIZEOF_JAVA_RAW == 8
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  *args = (ଶୂନ୍ୟ *)raw;
	  raw += 2;
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_POINTER:
	  *args = (ଶୂନ୍ୟ*) &(raw++)->ptr;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->size, ଆକାର(ffi_java_raw)) / ଆକାର(ffi_java_raw);
	}
    }

#ଅନ୍ୟଥା /* WORDS_BIGENDIAN */

#ଯଦି !PDP

  /* then assume little endian */
  ପାଇଁ (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#ଯଦି FFI_SIZEOF_JAVA_RAW == 8
      ବାଛ((*tp)->type) {
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  *args = (ଶୂନ୍ୟ*) raw;
	  raw += 2;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  *args = (ଶୂନ୍ୟ*) raw++;
      }
#ଅନ୍ୟଥା /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (ଶୂନ୍ୟ*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->size, ଆକାର(ffi_java_raw)) / ଆକାର(ffi_java_raw);
#endif /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#ଅନ୍ୟଥା
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

ଶୂନ୍ୟ
ffi_java_ptrarray_to_raw (ffi_cif *cif, ଶୂନ୍ୟ **args, ffi_java_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  ପାଇଁ (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      ବାଛ ((*tp)->type)
	{
	ସ୍ଥିତି FFI_TYPE_UINT8:
#ଯଦି ଶବ୍ଦଗୁଡ଼ିକ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#ଅନ୍ୟଥା
	  (raw++)->uint = *(UINT8*) (*args);
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_SINT8:
#ଯଦି ଶବ୍ଦଗୁଡ଼ିକ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#ଅନ୍ୟଥା
	  (raw++)->sint = *(SINT8*) (*args);
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT16:
#ଯଦି ଶବ୍ଦଗୁଡ଼ିକ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#ଅନ୍ୟଥା
	  (raw++)->uint = *(UINT16*) (*args);
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_SINT16:
#ଯଦି ଶବ୍ଦଗୁଡ଼ିକ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#ଅନ୍ୟଥା
	  (raw++)->sint = *(SINT16*) (*args);
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT32:
#ଯଦି ଶବ୍ଦଗୁଡ଼ିକ_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#ଅନ୍ୟଥା
	  (raw++)->uint = *(UINT32*) (*args);
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_SINT32:
#ଯଦି ଶବ୍ଦଗୁଡ଼ିକ_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#ଅନ୍ୟଥା
	  (raw++)->sint = *(SINT32*) (*args);
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  ଭାଙ୍ଗ;

#ଯଦି FFI_SIZEOF_JAVA_RAW == 8
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_POINTER:
	  (raw++)->ptr = **(ଶୂନ୍ୟ***) args;
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
#ଯଦି FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#ଅନ୍ୟଥା
	  memcpy ((ଶୂନ୍ୟ*) raw->data, (ଶୂନ୍ୟ*)*args, (*tp)->size);
	  raw +=
	    FFI_ALIGN ((*tp)->size, ଆକାର(ffi_java_raw)) / ଆକାର(ffi_java_raw);
#endif
	}
    }
}

#ଯଦି !FFI_NATIVE_RAW_API

ସ୍ଥିର ଶୂନ୍ୟ
ffi_java_rvalue_to_raw (ffi_cif *cif, ଶୂନ୍ୟ *rvalue)
{
#ଯଦି ଶବ୍ଦଗୁଡ଼ିକ_BIGENDIAN && FFI_SIZEOF_ARG == 8
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_UINT8:
    ସ୍ଥିତି FFI_TYPE_UINT16:
    ସ୍ଥିତି FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_SINT8:
    ସ୍ଥିତି FFI_TYPE_SINT16:
    ସ୍ଥିତି FFI_TYPE_SINT32:
    ସ୍ଥିତି FFI_TYPE_INT:
#ଯଦି FFI_SIZEOF_JAVA_RAW == 4
    ସ୍ଥିତି FFI_TYPE_POINTER:
#endif
      *(SINT64 *)rvalue <<= 32;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଭାଙ୍ଗ;
    }
#endif
}

ସ୍ଥିର ଶୂନ୍ୟ
ffi_java_raw_to_rvalue (ffi_cif *cif, ଶୂନ୍ୟ *rvalue)
{
#ଯଦି ଶବ୍ଦଗୁଡ଼ିକ_BIGENDIAN && FFI_SIZEOF_ARG == 8
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_UINT8:
    ସ୍ଥିତି FFI_TYPE_UINT16:
    ସ୍ଥିତି FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_SINT8:
    ସ୍ଥିତି FFI_TYPE_SINT16:
    ସ୍ଥିତି FFI_TYPE_SINT32:
    ସ୍ଥିତି FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଭାଙ୍ଗ;
    }
#endif
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

ଶୂନ୍ୟ ffi_java_raw_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
			ffi_java_raw *raw)
{
  ଶୂନ୍ୟ **avalue = (ଶୂନ୍ୟ**) alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ*));
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#ଯଦି FFI_CLOSURES		/* base system provides closures */

ସ୍ଥିର ଶୂନ୍ୟ
ffi_java_translate_args (ffi_cif *cif, ଶୂନ୍ୟ *rvalue,
		    ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *user_data)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_size (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_data;

  ffi_java_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_data);
  ffi_java_raw_to_rvalue (cif, rvalue);
}

ffi_status
ffi_prep_java_raw_closure_loc (ffi_java_raw_closure* cl,
			       ffi_cif *cif,
			       ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ffi_java_raw*,ଶୂନ୍ୟ*),
			       ଶୂନ୍ୟ *user_data,
			       ଶୂନ୍ୟ *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_java_translate_args,
				 codeloc,
				 codeloc);
  ଯଦି (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  ଫେରନ୍ତୁ status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_java_raw_closure (ffi_java_raw_closure* cl,
			   ffi_cif *cif,
			   ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ffi_java_raw*,ଶୂନ୍ୟ*),
			   ଶୂନ୍ୟ *user_data)
{
  ଫେରନ୍ତୁ ffi_prep_java_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */
#endif /* !NO_JAVA_RAW_API */
