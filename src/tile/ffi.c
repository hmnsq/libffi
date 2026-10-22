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

#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdlib.h>
#شامل <stdint.h>
#شامل <unistd.h>
#شامل <arch/abi.h>
#شامل <arch/icache.h>
#شامل <arch/opcode.h>


/* The first 10 registers are used to pass arguments and return values. */
#تعریف_کرو عدد_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
خارجی خالی ffi_call_tile(ffi_sarg reg_دلائل[عدد_ARG_REGS],
                          ثابت ffi_sarg *ڈھیر_دلائل,
                          size_t ڈھیر_دلائل_بائٹیں,
                          خالی (*fnaddr)(خالی))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
خارجی خالی ffi_احاطہ_tile(خالی) FFI_HIDDEN;


ffi_حالت
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  اگر (cif->بائٹیں < عدد_ARG_REGS * FFI_SIZEOF_ARG)
    cif->بائٹیں = عدد_ARG_REGS * FFI_SIZEOF_ARG;

  اگر (cif->rtype->سائز > عدد_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  ورنہ
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  واپس FFI_OK;
}


ساکن طویل
تفویض_مقصد_ffi_arg(ffi_sarg *برآمد, خالی *اندر, ثابت ffi_type *type,
                  صحیح write_مقصد_reg)
{
  انتخاب (type->type)
    {
    حالت FFI_TYPE_SINT8:
      *برآمد = *(SINT8 *)اندر;
      واپس 1;

    حالت FFI_TYPE_UINT8:
      *برآمد = *(UINT8 *)اندر;
      واپس 1;

    حالت FFI_TYPE_SINT16:
      *برآمد = *(SINT16 *)اندر;
      واپس 1;

    حالت FFI_TYPE_UINT16:
      *برآمد = *(UINT16 *)اندر;
      واپس 1;

    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
#اگر_غیر_تعریف_شدہ __LP64__
    حالت FFI_TYPE_اشارہ_گر:
#اگر_ختم
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *برآمد = *(SINT32 *)اندر;
      واپس 1;

    حالت FFI_TYPE_FLOAT:
#اگر_تعریف_شدہ __tilegx__
      اگر (write_مقصد_reg)
        {
          /* Properly sign extend the value.  */
          اتحاد { اعشاری f; SINT32 s32; } val;
          val.f = *(اعشاری *)اندر;
          *برآمد = val.s32;
        }
      ورنہ
#اگر_ختم
        {
          *(اعشاری *)برآمد = *(اعشاری *)اندر;
        }
      واپس 1;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_DOUBLE:
#اگر_تعریف_شدہ __LP64__
    حالت FFI_TYPE_اشارہ_گر:
#اگر_ختم
      *(UINT64 *)برآمد = *(UINT64 *)اندر;
      واپس حجم(UINT64) / FFI_SIZEOF_ARG;

    حالت FFI_TYPE_STRUCT:
      حافظہ_نقل(برآمد, اندر, type->سائز);
      واپس (type->سائز + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

    حالت FFI_TYPE_VOID:
      /* Must be a return type. Nothing to do. */
      واپس 0;

    طےشدہ:
      FFI_ASSERT(0);
      واپس -1;
    }
}


خالی
ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  ffi_sarg * ثابت arg_mem = alloca(cif->بائٹیں);
  ffi_sarg * ثابت reg_دلائل = arg_mem;
  ffi_sarg * ثابت ڈھیر_دلائل = &reg_دلائل[عدد_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** ثابت arg_types = cif->arg_types;
  ثابت طویل عدد_دلائل = cif->nargs;
  طویل i;

  اگر (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->سائز));

      /* No more work needed to return anything. */
      rvalue = تہی;
    }

  برائے (i = 0; i < عدد_دلائل; i++)
    {
      ffi_type *type = arg_types[i];
      خالی * ثابت arg_اندر = avalue[i];
      ptrdiff_t arg_لفظ = argp - arg_mem;

#اگر_غیر_تعریف_شدہ __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      طویل align = arg_لفظ & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_لفظ += align;
#اگر_ختم

      اگر (type->type == FFI_TYPE_STRUCT)
        {
          ثابت size_t arg_سائز_اندر_الفاظ =
            (type->سائز + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          اگر (arg_لفظ < عدد_ARG_REGS &&
              arg_لفظ + arg_سائز_اندر_الفاظ > عدد_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = ڈھیر_دلائل;
            }

          حافظہ_نقل(argp, arg_اندر, type->سائز);
          argp += arg_سائز_اندر_الفاظ;
        }
      ورنہ
        {
          argp += تفویض_مقصد_ffi_arg(argp, arg_اندر, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_دلائل, ڈھیر_دلائل,
                cif->بائٹیں - (عدد_ARG_REGS * FFI_SIZEOF_ARG), fn);

  اگر (rvalue != تہی)
    تفویض_مقصد_ffi_arg(rvalue, reg_دلائل, cif->rtype, 0);
}


/* Template code for closure. */
خارجی ثابت UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ *احاطہ,
                      ffi_cif *cif,
                      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
                      خالی *user_ڈیٹا,
                      خالی *codeloc)
{
#اگر_تعریف_شدہ __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  صحیح s;
  UINT64 *برآمد;

  اگر (cif->abi != FFI_UNIX)
    واپس FFI_BAD_ABI;

  برآمد = (UINT64 *)احاطہ->tramp;

  c = (intptr_t)احاطہ;
  h = (intptr_t)ffi_احاطہ_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  جب ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#تعریف_کرو OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *برآمد++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  برائے (s -= 16; s >= 0; s -= 16)
    *برآمد++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#تعریف_منسوخ OPS

  *برآمد++ = ffi_template_tramp_tile[2];

#ورنہ
  /* TILEPro */
  UINT64 *برآمد;
  intptr_t delta;

  اگر (cif->abi != FFI_UNIX)
    واپس FFI_BAD_ABI;

  برآمد = (UINT64 *)احاطہ->tramp;
  delta = (intptr_t)ffi_احاطہ_tile - (intptr_t)codeloc;

  *برآمد++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#اگر_ختم

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  invalidate_icache(احاطہ->tramp, (حرف *)برآمد - احاطہ->tramp,
                    getpagesize());

  واپس FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
خالی FFI_HIDDEN
ffi_احاطہ_tile_inner(ffi_احاطہ *احاطہ,
                       ffi_sarg reg_دلائل[2][عدد_ARG_REGS],
                       ffi_sarg *ڈھیر_دلائل)
{
  ffi_cif * ثابت cif = احاطہ->cif;
  خالی ** ثابت avalue = alloca(cif->nargs * حجم(خالی *));
  خالی *rvalue;
  ffi_type ** ثابت arg_types = cif->arg_types;
  ffi_sarg * ثابت reg_دلائل_اندر = reg_دلائل[0];
  ffi_sarg * ثابت reg_دلائل_برآمد = reg_دلائل[1];
  ffi_sarg * argp;
  طویل i, arg_لفظ, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  اتحاد { ffi_sarg arg[عدد_ARG_REGS]; دوہرا d; UINT64 u64; } احاطہ_ret;

  /* Start out reading register arguments. */
  argp = reg_دلائل_اندر;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  اگر (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Return by reference via hidden pointer. */
      rvalue = (خالی *)(intptr_t)*argp++;
      arg_لفظ = 1;
    }
  ورنہ
    {
      /* Return the value in registers. */
      rvalue = &احاطہ_ret;
      arg_لفظ = 0;
    }

  /* Grab the addresses of the arguments. */
  برائے (i = 0; i < nargs; i++)
    {
      ffi_type * ثابت type = arg_types[i];
      ثابت size_t arg_سائز_اندر_الفاظ =
        (type->سائز + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#اگر_غیر_تعریف_شدہ __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      طویل align = arg_لفظ & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_لفظ += align;
#اگر_ختم

      اگر (arg_لفظ == عدد_ARG_REGS ||
          (arg_لفظ < عدد_ARG_REGS &&
           arg_لفظ + arg_سائز_اندر_الفاظ > عدد_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = ڈھیر_دلائل;
          arg_لفظ = عدد_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_سائز_اندر_الفاظ;
      arg_لفظ += arg_سائز_اندر_الفاظ;
    }

  /* Invoke the closure.  */
  احاطہ->fun(cif, rvalue, avalue, احاطہ->user_ڈیٹا);

  اگر (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      تفویض_مقصد_ffi_arg(reg_دلائل_برآمد, &احاطہ_ret, cif->rtype, 1);
    }
}
