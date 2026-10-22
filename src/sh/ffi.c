/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2002-2008, 2012 Kaz Kojima
           Copyright (c) 2008 Red Hat, Inc.
   
   SuperH Foreign Function Interface 

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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ವ್ಯಾಖ್ಯಾನಿಸು NGREGARG 4
#ಆದರೆ defined(__SH4__)
#ವ್ಯಾಖ್ಯಾನಿಸು NFREGARG 8
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__HITACHI__)
#ವ್ಯಾಖ್ಯಾನಿಸು STRUCT_ಮೌಲ್ಯ_ವಿಳಾಸ_WITH_ARG 1
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು STRUCT_ಮೌಲ್ಯ_ವಿಳಾಸ_WITH_ARG 0
#ಆದರೆ_ಅಂತ್ಯ

/* If the structure has essentially an unique element, return its type.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
simple_type (ffi_type *arg)
{
  ಆದರೆ (arg->type != FFI_TYPE_STRUCT)
    ಹಿಂತಿರುಗಿ arg->type;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg->elements[1])
    ಹಿಂತಿರುಗಿ FFI_TYPE_STRUCT;

  ಹಿಂತಿರುಗಿ simple_type (arg->elements[0]);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
return_type (ffi_type *arg)
{
  ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ type;

  ಆದರೆ (arg->type != FFI_TYPE_STRUCT)
    ಹಿಂತಿರುಗಿ arg->type;

  type = simple_type (arg->elements[0]);
  ಆದರೆ (! arg->elements[1])
    {
      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  ಹಿಂತಿರುಗಿ FFI_TYPE_INT;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  ಹಿಂತಿರುಗಿ type;
	}
    }

  /* gcc uses r0/r1 pair for some kind of structures.  */
  ಆದರೆ (arg->ಗಾತ್ರ_2 <= 2 * ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
    {
      ಪೂರ್ಣಾಂಕ i = 0;
      ffi_type *e;

      ಆಗ ((e = arg->elements[i++]))
	{
	  type = simple_type (e);
	  ಆಯ್ಕೆ (type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	    ಸ್ಥಿತಿ FFI_TYPE_INT:
	    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	      ಹಿಂತಿರುಗಿ FFI_TYPE_UINT64;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      ನಿಲ್ಲಿಸು;
	    }
	}
    }

  ಹಿಂತಿರುಗಿ FFI_TYPE_STRUCT;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಶೂನ್ಯ ffi_prep_ವಾದಗಳು(ಅಕ್ಷರ *ರಾಶಿ, extended_cif *ecif)
{
  ನೋಂದಣಿ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ನೋಂದಣಿ ಪೂರ್ಣಾಂಕ tmp;
  ನೋಂದಣಿ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ avn;
  ನೋಂದಣಿ ಶೂನ್ಯ **p_argv;
  ನೋಂದಣಿ ಅಕ್ಷರ *argp;
  ನೋಂದಣಿ ffi_type **p_arg;
  ಪೂರ್ಣಾಂಕ greg, ireg;
#ಆದರೆ defined(__SH4__)
  ಪೂರ್ಣಾಂಕ freg = 0;
#ಆದರೆ_ಅಂತ್ಯ

  tmp = 0;
  argp = ರಾಶಿ;

  ಆದರೆ (return_type (ecif->cif->rtype) == FFI_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) argp = ecif->rvalue;
      argp += 4;
      ireg = STRUCT_ಮೌಲ್ಯ_ವಿಳಾಸ_WITH_ARG ? 1 : 0;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ireg = 0;

  /* Set arguments for registers.  */
  greg = ireg;
  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ಗಾಗಿ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->ಗಾತ್ರ_2;
      ಆದರೆ (z < ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
	{
	  ಆದರೆ (greg++ >= NGREGARG)
	    ಮುಂದುವರಿ;

	  z = ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ);
	  ಆಯ್ಕೆ ((*p_arg)->type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	      *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT8 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT8 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	      *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT16 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT16 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT32 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z == ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
	{
#ಆದರೆ defined(__SH4__)
	  ಆದರೆ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ಆದರೆ (freg++ >= NFREGARG)
		ಮುಂದುವರಿ;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	    {
	      ಆದರೆ (greg++ >= NGREGARG)
		ಮುಂದುವರಿ;
	    }
	  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ಆದರೆ defined(__SH4__)
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ಆದರೆ (freg + 1 >= NFREGARG)
	    ಮುಂದುವರಿ;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
	  argp += z;
	}
#ಆದರೆ_ಅಂತ್ಯ
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಪೂರ್ಣಾಂಕ n = (z + ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - 1) / ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
#ಆದರೆ defined(__SH4__)
	  ಆದರೆ (greg + n - 1 >= NGREGARG)
	    ಮುಂದುವರಿ;
#ಇಲ್ಲದಿದ್ದರೆ
	  ಆದರೆ (greg >= NGREGARG)
	    ಮುಂದುವರಿ;
#ಆದರೆ_ಅಂತ್ಯ
	  greg += n;
	  ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
	  argp += n * ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	}
    }

  /* Set arguments on stack.  */
  greg = ireg;
#ಆದರೆ defined(__SH4__)
  freg = 0;
#ಆದರೆ_ಅಂತ್ಯ
  p_argv = ecif->avalue;

  ಗಾಗಿ (i = 0, p_arg = ecif->cif->arg_types; i < avn; i++, p_arg++, p_argv++)
    {
      size_t z;

      z = (*p_arg)->ಗಾತ್ರ_2;
      ಆದರೆ (z < ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
	{
	  ಆದರೆ (greg++ < NGREGARG)
	    ಮುಂದುವರಿ;

	  z = ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ);
	  ಆಯ್ಕೆ ((*p_arg)->type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	      *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT8 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT8 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	      *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT16 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT16 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT32 *)(* p_argv);
	      ನಿಲ್ಲಿಸು;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      FFI_ASSERT(0);
	    }
	  argp += z;
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z == ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
	{
#ಆದರೆ defined(__SH4__)
	  ಆದರೆ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ಆದರೆ (freg++ < NFREGARG)
		ಮುಂದುವರಿ;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	    {
	      ಆದರೆ (greg++ < NGREGARG)
		ಮುಂದುವರಿ;
	    }
	  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT32 *)(* p_argv);
	  argp += z;
	}
#ಆದರೆ defined(__SH4__)
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ಆದರೆ (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ಮುಂದುವರಿ;
	    }
	  ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
	  argp += z;
	}
#ಆದರೆ_ಅಂತ್ಯ
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಪೂರ್ಣಾಂಕ n = (z + ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - 1) / ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	  ಆದರೆ (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ಮುಂದುವರಿ;
	    }
#ಆದರೆ (! defined(__SH4__))
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (greg < NGREGARG)
	    {
	      greg = NGREGARG;
	      ಮುಂದುವರಿ;
	    }
#ಆದರೆ_ಅಂತ್ಯ
	  ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
	  argp += n * ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	}
    }

  ಹಿಂತಿರುಗಿ;
}

