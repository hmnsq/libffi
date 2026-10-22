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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <asm/unistd.h>

/* #define DEBUG */

బాహ్య శూన్య ffi_call_SYSV(శూన్య (*)(అక్షరం *, extended_cif *), extended_cif *,
    గుర్తులేని పూర్ణాంకం, గుర్తులేని పూర్ణాంకం, గుర్తులేని పూర్ణాంకం*, గుర్తులేని పూర్ణాంకం,
    శూన్య (*fn)(శూన్య));
బాహ్య శూన్య ffi_సంవృతం_SYSV (ffi_సంవృతం *);

గుర్తులేని పూర్ణాంకం pass_struct_on_దొంతర(ffi_type *type)
{
    ఒకవేళ(type->type != FFI_TYPE_STRUCT)
        తిరిగి 0;

    ఒకవేళ(type->alignment < type->కొలత &&
        !(type->కొలత == 4 || type->కొలత == 8) &&
        !(type->కొలత == 8 && type->alignment >= 4))
        తిరిగి 1;

    ఒకవేళ(type->కొలత == 3 || type->కొలత == 5 || type->కొలత == 6 ||
        type->కొలత == 7)
        తిరిగి 1;

    తిరిగి 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

శూన్య ffi_prep_వాదనలు(అక్షరం *దొంతర, extended_cif *ecif)
{
    గుర్తులేని పూర్ణాంకం i;
    శూన్య **p_argv;
    ffi_type **p_arg;
    అక్షరం *reg_పునాది = దొంతర;
    అక్షరం *దొంతర_పునాది = దొంతర + 20;
    గుర్తులేని పూర్ణాంకం దొంతర_స్థానభ్రంశం = 0;
    గుర్తులేని పూర్ణాంకం reg_ముసుగు = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    ఒకవేళ(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(శూన్య**)reg_పునాది = ecif->rvalue;
        reg_ముసుగు |= 1;
    }

    కొరకు(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->కొలత;
        పూర్ణాంకం alignment = (*p_arg)->alignment;
        పూర్ణాంకం type = (*p_arg)->type;
        అక్షరం *addr = 0;

        ఒకవేళ(z % 4 != 0)
            z += (4 - z % 4);

        ఒకవేళ(reg_ముసుగు != 0x1f)
        {
            ఒకవేళ(pass_struct_on_దొంతర(*p_arg))
            {
                addr = దొంతర_పునాది + దొంతర_స్థానభ్రంశం;
                దొంతర_స్థానభ్రంశం += z;
            }
            లేకపోతే ఒకవేళ(z == పరిమాణం(పూర్ణాంకం))
            {
                అక్షరం సూచీ = 0;

                అయితే((reg_ముసుగు >> సూచీ) & 1)
                    సూచీ++;

                addr = reg_పునాది + (సూచీ * 4);
                reg_ముసుగు |= (1 << సూచీ);
            }
            లేకపోతే ఒకవేళ(z == 2 * పరిమాణం(పూర్ణాంకం))
            {
                ఒకవేళ(!((reg_ముసుగు >> 1) & 1))
                {
                    addr = reg_పునాది + 4;
                    reg_ముసుగు |= (3 << 1);
                }
                లేకపోతే ఒకవేళ(!((reg_ముసుగు >> 3) & 1))
                {
                    addr = reg_పునాది + 12;
                    reg_ముసుగు |= (3 << 3);
                }
            }
        }

        ఒకవేళ(!addr)
        {
            addr = దొంతర_పునాది + దొంతర_స్థానభ్రంశం;
            దొంతర_స్థానభ్రంశం += z;
        }

        ఒకవేళ(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == ఖాళీ)
            type = (*p_arg)->elements[0]->type;

        ఎంపిక(type)
        {
        స్థితి FFI_TYPE_UINT8:
            *(గుర్తులేని పూర్ణాంకం *)addr = (గుర్తులేని పూర్ణాంకం)*(UINT8 *)(*p_argv);
            ఆపు;
        స్థితి FFI_TYPE_SINT8:
            *(గుర్తుగల పూర్ణాంకం *)addr = (గుర్తుగల పూర్ణాంకం)*(SINT8 *)(*p_argv);
            ఆపు;
        స్థితి FFI_TYPE_UINT16:
            *(గుర్తులేని పూర్ణాంకం *)addr = (గుర్తులేని పూర్ణాంకం)*(UINT16 *)(*p_argv);
            ఆపు;
        స్థితి FFI_TYPE_SINT16:
            *(గుర్తుగల పూర్ణాంకం *)addr = (గుర్తుగల పూర్ణాంకం)*(SINT16 *)(*p_argv);
            ఆపు;
        అప్రమేయ:
            జ్ఞాపకం_నకలు(addr, *p_argv, z);
        }

        p_argv++;
    }

#నిర్వచితమైతే DEBUG
    /* Debugging */
    కొరకు(i = 0; i < 5; i++)
    {
        ఒకవేళ((reg_ముసుగు & (1 << i)) == 0)
            రాయి("r%d: (unused)\n", 12 - i);
        లేకపోతే
            రాయి("r%d: 0x%08x\n", 12 - i, ((గుర్తులేని పూర్ణాంకం*)reg_పునాది)[i]);
    }

    కొరకు(i = 0; i < దొంతర_స్థానభ్రంశం / 4; i++)
    {
        రాయి("sp+%d: 0x%08x\n", i*4, ((గుర్తులేని పూర్ణాంకం*)దొంతర_పునాది)[i]);
    }
#అయితే_ముగింపు
}

/* Perform machine dependent cif processing */
ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif)
{
    /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     * everywhere, but it is on some platforms, and it doesn't harm
     * anything when it isn't needed. */
    cif->బైట్లు = (cif->బైట్లు + 7) & ~7;

    /* Flag to indicate that he return value is in fact a struct */
    cif->rstruct_flag = 0;

    /* Set the return type flag */
    ఎంపిక(cif->rtype->type)
    {
    స్థితి FFI_TYPE_SINT8:
    స్థితి FFI_TYPE_UINT8:
        cif->flags = (గుర్తులేని)FFI_TYPE_UINT8;
        ఆపు;
    స్థితి FFI_TYPE_SINT16:
    స్థితి FFI_TYPE_UINT16:
        cif->flags = (గుర్తులేని)FFI_TYPE_UINT16;
        ఆపు;
    స్థితి FFI_TYPE_FLOAT:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_UINT32:
    స్థితి FFI_TYPE_సూచిక:
        cif->flags = (గుర్తులేని)FFI_TYPE_UINT32;
        ఆపు;
    స్థితి FFI_TYPE_DOUBLE:
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
        cif->flags = (గుర్తులేని)FFI_TYPE_UINT64;
        ఆపు;
    స్థితి FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        ఒకవేళ(!pass_struct_on_దొంతర(cif->rtype))
        {
            ఒకవేళ(cif->rtype->కొలత <= 1)
                cif->flags = (గుర్తులేని)FFI_TYPE_UINT8;
            లేకపోతే ఒకవేళ(cif->rtype->కొలత <= 2)
                cif->flags = (గుర్తులేని)FFI_TYPE_UINT16;
            లేకపోతే ఒకవేళ(cif->rtype->కొలత <= 4)
                cif->flags = (గుర్తులేని)FFI_TYPE_UINT32;
            లేకపోతే ఒకవేళ(cif->rtype->కొలత <= 8)
                cif->flags = (గుర్తులేని)FFI_TYPE_UINT64;
            లేకపోతే
                cif->flags = (గుర్తులేని)cif->rtype->type;
        }
        లేకపోతే
            cif->flags = (గుర్తులేని)cif->rtype->type;
        ఆపు;
    అప్రమేయ:
        cif->flags = (గుర్తులేని)cif->rtype->type;
        ఆపు;
    }

    తిరిగి FFI_OK;
}

