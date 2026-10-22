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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಯದಿ !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  ಪೂರ್ಣಾಙ್ಕ i;

  ffi_type **at = cif->arg_types;

  ಪ್ರತಿ (i = cif->nargs-1; i >= 0; i--, at++)
    {
      ವಿಕಲ್ಪ((*at) -> type) {
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  result += 2 * FFI_SIZEOF_JAVA_RAW;
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	ಪಕ್ಷ FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	ಸ್ವಭಾವ:
	  result += FFI_SIZEOF_JAVA_RAW;
      }
    }

  ಪ್ರತ್ಯಾಗಮ result;
}


ಶೂನ್ಯ
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, ಶೂನ್ಯ **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#ಯದಿ ಪದಗಳು_BIGENDIAN

  ಪ್ರತಿ (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      ವಿಕಲ್ಪ ((*tp)->type)
	{
	ಪಕ್ಷ FFI_TYPE_UINT8:
	ಪಕ್ಷ FFI_TYPE_SINT8:
	  *args = (ಶೂನ್ಯ*) ((ವರ್ಣ*)(raw++) + 3);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT16:
	ಪಕ್ಷ FFI_TYPE_SINT16:
	  *args = (ಶೂನ್ಯ*) ((ವರ್ಣ*)(raw++) + 2);
	  ಭಙ್ಗ;

#ಯದಿ FFI_SIZEOF_JAVA_RAW == 8
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  *args = (ಶೂನ್ಯ *)raw;
	  raw += 2;
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_POINTER:
	  *args = (ಶೂನ್ಯ*) &(raw++)->ptr;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	ಸ್ವಭಾವ:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->size, ಪರಿಮಾಣ(ffi_java_raw)) / ಪರಿಮಾಣ(ffi_java_raw);
	}
    }

#ಅನ್ಯಥಾ /* WORDS_BIGENDIAN */

#ಯದಿ !PDP

  /* then assume little endian */
  ಪ್ರತಿ (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#ಯದಿ FFI_SIZEOF_JAVA_RAW == 8
      ವಿಕಲ್ಪ((*tp)->type) {
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  *args = (ಶೂನ್ಯ*) raw;
	  raw += 2;
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	ಸ್ವಭಾವ:
	  *args = (ಶೂನ್ಯ*) raw++;
      }
#ಅನ್ಯಥಾ /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (ಶೂನ್ಯ*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->size, ಪರಿಮಾಣ(ffi_java_raw)) / ಪರಿಮಾಣ(ffi_java_raw);
#endif /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#ಅನ್ಯಥಾ
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

ಶೂನ್ಯ
ffi_java_ptrarray_to_raw (ffi_cif *cif, ಶೂನ್ಯ **args, ffi_java_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  ಪ್ರತಿ (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      ವಿಕಲ್ಪ ((*tp)->type)
	{
	ಪಕ್ಷ FFI_TYPE_UINT8:
#ಯದಿ ಪದಗಳು_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#ಅನ್ಯಥಾ
	  (raw++)->uint = *(UINT8*) (*args);
#endif
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT8:
#ಯದಿ ಪದಗಳು_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#ಅನ್ಯಥಾ
	  (raw++)->sint = *(SINT8*) (*args);
#endif
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT16:
#ಯದಿ ಪದಗಳು_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#ಅನ್ಯಥಾ
	  (raw++)->uint = *(UINT16*) (*args);
#endif
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT16:
#ಯದಿ ಪದಗಳು_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#ಅನ್ಯಥಾ
	  (raw++)->sint = *(SINT16*) (*args);
#endif
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT32:
#ಯದಿ ಪದಗಳು_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#ಅನ್ಯಥಾ
	  (raw++)->uint = *(UINT32*) (*args);
#endif
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT32:
#ಯದಿ ಪದಗಳು_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#ಅನ್ಯಥಾ
	  (raw++)->sint = *(SINT32*) (*args);
#endif
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  ಭಙ್ಗ;

#ಯದಿ FFI_SIZEOF_JAVA_RAW == 8
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_POINTER:
	  (raw++)->ptr = **(ಶೂನ್ಯ***) args;
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
#ಯದಿ FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#ಅನ್ಯಥಾ
	  memcpy ((ಶೂನ್ಯ*) raw->data, (ಶೂನ್ಯ*)*args, (*tp)->size);
	  raw +=
	    FFI_ALIGN ((*tp)->size, ಪರಿಮಾಣ(ffi_java_raw)) / ಪರಿಮಾಣ(ffi_java_raw);
#endif
	}
    }
}

#ಯದಿ !FFI_NATIVE_RAW_API

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_java_rvalue_to_raw (ffi_cif *cif, ಶೂನ್ಯ *rvalue)
{
#ಯದಿ ಪದಗಳು_BIGENDIAN && FFI_SIZEOF_ARG == 8
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_UINT8:
    ಪಕ್ಷ FFI_TYPE_UINT16:
    ಪಕ್ಷ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_SINT8:
    ಪಕ್ಷ FFI_TYPE_SINT16:
    ಪಕ್ಷ FFI_TYPE_SINT32:
    ಪಕ್ಷ FFI_TYPE_INT:
#ಯದಿ FFI_SIZEOF_JAVA_RAW == 4
    ಪಕ್ಷ FFI_TYPE_POINTER:
#endif
      *(SINT64 *)rvalue <<= 32;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    ಸ್ವಭಾವ:
      ಭಙ್ಗ;
    }
#endif
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_java_raw_to_rvalue (ffi_cif *cif, ಶೂನ್ಯ *rvalue)
{
#ಯದಿ ಪದಗಳು_BIGENDIAN && FFI_SIZEOF_ARG == 8
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_UINT8:
    ಪಕ್ಷ FFI_TYPE_UINT16:
    ಪಕ್ಷ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_SINT8:
    ಪಕ್ಷ FFI_TYPE_SINT16:
    ಪಕ್ಷ FFI_TYPE_SINT32:
    ಪಕ್ಷ FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    ಸ್ವಭಾವ:
      ಭಙ್ಗ;
    }
#endif
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

ಶೂನ್ಯ ffi_java_raw_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
			ffi_java_raw *raw)
{
  ಶೂನ್ಯ **avalue = (ಶೂನ್ಯ**) alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ*));
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#ಯದಿ FFI_CLOSURES		/* base system provides closures */

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_java_translate_args (ffi_cif *cif, ಶೂನ್ಯ *rvalue,
		    ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *user_data)
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
			       ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ffi_java_raw*,ಶೂನ್ಯ*),
			       ಶೂನ್ಯ *user_data,
			       ಶೂನ್ಯ *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_java_translate_args,
				 codeloc,
				 codeloc);
  ಯದಿ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  ಪ್ರತ್ಯಾಗಮ status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_java_raw_closure (ffi_java_raw_closure* cl,
			   ffi_cif *cif,
			   ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ffi_java_raw*,ಶೂನ್ಯ*),
			   ಶೂನ್ಯ *user_data)
{
  ಪ್ರತ್ಯಾಗಮ ffi_prep_java_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */
#endif /* !NO_JAVA_RAW_API */
