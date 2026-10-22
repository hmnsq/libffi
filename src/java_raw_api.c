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
ffi_java_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  صحيح i;

  ffi_type **at = cif->arg_types;

  لكل (i = cif->nargs-1; i >= 0; i--, at++)
    {
      بدل((*at) -> type) {
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  result += 2 * FFI_SIZEOF_JAVA_RAW;
	  اكسر;
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	افتراضي:
	  result += FFI_SIZEOF_JAVA_RAW;
      }
    }

  أرجع result;
}


فراغ
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, فراغ **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#إذا ڤرکاتاءن_BIGENDIAN

  لكل (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	  *args = (فراغ*) ((حرف*)(raw++) + 3);
	  اكسر;

	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	  *args = (فراغ*) ((حرف*)(raw++) + 2);
	  اكسر;

#إذا FFI_SIZEOF_JAVA_RAW == 8
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  *args = (فراغ *)raw;
	  raw += 2;
	  اكسر;
#endif

	حالة FFI_TYPE_POINTER:
	  *args = (فراغ*) &(raw++)->ptr;
	  اكسر;

	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	افتراضي:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->size, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
	}
    }

#وإلا /* WORDS_BIGENDIAN */

#إذا !PDP

  /* then assume little endian */
  لكل (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#إذا FFI_SIZEOF_JAVA_RAW == 8
      بدل((*tp)->type) {
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  *args = (فراغ*) raw;
	  raw += 2;
	  اكسر;
	حالة FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	افتراضي:
	  *args = (فراغ*) raw++;
      }
#وإلا /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (فراغ*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->size, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
#endif /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#وإلا
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

فراغ
ffi_java_ptrarray_to_raw (ffi_cif *cif, فراغ **args, ffi_java_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  لكل (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      بدل ((*tp)->type)
	{
	حالة FFI_TYPE_UINT8:
#إذا ڤرکاتاءن_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#وإلا
	  (raw++)->uint = *(UINT8*) (*args);
#endif
	  اكسر;

	حالة FFI_TYPE_SINT8:
#إذا ڤرکاتاءن_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#وإلا
	  (raw++)->sint = *(SINT8*) (*args);
#endif
	  اكسر;

	حالة FFI_TYPE_UINT16:
#إذا ڤرکاتاءن_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#وإلا
	  (raw++)->uint = *(UINT16*) (*args);
#endif
	  اكسر;

	حالة FFI_TYPE_SINT16:
#إذا ڤرکاتاءن_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#وإلا
	  (raw++)->sint = *(SINT16*) (*args);
#endif
	  اكسر;

	حالة FFI_TYPE_UINT32:
#إذا ڤرکاتاءن_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#وإلا
	  (raw++)->uint = *(UINT32*) (*args);
#endif
	  اكسر;

	حالة FFI_TYPE_SINT32:
#إذا ڤرکاتاءن_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#وإلا
	  (raw++)->sint = *(SINT32*) (*args);
#endif
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  اكسر;

#إذا FFI_SIZEOF_JAVA_RAW == 8
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  اكسر;
#endif

	حالة FFI_TYPE_POINTER:
	  (raw++)->ptr = **(فراغ***) args;
	  اكسر;

	افتراضي:
#إذا FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#وإلا
	  memcpy ((فراغ*) raw->data, (فراغ*)*args, (*tp)->size);
	  raw +=
	    FFI_ALIGN ((*tp)->size, حجم(ffi_java_raw)) / حجم(ffi_java_raw);
#endif
	}
    }
}

#إذا !FFI_NATIVE_RAW_API

ساكن فراغ
ffi_java_rvalue_to_raw (ffi_cif *cif, فراغ *rvalue)
{
#إذا ڤرکاتاءن_BIGENDIAN && FFI_SIZEOF_ARG == 8
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
    حالة FFI_TYPE_POINTER:
#endif
      *(SINT64 *)rvalue <<= 32;
      اكسر;

    حالة FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    افتراضي:
      اكسر;
    }
#endif
}

ساكن فراغ
ffi_java_raw_to_rvalue (ffi_cif *cif, فراغ *rvalue)
{
#إذا ڤرکاتاءن_BIGENDIAN && FFI_SIZEOF_ARG == 8
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
#endif
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
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#إذا FFI_CLOSURES		/* base system provides closures */

ساكن فراغ
ffi_java_translate_args (ffi_cif *cif, فراغ *rvalue,
		    فراغ **avalue, فراغ *user_data)
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
			       فراغ (*fun)(ffi_cif*,فراغ*,ffi_java_raw*,فراغ*),
			       فراغ *user_data,
			       فراغ *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_java_translate_args,
				 codeloc,
				 codeloc);
  إذا (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  أرجع status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_java_raw_closure (ffi_java_raw_closure* cl,
			   ffi_cif *cif,
			   فراغ (*fun)(ffi_cif*,فراغ*,ffi_java_raw*,فراغ*),
			   فراغ *user_data)
{
  أرجع ffi_prep_java_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */
#endif /* !NO_JAVA_RAW_API */