శూన్య ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
    extended_cif ecif;

    గుర్తులేని పూర్ణాంకం కొలత = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    కొరకు(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        కొలత += (*p_arg)->కొలత + (4 - (*p_arg)->కొలత % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    ఒకవేళ((rvalue == ఖాళీ) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->కొలత);
    లేకపోతే
        ecif.rvalue = rvalue;

    ఎంపిక(cif->abi)
    {
    స్థితి FFI_SYSV:
        ffi_call_SYSV(ffi_prep_వాదనలు, &ecif, కొలత, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        ఆపు;
    అప్రమేయ:
        FFI_ASSERT(0);
        ఆపు;
    }
}

స్థిర శూన్య ffi_prep_incoming_వాదనలు_SYSV(అక్షరం *దొంతర, శూన్య **rvalue,
    శూన్య **avalue, ffi_cif *cif)
{
    నమోదు గుర్తులేని పూర్ణాంకం i, reg_ముసుగు = 0;
    నమోదు శూన్య **p_argv;
    నమోదు ffi_type **p_arg;
    నమోదు అక్షరం *reg_పునాది = దొంతర;
    నమోదు అక్షరం *దొంతర_పునాది = దొంతర + 20;
    నమోదు గుర్తులేని పూర్ణాంకం దొంతర_స్థానభ్రంశం = 0;

#నిర్వచితమైతే DEBUG
    /* Debugging */
    కొరకు(i = 0; i < cif->nargs + 7; i++)
    {
        రాయి("sp+%d: 0x%08x\n", i*4, ((గుర్తులేని పూర్ణాంకం*)దొంతర)[i]);
    }
#అయితే_ముగింపు

    /* If cif->flags is struct then we know it's not passed in registers */
    ఒకవేళ(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(శూన్య **)reg_పునాది;
        reg_ముసుగు |= 1;
    }

    p_argv = avalue;

    కొరకు(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->కొలత;
        పూర్ణాంకం alignment = (*p_arg)->alignment;

        *p_argv = 0;

        ఒకవేళ(z % 4 != 0)
            z += (4 - z % 4);

        ఒకవేళ(reg_ముసుగు != 0x1f)
        {
            ఒకవేళ(pass_struct_on_దొంతర(*p_arg))
            {
                *p_argv = (శూన్య*)దొంతర_పునాది + దొంతర_స్థానభ్రంశం;
                దొంతర_స్థానభ్రంశం += z;
            }
            లేకపోతే ఒకవేళ(z <= పరిమాణం(పూర్ణాంకం))
            {
                అక్షరం సూచీ = 0;

                అయితే((reg_ముసుగు >> సూచీ) & 1)
                    సూచీ++;

                *p_argv = (శూన్య*)reg_పునాది + (సూచీ * 4);
                reg_ముసుగు |= (1 << సూచీ);
            }
            లేకపోతే ఒకవేళ(z == 2 * పరిమాణం(పూర్ణాంకం))
            {
                ఒకవేళ(!((reg_ముసుగు >> 1) & 1))
                {
                    *p_argv = (శూన్య*)reg_పునాది + 4;
                    reg_ముసుగు |= (3 << 1);
                }
                లేకపోతే ఒకవేళ(!((reg_ముసుగు >> 3) & 1))
                {
                    *p_argv = (శూన్య*)reg_పునాది + 12;
                    reg_ముసుగు |= (3 << 3);
                }
            }
        }

        ఒకవేళ(!*p_argv)
        {
            *p_argv = (శూన్య*)దొంతర_పునాది + దొంతర_స్థానభ్రంశం;
            దొంతర_స్థానభ్రంశం += z;
        }

        ఒకవేళ((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == ఖాళీ)
        {
            ఒకవేళ(alignment == 1)
                **(గుర్తులేని పూర్ణాంకం**)p_argv <<= 24;
            లేకపోతే ఒకవేళ(alignment == 2)
                **(గుర్తులేని పూర్ణాంకం**)p_argv <<= 16;
        }

        p_argv++;
    }

#నిర్వచితమైతే DEBUG
    /* Debugging */
    కొరకు(i = 0; i < cif->nargs; i++)
    {
        రాయి("sp+%d: 0x%08x\n", i*4, *(((గుర్తులేని పూర్ణాంకం**)avalue)[i]));
    }
#అయితే_ముగింపు
}

/* This function is jumped to by the trampoline */

గుర్తులేని పూర్ణాంకం ffi_సంవృతం_SYSV_inner(ffi_సంవృతం *సంవృతం, శూన్య **respp,
    శూన్య *వాదనలు)
{
    ffi_cif *cif;
    శూన్య **arg_area;
    గుర్తులేని పూర్ణాంకం i, కొలత = 0;
    ffi_type **p_arg;

    cif = సంవృతం->cif;

    కొరకు(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        కొలత += (*p_arg)->కొలత + (4 - (*p_arg)->కొలత % 4);

    arg_area = (శూన్య **)alloca(కొలత);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_వాదనలు_SYSV(వాదనలు, respp, arg_area, cif);

    (సంవృతం->fun)(cif, *respp, arg_area, సంవృతం->user_దత్తాంశం);

    తిరిగి cif->flags;
}

ffi_స్థితి ffi_prep_సంవృతం_loc(ffi_సంవృతం* సంవృతం, ffi_cif* cif,
    శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*), శూన్య *user_దత్తాంశం,
    శూన్య *codeloc)
{
    ఒకవేళ (cif->abi != FFI_SYSV)
      తిరిగి FFI_BAD_ABI;

    గుర్తులేని అక్షరం *__tramp = (గుర్తులేని అక్షరం*)(&సంవృతం->tramp[0]);
    గుర్తులేని పూర్ణాంకం  __fun = (గుర్తులేని పూర్ణాంకం)(&ffi_సంవృతం_SYSV);
    గుర్తులేని పూర్ణాంకం  __ctx = (గుర్తులేని పూర్ణాంకం)(codeloc);
    గుర్తులేని పూర్ణాంకం  __rstruct_flag = (గుర్తులేని పూర్ణాంకం)(cif->rstruct_flag);
    గుర్తులేని పూర్ణాంకం  __inner = (గుర్తులేని పూర్ణాంకం)(&ffi_సంవృతం_SYSV_inner);
    *(గుర్తులేని పూర్ణాంకం*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(గుర్తులేని పూర్ణాంకం*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(గుర్తులేని పూర్ణాంకం*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(గుర్తులేని పూర్ణాంకం*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(గుర్తులేని పూర్ణాంకం*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(గుర్తులేని పూర్ణాంకం*) &__tramp[20] = __ctx;
    *(గుర్తులేని పూర్ణాంకం*) &__tramp[24] = __rstruct_flag;
    *(గుర్తులేని పూర్ణాంకం*) &__tramp[28] = __inner;
    *(గుర్తులేని పూర్ణాంకం*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    సంవృతం->cif = cif;
    సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
    సంవృతం->fun  = fun;

    తిరిగి FFI_OK;
}

