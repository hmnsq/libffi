/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2012 Tilera Corp.

   TILE Foreign Function Interface

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
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <arch/abi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <arch/icache.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <arch/opcode.h>


/* The first 10 registers are used to pass arguments and return values. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕ_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
ਬਾਹਰੀ ਖਾਲੀ ffi_call_tile(ffi_sarg reg_ਦਲੀਲਾਂ[ਅੰਕ_ARG_REGS],
                          ਅਚਲ ffi_sarg *ਢੇਰ_ਦਲੀਲਾਂ,
                          size_t ਢੇਰ_ਦਲੀਲਾਂ_ਬਾਈਟਾਂ,
                          ਖਾਲੀ (*fnaddr)(ਖਾਲੀ))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_tile(ਖਾਲੀ) FFI_HIDDEN;


ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  ਜੇ (cif->ਬਾਈਟਾਂ < ਅੰਕ_ARG_REGS * FFI_SIZEOF_ARG)
    cif->ਬਾਈਟਾਂ = ਅੰਕ_ARG_REGS * FFI_SIZEOF_ARG;

  ਜੇ (cif->rtype->ਮਾਪ > ਅੰਕ_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  ਨਹੀਂਤਾਂ
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  ਵਾਪਸ FFI_OK;
}


ਸਥਿਰ ਲੰਮਾ
ਸੌਂਪੋ_ਮੰਜ਼ਿਲ_ffi_arg(ffi_sarg *ਬਾਹਰਲਾ, ਖਾਲੀ *ਵਿੱਚ_2, ਅਚਲ ffi_type *type,
                  ਪੂਰਨ write_ਮੰਜ਼ਿਲ_reg)
{
  ਚੋਣ (type->type)
    {
    ਹਾਲਤ FFI_TYPE_SINT8:
      *ਬਾਹਰਲਾ = *(SINT8 *)ਵਿੱਚ_2;
      ਵਾਪਸ 1;

    ਹਾਲਤ FFI_TYPE_UINT8:
      *ਬਾਹਰਲਾ = *(UINT8 *)ਵਿੱਚ_2;
      ਵਾਪਸ 1;

    ਹਾਲਤ FFI_TYPE_SINT16:
      *ਬਾਹਰਲਾ = *(SINT16 *)ਵਿੱਚ_2;
      ਵਾਪਸ 1;

    ਹਾਲਤ FFI_TYPE_UINT16:
      *ਬਾਹਰਲਾ = *(UINT16 *)ਵਿੱਚ_2;
      ਵਾਪਸ 1;

    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_UINT32:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LP64__
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
#ਜੇ_ਅੰਤ
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *ਬਾਹਰਲਾ = *(SINT32 *)ਵਿੱਚ_2;
      ਵਾਪਸ 1;

    ਹਾਲਤ FFI_TYPE_FLOAT:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __tilegx__
      ਜੇ (write_ਮੰਜ਼ਿਲ_reg)
        {
          /* Properly sign extend the value.  */
          ਸੰਘ { ਦਸ਼ਮਲਵ f; SINT32 s32; } val;
          val.f = *(ਦਸ਼ਮਲਵ *)ਵਿੱਚ_2;
          *ਬਾਹਰਲਾ = val.s32;
        }
      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
        {
          *(ਦਸ਼ਮਲਵ *)ਬਾਹਰਲਾ = *(ਦਸ਼ਮਲਵ *)ਵਿੱਚ_2;
        }
      ਵਾਪਸ 1;

    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LP64__
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
#ਜੇ_ਅੰਤ
      *(UINT64 *)ਬਾਹਰਲਾ = *(UINT64 *)ਵਿੱਚ_2;
      ਵਾਪਸ ਆਕਾਰ(UINT64) / FFI_SIZEOF_ARG;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ, ਵਿੱਚ_2, type->ਮਾਪ);
      ਵਾਪਸ (type->ਮਾਪ + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

    ਹਾਲਤ FFI_TYPE_VOID:
      /* Must be a return type. Nothing to do. */
      ਵਾਪਸ 0;

    ਮੂਲ:
      FFI_ASSERT(0);
      ਵਾਪਸ -1;
    }
}


