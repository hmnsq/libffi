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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>
#समावेश <stdio.h>
#समावेश <unistd.h>
#समावेश <asm/unistd.h>

/* #define DEBUG */

बाह्य व्योम ffi_call_SYSV(व्योम (*)(अक्षर *, extended_cif *), extended_cif *,
    अचिन्हित पूर्णांक, अचिन्हित पूर्णांक, अचिन्हित पूर्णांक*, अचिन्हित पूर्णांक,
    व्योम (*fn)(व्योम));
बाह्य व्योम ffi_closure_SYSV (ffi_closure *);

अचिन्हित पूर्णांक pass_struct_on_ढेर(ffi_type *type)
{
    अगर(type->type != FFI_TYPE_STRUCT)
        वापस 0;

    अगर(type->alignment < type->आकार &&
        !(type->आकार == 4 || type->आकार == 8) &&
        !(type->आकार == 8 && type->alignment >= 4))
        वापस 1;

    अगर(type->आकार == 3 || type->आकार == 5 || type->आकार == 6 ||
        type->आकार == 7)
        वापस 1;

    वापस 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

व्योम ffi_prep_args(अक्षर *ढेर, extended_cif *ecif)
{
    अचिन्हित पूर्णांक i;
    व्योम **p_argv;
    ffi_type **p_arg;
    अक्षर *reg_base = ढेर;
    अक्षर *ढेर_base = ढेर + 20;
    अचिन्हित पूर्णांक ढेर_offset = 0;
    अचिन्हित पूर्णांक reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    अगर(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(व्योम**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    क्रम(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        माप_प्रकार z = (*p_arg)->आकार;
        पूर्णांक alignment = (*p_arg)->alignment;
        पूर्णांक type = (*p_arg)->type;
        अक्षर *addr = 0;

        अगर(z % 4 != 0)
            z += (4 - z % 4);

        अगर(reg_mask != 0x1f)
        {
            अगर(pass_struct_on_ढेर(*p_arg))
            {
                addr = ढेर_base + ढेर_offset;
                ढेर_offset += z;
            }
            अन्यथा अगर(z == माप(पूर्णांक))
            {
                अक्षर सूचकांक = 0;

                जबतक((reg_mask >> सूचकांक) & 1)
                    सूचकांक++;

                addr = reg_base + (सूचकांक * 4);
                reg_mask |= (1 << सूचकांक);
            }
            अन्यथा अगर(z == 2 * माप(पूर्णांक))
            {
                अगर(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                अन्यथा अगर(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        अगर(!addr)
        {
            addr = ढेर_base + ढेर_offset;
            ढेर_offset += z;
        }

        अगर(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == शून्य)
            type = (*p_arg)->elements[0]->type;

        चयन(type)
        {
        हाल FFI_TYPE_UINT8:
            *(अचिन्हित पूर्णांक *)addr = (अचिन्हित पूर्णांक)*(UINT8 *)(*p_argv);
            अवरोध;
        हाल FFI_TYPE_SINT8:
            *(चिन्हित पूर्णांक *)addr = (चिन्हित पूर्णांक)*(SINT8 *)(*p_argv);
            अवरोध;
        हाल FFI_TYPE_UINT16:
            *(अचिन्हित पूर्णांक *)addr = (अचिन्हित पूर्णांक)*(UINT16 *)(*p_argv);
            अवरोध;
        हाल FFI_TYPE_SINT16:
            *(चिन्हित पूर्णांक *)addr = (चिन्हित पूर्णांक)*(SINT16 *)(*p_argv);
            अवरोध;
        शेष:
            स_नकल(addr, *p_argv, z);
        }

        p_argv++;
    }

#अगर_घोषित DEBUG
    /* Debugging */
    क्रम(i = 0; i < 5; i++)
    {
        अगर((reg_mask & (1 << i)) == 0)
            म_लिखो("r%d: (unused)\n", 12 - i);
        अन्यथा
            म_लिखो("r%d: 0x%08x\n", 12 - i, ((अचिन्हित पूर्णांक*)reg_base)[i]);
    }

    क्रम(i = 0; i < ढेर_offset / 4; i++)
    {
        म_लिखो("sp+%d: 0x%08x\n", i*4, ((अचिन्हित पूर्णांक*)ढेर_base)[i]);
    }
#पूर्ण_अगर
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
    चयन(cif->rtype->type)
    {
    हाल FFI_TYPE_SINT8:
    हाल FFI_TYPE_UINT8:
        cif->flags = (अचिन्हित)FFI_TYPE_UINT8;
        अवरोध;
    हाल FFI_TYPE_SINT16:
    हाल FFI_TYPE_UINT16:
        cif->flags = (अचिन्हित)FFI_TYPE_UINT16;
        अवरोध;
    हाल FFI_TYPE_FLOAT:
    हाल FFI_TYPE_SINT32:
    हाल FFI_TYPE_UINT32:
    हाल FFI_TYPE_POINTER:
        cif->flags = (अचिन्हित)FFI_TYPE_UINT32;
        अवरोध;
    हाल FFI_TYPE_DOUBLE:
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
        cif->flags = (अचिन्हित)FFI_TYPE_UINT64;
        अवरोध;
    हाल FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        अगर(!pass_struct_on_ढेर(cif->rtype))
        {
            अगर(cif->rtype->आकार <= 1)
                cif->flags = (अचिन्हित)FFI_TYPE_UINT8;
            अन्यथा अगर(cif->rtype->आकार <= 2)
                cif->flags = (अचिन्हित)FFI_TYPE_UINT16;
            अन्यथा अगर(cif->rtype->आकार <= 4)
                cif->flags = (अचिन्हित)FFI_TYPE_UINT32;
            अन्यथा अगर(cif->rtype->आकार <= 8)
                cif->flags = (अचिन्हित)FFI_TYPE_UINT64;
            अन्यथा
                cif->flags = (अचिन्हित)cif->rtype->type;
        }
        अन्यथा
            cif->flags = (अचिन्हित)cif->rtype->type;
        अवरोध;
    शेष:
        cif->flags = (अचिन्हित)cif->rtype->type;
        अवरोध;
    }

    वापस FFI_OK;
}

व्योम ffi_call(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
    extended_cif ecif;

    अचिन्हित पूर्णांक आकार = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    क्रम(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        आकार += (*p_arg)->आकार + (4 - (*p_arg)->आकार % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    अगर((rvalue == शून्य) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->आकार);
    अन्यथा
        ecif.rvalue = rvalue;

    चयन(cif->abi)
    {
    हाल FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, आकार, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        अवरोध;
    शेष:
        FFI_ASSERT(0);
        अवरोध;
    }
}

अटल व्योम ffi_prep_incoming_args_SYSV(अक्षर *ढेर, व्योम **rvalue,
    व्योम **avalue, ffi_cif *cif)
{
    रेजिस्टर अचिन्हित पूर्णांक i, reg_mask = 0;
    रेजिस्टर व्योम **p_argv;
    रेजिस्टर ffi_type **p_arg;
    रेजिस्टर अक्षर *reg_base = ढेर;
    रेजिस्टर अक्षर *ढेर_base = ढेर + 20;
    रेजिस्टर अचिन्हित पूर्णांक ढेर_offset = 0;

#अगर_घोषित DEBUG
    /* Debugging */
    क्रम(i = 0; i < cif->nargs + 7; i++)
    {
        म_लिखो("sp+%d: 0x%08x\n", i*4, ((अचिन्हित पूर्णांक*)ढेर)[i]);
    }
#पूर्ण_अगर

    /* If cif->flags is struct then we know it's not passed in registers */
    अगर(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(व्योम **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    क्रम(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        माप_प्रकार z = (*p_arg)->आकार;
        पूर्णांक alignment = (*p_arg)->alignment;

        *p_argv = 0;

        अगर(z % 4 != 0)
            z += (4 - z % 4);

        अगर(reg_mask != 0x1f)
        {
            अगर(pass_struct_on_ढेर(*p_arg))
            {
                *p_argv = (व्योम*)ढेर_base + ढेर_offset;
                ढेर_offset += z;
            }
            अन्यथा अगर(z <= माप(पूर्णांक))
            {
                अक्षर सूचकांक = 0;

                जबतक((reg_mask >> सूचकांक) & 1)
                    सूचकांक++;

                *p_argv = (व्योम*)reg_base + (सूचकांक * 4);
                reg_mask |= (1 << सूचकांक);
            }
            अन्यथा अगर(z == 2 * माप(पूर्णांक))
            {
                अगर(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (व्योम*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                अन्यथा अगर(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (व्योम*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        अगर(!*p_argv)
        {
            *p_argv = (व्योम*)ढेर_base + ढेर_offset;
            ढेर_offset += z;
        }

        अगर((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == शून्य)
        {
            अगर(alignment == 1)
                **(अचिन्हित पूर्णांक**)p_argv <<= 24;
            अन्यथा अगर(alignment == 2)
                **(अचिन्हित पूर्णांक**)p_argv <<= 16;
        }

        p_argv++;
    }

#अगर_घोषित DEBUG
    /* Debugging */
    क्रम(i = 0; i < cif->nargs; i++)
    {
        म_लिखो("sp+%d: 0x%08x\n", i*4, *(((अचिन्हित पूर्णांक**)avalue)[i]));
    }
#पूर्ण_अगर
}

/* This function is jumped to by the trampoline */

अचिन्हित पूर्णांक ffi_closure_SYSV_inner(ffi_closure *closure, व्योम **respp,
    व्योम *args)
{
    ffi_cif *cif;
    व्योम **arg_area;
    अचिन्हित पूर्णांक i, आकार = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    क्रम(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        आकार += (*p_arg)->आकार + (4 - (*p_arg)->आकार % 4);

    arg_area = (व्योम **)alloca(आकार);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_तथ्य);

    वापस cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*), व्योम *user_तथ्य,
    व्योम *codeloc)
{
    अगर (cif->abi != FFI_SYSV)
      वापस FFI_BAD_ABI;

    अचिन्हित अक्षर *__tramp = (अचिन्हित अक्षर*)(&closure->tramp[0]);
    अचिन्हित पूर्णांक  __fun = (अचिन्हित पूर्णांक)(&ffi_closure_SYSV);
    अचिन्हित पूर्णांक  __ctx = (अचिन्हित पूर्णांक)(codeloc);
    अचिन्हित पूर्णांक  __rstruct_flag = (अचिन्हित पूर्णांक)(cif->rstruct_flag);
    अचिन्हित पूर्णांक  __inner = (अचिन्हित पूर्णांक)(&ffi_closure_SYSV_inner);
    *(अचिन्हित पूर्णांक*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(अचिन्हित पूर्णांक*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(अचिन्हित पूर्णांक*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(अचिन्हित पूर्णांक*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(अचिन्हित पूर्णांक*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(अचिन्हित पूर्णांक*) &__tramp[20] = __ctx;
    *(अचिन्हित पूर्णांक*) &__tramp[24] = __rstruct_flag;
    *(अचिन्हित पूर्णांक*) &__tramp[28] = __inner;
    *(अचिन्हित पूर्णांक*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_तथ्य = user_तथ्य;
    closure->fun  = fun;

    वापस FFI_OK;
}

