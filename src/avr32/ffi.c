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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <asm/unistd.h>

/* #define DEBUG */

বাহ্যিক শূন্য ffi_call_SYSV(শূন্য (*)(অক্ষর *, extended_cif *), extended_cif *,
    unsigned পূর্ণসংখ্যা, unsigned পূর্ণসংখ্যা, unsigned পূর্ণসংখ্যা*, unsigned পূর্ণসংখ্যা,
    শূন্য (*fn)(শূন্য));
বাহ্যিক শূন্য ffi_closure_SYSV (ffi_closure *);

unsigned পূর্ণসংখ্যা pass_struct_on_স্তূপ(ffi_type *type)
{
    যদি(type->type != FFI_TYPE_STRUCT)
        ফেরত 0;

    যদি(type->alignment < type->আকার_2 &&
        !(type->আকার_2 == 4 || type->আকার_2 == 8) &&
        !(type->আকার_2 == 8 && type->alignment >= 4))
        ফেরত 1;

    যদি(type->আকার_2 == 3 || type->আকার_2 == 5 || type->আকার_2 == 6 ||
        type->আকার_2 == 7)
        ফেরত 1;

    ফেরত 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

শূন্য ffi_prep_args(অক্ষর *স্তূপ, extended_cif *ecif)
{
    unsigned পূর্ণসংখ্যা i;
    শূন্য **p_argv;
    ffi_type **p_arg;
    অক্ষর *reg_base = স্তূপ;
    অক্ষর *স্তূপ_base = স্তূপ + 20;
    unsigned পূর্ণসংখ্যা স্তূপ_offset = 0;
    unsigned পূর্ণসংখ্যা reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    যদি(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(শূন্য**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    জন্য(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->আকার_2;
        পূর্ণসংখ্যা alignment = (*p_arg)->alignment;
        পূর্ণসংখ্যা type = (*p_arg)->type;
        অক্ষর *addr = 0;

        যদি(z % 4 != 0)
            z += (4 - z % 4);

        যদি(reg_mask != 0x1f)
        {
            যদি(pass_struct_on_স্তূপ(*p_arg))
            {
                addr = স্তূপ_base + স্তূপ_offset;
                স্তূপ_offset += z;
            }
            অন্যথা যদি(z == আকার(পূর্ণসংখ্যা))
            {
                অক্ষর সূচক = 0;

                যখন((reg_mask >> সূচক) & 1)
                    সূচক++;

                addr = reg_base + (সূচক * 4);
                reg_mask |= (1 << সূচক);
            }
            অন্যথা যদি(z == 2 * আকার(পূর্ণসংখ্যা))
            {
                যদি(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                অন্যথা যদি(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        যদি(!addr)
        {
            addr = স্তূপ_base + স্তূপ_offset;
            স্তূপ_offset += z;
        }

        যদি(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == ফাঁকা)
            type = (*p_arg)->elements[0]->type;

        নির্বাচন(type)
        {
        ক্ষেত্র FFI_TYPE_UINT8:
            *(unsigned পূর্ণসংখ্যা *)addr = (unsigned পূর্ণসংখ্যা)*(UINT8 *)(*p_argv);
            বিরতি;
        ক্ষেত্র FFI_TYPE_SINT8:
            *(signed পূর্ণসংখ্যা *)addr = (signed পূর্ণসংখ্যা)*(SINT8 *)(*p_argv);
            বিরতি;
        ক্ষেত্র FFI_TYPE_UINT16:
            *(unsigned পূর্ণসংখ্যা *)addr = (unsigned পূর্ণসংখ্যা)*(UINT16 *)(*p_argv);
            বিরতি;
        ক্ষেত্র FFI_TYPE_SINT16:
            *(signed পূর্ণসংখ্যা *)addr = (signed পূর্ণসংখ্যা)*(SINT16 *)(*p_argv);
            বিরতি;
        পূর্বনির্ধারিত:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    জন্য(i = 0; i < 5; i++)
    {
        যদি((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        অন্যথা
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned পূর্ণসংখ্যা*)reg_base)[i]);
    }

    জন্য(i = 0; i < স্তূপ_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned পূর্ণসংখ্যা*)স্তূপ_base)[i]);
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
    নির্বাচন(cif->rtype->type)
    {
    ক্ষেত্র FFI_TYPE_SINT8:
    ক্ষেত্র FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        বিরতি;
    ক্ষেত্র FFI_TYPE_SINT16:
    ক্ষেত্র FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        বিরতি;
    ক্ষেত্র FFI_TYPE_FLOAT:
    ক্ষেত্র FFI_TYPE_SINT32:
    ক্ষেত্র FFI_TYPE_UINT32:
    ক্ষেত্র FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        বিরতি;
    ক্ষেত্র FFI_TYPE_DOUBLE:
    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        বিরতি;
    ক্ষেত্র FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        যদি(!pass_struct_on_স্তূপ(cif->rtype))
        {
            যদি(cif->rtype->আকার_2 <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            অন্যথা যদি(cif->rtype->আকার_2 <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            অন্যথা যদি(cif->rtype->আকার_2 <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            অন্যথা যদি(cif->rtype->আকার_2 <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            অন্যথা
                cif->flags = (unsigned)cif->rtype->type;
        }
        অন্যথা
            cif->flags = (unsigned)cif->rtype->type;
        বিরতি;
    পূর্বনির্ধারিত:
        cif->flags = (unsigned)cif->rtype->type;
        বিরতি;
    }

    ফেরত FFI_OK;
}

শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
    extended_cif ecif;

    unsigned পূর্ণসংখ্যা আকার_2 = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    জন্য(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        আকার_2 += (*p_arg)->আকার_2 + (4 - (*p_arg)->আকার_2 % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    যদি((rvalue == ফাঁকা) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->আকার_2);
    অন্যথা
        ecif.rvalue = rvalue;

    নির্বাচন(cif->abi)
    {
    ক্ষেত্র FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, আকার_2, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        বিরতি;
    পূর্বনির্ধারিত:
        FFI_ASSERT(0);
        বিরতি;
    }
}

স্থির শূন্য ffi_prep_incoming_args_SYSV(অক্ষর *স্তূপ, শূন্য **rvalue,
    শূন্য **avalue, ffi_cif *cif)
{
    নিবন্ধ unsigned পূর্ণসংখ্যা i, reg_mask = 0;
    নিবন্ধ শূন্য **p_argv;
    নিবন্ধ ffi_type **p_arg;
    নিবন্ধ অক্ষর *reg_base = স্তূপ;
    নিবন্ধ অক্ষর *স্তূপ_base = স্তূপ + 20;
    নিবন্ধ unsigned পূর্ণসংখ্যা স্তূপ_offset = 0;

#ifdef DEBUG
    /* Debugging */
    জন্য(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned পূর্ণসংখ্যা*)স্তূপ)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    যদি(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(শূন্য **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    জন্য(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->আকার_2;
        পূর্ণসংখ্যা alignment = (*p_arg)->alignment;

        *p_argv = 0;

        যদি(z % 4 != 0)
            z += (4 - z % 4);

        যদি(reg_mask != 0x1f)
        {
            যদি(pass_struct_on_স্তূপ(*p_arg))
            {
                *p_argv = (শূন্য*)স্তূপ_base + স্তূপ_offset;
                স্তূপ_offset += z;
            }
            অন্যথা যদি(z <= আকার(পূর্ণসংখ্যা))
            {
                অক্ষর সূচক = 0;

                যখন((reg_mask >> সূচক) & 1)
                    সূচক++;

                *p_argv = (শূন্য*)reg_base + (সূচক * 4);
                reg_mask |= (1 << সূচক);
            }
            অন্যথা যদি(z == 2 * আকার(পূর্ণসংখ্যা))
            {
                যদি(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (শূন্য*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                অন্যথা যদি(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (শূন্য*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        যদি(!*p_argv)
        {
            *p_argv = (শূন্য*)স্তূপ_base + স্তূপ_offset;
            স্তূপ_offset += z;
        }

        যদি((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == ফাঁকা)
        {
            যদি(alignment == 1)
                **(unsigned পূর্ণসংখ্যা**)p_argv <<= 24;
            অন্যথা যদি(alignment == 2)
                **(unsigned পূর্ণসংখ্যা**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    জন্য(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned পূর্ণসংখ্যা**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned পূর্ণসংখ্যা ffi_closure_SYSV_inner(ffi_closure *closure, শূন্য **respp,
    শূন্য *args)
{
    ffi_cif *cif;
    শূন্য **arg_area;
    unsigned পূর্ণসংখ্যা i, আকার_2 = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    জন্য(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        আকার_2 += (*p_arg)->আকার_2 + (4 - (*p_arg)->আকার_2 % 4);

    arg_area = (শূন্য **)alloca(আকার_2);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_তথ্য);

    ফেরত cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*), শূন্য *user_তথ্য,
    শূন্য *codeloc)
{
    যদি (cif->abi != FFI_SYSV)
      ফেরত FFI_BAD_ABI;

    unsigned অক্ষর *__tramp = (unsigned অক্ষর*)(&closure->tramp[0]);
    unsigned পূর্ণসংখ্যা  __fun = (unsigned পূর্ণসংখ্যা)(&ffi_closure_SYSV);
    unsigned পূর্ণসংখ্যা  __ctx = (unsigned পূর্ণসংখ্যা)(codeloc);
    unsigned পূর্ণসংখ্যা  __rstruct_flag = (unsigned পূর্ণসংখ্যা)(cif->rstruct_flag);
    unsigned পূর্ণসংখ্যা  __inner = (unsigned পূর্ণসংখ্যা)(&ffi_closure_SYSV_inner);
    *(unsigned পূর্ণসংখ্যা*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned পূর্ণসংখ্যা*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned পূর্ণসংখ্যা*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned পূর্ণসংখ্যা*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned পূর্ণসংখ্যা*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned পূর্ণসংখ্যা*) &__tramp[20] = __ctx;
    *(unsigned পূর্ণসংখ্যা*) &__tramp[24] = __rstruct_flag;
    *(unsigned পূর্ণসংখ্যা*) &__tramp[28] = __inner;
    *(unsigned পূর্ণসংখ্যা*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_তথ্য = user_তথ্য;
    closure->fun  = fun;

    ফেরত FFI_OK;
}

