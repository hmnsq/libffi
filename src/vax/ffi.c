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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>
#شامل <unistd.h>

#تعریف_کرو CIF_FLAGS_CHAR		1	/* for struct only */
#تعریف_کرو CIF_FLAGS_SHORT		2	/* for struct only */
#تعریف_کرو CIF_FLAGS_INT		4
#تعریف_کرو CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

خالی ffi_call_elfbsd (extended_cif *, غیرعلامتی, غیرعلامتی, خالی *,
		       خالی (*) ());
خالی *ffi_prep_دلائل (extended_cif *ecif, خالی *ڈھیر);

خالی *
ffi_prep_دلائل (extended_cif *ecif, خالی *ڈھیر)
{
  غیرعلامتی صحیح i;
  خالی **p_argv;
  حرف *argp;
  ffi_type **p_arg;
  خالی *struct_قدر_ptr;

  argp = ڈھیر;

  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_قدر_ptr = ecif->rvalue;
  ورنہ
    struct_قدر_ptr = تہی;

  p_argv = ecif->avalue;

  برائے (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->سائز;
      اگر (z < حجم (صحیح))
	{
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(علامتی صحیح *) argp = (علامتی صحیح) *(SINT8 *) *p_argv;
	      توڑو;

	    حالت FFI_TYPE_UINT8:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح) *(UINT8 *) *p_argv;
	      توڑو;

	    حالت FFI_TYPE_SINT16:
	      *(علامتی صحیح *) argp = (علامتی صحیح) *(SINT16 *) *p_argv;
	      توڑو;

	    حالت FFI_TYPE_UINT16:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح) *(UINT16 *) *p_argv;
	      توڑو;

	    حالت FFI_TYPE_STRUCT:
	      حافظہ_نقل (argp, *p_argv, z);
	      توڑو;

	    طےشدہ:
	      FFI_ASSERT (0);
	    }
	  z = حجم (صحیح);
	}
      ورنہ
	{
	  حافظہ_نقل (argp, *p_argv, z);

	  /* Align if necessary.  */
	  اگر ((حجم(صحیح) - 1) & z)
	    z = FFI_ALIGN(z, حجم(صحیح));
	}

      p_argv++;
      argp += z;
    }

  واپس struct_قدر_ptr;
}

ffi_حالت
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      cif->flags = 0;
      توڑو;

    حالت FFI_TYPE_STRUCT:
      اگر (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  توڑو;
	}

      اگر (cif->rtype->سائز == حجم (حرف))
	cif->flags = CIF_FLAGS_CHAR;
      ورنہ اگر (cif->rtype->سائز == حجم (مختصر))
	cif->flags = CIF_FLAGS_SHORT;
      ورنہ اگر (cif->rtype->سائز == حجم (صحیح))
	cif->flags = CIF_FLAGS_INT;
      ورنہ اگر (cif->rtype->سائز == 2 * حجم (صحیح))
	cif->flags = CIF_FLAGS_DINT;
      ورنہ
	cif->flags = 0;
      توڑو;

    طےشدہ:
      اگر (cif->rtype->سائز <= حجم (صحیح))
	cif->flags = CIF_FLAGS_INT;
      ورنہ
	cif->flags = CIF_FLAGS_DINT;
      توڑو;
    }

  واپس FFI_OK;
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn) (), خالی *rvalue, خالی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  اگر (rvalue == تہی
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->سائز);
  ورنہ
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->بائٹیں, cif->flags, ecif.rvalue, fn);
      توڑو;

    طےشدہ:
      FFI_ASSERT (0);
      توڑو;
    }
}

/*
 * Closure API
 */

خالی ffi_احاطہ_elfbsd (خالی);
خالی ffi_احاطہ_struct_elfbsd (خالی);
غیرعلامتی صحیح ffi_احاطہ_elfbsd_inner (ffi_احاطہ *, خالی *, حرف *);

ساکن خالی
ffi_prep_احاطہ_elfbsd (ffi_cif *cif, خالی **avalue, حرف *stackp)
{
  غیرعلامتی صحیح i;
  خالی **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  برائے (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->سائز;
      *p_argv = stackp;

      /* Align if necessary */
      اگر ((حجم (صحیح) - 1) & z)
	z = FFI_ALIGN(z, حجم (صحیح));

      p_argv++;
      stackp += z;
    }
}

غیرعلامتی صحیح
ffi_احاطہ_elfbsd_inner (ffi_احاطہ *احاطہ, خالی *resp, حرف *ڈھیر)
{
  ffi_cif *cif;
  خالی **arg_area;

  cif = احاطہ->cif;
  arg_area = (خالی **) alloca (cif->nargs * حجم (خالی *));

  ffi_prep_احاطہ_elfbsd (cif, arg_area, ڈھیر);

  (احاطہ->fun) (cif, resp, arg_area, احاطہ->user_ڈیٹا);

  واپس cif->flags;
}

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ *احاطہ, ffi_cif *cif,
		      خالی (*fun)(ffi_cif *, خالی *, خالی **, خالی *),
		      خالی *user_ڈیٹا, خالی *codeloc)
{
  حرف *tramp = (حرف *) codeloc;
  خالی *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(غیرعلامتی مختصر *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(غیرعلامتی صحیح *)(tramp + 4) = (غیرعلامتی صحیح) احاطہ;
  tramp[8] = 0x50;

  اگر (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_احاطہ_struct_elfbsd;
  ورنہ
    fn = &ffi_احاطہ_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(غیرعلامتی صحیح *)(tramp + 11) = (غیرعلامتی صحیح)fn + 2 -
				  (غیرعلامتی صحیح)tramp - 9 - 6;

  احاطہ->cif = cif;
  احاطہ->user_ڈیٹا = user_ڈیٹا;
  احاطہ->fun = fun;

  واپس FFI_OK;
}
