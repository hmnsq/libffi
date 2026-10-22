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

extern void ffi_call_SYSV(void (*)(char *, extended_cif *), extended_cif *,
    unsigned ціле, unsigned ціле, unsigned ціле*, unsigned ціле,
    void (*fn)(void));
extern void ffi_closure_SYSV (ffi_closure *);

unsigned ціле pass_struct_on_stack(ffi_type *type)
{
    якщо(type->type != FFI_TYPE_STRUCT)
        повернути 0;

    якщо(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        повернути 1;

    якщо(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        повернути 1;

    повернути 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

void ffi_prep_args(char *stack, extended_cif *ecif)
{
    unsigned ціле i;
    void **p_argv;
    ffi_type **p_arg;
    char *reg_base = stack;
    char *stack_base = stack + 20;
    unsigned ціле stack_offset = 0;
    unsigned ціле reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    якщо(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(void**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    для(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        ціле alignment = (*p_arg)->alignment;
        ціле type = (*p_arg)->type;
        char *addr = 0;

        якщо(z % 4 != 0)
            z += (4 - z % 4);

        якщо(reg_mask != 0x1f)
        {
            якщо(pass_struct_on_stack(*p_arg))
            {
                addr = stack_base + stack_offset;
                stack_offset += z;
            }
            інакше якщо(z == sizeof(ціле))
            {
                char index = 0;

                поки((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            інакше якщо(z == 2 * sizeof(ціле))
            {
                якщо(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                інакше якщо(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        якщо(!addr)
        {
            addr = stack_base + stack_offset;
            stack_offset += z;
        }

        якщо(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == NULL)
            type = (*p_arg)->elements[0]->type;

        switch(type)
        {
        case FFI_TYPE_UINT8:
            *(unsigned ціле *)addr = (unsigned ціле)*(UINT8 *)(*p_argv);
            break;
        case FFI_TYPE_SINT8:
            *(signed ціле *)addr = (signed ціле)*(SINT8 *)(*p_argv);
            break;
        case FFI_TYPE_UINT16:
            *(unsigned ціле *)addr = (unsigned ціле)*(UINT16 *)(*p_argv);
            break;
        case FFI_TYPE_SINT16:
            *(signed ціле *)addr = (signed ціле)*(SINT16 *)(*p_argv);
            break;
        default:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    для(i = 0; i < 5; i++)
    {
        якщо((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        інакше
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned ціле*)reg_base)[i]);
    }

    для(i = 0; i < stack_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned ціле*)stack_base)[i]);
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
    case FFI_TYPE_SINT8:
    case FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        break;
    case FFI_TYPE_SINT16:
    case FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        break;
    case FFI_TYPE_FLOAT:
    case FFI_TYPE_SINT32:
    case FFI_TYPE_UINT32:
    case FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        break;
    case FFI_TYPE_DOUBLE:
    case FFI_TYPE_SINT64:
    case FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        break;
    case FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        якщо(!pass_struct_on_stack(cif->rtype))
        {
            якщо(cif->rtype->size <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            інакше якщо(cif->rtype->size <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            інакше якщо(cif->rtype->size <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            інакше якщо(cif->rtype->size <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            інакше
                cif->flags = (unsigned)cif->rtype->type;
        }
        інакше
            cif->flags = (unsigned)cif->rtype->type;
        break;
    default:
        cif->flags = (unsigned)cif->rtype->type;
        break;
    }

    повернути FFI_OK;
}

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
    extended_cif ecif;

    unsigned ціле size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    для(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    якщо((rvalue == NULL) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    інакше
        ecif.rvalue = rvalue;

    switch(cif->abi)
    {
    case FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        break;
    default:
        FFI_ASSERT(0);
        break;
    }
}

static void ffi_prep_incoming_args_SYSV(char *stack, void **rvalue,
    void **avalue, ffi_cif *cif)
{
    register unsigned ціле i, reg_mask = 0;
    register void **p_argv;
    register ffi_type **p_arg;
    register char *reg_base = stack;
    register char *stack_base = stack + 20;
    register unsigned ціле stack_offset = 0;

#ifdef DEBUG
    /* Debugging */
    для(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned ціле*)stack)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    якщо(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(void **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    для(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        ціле alignment = (*p_arg)->alignment;

        *p_argv = 0;

        якщо(z % 4 != 0)
            z += (4 - z % 4);

        якщо(reg_mask != 0x1f)
        {
            якщо(pass_struct_on_stack(*p_arg))
            {
                *p_argv = (void*)stack_base + stack_offset;
                stack_offset += z;
            }
            інакше якщо(z <= sizeof(ціле))
            {
                char index = 0;

                поки((reg_mask >> index) & 1)
                    index++;

                *p_argv = (void*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            інакше якщо(z == 2 * sizeof(ціле))
            {
                якщо(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (void*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                інакше якщо(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (void*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        якщо(!*p_argv)
        {
            *p_argv = (void*)stack_base + stack_offset;
            stack_offset += z;
        }

        якщо((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == NULL)
        {
            якщо(alignment == 1)
                **(unsigned ціле**)p_argv <<= 24;
            інакше якщо(alignment == 2)
                **(unsigned ціле**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    для(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned ціле**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned ціле ffi_closure_SYSV_inner(ffi_closure *closure, void **respp,
    void *args)
{
    ffi_cif *cif;
    void **arg_area;
    unsigned ціле i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    для(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (void **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    повернути cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    void (*fun)(ffi_cif*, void*, void**, void*), void *user_data,
    void *codeloc)
{
    якщо (cif->abi != FFI_SYSV)
      повернути FFI_BAD_ABI;

    unsigned char *__tramp = (unsigned char*)(&closure->tramp[0]);
    unsigned ціле  __fun = (unsigned ціле)(&ffi_closure_SYSV);
    unsigned ціле  __ctx = (unsigned ціле)(codeloc);
    unsigned ціле  __rstruct_flag = (unsigned ціле)(cif->rstruct_flag);
    unsigned ціле  __inner = (unsigned ціле)(&ffi_closure_SYSV_inner);
    *(unsigned ціле*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned ціле*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned ціле*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned ціле*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned ціле*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned ціле*) &__tramp[20] = __ctx;
    *(unsigned ціле*) &__tramp[24] = __rstruct_flag;
    *(unsigned ціле*) &__tramp[28] = __inner;
    *(unsigned ціле*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_data = user_data;
    closure->fun  = fun;

    повернути FFI_OK;
}

