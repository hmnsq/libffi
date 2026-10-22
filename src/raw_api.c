/* -----------------------------------------------------------------------
   raw_api.c - Copyright (c) 1999, 2008  Red Hat, Inc.

   Author: Kresten Krab Thorup <krab@gnu.org>

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

/* This file defines generic functions for use with the raw api. */

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#ఒకవేళ !FFI_NO_RAW_API

size_t
ffi_raw_కొలత (ffi_cif *cif)
{
  size_t ఫలితం = 0;
  పూర్ణాంకం i;

  ffi_type **at = cif->arg_types;

  కొరకు (i = cif->nargs-1; i >= 0; i--, at++)
    {
#ఒకవేళ !FFI_NO_STRUCTS
      ఒకవేళ ((*at)->type == FFI_TYPE_STRUCT || (*at)->type == FFI_TYPE_సదిశ)
	ఫలితం += FFI_ALIGN (పరిమాణం (శూన్య*), FFI_SIZEOF_ARG);
      లేకపోతే
#అయితే_ముగింపు
	ఫలితం += FFI_ALIGN ((*at)->కొలత, FFI_SIZEOF_ARG);
    }

  తిరిగి ఫలితం;
}


శూన్య
ffi_raw_గమ్యం_ptrarray (ffi_cif *cif, ffi_raw *raw, శూన్య **వాదనలు)
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
	  *వాదనలు = (శూన్య*) ((అక్షరం*)(raw++) + FFI_SIZEOF_ARG - 1);
	  ఆపు;
	  
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT16:
	  *వాదనలు = (శూన్య*) ((అక్షరం*)(raw++) + FFI_SIZEOF_ARG - 2);
	  ఆపు;

#ఒకవేళ FFI_SIZEOF_ARG >= 4	  
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	  *వాదనలు = (శూన్య*) ((అక్షరం*)(raw++) + FFI_SIZEOF_ARG - 4);
	  ఆపు;
#అయితే_ముగింపు
	
#ఒకవేళ !FFI_NO_STRUCTS
	స్థితి FFI_TYPE_STRUCT:
	స్థితి FFI_TYPE_సదిశ:
	  *వాదనలు = (raw++)->ptr;
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_COMPLEX:
	  *వాదనలు = (raw++)->ptr;
	  ఆపు;

	స్థితి FFI_TYPE_సూచిక:
	  *వాదనలు = (శూన్య*) &(raw++)->ptr;
	  ఆపు;
	  
	అప్రమేయ:
	  *వాదనలు = raw;
	  raw += FFI_ALIGN ((*tp)->కొలత, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }

#లేకపోతే /* WORDS_BIGENDIAN */

#ఒకవేళ !PDP

  /* then assume little endian */
  కొరకు (i = 0; i < cif->nargs; i++, tp++, వాదనలు++)
    {	  
#ఒకవేళ !FFI_NO_STRUCTS
      ఒకవేళ ((*tp)->type == FFI_TYPE_STRUCT || (*tp)->type == FFI_TYPE_సదిశ)
	{
	  *వాదనలు = (raw++)->ptr;
	}
      లేకపోతే
#అయితే_ముగింపు
      ఒకవేళ ((*tp)->type == FFI_TYPE_COMPLEX)
	{
	  *వాదనలు = (raw++)->ptr;
	}
      లేకపోతే
	{
	  *వాదనలు = (శూన్య*) raw;
	  raw += FFI_ALIGN ((*tp)->కొలత, పరిమాణం (శూన్య*)) / పరిమాణం (శూన్య*);
	}
    }

#లేకపోతే
#దోషం "pdp endian not supported"
#అయితే_ముగింపు /* ! PDP */

#అయితే_ముగింపు /* WORDS_BIGENDIAN */
}

