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

#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश <stdlib.h>

#अगर !defined(NO_JAVA_RAW_API)

माप_प्रकार
ffi_java_raw_आकार (ffi_cif *cif)
{
  माप_प्रकार परिणाम = 0;
  पूर्णांक i;

  ffi_type **at = cif->arg_types;

  क्रम (i = cif->nargs-1; i >= 0; i--, at++)
    {
      चयन((*at) -> type) {
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_DOUBLE:
	  परिणाम += 2 * FFI_SIZEOF_JAVA_RAW;
	  अवरोध;
	हाल FFI_TYPE_STRUCT:
	हाल FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  पात();
	हाल FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  पात();
	शेष:
	  परिणाम += FFI_SIZEOF_JAVA_RAW;
      }
    }

  वापस परिणाम;
}


व्योम
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, व्योम **args)
{
  अचिन्हित i;
  ffi_type **tp = cif->arg_types;

#अगर शब्दगण_BIGENDIAN

  क्रम (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      चयन ((*tp)->type)
	{
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT8:
	  *args = (व्योम*) ((अक्षर*)(raw++) + 3);
	  अवरोध;

	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT16:
	  *args = (व्योम*) ((अक्षर*)(raw++) + 2);
	  अवरोध;

#अगर FFI_SIZEOF_JAVA_RAW == 8
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_DOUBLE:
	  *args = (व्योम *)raw;
	  raw += 2;
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_POINTER:
	  *args = (व्योम*) &(raw++)->ptr;
	  अवरोध;

	हाल FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  पात();

	शेष:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->आकार, माप(ffi_java_raw)) / माप(ffi_java_raw);
	}
    }

#अन्यथा /* WORDS_BIGENDIAN */

#अगर !PDP

  /* then assume little endian */
  क्रम (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#अगर FFI_SIZEOF_JAVA_RAW == 8
      चयन((*tp)->type) {
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_DOUBLE:
	  *args = (व्योम*) raw;
	  raw += 2;
	  अवरोध;
	हाल FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  पात();
	शेष:
	  *args = (व्योम*) raw++;
      }
#अन्यथा /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (व्योम*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->आकार, माप(ffi_java_raw)) / माप(ffi_java_raw);
#पूर्ण_अगर /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#अन्यथा
#त्रुटि "pdp endian not supported"
#पूर्ण_अगर /* ! PDP */

#पूर्ण_अगर /* WORDS_BIGENDIAN */
}

व्योम
ffi_java_ptrarray_to_raw (ffi_cif *cif, व्योम **args, ffi_java_raw *raw)
{
  अचिन्हित i;
  ffi_type **tp = cif->arg_types;

  क्रम (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      चयन ((*tp)->type)
	{
	हाल FFI_TYPE_UINT8:
#अगर शब्दगण_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#अन्यथा
	  (raw++)->uint = *(UINT8*) (*args);
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_SINT8:
#अगर शब्दगण_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#अन्यथा
	  (raw++)->sint = *(SINT8*) (*args);
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_UINT16:
#अगर शब्दगण_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#अन्यथा
	  (raw++)->uint = *(UINT16*) (*args);
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_SINT16:
#अगर शब्दगण_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#अन्यथा
	  (raw++)->sint = *(SINT16*) (*args);
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_UINT32:
#अगर शब्दगण_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#अन्यथा
	  (raw++)->uint = *(UINT32*) (*args);
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_SINT32:
#अगर शब्दगण_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#अन्यथा
	  (raw++)->sint = *(SINT32*) (*args);
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  अवरोध;

#अगर FFI_SIZEOF_JAVA_RAW == 8
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_POINTER:
	  (raw++)->ptr = **(व्योम***) args;
	  अवरोध;

	शेष:
#अगर FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#अन्यथा
	  स_नकल ((व्योम*) raw->तथ्य, (व्योम*)*args, (*tp)->आकार);
	  raw +=
	    FFI_ALIGN ((*tp)->आकार, माप(ffi_java_raw)) / माप(ffi_java_raw);
#पूर्ण_अगर
	}
    }
}

#अगर !FFI_NATIVE_RAW_API

अटल व्योम
ffi_java_rvalue_to_raw (ffi_cif *cif, व्योम *rvalue)
{
#अगर शब्दगण_BIGENDIAN && FFI_SIZEOF_ARG == 8
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_UINT8:
    हाल FFI_TYPE_UINT16:
    हाल FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      अवरोध;

    हाल FFI_TYPE_SINT8:
    हाल FFI_TYPE_SINT16:
    हाल FFI_TYPE_SINT32:
    हाल FFI_TYPE_INT:
#अगर FFI_SIZEOF_JAVA_RAW == 4
    हाल FFI_TYPE_POINTER:
#पूर्ण_अगर
      *(SINT64 *)rvalue <<= 32;
      अवरोध;

    हाल FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      पात();

    शेष:
      अवरोध;
    }
#पूर्ण_अगर
}

अटल व्योम
ffi_java_raw_to_rvalue (ffi_cif *cif, व्योम *rvalue)
{
#अगर शब्दगण_BIGENDIAN && FFI_SIZEOF_ARG == 8
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_UINT8:
    हाल FFI_TYPE_UINT16:
    हाल FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      अवरोध;

    हाल FFI_TYPE_SINT8:
    हाल FFI_TYPE_SINT16:
    हाल FFI_TYPE_SINT32:
    हाल FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      अवरोध;

    हाल FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      पात();

    शेष:
      अवरोध;
    }
#पूर्ण_अगर
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

व्योम ffi_java_raw_call (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
			ffi_java_raw *raw)
{
  व्योम **avalue = (व्योम**) alloca (cif->nargs * माप (व्योम*));
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#अगर FFI_CLOSURES		/* base system provides closures */

अटल व्योम
ffi_java_translate_args (ffi_cif *cif, व्योम *rvalue,
		    व्योम **avalue, व्योम *user_तथ्य)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_आकार (cif));
  ffi_raw_closure *cl = (ffi_raw_closure*)user_तथ्य;

  ffi_java_ptrarray_to_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_तथ्य);
  ffi_java_raw_to_rvalue (cif, rvalue);
}

ffi_status
ffi_prep_java_raw_closure_loc (ffi_java_raw_closure* cl,
			       ffi_cif *cif,
			       व्योम (*fun)(ffi_cif*,व्योम*,ffi_java_raw*,व्योम*),
			       व्योम *user_तथ्य,
			       व्योम *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_java_translate_args,
				 codeloc,
				 codeloc);
  अगर (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_तथ्य = user_तथ्य;
    }

  वापस status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_java_raw_closure (ffi_java_raw_closure* cl,
			   ffi_cif *cif,
			   व्योम (*fun)(ffi_cif*,व्योम*,ffi_java_raw*,व्योम*),
			   व्योम *user_तथ्य)
{
  वापस ffi_prep_java_raw_closure_loc (cl, cif, fun, user_तथ्य, cl);
}

#पूर्ण_अगर /* FFI_CLOSURES */
#पूर्ण_अगर /* !FFI_NATIVE_RAW_API */
#पूर्ण_अगर /* !NO_JAVA_RAW_API */
