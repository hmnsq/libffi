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
ffi_java_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  صحیح i;

  ffi_type **at = cif->arg_types;

  برای (i = cif->nargs-1; i >= 0; i--, at++)
    {
      انتخاب((*at) -> type) {
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_DOUBLE:
	  result += 2 * FFI_SIZEOF_JAVA_RAW;
	  بشکن;
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	حالت FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	پیشفرض:
	  result += FFI_SIZEOF_JAVA_RAW;
      }
    }

  بازگشت result;
}


تهی
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, تهی **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#اگر واژگان_BIGENDIAN

  برای (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      انتخاب ((*tp)->type)
	{
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT8:
	  *args = (تهی*) ((نویسه*)(raw++) + 3);
	  بشکن;

	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT16:
	  *args = (تهی*) ((نویسه*)(raw++) + 2);
	  بشکن;

#اگر FFI_SIZEOF_JAVA_RAW == 8
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_DOUBLE:
	  *args = (تهی *)raw;
	  raw += 2;
	  بشکن;
#endif

	حالت FFI_TYPE_POINTER:
	  *args = (تهی*) &(raw++)->ptr;
	  بشکن;

	حالت FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	پیشفرض:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->size, اندازه(ffi_java_raw)) / اندازه(ffi_java_raw);
	}
    }

#وگرنه /* WORDS_BIGENDIAN */

#اگر !PDP

  /* then assume little endian */
  برای (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#اگر FFI_SIZEOF_JAVA_RAW == 8
      انتخاب((*tp)->type) {
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_DOUBLE:
	  *args = (تهی*) raw;
	  raw += 2;
	  بشکن;
	حالت FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	پیشفرض:
	  *args = (تهی*) raw++;
      }
#وگرنه /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (تهی*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->size, اندازه(ffi_java_raw)) / اندازه(ffi_java_raw);
#endif /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#وگرنه
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

تهی
ffi_java_ptrarray_to_raw (ffi_cif *cif, تهی **args, ffi_java_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  برای (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      انتخاب ((*tp)->type)
	{
	حالت FFI_TYPE_UINT8:
#اگر واژگان_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#وگرنه
	  (raw++)->uint = *(UINT8*) (*args);
#endif
	  بشکن;

	حالت FFI_TYPE_SINT8:
#اگر واژگان_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#وگرنه
	  (raw++)->sint = *(SINT8*) (*args);
#endif
	  بشکن;

	حالت FFI_TYPE_UINT16:
#اگر واژگان_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#وگرنه
	  (raw++)->uint = *(UINT16*) (*args);
#endif
	  بشکن;

	حالت FFI_TYPE_SINT16:
#اگر واژگان_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#وگرنه
	  (raw++)->sint = *(SINT16*) (*args);
#endif
	  بشکن;

	حالت FFI_TYPE_UINT32:
#اگر واژگان_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#وگرنه
	  (raw++)->uint = *(UINT32*) (*args);
#endif
	  بشکن;

	حالت FFI_TYPE_SINT32:
#اگر واژگان_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#وگرنه
	  (raw++)->sint = *(SINT32*) (*args);
#endif
	  بشکن;

	حالت FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  بشکن;

#اگر FFI_SIZEOF_JAVA_RAW == 8
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  بشکن;
#endif

	حالت FFI_TYPE_POINTER:
	  (raw++)->ptr = **(تهی***) args;
	  بشکن;

	پیشفرض:
#اگر FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#وگرنه
	  memcpy ((تهی*) raw->data, (تهی*)*args, (*tp)->size);
	  raw +=
	    FFI_ALIGN ((*tp)->size, اندازه(ffi_java_raw)) / اندازه(ffi_java_raw);
#endif
	}
    }
}

#اگر !FFI_NATIVE_RAW_API

ایستا تهی
ffi_java_rvalue_to_raw (ffi_cif *cif, تهی *rvalue)
{
#اگر واژگان_BIGENDIAN && FFI_SIZEOF_ARG == 8
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_UINT16:
    حالت FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      بشکن;

    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_INT:
#اگر FFI_SIZEOF_JAVA_RAW == 4
    حالت FFI_TYPE_POINTER:
#endif
      *(SINT64 *)rvalue <<= 32;
      بشکن;

    حالت FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    پیشفرض:
      بشکن;
    }
#endif
}

ایستا تهی
ffi_java_raw_to_rvalue (ffi_cif *cif, تهی *rvalue)
{
#اگر واژگان_BIGENDIAN && FFI_SIZEOF_ARG == 8
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_UINT16:
    حالت FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      بشکن;

    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      بشکن;

    حالت FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    پیشفرض:
      بشکن;
    }
#endif
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

تهی ffi_java_raw_call (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
			ffi_java_raw *raw)
{
  تهی **avalue = (تهی**) alloca (cif->nargs * اندازه (تهی*));
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#اگر FFI_CLOSURES		/* base system provides closures */

ایستا تهی
ffi_java_translate_args (ffi_cif *cif, تهی *rvalue,
		    تهی **avalue, تهی *user_data)
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
			       تهی (*fun)(ffi_cif*,تهی*,ffi_java_raw*,تهی*),
			       تهی *user_data,
			       تهی *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_java_translate_args,
				 codeloc,
				 codeloc);
  اگر (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  بازگشت status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_java_raw_closure (ffi_java_raw_closure* cl,
			   ffi_cif *cif,
			   تهی (*fun)(ffi_cif*,تهی*,ffi_java_raw*,تهی*),
			   تهی *user_data)
{
  بازگشت ffi_prep_java_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */
#endif /* !NO_JAVA_RAW_API */
