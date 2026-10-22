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
    بیعلامت صحيح, بیعلامت صحيح, بیعلامت صحيح*, بیعلامت صحيح,
    فراغ (*fn)(فراغ));
خارجي فراغ ffi_بستگی_SYSV (ffi_بستگی *);

بیعلامت صحيح pass_struct_on_پشته(ffi_type *type)
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

فراغ ffi_prep_آرگومانها(حرف *پشته, extended_cif *ecif)
{
    بیعلامت صحيح i;
    فراغ **p_argv;
    ffi_type **p_arg;
    حرف *reg_پایه = پشته;
    حرف *پشته_پایه = پشته + 20;
    بیعلامت صحيح پشته_جابه_جایی = 0;
    بیعلامت صحيح reg_ماسک = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    إذا(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(فراغ**)reg_پایه = ecif->rvalue;
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
            إذا(pass_struct_on_پشته(*p_arg))
            {
                addr = پشته_پایه + پشته_جابه_جایی;
                پشته_جابه_جایی += z;
            }
            وإلا إذا(z == حجم(صحيح))
            {
                حرف شاخص = 0;

                طالما((reg_ماسک >> شاخص) & 1)
                    شاخص++;

                addr = reg_پایه + (شاخص * 4);
                reg_ماسک |= (1 << شاخص);
            }
            وإلا إذا(z == 2 * حجم(صحيح))
            {
                إذا(!((reg_ماسک >> 1) & 1))
                {
                    addr = reg_پایه + 4;
                    reg_ماسک |= (3 << 1);
                }
                وإلا إذا(!((reg_ماسک >> 3) & 1))
                {
                    addr = reg_پایه + 12;
                    reg_ماسک |= (3 << 3);
                }
            }
        }

        إذا(!addr)
        {
            addr = پشته_پایه + پشته_جابه_جایی;
            پشته_جابه_جایی += z;
        }

        إذا(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == عدم)
            type = (*p_arg)->elements[0]->type;

        بدل(type)
        {
        حالة FFI_TYPE_UINT8:
            *(بیعلامت صحيح *)addr = (بیعلامت صحيح)*(UINT8 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_SINT8:
            *(علامتدار صحيح *)addr = (علامتدار صحيح)*(SINT8 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_UINT16:
            *(بیعلامت صحيح *)addr = (بیعلامت صحيح)*(UINT16 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_SINT16:
            *(علامتدار صحيح *)addr = (علامتدار صحيح)*(SINT16 *)(*p_argv);
            اكسر;
        افتراضي:
            کپی_حافظه(addr, *p_argv, z);
        }

        p_argv++;
    }

#اگر_تعریف_شده DEBUG
    /* Debugging */
    لكل(i = 0; i < 5; i++)
    {
        إذا((reg_ماسک & (1 << i)) == 0)
            چاپ_کن("r%d: (unused)\n", 12 - i);
        وإلا
            چاپ_کن("r%d: 0x%08x\n", 12 - i, ((بیعلامت صحيح*)reg_پایه)[i]);
    }

    لكل(i = 0; i < پشته_جابه_جایی / 4; i++)
    {
        چاپ_کن("sp+%d: 0x%08x\n", i*4, ((بیعلامت صحيح*)پشته_پایه)[i]);
    }
#پایان_اگر
}

/* Perform machine dependent cif processing */
ffi_وضعیت ffi_prep_cif_machdep(ffi_cif *cif)
{
    /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     * everywhere, but it is on some platforms, and it doesn't harm
     * anything when it isn't needed. */
    cif->بایتها = (cif->بایتها + 7) & ~7;

    /* Flag to indicate that he return value is in fact a struct */
    cif->rstruct_flag = 0;

    /* Set the return type flag */
    بدل(cif->rtype->type)
    {
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT8:
        cif->flags = (بیعلامت)FFI_TYPE_UINT8;
        اكسر;
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT16:
        cif->flags = (بیعلامت)FFI_TYPE_UINT16;
        اكسر;
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_اشاره_گر:
        cif->flags = (بیعلامت)FFI_TYPE_UINT32;
        اكسر;
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
        cif->flags = (بیعلامت)FFI_TYPE_UINT64;
        اكسر;
    حالة FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        إذا(!pass_struct_on_پشته(cif->rtype))
        {
            إذا(cif->rtype->اندازه <= 1)
                cif->flags = (بیعلامت)FFI_TYPE_UINT8;
            وإلا إذا(cif->rtype->اندازه <= 2)
                cif->flags = (بیعلامت)FFI_TYPE_UINT16;
            وإلا إذا(cif->rtype->اندازه <= 4)
                cif->flags = (بیعلامت)FFI_TYPE_UINT32;
            وإلا إذا(cif->rtype->اندازه <= 8)
                cif->flags = (بیعلامت)FFI_TYPE_UINT64;
            وإلا
                cif->flags = (بیعلامت)cif->rtype->type;
        }
        وإلا
            cif->flags = (بیعلامت)cif->rtype->type;
        اكسر;
    افتراضي:
        cif->flags = (بیعلامت)cif->rtype->type;
        اكسر;
    }

    أرجع FFI_OK;
}

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
    extended_cif ecif;

    بیعلامت صحيح اندازه = 0, i = 0;
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
        ffi_call_SYSV(ffi_prep_آرگومانها, &ecif, اندازه, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        اكسر;
    افتراضي:
        FFI_ASSERT(0);
        اكسر;
    }
}

ساكن فراغ ffi_prep_incoming_آرگومانها_SYSV(حرف *پشته, فراغ **rvalue,
    فراغ **avalue, ffi_cif *cif)
{
    سجل بیعلامت صحيح i, reg_ماسک = 0;
    سجل فراغ **p_argv;
    سجل ffi_type **p_arg;
    سجل حرف *reg_پایه = پشته;
    سجل حرف *پشته_پایه = پشته + 20;
    سجل بیعلامت صحيح پشته_جابه_جایی = 0;

#اگر_تعریف_شده DEBUG
    /* Debugging */
    لكل(i = 0; i < cif->nargs + 7; i++)
    {
        چاپ_کن("sp+%d: 0x%08x\n", i*4, ((بیعلامت صحيح*)پشته)[i]);
    }
#پایان_اگر

    /* If cif->flags is struct then we know it's not passed in registers */
    إذا(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(فراغ **)reg_پایه;
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
            إذا(pass_struct_on_پشته(*p_arg))
            {
                *p_argv = (فراغ*)پشته_پایه + پشته_جابه_جایی;
                پشته_جابه_جایی += z;
            }
            وإلا إذا(z <= حجم(صحيح))
            {
                حرف شاخص = 0;

                طالما((reg_ماسک >> شاخص) & 1)
                    شاخص++;

                *p_argv = (فراغ*)reg_پایه + (شاخص * 4);
                reg_ماسک |= (1 << شاخص);
            }
            وإلا إذا(z == 2 * حجم(صحيح))
            {
                إذا(!((reg_ماسک >> 1) & 1))
                {
                    *p_argv = (فراغ*)reg_پایه + 4;
                    reg_ماسک |= (3 << 1);
                }
                وإلا إذا(!((reg_ماسک >> 3) & 1))
                {
                    *p_argv = (فراغ*)reg_پایه + 12;
                    reg_ماسک |= (3 << 3);
                }
            }
        }

        إذا(!*p_argv)
        {
            *p_argv = (فراغ*)پشته_پایه + پشته_جابه_جایی;
            پشته_جابه_جایی += z;
        }

        إذا((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == عدم)
        {
            إذا(alignment == 1)
                **(بیعلامت صحيح**)p_argv <<= 24;
            وإلا إذا(alignment == 2)
                **(بیعلامت صحيح**)p_argv <<= 16;
        }

        p_argv++;
    }

#اگر_تعریف_شده DEBUG
    /* Debugging */
    لكل(i = 0; i < cif->nargs; i++)
    {
        چاپ_کن("sp+%d: 0x%08x\n", i*4, *(((بیعلامت صحيح**)avalue)[i]));
    }
#پایان_اگر
}

/* This function is jumped to by the trampoline */

بیعلامت صحيح ffi_بستگی_SYSV_inner(ffi_بستگی *بستگی, فراغ **respp,
    فراغ *آرگومانها)
{
    ffi_cif *cif;
    فراغ **arg_area;
    بیعلامت صحيح i, اندازه = 0;
    ffi_type **p_arg;

    cif = بستگی->cif;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        اندازه += (*p_arg)->اندازه + (4 - (*p_arg)->اندازه % 4);

    arg_area = (فراغ **)alloca(اندازه);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_آرگومانها_SYSV(آرگومانها, respp, arg_area, cif);

    (بستگی->fun)(cif, *respp, arg_area, بستگی->user_داده);

    أرجع cif->flags;
}

ffi_وضعیت ffi_prep_بستگی_loc(ffi_بستگی* بستگی, ffi_cif* cif,
    فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*), فراغ *user_داده,
    فراغ *codeloc)
{
    إذا (cif->abi != FFI_SYSV)
      أرجع FFI_BAD_ABI;

    بیعلامت حرف *__tramp = (بیعلامت حرف*)(&بستگی->tramp[0]);
    بیعلامت صحيح  __fun = (بیعلامت صحيح)(&ffi_بستگی_SYSV);
    بیعلامت صحيح  __ctx = (بیعلامت صحيح)(codeloc);
    بیعلامت صحيح  __rstruct_flag = (بیعلامت صحيح)(cif->rstruct_flag);
    بیعلامت صحيح  __inner = (بیعلامت صحيح)(&ffi_بستگی_SYSV_inner);
    *(بیعلامت صحيح*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(بیعلامت صحيح*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(بیعلامت صحيح*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(بیعلامت صحيح*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(بیعلامت صحيح*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(بیعلامت صحيح*) &__tramp[20] = __ctx;
    *(بیعلامت صحيح*) &__tramp[24] = __rstruct_flag;
    *(بیعلامت صحيح*) &__tramp[28] = __inner;
    *(بیعلامت صحيح*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    بستگی->cif = cif;
    بستگی->user_داده = user_داده;
    بستگی->fun  = fun;

    أرجع FFI_OK;
}

