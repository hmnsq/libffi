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
    بنانشان صحيح, بنانشان صحيح, بنانشان صحيح*, بنانشان صحيح,
    فراغ (*fn)(فراغ));
خارجي فراغ ffi_گهيرو_SYSV (ffi_گهيرو *);

بنانشان صحيح pass_struct_on_ڍير(ffi_type *type)
{
    إذا(type->type != FFI_TYPE_STRUCT)
        أرجع 0;

    إذا(type->alignment < type->سائيز &&
        !(type->سائيز == 4 || type->سائيز == 8) &&
        !(type->سائيز == 8 && type->alignment >= 4))
        أرجع 1;

    إذا(type->سائيز == 3 || type->سائيز == 5 || type->سائيز == 6 ||
        type->سائيز == 7)
        أرجع 1;

    أرجع 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

فراغ ffi_prep_دليلون(حرف *ڍير, extended_cif *ecif)
{
    بنانشان صحيح i;
    فراغ **p_argv;
    ffi_type **p_arg;
    حرف *reg_بنياد = ڍير;
    حرف *ڍير_بنياد = ڍير + 20;
    بنانشان صحيح ڍير_وٿي = 0;
    بنانشان صحيح reg_نقاب = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    إذا(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(فراغ**)reg_بنياد = ecif->rvalue;
        reg_نقاب |= 1;
    }

    لكل(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->سائيز;
        صحيح alignment = (*p_arg)->alignment;
        صحيح type = (*p_arg)->type;
        حرف *addr = 0;

        إذا(z % 4 != 0)
            z += (4 - z % 4);

        إذا(reg_نقاب != 0x1f)
        {
            إذا(pass_struct_on_ڍير(*p_arg))
            {
                addr = ڍير_بنياد + ڍير_وٿي;
                ڍير_وٿي += z;
            }
            وإلا إذا(z == حجم(صحيح))
            {
                حرف اشاريو = 0;

                طالما((reg_نقاب >> اشاريو) & 1)
                    اشاريو++;

                addr = reg_بنياد + (اشاريو * 4);
                reg_نقاب |= (1 << اشاريو);
            }
            وإلا إذا(z == 2 * حجم(صحيح))
            {
                إذا(!((reg_نقاب >> 1) & 1))
                {
                    addr = reg_بنياد + 4;
                    reg_نقاب |= (3 << 1);
                }
                وإلا إذا(!((reg_نقاب >> 3) & 1))
                {
                    addr = reg_بنياد + 12;
                    reg_نقاب |= (3 << 3);
                }
            }
        }

        إذا(!addr)
        {
            addr = ڍير_بنياد + ڍير_وٿي;
            ڍير_وٿي += z;
        }

        إذا(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == عدم)
            type = (*p_arg)->elements[0]->type;

        بدل(type)
        {
        حالة FFI_TYPE_UINT8:
            *(بنانشان صحيح *)addr = (بنانشان صحيح)*(UINT8 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_SINT8:
            *(نشانوارو صحيح *)addr = (نشانوارو صحيح)*(SINT8 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_UINT16:
            *(بنانشان صحيح *)addr = (بنانشان صحيح)*(UINT16 *)(*p_argv);
            اكسر;
        حالة FFI_TYPE_SINT16:
            *(نشانوارو صحيح *)addr = (نشانوارو صحيح)*(SINT16 *)(*p_argv);
            اكسر;
        افتراضي:
            يادگيري_نقل(addr, *p_argv, z);
        }

        p_argv++;
    }

#جيڪڏهن_وصف_ٿيل DEBUG
    /* Debugging */
    لكل(i = 0; i < 5; i++)
    {
        إذا((reg_نقاب & (1 << i)) == 0)
            لکو("r%d: (unused)\n", 12 - i);
        وإلا
            لکو("r%d: 0x%08x\n", 12 - i, ((بنانشان صحيح*)reg_بنياد)[i]);
    }

    لكل(i = 0; i < ڍير_وٿي / 4; i++)
    {
        لکو("sp+%d: 0x%08x\n", i*4, ((بنانشان صحيح*)ڍير_بنياد)[i]);
    }
#جيڪڏهن_پورو
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
    /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     * everywhere, but it is on some platforms, and it doesn't harm
     * anything when it isn't needed. */
    cif->بائيٽون = (cif->بائيٽون + 7) & ~7;

    /* Flag to indicate that he return value is in fact a struct */
    cif->rstruct_flag = 0;

    /* Set the return type flag */
    بدل(cif->rtype->type)
    {
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT8:
        cif->flags = (بنانشان)FFI_TYPE_UINT8;
        اكسر;
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT16:
        cif->flags = (بنانشان)FFI_TYPE_UINT16;
        اكسر;
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_اشارو:
        cif->flags = (بنانشان)FFI_TYPE_UINT32;
        اكسر;
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
        cif->flags = (بنانشان)FFI_TYPE_UINT64;
        اكسر;
    حالة FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        إذا(!pass_struct_on_ڍير(cif->rtype))
        {
            إذا(cif->rtype->سائيز <= 1)
                cif->flags = (بنانشان)FFI_TYPE_UINT8;
            وإلا إذا(cif->rtype->سائيز <= 2)
                cif->flags = (بنانشان)FFI_TYPE_UINT16;
            وإلا إذا(cif->rtype->سائيز <= 4)
                cif->flags = (بنانشان)FFI_TYPE_UINT32;
            وإلا إذا(cif->rtype->سائيز <= 8)
                cif->flags = (بنانشان)FFI_TYPE_UINT64;
            وإلا
                cif->flags = (بنانشان)cif->rtype->type;
        }
        وإلا
            cif->flags = (بنانشان)cif->rtype->type;
        اكسر;
    افتراضي:
        cif->flags = (بنانشان)cif->rtype->type;
        اكسر;
    }

    أرجع FFI_OK;
}

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
    extended_cif ecif;

    بنانشان صحيح سائيز = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        سائيز += (*p_arg)->سائيز + (4 - (*p_arg)->سائيز % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    إذا((rvalue == عدم) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->سائيز);
    وإلا
        ecif.rvalue = rvalue;

    بدل(cif->abi)
    {
    حالة FFI_SYSV:
        ffi_call_SYSV(ffi_prep_دليلون, &ecif, سائيز, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        اكسر;
    افتراضي:
        FFI_ASSERT(0);
        اكسر;
    }
}

ساكن فراغ ffi_prep_incoming_دليلون_SYSV(حرف *ڍير, فراغ **rvalue,
    فراغ **avalue, ffi_cif *cif)
{
    سجل بنانشان صحيح i, reg_نقاب = 0;
    سجل فراغ **p_argv;
    سجل ffi_type **p_arg;
    سجل حرف *reg_بنياد = ڍير;
    سجل حرف *ڍير_بنياد = ڍير + 20;
    سجل بنانشان صحيح ڍير_وٿي = 0;

#جيڪڏهن_وصف_ٿيل DEBUG
    /* Debugging */
    لكل(i = 0; i < cif->nargs + 7; i++)
    {
        لکو("sp+%d: 0x%08x\n", i*4, ((بنانشان صحيح*)ڍير)[i]);
    }
#جيڪڏهن_پورو

    /* If cif->flags is struct then we know it's not passed in registers */
    إذا(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(فراغ **)reg_بنياد;
        reg_نقاب |= 1;
    }

    p_argv = avalue;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->سائيز;
        صحيح alignment = (*p_arg)->alignment;

        *p_argv = 0;

        إذا(z % 4 != 0)
            z += (4 - z % 4);

        إذا(reg_نقاب != 0x1f)
        {
            إذا(pass_struct_on_ڍير(*p_arg))
            {
                *p_argv = (فراغ*)ڍير_بنياد + ڍير_وٿي;
                ڍير_وٿي += z;
            }
            وإلا إذا(z <= حجم(صحيح))
            {
                حرف اشاريو = 0;

                طالما((reg_نقاب >> اشاريو) & 1)
                    اشاريو++;

                *p_argv = (فراغ*)reg_بنياد + (اشاريو * 4);
                reg_نقاب |= (1 << اشاريو);
            }
            وإلا إذا(z == 2 * حجم(صحيح))
            {
                إذا(!((reg_نقاب >> 1) & 1))
                {
                    *p_argv = (فراغ*)reg_بنياد + 4;
                    reg_نقاب |= (3 << 1);
                }
                وإلا إذا(!((reg_نقاب >> 3) & 1))
                {
                    *p_argv = (فراغ*)reg_بنياد + 12;
                    reg_نقاب |= (3 << 3);
                }
            }
        }

        إذا(!*p_argv)
        {
            *p_argv = (فراغ*)ڍير_بنياد + ڍير_وٿي;
            ڍير_وٿي += z;
        }

        إذا((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == عدم)
        {
            إذا(alignment == 1)
                **(بنانشان صحيح**)p_argv <<= 24;
            وإلا إذا(alignment == 2)
                **(بنانشان صحيح**)p_argv <<= 16;
        }

        p_argv++;
    }

#جيڪڏهن_وصف_ٿيل DEBUG
    /* Debugging */
    لكل(i = 0; i < cif->nargs; i++)
    {
        لکو("sp+%d: 0x%08x\n", i*4, *(((بنانشان صحيح**)avalue)[i]));
    }
#جيڪڏهن_پورو
}

/* This function is jumped to by the trampoline */

بنانشان صحيح ffi_گهيرو_SYSV_inner(ffi_گهيرو *گهيرو, فراغ **respp,
    فراغ *دليلون)
{
    ffi_cif *cif;
    فراغ **arg_area;
    بنانشان صحيح i, سائيز = 0;
    ffi_type **p_arg;

    cif = گهيرو->cif;

    لكل(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        سائيز += (*p_arg)->سائيز + (4 - (*p_arg)->سائيز % 4);

    arg_area = (فراغ **)alloca(سائيز);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_دليلون_SYSV(دليلون, respp, arg_area, cif);

    (گهيرو->fun)(cif, *respp, arg_area, گهيرو->user_معلومات);

    أرجع cif->flags;
}

ffi_حالت ffi_prep_گهيرو_loc(ffi_گهيرو* گهيرو, ffi_cif* cif,
    فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*), فراغ *user_معلومات,
    فراغ *codeloc)
{
    إذا (cif->abi != FFI_SYSV)
      أرجع FFI_BAD_ABI;

    بنانشان حرف *__tramp = (بنانشان حرف*)(&گهيرو->tramp[0]);
    بنانشان صحيح  __fun = (بنانشان صحيح)(&ffi_گهيرو_SYSV);
    بنانشان صحيح  __ctx = (بنانشان صحيح)(codeloc);
    بنانشان صحيح  __rstruct_flag = (بنانشان صحيح)(cif->rstruct_flag);
    بنانشان صحيح  __inner = (بنانشان صحيح)(&ffi_گهيرو_SYSV_inner);
    *(بنانشان صحيح*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(بنانشان صحيح*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(بنانشان صحيح*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(بنانشان صحيح*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(بنانشان صحيح*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(بنانشان صحيح*) &__tramp[20] = __ctx;
    *(بنانشان صحيح*) &__tramp[24] = __rstruct_flag;
    *(بنانشان صحيح*) &__tramp[28] = __inner;
    *(بنانشان صحيح*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    گهيرو->cif = cif;
    گهيرو->user_معلومات = user_معلومات;
    گهيرو->fun  = fun;

    أرجع FFI_OK;
}

