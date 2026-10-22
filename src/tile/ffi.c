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
#تعریف_کرو عدد_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
خارجي فراغ ffi_call_tile(ffi_sarg reg_دلیلاں[عدد_ARG_REGS],
                          ثابت ffi_sarg *ڈھیر_دلیلاں,
                          size_t ڈھیر_دلیلاں_بائٹاں,
                          فراغ (*fnaddr)(فراغ))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
خارجي فراغ ffi_گھیرا_tile(فراغ) FFI_HIDDEN;


ffi_حالت
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  إذا (cif->بائٹاں < عدد_ARG_REGS * FFI_SIZEOF_ARG)
    cif->بائٹاں = عدد_ARG_REGS * FFI_SIZEOF_ARG;

  إذا (cif->rtype->ناپ > عدد_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  وإلا
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  أرجع FFI_OK;
}


ساكن لما
سونپو_منزل_ffi_arg(ffi_sarg *باہرلا, فراغ *وچ, ثابت ffi_type *type,
                  صحيح write_منزل_reg)
{
  بدل (type->type)
    {
    حالة FFI_TYPE_SINT8:
      *باہرلا = *(SINT8 *)وچ;
      أرجع 1;

    حالة FFI_TYPE_UINT8:
      *باہرلا = *(UINT8 *)وچ;
      أرجع 1;

    حالة FFI_TYPE_SINT16:
      *باہرلا = *(SINT16 *)وچ;
      أرجع 1;

    حالة FFI_TYPE_UINT16:
      *باہرلا = *(UINT16 *)وچ;
      أرجع 1;

    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
#جے_بنا_تعریف __LP64__
    حالة FFI_TYPE_اشارہ_گر:
#جے_انت
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *باہرلا = *(SINT32 *)وچ;
      أرجع 1;

    حالة FFI_TYPE_FLOAT:
#جے_تعریف_شدہ __tilegx__
      إذا (write_منزل_reg)
        {
          /* Properly sign extend the value.  */
          اتحاد { عائم f; SINT32 s32; } val;
          val.f = *(عائم *)وچ;
          *باہرلا = val.s32;
        }
      وإلا
#جے_انت
        {
          *(عائم *)باہرلا = *(عائم *)وچ;
        }
      أرجع 1;

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_DOUBLE:
#جے_تعریف_شدہ __LP64__
    حالة FFI_TYPE_اشارہ_گر:
#جے_انت
      *(UINT64 *)باہرلا = *(UINT64 *)وچ;
      أرجع حجم(UINT64) / FFI_SIZEOF_ARG;

    حالة FFI_TYPE_STRUCT:
      حافظہ_نقل(باہرلا, وچ, type->ناپ);
      أرجع (type->ناپ + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

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
  ffi_sarg * ثابت arg_mem = alloca(cif->بائٹاں);
  ffi_sarg * ثابت reg_دلیلاں = arg_mem;
  ffi_sarg * ثابت ڈھیر_دلیلاں = &reg_دلیلاں[عدد_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** ثابت arg_types = cif->arg_types;
  ثابت لما عدد_دلیلاں = cif->nargs;
  لما i;

  إذا (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->ناپ));

      /* No more work needed to return anything. */
      rvalue = عدم;
    }

  لكل (i = 0; i < عدد_دلیلاں; i++)
    {
      ffi_type *type = arg_types[i];
      فراغ * ثابت arg_وچ = avalue[i];
      ptrdiff_t arg_لفظ = argp - arg_mem;

#جے_بنا_تعریف __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      لما align = arg_لفظ & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_لفظ += align;
#جے_انت

      إذا (type->type == FFI_TYPE_STRUCT)
        {
          ثابت size_t arg_ناپ_وچ_الفاظ =
            (type->ناپ + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          إذا (arg_لفظ < عدد_ARG_REGS &&
              arg_لفظ + arg_ناپ_وچ_الفاظ > عدد_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = ڈھیر_دلیلاں;
            }

          حافظہ_نقل(argp, arg_وچ, type->ناپ);
          argp += arg_ناپ_وچ_الفاظ;
        }
      وإلا
        {
          argp += سونپو_منزل_ffi_arg(argp, arg_وچ, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_دلیلاں, ڈھیر_دلیلاں,
                cif->بائٹاں - (عدد_ARG_REGS * FFI_SIZEOF_ARG), fn);

  إذا (rvalue != عدم)
    سونپو_منزل_ffi_arg(rvalue, reg_دلیلاں, cif->rtype, 0);
}


/* Template code for closure. */
خارجي ثابت UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا *گھیرا,
                      ffi_cif *cif,
                      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
                      فراغ *user_انکڑے,
                      فراغ *codeloc)
{
#جے_تعریف_شدہ __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  صحيح s;
  UINT64 *باہرلا;

  إذا (cif->abi != FFI_UNIX)
    أرجع FFI_BAD_ABI;

  باہرلا = (UINT64 *)گھیرا->tramp;

  c = (intptr_t)گھیرا;
  h = (intptr_t)ffi_گھیرا_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  طالما ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#تعریف_کرو OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *باہرلا++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  لكل (s -= 16; s >= 0; s -= 16)
    *باہرلا++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#تعریف_رد OPS

  *باہرلا++ = ffi_template_tramp_tile[2];

#وإلا
  /* TILEPro */
  UINT64 *باہرلا;
  intptr_t delta;

  إذا (cif->abi != FFI_UNIX)
    أرجع FFI_BAD_ABI;

  باہرلا = (UINT64 *)گھیرا->tramp;
  delta = (intptr_t)ffi_گھیرا_tile - (intptr_t)codeloc;

  *باہرلا++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#جے_انت

  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;

  invalidate_icache(گھیرا->tramp, (حرف *)باہرلا - گھیرا->tramp,
                    getpagesize());

  أرجع FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
فراغ FFI_HIDDEN
ffi_گھیرا_tile_inner(ffi_گھیرا *گھیرا,
                       ffi_sarg reg_دلیلاں[2][عدد_ARG_REGS],
                       ffi_sarg *ڈھیر_دلیلاں)
{
  ffi_cif * ثابت cif = گھیرا->cif;
  فراغ ** ثابت avalue = alloca(cif->nargs * حجم(فراغ *));
  فراغ *rvalue;
  ffi_type ** ثابت arg_types = cif->arg_types;
  ffi_sarg * ثابت reg_دلیلاں_وچ = reg_دلیلاں[0];
  ffi_sarg * ثابت reg_دلیلاں_باہرلا = reg_دلیلاں[1];
  ffi_sarg * argp;
  لما i, arg_لفظ, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  اتحاد { ffi_sarg arg[عدد_ARG_REGS]; مزدوج d; UINT64 u64; } گھیرا_ret;

  /* Start out reading register arguments. */
  argp = reg_دلیلاں_وچ;

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
      rvalue = &گھیرا_ret;
      arg_لفظ = 0;
    }

  /* Grab the addresses of the arguments. */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type * ثابت type = arg_types[i];
      ثابت size_t arg_ناپ_وچ_الفاظ =
        (type->ناپ + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#جے_بنا_تعریف __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      لما align = arg_لفظ & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_لفظ += align;
#جے_انت

      إذا (arg_لفظ == عدد_ARG_REGS ||
          (arg_لفظ < عدد_ARG_REGS &&
           arg_لفظ + arg_ناپ_وچ_الفاظ > عدد_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = ڈھیر_دلیلاں;
          arg_لفظ = عدد_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_ناپ_وچ_الفاظ;
      arg_لفظ += arg_ناپ_وچ_الفاظ;
    }

  /* Invoke the closure.  */
  گھیرا->fun(cif, rvalue, avalue, گھیرا->user_انکڑے);

  إذا (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      سونپو_منزل_ffi_arg(reg_دلیلاں_باہرلا, &گھیرا_ret, cif->rtype, 1);
    }
}
