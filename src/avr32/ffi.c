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
    بنانشانی صحيح, بنانشانی صحيح, بنانشانی صحيح*, بنانشانی صحيح,
    فراغ (*fn)(فراغ));
خارجي فراغ ffi_گھیرا_SYSV (ffi_گھیرا *);

بنانشانی صحيح pass_struct_on_ڈھیر(ffi_type *type)
{
    إذا(type->type != FFI_TYPE_STRUCT)
        أرجع 0;

    إذا(type->alignment < type->ناپ &&
        !(type->ناپ == 4 || type->ناپ == 8) &&
        !(type->ناپ == 8 && type->alignment >= 4))
        أرجع 1;

    إذا(type->ناپ == 3 || type->ناپ == 5 || type->ناپ == 6 ||
        type->ناپ == 7)
        أرجع 1;

    أرجع 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

فراغ ffi_prep_دلیلاں(حرف *ڈھیر, extended_cif *ecif)
{
    بنانشانی صحيح i;
    فراغ **p_argv;
    ffi_type **p_arg;
    حرف *reg_ادھار = ڈھیر;
    حرف *ڈھیر_ادھار = ڈھیر + 20;
    بنانشانی صحيح ڈھیر_وتھ = 0;
    بنانشانی صحيح reg_نقاب = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    إذا(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(فراغ**)reg_ادھار = ecif->rvalue;
        reg_نقاب |= 1;
    }

    لكل(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->ناپ;
        صحيح alignment = (*p_arg)->alignment;
        صحيح type = (*p_arg)->type;
        حرف *addr = 0;

        إذا(z % 4 != 0)
            z += (4 - z % 4);

        إذا(reg_نقاب != 0x1f)
        {
            إذا(pass_struct_on_ڈھیر(*p_arg))
            {
                addr = ڈھیر_ادھار + ڈھیر_وتھ;
                ڈھیر_وتھ += z;
            }
            وإلا إذا(z == حجم(صحيح))
            {
                حرف اشاریہ = 0;

                طالما((reg_نقاب >> اشاریہ) & 1)
                    اشاریہ++;

                addr = reg_ادھار + (اشاریہ * 4);
                reg_نقاب |= (1 << اشاریہ);
            }
            وإلا إذا(z == 2 * حجم(صحيح))
            {
                إذا(!((reg_نقاب >> 1) & 1))
                {
                    addr = reg_ادھار + 4;
                    reg_نقاب |= (3 << 1);
                }
                وإلا إذا(!((reg_نقاب >> 3) & 1))
                {
                    addr = reg_ادھار + 12;
                    reg_نقاب |= (3 << 3);
                }
            }
        }

        إذا(!addr)
        {
            addr = ڈھیر_ادھار + ڈھیر_وتھ;
            ڈھیر_وتھ += z;
        }

        إذا(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == عدم)
            type = (*p_arg)->elements[0]->type;

        بدل(type)
        {
        حالة FFI_TYPE_UINT8:
            *(بنانشانی صحيح *)addr = (بنانشانی صحيح)*(UINT8 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_SINT8:
            *(نشانیوالا صحيح *)addr = (نشانیوالا صحيح)*(SINT8 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_UINT16:
            *(بنانشانی صحيح *)addr = (بنانشانی صحيح)*(UINT16 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_SINT16:
            *(نشانیوالا صحيح *)addr = (نشانیوالا صحيح)*(SINT16 *)(*p_argv);
            اكسر;
        افتراضي:
            حافظہ_نقل(addr, *p_argv, z);
        }

        p_argv++;
    }

#جے_تعریف_شدہ DEBUG
    /* Debugging */
    لكل(i = 0; i < 5; i++)
    {
        إذا((reg_نقاب & (1 << i)) == 0)
            لکھو("r%d: (unused)\n", 12 - i);
        وإلا
            لکھو("r%d: 0x%08x\n", 12 - i, ((بنانشانی صحيح*)reg_ادھار)[i]);
    }

    لكل(i = 0; i < ڈھیر_وتھ / 4; i++)
    {
        لکھو("sp+%d: 0x%08x\n", i*4, ((بنانشانی صحيح*)ڈھیر_ادھار)[i]);
    }
#جے_انت
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
    /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     * everywhere, but it is on some platforms, and it doesn't harm
     * anything when it isn't needed. */
    cif->بائٹاں = (cif->بائٹاں + 7) & ~7;

    /* Flag to indicate that he return value is in fact a struct */
    cif->rstruct_flag = 0;

    /* Set the return type flag */
    بدل(cif->rtype->type)
    {
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT8:
        cif->flags = (بنانشانی)FFI_TYPE_UINT8;
        اكسر;
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT16:
        cif->flags = (بنانشانی)FFI_TYPE_UINT16;
        اكسر;
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_اشارہ_گر:
        cif->flags = (بنانشانی)FFI_TYPE_UINT32;
        اكسر;
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
        cif->flags = (بنانشانی)FFI_TYPE_UINT64;
        اكسر;
    حالة FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        إذا(!pass_struct_on_ڈھیر(cif->rtype))
        {
            إذا(cif->rtype->ناپ <= 1)
                cif->flags = (بنانشانی)FFI_TYPE_UINT8;
            وإلا إذا(cif->rtype->ناپ <= 2)
                cif->flags = (بنانشانی)FFI_TYPE_UINT16;
            وإلا إذا(cif->rtype->ناپ <= 4)
                cif->flags = (بنانشانی)FFI_TYPE_UINT32;
            وإلا إذا(cif->rtype->ناپ <= 8)
                cif->flags = (بنانشانی)FFI_TYPE_UINT64;
            وإلا
                cif->flags = (بنانشانی)cif->rtype->type;
        }
        وإلا
            cif->flags = (بنانشانی)cif->rtype->type;
        اكسر;
    افتراضي:
        cif->flags = (بنانشانی)cif->rtype->type;
        اكسر;
    }

    أرجع FFI_OK;
}

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
    extended_cif ecif;

    بنانشانی صحيح ناپ = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        ناپ += (*p_arg)->ناپ + (4 - (*p_arg)->ناپ % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    إذا((rvalue == عدم) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->ناپ);
    وإلا
        ecif.rvalue = rvalue;

    بدل(cif->abi)
    {
    حالة FFI_SYSV:
        ffi_call_SYSV(ffi_prep_دلیلاں, &ecif, ناپ, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        اكسر;
    افتراضي:
        FFI_ASSERT(0);
        اكسر;
    }
}

ساكن فراغ ffi_prep_incoming_دلیلاں_SYSV(حرف *ڈھیر, فراغ **rvalue,
    فراغ **avalue, ffi_cif *cif)
{
    سجل بنانشانی صحيح i, reg_نقاب = 0;
    سجل فراغ **p_argv;
    سجل ffi_type **p_arg;
    سجل حرف *reg_ادھار = ڈھیر;
    سجل حرف *ڈھیر_ادھار = ڈھیر + 20;
    سجل بنانشانی صحيح ڈھیر_وتھ = 0;

#جے_تعریف_شدہ DEBUG
    /* Debugging */
    لكل(i = 0; i < cif->nargs + 7; i++)
    {
        لکھو("sp+%d: 0x%08x\n", i*4, ((بنانشانی صحيح*)ڈھیر)[i]);
    }
#جے_انت

    /* If cif->flags is struct then we know it's not passed in registers */
    إذا(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(فراغ **)reg_ادھار;
        reg_نقاب |= 1;
    }

    p_argv = avalue;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->ناپ;
        صحيح alignment = (*p_arg)->alignment;

        *p_argv = 0;

        إذا(z % 4 != 0)
            z += (4 - z % 4);

        إذا(reg_نقاب != 0x1f)
        {
            إذا(pass_struct_on_ڈھیر(*p_arg))
            {
                *p_argv = (فراغ*)ڈھیر_ادھار + ڈھیر_وتھ;
                ڈھیر_وتھ += z;
            }
            وإلا إذا(z <= حجم(صحيح))
            {
                حرف اشاریہ = 0;

                طالما((reg_نقاب >> اشاریہ) & 1)
                    اشاریہ++;

                *p_argv = (فراغ*)reg_ادھار + (اشاریہ * 4);
                reg_نقاب |= (1 << اشاریہ);
            }
            وإلا إذا(z == 2 * حجم(صحيح))
            {
                إذا(!((reg_نقاب >> 1) & 1))
                {
                    *p_argv = (فراغ*)reg_ادھار + 4;
                    reg_نقاب |= (3 << 1);
                }
                وإلا إذا(!((reg_نقاب >> 3) & 1))
                {
                    *p_argv = (فراغ*)reg_ادھار + 12;
                    reg_نقاب |= (3 << 3);
                }
            }
        }

        إذا(!*p_argv)
        {
            *p_argv = (فراغ*)ڈھیر_ادھار + ڈھیر_وتھ;
            ڈھیر_وتھ += z;
        }

        إذا((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == عدم)
        {
            إذا(alignment == 1)
                **(بنانشانی صحيح**)p_argv <<= 24;
            وإلا إذا(alignment == 2)
                **(بنانشانی صحيح**)p_argv <<= 16;
        }

        p_argv++;
    }

#جے_تعریف_شدہ DEBUG
    /* Debugging */
    لكل(i = 0; i < cif->nargs; i++)
    {
        لکھو("sp+%d: 0x%08x\n", i*4, *(((بنانشانی صحيح**)avalue)[i]));
    }
#جے_انت
}

/* This function is jumped to by the trampoline */

بنانشانی صحيح ffi_گھیرا_SYSV_inner(ffi_گھیرا *گھیرا, فراغ **respp,
    فراغ *دلیلاں)
{
    ffi_cif *cif;
    فراغ **arg_area;
    بنانشانی صحيح i, ناپ = 0;
    ffi_type **p_arg;

    cif = گھیرا->cif;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        ناپ += (*p_arg)->ناپ + (4 - (*p_arg)->ناپ % 4);

    arg_area = (فراغ **)alloca(ناپ);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_دلیلاں_SYSV(دلیلاں, respp, arg_area, cif);

    (گھیرا->fun)(cif, *respp, arg_area, گھیرا->user_انکڑے);

    أرجع cif->flags;
}

ffi_حالت ffi_prep_گھیرا_loc(ffi_گھیرا* گھیرا, ffi_cif* cif,
    فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*), فراغ *user_انکڑے,
    فراغ *codeloc)
{
    إذا (cif->abi != FFI_SYSV)
      أرجع FFI_BAD_ABI;

    بنانشانی حرف *__tramp = (بنانشانی حرف*)(&گھیرا->tramp[0]);
    بنانشانی صحيح  __fun = (بنانشانی صحيح)(&ffi_گھیرا_SYSV);
    بنانشانی صحيح  __ctx = (بنانشانی صحيح)(codeloc);
    بنانشانی صحيح  __rstruct_flag = (بنانشانی صحيح)(cif->rstruct_flag);
    بنانشانی صحيح  __inner = (بنانشانی صحيح)(&ffi_گھیرا_SYSV_inner);
    *(بنانشانی صحيح*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(بنانشانی صحيح*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(بنانشانی صحيح*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(بنانشانی صحيح*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(بنانشانی صحيح*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(بنانشانی صحيح*) &__tramp[20] = __ctx;
    *(بنانشانی صحيح*) &__tramp[24] = __rstruct_flag;
    *(بنانشانی صحيح*) &__tramp[28] = __inner;
    *(بنانشانی صحيح*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    گھیرا->cif = cif;
    گھیرا->user_انکڑے = user_انکڑے;
    گھیرا->fun  = fun;

    أرجع FFI_OK;
}

