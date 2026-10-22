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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <asm/unistd.h>

/* #define DEBUG */

બાહ્ય શૂન્ય ffi_call_SYSV(શૂન્ય (*)(અક્ષર *, extended_cif *), extended_cif *,
    unsigned પૂર્ણાંક, unsigned પૂર્ણાંક, unsigned પૂર્ણાંક*, unsigned પૂર્ણાંક,
    શૂન્ય (*fn)(શૂન્ય));
બાહ્ય શૂન્ય ffi_closure_SYSV (ffi_closure *);

unsigned પૂર્ણાંક pass_struct_on_ઢગલો(ffi_type *type)
{
    જો(type->type != FFI_TYPE_STRUCT)
        પાછા 0;

    જો(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        પાછા 1;

    જો(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        પાછા 1;

    પાછા 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

શૂન્ય ffi_prep_args(અક્ષર *ઢગલો, extended_cif *ecif)
{
    unsigned પૂર્ણાંક i;
    શૂન્ય **p_argv;
    ffi_type **p_arg;
    અક્ષર *reg_base = ઢગલો;
    અક્ષર *ઢગલો_base = ઢગલો + 20;
    unsigned પૂર્ણાંક ઢગલો_offset = 0;
    unsigned પૂર્ણાંક reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    જો(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(શૂન્ય**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    માટે(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        પૂર્ણાંક alignment = (*p_arg)->alignment;
        પૂર્ણાંક type = (*p_arg)->type;
        અક્ષર *addr = 0;

        જો(z % 4 != 0)
            z += (4 - z % 4);

        જો(reg_mask != 0x1f)
        {
            જો(pass_struct_on_ઢગલો(*p_arg))
            {
                addr = ઢગલો_base + ઢગલો_offset;
                ઢગલો_offset += z;
            }
            નહીંતર જો(z == કદ(પૂર્ણાંક))
            {
                અક્ષર index = 0;

                જ્યારે((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            નહીંતર જો(z == 2 * કદ(પૂર્ણાંક))
            {
                જો(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                નહીંતર જો(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        જો(!addr)
        {
            addr = ઢગલો_base + ઢગલો_offset;
            ઢગલો_offset += z;
        }

        જો(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == ખાલી)
            type = (*p_arg)->elements[0]->type;

        પસંદગી(type)
        {
        સ્થિતિ FFI_TYPE_UINT8:
            *(unsigned પૂર્ણાંક *)addr = (unsigned પૂર્ણાંક)*(UINT8 *)(*p_argv);
            તોડો;
        સ્થિતિ FFI_TYPE_SINT8:
            *(signed પૂર્ણાંક *)addr = (signed પૂર્ણાંક)*(SINT8 *)(*p_argv);
            તોડો;
        સ્થિતિ FFI_TYPE_UINT16:
            *(unsigned પૂર્ણાંક *)addr = (unsigned પૂર્ણાંક)*(UINT16 *)(*p_argv);
            તોડો;
        સ્થિતિ FFI_TYPE_SINT16:
            *(signed પૂર્ણાંક *)addr = (signed પૂર્ણાંક)*(SINT16 *)(*p_argv);
            તોડો;
        મૂળભૂત:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    માટે(i = 0; i < 5; i++)
    {
        જો((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        નહીંતર
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned પૂર્ણાંક*)reg_base)[i]);
    }

    માટે(i = 0; i < ઢગલો_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned પૂર્ણાંક*)ઢગલો_base)[i]);
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
    પસંદગી(cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_SINT8:
    સ્થિતિ FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        તોડો;
    સ્થિતિ FFI_TYPE_SINT16:
    સ્થિતિ FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        તોડો;
    સ્થિતિ FFI_TYPE_FLOAT:
    સ્થિતિ FFI_TYPE_SINT32:
    સ્થિતિ FFI_TYPE_UINT32:
    સ્થિતિ FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        તોડો;
    સ્થિતિ FFI_TYPE_DOUBLE:
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        તોડો;
    સ્થિતિ FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        જો(!pass_struct_on_ઢગલો(cif->rtype))
        {
            જો(cif->rtype->size <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            નહીંતર જો(cif->rtype->size <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            નહીંતર જો(cif->rtype->size <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            નહીંતર જો(cif->rtype->size <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            નહીંતર
                cif->flags = (unsigned)cif->rtype->type;
        }
        નહીંતર
            cif->flags = (unsigned)cif->rtype->type;
        તોડો;
    મૂળભૂત:
        cif->flags = (unsigned)cif->rtype->type;
        તોડો;
    }

    પાછા FFI_OK;
}

શૂન્ય ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
    extended_cif ecif;

    unsigned પૂર્ણાંક size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    માટે(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    જો((rvalue == ખાલી) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    નહીંતર
        ecif.rvalue = rvalue;

    પસંદગી(cif->abi)
    {
    સ્થિતિ FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        તોડો;
    મૂળભૂત:
        FFI_ASSERT(0);
        તોડો;
    }
}

સ્થિર શૂન્ય ffi_prep_incoming_args_SYSV(અક્ષર *ઢગલો, શૂન્ય **rvalue,
    શૂન્ય **avalue, ffi_cif *cif)
{
    નોંધણી unsigned પૂર્ણાંક i, reg_mask = 0;
    નોંધણી શૂન્ય **p_argv;
    નોંધણી ffi_type **p_arg;
    નોંધણી અક્ષર *reg_base = ઢગલો;
    નોંધણી અક્ષર *ઢગલો_base = ઢગલો + 20;
    નોંધણી unsigned પૂર્ણાંક ઢગલો_offset = 0;

#ifdef DEBUG
    /* Debugging */
    માટે(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned પૂર્ણાંક*)ઢગલો)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    જો(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(શૂન્ય **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    માટે(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        પૂર્ણાંક alignment = (*p_arg)->alignment;

        *p_argv = 0;

        જો(z % 4 != 0)
            z += (4 - z % 4);

        જો(reg_mask != 0x1f)
        {
            જો(pass_struct_on_ઢગલો(*p_arg))
            {
                *p_argv = (શૂન્ય*)ઢગલો_base + ઢગલો_offset;
                ઢગલો_offset += z;
            }
            નહીંતર જો(z <= કદ(પૂર્ણાંક))
            {
                અક્ષર index = 0;

                જ્યારે((reg_mask >> index) & 1)
                    index++;

                *p_argv = (શૂન્ય*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            નહીંતર જો(z == 2 * કદ(પૂર્ણાંક))
            {
                જો(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (શૂન્ય*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                નહીંતર જો(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (શૂન્ય*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        જો(!*p_argv)
        {
            *p_argv = (શૂન્ય*)ઢગલો_base + ઢગલો_offset;
            ઢગલો_offset += z;
        }

        જો((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == ખાલી)
        {
            જો(alignment == 1)
                **(unsigned પૂર્ણાંક**)p_argv <<= 24;
            નહીંતર જો(alignment == 2)
                **(unsigned પૂર્ણાંક**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    માટે(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned પૂર્ણાંક**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned પૂર્ણાંક ffi_closure_SYSV_inner(ffi_closure *closure, શૂન્ય **respp,
    શૂન્ય *args)
{
    ffi_cif *cif;
    શૂન્ય **arg_area;
    unsigned પૂર્ણાંક i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    માટે(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (શૂન્ય **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    પાછા cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*), શૂન્ય *user_data,
    શૂન્ય *codeloc)
{
    જો (cif->abi != FFI_SYSV)
      પાછા FFI_BAD_ABI;

    unsigned અક્ષર *__tramp = (unsigned અક્ષર*)(&closure->tramp[0]);
    unsigned પૂર્ણાંક  __fun = (unsigned પૂર્ણાંક)(&ffi_closure_SYSV);
    unsigned પૂર્ણાંક  __ctx = (unsigned પૂર્ણાંક)(codeloc);
    unsigned પૂર્ણાંક  __rstruct_flag = (unsigned પૂર્ણાંક)(cif->rstruct_flag);
    unsigned પૂર્ણાંક  __inner = (unsigned પૂર્ણાંક)(&ffi_closure_SYSV_inner);
    *(unsigned પૂર્ણાંક*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned પૂર્ણાંક*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned પૂર્ણાંક*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned પૂર્ણાંક*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned પૂર્ણાંક*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned પૂર્ણાંક*) &__tramp[20] = __ctx;
    *(unsigned પૂર્ણાંક*) &__tramp[24] = __rstruct_flag;
    *(unsigned પૂર્ણાંક*) &__tramp[28] = __inner;
    *(unsigned પૂર્ણાંક*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_data = user_data;
    closure->fun  = fun;

    પાછા FFI_OK;
}

