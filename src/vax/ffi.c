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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <unistd.h>

#define CIF_FLAGS_CHAR		1	/* for struct only */
#define CIF_FLAGS_SHORT		2	/* for struct only */
#define CIF_FLAGS_INT		4
#define CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

ശൂന്യം ffi_call_elfbsd (extended_cif *, unsigned, unsigned, ശൂന്യം *,
		       ശൂന്യം (*) ());
ശൂന്യം *ffi_prep_args (extended_cif *ecif, ശൂന്യം *അട്ടി);

ശൂന്യം *
ffi_prep_args (extended_cif *ecif, ശൂന്യം *അട്ടി)
{
  unsigned പൂർണ്ണസംഖ്യ i;
  ശൂന്യം **p_argv;
  അക്ഷരം *argp;
  ffi_type **p_arg;
  ശൂന്യം *struct_മൂല്യം_ptr;

  argp = അട്ടി;

  എങ്കിൽ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_മൂല്യം_ptr = ecif->rvalue;
  അല്ലെങ്കിൽ
    struct_മൂല്യം_ptr = ശൂന്യ;

  p_argv = ecif->avalue;

  വേണ്ടി (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      എങ്കിൽ (z < വലുപ്പം (പൂർണ്ണസംഖ്യ))
	{
	  തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT8:
	      *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ) *(SINT8 *) *p_argv;
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_UINT8:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ) *(UINT8 *) *p_argv;
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_SINT16:
	      *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ) *(SINT16 *) *p_argv;
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_UINT16:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ) *(UINT16 *) *p_argv;
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      നിർത്തുക;

	    സ്ഥിരസ്ഥിതി:
	      FFI_ASSERT (0);
	    }
	  z = വലുപ്പം (പൂർണ്ണസംഖ്യ);
	}
      അല്ലെങ്കിൽ
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  എങ്കിൽ ((വലുപ്പം(പൂർണ്ണസംഖ്യ) - 1) & z)
	    z = FFI_ALIGN(z, വലുപ്പം(പൂർണ്ണസംഖ്യ));
	}

      p_argv++;
      argp += z;
    }

  തിരികെ struct_മൂല്യം_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      cif->flags = 0;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
      എങ്കിൽ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  നിർത്തുക;
	}

      എങ്കിൽ (cif->rtype->size == വലുപ്പം (അക്ഷരം))
	cif->flags = CIF_FLAGS_CHAR;
      അല്ലെങ്കിൽ എങ്കിൽ (cif->rtype->size == വലുപ്പം (short))
	cif->flags = CIF_FLAGS_SHORT;
      അല്ലെങ്കിൽ എങ്കിൽ (cif->rtype->size == വലുപ്പം (പൂർണ്ണസംഖ്യ))
	cif->flags = CIF_FLAGS_INT;
      അല്ലെങ്കിൽ എങ്കിൽ (cif->rtype->size == 2 * വലുപ്പം (പൂർണ്ണസംഖ്യ))
	cif->flags = CIF_FLAGS_DINT;
      അല്ലെങ്കിൽ
	cif->flags = 0;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      എങ്കിൽ (cif->rtype->size <= വലുപ്പം (പൂർണ്ണസംഖ്യ))
	cif->flags = CIF_FLAGS_INT;
      അല്ലെങ്കിൽ
	cif->flags = CIF_FLAGS_DINT;
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn) (), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  എങ്കിൽ (rvalue == ശൂന്യ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->size);
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT (0);
      നിർത്തുക;
    }
}

/*
 * Closure API
 */

ശൂന്യം ffi_closure_elfbsd (ശൂന്യം);
ശൂന്യം ffi_closure_struct_elfbsd (ശൂന്യം);
unsigned പൂർണ്ണസംഖ്യ ffi_closure_elfbsd_inner (ffi_closure *, ശൂന്യം *, അക്ഷരം *);

സ്ഥിതം ശൂന്യം
ffi_prep_closure_elfbsd (ffi_cif *cif, ശൂന്യം **avalue, അക്ഷരം *stackp)
{
  unsigned പൂർണ്ണസംഖ്യ i;
  ശൂന്യം **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  വേണ്ടി (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      *p_argv = stackp;

      /* Align if necessary */
      എങ്കിൽ ((വലുപ്പം (പൂർണ്ണസംഖ്യ) - 1) & z)
	z = FFI_ALIGN(z, വലുപ്പം (പൂർണ്ണസംഖ്യ));

      p_argv++;
      stackp += z;
    }
}

unsigned പൂർണ്ണസംഖ്യ
ffi_closure_elfbsd_inner (ffi_closure *closure, ശൂന്യം *resp, അക്ഷരം *അട്ടി)
{
  ffi_cif *cif;
  ശൂന്യം **arg_area;

  cif = closure->cif;
  arg_area = (ശൂന്യം **) alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));

  ffi_prep_closure_elfbsd (cif, arg_area, അട്ടി);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  തിരികെ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      ശൂന്യം (*fun)(ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
		      ശൂന്യം *user_data, ശൂന്യം *codeloc)
{
  അക്ഷരം *tramp = (അക്ഷരം *) codeloc;
  ശൂന്യം *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(unsigned short *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(unsigned പൂർണ്ണസംഖ്യ *)(tramp + 4) = (unsigned പൂർണ്ണസംഖ്യ) closure;
  tramp[8] = 0x50;

  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  അല്ലെങ്കിൽ
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(unsigned പൂർണ്ണസംഖ്യ *)(tramp + 11) = (unsigned പൂർണ്ണസംഖ്യ)fn + 2 -
				  (unsigned പൂർണ്ണസംഖ്യ)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  തിരികെ FFI_OK;
}
