/*
 * Copyright (c) 2013 Miodrag Vallat.  <miod@openbsd.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * vax Foreign Function Interface
 *
 * This file attempts to provide all the FFI entry points which can reliably
 * be implemented in C.
 */

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <unistd.h>

#تعریف_کن CIF_FLAGS_CHAR		1	/* for struct only */
#تعریف_کن CIF_FLAGS_SHORT		2	/* for struct only */
#تعریف_کن CIF_FLAGS_INT		4
#تعریف_کن CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

فراغ ffi_call_elfbsd (extended_cif *, بیعلامت, بیعلامت, فراغ *,
		       فراغ (*) ());
فراغ *ffi_prep_آرگومانها (extended_cif *ecif, فراغ *پشته);

فراغ *
ffi_prep_آرگومانها (extended_cif *ecif, فراغ *پشته)
{
  بیعلامت صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;
  فراغ *struct_مقدار_ptr;

  argp = پشته;

  إذا (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_مقدار_ptr = ecif->rvalue;
  وإلا
    struct_مقدار_ptr = عدم;

  p_argv = ecif->avalue;

  لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->اندازه;
      إذا (z < حجم (صحيح))
	{
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(علامتدار صحيح *) argp = (علامتدار صحيح) *(SINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(بیعلامت صحيح *) argp = (بیعلامت صحيح) *(UINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(علامتدار صحيح *) argp = (علامتدار صحيح) *(SINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(بیعلامت صحيح *) argp = (بیعلامت صحيح) *(UINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_STRUCT:
	      کپی_حافظه (argp, *p_argv, z);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT (0);
	    }
	  z = حجم (صحيح);
	}
      وإلا
	{
	  کپی_حافظه (argp, *p_argv, z);

	  /* Align if necessary.  */
	  إذا ((حجم(صحيح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحيح));
	}

      p_argv++;
      argp += z;
    }

  أرجع struct_مقدار_ptr;
}

ffi_وضعیت
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      cif->flags = 0;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      إذا (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  اكسر;
	}

      إذا (cif->rtype->اندازه == حجم (حرف))
	cif->flags = CIF_FLAGS_CHAR;
      وإلا إذا (cif->rtype->اندازه == حجم (کوتاه))
	cif->flags = CIF_FLAGS_SHORT;
      وإلا إذا (cif->rtype->اندازه == حجم (صحيح))
	cif->flags = CIF_FLAGS_INT;
      وإلا إذا (cif->rtype->اندازه == 2 * حجم (صحيح))
	cif->flags = CIF_FLAGS_DINT;
      وإلا
	cif->flags = 0;
      اكسر;

    افتراضي:
      إذا (cif->rtype->اندازه <= حجم (صحيح))
	cif->flags = CIF_FLAGS_INT;
      وإلا
	cif->flags = CIF_FLAGS_DINT;
      اكسر;
    }

  أرجع FFI_OK;
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  إذا (rvalue == عدم
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->اندازه);
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->بایتها, cif->flags, ecif.rvalue, fn);
      اكسر;

    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

/*
 * Closure API
 */

فراغ ffi_بستگی_elfbsd (فراغ);
فراغ ffi_بستگی_struct_elfbsd (فراغ);
بیعلامت صحيح ffi_بستگی_elfbsd_inner (ffi_بستگی *, فراغ *, حرف *);

ساكن فراغ
ffi_prep_بستگی_elfbsd (ffi_cif *cif, فراغ **avalue, حرف *stackp)
{
  بیعلامت صحيح i;
  فراغ **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  لكل (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->اندازه;
      *p_argv = stackp;

      /* Align if necessary */
      إذا ((حجم (صحيح) - 1) & z)
	z = FFI_ALIGN(z, حجم (صحيح));

      p_argv++;
      stackp += z;
    }
}

بیعلامت صحيح
ffi_بستگی_elfbsd_inner (ffi_بستگی *بستگی, فراغ *resp, حرف *پشته)
{
  ffi_cif *cif;
  فراغ **arg_area;

  cif = بستگی->cif;
  arg_area = (فراغ **) alloca (cif->nargs * حجم (فراغ *));

  ffi_prep_بستگی_elfbsd (cif, arg_area, پشته);

  (بستگی->fun) (cif, resp, arg_area, بستگی->user_داده);

  أرجع cif->flags;
}

ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی *بستگی, ffi_cif *cif,
		      فراغ (*fun)(ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_داده, فراغ *codeloc)
{
  حرف *tramp = (حرف *) codeloc;
  فراغ *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(بیعلامت کوتاه *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(بیعلامت صحيح *)(tramp + 4) = (بیعلامت صحيح) بستگی;
  tramp[8] = 0x50;

  إذا (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_بستگی_struct_elfbsd;
  وإلا
    fn = &ffi_بستگی_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(بیعلامت صحيح *)(tramp + 11) = (بیعلامت صحيح)fn + 2 -
				  (بیعلامت صحيح)tramp - 9 - 6;

  بستگی->cif = cif;
  بستگی->user_داده = user_داده;
  بستگی->fun = fun;

  أرجع FFI_OK;
}
