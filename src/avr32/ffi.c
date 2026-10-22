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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <asm/unistd.h>

/* #define DEBUG */

বাহ্য শূন্য ffi_call_SYSV(শূন্য (*)(ৱর্ণ *, extended_cif *), extended_cif *,
    unsigned পূর্ণাঙ্ক, unsigned পূর্ণাঙ্ক, unsigned পূর্ণাঙ্ক*, unsigned পূর্ণাঙ্ক,
    শূন্য (*fn)(শূন্য));
বাহ্য শূন্য ffi_closure_SYSV (ffi_closure *);

unsigned পূর্ণাঙ্ক pass_struct_on_স্তূপ(ffi_type *type)
{
    যদি(type->type != FFI_TYPE_STRUCT)
        ঘূৰাই 0;

    যদি(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        ঘূৰাই 1;

    যদি(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        ঘূৰাই 1;

    ঘূৰাই 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

শূন্য ffi_prep_args(ৱর্ণ *স্তূপ, extended_cif *ecif)
{
    unsigned পূর্ণাঙ্ক i;
    শূন্য **p_argv;
    ffi_type **p_arg;
    ৱর্ণ *reg_base = স্তূপ;
    ৱর্ণ *স্তূপ_base = স্তূপ + 20;
    unsigned পূর্ণাঙ্ক স্তূপ_offset = 0;
    unsigned পূর্ণাঙ্ক reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    যদি(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(শূন্য**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    বাবে(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        পূর্ণাঙ্ক alignment = (*p_arg)->alignment;
        পূর্ণাঙ্ক type = (*p_arg)->type;
        ৱর্ণ *addr = 0;

        যদি(z % 4 != 0)
            z += (4 - z % 4);

        যদি(reg_mask != 0x1f)
        {
            যদি(pass_struct_on_স্তূপ(*p_arg))
            {
                addr = স্তূপ_base + স্তূপ_offset;
                স্তূপ_offset += z;
            }
            অন্যথা যদি(z == পরিমাণ(পূর্ণাঙ্ক))
            {
                ৱর্ণ index = 0;

                যেতিয়ালৈ((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            অন্যথা যদি(z == 2 * পরিমাণ(পূর্ণাঙ্ক))
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

        যদি(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == রিক্ত)
            type = (*p_arg)->elements[0]->type;

        ৱিকল্প(type)
        {
        পক্ষ FFI_TYPE_UINT8:
            *(unsigned পূর্ণাঙ্ক *)addr = (unsigned পূর্ণাঙ্ক)*(UINT8 *)(*p_argv);
            ভঙ্গ;
        পক্ষ FFI_TYPE_SINT8:
            *(signed পূর্ণাঙ্ক *)addr = (signed পূর্ণাঙ্ক)*(SINT8 *)(*p_argv);
            ভঙ্গ;
        পক্ষ FFI_TYPE_UINT16:
            *(unsigned পূর্ণাঙ্ক *)addr = (unsigned পূর্ণাঙ্ক)*(UINT16 *)(*p_argv);
            ভঙ্গ;
        পক্ষ FFI_TYPE_SINT16:
            *(signed পূর্ণাঙ্ক *)addr = (signed পূর্ণাঙ্ক)*(SINT16 *)(*p_argv);
            ভঙ্গ;
        স্ৱভাৱ:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    বাবে(i = 0; i < 5; i++)
    {
        যদি((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        অন্যথা
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned পূর্ণাঙ্ক*)reg_base)[i]);
    }

    বাবে(i = 0; i < স্তূপ_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned পূর্ণাঙ্ক*)স্তূপ_base)[i]);
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
    ৱিকল্প(cif->rtype->type)
    {
    পক্ষ FFI_TYPE_SINT8:
    পক্ষ FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        ভঙ্গ;
    পক্ষ FFI_TYPE_SINT16:
    পক্ষ FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        ভঙ্গ;
    পক্ষ FFI_TYPE_FLOAT:
    পক্ষ FFI_TYPE_SINT32:
    পক্ষ FFI_TYPE_UINT32:
    পক্ষ FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        ভঙ্গ;
    পক্ষ FFI_TYPE_DOUBLE:
    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        ভঙ্গ;
    পক্ষ FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        যদি(!pass_struct_on_স্তূপ(cif->rtype))
        {
            যদি(cif->rtype->size <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            অন্যথা যদি(cif->rtype->size <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            অন্যথা যদি(cif->rtype->size <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            অন্যথা যদি(cif->rtype->size <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            অন্যথা
                cif->flags = (unsigned)cif->rtype->type;
        }
        অন্যথা
            cif->flags = (unsigned)cif->rtype->type;
        ভঙ্গ;
    স্ৱভাৱ:
        cif->flags = (unsigned)cif->rtype->type;
        ভঙ্গ;
    }

    ঘূৰাই FFI_OK;
}

শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
    extended_cif ecif;

    unsigned পূর্ণাঙ্ক size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    বাবে(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    যদি((rvalue == রিক্ত) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    অন্যথা
        ecif.rvalue = rvalue;

    ৱিকল্প(cif->abi)
    {
    পক্ষ FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        ভঙ্গ;
    স্ৱভাৱ:
        FFI_ASSERT(0);
        ভঙ্গ;
    }
}

স্থির শূন্য ffi_prep_incoming_args_SYSV(ৱর্ণ *স্তূপ, শূন্য **rvalue,
    শূন্য **avalue, ffi_cif *cif)
{
    পঞ্জিকা unsigned পূর্ণাঙ্ক i, reg_mask = 0;
    পঞ্জিকা শূন্য **p_argv;
    পঞ্জিকা ffi_type **p_arg;
    পঞ্জিকা ৱর্ণ *reg_base = স্তূপ;
    পঞ্জিকা ৱর্ণ *স্তূপ_base = স্তূপ + 20;
    পঞ্জিকা unsigned পূর্ণাঙ্ক স্তূপ_offset = 0;

#ifdef DEBUG
    /* Debugging */
    বাবে(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned পূর্ণাঙ্ক*)স্তূপ)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    যদি(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(শূন্য **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    বাবে(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        পূর্ণাঙ্ক alignment = (*p_arg)->alignment;

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
            অন্যথা যদি(z <= পরিমাণ(পূর্ণাঙ্ক))
            {
                ৱর্ণ index = 0;

                যেতিয়ালৈ((reg_mask >> index) & 1)
                    index++;

                *p_argv = (শূন্য*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            অন্যথা যদি(z == 2 * পরিমাণ(পূর্ণাঙ্ক))
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
            (*p_arg)->elements[1] == রিক্ত)
        {
            যদি(alignment == 1)
                **(unsigned পূর্ণাঙ্ক**)p_argv <<= 24;
            অন্যথা যদি(alignment == 2)
                **(unsigned পূর্ণাঙ্ক**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    বাবে(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned পূর্ণাঙ্ক**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned পূর্ণাঙ্ক ffi_closure_SYSV_inner(ffi_closure *closure, শূন্য **respp,
    শূন্য *args)
{
    ffi_cif *cif;
    শূন্য **arg_area;
    unsigned পূর্ণাঙ্ক i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    বাবে(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (শূন্য **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    ঘূৰাই cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*), শূন্য *user_data,
    শূন্য *codeloc)
{
    যদি (cif->abi != FFI_SYSV)
      ঘূৰাই FFI_BAD_ABI;

    unsigned ৱর্ণ *__tramp = (unsigned ৱর্ণ*)(&closure->tramp[0]);
    unsigned পূর্ণাঙ্ক  __fun = (unsigned পূর্ণাঙ্ক)(&ffi_closure_SYSV);
    unsigned পূর্ণাঙ্ক  __ctx = (unsigned পূর্ণাঙ্ক)(codeloc);
    unsigned পূর্ণাঙ্ক  __rstruct_flag = (unsigned পূর্ণাঙ্ক)(cif->rstruct_flag);
    unsigned পূর্ণাঙ্ক  __inner = (unsigned পূর্ণাঙ্ক)(&ffi_closure_SYSV_inner);
    *(unsigned পূর্ণাঙ্ক*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned পূর্ণাঙ্ক*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned পূর্ণাঙ্ক*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned পূর্ণাঙ্ক*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned পূর্ণাঙ্ক*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned পূর্ণাঙ্ক*) &__tramp[20] = __ctx;
    *(unsigned পূর্ণাঙ্ক*) &__tramp[24] = __rstruct_flag;
    *(unsigned পূর্ণাঙ্ক*) &__tramp[28] = __inner;
    *(unsigned পূর্ণাঙ্ক*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_data = user_data;
    closure->fun  = fun;

    ঘূৰাই FFI_OK;
}

