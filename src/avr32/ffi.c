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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <asm/unistd.h>

/* #define DEBUG */

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV(ଶୂନ୍ୟ (*)(ଅକ୍ଷର *, extended_cif *), extended_cif *,
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା,
    ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ));
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_SYSV (ffi_closure *);

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pass_struct_on_ଗଦା(ffi_type *type)
{
    ଯଦି(type->type != FFI_TYPE_STRUCT)
        ଫେରନ୍ତୁ 0;

    ଯଦି(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        ଫେରନ୍ତୁ 1;

    ଯଦି(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        ଫେରନ୍ତୁ 1;

    ଫେରନ୍ତୁ 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

ଶୂନ୍ୟ ffi_prep_args(ଅକ୍ଷର *ଗଦା, extended_cif *ecif)
{
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ଶୂନ୍ୟ **p_argv;
    ffi_type **p_arg;
    ଅକ୍ଷର *reg_base = ଗଦା;
    ଅକ୍ଷର *ଗଦା_base = ଗଦା + 20;
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଦା_offset = 0;
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    ଯଦି(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(ଶୂନ୍ୟ**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    ପାଇଁ(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା alignment = (*p_arg)->alignment;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା type = (*p_arg)->type;
        ଅକ୍ଷର *addr = 0;

        ଯଦି(z % 4 != 0)
            z += (4 - z % 4);

        ଯଦି(reg_mask != 0x1f)
        {
            ଯଦି(pass_struct_on_ଗଦା(*p_arg))
            {
                addr = ଗଦା_base + ଗଦା_offset;
                ଗଦା_offset += z;
            }
            ଅନ୍ୟଥା ଯଦି(z == ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
            {
                ଅକ୍ଷର index = 0;

                ଯେତେବେଳେ((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            ଅନ୍ୟଥା ଯଦି(z == 2 * ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
            {
                ଯଦି(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                ଅନ୍ୟଥା ଯଦି(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        ଯଦି(!addr)
        {
            addr = ଗଦା_base + ଗଦା_offset;
            ଗଦା_offset += z;
        }

        ଯଦି(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == ଖାଲି)
            type = (*p_arg)->elements[0]->type;

        ବାଛ(type)
        {
        ସ୍ଥିତି FFI_TYPE_UINT8:
            *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)addr = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT8 *)(*p_argv);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_SINT8:
            *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)addr = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT8 *)(*p_argv);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_UINT16:
            *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)addr = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT16 *)(*p_argv);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_SINT16:
            *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)addr = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT16 *)(*p_argv);
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    ପାଇଁ(i = 0; i < 5; i++)
    {
        ଯଦି((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        ଅନ୍ୟଥା
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)reg_base)[i]);
    }

    ପାଇଁ(i = 0; i < ଗଦା_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)ଗଦା_base)[i]);
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
    ବାଛ(cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_SINT8:
    ସ୍ଥିତି FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT16:
    ସ୍ଥିତି FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_FLOAT:
    ସ୍ଥିତି FFI_TYPE_SINT32:
    ସ୍ଥିତି FFI_TYPE_UINT32:
    ସ୍ଥିତି FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        ଯଦି(!pass_struct_on_ଗଦା(cif->rtype))
        {
            ଯଦି(cif->rtype->size <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            ଅନ୍ୟଥା ଯଦି(cif->rtype->size <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            ଅନ୍ୟଥା ଯଦି(cif->rtype->size <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            ଅନ୍ୟଥା ଯଦି(cif->rtype->size <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            ଅନ୍ୟଥା
                cif->flags = (unsigned)cif->rtype->type;
        }
        ଅନ୍ୟଥା
            cif->flags = (unsigned)cif->rtype->type;
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        cif->flags = (unsigned)cif->rtype->type;
        ଭାଙ୍ଗ;
    }

    ଫେରନ୍ତୁ FFI_OK;
}

ଶୂନ୍ୟ ffi_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
    extended_cif ecif;

    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    ପାଇଁ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    ଯଦି((rvalue == ଖାଲି) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    ଅନ୍ୟଥା
        ecif.rvalue = rvalue;

    ବାଛ(cif->abi)
    {
    ସ୍ଥିତି FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        FFI_ASSERT(0);
        ଭାଙ୍ଗ;
    }
}

ସ୍ଥିର ଶୂନ୍ୟ ffi_prep_incoming_args_SYSV(ଅକ୍ଷର *ଗଦା, ଶୂନ୍ୟ **rvalue,
    ଶୂନ୍ୟ **avalue, ffi_cif *cif)
{
    ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, reg_mask = 0;
    ପଞ୍ଜିକରଣ ଶୂନ୍ୟ **p_argv;
    ପଞ୍ଜିକରଣ ffi_type **p_arg;
    ପଞ୍ଜିକରଣ ଅକ୍ଷର *reg_base = ଗଦା;
    ପଞ୍ଜିକରଣ ଅକ୍ଷର *ଗଦା_base = ଗଦା + 20;
    ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଦା_offset = 0;

#ifdef DEBUG
    /* Debugging */
    ପାଇଁ(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)ଗଦା)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    ଯଦି(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(ଶୂନ୍ୟ **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    ପାଇଁ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା alignment = (*p_arg)->alignment;

        *p_argv = 0;

        ଯଦି(z % 4 != 0)
            z += (4 - z % 4);

        ଯଦି(reg_mask != 0x1f)
        {
            ଯଦି(pass_struct_on_ଗଦା(*p_arg))
            {
                *p_argv = (ଶୂନ୍ୟ*)ଗଦା_base + ଗଦା_offset;
                ଗଦା_offset += z;
            }
            ଅନ୍ୟଥା ଯଦି(z <= ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
            {
                ଅକ୍ଷର index = 0;

                ଯେତେବେଳେ((reg_mask >> index) & 1)
                    index++;

                *p_argv = (ଶୂନ୍ୟ*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            ଅନ୍ୟଥା ଯଦି(z == 2 * ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
            {
                ଯଦି(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (ଶୂନ୍ୟ*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                ଅନ୍ୟଥା ଯଦି(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (ଶୂନ୍ୟ*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        ଯଦି(!*p_argv)
        {
            *p_argv = (ଶୂନ୍ୟ*)ଗଦା_base + ଗଦା_offset;
            ଗଦା_offset += z;
        }

        ଯଦି((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == ଖାଲି)
        {
            ଯଦି(alignment == 1)
                **(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା**)p_argv <<= 24;
            ଅନ୍ୟଥା ଯଦି(alignment == 2)
                **(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    ପାଇଁ(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_closure_SYSV_inner(ffi_closure *closure, ଶୂନ୍ୟ **respp,
    ଶୂନ୍ୟ *args)
{
    ffi_cif *cif;
    ଶୂନ୍ୟ **arg_area;
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    ପାଇଁ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (ଶୂନ୍ୟ **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    ଫେରନ୍ତୁ cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*), ଶୂନ୍ୟ *user_data,
    ଶୂନ୍ୟ *codeloc)
{
    ଯଦି (cif->abi != FFI_SYSV)
      ଫେରନ୍ତୁ FFI_BAD_ABI;

    unsigned ଅକ୍ଷର *__tramp = (unsigned ଅକ୍ଷର*)(&closure->tramp[0]);
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା  __fun = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(&ffi_closure_SYSV);
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା  __ctx = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(codeloc);
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା  __rstruct_flag = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(cif->rstruct_flag);
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା  __inner = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(&ffi_closure_SYSV_inner);
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[20] = __ctx;
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[24] = __rstruct_flag;
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[28] = __inner;
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_data = user_data;
    closure->fun  = fun;

    ଫେରନ୍ତୁ FFI_OK;
}

