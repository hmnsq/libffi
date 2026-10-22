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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <asm/unistd.h>

/* #define DEBUG */

வெளி வெறுமை ffi_call_SYSV(வெறுமை (*)(எழுத்து *, extended_cif *), extended_cif *,
    unsigned முழு, unsigned முழு, unsigned முழு*, unsigned முழு,
    வெறுமை (*fn)(வெறுமை));
வெளி வெறுமை ffi_closure_SYSV (ffi_closure *);

unsigned முழு pass_struct_on_அடுக்கு(ffi_type *type)
{
    எனில்(type->type != FFI_TYPE_STRUCT)
        திரும்பு 0;

    எனில்(type->alignment < type->size &&
        !(type->size == 4 || type->size == 8) &&
        !(type->size == 8 && type->alignment >= 4))
        திரும்பு 1;

    எனில்(type->size == 3 || type->size == 5 || type->size == 6 ||
        type->size == 7)
        திரும்பு 1;

    திரும்பு 0;
}

/* ffi_prep_args is called by the assembly routine once stack space
 * has been allocated for the function's arguments
 *
 * This is annoyingly complex since we need to keep track of used
 * registers.
 */

வெறுமை ffi_prep_args(எழுத்து *அடுக்கு, extended_cif *ecif)
{
    unsigned முழு i;
    வெறுமை **p_argv;
    ffi_type **p_arg;
    எழுத்து *reg_base = அடுக்கு;
    எழுத்து *அடுக்கு_base = அடுக்கு + 20;
    unsigned முழு அடுக்கு_offset = 0;
    unsigned முழு reg_mask = 0;

    p_argv = ecif->avalue;

    /* If cif->flags is struct then we know it's not passed in registers */
    எனில்(ecif->cif->flags == FFI_TYPE_STRUCT)
    {
        *(வெறுமை**)reg_base = ecif->rvalue;
        reg_mask |= 1;
    }

    க்கு(i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs;
        i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        முழு alignment = (*p_arg)->alignment;
        முழு type = (*p_arg)->type;
        எழுத்து *addr = 0;

        எனில்(z % 4 != 0)
            z += (4 - z % 4);

        எனில்(reg_mask != 0x1f)
        {
            எனில்(pass_struct_on_அடுக்கு(*p_arg))
            {
                addr = அடுக்கு_base + அடுக்கு_offset;
                அடுக்கு_offset += z;
            }
            இல்லையெனில் எனில்(z == அளவு(முழு))
            {
                எழுத்து index = 0;

                போது((reg_mask >> index) & 1)
                    index++;

                addr = reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            இல்லையெனில் எனில்(z == 2 * அளவு(முழு))
            {
                எனில்(!((reg_mask >> 1) & 1))
                {
                    addr = reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                இல்லையெனில் எனில்(!((reg_mask >> 3) & 1))
                {
                    addr = reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        எனில்(!addr)
        {
            addr = அடுக்கு_base + அடுக்கு_offset;
            அடுக்கு_offset += z;
        }

        எனில்(type == FFI_TYPE_STRUCT && (*p_arg)->elements[1] == வெற்று)
            type = (*p_arg)->elements[0]->type;

        தேர்வு(type)
        {
        நிலை FFI_TYPE_UINT8:
            *(unsigned முழு *)addr = (unsigned முழு)*(UINT8 *)(*p_argv);
            நிறுத்து;
        நிலை FFI_TYPE_SINT8:
            *(signed முழு *)addr = (signed முழு)*(SINT8 *)(*p_argv);
            நிறுத்து;
        நிலை FFI_TYPE_UINT16:
            *(unsigned முழு *)addr = (unsigned முழு)*(UINT16 *)(*p_argv);
            நிறுத்து;
        நிலை FFI_TYPE_SINT16:
            *(signed முழு *)addr = (signed முழு)*(SINT16 *)(*p_argv);
            நிறுத்து;
        இயல்பு:
            memcpy(addr, *p_argv, z);
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    க்கு(i = 0; i < 5; i++)
    {
        எனில்((reg_mask & (1 << i)) == 0)
            printf("r%d: (unused)\n", 12 - i);
        இல்லையெனில்
            printf("r%d: 0x%08x\n", 12 - i, ((unsigned முழு*)reg_base)[i]);
    }

    க்கு(i = 0; i < அடுக்கு_offset / 4; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned முழு*)அடுக்கு_base)[i]);
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
    தேர்வு(cif->rtype->type)
    {
    நிலை FFI_TYPE_SINT8:
    நிலை FFI_TYPE_UINT8:
        cif->flags = (unsigned)FFI_TYPE_UINT8;
        நிறுத்து;
    நிலை FFI_TYPE_SINT16:
    நிலை FFI_TYPE_UINT16:
        cif->flags = (unsigned)FFI_TYPE_UINT16;
        நிறுத்து;
    நிலை FFI_TYPE_FLOAT:
    நிலை FFI_TYPE_SINT32:
    நிலை FFI_TYPE_UINT32:
    நிலை FFI_TYPE_POINTER:
        cif->flags = (unsigned)FFI_TYPE_UINT32;
        நிறுத்து;
    நிலை FFI_TYPE_DOUBLE:
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
        cif->flags = (unsigned)FFI_TYPE_UINT64;
        நிறுத்து;
    நிலை FFI_TYPE_STRUCT:
        cif->rstruct_flag = 1;
        எனில்(!pass_struct_on_அடுக்கு(cif->rtype))
        {
            எனில்(cif->rtype->size <= 1)
                cif->flags = (unsigned)FFI_TYPE_UINT8;
            இல்லையெனில் எனில்(cif->rtype->size <= 2)
                cif->flags = (unsigned)FFI_TYPE_UINT16;
            இல்லையெனில் எனில்(cif->rtype->size <= 4)
                cif->flags = (unsigned)FFI_TYPE_UINT32;
            இல்லையெனில் எனில்(cif->rtype->size <= 8)
                cif->flags = (unsigned)FFI_TYPE_UINT64;
            இல்லையெனில்
                cif->flags = (unsigned)cif->rtype->type;
        }
        இல்லையெனில்
            cif->flags = (unsigned)cif->rtype->type;
        நிறுத்து;
    இயல்பு:
        cif->flags = (unsigned)cif->rtype->type;
        நிறுத்து;
    }

    திரும்பு FFI_OK;
}

வெறுமை ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
    extended_cif ecif;

    unsigned முழு size = 0, i = 0;
    ffi_type **p_arg;

    ecif.cif = cif;
    ecif.avalue = avalue;

    க்கு(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    /* If the return value is a struct and we don't have a return value
     * address then we need to make one */

    /* If cif->flags is struct then it's not suitable for registers */
    எனில்((rvalue == வெற்று) && (cif->flags == FFI_TYPE_STRUCT))
        ecif.rvalue = alloca(cif->rtype->size);
    இல்லையெனில்
        ecif.rvalue = rvalue;

    தேர்வு(cif->abi)
    {
    நிலை FFI_SYSV:
        ffi_call_SYSV(ffi_prep_args, &ecif, size, cif->flags,
            ecif.rvalue, cif->rstruct_flag, fn);
        நிறுத்து;
    இயல்பு:
        FFI_ASSERT(0);
        நிறுத்து;
    }
}

நிலையான வெறுமை ffi_prep_incoming_args_SYSV(எழுத்து *அடுக்கு, வெறுமை **rvalue,
    வெறுமை **avalue, ffi_cif *cif)
{
    பதிவு unsigned முழு i, reg_mask = 0;
    பதிவு வெறுமை **p_argv;
    பதிவு ffi_type **p_arg;
    பதிவு எழுத்து *reg_base = அடுக்கு;
    பதிவு எழுத்து *அடுக்கு_base = அடுக்கு + 20;
    பதிவு unsigned முழு அடுக்கு_offset = 0;

#ifdef DEBUG
    /* Debugging */
    க்கு(i = 0; i < cif->nargs + 7; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, ((unsigned முழு*)அடுக்கு)[i]);
    }
#endif

    /* If cif->flags is struct then we know it's not passed in registers */
    எனில்(cif->flags == FFI_TYPE_STRUCT)
    {
        *rvalue = *(வெறுமை **)reg_base;
        reg_mask |= 1;
    }

    p_argv = avalue;

    க்கு(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
    {
        size_t z = (*p_arg)->size;
        முழு alignment = (*p_arg)->alignment;

        *p_argv = 0;

        எனில்(z % 4 != 0)
            z += (4 - z % 4);

        எனில்(reg_mask != 0x1f)
        {
            எனில்(pass_struct_on_அடுக்கு(*p_arg))
            {
                *p_argv = (வெறுமை*)அடுக்கு_base + அடுக்கு_offset;
                அடுக்கு_offset += z;
            }
            இல்லையெனில் எனில்(z <= அளவு(முழு))
            {
                எழுத்து index = 0;

                போது((reg_mask >> index) & 1)
                    index++;

                *p_argv = (வெறுமை*)reg_base + (index * 4);
                reg_mask |= (1 << index);
            }
            இல்லையெனில் எனில்(z == 2 * அளவு(முழு))
            {
                எனில்(!((reg_mask >> 1) & 1))
                {
                    *p_argv = (வெறுமை*)reg_base + 4;
                    reg_mask |= (3 << 1);
                }
                இல்லையெனில் எனில்(!((reg_mask >> 3) & 1))
                {
                    *p_argv = (வெறுமை*)reg_base + 12;
                    reg_mask |= (3 << 3);
                }
            }
        }

        எனில்(!*p_argv)
        {
            *p_argv = (வெறுமை*)அடுக்கு_base + அடுக்கு_offset;
            அடுக்கு_offset += z;
        }

        எனில்((*p_arg)->type != FFI_TYPE_STRUCT ||
            (*p_arg)->elements[1] == வெற்று)
        {
            எனில்(alignment == 1)
                **(unsigned முழு**)p_argv <<= 24;
            இல்லையெனில் எனில்(alignment == 2)
                **(unsigned முழு**)p_argv <<= 16;
        }

        p_argv++;
    }

#ifdef DEBUG
    /* Debugging */
    க்கு(i = 0; i < cif->nargs; i++)
    {
        printf("sp+%d: 0x%08x\n", i*4, *(((unsigned முழு**)avalue)[i]));
    }
#endif
}

/* This function is jumped to by the trampoline */

unsigned முழு ffi_closure_SYSV_inner(ffi_closure *closure, வெறுமை **respp,
    வெறுமை *args)
{
    ffi_cif *cif;
    வெறுமை **arg_area;
    unsigned முழு i, size = 0;
    ffi_type **p_arg;

    cif = closure->cif;

    க்கு(i = 0, p_arg = cif->arg_types; i < cif->nargs; i++, p_arg++)
        size += (*p_arg)->size + (4 - (*p_arg)->size % 4);

    arg_area = (வெறுமை **)alloca(size);

    /* this call will initialize ARG_AREA, such that each element in that
     * array points to the corresponding value on the stack; and if the
     * function returns a structure, it will re-set RESP to point to the
     * structure return address. */

    ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

    (closure->fun)(cif, *respp, arg_area, closure->user_data);

    திரும்பு cif->flags;
}

ffi_status ffi_prep_closure_loc(ffi_closure* closure, ffi_cif* cif,
    வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*), வெறுமை *user_data,
    வெறுமை *codeloc)
{
    எனில் (cif->abi != FFI_SYSV)
      திரும்பு FFI_BAD_ABI;

    unsigned எழுத்து *__tramp = (unsigned எழுத்து*)(&closure->tramp[0]);
    unsigned முழு  __fun = (unsigned முழு)(&ffi_closure_SYSV);
    unsigned முழு  __ctx = (unsigned முழு)(codeloc);
    unsigned முழு  __rstruct_flag = (unsigned முழு)(cif->rstruct_flag);
    unsigned முழு  __inner = (unsigned முழு)(&ffi_closure_SYSV_inner);
    *(unsigned முழு*) &__tramp[0] = 0xebcd1f00;    /* pushm  r8-r12 */
    *(unsigned முழு*) &__tramp[4] = 0xfefc0010;    /* ld.w   r12, pc[16] */
    *(unsigned முழு*) &__tramp[8] = 0xfefb0010;    /* ld.w   r11, pc[16] */
    *(unsigned முழு*) &__tramp[12] = 0xfefa0010;   /* ld.w   r10, pc[16] */
    *(unsigned முழு*) &__tramp[16] = 0xfeff0010;   /* ld.w   pc, pc[16] */
    *(unsigned முழு*) &__tramp[20] = __ctx;
    *(unsigned முழு*) &__tramp[24] = __rstruct_flag;
    *(unsigned முழு*) &__tramp[28] = __inner;
    *(unsigned முழு*) &__tramp[32] = __fun;
    syscall(__NR_cacheflush, 0, (&__tramp[0]), 36);

    closure->cif = cif;
    closure->user_data = user_data;
    closure->fun  = fun;

    திரும்பு FFI_OK;
}

