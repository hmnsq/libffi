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
#تعریف_کن رقم_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
خارجي فراغ ffi_call_tile(ffi_sarg reg_آرگومانها[رقم_ARG_REGS],
                          ثابت ffi_sarg *پشته_آرگومانها,
                          size_t پشته_آرگومانها_بایتها,
                          فراغ (*fnaddr)(فراغ))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
خارجي فراغ ffi_بستگی_tile(فراغ) FFI_HIDDEN;


ffi_وضعیت
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  إذا (cif->بایتها < رقم_ARG_REGS * FFI_SIZEOF_ARG)
    cif->بایتها = رقم_ARG_REGS * FFI_SIZEOF_ARG;

  إذا (cif->rtype->اندازه > رقم_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  وإلا
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  أرجع FFI_OK;
}


ساكن دراز
تخصیص_بده_مقصد_ffi_arg(ffi_sarg *بیرون, فراغ *درون, ثابت ffi_type *type,
                  صحيح write_مقصد_reg)
{
  بدل (type->type)
    {
    حالة FFI_TYPE_SINT8:
      *بیرون = *(SINT8 *)درون;
      أرجع 1;

    حالة FFI_TYPE_UINT8:
      *بیرون = *(UINT8 *)درون;
      أرجع 1;

    حالة FFI_TYPE_SINT16:
      *بیرون = *(SINT16 *)درون;
      أرجع 1;

    حالة FFI_TYPE_UINT16:
      *بیرون = *(UINT16 *)درون;
      أرجع 1;

    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
#اگر_تعریف_نشده __LP64__
    حالة FFI_TYPE_اشاره_گر:
#پایان_اگر
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *بیرون = *(SINT32 *)درون;
      أرجع 1;

    حالة FFI_TYPE_FLOAT:
#اگر_تعریف_شده __tilegx__
      إذا (write_مقصد_reg)
        {
          /* Properly sign extend the value.  */
          اتحاد { عائم f; SINT32 s32; } val;
          val.f = *(عائم *)درون;
          *بیرون = val.s32;
        }
      وإلا
#پایان_اگر
        {
          *(عائم *)بیرون = *(عائم *)درون;
        }
      أرجع 1;

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_DOUBLE:
#اگر_تعریف_شده __LP64__
    حالة FFI_TYPE_اشاره_گر:
#پایان_اگر
      *(UINT64 *)بیرون = *(UINT64 *)درون;
      أرجع حجم(UINT64) / FFI_SIZEOF_ARG;

    حالة FFI_TYPE_STRUCT:
      کپی_حافظه(بیرون, درون, type->اندازه);
      أرجع (type->اندازه + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

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
  ffi_sarg * ثابت arg_mem = alloca(cif->بایتها);
  ffi_sarg * ثابت reg_آرگومانها = arg_mem;
  ffi_sarg * ثابت پشته_آرگومانها = &reg_آرگومانها[رقم_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** ثابت arg_types = cif->arg_types;
  ثابت دراز رقم_آرگومانها = cif->nargs;
  دراز i;

  إذا (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->اندازه));

      /* No more work needed to return anything. */
      rvalue = عدم;
    }

  لكل (i = 0; i < رقم_آرگومانها; i++)
    {
      ffi_type *type = arg_types[i];
      فراغ * ثابت arg_درون = avalue[i];
      ptrdiff_t arg_کلمه = argp - arg_mem;

#اگر_تعریف_نشده __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      دراز align = arg_کلمه & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_کلمه += align;
#پایان_اگر

      إذا (type->type == FFI_TYPE_STRUCT)
        {
          ثابت size_t arg_اندازه_درون_واژگان =
            (type->اندازه + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          إذا (arg_کلمه < رقم_ARG_REGS &&
              arg_کلمه + arg_اندازه_درون_واژگان > رقم_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = پشته_آرگومانها;
            }

          کپی_حافظه(argp, arg_درون, type->اندازه);
          argp += arg_اندازه_درون_واژگان;
        }
      وإلا
        {
          argp += تخصیص_بده_مقصد_ffi_arg(argp, arg_درون, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_آرگومانها, پشته_آرگومانها,
                cif->بایتها - (رقم_ARG_REGS * FFI_SIZEOF_ARG), fn);

  إذا (rvalue != عدم)
    تخصیص_بده_مقصد_ffi_arg(rvalue, reg_آرگومانها, cif->rtype, 0);
}


/* Template code for closure. */
خارجي ثابت UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی *بستگی,
                      ffi_cif *cif,
                      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
                      فراغ *user_داده,
                      فراغ *codeloc)
{
#اگر_تعریف_شده __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  صحيح s;
  UINT64 *بیرون;

  إذا (cif->abi != FFI_UNIX)
    أرجع FFI_BAD_ABI;

  بیرون = (UINT64 *)بستگی->tramp;

  c = (intptr_t)بستگی;
  h = (intptr_t)ffi_بستگی_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  طالما ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#تعریف_کن OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *بیرون++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  لكل (s -= 16; s >= 0; s -= 16)
    *بیرون++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#لغو_تعریف OPS

  *بیرون++ = ffi_template_tramp_tile[2];

#وإلا
  /* TILEPro */
  UINT64 *بیرون;
  intptr_t delta;

  إذا (cif->abi != FFI_UNIX)
    أرجع FFI_BAD_ABI;

  بیرون = (UINT64 *)بستگی->tramp;
  delta = (intptr_t)ffi_بستگی_tile - (intptr_t)codeloc;

  *بیرون++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#پایان_اگر

  بستگی->cif = cif;
  بستگی->fun = fun;
  بستگی->user_داده = user_داده;

  invalidate_icache(بستگی->tramp, (حرف *)بیرون - بستگی->tramp,
                    getpagesize());

  أرجع FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
فراغ FFI_HIDDEN
ffi_بستگی_tile_inner(ffi_بستگی *بستگی,
                       ffi_sarg reg_آرگومانها[2][رقم_ARG_REGS],
                       ffi_sarg *پشته_آرگومانها)
{
  ffi_cif * ثابت cif = بستگی->cif;
  فراغ ** ثابت avalue = alloca(cif->nargs * حجم(فراغ *));
  فراغ *rvalue;
  ffi_type ** ثابت arg_types = cif->arg_types;
  ffi_sarg * ثابت reg_آرگومانها_درون = reg_آرگومانها[0];
  ffi_sarg * ثابت reg_آرگومانها_بیرون = reg_آرگومانها[1];
  ffi_sarg * argp;
  دراز i, arg_کلمه, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  اتحاد { ffi_sarg arg[رقم_ARG_REGS]; مزدوج d; UINT64 u64; } بستگی_ret;

  /* Start out reading register arguments. */
  argp = reg_آرگومانها_درون;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  إذا (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Return by reference via hidden pointer. */
      rvalue = (فراغ *)(intptr_t)*argp++;
      arg_کلمه = 1;
    }
  وإلا
    {
      /* Return the value in registers. */
      rvalue = &بستگی_ret;
      arg_کلمه = 0;
    }

  /* Grab the addresses of the arguments. */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type * ثابت type = arg_types[i];
      ثابت size_t arg_اندازه_درون_واژگان =
        (type->اندازه + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#اگر_تعریف_نشده __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      دراز align = arg_کلمه & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_کلمه += align;
#پایان_اگر

      إذا (arg_کلمه == رقم_ARG_REGS ||
          (arg_کلمه < رقم_ARG_REGS &&
           arg_کلمه + arg_اندازه_درون_واژگان > رقم_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = پشته_آرگومانها;
          arg_کلمه = رقم_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_اندازه_درون_واژگان;
      arg_کلمه += arg_اندازه_درون_واژگان;
    }

  /* Invoke the closure.  */
  بستگی->fun(cif, rvalue, avalue, بستگی->user_داده);

  إذا (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      تخصیص_بده_مقصد_ffi_arg(reg_آرگومانها_بیرون, &بستگی_ret, cif->rtype, 1);
    }
}
