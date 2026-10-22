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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/unistd.h>

/* #define DEBUG */

ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV(ਖਾਲੀ (*)(ਅੱਖਰ *, extended_cif *), extended_cif *,
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ,
    ਖਾਲੀ (*fn)(ਖਾਲੀ));
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_SYSV (ffi_ਘੇਰਾ *);

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pass_struct_on_ਢੇਰ(ffi_type *type)
{
    ਜੇ(type->type != FFI_TYPE_STRUCT)
        ਵਾਪਸ 0;

    ਜੇ(type->alignment < type->ਮਾਪ &&
        !(type->ਮਾਪ == 4 || type->ਮਾਪ == 8) &&
        !(type->ਮਾਪ == 8 && type->alignment >= 4))
        ਵਾਪਸ 1;

    ਜੇ(type->ਮਾਪ == 3 || type->ਮਾਪ == 5 || type->ਮਾਪ == 6 ||
        type->ਮਾਪ == 7)
        ਵਾਪਸ 1;

    ਵਾਪਸ 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

ਖਾਲੀ ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, extended_cif *ecif)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
    ਖਾਲੀ **p_argv;
    ffi_type **p_arg;
    ਅੱਖਰ *reg_ਅਧਾਰ = ਢੇਰ;
    ਅੱਖਰ *ਢੇਰ_ਅਧਾਰ = ਢੇਰ + 20;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਢੇਰ_ਵਿੱਥ = 0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ reg_ਨਕਾਬ = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    ਜੇ(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(ਖਾਲੀ**)reg_ਅਧਾਰ = ecif->rvalue;
        reg_ਨਕਾਬ |= 1;
    }

    ਲਈ(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->ਮਾਪ;
        ਪੂਰਨ alignment = (*p_arg)->alignment;
        ਪੂਰਨ type = (*p_arg)->type;
        ਅੱਖਰ *addr = 0;

        ਜੇ(z % 4 != 0)
            z += (4 - z % 4);

        ਜੇ(reg_ਨਕਾਬ != 0x1f)
        {
            ਜੇ(pass_struct_on_ਢੇਰ(*p_arg))
            {
                addr = ਢੇਰ_ਅਧਾਰ + ਢੇਰ_ਵਿੱਥ;
                ਢੇਰ_ਵਿੱਥ += z;
            }
            ਨਹੀਂਤਾਂ ਜੇ(z == ਆਕਾਰ(ਪੂਰਨ))
            {
                ਅੱਖਰ ਸੂਚਕਾਂਕ = 0;

                ਜਦੋਂ((reg_ਨਕਾਬ >> ਸੂਚਕਾਂਕ) & 1)
                    ਸੂਚਕਾਂਕ++;

                addr = reg_ਅਧਾਰ + (ਸੂਚਕਾਂਕ * 4);
                reg_ਨਕਾਬ |= (1 << ਸੂਚਕਾਂਕ);
            }
            ਨਹੀਂਤਾਂ ਜੇ(z == 2 * ਆਕਾਰ(ਪੂਰਨ))
            {
                ਜੇ(!((reg_ਨਕਾਬ >> 1) & 1))
                {
                    addr = reg_ਅਧਾਰ + 4;
                    reg_ਨਕਾਬ |= (3 << 1);
                }
                ਨਹੀਂਤਾਂ ਜੇ(!((reg_ਨਕਾਬ >> 3) & 1))
                {
                    addr = reg_ਅਧਾਰ + 12;
                    reg_ਨਕਾਬ |= (3 << 3);
                }
            }
        }

        ਜੇ(!addr)
        {
            addr = ਢੇਰ_ਅਧਾਰ + ਢੇਰ_ਵਿੱਥ;
            ਢੇਰ_ਵਿੱਥ += z;
        }

        ਜੇ(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == ਨਿਰਾ)
            type = (*p_arg)->elements[0]->type;

        ਚੋਣ(type)
        {
        ਹਾਲਤ FFI_TYPE_UINT8:
            *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)addr = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT8 *)(*p_argv);
            ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_SINT8:
            *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *)addr = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT8 *)(*p_argv);
            ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_UINT16:
            *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)addr = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT16 *)(*p_argv);
            ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_SINT16:
            *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *)addr = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT16 *)(*p_argv);
            ਰੋਕੋ;
        ਮੂਲ:
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(addr, *p_argv, z);
        }

        p_argv++;
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
    /* Debugging */
    ਲਈ(i = 0; i < 5; i++)
    {
        ਜੇ((reg_ਨਕਾਬ & (1 << i)) == 0)
            ਲਿਖੋ("r%d: (unused)\n", 12 - i);
        ਨਹੀਂਤਾਂ
            ਲਿਖੋ("r%d: 0x%08x\n", 12 - i, ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*)reg_ਅਧਾਰ)[i]);
    }

    ਲਈ(i = 0; i < ਢੇਰ_ਵਿੱਥ / 4; i++)
    {
        ਲਿਖੋ("sp+%d: 0x%08x\n", i*4, ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*)ਢੇਰ_ਅਧਾਰ)[i]);
    }
