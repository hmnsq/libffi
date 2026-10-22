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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <stdlib.h>

#যদি !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  পূর্ণাঙ্ক i;

  ffi_type **at = cif->arg_types;

  প্রতি (i = cif->nargs-1; i >= 0; i--, at++)
    {
      বিকল্প((*at) -> type) {
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_DOUBLE:
	  result += 2 * FFI_SIZEOF_JAVA_RAW;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_STRUCT:
	পক্ষ FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	পক্ষ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	স্বভাব:
	  result += FFI_SIZEOF_JAVA_RAW;
      }
    }

  প্রত্যাগম result;
}


শূন্য
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, শূন্য **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#যদি শব্দগুলি_BIGENDIAN

  প্রতি (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      বিকল্প ((*tp)->type)
	{
	পক্ষ FFI_TYPE_UINT8:
	পক্ষ FFI_TYPE_SINT8:
	  *args = (শূন্য*) ((বর্ণ*)(raw++) + 3);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT16:
	পক্ষ FFI_TYPE_SINT16:
	  *args = (শূন্য*) ((বর্ণ*)(raw++) + 2);
	  ভঙ্গ;

#যদি FFI_SIZEOF_JAVA_RAW == 8
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_DOUBLE:
	  *args = (শূন্য *)raw;
	  raw += 2;
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_POINTER:
	  *args = (শূন্য*) &(raw++)->ptr;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	স্বভাব:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->size, পরিমাণ(ffi_java_raw)) / পরিমাণ(ffi_java_raw);
	}
    }

#অন্যথা /* WORDS_BIGENDIAN */

#যদি !PDP

  /* then assume little endian */
  প্রতি (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#যদি FFI_SIZEOF_JAVA_RAW == 8
      বিকল্প((*tp)->type) {
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_DOUBLE:
	  *args = (শূন্য*) raw;
	  raw += 2;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	স্বভাব:
	  *args = (শূন্য*) raw++;
      }
#অন্যথা /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (শূন্য*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->size, পরিমাণ(ffi_java_raw)) / পরিমাণ(ffi_java_raw);
#endif /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#অন্যথা
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

শূন্য
ffi_java_ptrarray_to_raw (ffi_cif *cif, শূন্য **args, ffi_java_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  প্রতি (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      বিকল্প ((*tp)->type)
	{
	পক্ষ FFI_TYPE_UINT8:
#যদি শব্দগুলি_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#অন্যথা
	  (raw++)->uint = *(UINT8*) (*args);
#endif
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT8:
#যদি শব্দগুলি_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#অন্যথা
	  (raw++)->sint = *(SINT8*) (*args);
#endif
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT16:
#যদি শব্দগুলি_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#অন্যথা
	  (raw++)->uint = *(UINT16*) (*args);
#endif
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT16:
#যদি শব্দগুলি_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#অন্যথা
	  (raw++)->sint = *(SINT16*) (*args);
#endif
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT32:
#যদি শব্দগুলি_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#অন্যথা
	  (raw++)->uint = *(UINT32*) (*args);
#endif
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT32:
#যদি শব্দগুলি_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#অন্যথা
	  (raw++)->sint = *(SINT32*) (*args);
#endif
	  ভঙ্গ;

	পক্ষ FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  ভঙ্গ;

#যদি FFI_SIZEOF_JAVA_RAW == 8
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_POINTER:
	  (raw++)->ptr = **(শূন্য***) args;
	  ভঙ্গ;

	স্বভাব:
#যদি FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#অন্যথা
	  memcpy ((শূন্য*) raw->data, (শূন্য*)*args, (*tp)->size);
	  raw +=
	    FFI_ALIGN ((*tp)->size, পরিমাণ(ffi_java_raw)) / পরিমাণ(ffi_java_raw);
#endif
	}
    }
}

#যদি !FFI_NATIVE_RAW_API

স্থির শূন্য
ffi_java_rvalue_to_raw (ffi_cif *cif, শূন্য *rvalue)
{
#যদি শব্দগুলি_BIGENDIAN && FFI_SIZEOF_ARG == 8
  বিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_UINT8:
    পক্ষ FFI_TYPE_UINT16:
    পক্ষ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      ভঙ্গ;

    পক্ষ FFI_TYPE_SINT8:
    পক্ষ FFI_TYPE_SINT16:
    পক্ষ FFI_TYPE_SINT32:
    পক্ষ FFI_TYPE_INT:
#যদি FFI_SIZEOF_JAVA_RAW == 4
    পক্ষ FFI_TYPE_POINTER:
#endif
      *(SINT64 *)rvalue <<= 32;
      ভঙ্গ;

    পক্ষ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    স্বভাব:
      ভঙ্গ;
    }
#endif
}

স্থির শূন্য
ffi_java_raw_to_rvalue (ffi_cif *cif, শূন্য *rvalue)
{
#যদি শব্দগুলি_BIGENDIAN && FFI_SIZEOF_ARG == 8
  বিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_UINT8:
    পক্ষ FFI_TYPE_UINT16:
    পক্ষ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      ভঙ্গ;

    পক্ষ FFI_TYPE_SINT8:
    পক্ষ FFI_TYPE_SINT16:
    পক্ষ FFI_TYPE_SINT32:
    পক্ষ FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      ভঙ্গ;

    পক্ষ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    স্বভাব:
      ভঙ্গ;
    }
#endif
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

শূন্য ffi_java_raw_call (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
			ffi_java_raw *raw)
{
  শূন্য **avalue = (শূন্য**) alloca (cif->nargs * পরিমাণ (শূন্য*));
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#যদি FFI_CLOSURES		/* base system provides closures */

স্থির শূন্য
ffi_java_translate_args (ffi_cif *cif, শূন্য *rvalue,
		    শূন্য **avalue, শূন্য *user_data)
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
			       শূন্য (*fun)(ffi_cif*,শূন্য*,ffi_java_raw*,শূন্য*),
			       শূন্য *user_data,
			       শূন্য *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_java_translate_args,
				 codeloc,
				 codeloc);
  যদি (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  প্রত্যাগম status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_java_raw_closure (ffi_java_raw_closure* cl,
			   ffi_cif *cif,
			   শূন্য (*fun)(ffi_cif*,শূন্য*,ffi_java_raw*,শূন্য*),
			   শূন্য *user_data)
{
  প্রত্যাগম ffi_prep_java_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */
#endif /* !NO_JAVA_RAW_API */
