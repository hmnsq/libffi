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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <stdlib.h>

#ఒకవేళ !defined(NO_JAVA_RAW_API)

size_t
ffi_java_raw_కొలత (ffi_cif *cif)
{
  size_t ఫలితం = 0;
  పూర్ణాంకం i;

  ffi_type **at = cif->arg_types;

  కొరకు (i = cif->nargs-1; i >= 0; i--, at++)
    {
      ఎంపిక((*at) -> type) {
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_DOUBLE:
	  ఫలితం += 2 * FFI_SIZEOF_JAVA_RAW;
	  ఆపు;
	స్థితి FFI_TYPE_STRUCT:
	స్థితి FFI_TYPE_సదిశ:
	  /* No structure or vector parameters in Java.	*/
	  abort();
	స్థితి FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	అప్రమేయ:
	  ఫలితం += FFI_SIZEOF_JAVA_RAW;
      }
    }

  తిరిగి ఫలితం;
}


శూన్య
ffi_java_raw_గమ్యం_ptrarray (ffi_cif *cif, ffi_java_raw *raw, శూన్య **వాదనలు)
{
  గుర్తులేని i;
  ffi_type **tp = cif->arg_types;

#ఒకవేళ పదాలు_BIGENDIAN

  కొరకు (i = 0; i < cif->nargs; i++, tp++, వాదనలు++)
    {
      ఎంపిక ((*tp)->type)
	{
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT8:
	  *వాదనలు = (శూన్య*) ((అక్షరం*)(raw++) + 3);
	  ఆపు;

	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT16:
	  *వాదనలు = (శూన్య*) ((అక్షరం*)(raw++) + 2);
	  ఆపు;

#ఒకవేళ FFI_SIZEOF_JAVA_RAW == 8
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_DOUBLE:
	  *వాదనలు = (శూన్య *)raw;
	  raw += 2;
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_సూచిక:
	  *వాదనలు = (శూన్య*) &(raw++)->ptr;
	  ఆపు;

	స్థితి FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();

	అప్రమేయ:
	  *వాదనలు = raw;
	  raw +=
	    FFI_ALIGN ((*tp)->కొలత, పరిమాణం(ffi_java_raw)) / పరిమాణం(ffi_java_raw);
	}
    }

#లేకపోతే /* WORDS_BIGENDIAN */

#ఒకవేళ !PDP

  /* then assume little endian */
  కొరకు (i = 0; i < cif->nargs; i++, tp++, వాదనలు++)
    {
#ఒకవేళ FFI_SIZEOF_JAVA_RAW == 8
      ఎంపిక((*tp)->type) {
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_DOUBLE:
	  *వాదనలు = (శూన్య*) raw;
	  raw += 2;
	  ఆపు;
	స్థితి FFI_TYPE_COMPLEX:
	  /* Not supported yet.  */
	  abort();
	అప్రమేయ:
	  *వాదనలు = (శూన్య*) raw++;
      }
#లేకపోతే /* FFI_SIZEOF_JAVA_RAW != 8 */
	*వాదనలు = (శూన్య*) raw;
	raw +=
	  FFI_ALIGN ((*tp)->కొలత, పరిమాణం(ffi_java_raw)) / పరిమాణం(ffi_java_raw);
#అయితే_ముగింపు /* FFI_SIZEOF_JAVA_RAW == 8 */
    }

#లేకపోతే
#దోషం "pdp endian not supported"
#అయితే_ముగింపు /* ! PDP */

#అయితే_ముగింపు /* WORDS_BIGENDIAN */
}

శూన్య
ffi_java_ptrarray_గమ్యం_raw (ffi_cif *cif, శూన్య **వాదనలు, ffi_java_raw *raw)
{
  గుర్తులేని i;
  ffi_type **tp = cif->arg_types;

  కొరకు (i = 0; i < cif->nargs; i++, tp++, వాదనలు++)
    {
      ఎంపిక ((*tp)->type)
	{
	స్థితి FFI_TYPE_UINT8:
#ఒకవేళ పదాలు_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT8*) (*వాదనలు);
#లేకపోతే
	  (raw++)->uint = *(UINT8*) (*వాదనలు);
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_SINT8:
#ఒకవేళ పదాలు_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT8*) (*వాదనలు);
#లేకపోతే
	  (raw++)->sint = *(SINT8*) (*వాదనలు);
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_UINT16:
#ఒకవేళ పదాలు_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT16*) (*వాదనలు);
#లేకపోతే
	  (raw++)->uint = *(UINT16*) (*వాదనలు);
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_SINT16:
#ఒకవేళ పదాలు_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT16*) (*వాదనలు);
#లేకపోతే
	  (raw++)->sint = *(SINT16*) (*వాదనలు);
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_UINT32:
#ఒకవేళ పదాలు_BIGENDIAN
	  *(UINT32*)(raw++) = *(UINT32*) (*వాదనలు);
#లేకపోతే
	  (raw++)->uint = *(UINT32*) (*వాదనలు);
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_SINT32:
#ఒకవేళ పదాలు_BIGENDIAN
	  *(SINT32*)(raw++) = *(SINT32*) (*వాదనలు);
#లేకపోతే
	  (raw++)->sint = *(SINT32*) (*వాదనలు);
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	  (raw++)->flt = *(FLOAT32*) (*వాదనలు);
	  ఆపు;

#ఒకవేళ FFI_SIZEOF_JAVA_RAW == 8
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_DOUBLE:
	  raw->uint = *(UINT64*) (*వాదనలు);
	  raw += 2;
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_సూచిక:
	  (raw++)->ptr = **(శూన్య***) వాదనలు;
	  ఆపు;

	అప్రమేయ:
#ఒకవేళ FFI_SIZEOF_JAVA_RAW == 8
	  FFI_ASSERT(0);	/* Should have covered all cases */
#లేకపోతే
	  జ్ఞాపకం_నకలు ((శూన్య*) raw->దత్తాంశం, (శూన్య*)*వాదనలు, (*tp)->కొలత);
	  raw +=
	    FFI_ALIGN ((*tp)->కొలత, పరిమాణం(ffi_java_raw)) / పరిమాణం(ffi_java_raw);
#అయితే_ముగింపు
	}
    }
}

