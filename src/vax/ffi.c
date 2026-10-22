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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <unistd.h>

#define CIF_FLAGS_CHAR		1	/* for struct only */
#define CIF_FLAGS_SHORT		2	/* for struct only */
#define CIF_FLAGS_INT		4
#define CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

શૂન્ય ffi_call_elfbsd (extended_cif *, unsigned, unsigned, શૂન્ય *,
		       શૂન્ય (*) ());
શૂન્ય *ffi_prep_args (extended_cif *ecif, શૂન્ય *ઢગલો);

શૂન્ય *
ffi_prep_args (extended_cif *ecif, શૂન્ય *ઢગલો)
{
  unsigned પૂર્ણાંક i;
  શૂન્ય **p_argv;
  અક્ષર *argp;
  ffi_type **p_arg;
  શૂન્ય *struct_કિંમત_ptr;

  argp = ઢગલો;

  જો (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_કિંમત_ptr = ecif->rvalue;
  નહીંતર
    struct_કિંમત_ptr = ખાલી;

  p_argv = ecif->avalue;

  માટે (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      જો (z < કદ (પૂર્ણાંક))
	{
	  પસંદગી ((*p_arg)->type)
	    {
	    સ્થિતિ FFI_TYPE_SINT8:
	      *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક) *(SINT8 *) *p_argv;
	      તોડો;

	    સ્થિતિ FFI_TYPE_UINT8:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક) *(UINT8 *) *p_argv;
	      તોડો;

	    સ્થિતિ FFI_TYPE_SINT16:
	      *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક) *(SINT16 *) *p_argv;
	      તોડો;

	    સ્થિતિ FFI_TYPE_UINT16:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક) *(UINT16 *) *p_argv;
	      તોડો;

	    સ્થિતિ FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      તોડો;

	    મૂળભૂત:
	      FFI_ASSERT (0);
	    }
	  z = કદ (પૂર્ણાંક);
	}
      નહીંતર
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  જો ((કદ(પૂર્ણાંક) - 1) & z)
	    z = FFI_ALIGN(z, કદ(પૂર્ણાંક));
	}

      p_argv++;
      argp += z;
    }

  પાછા struct_કિંમત_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      cif->flags = 0;
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
      જો (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  તોડો;
	}

      જો (cif->rtype->size == કદ (અક્ષર))
	cif->flags = CIF_FLAGS_CHAR;
      નહીંતર જો (cif->rtype->size == કદ (short))
	cif->flags = CIF_FLAGS_SHORT;
      નહીંતર જો (cif->rtype->size == કદ (પૂર્ણાંક))
	cif->flags = CIF_FLAGS_INT;
      નહીંતર જો (cif->rtype->size == 2 * કદ (પૂર્ણાંક))
	cif->flags = CIF_FLAGS_DINT;
      નહીંતર
	cif->flags = 0;
      તોડો;

    મૂળભૂત:
      જો (cif->rtype->size <= કદ (પૂર્ણાંક))
	cif->flags = CIF_FLAGS_INT;
      નહીંતર
	cif->flags = CIF_FLAGS_DINT;
      તોડો;
    }

  પાછા FFI_OK;
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn) (), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  જો (rvalue == ખાલી
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->size);
  નહીંતર
    ecif.rvalue = rvalue;

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      તોડો;

    મૂળભૂત:
      FFI_ASSERT (0);
      તોડો;
    }
}

/*
 * Closure API
 */

શૂન્ય ffi_closure_elfbsd (શૂન્ય);
શૂન્ય ffi_closure_struct_elfbsd (શૂન્ય);
unsigned પૂર્ણાંક ffi_closure_elfbsd_inner (ffi_closure *, શૂન્ય *, અક્ષર *);

સ્થિર શૂન્ય
ffi_prep_closure_elfbsd (ffi_cif *cif, શૂન્ય **avalue, અક્ષર *stackp)
{
  unsigned પૂર્ણાંક i;
  શૂન્ય **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  માટે (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      *p_argv = stackp;

      /* Align if necessary */
      જો ((કદ (પૂર્ણાંક) - 1) & z)
	z = FFI_ALIGN(z, કદ (પૂર્ણાંક));

      p_argv++;
      stackp += z;
    }
}

unsigned પૂર્ણાંક
ffi_closure_elfbsd_inner (ffi_closure *closure, શૂન્ય *resp, અક્ષર *ઢગલો)
{
  ffi_cif *cif;
  શૂન્ય **arg_area;

  cif = closure->cif;
  arg_area = (શૂન્ય **) alloca (cif->nargs * કદ (શૂન્ય *));

  ffi_prep_closure_elfbsd (cif, arg_area, ઢગલો);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  પાછા cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      શૂન્ય (*fun)(ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
		      શૂન્ય *user_data, શૂન્ય *codeloc)
{
  અક્ષર *tramp = (અક્ષર *) codeloc;
  શૂન્ય *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(unsigned short *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(unsigned પૂર્ણાંક *)(tramp + 4) = (unsigned પૂર્ણાંક) closure;
  tramp[8] = 0x50;

  જો (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  નહીંતર
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(unsigned પૂર્ણાંક *)(tramp + 11) = (unsigned પૂર્ણાંક)fn + 2 -
				  (unsigned પૂર્ણાંક)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  પાછા FFI_OK;
}
