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

बाह्य शून्य ffi_call_SYSV(शून्य (*)(अक्षर *, extended_cif *), extended_cif *,
    अचिन्हित पूर्णांक, अचिन्हित पूर्णांक, अचिन्हित पूर्णांक*, अचिन्हित पूर्णांक,
    शून्य (*fn)(शून्य));
बाह्य शून्य ffi_closure_SYSV (ffi_closure *);

अचिन्हित पूर्णांक pass_struct_on_चळत(ffi_type *type)
{
    जर(type->type != FFI_TYPE_STRUCT)
        परत 0;

    जर(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        परत 1;

    जर(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        परत 1;

    परत 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

शून्य ffi_prep_args(अक्षर *चळत, extended_cif *ecif)
{
    अचिन्हित पूर्णांक i;
    शून्य **p_argv;
    ffi_type **p_arg;
    अक्षर *reg_base = चळत;
    अक्षर *चळत_base = चळत + 20;
    अचिन्हित पूर्णांक चळत_offset = 0;
    अचिन्हित पूर्णांक reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    जर(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(शून्य**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    साठी(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        माप_प्रकार z = (*p_arg)->size;
        पूर्णांक alignment = (*p_arg)->alignment;
        पूर्णांक type = (*p_arg)->type;
        अक्षर *addr = 0;

        जर(z % 4 != 0)
            z += (4 - z % 4);

        जर(reg_mask != 0x1f)
        {
            जर(pass_struct_on_चळत(*p_arg))
            {
                addr = चळत_base + चळत_offset;
                चळत_offset += z;
            }
            अन्यथा जर(z == आकार(पूर्णांक))
            {
                अक्षर index = 0;

                पर्यंत((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            अन्यथा जर(z == 2 * आकार(पूर्णांक))
            {
                जर(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                अन्यथा जर(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        जर(!addr)
        {
            addr = चळत_base + चळत_offset;
            चळत_offset += z;
        }

        जर(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == रिक्त)
            type = (*p_arg)->elements[0]->type;

        निवड(type)
        {
        स्थिती FFI_TYPE_UINT8:
            *(अचिन्हित पूर्णांक *)addr = (अचिन्हित पूर्णांक)*(UINT8 *)(*p_argv);
            थांबा;
        स्थिती FFI_TYPE_SINT8:
            *(चिन्हित पूर्णांक *)addr = (चिन्हित पूर्णांक)*(SINT8 *)(*p_argv);
            थांबा;
        स्थिती FFI_TYPE_UINT16:
            *(अचिन्हित पूर्णांक *)addr = (अचिन्हित पूर्णांक)*(UINT16 *)(*p_argv);
            थांबा;
        स्थिती FFI_TYPE_SINT16:
            *(चिन्हित पूर्णांक *)addr = (चिन्हित पूर्णांक)*(SINT16 *)(*p_argv);
            थांबा;
        पूर्वनिर्धारित:
            स_नकल(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    साठी(i = 0; i < 5; i++)
    {
        जर((reg_mask & (1 << i)) == 0)
            म_लिखो("r%d: (unused)\n", 12 - i);
        अन्यथा
            म_लिखो("r%d: 0x%08x\n", 12 - i, ((अचिन्हित पूर्णांक*)reg_base)[i]);
    }

    साठी(i = 0; i < चळत_offset / 4; i++)
    {
        म_लिखो("sp+%d: 0x%08x\n", i*4, ((अचिन्हित पूर्णांक*)चळत_base)[i]);
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
    निवड(cif->rtype->type)
    {
    स्थिती FFI_TYPE_SINT8:
    स्थिती FFI_TYPE_UINT8:
        cif->flags = (अचिन्हित)FFI_TYPE_UINT8;
        थांबा;
    स्थिती FFI_TYPE_SINT16:
    स्थिती FFI_TYPE_UINT16:
        cif->flags = (अचिन्हित)FFI_TYPE_UINT16;
        थांबा;
    स्थिती FFI_TYPE_FLOAT:
    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_UINT32:
    स्थिती FFI_TYPE_POINTER:
        cif->flags = (अचिन्हित)FFI_TYPE_UINT32;
        थांबा;
    स्थिती FFI_TYPE_DOUBLE:
    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
        cif->flags = (अचिन्हित)FFI_TYPE_UINT64;
        थांबा;
    स्थिती FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        जर(!pass_struct_on_चळत(cif->rtype))
        {
            जर(cif->rtype->size <= 1)
                cif->flags = (अचिन्हित)FFI_TYPE_UINT8;
            अन्यथा जर(cif->rtype->size <= 2)
                cif->flags = (अचिन्हित)FFI_TYPE_UINT16;
            अन्यथा जर(cif->rtype->size <= 4)
                cif->flags = (अचिन्हित)FFI_TYPE_UINT32;
            अन्यथा जर(cif->rtype->size <= 8)
                cif->flags = (अचिन्हित)FFI_TYPE_UINT64;
            अन्यथा
                cif->flags = (अचिन्हित)cif->rtype->type;
        }
        अन्यथा
            cif->flags = (अचिन्हित)cif->rtype->type;
        थांबा;
    पूर्वनिर्धारित:
        cif->flags = (अचिन्हित)cif->rtype->type;
        थांबा;
    }

    परत FFI_OK;
}

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
    extended_cif ecif;

    अचिन्हित पूर्णांक size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    साठी(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    जर((rvalue == रिक्त) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    अन्यथा
        ecif.rvalue = rvalue;

    निवड(cif->abi)
    {
    स्थिती FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        थांबा;
    पूर्वनिर्धारित:
        FFI_ASSERT(0);
        थांबा;
    }
}

स्थिर शून्य ffi_prep_incoming_args_SYSV(अक्षर *चळत, शून्य **rvalue,
    शून्य **avalue, ffi_cif *cif)
{
    नोंदणी अचिन्हित पूर्णांक i, reg_mask = 0;
    नोंदणी शून्य **p_argv;
    नोंदणी ffi_type **p_arg;
    नोंदणी अक्षर *reg_base = चळत;
    नोंदणी अक्षर *चळत_base = चळत + 20;
    नोंदणी अचिन्हित पूर्णांक चळत_offset = 0;

#ifdef DEBUG
    /* Debugging */
    साठी(i = 0; i < cif->nargs + 7; i++)
    {
        म_लिखो("sp+%d: 0x%08x\n", i*4, ((अचिन्हित पूर्णांक*)चळत)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    जर(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(शून्य **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    साठी(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        माप_प्रकार z = (*p_arg)->size;
        पूर्णांक alignment = (*p_arg)->alignment;

        *p_argv = 0;

        जर(z % 4 != 0)
            z += (4 - z % 4);

        जर(reg_mask != 0x1f)
        {
            जर(pass_struct_on_चळत(*p_arg))
            {
                *p_argv = (शून्य*)चळत_base + चळत_offset;
                चळत_offset += z;
            }
            अन्यथा जर(z <= आकार(पूर्णांक))
            {
                अक्षर index = 0;

                पर्यंत((reg_mask >> index) & 1)
                    index++;

                *p_argv = (शून्य*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            अन्यथा जर(z == 2 * आकार(पूर्णांक))
            {
                जर(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (शून्य*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                अन्यथा जर(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (शून्य*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        जर(!*p_argv)
        {
            *p_argv = (शून्य*)चळत_base + चळत_offset;
            चळत_offset += z;
        }

        जर((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == रिक्त)
        {
            जर(alignment == 1)
                **(अचिन्हित पूर्णांक**)p_argv <<= 24;
            अन्यथा जर(alignment == 2)
                **(अचिन्हित पूर्णांक**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    साठी(i = 0; i < cif->nargs; i++)
    {
        म_लिखो("sp+%d: 0x%08x\n", i*4, *(((अचिन्हित पूर्णांक**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

अचिन्हित पूर्णांक ffi_closure_SYSV_inner(ffi_closure *closure, शून्य **respp,
    शून्य *args)
{
    ffi_cif *cif;
    शून्य **arg_area;
    अचिन्हित पूर्णांक i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    साठी(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (शून्य **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    परत cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*), शून्य *user_data,
    शून्य *codeloc)
{
    जर (cif->abi != FFI_SYSV)
      परत FFI_BAD_ABI;

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
    closure->user_data = user_data;
    closure->fun  = fun;

    परत FFI_OK;
}