ਖਾਲੀ
ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_sarg * ਅਚਲ arg_mem = alloca(cif->ਬਾਈਟਾਂ);
  ffi_sarg * ਅਚਲ reg_ਦਲੀਲਾਂ = arg_mem;
  ffi_sarg * ਅਚਲ ਢੇਰ_ਦਲੀਲਾਂ = &reg_ਦਲੀਲਾਂ[ਅੰਕ_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** ਅਚਲ arg_types = cif->arg_types;
  ਅਚਲ ਲੰਮਾ ਅੰਕ_ਦਲੀਲਾਂ = cif->nargs;
  ਲੰਮਾ i;

  ਜੇ (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->ਮਾਪ));

      /* No more work needed to return anything. */
      rvalue = ਨਿਰਾ;
    }

  ਲਈ (i = 0; i < ਅੰਕ_ਦਲੀਲਾਂ; i++)
    {
      ffi_type *type = arg_types[i];
      ਖਾਲੀ * ਅਚਲ arg_ਵਿੱਚ = avalue[i];
      ptrdiff_t arg_ਸ਼ਬਦ = argp - arg_mem;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      ਲੰਮਾ align = arg_ਸ਼ਬਦ & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_ਸ਼ਬਦ += align;
#ਜੇ_ਅੰਤ

      ਜੇ (type->type == FFI_TYPE_STRUCT)
        {
          ਅਚਲ size_t arg_ਮਾਪ_ਵਿੱਚ_ਸ਼ਬਦ =
            (type->ਮਾਪ + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          ਜੇ (arg_ਸ਼ਬਦ < ਅੰਕ_ARG_REGS &&
              arg_ਸ਼ਬਦ + arg_ਮਾਪ_ਵਿੱਚ_ਸ਼ਬਦ > ਅੰਕ_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = ਢੇਰ_ਦਲੀਲਾਂ;
            }

          ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, arg_ਵਿੱਚ, type->ਮਾਪ);
          argp += arg_ਮਾਪ_ਵਿੱਚ_ਸ਼ਬਦ;
        }
      ਨਹੀਂਤਾਂ
        {
          argp += ਸੌਂਪੋ_ਮੰਜ਼ਿਲ_ffi_arg(argp, arg_ਵਿੱਚ, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_ਦਲੀਲਾਂ, ਢੇਰ_ਦਲੀਲਾਂ,
                cif->ਬਾਈਟਾਂ - (ਅੰਕ_ARG_REGS * FFI_SIZEOF_ARG), fn);

  ਜੇ (rvalue != ਨਿਰਾ)
    ਸੌਂਪੋ_ਮੰਜ਼ਿਲ_ffi_arg(rvalue, reg_ਦਲੀਲਾਂ, cif->rtype, 0);
}


/* Template code for closure. */
ਬਾਹਰੀ ਅਚਲ UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ *ਘੇਰਾ,
                      ffi_cif *cif,
                      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
                      ਖਾਲੀ *user_ਅੰਕੜੇ,
                      ਖਾਲੀ *codeloc)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  ਪੂਰਨ s;
  UINT64 *ਬਾਹਰਲਾ;

  ਜੇ (cif->abi != FFI_UNIX)
    ਵਾਪਸ FFI_BAD_ABI;

  ਬਾਹਰਲਾ = (UINT64 *)ਘੇਰਾ->tramp;

  c = (intptr_t)ਘੇਰਾ;
  h = (intptr_t)ffi_ਘੇਰਾ_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  ਜਦੋਂ ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *ਬਾਹਰਲਾ++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  ਲਈ (s -= 16; s >= 0; s -= 16)
    *ਬਾਹਰਲਾ++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ OPS

  *ਬਾਹਰਲਾ++ = ffi_template_tramp_tile[2];

