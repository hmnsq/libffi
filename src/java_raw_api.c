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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ <stdlib.h>

#જો !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_size (ffi_cif *cif)
{
  size_t result = 0;
  પૂર્ણાંક i;

  ffi_type **at = cif->arg_types;

  માટે (i = cif->nargs-1; i >= 0; i--, at++)
    {
      પસંદગી((*at) -> type) {
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_DOUBLE:
	  result += 2 * FFI_SIZEOF_JAVA_RAW;
	  તોડો;
	સ્થિતિ FFI_TYPE_STRUCT:
	સ્થિતિ FFI_TYPE_VECTOR:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	સ્થિતિ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	મૂળભૂત:
	  result += FFI_SIZEOF_JAVA_RAW;
      }
    }

  પાછા result;
}


શૂન્ય
ffi_java_raw_to_ptrarray (ffi_cif *cif, ffi_java_raw *raw, શૂન્ય **args)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

#જો શબ્દો_BIGENDIAN

  માટે (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      પસંદગી ((*tp)->type)
	{
	સ્થિતિ FFI_TYPE_UINT8:
	સ્થિતિ FFI_TYPE_SINT8:
	  *args = (શૂન્ય*) ((અક્ષર*)(raw++) + 3);
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT16:
	સ્થિતિ FFI_TYPE_SINT16:
	  *args = (શૂન્ય*) ((અક્ષર*)(raw++) + 2);
	  તોડો;

#જો FFI_SIZEOF_JAVA_RAW == 8
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_DOUBLE:
	  *args = (શૂન્ય *)raw;
	  raw += 2;
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_POINTER:
	  *args = (શૂન્ય*) &(raw++)->ptr;
	  તોડો;

	સ્થિતિ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	મૂળભૂત:
	  *args = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->size, કદ(ffi_java_raw)) / કદ(ffi_java_raw);
	}
    }

#નહીંતર /* WORDS_BIGENDIAN */

#જો !PDP

  /* then assume little endian */
  માટે (i = 0; i < cif->nargs; i++, tp++, args++)
    {
#જો FFI_SIZEOF_JAVA_RAW == 8
      પસંદગી((*tp)->type) {
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_DOUBLE:
	  *args = (શૂન્ય*) raw;
	  raw += 2;
	  તોડો;
	સ્થિતિ FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	મૂળભૂત:
	  *args = (શૂન્ય*) raw++;
      }
#નહીંતર /* FFI_SIZEOF_JAVA_RAW != 8 */
	*args = (શૂન્ય*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->size, કદ(ffi_java_raw)) / કદ(ffi_java_raw);
#endif /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#નહીંતર
#error "pdp endian not supported"
#endif /* ! PDP */

#endif /* WORDS_BIGENDIAN */
}

શૂન્ય
ffi_java_ptrarray_to_raw (ffi_cif *cif, શૂન્ય **args, ffi_java_raw *raw)
{
  unsigned i;
  ffi_type **tp = cif->arg_types;

  માટે (i = 0; i < cif->nargs; i++, tp++, args++)
    {
      પસંદગી ((*tp)->type)
	{
	સ્થિતિ FFI_TYPE_UINT8:
#જો શબ્દો_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*args);
#નહીંતર
	  (raw++)->uint = *(UINT8*) (*args);
#endif
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT8:
#જો શબ્દો_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*args);
#નહીંતર
	  (raw++)->sint = *(SINT8*) (*args);
#endif
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT16:
#જો શબ્દો_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*args);
#નહીંતર
	  (raw++)->uint = *(UINT16*) (*args);
#endif
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT16:
#જો શબ્દો_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*args);
#નહીંતર
	  (raw++)->sint = *(SINT16*) (*args);
#endif
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT32:
#જો શબ્દો_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*args);
#નહીંતર
	  (raw++)->uint = *(UINT32*) (*args);
#endif
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT32:
#જો શબ્દો_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*args);
#નહીંતર
	  (raw++)->sint = *(SINT32*) (*args);
#endif
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*args);
	  તોડો;

#જો FFI_SIZEOF_JAVA_RAW == 8
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*args);
	  raw += 2;
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_POINTER:
	  (raw++)->ptr = **(શૂન્ય***) args;
	  તોડો;

	મૂળભૂત:
#જો FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#નહીંતર
	  memcpy ((શૂન્ય*) raw->data, (શૂન્ય*)*args, (*tp)->size);
	  raw +=
	    FFI_ALIGN ((*tp)->size, કદ(ffi_java_raw)) / કદ(ffi_java_raw);
#endif
	}
    }
}

#જો !FFI_NATIVE_RAW_API

સ્થિર શૂન્ય
ffi_java_rvalue_to_raw (ffi_cif *cif, શૂન્ય *rvalue)
{
#જો શબ્દો_BIGENDIAN && FFI_SIZEOF_ARG == 8
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_UINT8:
    સ્થિતિ FFI_TYPE_UINT16:
    સ્થિતિ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      તોડો;

    સ્થિતિ FFI_TYPE_SINT8:
    સ્થિતિ FFI_TYPE_SINT16:
    સ્થિતિ FFI_TYPE_SINT32:
    સ્થિતિ FFI_TYPE_INT:
#જો FFI_SIZEOF_JAVA_RAW == 4
    સ્થિતિ FFI_TYPE_POINTER:
#endif
      *(SINT64 *)rvalue <<= 32;
      તોડો;

    સ્થિતિ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    મૂળભૂત:
      તોડો;
    }
#endif
}

સ્થિર શૂન્ય
ffi_java_raw_to_rvalue (ffi_cif *cif, શૂન્ય *rvalue)
{
#જો શબ્દો_BIGENDIAN && FFI_SIZEOF_ARG == 8
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_UINT8:
    સ્થિતિ FFI_TYPE_UINT16:
    સ્થિતિ FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      તોડો;

    સ્થિતિ FFI_TYPE_SINT8:
    સ્થિતિ FFI_TYPE_SINT16:
    સ્થિતિ FFI_TYPE_SINT32:
    સ્થિતિ FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      તોડો;

    સ્થિતિ FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    મૂળભૂત:
      તોડો;
    }
#endif
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

શૂન્ય ffi_java_raw_call (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
			ffi_java_raw *raw)
{
  શૂન્ય **avalue = (શૂન્ય**) alloca (cif->nargs * કદ (શૂન્ય*));
  ffi_java_raw_to_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_to_raw (cif, rvalue);
}

#જો FFI_CLOSURES		/* base system provides closures */

સ્થિર શૂન્ય
ffi_java_translate_args (ffi_cif *cif, શૂન્ય *rvalue,
		    શૂન્ય **avalue, શૂન્ય *user_data)
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
			       શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,ffi_java_raw*,શૂન્ય*),
			       શૂન્ય *user_data,
			       શૂન્ય *codeloc)
{
  ffi_status status;

  status = ffi_prep_closure_loc ((ffi_closure*) cl,
				 cif,
				 &ffi_java_translate_args,
				 codeloc,
				 codeloc);
  જો (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_data = user_data;
    }

  પાછા status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_status
ffi_prep_java_raw_closure (ffi_java_raw_closure* cl,
			   ffi_cif *cif,
			   શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,ffi_java_raw*,શૂન્ય*),
			   શૂન્ય *user_data)
{
  પાછા ffi_prep_java_raw_closure_loc (cl, cif, fun, user_data, cl);
}

#endif /* FFI_CLOSURES */
#endif /* !FFI_NATIVE_RAW_API */
#endif /* !NO_JAVA_RAW_API */
