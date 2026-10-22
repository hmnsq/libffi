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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>
#شامل <stdio.h>
#شامل <unistd.h>
#شامل <asm/unistd.h>

/* #define DEBUG */

خارجی خالی ffi_call_SYSV(خالی (*)(حرف *, extended_cif *), extended_cif *,
    غیرعلامتی صحیح, غیرعلامتی صحیح, غیرعلامتی صحیح*, غیرعلامتی صحیح,
    خالی (*fn)(خالی));
خارجی خالی ffi_احاطہ_SYSV (ffi_احاطہ *);

غیرعلامتی صحیح pass_struct_on_ڈھیر(ffi_type *type)
{
    اگر(type->type != FFI_TYPE_STRUCT)
        واپس 0;

    اگر(type->alignment < type->سائز &&
        !(type->سائز == 4 || type->سائز == 8) &&
        !(type->سائز == 8 && type->alignment >= 4))
        واپس 1;

    اگر(type->سائز == 3 || type->سائز == 5 || type->سائز == 6 ||
        type->سائز == 7)
        واپس 1;

    واپس 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

خالی ffi_prep_دلائل(حرف *ڈھیر, extended_cif *ecif)
{
    غیرعلامتی صحیح i;
    خالی **p_argv;
    ffi_type **p_arg;
    حرف *reg_بنیاد = ڈھیر;
    حرف *ڈھیر_بنیاد = ڈھیر + 20;
    غیرعلامتی صحیح ڈھیر_انحراف = 0;
    غیرعلامتی صحیح reg_نقاب = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    اگر(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(خالی**)reg_بنیاد = ecif->rvalue;
        reg_نقاب |= 1;
    }

    برائے(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->سائز;
        صحیح alignment = (*p_arg)->alignment;
        صحیح type = (*p_arg)->type;
        حرف *addr = 0;

        اگر(z % 4 != 0)
            z += (4 - z % 4);

        اگر(reg_نقاب != 0x1f)
        {
            اگر(pass_struct_on_ڈھیر(*p_arg))
            {
                addr = ڈھیر_بنیاد + ڈھیر_انحراف;
                ڈھیر_انحراف += z;
            }
            ورنہ اگر(z == حجم(صحیح))
            {
                حرف اشاریہ = 0;

                جب((reg_نقاب >> اشاریہ) & 1)
                    اشاریہ++;

                addr = reg_بنیاد + (اشاریہ * 4);
                reg_نقاب |= (1 << اشاریہ);
            }
            ورنہ اگر(z == 2 * حجم(صحیح))
            {
                اگر(!((reg_نقاب >> 1) & 1))
                {
                    addr = reg_بنیاد + 4;
                    reg_نقاب |= (3 << 1);
                }
                ورنہ اگر(!((reg_نقاب >> 3) & 1))
                {
                    addr = reg_بنیاد + 12;
                    reg_نقاب |= (3 << 3);
                }
            }
        }

        اگر(!addr)
        {
            addr = ڈھیر_بنیاد + ڈھیر_انحراف;
            ڈھیر_انحراف += z;
        }

        اگر(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == تہی)
            type = (*p_arg)->elements[0]->type;

        انتخاب(type)
        {
        حالت FFI_TYPE_UINT8:
            *(غیرعلامتی صحیح *)addr = (غیرعلامتی صحیح)*(UINT8 *)(*p_argv);
            توڑو;
        حالت FFI_TYPE_SINT8:
            *(علامتی صحیح *)addr = (علامتی صحیح)*(SINT8 *)(*p_argv);
            توڑو;
        حالت FFI_TYPE_UINT16:
            *(غیرعلامتی صحیح *)addr = (غیرعلامتی صحیح)*(UINT16 *)(*p_argv);
            توڑو;
        حالت FFI_TYPE_SINT16:
            *(علامتی صحیح *)addr = (علامتی صحیح)*(SINT16 *)(*p_argv);
            توڑو;
        طےشدہ:
            حافظہ_نقل(addr, *p_argv, z);
        }

        p_argv++;
    }

#اگر_تعریف_شدہ DEBUG
    /* Debugging */
    برائے(i = 0; i < 5; i++)
    {
        اگر((reg_نقاب & (1 << i)) == 0)
            لکھو("r%d: (unused)\n", 12 - i);
        ورنہ
            لکھو("r%d: 0x%08x\n", 12 - i, ((غیرعلامتی صحیح*)reg_بنیاد)[i]);
    }

    برائے(i = 0; i < ڈھیر_انحراف / 4; i++)
    {
        لکھو("sp+%d: 0x%08x\n", i*4, ((غیرعلامتی صحیح*)ڈھیر_بنیاد)[i]);
    }
#اگر_ختم
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
    /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     * everywhere, but it is on some platforms, and it doesn't harm
     * anything when it isn't needed. */
    cif->بائٹیں = (cif->بائٹیں + 7) & ~7;

    /* Flag to indicate that he return value is in fact a struct */
    cif->rstruct_flag = 0;

    /* Set the return type flag */
    انتخاب(cif->rtype->type)
    {
    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_UINT8:
        cif->flags = (غیرعلامتی)FFI_TYPE_UINT8;
        توڑو;
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_UINT16:
        cif->flags = (غیرعلامتی)FFI_TYPE_UINT16;
        توڑو;
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_اشارہ_گر:
        cif->flags = (غیرعلامتی)FFI_TYPE_UINT32;
        توڑو;
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
        cif->flags = (غیرعلامتی)FFI_TYPE_UINT64;
        توڑو;
    حالت FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        اگر(!pass_struct_on_ڈھیر(cif->rtype))
        {
            اگر(cif->rtype->سائز <= 1)
                cif->flags = (غیرعلامتی)FFI_TYPE_UINT8;
            ورنہ اگر(cif->rtype->سائز <= 2)
                cif->flags = (غیرعلامتی)FFI_TYPE_UINT16;
            ورنہ اگر(cif->rtype->سائز <= 4)
                cif->flags = (غیرعلامتی)FFI_TYPE_UINT32;
            ورنہ اگر(cif->rtype->سائز <= 8)
                cif->flags = (غیرعلامتی)FFI_TYPE_UINT64;
            ورنہ
                cif->flags = (غیرعلامتی)cif->rtype->type;
        }
        ورنہ
            cif->flags = (غیرعلامتی)cif->rtype->type;
        توڑو;
    طےشدہ:
        cif->flags = (غیرعلامتی)cif->rtype->type;
        توڑو;
    }

    واپس FFI_OK;
}

خالی ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
    extended_cif ecif;

    غیرعلامتی صحیح سائز = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    برائے(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        سائز += (*p_arg)->سائز + (4 - (*p_arg)->سائز % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    اگر((rvalue == تہی) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->سائز);
    ورنہ
        ecif.rvalue = rvalue;

    انتخاب(cif->abi)
    {
    حالت FFI_SYSV:
        ffi_call_SYSV(ffi_prep_دلائل, &ecif, سائز, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        توڑو;
    طےشدہ:
        FFI_ASSERT(0);
        توڑو;
    }
}

ساکن خالی ffi_prep_incoming_دلائل_SYSV(حرف *ڈھیر, خالی **rvalue,
    خالی **avalue, ffi_cif *cif)
{
    اندراج غیرعلامتی صحیح i, reg_نقاب = 0;
    اندراج خالی **p_argv;
    اندراج ffi_type **p_arg;
    اندراج حرف *reg_بنیاد = ڈھیر;
    اندراج حرف *ڈھیر_بنیاد = ڈھیر + 20;
    اندراج غیرعلامتی صحیح ڈھیر_انحراف = 0;

#اگر_تعریف_شدہ DEBUG
    /* Debugging */
    برائے(i = 0; i < cif->nargs + 7; i++)
    {
        لکھو("sp+%d: 0x%08x\n", i*4, ((غیرعلامتی صحیح*)ڈھیر)[i]);
    }
#اگر_ختم

    /* If cif->flags is struct then we know it's not passed in registers */
    اگر(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(خالی **)reg_بنیاد;
        reg_نقاب |= 1;
    }

    p_argv = avalue;

    برائے(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->سائز;
        صحیح alignment = (*p_arg)->alignment;

        *p_argv = 0;

        اگر(z % 4 != 0)
            z += (4 - z % 4);

        اگر(reg_نقاب != 0x1f)
        {
            اگر(pass_struct_on_ڈھیر(*p_arg))
            {
                *p_argv = (خالی*)ڈھیر_بنیاد + ڈھیر_انحراف;
                ڈھیر_انحراف += z;
            }
            ورنہ اگر(z <= حجم(صحیح))
            {
                حرف اشاریہ = 0;

                جب((reg_نقاب >> اشاریہ) & 1)
                    اشاریہ++;

                *p_argv = (خالی*)reg_بنیاد + (اشاریہ * 4);
                reg_نقاب |= (1 << اشاریہ);
            }
            ورنہ اگر(z == 2 * حجم(صحیح))
            {
                اگر(!((reg_نقاب >> 1) & 1))
                {
                    *p_argv = (خالی*)reg_بنیاد + 4;
                    reg_نقاب |= (3 << 1);
                }
                ورنہ اگر(!((reg_نقاب >> 3) & 1))
                {
                    *p_argv = (خالی*)reg_بنیاد + 12;
                    reg_نقاب |= (3 << 3);
                }
            }
        }

        اگر(!*p_argv)
        {
            *p_argv = (خالی*)ڈھیر_بنیاد + ڈھیر_انحراف;
            ڈھیر_انحراف += z;
        }

        اگر((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == تہی)
        {
            اگر(alignment == 1)
                **(غیرعلامتی صحیح**)p_argv <<= 24;
            ورنہ اگر(alignment == 2)
                **(غیرعلامتی صحیح**)p_argv <<= 16;
        }

        p_argv++;
    }

#اگر_تعریف_شدہ DEBUG
    /* Debugging */
    برائے(i = 0; i < cif->nargs; i++)
    {
        لکھو("sp+%d: 0x%08x\n", i*4, *(((غیرعلامتی صحیح**)avalue)[i]));
    }
#اگر_ختم
}

/* This function is jumped to by the trampoline */

غیرعلامتی صحیح ffi_احاطہ_SYSV_inner(ffi_احاطہ *احاطہ, خالی **respp,
    خالی *دلائل)
{
    ffi_cif *cif;
    خالی **arg_area;
    غیرعلامتی صحیح i, سائز = 0;
    ffi_type **p_arg;

    cif = احاطہ->cif;

    برائے(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        سائز += (*p_arg)->سائز + (4 - (*p_arg)->سائز % 4);

    arg_area = (خالی **)alloca(سائز);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_دلائل_SYSV(دلائل, respp, arg_area, cif);

    (احاطہ->fun)(cif, *respp, arg_area, احاطہ->user_ڈیٹا);

    واپس cif->flags;
}

ffi_حالت ffi_prep_احاطہ_loc(ffi_احاطہ* احاطہ, ffi_cif* cif,
    خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*), خالی *user_ڈیٹا,
    خالی *codeloc)
{
    اگر (cif->abi != FFI_SYSV)
      واپس FFI_BAD_ABI;

    غیرعلامتی حرف *__tramp = (غیرعلامتی حرف*)(&احاطہ->tramp[0]);
    غیرعلامتی صحیح  __fun = (غیرعلامتی صحیح)(&ffi_احاطہ_SYSV);
    غیرعلامتی صحیح  __ctx = (غیرعلامتی صحیح)(codeloc);
    غیرعلامتی صحیح  __rstruct_flag = (غیرعلامتی صحیح)(cif->rstruct_flag);
    غیرعلامتی صحیح  __inner = (غیرعلامتی صحیح)(&ffi_احاطہ_SYSV_inner);
    *(غیرعلامتی صحیح*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(غیرعلامتی صحیح*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(غیرعلامتی صحیح*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(غیرعلامتی صحیح*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(غیرعلامتی صحیح*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(غیرعلامتی صحیح*) &__tramp[20] = __ctx;
    *(غیرعلامتی صحیح*) &__tramp[24] = __rstruct_flag;
    *(غیرعلامتی صحیح*) &__tramp[28] = __inner;
    *(غیرعلامتی صحیح*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    احاطہ->cif = cif;
    احاطہ->user_ڈیٹا = user_ڈیٹا;
    احاطہ->fun  = fun;

    واپس FFI_OK;
}

