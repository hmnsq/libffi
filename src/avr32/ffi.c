/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2011  Anthony Green
           Copyright (c) 2009  Bradley Smith <brad@brad-smith.co.uk>

   AVR32 Foreign Function Interface

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
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <asm/unistd.h>

/* #define DEBUG */

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ (*)(ಅಕ್ಷರ *, extended_cif *), extended_cif *,
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ,
    ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_SYSV (ffi_ಆವರಣ *);

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pass_struct_on_ರಾಶಿ(ffi_type *type)
{
    ಆದರೆ(type->type != FFI_TYPE_STRUCT)
        ಹಿಂತಿರುಗಿ 0;

    ಆದರೆ(type->alignment < type->ಗಾತ್ರ_2 &&
        !(type->ಗಾತ್ರ_2 == 4 || type->ಗಾತ್ರ_2 == 8) &&
        !(type->ಗಾತ್ರ_2 == 8 && type->alignment >= 4))
        ಹಿಂತಿರುಗಿ 1;

    ಆದರೆ(type->ಗಾತ್ರ_2 == 3 || type->ಗಾತ್ರ_2 == 5 || type->ಗಾತ್ರ_2 == 6 ||
        type->ಗಾತ್ರ_2 == 7)
        ಹಿಂತಿರುಗಿ 1;

    ಹಿಂತಿರುಗಿ 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

ಶೂನ್ಯ ffi_prep_ವಾದಗಳು(ಅಕ್ಷರ *ರಾಶಿ, extended_cif *ecif)
{
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
    ಶೂನ್ಯ **p_argv;
    ffi_type **p_arg;
    ಅಕ್ಷರ *reg_ತಳ = ರಾಶಿ;
    ಅಕ್ಷರ *ರಾಶಿ_ತಳ = ರಾಶಿ + 20;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ರಾಶಿ_ಸ್ಥಾನಾಂತರ = 0;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ reg_ಮುಖವಾಡ = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    ಆದರೆ(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(ಶೂನ್ಯ**)reg_ತಳ = ecif->rvalue;
        reg_ಮುಖವಾಡ |= 1;
    }

    ಗಾಗಿ(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->ಗಾತ್ರ_2;
        ಪೂರ್ಣಾಂಕ alignment = (*p_arg)->alignment;
        ಪೂರ್ಣಾಂಕ type = (*p_arg)->type;
        ಅಕ್ಷರ *addr = 0;

        ಆದರೆ(z % 4 != 0)
            z += (4 - z % 4);

        ಆದರೆ(reg_ಮುಖವಾಡ != 0x1f)
        {
            ಆದರೆ(pass_struct_on_ರಾಶಿ(*p_arg))
            {
                addr = ರಾಶಿ_ತಳ + ರಾಶಿ_ಸ್ಥಾನಾಂತರ;
                ರಾಶಿ_ಸ್ಥಾನಾಂತರ += z;
            }
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(z == ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
            {
                ಅಕ್ಷರ ಸೂಚ್ಯಂಕ = 0;

                ಆಗ((reg_ಮುಖವಾಡ >> ಸೂಚ್ಯಂಕ) & 1)
                    ಸೂಚ್ಯಂಕ++;

                addr = reg_ತಳ + (ಸೂಚ್ಯಂಕ * 4);
                reg_ಮುಖವಾಡ |= (1 << ಸೂಚ್ಯಂಕ);
            }
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(z == 2 * ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
            {
                ಆದರೆ(!((reg_ಮುಖವಾಡ >> 1) & 1))
                {
                    addr = reg_ತಳ + 4;
                    reg_ಮುಖವಾಡ |= (3 << 1);
                }
                ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(!((reg_ಮುಖವಾಡ >> 3) & 1))
                {
                    addr = reg_ತಳ + 12;
                    reg_ಮುಖವಾಡ |= (3 << 3);
                }
            }
        }

        ಆದರೆ(!addr)
        {
            addr = ರಾಶಿ_ತಳ + ರಾಶಿ_ಸ್ಥಾನಾಂತರ;
            ರಾಶಿ_ಸ್ಥಾನಾಂತರ += z;
        }

        ಆದರೆ(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == ಖಾಲಿ)
            type = (*p_arg)->elements[0]->type;

        ಆಯ್ಕೆ(type)
        {
        ಸ್ಥಿತಿ FFI_TYPE_UINT8:
            *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *)addr = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT8 *)(*p_argv);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ FFI_TYPE_SINT8:
            *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *)addr = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT8 *)(*p_argv);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ FFI_TYPE_UINT16:
            *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *)addr = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT16 *)(*p_argv);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ FFI_TYPE_SINT16:
            *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *)addr = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT16 *)(*p_argv);
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            ಸ್ಮರಣೆ_ನಕಲು(addr, *p_argv, z);
        }

        p_argv++;
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
    /* Debugging */
    ಗಾಗಿ(i = 0; i < 5; i++)
    {
        ಆದರೆ((reg_ಮುಖವಾಡ & (1 << i)) == 0)
            ಬರೆ("r%d: (unused)\n", 12 - i);
        ಇಲ್ಲದಿದ್ದರೆ
            ಬರೆ("r%d: 0x%08x\n", 12 - i, ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*)reg_ತಳ)[i]);
    }

    ಗಾಗಿ(i = 0; i < ರಾಶಿ_ಸ್ಥಾನಾಂತರ / 4; i++)
    {
        ಬರೆ("sp+%d: 0x%08x\n", i*4, ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*)ರಾಶಿ_ತಳ)[i]);
    }
