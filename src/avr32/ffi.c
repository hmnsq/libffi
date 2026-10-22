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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <asm/unistd.h>

/* #define DEBUG */

ബാഹ്യ ശൂന്യം ffi_call_SYSV(ശൂന്യം (*)(അക്ഷരം *, extended_cif *), extended_cif *,
    unsigned പൂർണ്ണസംഖ്യ, unsigned പൂർണ്ണസംഖ്യ, unsigned പൂർണ്ണസംഖ്യ*, unsigned പൂർണ്ണസംഖ്യ,
    ശൂന്യം (*fn)(ശൂന്യം));
ബാഹ്യ ശൂന്യം ffi_closure_SYSV (ffi_closure *);

unsigned പൂർണ്ണസംഖ്യ pass_struct_on_അട്ടി(ffi_type *type)
{
    എങ്കിൽ(type->type != FFI_TYPE_STRUCT)
        തിരികെ 0;

    എങ്കിൽ(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        തിരികെ 1;

    എങ്കിൽ(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        തിരികെ 1;

    തിരികെ 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

ശൂന്യം ffi_prep_args(അക്ഷരം *അട്ടി, extended_cif *ecif)
{
    unsigned പൂർണ്ണസംഖ്യ i;
    ശൂന്യം **p_argv;
    ffi_type **p_arg;
    അക്ഷരം *reg_base = അട്ടി;
    അക്ഷരം *അട്ടി_base = അട്ടി + 20;
    unsigned പൂർണ്ണസംഖ്യ അട്ടി_offset = 0;
    unsigned പൂർണ്ണസംഖ്യ reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    എങ്കിൽ(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(ശൂന്യം**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    വേണ്ടി(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        പൂർണ്ണസംഖ്യ alignment = (*p_arg)->alignment;
        പൂർണ്ണസംഖ്യ type = (*p_arg)->type;
        അക്ഷരം *addr = 0;

        എങ്കിൽ(z % 4 != 0)
            z += (4 - z % 4);

        എങ്കിൽ(reg_mask != 0x1f)
        {
            എങ്കിൽ(pass_struct_on_അട്ടി(*p_arg))
            {
                addr = അട്ടി_base + അട്ടി_offset;
                അട്ടി_offset += z;
            }
            അല്ലെങ്കിൽ എങ്കിൽ(z == വലുപ്പം(പൂർണ്ണസംഖ്യ))
            {
                അക്ഷരം index = 0;

                ആകുമ്പോൾ((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            അല്ലെങ്കിൽ എങ്കിൽ(z == 2 * വലുപ്പം(പൂർണ്ണസംഖ്യ))
            {
                എങ്കിൽ(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                അല്ലെങ്കിൽ എങ്കിൽ(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        എങ്കിൽ(!addr)
        {
            addr = അട്ടി_base + അട്ടി_offset;
            അട്ടി_offset += z;
        }

        എങ്കിൽ(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == ശൂന്യ)
            type = (*p_arg)->elements[0]->type;

        തിരഞ്ഞെടുപ്പ്(type)
        {
        അവസ്ഥ FFI_TYPE_UINT8:
            *(unsigned പൂർണ്ണസംഖ്യ *)addr = (unsigned പൂർണ്ണസംഖ്യ)*(UINT8 *)(*p_argv);
            നിർത്തുക;
        അവസ്ഥ FFI_TYPE_SINT8:
            *(signed പൂർണ്ണസംഖ്യ *)addr = (signed പൂർണ്ണസംഖ്യ)*(SINT8 *)(*p_argv);
            നിർത്തുക;
        അവസ്ഥ FFI_TYPE_UINT16:
            *(unsigned പൂർണ്ണസംഖ്യ *)addr = (unsigned പൂർണ്ണസംഖ്യ)*(UINT16 *)(*p_argv);
            നിർത്തുക;
        അവസ്ഥ FFI_TYPE_SINT16:
            *(signed പൂർണ്ണസംഖ്യ *)addr = (signed പൂർണ്ണസംഖ്യ)*(SINT16 *)(*p_argv);
            നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    വേണ്ടി(i = 0; i < 5; i++)
    {
        എങ്കിൽ((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        അല്ലെങ്കിൽ
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned പൂർണ്ണസംഖ്യ*)reg_base)[i]);
    }

    വേണ്ടി(i = 0; i < അട്ടി_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned പൂർണ്ണസംഖ്യ*)അട്ടി_base)[i]);
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
    തിരഞ്ഞെടുപ്പ്(cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_SINT8:
    അവസ്ഥ FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16:
    അവസ്ഥ FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        നിർത്തുക;
    അവസ്ഥ FFI_TYPE_FLOAT:
    അവസ്ഥ FFI_TYPE_SINT32:
    അവസ്ഥ FFI_TYPE_UINT32:
    അവസ്ഥ FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        നിർത്തുക;
    അവസ്ഥ FFI_TYPE_DOUBLE:
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        നിർത്തുക;
    അവസ്ഥ FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        എങ്കിൽ(!pass_struct_on_അട്ടി(cif->rtype))
        {
            എങ്കിൽ(cif->rtype->size <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            അല്ലെങ്കിൽ എങ്കിൽ(cif->rtype->size <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            അല്ലെങ്കിൽ എങ്കിൽ(cif->rtype->size <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            അല്ലെങ്കിൽ എങ്കിൽ(cif->rtype->size <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            അല്ലെങ്കിൽ
                cif->flags = (unsigned)cif->rtype->type;
        }
        അല്ലെങ്കിൽ
            cif->flags = (unsigned)cif->rtype->type;
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
        cif->flags = (unsigned)cif->rtype->type;
        നിർത്തുക;
    }

    തിരികെ FFI_OK;
}

ശൂന്യം ffi_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
    extended_cif ecif;

    unsigned പൂർണ്ണസംഖ്യ size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    വേണ്ടി(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    എങ്കിൽ((rvalue == ശൂന്യ) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    അല്ലെങ്കിൽ
        ecif.rvalue = rvalue;

    തിരഞ്ഞെടുപ്പ്(cif->abi)
    {
    അവസ്ഥ FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
        FFI_ASSERT(0);
        നിർത്തുക;
    }
}

സ്ഥിതം ശൂന്യം ffi_prep_incoming_args_SYSV(അക്ഷരം *അട്ടി, ശൂന്യം **rvalue,
    ശൂന്യം **avalue, ffi_cif *cif)
{
    രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ i, reg_mask = 0;
    രജിസ്റ്റർ ശൂന്യം **p_argv;
    രജിസ്റ്റർ ffi_type **p_arg;
    രജിസ്റ്റർ അക്ഷരം *reg_base = അട്ടി;
    രജിസ്റ്റർ അക്ഷരം *അട്ടി_base = അട്ടി + 20;
    രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ അട്ടി_offset = 0;

#ifdef DEBUG
    /* Debugging */
    വേണ്ടി(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned പൂർണ്ണസംഖ്യ*)അട്ടി)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    എങ്കിൽ(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(ശൂന്യം **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    വേണ്ടി(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        പൂർണ്ണസംഖ്യ alignment = (*p_arg)->alignment;

        *p_argv = 0;

        എങ്കിൽ(z % 4 != 0)
            z += (4 - z % 4);

        എങ്കിൽ(reg_mask != 0x1f)
        {
            എങ്കിൽ(pass_struct_on_അട്ടി(*p_arg))
            {
                *p_argv = (ശൂന്യം*)അട്ടി_base + അട്ടി_offset;
                അട്ടി_offset += z;
            }
            അല്ലെങ്കിൽ എങ്കിൽ(z <= വലുപ്പം(പൂർണ്ണസംഖ്യ))
            {
                അക്ഷരം index = 0;

                ആകുമ്പോൾ((reg_mask >> index) & 1)
                    index++;

                *p_argv = (ശൂന്യം*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            അല്ലെങ്കിൽ എങ്കിൽ(z == 2 * വലുപ്പം(പൂർണ്ണസംഖ്യ))
            {
                എങ്കിൽ(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (ശൂന്യം*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                അല്ലെങ്കിൽ എങ്കിൽ(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (ശൂന്യം*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        എങ്കിൽ(!*p_argv)
        {
            *p_argv = (ശൂന്യം*)അട്ടി_base + അട്ടി_offset;
            അട്ടി_offset += z;
        }

        എങ്കിൽ((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == ശൂന്യ)
        {
            എങ്കിൽ(alignment == 1)
                **(unsigned പൂർണ്ണസംഖ്യ**)p_argv <<= 24;
            അല്ലെങ്കിൽ എങ്കിൽ(alignment == 2)
                **(unsigned പൂർണ്ണസംഖ്യ**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    വേണ്ടി(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned പൂർണ്ണസംഖ്യ**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned പൂർണ്ണസംഖ്യ ffi_closure_SYSV_inner(ffi_closure *closure, ശൂന്യം **respp,
    ശൂന്യം *args)
{
    ffi_cif *cif;
    ശൂന്യം **arg_area;
    unsigned പൂർണ്ണസംഖ്യ i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    വേണ്ടി(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (ശൂന്യം **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    തിരികെ cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*), ശൂന്യം *user_data,
    ശൂന്യം *codeloc)
{
    എങ്കിൽ (cif->abi != FFI_SYSV)
      തിരികെ FFI_BAD_ABI;

    unsigned അക്ഷരം *__tramp = (unsigned അക്ഷരം*)(&closure->tramp[0]);
    unsigned പൂർണ്ണസംഖ്യ  __fun = (unsigned പൂർണ്ണസംഖ്യ)(&ffi_closure_SYSV);
    unsigned പൂർണ്ണസംഖ്യ  __ctx = (unsigned പൂർണ്ണസംഖ്യ)(codeloc);
    unsigned പൂർണ്ണസംഖ്യ  __rstruct_flag = (unsigned പൂർണ്ണസംഖ്യ)(cif->rstruct_flag);
    unsigned പൂർണ്ണസംഖ്യ  __inner = (unsigned പൂർണ്ണസംഖ്യ)(&ffi_closure_SYSV_inner);
    *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[20] = __ctx;
    *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[24] = __rstruct_flag;
    *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[28] = __inner;
    *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_data = user_data;
    closure->fun  = fun;

    തിരികെ FFI_OK;
}