/* Perform machine dependent cif processing */
ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep(ffi_cif *cif)
{
  ಪೂರ್ಣಾಂಕ i, j;
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, type;
  ಪೂರ್ಣಾಂಕ n, m;
  ಪೂರ್ಣಾಂಕ greg;
#ಆದರೆ defined(__SH4__)
  ಪೂರ್ಣಾಂಕ freg = 0;
#ಆದರೆ_ಅಂತ್ಯ

  cif->flags = 0;

  greg = ((return_type (cif->rtype) == FFI_TYPE_STRUCT) &&
	  STRUCT_ಮೌಲ್ಯ_ವಿಳಾಸ_WITH_ARG) ? 1 : 0;

#ಆದರೆ defined(__SH4__)
  ಗಾಗಿ (i = j = 0; i < cif->nargs && j < 12; i++)
    {
      type = (cif->arg_types)[i]->type;
      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  ಆದರೆ (freg >= NFREGARG)
	    ಮುಂದುವರಿ;
	  freg++;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಆದರೆ ((freg + 1) >= NFREGARG)
	    ಮುಂದುವರಿ;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  cif->flags += ((cif->arg_types)[i]->type) << (2 * j);
	  j++;
	  ನಿಲ್ಲಿಸು;
	      
	ಪೂರ್ವನಿಯೋಜಿತ:
	  ಗಾತ್ರ_2 = (cif->arg_types)[i]->ಗಾತ್ರ_2;
	  n = (ಗಾತ್ರ_2 + ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - 1) / ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	  ಆದರೆ (greg + n - 1 >= NGREGARG)
		ಮುಂದುವರಿ;
	  greg += n;
	  ಗಾಗಿ (m = 0; m < n; m++)
	    cif->flags += FFI_TYPE_INT << (2 * j++);
	  ನಿಲ್ಲಿಸು;
	}
    }
