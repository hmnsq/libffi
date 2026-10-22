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
    بینښه صحيح, بینښه صحيح, بینښه صحيح*, بینښه صحيح,
    فراغ (*fn)(فراغ));
خارجي فراغ ffi_تړون_SYSV (ffi_تړون *);

بینښه صحيح pass_struct_on_ډېری(ffi_type *type)
{
    إذا(type->type != FFI_TYPE_STRUCT)
        أرجع 0;

    إذا(type->alignment < type->اندازه &&
        !(type->اندازه == 4 || type->اندازه == 8) &&
        !(type->اندازه == 8 && type->alignment >= 4))
        أرجع 1;

    إذا(type->اندازه == 3 || type->اندازه == 5 || type->اندازه == 6 ||
        type->اندازه == 7)
        أرجع 1;

    أرجع 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

فراغ ffi_prep_دلیلونه(حرف *ډېری, extended_cif *ecif)
{
    بینښه صحيح i;
    فراغ **p_argv;
    ffi_type **p_arg;
    حرف *reg_بنسټ = ډېری;
    حرف *ډېری_بنسټ = ډېری + 20;
    بینښه صحيح ډېری_دننه_واټن = 0;
    بینښه صحيح reg_ماسک = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    إذا(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(فراغ**)reg_بنسټ = ecif->rvalue;
        reg_ماسک |= 1;
    }

    لكل(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->اندازه;
        صحيح alignment = (*p_arg)->alignment;
        صحيح type = (*p_arg)->type;
        حرف *addr = 0;

        إذا(z % 4 != 0)
            z += (4 - z % 4);

        إذا(reg_ماسک != 0x1f)
        {
            إذا(pass_struct_on_ډېری(*p_arg))
            {
                addr = ډېری_بنسټ + ډېری_دننه_واټن;
                ډېری_دننه_واټن += z;
            }
            وإلا إذا(z == حجم(صحيح))
            {
                حرف شاخص = 0;

                طالما((reg_ماسک >> شاخص) & 1)
                    شاخص++;

                addr = reg_بنسټ + (شاخص * 4);
                reg_ماسک |= (1 << شاخص);
            }
            وإلا إذا(z == 2 * حجم(صحيح))
            {
                إذا(!((reg_ماسک >> 1) & 1))
                {
                    addr = reg_بنسټ + 4;
                    reg_ماسک |= (3 << 1);
                }
                وإلا إذا(!((reg_ماسک >> 3) & 1))
                {
                    addr = reg_بنسټ + 12;
                    reg_ماسک |= (3 << 3);
                }
            }
        }

        إذا(!addr)
        {
            addr = ډېری_بنسټ + ډېری_دننه_واټن;
            ډېری_دننه_واټن += z;
        }

        إذا(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == عدم)
            type = (*p_arg)->elements[0]->type;

        بدل(type)
        {
        حالة FFI_TYPE_UINT8:
            *(بینښه صحيح *)addr = (بینښه صحيح)*(UINT8 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_SINT8:
            *(نښهدار صحيح *)addr = (نښهدار صحيح)*(SINT8 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_UINT16:
            *(بینښه صحيح *)addr = (بینښه صحيح)*(UINT16 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_SINT16:
            *(نښهدار صحيح *)addr = (نښهدار صحيح)*(SINT16 *)(*p_argv);
            اكسر;
        افتراضي:
            حافظه_نقل(addr, *p_argv, z);
        }

        p_argv++;
    }

#که_تعریف_شوی DEBUG
    /* Debugging */
    لكل(i = 0; i < 5; i++)
    {
        إذا((reg_ماسک & (1 << i)) == 0)
            چاپ_کړه("r%d: (unused)\n", 12 - i);
        وإلا
            چاپ_کړه("r%d: 0x%08x\n", 12 - i, ((بینښه صحيح*)reg_بنسټ)[i]);
    }

    لكل(i = 0; i < ډېری_دننه_واټن / 4; i++)
    {
        چاپ_کړه("sp+%d: 0x%08x\n", i*4, ((بینښه صحيح*)ډېری_بنسټ)[i]);
    }
#د_که_پای
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
    /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     * everywhere, but it is on some platforms, and it doesn't harm
     * anything when it isn't needed. */
    cif->بایټونه = (cif->بایټونه + 7) & ~7;

    /* Flag to indicate that he return value is in fact a struct */
    cif->rstruct_flag = 0;

    /* Set the return type flag */
    بدل(cif->rtype->type)
    {
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT8:
        cif->flags = (بینښه)FFI_TYPE_UINT8;
        اكسر;
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT16:
        cif->flags = (بینښه)FFI_TYPE_UINT16;
        اكسر;
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_اشاره_ګر:
        cif->flags = (بینښه)FFI_TYPE_UINT32;
        اكسر;
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
        cif->flags = (بینښه)FFI_TYPE_UINT64;
        اكسر;
    حالة FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        إذا(!pass_struct_on_ډېری(cif->rtype))
        {
            إذا(cif->rtype->اندازه <= 1)
                cif->flags = (بینښه)FFI_TYPE_UINT8;
            وإلا إذا(cif->rtype->اندازه <= 2)
                cif->flags = (بینښه)FFI_TYPE_UINT16;
            وإلا إذا(cif->rtype->اندازه <= 4)
                cif->flags = (بینښه)FFI_TYPE_UINT32;
            وإلا إذا(cif->rtype->اندازه <= 8)
                cif->flags = (بینښه)FFI_TYPE_UINT64;
            وإلا
                cif->flags = (بینښه)cif->rtype->type;
        }
        وإلا
            cif->flags = (بینښه)cif->rtype->type;
        اكسر;
    افتراضي:
        cif->flags = (بینښه)cif->rtype->type;
        اكسر;
    }

    أرجع FFI_OK;
}

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
    extended_cif ecif;

    بینښه صحيح اندازه = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        اندازه += (*p_arg)->اندازه + (4 - (*p_arg)->اندازه % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    إذا((rvalue == عدم) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->اندازه);
    وإلا
        ecif.rvalue = rvalue;

    بدل(cif->abi)
    {
    حالة FFI_SYSV:
        ffi_call_SYSV(ffi_prep_دلیلونه, &ecif, اندازه, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        اكسر;
    افتراضي:
        FFI_ASSERT(0);
        اكسر;
    }
}

ساكن فراغ ffi_prep_incoming_دلیلونه_SYSV(حرف *ډېری, فراغ **rvalue,
    فراغ **avalue, ffi_cif *cif)
{
    سجل بینښه صحيح i, reg_ماسک = 0;
    سجل فراغ **p_argv;
    سجل ffi_type **p_arg;
    سجل حرف *reg_بنسټ = ډېری;
    سجل حرف *ډېری_بنسټ = ډېری + 20;
    سجل بینښه صحيح ډېری_دننه_واټن = 0;

#که_تعریف_شوی DEBUG
    /* Debugging */
    لكل(i = 0; i < cif->nargs + 7; i++)
    {
        چاپ_کړه("sp+%d: 0x%08x\n", i*4, ((بینښه صحيح*)ډېری)[i]);
    }
#د_که_پای

    /* If cif->flags is struct then we know it's not passed in registers */
    إذا(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(فراغ **)reg_بنسټ;
        reg_ماسک |= 1;
    }

    p_argv = avalue;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->اندازه;
        صحيح alignment = (*p_arg)->alignment;

        *p_argv = 0;

        إذا(z % 4 != 0)
            z += (4 - z % 4);

        إذا(reg_ماسک != 0x1f)
        {
            إذا(pass_struct_on_ډېری(*p_arg))
            {
                *p_argv = (فراغ*)ډېری_بنسټ + ډېری_دننه_واټن;
                ډېری_دننه_واټن += z;
            }
            وإلا إذا(z <= حجم(صحيح))
            {
                حرف شاخص = 0;

                طالما((reg_ماسک >> شاخص) & 1)
                    شاخص++;

                *p_argv = (فراغ*)reg_بنسټ + (شاخص * 4);
                reg_ماسک |= (1 << شاخص);
            }
            وإلا إذا(z == 2 * حجم(صحيح))
            {
                إذا(!((reg_ماسک >> 1) & 1))
                {
                    *p_argv = (فراغ*)reg_بنسټ + 4;
                    reg_ماسک |= (3 << 1);
                }
                وإلا إذا(!((reg_ماسک >> 3) & 1))
                {
                    *p_argv = (فراغ*)reg_بنسټ + 12;
                    reg_ماسک |= (3 << 3);
                }
            }
        }

        إذا(!*p_argv)
        {
            *p_argv = (فراغ*)ډېری_بنسټ + ډېری_دننه_واټن;
            ډېری_دننه_واټن += z;
        }

        إذا((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == عدم)
        {
            إذا(alignment == 1)
                **(بینښه صحيح**)p_argv <<= 24;
            وإلا إذا(alignment == 2)
                **(بینښه صحيح**)p_argv <<= 16;
        }

        p_argv++;
    }

#که_تعریف_شوی DEBUG
    /* Debugging */
    لكل(i = 0; i < cif->nargs; i++)
    {
        چاپ_کړه("sp+%d: 0x%08x\n", i*4, *(((بینښه صحيح**)avalue)[i]));
    }
#د_که_پای
}

/* This function is jumped to by the trampoline */

بینښه صحيح ffi_تړون_SYSV_inner(ffi_تړون *تړون, فراغ **respp,
    فراغ *دلیلونه)
{
    ffi_cif *cif;
    فراغ **arg_area;
    بینښه صحيح i, اندازه = 0;
    ffi_type **p_arg;

    cif = تړون->cif;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        اندازه += (*p_arg)->اندازه + (4 - (*p_arg)->اندازه % 4);

    arg_area = (فراغ **)alloca(اندازه);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_دلیلونه_SYSV(دلیلونه, respp, arg_area, cif);

    (تړون->fun)(cif, *respp, arg_area, تړون->user_معلومات);

    أرجع cif->flags;
}

ffi_حالت ffi_prep_تړون_loc(ffi_تړون* تړون, ffi_cif* cif,
    فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*), فراغ *user_معلومات,
    فراغ *codeloc)
{
    إذا (cif->abi != FFI_SYSV)
      أرجع FFI_BAD_ABI;

    بینښه حرف *__tramp = (بینښه حرف*)(&تړون->tramp[0]);
    بینښه صحيح  __fun = (بینښه صحيح)(&ffi_تړون_SYSV);
    بینښه صحيح  __ctx = (بینښه صحيح)(codeloc);
    بینښه صحيح  __rstruct_flag = (بینښه صحيح)(cif->rstruct_flag);
    بینښه صحيح  __inner = (بینښه صحيح)(&ffi_تړون_SYSV_inner);
    *(بینښه صحيح*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(بینښه صحيح*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(بینښه صحيح*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(بینښه صحيح*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(بینښه صحيح*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(بینښه صحيح*) &__tramp[20] = __ctx;
    *(بینښه صحيح*) &__tramp[24] = __rstruct_flag;
    *(بینښه صحيح*) &__tramp[28] = __inner;
    *(بینښه صحيح*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    تړون->cif = cif;
    تړون->user_معلومات = user_معلومات;
    تړون->fun  = fun;

    أرجع FFI_OK;
}

