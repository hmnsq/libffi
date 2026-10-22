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

#सचे !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  पूर्णाङ्क i;

  ffi_type **at = cif->arg_types;

  अत्थाय (i = cif->nargs-1; i >= 0; i--, at++)
    {
      विकल्प((*at) -> type) {
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_DOUBLE:
	  result += 2 * FFI_SIZEOF_JAVA_RAW;
	  भङ्ग;
	पक्ष FFI_TYPE_STRUCT:
	पक्ष FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	पक्ष FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	स्वभाव:
	  result += FFI_SIZEOF_JAVA_RAW;
      }
    }

  पटिनिवत्त result;
}


शून्य
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, शून्य **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#सचे शब्दाः_BIGENDIAN

  अत्थाय (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      विकल्प ((*tp)->type)
	{
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT8:
	  *args = (शून्य*) ((वर्ण*)(raw++) + 3);
	  भङ्ग;

	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT16:
	  *args = (शून्य*) ((वर्ण*)(raw++) + 2);
	  भङ्ग;

#सचे FFI_SIZEOF_JAVA_RAW == 8
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_DOUBLE:
	  *args = (शून्य *)raw;
	  raw += 2;
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_POINTER:
	  *args = (शून्य*) &(raw++)->ptr;
	  भङ्ग;

	पक्ष FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	स्वभाव:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->size, परिमाण(ffi_java_raw)) / परिमाण(ffi_java_raw);
	}
    }

#अञ्ञथा /* WORDS_BIGENDIAN */

#सचे !PDP

  /* then assume little endian */
  अत्थाय (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#सचे FFI_SIZEOF_JAVA_RAW == 8
      विकल्प((*tp)->type) {
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_DOUBLE:
	  *args = (शून्य*) raw;
	  raw += 2;
	  भङ्ग;
	पक्ष FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	स्वभाव:
	  *args = (शून्य*) raw++;
      }
#अञ्ञथा /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (शून्य*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->size, परिमाण(ffi_java_raw)) / परिमाण(ffi_java_raw);
#endif /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#अञ्ञथा
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

शून्य
ffi_java_ptrarray_to_raw (ffi_cif *cif, शून्य **args, ffi_java_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  अत्थाय (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      विकल्प ((*tp)->type)
	{
	पक्ष FFI_TYPE_UINT8:
#सचे शब्दाः_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#अञ्ञथा
	  (raw++)->uint = *(UINT8*) (*args);
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_SINT8:
#सचे शब्दाः_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#अञ्ञथा
	  (raw++)->sint = *(SINT8*) (*args);
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_UINT16:
#सचे शब्दाः_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#अञ्ञथा
	  (raw++)->uint = *(UINT16*) (*args);
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_SINT16:
#सचे शब्दाः_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#अञ्ञथा
	  (raw++)->sint = *(SINT16*) (*args);
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_UINT32:
#सचे शब्दाः_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#अञ्ञथा
	  (raw++)->uint = *(UINT32*) (*args);
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_SINT32:
#सचे शब्दाः_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#अञ्ञथा
	  (raw++)->sint = *(SINT32*) (*args);
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  भङ्ग;

#सचे FFI_SIZEOF_JAVA_RAW == 8
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_POINTER:
	  (raw++)->ptr = **(शून्य***) args;
	  भङ्ग;

	स्वभाव:
#सचे FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#अञ्ञथा
	  memcpy ((शून्य*) raw->data, (शून्य*)*args, (*tp)->size);
	  raw +=
	    FFI_ALIGN ((*tp)->size, परिमाण(ffi_java_raw)) / परिमाण(ffi_java_raw);
#endif
	}
    }
}

#सचे !FFI_NATIVE_RAW_API

स्थिर शून्य
ffi_java_rvalue_to_raw (ffi_cif *cif, शून्य *rvalue)
{
#सचे शब्दाः_BIGENDIAN && FFI_SIZEOF_ARG == 8
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
#सचे FFI_SIZEOF_JAVA_RAW == 4
    पक्ष FFI_TYPE_POINTER:
#endif
      *(SINT64 *)rvalue <<= 32;
      भङ्ग;

    पक्ष FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    स्वभाव:
      भङ्ग;
    }
#endif
}

स्थिर शून्य
ffi_java_raw_to_rvalue (ffi_cif *cif, शून्य *rvalue)
{
#सचे शब्दाः_BIGENDIAN && FFI_SIZEOF_ARG == 8
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
#endif
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
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#सचे FFI_CLOSURES		/* base system provides closures */

स्थिर शून्य
ffi_java_translate_args (ffi_cif *cif, शून्य *rvalue,
		    शून्य **avalue, शून्य *user_data)
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
			       शून्य (*fun)(ffi_cif*,शून्य*,ffi_java_raw*,शून्य*),
			       शून्य *user_data,
			       शून्य *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_java_translate_args,
				 codeloc,
				 codeloc);
  सचे (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  पटिनिवत्त status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_java_raw_closure (ffi_java_raw_closure* cl,
			   ffi_cif *cif,
			   शून्य (*fun)(ffi_cif*,शून्य*,ffi_java_raw*,शून्य*),
			   शून्य *user_data)
{
  पटिनिवत्त ffi_prep_java_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */
#endif /* !NO_JAVA_RAW_API */
