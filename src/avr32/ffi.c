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

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ (*)(ವರ್ಣ *, extended_cif *), extended_cif *,
    unsigned ಪೂರ್ಣಾಙ್ಕ, unsigned ಪೂರ್ಣಾಙ್ಕ, unsigned ಪೂರ್ಣಾಙ್ಕ*, unsigned ಪೂರ್ಣಾಙ್ಕ,
    ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_SYSV (ffi_closure *);

unsigned ಪೂರ್ಣಾಙ್ಕ pass_struct_on_ರಾಶಿ(ffi_type *type)
{
    ಯದಿ(type->type != FFI_TYPE_STRUCT)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಯದಿ(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        ಪ್ರತ್ಯಾಗಮ 1;

    ಯದಿ(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        ಪ್ರತ್ಯಾಗಮ 1;

    ಪ್ರತ್ಯಾಗಮ 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

ಶೂನ್ಯ ffi_prep_args(ವರ್ಣ *ರಾಶಿ, extended_cif *ecif)
{
    unsigned ಪೂರ್ಣಾಙ್ಕ i;
    ಶೂನ್ಯ **p_argv;
    ffi_type **p_arg;
    ವರ್ಣ *reg_base = ರಾಶಿ;
    ವರ್ಣ *ರಾಶಿ_base = ರಾಶಿ + 20;
    unsigned ಪೂರ್ಣಾಙ್ಕ ರಾಶಿ_offset = 0;
    unsigned ಪೂರ್ಣಾಙ್ಕ reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    ಯದಿ(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(ಶೂನ್ಯ**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    ಪ್ರತಿ(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        ಪೂರ್ಣಾಙ್ಕ alignment = (*p_arg)->alignment;
        ಪೂರ್ಣಾಙ್ಕ type = (*p_arg)->type;
        ವರ್ಣ *addr = 0;

        ಯದಿ(z % 4 != 0)
            z += (4 - z % 4);

        ಯದಿ(reg_mask != 0x1f)
        {
            ಯದಿ(pass_struct_on_ರಾಶಿ(*p_arg))
            {
                addr = ರಾಶಿ_base + ರಾಶಿ_offset;
                ರಾಶಿ_offset += z;
            }
            ಅನ್ಯಥಾ ಯದಿ(z == ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
            {
                ವರ್ಣ index = 0;

                ಯಾವತ್((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            ಅನ್ಯಥಾ ಯದಿ(z == 2 * ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
            {
                ಯದಿ(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                ಅನ್ಯಥಾ ಯದಿ(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        ಯದಿ(!addr)
        {
            addr = ರಾಶಿ_base + ರಾಶಿ_offset;
            ರಾಶಿ_offset += z;
        }

        ಯದಿ(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == ರಿಕ್ತ)
            type = (*p_arg)->elements[0]->type;

        ವಿಕಲ್ಪ(type)
        {
        ಪಕ್ಷ FFI_TYPE_UINT8:
            *(unsigned ಪೂರ್ಣಾಙ್ಕ *)addr = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT8 *)(*p_argv);
            ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_SINT8:
            *(signed ಪೂರ್ಣಾಙ್ಕ *)addr = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT8 *)(*p_argv);
            ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_UINT16:
            *(unsigned ಪೂರ್ಣಾಙ್ಕ *)addr = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT16 *)(*p_argv);
            ಭಙ್ಗ;
        ಪಕ್ಷ FFI_TYPE_SINT16:
            *(signed ಪೂರ್ಣಾಙ್ಕ *)addr = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT16 *)(*p_argv);
            ಭಙ್ಗ;
        ಸ್ವಭಾವ:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    ಪ್ರತಿ(i = 0; i < 5; i++)
    {
        ಯದಿ((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        ಅನ್ಯಥಾ
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned ಪೂರ್ಣಾಙ್ಕ*)reg_base)[i]);
    }

    ಪ್ರತಿ(i = 0; i < ರಾಶಿ_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned ಪೂರ್ಣಾಙ್ಕ*)ರಾಶಿ_base)[i]);
    }
#endif
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     * everywhere, but it is on some platforms, and it doesn't harm
     * anything when it isn't needed. */
    cif->bytes = (cif->bytes + 7) & ~7;

    /* Flag to indicate that he return value is in fact a struct */
    cif->rstruct_flag = 0;

    /* Set the return type flag */
    ವಿಕಲ್ಪ(cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_SINT8:
    ಪಕ್ಷ FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT16:
    ಪಕ್ಷ FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಪಕ್ಷ FFI_TYPE_SINT32:
    ಪಕ್ಷ FFI_TYPE_UINT32:
    ಪಕ್ಷ FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        ಯದಿ(!pass_struct_on_ರಾಶಿ(cif->rtype))
        {
            ಯದಿ(cif->rtype->size <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            ಅನ್ಯಥಾ ಯದಿ(cif->rtype->size <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            ಅನ್ಯಥಾ ಯದಿ(cif->rtype->size <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            ಅನ್ಯಥಾ ಯದಿ(cif->rtype->size <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            ಅನ್ಯಥಾ
                cif->flags = (unsigned)cif->rtype->type;
        }
        ಅನ್ಯಥಾ
            cif->flags = (unsigned)cif->rtype->type;
        ಭಙ್ಗ;
    ಸ್ವಭಾವ:
        cif->flags = (unsigned)cif->rtype->type;
        ಭಙ್ಗ;
    }

    ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
    extended_cif ecif;

    unsigned ಪೂರ್ಣಾಙ್ಕ size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    ಪ್ರತಿ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    ಯದಿ((rvalue == ರಿಕ್ತ) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    ಅನ್ಯಥಾ
        ecif.rvalue = rvalue;

    ವಿಕಲ್ಪ(cif->abi)
    {
    ಪಕ್ಷ FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        ಭಙ್ಗ;
    ಸ್ವಭಾವ:
        FFI_ASSERT(0);
        ಭಙ್ಗ;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ ffi_prep_incoming_args_SYSV(ವರ್ಣ *ರಾಶಿ, ಶೂನ್ಯ **rvalue,
    ಶೂನ್ಯ **avalue, ffi_cif *cif)
{
    ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i, reg_mask = 0;
    ಪಞ್ಜಿಕಾ ಶೂನ್ಯ **p_argv;
    ಪಞ್ಜಿಕಾ ffi_type **p_arg;
    ಪಞ್ಜಿಕಾ ವರ್ಣ *reg_base = ರಾಶಿ;
    ಪಞ್ಜಿಕಾ ವರ್ಣ *ರಾಶಿ_base = ರಾಶಿ + 20;
    ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ ರಾಶಿ_offset = 0;

#ifdef DEBUG
    /* Debugging */
    ಪ್ರತಿ(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned ಪೂರ್ಣಾಙ್ಕ*)ರಾಶಿ)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    ಯದಿ(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(ಶೂನ್ಯ **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    ಪ್ರತಿ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        ಪೂರ್ಣಾಙ್ಕ alignment = (*p_arg)->alignment;

        *p_argv = 0;

        ಯದಿ(z % 4 != 0)
            z += (4 - z % 4);

        ಯದಿ(reg_mask != 0x1f)
        {
            ಯದಿ(pass_struct_on_ರಾಶಿ(*p_arg))
            {
                *p_argv = (ಶೂನ್ಯ*)ರಾಶಿ_base + ರಾಶಿ_offset;
                ರಾಶಿ_offset += z;
            }
            ಅನ್ಯಥಾ ಯದಿ(z <= ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
            {
                ವರ್ಣ index = 0;

                ಯಾವತ್((reg_mask >> index) & 1)
                    index++;

                *p_argv = (ಶೂನ್ಯ*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            ಅನ್ಯಥಾ ಯದಿ(z == 2 * ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
            {
                ಯದಿ(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (ಶೂನ್ಯ*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                ಅನ್ಯಥಾ ಯದಿ(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (ಶೂನ್ಯ*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        ಯದಿ(!*p_argv)
        {
            *p_argv = (ಶೂನ್ಯ*)ರಾಶಿ_base + ರಾಶಿ_offset;
            ರಾಶಿ_offset += z;
        }

        ಯದಿ((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == ರಿಕ್ತ)
        {
            ಯದಿ(alignment == 1)
                **(unsigned ಪೂರ್ಣಾಙ್ಕ**)p_argv <<= 24;
            ಅನ್ಯಥಾ ಯದಿ(alignment == 2)
                **(unsigned ಪೂರ್ಣಾಙ್ಕ**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    ಪ್ರತಿ(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned ಪೂರ್ಣಾಙ್ಕ**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned ಪೂರ್ಣಾಙ್ಕ ffi_closure_SYSV_inner(ffi_closure *closure, ಶೂನ್ಯ **respp,
    ಶೂನ್ಯ *args)
{
    ffi_cif *cif;
    ಶೂನ್ಯ **arg_area;
    unsigned ಪೂರ್ಣಾಙ್ಕ i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    ಪ್ರತಿ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (ಶೂನ್ಯ **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    ಪ್ರತ್ಯಾಗಮ cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*), ಶೂನ್ಯ *user_data,
    ಶೂನ್ಯ *codeloc)
{
    ಯದಿ (cif->abi != FFI_SYSV)
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

    unsigned ವರ್ಣ *__tramp = (unsigned ವರ್ಣ*)(&closure->tramp[0]);
    unsigned ಪೂರ್ಣಾಙ್ಕ  __fun = (unsigned ಪೂರ್ಣಾಙ್ಕ)(&ffi_closure_SYSV);
    unsigned ಪೂರ್ಣಾಙ್ಕ  __ctx = (unsigned ಪೂರ್ಣಾಙ್ಕ)(codeloc);
    unsigned ಪೂರ್ಣಾಙ್ಕ  __rstruct_flag = (unsigned ಪೂರ್ಣಾಙ್ಕ)(cif->rstruct_flag);
    unsigned ಪೂರ್ಣಾಙ್ಕ  __inner = (unsigned ಪೂರ್ಣಾಙ್ಕ)(&ffi_closure_SYSV_inner);
    *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[20] = __ctx;
    *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[24] = __rstruct_flag;
    *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[28] = __inner;
    *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_data = user_data;
    closure->fun  = fun;

    ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

