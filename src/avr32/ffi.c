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
    अचिह्नित पूर्णाङ्क, अचिह्नित पूर्णाङ्क, अचिह्नित पूर्णाङ्क*, अचिह्नित पूर्णाङ्क,
    शून्य (*fn)(शून्य));
बाह्य शून्य ffi_संवृतिः_SYSV (ffi_संवृतिः *);

अचिह्नित पूर्णाङ्क pass_struct_on_राशिः(ffi_type *type)
{
    यदि(type->type != FFI_TYPE_STRUCT)
        प्रत्यागम 0;

    यदि(type->alignment < type->आकार &&
        !(type->आकार == 4 || type->आकार == 8) &&
        !(type->आकार == 8 && type->alignment >= 4))
        प्रत्यागम 1;

    यदि(type->आकार == 3 || type->आकार == 5 || type->आकार == 6 ||
        type->आकार == 7)
        प्रत्यागम 1;

    प्रत्यागम 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

शून्य ffi_prep_तर्काः(वर्ण *राशिः, extended_cif *ecif)
{
    अचिह्नित पूर्णाङ्क i;
    शून्य **p_argv;
    ffi_type **p_arg;
    वर्ण *reg_आधारः = राशिः;
    वर्ण *राशिः_आधारः = राशिः + 20;
    अचिह्नित पूर्णाङ्क राशिः_विस्थापनम् = 0;
    अचिह्नित पूर्णाङ्क reg_आवरणम् = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    यदि(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(शून्य**)reg_आधारः = ecif->rvalue;
        reg_आवरणम् |= 1;
    }

    प्रति(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->आकार;
        पूर्णाङ्क alignment = (*p_arg)->alignment;
        पूर्णाङ्क type = (*p_arg)->type;
        वर्ण *addr = 0;

        यदि(z % 4 != 0)
            z += (4 - z % 4);

        यदि(reg_आवरणम् != 0x1f)
        {
            यदि(pass_struct_on_राशिः(*p_arg))
            {
                addr = राशिः_आधारः + राशिः_विस्थापनम्;
                राशिः_विस्थापनम् += z;
            }
            अन्यथा यदि(z == परिमाण(पूर्णाङ्क))
            {
                वर्ण सूचकांक = 0;

                यावत्((reg_आवरणम् >> सूचकांक) & 1)
                    सूचकांक++;

                addr = reg_आधारः + (सूचकांक * 4);
                reg_आवरणम् |= (1 << सूचकांक);
            }
            अन्यथा यदि(z == 2 * परिमाण(पूर्णाङ्क))
            {
                यदि(!((reg_आवरणम् >> 1) & 1))
                {
                    addr = reg_आधारः + 4;
                    reg_आवरणम् |= (3 << 1);
                }
                अन्यथा यदि(!((reg_आवरणम् >> 3) & 1))
                {
                    addr = reg_आधारः + 12;
                    reg_आवरणम् |= (3 << 3);
                }
            }
        }

        यदि(!addr)
        {
            addr = राशिः_आधारः + राशिः_विस्थापनम्;
            राशिः_विस्थापनम् += z;
        }

        यदि(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == रिक्त)
            type = (*p_arg)->elements[0]->type;

        विकल्प(type)
        {
        पक्ष FFI_TYPE_UINT8:
            *(अचिह्नित पूर्णाङ्क *)addr = (अचिह्नित पूर्णाङ्क)*(UINT8 *)(*p_argv);
            भङ्ग;
        पक्ष FFI_TYPE_SINT8:
            *(चिह्नित पूर्णाङ्क *)addr = (चिह्नित पूर्णाङ्क)*(SINT8 *)(*p_argv);
            भङ्ग;
        पक्ष FFI_TYPE_UINT16:
            *(अचिह्नित पूर्णाङ्क *)addr = (अचिह्नित पूर्णाङ्क)*(UINT16 *)(*p_argv);
            भङ्ग;
        पक्ष FFI_TYPE_SINT16:
            *(चिह्नित पूर्णाङ्क *)addr = (चिह्नित पूर्णाङ्क)*(SINT16 *)(*p_argv);
            भङ्ग;
        स्वभाव:
            स्मृतिं_प्रतिलिख(addr, *p_argv, z);
        }

        p_argv++;
    }

#यदि_परिभाषितम् DEBUG
    /* Debugging */
    प्रति(i = 0; i < 5; i++)
    {
        यदि((reg_आवरणम् & (1 << i)) == 0)
            प्रारूप_लिख("r%d: (unused)\n", 12 - i);
        अन्यथा
            प्रारूप_लिख("r%d: 0x%08x\n", 12 - i, ((अचिह्नित पूर्णाङ्क*)reg_आधारः)[i]);
    }

    प्रति(i = 0; i < राशिः_विस्थापनम् / 4; i++)
    {
        प्रारूप_लिख("sp+%d: 0x%08x\n", i*4, ((अचिह्नित पूर्णाङ्क*)राशिः_आधारः)[i]);
    }
#यदि_अन्तः
}

/* Perform machine dependent cif processing */
ffi_स्थितिः ffi_prep_cif_machdep(ffi_cif *cif)
{
    /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     * everywhere, but it is on some platforms, and it doesn't harm
     * anything when it isn't needed. */
    cif->अष्टकानि = (cif->अष्टकानि + 7) & ~7;

    /* Flag to indicate that he return value is in fact a struct */
    cif->rstruct_flag = 0;

    /* Set the return type flag */
    विकल्प(cif->rtype->type)
    {
    पक्ष FFI_TYPE_SINT8:
    पक्ष FFI_TYPE_UINT8:
        cif->flags = (अचिह्नित)FFI_TYPE_UINT8;
        भङ्ग;
    पक्ष FFI_TYPE_SINT16:
    पक्ष FFI_TYPE_UINT16:
        cif->flags = (अचिह्नित)FFI_TYPE_UINT16;
        भङ्ग;
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_UINT32:
    पक्ष FFI_TYPE_POINTER:
        cif->flags = (अचिह्नित)FFI_TYPE_UINT32;
        भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
        cif->flags = (अचिह्नित)FFI_TYPE_UINT64;
        भङ्ग;
    पक्ष FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        यदि(!pass_struct_on_राशिः(cif->rtype))
        {
            यदि(cif->rtype->आकार <= 1)
                cif->flags = (अचिह्नित)FFI_TYPE_UINT8;
            अन्यथा यदि(cif->rtype->आकार <= 2)
                cif->flags = (अचिह्नित)FFI_TYPE_UINT16;
            अन्यथा यदि(cif->rtype->आकार <= 4)
                cif->flags = (अचिह्नित)FFI_TYPE_UINT32;
            अन्यथा यदि(cif->rtype->आकार <= 8)
                cif->flags = (अचिह्नित)FFI_TYPE_UINT64;
            अन्यथा
                cif->flags = (अचिह्नित)cif->rtype->type;
        }
        अन्यथा
            cif->flags = (अचिह्नित)cif->rtype->type;
        भङ्ग;
    स्वभाव:
        cif->flags = (अचिह्नित)cif->rtype->type;
        भङ्ग;
    }

    प्रत्यागम FFI_OK;
}

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
    extended_cif ecif;

    अचिह्नित पूर्णाङ्क आकार = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    प्रति(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        आकार += (*p_arg)->आकार + (4 - (*p_arg)->आकार % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    यदि((rvalue == रिक्त) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->आकार);
    अन्यथा
        ecif.rvalue = rvalue;

    विकल्प(cif->abi)
    {
    पक्ष FFI_SYSV:
        ffi_call_SYSV(ffi_prep_तर्काः, &ecif, आकार, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        भङ्ग;
    स्वभाव:
        FFI_ASSERT(0);
        भङ्ग;
    }
}

स्थिर शून्य ffi_prep_incoming_तर्काः_SYSV(वर्ण *राशिः, शून्य **rvalue,
    शून्य **avalue, ffi_cif *cif)
{
    पञ्जिका अचिह्नित पूर्णाङ्क i, reg_आवरणम् = 0;
    पञ्जिका शून्य **p_argv;
    पञ्जिका ffi_type **p_arg;
    पञ्जिका वर्ण *reg_आधारः = राशिः;
    पञ्जिका वर्ण *राशिः_आधारः = राशिः + 20;
    पञ्जिका अचिह्नित पूर्णाङ्क राशिः_विस्थापनम् = 0;

#यदि_परिभाषितम् DEBUG
    /* Debugging */
    प्रति(i = 0; i < cif->nargs + 7; i++)
    {
        प्रारूप_लिख("sp+%d: 0x%08x\n", i*4, ((अचिह्नित पूर्णाङ्क*)राशिः)[i]);
    }
#यदि_अन्तः

    /* If cif->flags is struct then we know it's not passed in registers */
    यदि(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(शून्य **)reg_आधारः;
        reg_आवरणम् |= 1;
    }

    p_argv = avalue;

    प्रति(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->आकार;
        पूर्णाङ्क alignment = (*p_arg)->alignment;

        *p_argv = 0;

        यदि(z % 4 != 0)
            z += (4 - z % 4);

        यदि(reg_आवरणम् != 0x1f)
        {
            यदि(pass_struct_on_राशिः(*p_arg))
            {
                *p_argv = (शून्य*)राशिः_आधारः + राशिः_विस्थापनम्;
                राशिः_विस्थापनम् += z;
            }
            अन्यथा यदि(z <= परिमाण(पूर्णाङ्क))
            {
                वर्ण सूचकांक = 0;

                यावत्((reg_आवरणम् >> सूचकांक) & 1)
                    सूचकांक++;

                *p_argv = (शून्य*)reg_आधारः + (सूचकांक * 4);
                reg_आवरणम् |= (1 << सूचकांक);
            }
            अन्यथा यदि(z == 2 * परिमाण(पूर्णाङ्क))
            {
                यदि(!((reg_आवरणम् >> 1) & 1))
                {
                    *p_argv = (शून्य*)reg_आधारः + 4;
                    reg_आवरणम् |= (3 << 1);
                }
                अन्यथा यदि(!((reg_आवरणम् >> 3) & 1))
                {
                    *p_argv = (शून्य*)reg_आधारः + 12;
                    reg_आवरणम् |= (3 << 3);
                }
            }
        }

        यदि(!*p_argv)
        {
            *p_argv = (शून्य*)राशिः_आधारः + राशिः_विस्थापनम्;
            राशिः_विस्थापनम् += z;
        }

        यदि((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == रिक्त)
        {
            यदि(alignment == 1)
                **(अचिह्नित पूर्णाङ्क**)p_argv <<= 24;
            अन्यथा यदि(alignment == 2)
                **(अचिह्नित पूर्णाङ्क**)p_argv <<= 16;
        }

        p_argv++;
    }

#यदि_परिभाषितम् DEBUG
    /* Debugging */
    प्रति(i = 0; i < cif->nargs; i++)
    {
        प्रारूप_लिख("sp+%d: 0x%08x\n", i*4, *(((अचिह्नित पूर्णाङ्क**)avalue)[i]));
    }
#यदि_अन्तः
}

/* This function is jumped to by the trampoline */

अचिह्नित पूर्णाङ्क ffi_संवृतिः_SYSV_inner(ffi_संवृतिः *संवृतिः, शून्य **respp,
    शून्य *तर्काः)
{
    ffi_cif *cif;
    शून्य **arg_area;
    अचिह्नित पूर्णाङ्क i, आकार = 0;
    ffi_type **p_arg;

    cif = संवृतिः->cif;

    प्रति(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        आकार += (*p_arg)->आकार + (4 - (*p_arg)->आकार % 4);

    arg_area = (शून्य **)alloca(आकार);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_तर्काः_SYSV(तर्काः, respp, arg_area, cif);

    (संवृतिः->fun)(cif, *respp, arg_area, संवृतिः->user_दत्त);

    प्रत्यागम cif->flags;
}

ffi_स्थितिः ffi_prep_संवृतिः_loc(ffi_संवृतिः* संवृतिः, ffi_cif* cif,
    शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*), शून्य *user_दत्त,
    शून्य *codeloc)
{
    यदि (cif->abi != FFI_SYSV)
      प्रत्यागम FFI_BAD_ABI;

    अचिह्नित वर्ण *__tramp = (अचिह्नित वर्ण*)(&संवृतिः->tramp[0]);
    अचिह्नित पूर्णाङ्क  __fun = (अचिह्नित पूर्णाङ्क)(&ffi_संवृतिः_SYSV);
    अचिह्नित पूर्णाङ्क  __ctx = (अचिह्नित पूर्णाङ्क)(codeloc);
    अचिह्नित पूर्णाङ्क  __rstruct_flag = (अचिह्नित पूर्णाङ्क)(cif->rstruct_flag);
    अचिह्नित पूर्णाङ्क  __inner = (अचिह्नित पूर्णाङ्क)(&ffi_संवृतिः_SYSV_inner);
    *(अचिह्नित पूर्णाङ्क*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(अचिह्नित पूर्णाङ्क*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(अचिह्नित पूर्णाङ्क*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(अचिह्नित पूर्णाङ्क*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(अचिह्नित पूर्णाङ्क*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(अचिह्नित पूर्णाङ्क*) &__tramp[20] = __ctx;
    *(अचिह्नित पूर्णाङ्क*) &__tramp[24] = __rstruct_flag;
    *(अचिह्नित पूर्णाङ्क*) &__tramp[28] = __inner;
    *(अचिह्नित पूर्णाङ्क*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    संवृतिः->cif = cif;
    संवृतिः->user_दत्त = user_दत्त;
    संवृतिः->fun  = fun;

    प्रत्यागम FFI_OK;
}