#ਨਹੀਂਤਾਂ
  /* TILEPro */
  UINT64 *ਬਾਹਰਲਾ;
  intptr_t delta;

  ਜੇ (cif->abi != FFI_UNIX)
    ਵਾਪਸ FFI_BAD_ABI;

  ਬਾਹਰਲਾ = (UINT64 *)ਘੇਰਾ->tramp;
  delta = (intptr_t)ffi_ਘੇਰਾ_tile - (intptr_t)codeloc;

  *ਬਾਹਰਲਾ++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#ਜੇ_ਅੰਤ

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  invalidate_icache(ਘੇਰਾ->tramp, (ਅੱਖਰ *)ਬਾਹਰਲਾ - ਘੇਰਾ->tramp,
                    getpagesize());

  ਵਾਪਸ FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
ਖਾਲੀ FFI_HIDDEN
ffi_ਘੇਰਾ_tile_inner(ffi_ਘੇਰਾ *ਘੇਰਾ,
                       ffi_sarg reg_ਦਲੀਲਾਂ[2][ਅੰਕ_ARG_REGS],
                       ffi_sarg *ਢੇਰ_ਦਲੀਲਾਂ)
{
  ffi_cif * ਅਚਲ cif = ਘੇਰਾ->cif;
  ਖਾਲੀ ** ਅਚਲ avalue = alloca(cif->nargs * ਆਕਾਰ(ਖਾਲੀ *));
  ਖਾਲੀ *rvalue;
  ffi_type ** ਅਚਲ arg_types = cif->arg_types;
  ffi_sarg * ਅਚਲ reg_ਦਲੀਲਾਂ_ਵਿੱਚ = reg_ਦਲੀਲਾਂ[0];
  ffi_sarg * ਅਚਲ reg_ਦਲੀਲਾਂ_ਬਾਹਰਲਾ = reg_ਦਲੀਲਾਂ[1];
  ffi_sarg * argp;
  ਲੰਮਾ i, arg_ਸ਼ਬਦ, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  ਸੰਘ { ffi_sarg arg[ਅੰਕ_ARG_REGS]; ਦੁੱਗਣਾ d; UINT64 u64; } ਘੇਰਾ_ret;

  /* Start out reading register arguments. */
  argp = reg_ਦਲੀਲਾਂ_ਵਿੱਚ;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  ਜੇ (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Return by reference via hidden pointer. */
      rvalue = (ਖਾਲੀ *)(intptr_t)*argp++;
      arg_ਸ਼ਬਦ = 1;
    }
  ਨਹੀਂਤਾਂ
    {
      /* Return the value in registers. */
      rvalue = &ਘੇਰਾ_ret;
      arg_ਸ਼ਬਦ = 0;
    }

  /* Grab the addresses of the arguments. */
  ਲਈ (i = 0; i < nargs; i++)
    {
      ffi_type * ਅਚਲ type = arg_types[i];
      ਅਚਲ size_t arg_ਮਾਪ_ਵਿੱਚ_ਸ਼ਬਦ =
        (type->ਮਾਪ + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      ਲੰਮਾ align = arg_ਸ਼ਬਦ & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_ਸ਼ਬਦ += align;
#ਜੇ_ਅੰਤ

      ਜੇ (arg_ਸ਼ਬਦ == ਅੰਕ_ARG_REGS ||
          (arg_ਸ਼ਬਦ < ਅੰਕ_ARG_REGS &&
           arg_ਸ਼ਬਦ + arg_ਮਾਪ_ਵਿੱਚ_ਸ਼ਬਦ > ਅੰਕ_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = ਢੇਰ_ਦਲੀਲਾਂ;
          arg_ਸ਼ਬਦ = ਅੰਕ_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_ਮਾਪ_ਵਿੱਚ_ਸ਼ਬਦ;
      arg_ਸ਼ਬਦ += arg_ਮਾਪ_ਵਿੱਚ_ਸ਼ਬਦ;
    }

  /* Invoke the closure.  */
  ਘੇਰਾ->fun(cif, rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);

  ਜੇ (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      ਸੌਂਪੋ_ਮੰਜ਼ਿਲ_ffi_arg(reg_ਦਲੀਲਾਂ_ਬਾਹਰਲਾ, &ਘੇਰਾ_ret, cif->rtype, 1);
    }
}