#ಇಲ್ಲದಿದ್ದರೆ
  ಗಾಗಿ (i = j = 0; i < cif->nargs && j < 4; i++)
    {
      ಗಾತ್ರ_2 = (cif->arg_types)[i]->ಗಾತ್ರ_2;
      n = (ಗಾತ್ರ_2 + ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - 1) / ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
      ಆದರೆ (greg >= NGREGARG)
	ಮುಂದುವರಿ;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (greg + n - 1 >= NGREGARG)
	n = NGREGARG - greg;
      greg += n;
      ಗಾಗಿ (m = 0; m < n; m++)
        cif->flags += FFI_TYPE_INT << (2 * j++);
    }
#ಆದರೆ_ಅಂತ್ಯ

  /* Set the return type flag */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      cif->flags += (ಚಿಹ್ನೆರಹಿತ) (return_type (cif->rtype)) << 24;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_VOID:
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      cif->flags += (ಚಿಹ್ನೆರಹಿತ) cif->rtype->type << 24;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      cif->flags += FFI_TYPE_INT << 24;
      ನಿಲ್ಲಿಸು;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ (*)(ಅಕ್ಷರ *, extended_cif *), extended_cif *,
			  ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ *, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;
  UINT64 trvalue;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ecif.rvalue = &trvalue;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((rvalue == ಖಾಲಿ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ಗಾತ್ರ_2);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;

  ಆಯ್ಕೆ (cif->abi) 
    {
    ಸ್ಥಿತಿ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_ವಾದಗಳು, &ecif, cif->ಬೈಟುಗಳು, cif->flags, ecif.rvalue,
		    fn);
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }

  ಆದರೆ (rvalue
      && cif->rtype->type == FFI_TYPE_STRUCT
      && return_type (cif->rtype) != FFI_TYPE_STRUCT)
    ಸ್ಮರಣೆ_ನಕಲು (rvalue, &trvalue, cif->rtype->ಗಾತ್ರ_2);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_SYSV (ಶೂನ್ಯ);
#ಆದರೆ defined(__SH4__)
ಬಾಹ್ಯ ಶೂನ್ಯ __ic_invalidate (ಶೂನ್ಯ *ಸಾಲು);
#ಆದರೆ_ಅಂತ್ಯ

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ insn;

  ಆದರೆ (cif->abi != FFI_SYSV)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &ಆವರಣ->tramp[0];
  /* Set T bit if the function returns a struct pointed with R2.  */
  insn = (return_type (cif->rtype) == FFI_TYPE_STRUCT
	  ? 0x0018 /* sett */
	  : 0x0008 /* clrt */);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN__
  tramp[0] = 0xd301d102;
  tramp[1] = 0x0000412b | (insn << 16);
#ಇಲ್ಲದಿದ್ದರೆ
  tramp[0] = 0xd102d301;
  tramp[1] = 0x412b0000 | insn;
#ಆದರೆ_ಅಂತ್ಯ
  *(ಶೂನ್ಯ **) &tramp[2] = (ಶೂನ್ಯ *)codeloc;          /* ctx */
  *(ಶೂನ್ಯ **) &tramp[3] = (ಶೂನ್ಯ *)ffi_ಆವರಣ_SYSV; /* funaddr */

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

#ಆದರೆ defined(__SH4__)
  /* Flush the icache.  */
  __ic_invalidate(codeloc);
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on 
 * entry, r3 holds the address of the closure.
 * After storing the registers that could possibly contain
 * parameters to be passed into the stack frame and setting
 * up space for a return value, ffi_closure_SYSV invokes the 
 * following helper function to do most of the work.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN__
