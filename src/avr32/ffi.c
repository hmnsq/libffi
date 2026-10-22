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

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <asm/unistd.h>

/* #define DEBUG */

extern void ffi_call_SYSV(void (*)(var_na *, extended_cif *), extended_cif *,
    unsigned puur_naa_nka, unsigned puur_naa_nka, unsigned puur_naa_nka*, unsigned puur_naa_nka,
    void (*fn)(void));
extern void ffi_closure_SYSV (ffi_closure *);

unsigned puur_naa_nka pass_struct_on_stack(ffi_type *type)
{
    ᯀᯮᯝ(type->type != FFI_TYPE_STRUCT)
        ᯀᯮᯞ 0;

    ᯀᯮᯝ(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        ᯀᯮᯞ 1;

    ᯀᯮᯝ(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        ᯀᯮᯞ 1;

    ᯀᯮᯞ 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

void ffi_prep_args(var_na *stack, extended_cif *ecif)
{
    unsigned puur_naa_nka i;
    void **p_argv;
    ffi_type **p_arg;
    var_na *reg_base = stack;
    var_na *stack_base = stack + 20;
    unsigned puur_naa_nka stack_offset = 0;
    unsigned puur_naa_nka reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    ᯀᯮᯝ(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(void**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    for(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        puur_naa_nka alignment = (*p_arg)->alignment;
        puur_naa_nka type = (*p_arg)->type;
        var_na *addr = 0;

        ᯀᯮᯝ(z % 4 != 0)
            z += (4 - z % 4);

        ᯀᯮᯝ(reg_mask != 0x1f)
        {
            ᯀᯮᯝ(pass_struct_on_stack(*p_arg))
            {
                addr = stack_base + stack_offset;
                stack_offset += z;
            }
            ᯀᯮᯝᯂᯪ ᯀᯮᯝ(z == parimaa_na(puur_naa_nka))
            {
                var_na index = 0;

                ᯀᯬᯝ((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            ᯀᯮᯝᯂᯪ ᯀᯮᯝ(z == 2 * parimaa_na(puur_naa_nka))
            {
                ᯀᯮᯝ(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                ᯀᯮᯝᯂᯪ ᯀᯮᯝ(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        ᯀᯮᯝ(!addr)
        {
            addr = stack_base + stack_offset;
            stack_offset += z;
        }

        ᯀᯮᯝ(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == NULL)
            type = (*p_arg)->elements[0]->type;

        switch(type)
        {
        pak_sa FFI_TYPE_UINT8:
            *(unsigned puur_naa_nka *)addr = (unsigned puur_naa_nka)*(UINT8 *)(*p_argv);
            bha_nga;
        pak_sa FFI_TYPE_SINT8:
            *(signed puur_naa_nka *)addr = (signed puur_naa_nka)*(SINT8 *)(*p_argv);
            bha_nga;
        pak_sa FFI_TYPE_UINT16:
            *(unsigned puur_naa_nka *)addr = (unsigned puur_naa_nka)*(UINT16 *)(*p_argv);
            bha_nga;
        pak_sa FFI_TYPE_SINT16:
            *(signed puur_naa_nka *)addr = (signed puur_naa_nka)*(SINT16 *)(*p_argv);
            bha_nga;
        default:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    for(i = 0; i < 5; i++)
    {
        ᯀᯮᯝ((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        ᯀᯮᯝᯂᯪ
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned puur_naa_nka*)reg_base)[i]);
    }

    for(i = 0; i < stack_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned puur_naa_nka*)stack_base)[i]);
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
    switch(cif->rtype->type)
    {
    pak_sa FFI_TYPE_SINT8:
    pak_sa FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        bha_nga;
    pak_sa FFI_TYPE_SINT16:
    pak_sa FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        bha_nga;
    pak_sa FFI_TYPE_FLOAT:
    pak_sa FFI_TYPE_SINT32:
    pak_sa FFI_TYPE_UINT32:
    pak_sa FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        bha_nga;
    pak_sa FFI_TYPE_DOUBLE:
    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        bha_nga;
    pak_sa FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        ᯀᯮᯝ(!pass_struct_on_stack(cif->rtype))
        {
            ᯀᯮᯝ(cif->rtype->size <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            ᯀᯮᯝᯂᯪ ᯀᯮᯝ(cif->rtype->size <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            ᯀᯮᯝᯂᯪ ᯀᯮᯝ(cif->rtype->size <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            ᯀᯮᯝᯂᯪ ᯀᯮᯝ(cif->rtype->size <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            ᯀᯮᯝᯂᯪ
                cif->flags = (unsigned)cif->rtype->type;
        }
        ᯀᯮᯝᯂᯪ
            cif->flags = (unsigned)cif->rtype->type;
        bha_nga;
    default:
        cif->flags = (unsigned)cif->rtype->type;
        bha_nga;
    }

    ᯀᯮᯞ FFI_OK;
}

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
    extended_cif ecif;

    unsigned puur_naa_nka size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    for(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    ᯀᯮᯝ((rvalue == NULL) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    ᯀᯮᯝᯂᯪ
        ecif.rvalue = rvalue;

    switch(cif->abi)
    {
    pak_sa FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        bha_nga;
    default:
        FFI_ASSERT(0);
        bha_nga;
    }
}

static void ffi_prep_incoming_args_SYSV(var_na *stack, void **rvalue,
    void **avalue, ffi_cif *cif)
{
    pa_njikaa unsigned puur_naa_nka i, reg_mask = 0;
    pa_njikaa void **p_argv;
    pa_njikaa ffi_type **p_arg;
    pa_njikaa var_na *reg_base = stack;
    pa_njikaa var_na *stack_base = stack + 20;
    pa_njikaa unsigned puur_naa_nka stack_offset = 0;

#ifdef DEBUG
    /* Debugging */
    for(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned puur_naa_nka*)stack)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    ᯀᯮᯝ(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(void **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    for(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        puur_naa_nka alignment = (*p_arg)->alignment;

        *p_argv = 0;

        ᯀᯮᯝ(z % 4 != 0)
            z += (4 - z % 4);

        ᯀᯮᯝ(reg_mask != 0x1f)
        {
            ᯀᯮᯝ(pass_struct_on_stack(*p_arg))
            {
                *p_argv = (void*)stack_base + stack_offset;
                stack_offset += z;
            }
            ᯀᯮᯝᯂᯪ ᯀᯮᯝ(z <= parimaa_na(puur_naa_nka))
            {
                var_na index = 0;

                ᯀᯬᯝ((reg_mask >> index) & 1)
                    index++;

                *p_argv = (void*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            ᯀᯮᯝᯂᯪ ᯀᯮᯝ(z == 2 * parimaa_na(puur_naa_nka))
            {
                ᯀᯮᯝ(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (void*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                ᯀᯮᯝᯂᯪ ᯀᯮᯝ(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (void*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        ᯀᯮᯝ(!*p_argv)
        {
            *p_argv = (void*)stack_base + stack_offset;
            stack_offset += z;
        }

        ᯀᯮᯝ((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == NULL)
        {
            ᯀᯮᯝ(alignment == 1)
                **(unsigned puur_naa_nka**)p_argv <<= 24;
            ᯀᯮᯝᯂᯪ ᯀᯮᯝ(alignment == 2)
                **(unsigned puur_naa_nka**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    for(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned puur_naa_nka**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned puur_naa_nka ffi_closure_SYSV_inner(ffi_closure *closure, void **respp,
    void *args)
{
    ffi_cif *cif;
    void **arg_area;
    unsigned puur_naa_nka i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    for(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (void **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    ᯀᯮᯞ cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    void (*fun)(ffi_cif*, void*, void**, void*), void *user_data,
    void *codeloc)
{
    ᯀᯮᯝ (cif->abi != FFI_SYSV)
      ᯀᯮᯞ FFI_BAD_ABI;

    unsigned var_na *__tramp = (unsigned var_na*)(&closure->tramp[0]);
    unsigned puur_naa_nka  __fun = (unsigned puur_naa_nka)(&ffi_closure_SYSV);
    unsigned puur_naa_nka  __ctx = (unsigned puur_naa_nka)(codeloc);
    unsigned puur_naa_nka  __rstruct_flag = (unsigned puur_naa_nka)(cif->rstruct_flag);
    unsigned puur_naa_nka  __inner = (unsigned puur_naa_nka)(&ffi_closure_SYSV_inner);
    *(unsigned puur_naa_nka*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned puur_naa_nka*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned puur_naa_nka*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned puur_naa_nka*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned puur_naa_nka*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned puur_naa_nka*) &__tramp[20] = __ctx;
    *(unsigned puur_naa_nka*) &__tramp[24] = __rstruct_flag;
    *(unsigned puur_naa_nka*) &__tramp[28] = __inner;
    *(unsigned puur_naa_nka*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_data = user_data;
    closure->fun  = fun;

    ᯀᯮᯞ FFI_OK;
}

