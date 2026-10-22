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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <stdlib.h>

#எனில் !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  முழு i;

  ffi_type **at = cif->arg_types;

  க்கு (i = cif->nargs-1; i >= 0; i--, at++)
    {
      தேர்வு((*at) -> type) {
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_DOUBLE:
	  result += 2 * FFI_SIZEOF_JAVA_RAW;
	  நிறுத்து;
	நிலை FFI_TYPE_STRUCT:
	நிலை FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	நிலை FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	இயல்பு:
	  result += FFI_SIZEOF_JAVA_RAW;
      }
    }

  திரும்பு result;
}


வெறுமை
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, வெறுமை **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#எனில் சொற்கள்_BIGENDIAN

  க்கு (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      தேர்வு ((*tp)->type)
	{
	நிலை FFI_TYPE_UINT8:
	நிலை FFI_TYPE_SINT8:
	  *args = (வெறுமை*) ((எழுத்து*)(raw++) + 3);
	  நிறுத்து;

	நிலை FFI_TYPE_UINT16:
	நிலை FFI_TYPE_SINT16:
	  *args = (வெறுமை*) ((எழுத்து*)(raw++) + 2);
	  நிறுத்து;

#எனில் FFI_SIZEOF_JAVA_RAW == 8
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_DOUBLE:
	  *args = (வெறுமை *)raw;
	  raw += 2;
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_POINTER:
	  *args = (வெறுமை*) &(raw++)->ptr;
	  நிறுத்து;

	நிலை FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	இயல்பு:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->size, அளவு(ffi_java_raw)) / அளவு(ffi_java_raw);
	}
    }

#இல்லையெனில் /* WORDS_BIGENDIAN */

#எனில் !PDP

  /* then assume little endian */
  க்கு (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#எனில் FFI_SIZEOF_JAVA_RAW == 8
      தேர்வு((*tp)->type) {
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_DOUBLE:
	  *args = (வெறுமை*) raw;
	  raw += 2;
	  நிறுத்து;
	நிலை FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	இயல்பு:
	  *args = (வெறுமை*) raw++;
      }
#இல்லையெனில் /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (வெறுமை*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->size, அளவு(ffi_java_raw)) / அளவு(ffi_java_raw);
#endif /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#இல்லையெனில்
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

வெறுமை
ffi_java_ptrarray_to_raw (ffi_cif *cif, வெறுமை **args, ffi_java_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  க்கு (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      தேர்வு ((*tp)->type)
	{
	நிலை FFI_TYPE_UINT8:
#எனில் சொற்கள்_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#இல்லையெனில்
	  (raw++)->uint = *(UINT8*) (*args);
#endif
	  நிறுத்து;

	நிலை FFI_TYPE_SINT8:
#எனில் சொற்கள்_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#இல்லையெனில்
	  (raw++)->sint = *(SINT8*) (*args);
#endif
	  நிறுத்து;

	நிலை FFI_TYPE_UINT16:
#எனில் சொற்கள்_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#இல்லையெனில்
	  (raw++)->uint = *(UINT16*) (*args);
#endif
	  நிறுத்து;

	நிலை FFI_TYPE_SINT16:
#எனில் சொற்கள்_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#இல்லையெனில்
	  (raw++)->sint = *(SINT16*) (*args);
#endif
	  நிறுத்து;

	நிலை FFI_TYPE_UINT32:
#எனில் சொற்கள்_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#இல்லையெனில்
	  (raw++)->uint = *(UINT32*) (*args);
#endif
	  நிறுத்து;

	நிலை FFI_TYPE_SINT32:
#எனில் சொற்கள்_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#இல்லையெனில்
	  (raw++)->sint = *(SINT32*) (*args);
#endif
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  நிறுத்து;

#எனில் FFI_SIZEOF_JAVA_RAW == 8
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_POINTER:
	  (raw++)->ptr = **(வெறுமை***) args;
	  நிறுத்து;

	இயல்பு:
#எனில் FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#இல்லையெனில்
	  memcpy ((வெறுமை*) raw->data, (வெறுமை*)*args, (*tp)->size);
	  raw +=
	    FFI_ALIGN ((*tp)->size, அளவு(ffi_java_raw)) / அளவு(ffi_java_raw);
#endif
	}
    }
}

#எனில் !FFI_NATIVE_RAW_API

நிலையான வெறுமை
ffi_java_rvalue_to_raw (ffi_cif *cif, வெறுமை *rvalue)
{
#எனில் சொற்கள்_BIGENDIAN && FFI_SIZEOF_ARG == 8
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_UINT8:
    நிலை FFI_TYPE_UINT16:
    நிலை FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      நிறுத்து;

    நிலை FFI_TYPE_SINT8:
    நிலை FFI_TYPE_SINT16:
    நிலை FFI_TYPE_SINT32:
    நிலை FFI_TYPE_INT:
#எனில் FFI_SIZEOF_JAVA_RAW == 4
    நிலை FFI_TYPE_POINTER:
#endif
      *(SINT64 *)rvalue <<= 32;
      நிறுத்து;

    நிலை FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    இயல்பு:
      நிறுத்து;
    }
#endif
}

நிலையான வெறுமை
ffi_java_raw_to_rvalue (ffi_cif *cif, வெறுமை *rvalue)
{
#எனில் சொற்கள்_BIGENDIAN && FFI_SIZEOF_ARG == 8
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_UINT8:
    நிலை FFI_TYPE_UINT16:
    நிலை FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      நிறுத்து;

    நிலை FFI_TYPE_SINT8:
    நிலை FFI_TYPE_SINT16:
    நிலை FFI_TYPE_SINT32:
    நிலை FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      நிறுத்து;

    நிலை FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    இயல்பு:
      நிறுத்து;
    }
#endif
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

வெறுமை ffi_java_raw_call (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
			ffi_java_raw *raw)
{
  வெறுமை **avalue = (வெறுமை**) alloca (cif->nargs * அளவு (வெறுமை*));
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#எனில் FFI_CLOSURES		/* base system provides closures */

நிலையான வெறுமை
ffi_java_translate_args (ffi_cif *cif, வெறுமை *rvalue,
		    வெறுமை **avalue, வெறுமை *user_data)
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
			       வெறுமை (*fun)(ffi_cif*,வெறுமை*,ffi_java_raw*,வெறுமை*),
			       வெறுமை *user_data,
			       வெறுமை *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_java_translate_args,
				 codeloc,
				 codeloc);
  எனில் (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  திரும்பு status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_java_raw_closure (ffi_java_raw_closure* cl,
			   ffi_cif *cif,
			   வெறுமை (*fun)(ffi_cif*,வெறுமை*,ffi_java_raw*,வெறுமை*),
			   வெறுமை *user_data)
{
  திரும்பு ffi_prep_java_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */
#endif /* !NO_JAVA_RAW_API */
