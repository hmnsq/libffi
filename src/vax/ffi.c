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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#define CIF_FLAGS_CHAR		1	/* for struct only */
#define CIF_FLAGS_SHORT		2	/* for struct only */
#define CIF_FLAGS_INT		4
#define CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

ଶୂନ୍ୟ ffi_call_elfbsd (extended_cif *, unsigned, unsigned, ଶୂନ୍ୟ *,
		       ଶୂନ୍ୟ (*) ());
ଶୂନ୍ୟ *ffi_prep_args (extended_cif *ecif, ଶୂନ୍ୟ *ଗଦା);

ଶୂନ୍ୟ *
ffi_prep_args (extended_cif *ecif, ଶୂନ୍ୟ *ଗଦା)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଶୂନ୍ୟ **p_argv;
  ଅକ୍ଷର *argp;
  ffi_type **p_arg;
  ଶୂନ୍ୟ *struct_ମୂଲ୍ୟ_ptr;

  argp = ଗଦା;

  ଯଦି (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_ମୂଲ୍ୟ_ptr = ecif->rvalue;
  ଅନ୍ୟଥା
    struct_ମୂଲ୍ୟ_ptr = ଖାଲି;

  p_argv = ecif->avalue;

  ପାଇଁ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      ଯଦି (z < ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  ବାଛ ((*p_arg)->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(SINT8 *) *p_argv;
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_UINT8:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(UINT8 *) *p_argv;
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_SINT16:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(SINT16 *) *p_argv;
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_UINT16:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *(UINT16 *) *p_argv;
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      ଭାଙ୍ଗ;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT (0);
	    }
	  z = ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	}
      ଅନ୍ୟଥା
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ଯଦି ((ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) & z)
	    z = FFI_ALIGN(z, ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
	}

      p_argv++;
      argp += z;
    }

  ଫେରନ୍ତୁ struct_ମୂଲ୍ୟ_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      cif->flags = 0;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
      ଯଦି (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  ଭାଙ୍ଗ;
	}

      ଯଦି (cif->rtype->size == ଆକାର (ଅକ୍ଷର))
	cif->flags = CIF_FLAGS_CHAR;
      ଅନ୍ୟଥା ଯଦି (cif->rtype->size == ଆକାର (short))
	cif->flags = CIF_FLAGS_SHORT;
      ଅନ୍ୟଥା ଯଦି (cif->rtype->size == ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	cif->flags = CIF_FLAGS_INT;
      ଅନ୍ୟଥା ଯଦି (cif->rtype->size == 2 * ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	cif->flags = CIF_FLAGS_DINT;
      ଅନ୍ୟଥା
	cif->flags = 0;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଯଦି (cif->rtype->size <= ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	cif->flags = CIF_FLAGS_INT;
      ଅନ୍ୟଥା
	cif->flags = CIF_FLAGS_DINT;
      ଭାଙ୍ଗ;
    }

  ଫେରନ୍ତୁ FFI_OK;
}

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ଯଦି (rvalue == ଖାଲି
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->size);
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT (0);
      ଭାଙ୍ଗ;
    }
}

/*
 * Closure API
 */

ଶୂନ୍ୟ ffi_closure_elfbsd (ଶୂନ୍ୟ);
ଶୂନ୍ୟ ffi_closure_struct_elfbsd (ଶୂନ୍ୟ);
unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_closure_elfbsd_inner (ffi_closure *, ଶୂନ୍ୟ *, ଅକ୍ଷର *);

ସ୍ଥିର ଶୂନ୍ୟ
ffi_prep_closure_elfbsd (ffi_cif *cif, ଶୂନ୍ୟ **avalue, ଅକ୍ଷର *stackp)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଶୂନ୍ୟ **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  ପାଇଁ (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      *p_argv = stackp;

      /* Align if necessary */
      ଯଦି ((ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) & z)
	z = FFI_ALIGN(z, ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା));

      p_argv++;
      stackp += z;
    }
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_elfbsd_inner (ffi_closure *closure, ଶୂନ୍ୟ *resp, ଅକ୍ଷର *ଗଦା)
{
  ffi_cif *cif;
  ଶୂନ୍ୟ **arg_area;

  cif = closure->cif;
  arg_area = (ଶୂନ୍ୟ **) alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));

  ffi_prep_closure_elfbsd (cif, arg_area, ଗଦା);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  ଫେରନ୍ତୁ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
		      ଶୂନ୍ୟ *user_data, ଶୂନ୍ୟ *codeloc)
{
  ଅକ୍ଷର *tramp = (ଅକ୍ଷର *) codeloc;
  ଶୂନ୍ୟ *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(unsigned short *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)(tramp + 4) = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) closure;
  tramp[8] = 0x50;

  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  ଅନ୍ୟଥା
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)(tramp + 11) = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fn + 2 -
				  (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}