#ಆದರೆ_ಅಂತ್ಯ
}

/* Perform machine dependent cif processing */
ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep(ffi_cif *cif)
{
    /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     * everywhere, but it is on some platforms, and it doesn't harm
     * anything when it isn't needed. */
    cif->ಬೈಟುಗಳು = (cif->ಬೈಟುಗಳು + 7) & ~7;

    /* Flag to indicate that he return value is in fact a struct */
    cif->rstruct_flag = 0;

    /* Set the return type flag */
    ಆಯ್ಕೆ(cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
        cif->flags = (ಚಿಹ್ನೆರಹಿತ)FFI_TYPE_UINT8;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
        cif->flags = (ಚಿಹ್ನೆರಹಿತ)FFI_TYPE_UINT16;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
        cif->flags = (ಚಿಹ್ನೆರಹಿತ)FFI_TYPE_UINT32;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
        cif->flags = (ಚಿಹ್ನೆರಹಿತ)FFI_TYPE_UINT64;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        ಆದರೆ(!pass_struct_on_ರಾಶಿ(cif->rtype))
        {
            ಆದರೆ(cif->rtype->ಗಾತ್ರ_2 <= 1)
                cif->flags = (ಚಿಹ್ನೆರಹಿತ)FFI_TYPE_UINT8;
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(cif->rtype->ಗಾತ್ರ_2 <= 2)
                cif->flags = (ಚಿಹ್ನೆರಹಿತ)FFI_TYPE_UINT16;
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(cif->rtype->ಗಾತ್ರ_2 <= 4)
                cif->flags = (ಚಿಹ್ನೆರಹಿತ)FFI_TYPE_UINT32;
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(cif->rtype->ಗಾತ್ರ_2 <= 8)
                cif->flags = (ಚಿಹ್ನೆರಹಿತ)FFI_TYPE_UINT64;
            ಇಲ್ಲದಿದ್ದರೆ
                cif->flags = (ಚಿಹ್ನೆರಹಿತ)cif->rtype->type;
        }
        ಇಲ್ಲದಿದ್ದರೆ
            cif->flags = (ಚಿಹ್ನೆರಹಿತ)cif->rtype->type;
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
        cif->flags = (ಚಿಹ್ನೆರಹಿತ)cif->rtype->type;
        ನಿಲ್ಲಿಸು;
    }

    ಹಿಂತಿರುಗಿ FFI_OK;
}

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
    extended_cif ecif;

    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    ಗಾಗಿ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        ಗಾತ್ರ_2 += (*p_arg)->ಗಾತ್ರ_2 + (4 - (*p_arg)->ಗಾತ್ರ_2 % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    ಆದರೆ((rvalue == ಖಾಲಿ) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->ಗಾತ್ರ_2);
    ಇಲ್ಲದಿದ್ದರೆ
        ecif.rvalue = rvalue;

    ಆಯ್ಕೆ(cif->abi)
    {
    ಸ್ಥಿತಿ FFI_SYSV:
        ffi_call_SYSV(ffi_prep_ವಾದಗಳು, &ecif, ಗಾತ್ರ_2, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
        FFI_ASSERT(0);
        ನಿಲ್ಲಿಸು;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ ffi_prep_incoming_ವಾದಗಳು_SYSV(ಅಕ್ಷರ *ರಾಶಿ, ಶೂನ್ಯ **rvalue,
    ಶೂನ್ಯ **avalue, ffi_cif *cif)
{
    ನೋಂದಣಿ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, reg_ಮುಖವಾಡ = 0;
    ನೋಂದಣಿ ಶೂನ್ಯ **p_argv;
    ನೋಂದಣಿ ffi_type **p_arg;
    ನೋಂದಣಿ ಅಕ್ಷರ *reg_ತಳ = ರಾಶಿ;
    ನೋಂದಣಿ ಅಕ್ಷರ *ರಾಶಿ_ತಳ = ರಾಶಿ + 20;
    ನೋಂದಣಿ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ರಾಶಿ_ಸ್ಥಾನಾಂತರ = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
    /* Debugging */
    ಗಾಗಿ(i = 0; i < cif->nargs + 7; i++)
    {
        ಬರೆ("sp+%d: 0x%08x\n", i*4, ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*)ರಾಶಿ)[i]);
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* If cif->flags is struct then we know it's not passed in registers */
    ಆದರೆ(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(ಶೂನ್ಯ **)reg_ತಳ;
        reg_ಮುಖವಾಡ |= 1;
    }

    p_argv = avalue;

    ಗಾಗಿ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->ಗಾತ್ರ_2;
        ಪೂರ್ಣಾಂಕ alignment = (*p_arg)->alignment;

        *p_argv = 0;

        ಆದರೆ(z % 4 != 0)
            z += (4 - z % 4);

        ಆದರೆ(reg_ಮುಖವಾಡ != 0x1f)
        {
            ಆದರೆ(pass_struct_on_ರಾಶಿ(*p_arg))
            {
                *p_argv = (ಶೂನ್ಯ*)ರಾಶಿ_ತಳ + ರಾಶಿ_ಸ್ಥಾನಾಂತರ;
                ರಾಶಿ_ಸ್ಥಾನಾಂತರ += z;
            }
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(z <= ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
            {
                ಅಕ್ಷರ ಸೂಚ್ಯಂಕ = 0;

                ಆಗ((reg_ಮುಖವಾಡ >> ಸೂಚ್ಯಂಕ) & 1)
                    ಸೂಚ್ಯಂಕ++;

                *p_argv = (ಶೂನ್ಯ*)reg_ತಳ + (ಸೂಚ್ಯಂಕ * 4);
                reg_ಮುಖವಾಡ |= (1 << ಸೂಚ್ಯಂಕ);
            }
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(z == 2 * ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))
            {
                ಆದರೆ(!((reg_ಮುಖವಾಡ >> 1) & 1))
                {
                    *p_argv = (ಶೂನ್ಯ*)reg_ತಳ + 4;
                    reg_ಮುಖವಾಡ |= (3 << 1);
                }
                ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(!((reg_ಮುಖವಾಡ >> 3) & 1))
                {
                    *p_argv = (ಶೂನ್ಯ*)reg_ತಳ + 12;
                    reg_ಮುಖವಾಡ |= (3 << 3);
                }
            }
        }

        ಆದರೆ(!*p_argv)
        {
            *p_argv = (ಶೂನ್ಯ*)ರಾಶಿ_ತಳ + ರಾಶಿ_ಸ್ಥಾನಾಂತರ;
            ರಾಶಿ_ಸ್ಥಾನಾಂತರ += z;
        }

        ಆದರೆ((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == ಖಾಲಿ)
        {
            ಆದರೆ(alignment == 1)
                **(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ**)p_argv <<= 24;
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(alignment == 2)
                **(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ**)p_argv <<= 16;
        }

        p_argv++;
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
    /* Debugging */
    ಗಾಗಿ(i = 0; i < cif->nargs; i++)
    {
        ಬರೆ("sp+%d: 0x%08x\n", i*4, *(((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ**)avalue)[i]));
    }
#ಆದರೆ_ಅಂತ್ಯ
}

/* This function is jumped to by the trampoline */

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ffi_ಆವರಣ_SYSV_inner(ffi_ಆವರಣ *ಆವರಣ, ಶೂನ್ಯ **respp,
    ಶೂನ್ಯ *ವಾದಗಳು)
{
    ffi_cif *cif;
    ಶೂನ್ಯ **arg_area;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, ಗಾತ್ರ_2 = 0;
    ffi_type **p_arg;

    cif = ಆವರಣ->cif;

    ಗಾಗಿ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        ಗಾತ್ರ_2 += (*p_arg)->ಗಾತ್ರ_2 + (4 - (*p_arg)->ಗಾತ್ರ_2 % 4);

    arg_area = (ಶೂನ್ಯ **)alloca(ಗಾತ್ರ_2);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_ವಾದಗಳು_SYSV(ವಾದಗಳು, respp, arg_area, cif);

    (ಆವರಣ->fun)(cif, *respp, arg_area, ಆವರಣ->user_ದತ್ತಾಂಶ);

    ಹಿಂತಿರುಗಿ cif->flags;
}

ffi_ಸ್ಥಿತಿ ffi_prep_ಆವರಣ_loc(ffi_ಆವರಣ* ಆವರಣ, ffi_cif* cif,
    ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*), ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
    ಶೂನ್ಯ *codeloc)
{
    ಆದರೆ (cif->abi != FFI_SYSV)
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *__tramp = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)(&ಆವರಣ->tramp[0]);
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ  __fun = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(&ffi_ಆವರಣ_SYSV);
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ  __ctx = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(codeloc);
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ  __rstruct_flag = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(cif->rstruct_flag);
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ  __inner = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(&ffi_ಆವರಣ_SYSV_inner);
    *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*) &__tramp[20] = __ctx;
    *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*) &__tramp[24] = __rstruct_flag;
    *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*) &__tramp[28] = __inner;
    *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    ಆವರಣ->cif = cif;
    ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
    ಆವರಣ->fun  = fun;

    ಹಿಂತಿರುಗಿ FFI_OK;
}