#ਜੇ_ਅੰਤ
}

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
    /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     * everywhere, but it is on some platforms, and it doesn't harm
     * anything when it isn't needed. */
    cif->ਬਾਈਟਾਂ = (cif->ਬਾਈਟਾਂ + 7) & ~7;

    /* Flag to indicate that he return value is in fact a struct */
    cif->rstruct_flag = 0;

    /* Set the return type flag */
    ਚੋਣ(cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_SINT8:
    ਹਾਲਤ FFI_TYPE_UINT8:
        cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_UINT8;
        ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
    ਹਾਲਤ FFI_TYPE_UINT16:
        cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_UINT16;
        ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_UINT32:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
        cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_UINT32;
        ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
        cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_UINT64;
        ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        ਜੇ(!pass_struct_on_ਢੇਰ(cif->rtype))
        {
            ਜੇ(cif->rtype->ਮਾਪ <= 1)
                cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_UINT8;
            ਨਹੀਂਤਾਂ ਜੇ(cif->rtype->ਮਾਪ <= 2)
                cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_UINT16;
            ਨਹੀਂਤਾਂ ਜੇ(cif->rtype->ਮਾਪ <= 4)
                cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_UINT32;
            ਨਹੀਂਤਾਂ ਜੇ(cif->rtype->ਮਾਪ <= 8)
                cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_UINT64;
            ਨਹੀਂਤਾਂ
                cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)cif->rtype->type;
        }
        ਨਹੀਂਤਾਂ
            cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)cif->rtype->type;
        ਰੋਕੋ;
    ਮੂਲ:
        cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)cif->rtype->type;
        ਰੋਕੋ;
    }

    ਵਾਪਸ FFI_OK;
}

ਖਾਲੀ ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
    extended_cif ecif;

    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    ਲਈ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        ਮਾਪ += (*p_arg)->ਮਾਪ + (4 - (*p_arg)->ਮਾਪ % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    ਜੇ((rvalue == ਨਿਰਾ) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->ਮਾਪ);
    ਨਹੀਂਤਾਂ
        ecif.rvalue = rvalue;

    ਚੋਣ(cif->abi)
    {
    ਹਾਲਤ FFI_SYSV:
        ffi_call_SYSV(ffi_prep_ਦਲੀਲਾਂ, &ecif, ਮਾਪ, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        ਰੋਕੋ;
    ਮੂਲ:
        FFI_ASSERT(0);
        ਰੋਕੋ;
    }
}

ਸਥਿਰ ਖਾਲੀ ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV(ਅੱਖਰ *ਢੇਰ, ਖਾਲੀ **rvalue,
    ਖਾਲੀ **avalue, ffi_cif *cif)
{
    ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, reg_ਨਕਾਬ = 0;
    ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
    ਰਜਿਸਟਰ ffi_type **p_arg;
    ਰਜਿਸਟਰ ਅੱਖਰ *reg_ਅਧਾਰ = ਢੇਰ;
    ਰਜਿਸਟਰ ਅੱਖਰ *ਢੇਰ_ਅਧਾਰ = ਢੇਰ + 20;
    ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਢੇਰ_ਵਿੱਥ = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
    /* Debugging */
    ਲਈ(i = 0; i < cif->nargs + 7; i++)
    {
        ਲਿਖੋ("sp+%d: 0x%08x\n", i*4, ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*)ਢੇਰ)[i]);
    }
