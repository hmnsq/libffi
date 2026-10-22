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

#تعریف_کړه CIF_FLAGS_CHAR		1	/* for struct only */
#تعریف_کړه CIF_FLAGS_SHORT		2	/* for struct only */
#تعریف_کړه CIF_FLAGS_INT		4
#تعریف_کړه CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

فراغ ffi_call_elfbsd (extended_cif *, بینښه, بینښه, فراغ *,
		       فراغ (*) ());
فراغ *ffi_prep_دلیلونه (extended_cif *ecif, فراغ *ډېری);

فراغ *
ffi_prep_دلیلونه (extended_cif *ecif, فراغ *ډېری)
{
  بینښه صحيح i;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;
  فراغ *struct_ارزښت_ptr;

  argp = ډېری;

  إذا (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_ارزښت_ptr = ecif->rvalue;
  وإلا
    struct_ارزښت_ptr = عدم;

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
	      *(نښهدار صحيح *) argp = (نښهدار صحيح) *(SINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(بینښه صحيح *) argp = (بینښه صحيح) *(UINT8 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(نښهدار صحيح *) argp = (نښهدار صحيح) *(SINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(بینښه صحيح *) argp = (بینښه صحيح) *(UINT16 *) *p_argv;
	      اكسر;

	    حالة FFI_TYPE_STRUCT:
	      حافظه_نقل (argp, *p_argv, z);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT (0);
	    }
	  z = حجم (صحيح);
	}
      وإلا
	{
	  حافظه_نقل (argp, *p_argv, z);

	  /* Align if necessary.  */
	  إذا ((حجم(صحيح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحيح));
	}

      p_argv++;
      argp += z;
    }

  أرجع struct_ارزښت_ptr;
}

ffi_حالت
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
      وإلا إذا (cif->rtype->اندازه == حجم (لنډ))
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
      ffi_call_elfbsd (&ecif, cif->بایټونه, cif->flags, ecif.rvalue, fn);
      اكسر;

    افتراضي:
      FFI_ASSERT (0);
      اكسر;
    }
}

/*
 * Closure API
 */

فراغ ffi_تړون_elfbsd (فراغ);
فراغ ffi_تړون_struct_elfbsd (فراغ);
بینښه صحيح ffi_تړون_elfbsd_inner (ffi_تړون *, فراغ *, حرف *);

ساكن فراغ
ffi_prep_تړون_elfbsd (ffi_cif *cif, فراغ **avalue, حرف *stackp)
{
  بینښه صحيح i;
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

بینښه صحيح
ffi_تړون_elfbsd_inner (ffi_تړون *تړون, فراغ *resp, حرف *ډېری)
{
  ffi_cif *cif;
  فراغ **arg_area;

  cif = تړون->cif;
  arg_area = (فراغ **) alloca (cif->nargs * حجم (فراغ *));

  ffi_prep_تړون_elfbsd (cif, arg_area, ډېری);

  (تړون->fun) (cif, resp, arg_area, تړون->user_معلومات);

  أرجع cif->flags;
}

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون *تړون, ffi_cif *cif,
		      فراغ (*fun)(ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_معلومات, فراغ *codeloc)
{
  حرف *tramp = (حرف *) codeloc;
  فراغ *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(بینښه لنډ *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(بینښه صحيح *)(tramp + 4) = (بینښه صحيح) تړون;
  tramp[8] = 0x50;

  إذا (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_تړون_struct_elfbsd;
  وإلا
    fn = &ffi_تړون_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(بینښه صحيح *)(tramp + 11) = (بینښه صحيح)fn + 2 -
				  (بینښه صحيح)tramp - 9 - 6;

  تړون->cif = cif;
  تړون->user_معلومات = user_معلومات;
  تړون->fun = fun;

  أرجع FFI_OK;
}
