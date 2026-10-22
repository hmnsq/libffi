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

#जर !defined(NO_JAVA_RAW_API)

माप_प्रकार
ffi_java_raw_size (ffi_cif *cif)
{
  माप_प्रकार result = 0;
  पूर्णांक i;

  ffi_type **at = cif->arg_types;

  साठी (i = cif->nargs-1; i >= 0; i--, at++)
    {
      निवड((*at) -> type) {
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_DOUBLE:
	  result += 2 * FFI_SIZEOF_JAVA_RAW;
	  थांबा;
	स्थिती FFI_TYPE_STRUCT:
	स्थिती FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  पात();
	स्थिती FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  पात();
	पूर्वनिर्धारित:
	  result += FFI_SIZEOF_JAVA_RAW;
      }
    }

  परत result;
}


शून्य
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, शून्य **args)
{
  अचिन्हित i;
  ffi_type **tp = cif->arg_types;

#जर शब्द_BIGENDIAN

  साठी (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      निवड ((*tp)->type)
	{
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT8:
	  *args = (शून्य*) ((अक्षर*)(raw++) + 3);
	  थांबा;

	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT16:
	  *args = (शून्य*) ((अक्षर*)(raw++) + 2);
	  थांबा;

#जर FFI_SIZEOF_JAVA_RAW == 8
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_DOUBLE:
	  *args = (शून्य *)raw;
	  raw += 2;
	  थांबा;
#endif

	स्थिती FFI_TYPE_POINTER:
	  *args = (शून्य*) &(raw++)->ptr;
	  थांबा;

	स्थिती FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  पात();

	पूर्वनिर्धारित:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->size, आकार(ffi_java_raw)) / आकार(ffi_java_raw);
	}
    }

#अन्यथा /* WORDS_BIGENDIAN */

#जर !PDP

  /* then assume little endian */
  साठी (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#जर FFI_SIZEOF_JAVA_RAW == 8
      निवड((*tp)->type) {
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_DOUBLE:
	  *args = (शून्य*) raw;
	  raw += 2;
	  थांबा;
	स्थिती FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  पात();
	पूर्वनिर्धारित:
	  *args = (शून्य*) raw++;
      }
#अन्यथा /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (शून्य*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->size, आकार(ffi_java_raw)) / आकार(ffi_java_raw);
#endif /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#अन्यथा
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

शून्य
ffi_java_ptrarray_to_raw (ffi_cif *cif, शून्य **args, ffi_java_raw *raw)
{
  अचिन्हित i;
  ffi_type **tp = cif->arg_types;

  साठी (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      निवड ((*tp)->type)
	{
	स्थिती FFI_TYPE_UINT8:
#जर शब्द_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#अन्यथा
	  (raw++)->uint = *(UINT8*) (*args);
#endif
	  थांबा;

	स्थिती FFI_TYPE_SINT8:
#जर शब्द_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#अन्यथा
	  (raw++)->sint = *(SINT8*) (*args);
#endif
	  थांबा;

	स्थिती FFI_TYPE_UINT16:
#जर शब्द_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#अन्यथा
	  (raw++)->uint = *(UINT16*) (*args);
#endif
	  थांबा;

	स्थिती FFI_TYPE_SINT16:
#जर शब्द_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#अन्यथा
	  (raw++)->sint = *(SINT16*) (*args);
#endif
	  थांबा;

	स्थिती FFI_TYPE_UINT32:
#जर शब्द_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#अन्यथा
	  (raw++)->uint = *(UINT32*) (*args);
#endif
	  थांबा;

	स्थिती FFI_TYPE_SINT32:
#जर शब्द_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#अन्यथा
	  (raw++)->sint = *(SINT32*) (*args);
#endif
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  थांबा;

#जर FFI_SIZEOF_JAVA_RAW == 8
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  थांबा;
#endif

	स्थिती FFI_TYPE_POINTER:
	  (raw++)->ptr = **(शून्य***) args;
	  थांबा;

	पूर्वनिर्धारित:
#जर FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#अन्यथा
	  स_नकल ((शून्य*) raw->data, (शून्य*)*args, (*tp)->size);
	  raw +=
	    FFI_ALIGN ((*tp)->size, आकार(ffi_java_raw)) / आकार(ffi_java_raw);
#endif
	}
    }
}

#जर !FFI_NATIVE_RAW_API

स्थिर शून्य
ffi_java_rvalue_to_raw (ffi_cif *cif, शून्य *rvalue)
{
#जर शब्द_BIGENDIAN && FFI_SIZEOF_ARG == 8
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_UINT8:
    स्थिती FFI_TYPE_UINT16:
    स्थिती FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      थांबा;

    स्थिती FFI_TYPE_SINT8:
    स्थिती FFI_TYPE_SINT16:
    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_INT:
#जर FFI_SIZEOF_JAVA_RAW == 4
    स्थिती FFI_TYPE_POINTER:
#endif
      *(SINT64 *)rvalue <<= 32;
      थांबा;

    स्थिती FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      पात();

    पूर्वनिर्धारित:
      थांबा;
    }
#endif
}

स्थिर शून्य
ffi_java_raw_to_rvalue (ffi_cif *cif, शून्य *rvalue)
{
#जर शब्द_BIGENDIAN && FFI_SIZEOF_ARG == 8
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_UINT8:
    स्थिती FFI_TYPE_UINT16:
    स्थिती FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      थांबा;

    स्थिती FFI_TYPE_SINT8:
    स्थिती FFI_TYPE_SINT16:
    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      थांबा;

    स्थिती FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      पात();

    पूर्वनिर्धारित:
      थांबा;
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
  शून्य **avalue = (शून्य**) alloca (cif->nargs * आकार (शून्य*));
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#जर FFI_CLOSURES		/* base system provides closures */

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
  जर (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  परत status;
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
  परत ffi_prep_java_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */
#endif /* !NO_JAVA_RAW_API */
