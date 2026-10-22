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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <stdio.h>
#ضمن <unistd.h>
#ضمن <asm/unistd.h>

/* #define DEBUG */

خارجي فراغ ffi_call_SYSV(فراغ (*)(حرف *, extended_cif *), extended_cif *,
    unsigned صحيح, unsigned صحيح, unsigned صحيح*, unsigned صحيح,
    فراغ (*fn)(فراغ));
خارجي فراغ ffi_closure_SYSV (ffi_closure *);

unsigned صحيح pass_struct_on_مكدس(ffi_type *type)
{
    إذا(type->type != FFI_TYPE_STRUCT)
        أرجع 0;

    إذا(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        أرجع 1;

    إذا(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        أرجع 1;

    أرجع 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

فراغ ffi_prep_وسائط(حرف *مكدس, extended_cif *ecif)
{
    unsigned صحيح i;
    فراغ **p_argv;
    ffi_type **p_arg;
    حرف *reg_base = مكدس;
    حرف *مكدس_base = مكدس + 20;
    unsigned صحيح مكدس_offset = 0;
    unsigned صحيح reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    إذا(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(فراغ**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    لكل(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        صحيح alignment = (*p_arg)->alignment;
        صحيح type = (*p_arg)->type;
        حرف *addr = 0;

        إذا(z % 4 != 0)
            z += (4 - z % 4);

        إذا(reg_mask != 0x1f)
        {
            إذا(pass_struct_on_مكدس(*p_arg))
            {
                addr = مكدس_base + مكدس_offset;
                مكدس_offset += z;
            }
            وإلا إذا(z == حجم(صحيح))
            {
                حرف index = 0;

                طالما((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            وإلا إذا(z == 2 * حجم(صحيح))
            {
                إذا(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                وإلا إذا(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        إذا(!addr)
        {
            addr = مكدس_base + مكدس_offset;
            مكدس_offset += z;
        }

        إذا(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == عدم)
            type = (*p_arg)->elements[0]->type;

        بدل(type)
        {
        حالة FFI_TYPE_UINT8:
            *(unsigned صحيح *)addr = (unsigned صحيح)*(UINT8 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_SINT8:
            *(signed صحيح *)addr = (signed صحيح)*(SINT8 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_UINT16:
            *(unsigned صحيح *)addr = (unsigned صحيح)*(UINT16 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_SINT16:
            *(signed صحيح *)addr = (signed صحيح)*(SINT16 *)(*p_argv);
            اكسر;
        افتراضي:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    لكل(i = 0; i < 5; i++)
    {
        إذا((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        وإلا
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned صحيح*)reg_base)[i]);
    }

    لكل(i = 0; i < مكدس_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned صحيح*)مكدس_base)[i]);
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
    بدل(cif->rtype->type)
    {
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        اكسر;
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        اكسر;
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        اكسر;
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        اكسر;
    حالة FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        إذا(!pass_struct_on_مكدس(cif->rtype))
        {
            إذا(cif->rtype->size <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            وإلا إذا(cif->rtype->size <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            وإلا إذا(cif->rtype->size <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            وإلا إذا(cif->rtype->size <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            وإلا
                cif->flags = (unsigned)cif->rtype->type;
        }
        وإلا
            cif->flags = (unsigned)cif->rtype->type;
        اكسر;
    افتراضي:
        cif->flags = (unsigned)cif->rtype->type;
        اكسر;
    }

    أرجع FFI_OK;
}

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
    extended_cif ecif;

    unsigned صحيح size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    إذا((rvalue == عدم) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    وإلا
        ecif.rvalue = rvalue;

    بدل(cif->abi)
    {
    حالة FFI_SYSV:
        ffi_call_SYSV(ffi_prep_وسائط, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        اكسر;
    افتراضي:
        FFI_ASSERT(0);
        اكسر;
    }
}

ساكن فراغ ffi_prep_incoming_وسائط_SYSV(حرف *مكدس, فراغ **rvalue,
    فراغ **avalue, ffi_cif *cif)
{
    سجل unsigned صحيح i, reg_mask = 0;
    سجل فراغ **p_argv;
    سجل ffi_type **p_arg;
    سجل حرف *reg_base = مكدس;
    سجل حرف *مكدس_base = مكدس + 20;
    سجل unsigned صحيح مكدس_offset = 0;

#ifdef DEBUG
    /* Debugging */
    لكل(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned صحيح*)مكدس)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    إذا(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(فراغ **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        صحيح alignment = (*p_arg)->alignment;

        *p_argv = 0;

        إذا(z % 4 != 0)
            z += (4 - z % 4);

        إذا(reg_mask != 0x1f)
        {
            إذا(pass_struct_on_مكدس(*p_arg))
            {
                *p_argv = (فراغ*)مكدس_base + مكدس_offset;
                مكدس_offset += z;
            }
            وإلا إذا(z <= حجم(صحيح))
            {
                حرف index = 0;

                طالما((reg_mask >> index) & 1)
                    index++;

                *p_argv = (فراغ*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            وإلا إذا(z == 2 * حجم(صحيح))
            {
                إذا(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (فراغ*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                وإلا إذا(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (فراغ*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        إذا(!*p_argv)
        {
            *p_argv = (فراغ*)مكدس_base + مكدس_offset;
            مكدس_offset += z;
        }

        إذا((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == عدم)
        {
            إذا(alignment == 1)
                **(unsigned صحيح**)p_argv <<= 24;
            وإلا إذا(alignment == 2)
                **(unsigned صحيح**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    لكل(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned صحيح**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned صحيح ffi_closure_SYSV_inner(ffi_closure *closure, فراغ **respp,
    فراغ *وسائط)
{
    ffi_cif *cif;
    فراغ **arg_area;
    unsigned صحيح i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (فراغ **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_وسائط_SYSV(وسائط, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    أرجع cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*), فراغ *user_data,
    فراغ *codeloc)
{
    إذا (cif->abi != FFI_SYSV)
      أرجع FFI_BAD_ABI;

    unsigned حرف *__tramp = (unsigned حرف*)(&closure->tramp[0]);
    unsigned صحيح  __fun = (unsigned صحيح)(&ffi_closure_SYSV);
    unsigned صحيح  __ctx = (unsigned صحيح)(codeloc);
    unsigned صحيح  __rstruct_flag = (unsigned صحيح)(cif->rstruct_flag);
    unsigned صحيح  __inner = (unsigned صحيح)(&ffi_closure_SYSV_inner);
    *(unsigned صحيح*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned صحيح*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned صحيح*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned صحيح*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned صحيح*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned صحيح*) &__tramp[20] = __ctx;
    *(unsigned صحيح*) &__tramp[24] = __rstruct_flag;
    *(unsigned صحيح*) &__tramp[28] = __inner;
    *(unsigned صحيح*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_data = user_data;
    closure->fun  = fun;

    أرجع FFI_OK;
}