#ವ್ಯಾಖ್ಯಾನಿಸು OFS_INT8	0
#ವ್ಯಾಖ್ಯಾನಿಸು OFS_INT16	0
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು OFS_INT8	3
#ವ್ಯಾಖ್ಯಾನಿಸು OFS_INT16	2
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_helper_SYSV (ffi_ಆವರಣ *ಆವರಣ, ಶೂನ್ಯ *rvalue, 
			 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *pgr, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *pfr, 
			 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *pst)
{
  ಶೂನ್ಯ **avalue;
  ffi_type **p_arg;
  ಪೂರ್ಣಾಂಕ i, avn;
  ಪೂರ್ಣಾಂಕ ireg, greg = 0;
#ಆದರೆ defined(__SH4__)
  ಪೂರ್ಣಾಂಕ freg = 0;
#ಆದರೆ_ಅಂತ್ಯ
  ffi_cif *cif; 

  cif = ಆವರಣ->cif;
  avalue = alloca(cif->nargs * ಗಾತ್ರ(ಶೂನ್ಯ *));

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.  */
  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT && STRUCT_ಮೌಲ್ಯ_ವಿಳಾಸ_WITH_ARG)
    {
      rvalue = (ಶೂನ್ಯ *) *pgr++;
      ireg = 1;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ireg = 0;

  cif = ಆವರಣ->cif;
  greg = ireg;
  avn = cif->nargs;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಗಾಗಿ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ಗಾತ್ರ_2;
      ಆದರೆ (z < ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
	{
	  ಆದರೆ (greg++ >= NGREGARG)
	    ಮುಂದುವರಿ;

	  z = ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ);
	  ಆಯ್ಕೆ ((*p_arg)->type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	      avalue[i] = (((ಅಕ್ಷರ *)pgr) + OFS_INT8);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	      avalue[i] = (((ಅಕ್ಷರ *)pgr) + OFS_INT16);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	      avalue[i] = pgr;
	      ನಿಲ್ಲಿಸು;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      FFI_ASSERT(0);
	    }
	  pgr++;
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z == ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
	{
#ಆದರೆ defined(__SH4__)
	  ಆದರೆ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ಆದರೆ (freg++ >= NFREGARG)
		ಮುಂದುವರಿ;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	    {
	      ಆದರೆ (greg++ >= NGREGARG)
		ಮುಂದುವರಿ;
	      avalue[i] = pgr;
	      pgr++;
	    }
	}
#ಆದರೆ defined(__SH4__)
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ಆದರೆ (freg + 1 >= NFREGARG)
	    ಮುಂದುವರಿ;
	  ಆದರೆ (freg & 1)
	    pfr++;
	  freg = (freg + 1) & ~1;
	  freg += 2;
	  avalue[i] = pfr;
	  pfr += 2;
	}
#ಆದರೆ_ಅಂತ್ಯ
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಪೂರ್ಣಾಂಕ n = (z + ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - 1) / ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
#ಆದರೆ defined(__SH4__)
	  ಆದರೆ (greg + n - 1 >= NGREGARG)
	    ಮುಂದುವರಿ;
#ಇಲ್ಲದಿದ್ದರೆ
	  ಆದರೆ (greg >= NGREGARG)
	    ಮುಂದುವರಿ;
#ಆದರೆ_ಅಂತ್ಯ
	  greg += n;
	  avalue[i] = pgr;
	  pgr += n;
	}
    }

  greg = ireg;
#ಆದರೆ defined(__SH4__)
  freg = 0;
#ಆದರೆ_ಅಂತ್ಯ

  ಗಾಗಿ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ಗಾತ್ರ_2;
      ಆದರೆ (z < ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
	{
	  ಆದರೆ (greg++ < NGREGARG)
	    ಮುಂದುವರಿ;

	  z = ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ);
	  ಆಯ್ಕೆ ((*p_arg)->type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	      avalue[i] = (((ಅಕ್ಷರ *)pst) + OFS_INT8);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	      avalue[i] = (((ಅಕ್ಷರ *)pst) + OFS_INT16);
	      ನಿಲ್ಲಿಸು;
  
	    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	      avalue[i] = pst;
	      ನಿಲ್ಲಿಸು;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      FFI_ASSERT(0);
	    }
	  pst++;
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z == ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
	{
#ಆದರೆ defined(__SH4__)
	  ಆದರೆ ((*p_arg)->type == FFI_TYPE_FLOAT)
	    {
	      ಆದರೆ (freg++ < NFREGARG)
		ಮುಂದುವರಿ;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	    {
	      ಆದರೆ (greg++ < NGREGARG)
		ಮುಂದುವರಿ;
	    }
	  avalue[i] = pst;
	  pst++;
	}
#ಆದರೆ defined(__SH4__)
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((*p_arg)->type == FFI_TYPE_DOUBLE)
	{
	  ಆದರೆ (freg + 1 < NFREGARG)
	    {
	      freg = (freg + 1) & ~1;
	      freg += 2;
	      ಮುಂದುವರಿ;
	    }
	  avalue[i] = pst;
	  pst += 2;
	}
#ಆದರೆ_ಅಂತ್ಯ
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಪೂರ್ಣಾಂಕ n = (z + ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - 1) / ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	  ಆದರೆ (greg + n - 1 < NGREGARG)
	    {
	      greg += n;
	      ಮುಂದುವರಿ;
	    }
#ಆದರೆ (! defined(__SH4__))
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (greg < NGREGARG)
	    {
	      greg += n;
	      pst += greg - NGREGARG;
	      ಮುಂದುವರಿ;
	    }
#ಆದರೆ_ಅಂತ್ಯ
	  avalue[i] = pst;
	  pst += n;
	}
    }

  (ಆವರಣ->fun) (cif, rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);

  /* Tell ffi_closure_SYSV how to perform return type promotions.  */
  ಹಿಂತಿರುಗಿ return_type (cif->rtype);
}