#ఒకవేళ !FFI_NATIVE_RAW_API

స్థిర శూన్య
ffi_java_rvalue_గమ్యం_raw (ffi_cif *cif, శూన్య *rvalue)
{
#ఒకవేళ పదాలు_BIGENDIAN && FFI_SIZEOF_ARG == 8
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_UINT8:
    స్థితి FFI_TYPE_UINT16:
    స్థితి FFI_TYPE_UINT32:
      *(UINT64 *)rvalue <<= 32;
      ఆపు;

    స్థితి FFI_TYPE_SINT8:
    స్థితి FFI_TYPE_SINT16:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_INT:
#ఒకవేళ FFI_SIZEOF_JAVA_RAW == 4
    స్థితి FFI_TYPE_సూచిక:
#అయితే_ముగింపు
      *(SINT64 *)rvalue <<= 32;
      ఆపు;

    స్థితి FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    అప్రమేయ:
      ఆపు;
    }
#అయితే_ముగింపు
}

స్థిర శూన్య
ffi_java_raw_గమ్యం_rvalue (ffi_cif *cif, శూన్య *rvalue)
{
#ఒకవేళ పదాలు_BIGENDIAN && FFI_SIZEOF_ARG == 8
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_UINT8:
    స్థితి FFI_TYPE_UINT16:
    స్థితి FFI_TYPE_UINT32:
      *(UINT64 *)rvalue >>= 32;
      ఆపు;

    స్థితి FFI_TYPE_SINT8:
    స్థితి FFI_TYPE_SINT16:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_INT:
      *(SINT64 *)rvalue >>= 32;
      ఆపు;

    స్థితి FFI_TYPE_COMPLEX:
      /* Not supported yet.  */
      abort();

    అప్రమేయ:
      ఆపు;
    }
#అయితే_ముగింపు
}

/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

శూన్య ffi_java_raw_call (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
			ffi_java_raw *raw)
{
  శూన్య **avalue = (శూన్య**) alloca (cif->nargs * పరిమాణం (శూన్య*));
  ffi_java_raw_గమ్యం_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
  ffi_java_rvalue_గమ్యం_raw (cif, rvalue);
}

#ఒకవేళ FFI_CLOSURES		/* base system provides closures */

స్థిర శూన్య
ffi_java_translate_వాదనలు (ffi_cif *cif, శూన్య *rvalue,
		    శూన్య **avalue, శూన్య *user_దత్తాంశం)
{
  ffi_java_raw *raw = (ffi_java_raw*)alloca (ffi_java_raw_కొలత (cif));
  ffi_raw_సంవృతం *cl = (ffi_raw_సంవృతం*)user_దత్తాంశం;

  ffi_java_ptrarray_గమ్యం_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, (ffi_raw*)raw, cl->user_దత్తాంశం);
  ffi_java_raw_గమ్యం_rvalue (cif, rvalue);
}

ffi_స్థితి
ffi_prep_java_raw_సంవృతం_loc (ffi_java_raw_సంవృతం* cl,
			       ffi_cif *cif,
			       శూన్య (*fun)(ffi_cif*,శూన్య*,ffi_java_raw*,శూన్య*),
			       శూన్య *user_దత్తాంశం,
			       శూన్య *codeloc)
{
  ffi_స్థితి status;

  status = ffi_prep_సంవృతం_loc ((ffi_సంవృతం*) cl,
				 cif,
				 &ffi_java_translate_వాదనలు,
				 codeloc,
				 codeloc);
  ఒకవేళ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_దత్తాంశం = user_దత్తాంశం;
    }

  తిరిగి status;
}

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_స్థితి
ffi_prep_java_raw_సంవృతం (ffi_java_raw_సంవృతం* cl,
			   ffi_cif *cif,
			   శూన్య (*fun)(ffi_cif*,శూన్య*,ffi_java_raw*,శూన్య*),
			   శూన్య *user_దత్తాంశం)
{
  తిరిగి ffi_prep_java_raw_సంవృతం_loc (cl, cif, fun, user_దత్తాంశం, cl);
}

#అయితే_ముగింపు /* FFI_CLOSURES */
#అయితే_ముగింపు /* !FFI_NATIVE_RAW_API */
#అయితే_ముగింపు /* !NO_JAVA_RAW_API */
