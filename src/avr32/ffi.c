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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <asm/unistd.h>

/* #define DEBUG */

बाह्य शून्य ffi_call_SYSV(शून्य (*)(वर्ण *, extended_cif *), extended_cif *,
    unsigned पूर्णाङ्क, unsigned पूर्णाङ्क, unsigned पूर्णाङ्क*, unsigned पूर्णाङ्क,
    शून्य (*fn)(शून्य));
बाह्य शून्य ffi_closure_SYSV (ffi_closure *);

unsigned पूर्णाङ्क pass_struct_on_राशिः(ffi_type *type)
{
    सचे(type->type != FFI_TYPE_STRUCT)
        पटिनिवत्त 0;

    सचे(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        पटिनिवत्त 1;

    सचे(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        पटिनिवत्त 1;

    पटिनिवत्त 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

शून्य ffi_prep_args(वर्ण *राशिः, extended_cif *ecif)
{
    unsigned पूर्णाङ्क i;
    शून्य **p_argv;
    ffi_type **p_arg;
    वर्ण *reg_base = राशिः;
    वर्ण *राशिः_base = राशिः + 20;
    unsigned पूर्णाङ्क राशिः_offset = 0;
    unsigned पूर्णाङ्क reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    सचे(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(शून्य**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    अत्थाय(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        पूर्णाङ्क alignment = (*p_arg)->alignment;
        पूर्णाङ्क type = (*p_arg)->type;
        वर्ण *addr = 0;

        सचे(z % 4 != 0)
            z += (4 - z % 4);

        सचे(reg_mask != 0x1f)
        {
            सचे(pass_struct_on_राशिः(*p_arg))
            {
                addr = राशिः_base + राशिः_offset;
                राशिः_offset += z;
            }
            अञ्ञथा सचे(z == परिमाण(पूर्णाङ्क))
            {
                वर्ण index = 0;

                याव((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            अञ्ञथा सचे(z == 2 * परिमाण(पूर्णाङ्क))
            {
                सचे(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                अञ्ञथा सचे(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        सचे(!addr)
        {
            addr = राशिः_base + राशिः_offset;
            राशिः_offset += z;
        }

        सचे(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == रिक्त)
            type = (*p_arg)->elements[0]->type;

        विकल्प(type)
        {
        पक्ष FFI_TYPE_UINT8:
            *(unsigned पूर्णाङ्क *)addr = (unsigned पूर्णाङ्क)*(UINT8 *)(*p_argv);
            भङ्ग;
        पक्ष FFI_TYPE_SINT8:
            *(signed पूर्णाङ्क *)addr = (signed पूर्णाङ्क)*(SINT8 *)(*p_argv);
            भङ्ग;
        पक्ष FFI_TYPE_UINT16:
            *(unsigned पूर्णाङ्क *)addr = (unsigned पूर्णाङ्क)*(UINT16 *)(*p_argv);
            भङ्ग;
        पक्ष FFI_TYPE_SINT16:
            *(signed पूर्णाङ्क *)addr = (signed पूर्णाङ्क)*(SINT16 *)(*p_argv);
            भङ्ग;
        स्वभाव:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    अत्थाय(i = 0; i < 5; i++)
    {
        सचे((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        अञ्ञथा
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned पूर्णाङ्क*)reg_base)[i]);
    }

    अत्थाय(i = 0; i < राशिः_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned पूर्णाङ्क*)राशिः_base)[i]);
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
    विकल्प(cif->rtype->type)
    {
    पक्ष FFI_TYPE_SINT8:
    पक्ष FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        भङ्ग;
    पक्ष FFI_TYPE_SINT16:
    पक्ष FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        भङ्ग;
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_UINT32:
    पक्ष FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        भङ्ग;
    पक्ष FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        सचे(!pass_struct_on_राशिः(cif->rtype))
        {
            सचे(cif->rtype->size <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            अञ्ञथा सचे(cif->rtype->size <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            अञ्ञथा सचे(cif->rtype->size <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            अञ्ञथा सचे(cif->rtype->size <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            अञ्ञथा
                cif->flags = (unsigned)cif->rtype->type;
        }
        अञ्ञथा
            cif->flags = (unsigned)cif->rtype->type;
        भङ्ग;
    स्वभाव:
        cif->flags = (unsigned)cif->rtype->type;
        भङ्ग;
    }

    पटिनिवत्त FFI_OK;
}

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
    extended_cif ecif;

    unsigned पूर्णाङ्क size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    अत्थाय(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    सचे((rvalue == रिक्त) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    अञ्ञथा
        ecif.rvalue = rvalue;

    विकल्प(cif->abi)
    {
    पक्ष FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        भङ्ग;
    स्वभाव:
        FFI_ASSERT(0);
        भङ्ग;
    }
}

स्थिर शून्य ffi_prep_incoming_args_SYSV(वर्ण *राशिः, शून्य **rvalue,
    शून्य **avalue, ffi_cif *cif)
{
    पञ्जिका unsigned पूर्णाङ्क i, reg_mask = 0;
    पञ्जिका शून्य **p_argv;
    पञ्जिका ffi_type **p_arg;
    पञ्जिका वर्ण *reg_base = राशिः;
    पञ्जिका वर्ण *राशिः_base = राशिः + 20;
    पञ्जिका unsigned पूर्णाङ्क राशिः_offset = 0;

#ifdef DEBUG
    /* Debugging */
    अत्थाय(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned पूर्णाङ्क*)राशिः)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    सचे(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(शून्य **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    अत्थाय(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        पूर्णाङ्क alignment = (*p_arg)->alignment;

        *p_argv = 0;

        सचे(z % 4 != 0)
            z += (4 - z % 4);

        सचे(reg_mask != 0x1f)
        {
            सचे(pass_struct_on_राशिः(*p_arg))
            {
                *p_argv = (शून्य*)राशिः_base + राशिः_offset;
                राशिः_offset += z;
            }
            अञ्ञथा सचे(z <= परिमाण(पूर्णाङ्क))
            {
                वर्ण index = 0;

                याव((reg_mask >> index) & 1)
                    index++;

                *p_argv = (शून्य*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            अञ्ञथा सचे(z == 2 * परिमाण(पूर्णाङ्क))
            {
                सचे(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (शून्य*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                अञ्ञथा सचे(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (शून्य*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        सचे(!*p_argv)
        {
            *p_argv = (शून्य*)राशिः_base + राशिः_offset;
            राशिः_offset += z;
        }

        सचे((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == रिक्त)
        {
            सचे(alignment == 1)
                **(unsigned पूर्णाङ्क**)p_argv <<= 24;
            अञ्ञथा सचे(alignment == 2)
                **(unsigned पूर्णाङ्क**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    अत्थाय(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned पूर्णाङ्क**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned पूर्णाङ्क ffi_closure_SYSV_inner(ffi_closure *closure, शून्य **respp,
    शून्य *args)
{
    ffi_cif *cif;
    शून्य **arg_area;
    unsigned पूर्णाङ्क i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    अत्थाय(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (शून्य **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    पटिनिवत्त cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*), शून्य *user_data,
    शून्य *codeloc)
{
    सचे (cif->abi != FFI_SYSV)
      पटिनिवत्त FFI_BAD_ABI;

    unsigned वर्ण *__tramp = (unsigned वर्ण*)(&closure->tramp[0]);
    unsigned पूर्णाङ्क  __fun = (unsigned पूर्णाङ्क)(&ffi_closure_SYSV);
    unsigned पूर्णाङ्क  __ctx = (unsigned पूर्णाङ्क)(codeloc);
    unsigned पूर्णाङ्क  __rstruct_flag = (unsigned पूर्णाङ्क)(cif->rstruct_flag);
    unsigned पूर्णाङ्क  __inner = (unsigned पूर्णाङ्क)(&ffi_closure_SYSV_inner);
    *(unsigned पूर्णाङ्क*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned पूर्णाङ्क*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned पूर्णाङ्क*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned पूर्णाङ्क*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned पूर्णाङ्क*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned पूर्णाङ्क*) &__tramp[20] = __ctx;
    *(unsigned पूर्णाङ्क*) &__tramp[24] = __rstruct_flag;
    *(unsigned पूर्णाङ्क*) &__tramp[28] = __inner;
    *(unsigned पूर्णाङ्क*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_data = user_data;
    closure->fun  = fun;

    पटिनिवत्त FFI_OK;
}