శూన్య
ffi_ptrarray_గమ్యం_raw (ffi_cif *cif, శూన్య **వాదనలు, ffi_raw *raw)
{
  గుర్తులేని i;
  ffi_type **tp = cif->arg_types;

  కొరకు (i = 0; i < cif->nargs; i++, tp++, వాదనలు++)
    {	  
      ఎంపిక ((*tp)->type)
	{
	స్థితి FFI_TYPE_UINT8:
	  (raw++)->uint = *(UINT8*) (*వాదనలు);
	  ఆపు;

	స్థితి FFI_TYPE_SINT8:
	  (raw++)->sint = *(SINT8*) (*వాదనలు);
	  ఆపు;

	స్థితి FFI_TYPE_UINT16:
	  (raw++)->uint = *(UINT16*) (*వాదనలు);
	  ఆపు;

	స్థితి FFI_TYPE_SINT16:
	  (raw++)->sint = *(SINT16*) (*వాదనలు);
	  ఆపు;

#ఒకవేళ FFI_SIZEOF_ARG >= 4
	స్థితి FFI_TYPE_UINT32:
	  (raw++)->uint = *(UINT32*) (*వాదనలు);
	  ఆపు;

	స్థితి FFI_TYPE_SINT32:
	  (raw++)->sint = *(SINT32*) (*వాదనలు);
	  ఆపు;
#అయితే_ముగింపు

#ఒకవేళ !FFI_NO_STRUCTS
	స్థితి FFI_TYPE_STRUCT:
	స్థితి FFI_TYPE_సదిశ:
	  (raw++)->ptr = *వాదనలు;
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_COMPLEX:
	  (raw++)->ptr = *వాదనలు;
	  ఆపు;

	స్థితి FFI_TYPE_సూచిక:
	  (raw++)->ptr = **(శూన్య***) వాదనలు;
	  ఆపు;

	అప్రమేయ:
	  జ్ఞాపకం_నకలు ((శూన్య*) raw->దత్తాంశం, (శూన్య*)*వాదనలు, (*tp)->కొలత);
	  raw += FFI_ALIGN ((*tp)->కొలత, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	}
    }
}

#ఒకవేళ !FFI_NATIVE_RAW_API


/* This is a generic definition of ffi_raw_call, to be used if the
 * native system does not provide a machine-specific implementation.
 * Having this, allows code to be written for the raw API, without
 * the need for system-specific code to handle input in that format;
 * these following couple of functions will handle the translation forth
 * and back automatically. */

శూన్య ffi_raw_call (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, ffi_raw *raw)
{
  శూన్య **avalue = (శూన్య**) alloca (cif->nargs * పరిమాణం (శూన్య*));
  ffi_raw_గమ్యం_ptrarray (cif, raw, avalue);
  ffi_call (cif, fn, rvalue, avalue);
}

#ఒకవేళ FFI_CLOSURES		/* base system provides closures */

స్థిర శూన్య
ffi_translate_వాదనలు (ffi_cif *cif, శూన్య *rvalue,
		    శూన్య **avalue, శూన్య *user_దత్తాంశం)
{
  ffi_raw *raw = (ffi_raw*)alloca (ffi_raw_కొలత (cif));
  ffi_raw_సంవృతం *cl = (ffi_raw_సంవృతం*)user_దత్తాంశం;

  ffi_ptrarray_గమ్యం_raw (cif, avalue, raw);
  (*cl->fun) (cif, rvalue, raw, cl->user_దత్తాంశం);
}

ffi_స్థితి
ffi_prep_raw_సంవృతం_loc (ffi_raw_సంవృతం* cl,
			  ffi_cif *cif,
			  శూన్య (*fun)(ffi_cif*,శూన్య*,ffi_raw*,శూన్య*),
			  శూన్య *user_దత్తాంశం,
			  శూన్య *codeloc)
{
  ffi_స్థితి status;

  status = ffi_prep_సంవృతం_loc ((ffi_సంవృతం*) cl,
				 cif,
				 &ffi_translate_వాదనలు,
				 codeloc,
				 codeloc);
  ఒకవేళ (status == FFI_OK)
    {
      cl->fun       = fun;
      cl->user_దత్తాంశం = user_దత్తాంశం;
    }

  తిరిగి status;
}

#అయితే_ముగింపు /* FFI_CLOSURES */
#అయితే_ముగింపు /* !FFI_NATIVE_RAW_API */

#ఒకవేళ FFI_CLOSURES

/* Again, here is the generic version of ffi_prep_raw_closure, which
 * will install an intermediate "hub" for translation of arguments from
 * the pointer-array format, to the raw format */

ffi_స్థితి
ffi_prep_raw_సంవృతం (ffi_raw_సంవృతం* cl,
		      ffi_cif *cif,
		      శూన్య (*fun)(ffi_cif*,శూన్య*,ffi_raw*,శూన్య*),
		      శూన్య *user_దత్తాంశం)
{
  తిరిగి ffi_prep_raw_సంవృతం_loc (cl, cif, fun, user_దత్తాంశం, cl);
}

#అయితే_ముగింపు /* FFI_CLOSURES */

#అయితే_ముగింపు /* !FFI_NO_RAW_API */
