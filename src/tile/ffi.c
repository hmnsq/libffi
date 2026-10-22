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

#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <stdlib.h>
#ضمن <stdint.h>
#ضمن <unistd.h>
#ضمن <arch/abi.h>
#ضمن <arch/icache.h>
#ضمن <arch/opcode.h>


/* The first 10 registers are used to pass arguments and return values. */
#وصف_ڪريو انگ_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
خارجي فراغ ffi_call_tile(ffi_sarg reg_دليلون[انگ_ARG_REGS],
                          ثابت ffi_sarg *ڍير_دليلون,
                          size_t ڍير_دليلون_بائيٽون,
                          فراغ (*fnaddr)(فراغ))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
خارجي فراغ ffi_گهيرو_tile(فراغ) FFI_HIDDEN;


ffi_حالت
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  إذا (cif->بائيٽون < انگ_ARG_REGS * FFI_SIZEOF_ARG)
    cif->بائيٽون = انگ_ARG_REGS * FFI_SIZEOF_ARG;

  إذا (cif->rtype->سائيز > انگ_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  وإلا
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  أرجع FFI_OK;
}


ساكن ڊگهو
سونپيو_منزل_ffi_arg(ffi_sarg *ٻاهريون, فراغ *اندريون, ثابت ffi_type *type,
                  صحيح write_منزل_reg)
{
  بدل (type->type)
    {
    حالة FFI_TYPE_SINT8:
      *ٻاهريون = *(SINT8 *)اندريون;
      أرجع 1;

    حالة FFI_TYPE_UINT8:
      *ٻاهريون = *(UINT8 *)اندريون;
      أرجع 1;

    حالة FFI_TYPE_SINT16:
      *ٻاهريون = *(SINT16 *)اندريون;
      أرجع 1;

    حالة FFI_TYPE_UINT16:
      *ٻاهريون = *(UINT16 *)اندريون;
      أرجع 1;

    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
#جيڪڏهن_وصف_نه_ٿيل __LP64__
    حالة FFI_TYPE_اشارو:
#جيڪڏهن_پورو
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *ٻاهريون = *(SINT32 *)اندريون;
      أرجع 1;

    حالة FFI_TYPE_FLOAT:
#جيڪڏهن_وصف_ٿيل __tilegx__
      إذا (write_منزل_reg)
        {
          /* Properly sign extend the value.  */
          اتحاد { عائم f; SINT32 s32; } val;
          val.f = *(عائم *)اندريون;
          *ٻاهريون = val.s32;
        }
      وإلا
#جيڪڏهن_پورو
        {
          *(عائم *)ٻاهريون = *(عائم *)اندريون;
        }
      أرجع 1;

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_DOUBLE:
#جيڪڏهن_وصف_ٿيل __LP64__
    حالة FFI_TYPE_اشارو:
#جيڪڏهن_پورو
      *(UINT64 *)ٻاهريون = *(UINT64 *)اندريون;
      أرجع حجم(UINT64) / FFI_SIZEOF_ARG;

    حالة FFI_TYPE_STRUCT:
      يادگيري_نقل(ٻاهريون, اندريون, type->سائيز);
      أرجع (type->سائيز + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

    حالة FFI_TYPE_VOID:
      /* Must be a return type. Nothing to do. */
      أرجع 0;

    افتراضي:
      FFI_ASSERT(0);
      أرجع -1;
    }
}


فراغ
ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_sarg * ثابت arg_mem = alloca(cif->بائيٽون);
  ffi_sarg * ثابت reg_دليلون = arg_mem;
  ffi_sarg * ثابت ڍير_دليلون = &reg_دليلون[انگ_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** ثابت arg_types = cif->arg_types;
  ثابت ڊگهو انگ_دليلون = cif->nargs;
  ڊگهو i;

  إذا (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->سائيز));

      /* No more work needed to return anything. */
      rvalue = عدم;
    }

  لكل (i = 0; i < انگ_دليلون; i++)
    {
      ffi_type *type = arg_types[i];
      فراغ * ثابت arg_اندريون = avalue[i];
      ptrdiff_t arg_لفظ = argp - arg_mem;

#جيڪڏهن_وصف_نه_ٿيل __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      ڊگهو align = arg_لفظ & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_لفظ += align;
#جيڪڏهن_پورو

      إذا (type->type == FFI_TYPE_STRUCT)
        {
          ثابت size_t arg_سائيز_اندريون_لفظ =
            (type->سائيز + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          إذا (arg_لفظ < انگ_ARG_REGS &&
              arg_لفظ + arg_سائيز_اندريون_لفظ > انگ_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = ڍير_دليلون;
            }

          يادگيري_نقل(argp, arg_اندريون, type->سائيز);
          argp += arg_سائيز_اندريون_لفظ;
        }
      وإلا
        {
          argp += سونپيو_منزل_ffi_arg(argp, arg_اندريون, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_دليلون, ڍير_دليلون,
                cif->بائيٽون - (انگ_ARG_REGS * FFI_SIZEOF_ARG), fn);

  إذا (rvalue != عدم)
    سونپيو_منزل_ffi_arg(rvalue, reg_دليلون, cif->rtype, 0);
}


/* Template code for closure. */
خارجي ثابت UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو *گهيرو,
                      ffi_cif *cif,
                      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
                      فراغ *user_معلومات,
                      فراغ *codeloc)
{
#جيڪڏهن_وصف_ٿيل __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  صحيح s;
  UINT64 *ٻاهريون;

  إذا (cif->abi != FFI_UNIX)
    أرجع FFI_BAD_ABI;

  ٻاهريون = (UINT64 *)گهيرو->tramp;

  c = (intptr_t)گهيرو;
  h = (intptr_t)ffi_گهيرو_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  طالما ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#وصف_ڪريو OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *ٻاهريون++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  لكل (s -= 16; s >= 0; s -= 16)
    *ٻاهريون++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#وصف_رد OPS

  *ٻاهريون++ = ffi_template_tramp_tile[2];

#وإلا
  /* TILEPro */
  UINT64 *ٻاهريون;
  intptr_t delta;

  إذا (cif->abi != FFI_UNIX)
    أرجع FFI_BAD_ABI;

  ٻاهريون = (UINT64 *)گهيرو->tramp;
  delta = (intptr_t)ffi_گهيرو_tile - (intptr_t)codeloc;

  *ٻاهريون++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#جيڪڏهن_پورو

  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

  invalidate_icache(گهيرو->tramp, (حرف *)ٻاهريون - گهيرو->tramp,
                    getpagesize());

  أرجع FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
فراغ FFI_HIDDEN
ffi_گهيرو_tile_inner(ffi_گهيرو *گهيرو,
                       ffi_sarg reg_دليلون[2][انگ_ARG_REGS],
                       ffi_sarg *ڍير_دليلون)
{
  ffi_cif * ثابت cif = گهيرو->cif;
  فراغ ** ثابت avalue = alloca(cif->nargs * حجم(فراغ *));
  فراغ *rvalue;
  ffi_type ** ثابت arg_types = cif->arg_types;
  ffi_sarg * ثابت reg_دليلون_اندريون = reg_دليلون[0];
  ffi_sarg * ثابت reg_دليلون_ٻاهريون = reg_دليلون[1];
  ffi_sarg * argp;
  ڊگهو i, arg_لفظ, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  اتحاد { ffi_sarg arg[انگ_ARG_REGS]; مزدوج d; UINT64 u64; } گهيرو_ret;

  /* Start out reading register arguments. */
  argp = reg_دليلون_اندريون;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  إذا (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Return by reference via hidden pointer. */
      rvalue = (فراغ *)(intptr_t)*argp++;
      arg_لفظ = 1;
    }
  وإلا
    {
      /* Return the value in registers. */
      rvalue = &گهيرو_ret;
      arg_لفظ = 0;
    }

  /* Grab the addresses of the arguments. */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type * ثابت type = arg_types[i];
      ثابت size_t arg_سائيز_اندريون_لفظ =
        (type->سائيز + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#جيڪڏهن_وصف_نه_ٿيل __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      ڊگهو align = arg_لفظ & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_لفظ += align;
#جيڪڏهن_پورو

      إذا (arg_لفظ == انگ_ARG_REGS ||
          (arg_لفظ < انگ_ARG_REGS &&
           arg_لفظ + arg_سائيز_اندريون_لفظ > انگ_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = ڍير_دليلون;
          arg_لفظ = انگ_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_سائيز_اندريون_لفظ;
      arg_لفظ += arg_سائيز_اندريون_لفظ;
    }

  /* Invoke the closure.  */
  گهيرو->fun(cif, rvalue, avalue, گهيرو->user_معلومات);

  إذا (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      سونپيو_منزل_ffi_arg(reg_دليلون_ٻاهريون, &گهيرو_ret, cif->rtype, 1);
    }
}