#ਜੇ_ਅੰਤ

    /* If cif->flags is struct then we know it's not passed in registers */
    ਜੇ(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(ਖਾਲੀ **)reg_ਅਧਾਰ;
        reg_ਨਕਾਬ |= 1;
    }

    p_argv = avalue;

    ਲਈ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->ਮਾਪ;
        ਪੂਰਨ alignment = (*p_arg)->alignment;

        *p_argv = 0;

        ਜੇ(z % 4 != 0)
            z += (4 - z % 4);

        ਜੇ(reg_ਨਕਾਬ != 0x1f)
        {
            ਜੇ(pass_struct_on_ਢੇਰ(*p_arg))
            {
                *p_argv = (ਖਾਲੀ*)ਢੇਰ_ਅਧਾਰ + ਢੇਰ_ਵਿੱਥ;
                ਢੇਰ_ਵਿੱਥ += z;
            }
            ਨਹੀਂਤਾਂ ਜੇ(z <= ਆਕਾਰ(ਪੂਰਨ))
            {
                ਅੱਖਰ ਸੂਚਕਾਂਕ = 0;

                ਜਦੋਂ((reg_ਨਕਾਬ >> ਸੂਚਕਾਂਕ) & 1)
                    ਸੂਚਕਾਂਕ++;

                *p_argv = (ਖਾਲੀ*)reg_ਅਧਾਰ + (ਸੂਚਕਾਂਕ * 4);
                reg_ਨਕਾਬ |= (1 << ਸੂਚਕਾਂਕ);
            }
            ਨਹੀਂਤਾਂ ਜੇ(z == 2 * ਆਕਾਰ(ਪੂਰਨ))
            {
                ਜੇ(!((reg_ਨਕਾਬ >> 1) & 1))
                {
                    *p_argv = (ਖਾਲੀ*)reg_ਅਧਾਰ + 4;
                    reg_ਨਕਾਬ |= (3 << 1);
                }
                ਨਹੀਂਤਾਂ ਜੇ(!((reg_ਨਕਾਬ >> 3) & 1))
                {
                    *p_argv = (ਖਾਲੀ*)reg_ਅਧਾਰ + 12;
                    reg_ਨਕਾਬ |= (3 << 3);
                }
            }
        }

        ਜੇ(!*p_argv)
        {
            *p_argv = (ਖਾਲੀ*)ਢੇਰ_ਅਧਾਰ + ਢੇਰ_ਵਿੱਥ;
            ਢੇਰ_ਵਿੱਥ += z;
        }

        ਜੇ((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == ਨਿਰਾ)
        {
            ਜੇ(alignment == 1)
                **(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ**)p_argv <<= 24;
            ਨਹੀਂਤਾਂ ਜੇ(alignment == 2)
                **(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ**)p_argv <<= 16;
        }

        p_argv++;
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
    /* Debugging */
    ਲਈ(i = 0; i < cif->nargs; i++)
    {
        ਲਿਖੋ("sp+%d: 0x%08x\n", i*4, *(((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ**)avalue)[i]));
    }
#ਜੇ_ਅੰਤ
}

/* This function is jumped to by the trampoline */

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ffi_ਘੇਰਾ_SYSV_inner(ffi_ਘੇਰਾ *ਘੇਰਾ, ਖਾਲੀ **respp,
    ਖਾਲੀ *ਦਲੀਲਾਂ)
{
    ffi_cif *cif;
    ਖਾਲੀ **arg_area;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, ਮਾਪ = 0;
    ffi_type **p_arg;

    cif = ਘੇਰਾ->cif;

    ਲਈ(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        ਮਾਪ += (*p_arg)->ਮਾਪ + (4 - (*p_arg)->ਮਾਪ % 4);

    arg_area = (ਖਾਲੀ **)alloca(ਮਾਪ);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV(ਦਲੀਲਾਂ, respp, arg_area, cif);

    (ਘੇਰਾ->fun)(cif, *respp, arg_area, ਘੇਰਾ->user_ਅੰਕੜੇ);

    ਵਾਪਸ cif->flags;
}

ffi_ਸਥਿਤੀ ffi_prep_ਘੇਰਾ_loc(ffi_ਘੇਰਾ* ਘੇਰਾ, ffi_cif* cif,
    ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*), ਖਾਲੀ *user_ਅੰਕੜੇ,
    ਖਾਲੀ *codeloc)
{
    ਜੇ (cif->abi != FFI_SYSV)
      ਵਾਪਸ FFI_BAD_ABI;

    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *__tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)(&ਘੇਰਾ->tramp[0]);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ  __fun = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(&ffi_ਘੇਰਾ_SYSV);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ  __ctx = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(codeloc);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ  __rstruct_flag = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(cif->rstruct_flag);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ  __inner = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(&ffi_ਘੇਰਾ_SYSV_inner);
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[20] = __ctx;
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[24] = __rstruct_flag;
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[28] = __inner;
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    ਘੇਰਾ->cif = cif;
    ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
    ਘੇਰਾ->fun  = fun;

    ਵਾਪਸ FFI_OK;
}

