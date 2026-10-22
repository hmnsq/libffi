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
#تعریف_کړه عدد_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
خارجي فراغ ffi_call_tile(ffi_sarg reg_دلیلونه[عدد_ARG_REGS],
                          ثابت ffi_sarg *ډېری_دلیلونه,
                          size_t ډېری_دلیلونه_بایټونه,
                          فراغ (*fnaddr)(فراغ))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
خارجي فراغ ffi_تړون_tile(فراغ) FFI_HIDDEN;


ffi_حالت
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  إذا (cif->بایټونه < عدد_ARG_REGS * FFI_SIZEOF_ARG)
    cif->بایټونه = عدد_ARG_REGS * FFI_SIZEOF_ARG;

  إذا (cif->rtype->اندازه > عدد_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  وإلا
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  أرجع FFI_OK;
}


ساكن اوږد
ټاکل_منزل_ffi_arg(ffi_sarg *بهر, فراغ *دننه, ثابت ffi_type *type,
                  صحيح write_منزل_reg)
{
  بدل (type->type)
    {
    حالة FFI_TYPE_SINT8:
      *بهر = *(SINT8 *)دننه;
      أرجع 1;

    حالة FFI_TYPE_UINT8:
      *بهر = *(UINT8 *)دننه;
      أرجع 1;

    حالة FFI_TYPE_SINT16:
      *بهر = *(SINT16 *)دننه;
      أرجع 1;

    حالة FFI_TYPE_UINT16:
      *بهر = *(UINT16 *)دننه;
      أرجع 1;

    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
#که_نه_تعریف_شوی __LP64__
    حالة FFI_TYPE_اشاره_ګر:
#د_که_پای
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *بهر = *(SINT32 *)دننه;
      أرجع 1;

    حالة FFI_TYPE_FLOAT:
#که_تعریف_شوی __tilegx__
      إذا (write_منزل_reg)
        {
          /* Properly sign extend the value.  */
          اتحاد { عائم f; SINT32 s32; } val;
          val.f = *(عائم *)دننه;
          *بهر = val.s32;
        }
      وإلا
#د_که_پای
        {
          *(عائم *)بهر = *(عائم *)دننه;
        }
      أرجع 1;

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_DOUBLE:
#که_تعریف_شوی __LP64__
    حالة FFI_TYPE_اشاره_ګر:
#د_که_پای
      *(UINT64 *)بهر = *(UINT64 *)دننه;
      أرجع حجم(UINT64) / FFI_SIZEOF_ARG;

    حالة FFI_TYPE_STRUCT:
      حافظه_نقل(بهر, دننه, type->اندازه);
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
  ffi_sarg * ثابت arg_mem = alloca(cif->بایټونه);
  ffi_sarg * ثابت reg_دلیلونه = arg_mem;
  ffi_sarg * ثابت ډېری_دلیلونه = &reg_دلیلونه[عدد_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** ثابت arg_types = cif->arg_types;
  ثابت اوږد عدد_دلیلونه = cif->nargs;
  اوږد i;

  إذا (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->اندازه));

      /* No more work needed to return anything. */
      rvalue = عدم;
    }

  لكل (i = 0; i < عدد_دلیلونه; i++)
    {
      ffi_type *type = arg_types[i];
      فراغ * ثابت arg_دننه = avalue[i];
      ptrdiff_t arg_کلمه = argp - arg_mem;

#که_نه_تعریف_شوی __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      اوږد align = arg_کلمه & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_کلمه += align;
#د_که_پای

      إذا (type->type == FFI_TYPE_STRUCT)
        {
          ثابت size_t arg_اندازه_دننه_ټکي =
            (type->اندازه + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          إذا (arg_کلمه < عدد_ARG_REGS &&
              arg_کلمه + arg_اندازه_دننه_ټکي > عدد_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = ډېری_دلیلونه;
            }

          حافظه_نقل(argp, arg_دننه, type->اندازه);
          argp += arg_اندازه_دننه_ټکي;
        }
      وإلا
        {
          argp += ټاکل_منزل_ffi_arg(argp, arg_دننه, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_دلیلونه, ډېری_دلیلونه,
                cif->بایټونه - (عدد_ARG_REGS * FFI_SIZEOF_ARG), fn);

  إذا (rvalue != عدم)
    ټاکل_منزل_ffi_arg(rvalue, reg_دلیلونه, cif->rtype, 0);
}


/* Template code for closure. */
خارجي ثابت UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_حالت
ffi_prep_تړون_loc (ffi_تړون *تړون,
                      ffi_cif *cif,
                      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
                      فراغ *user_معلومات,
                      فراغ *codeloc)
{
#که_تعریف_شوی __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  صحيح s;
  UINT64 *بهر;

  إذا (cif->abi != FFI_UNIX)
    أرجع FFI_BAD_ABI;

  بهر = (UINT64 *)تړون->tramp;

  c = (intptr_t)تړون;
  h = (intptr_t)ffi_تړون_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  طالما ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#تعریف_کړه OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *بهر++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  لكل (s -= 16; s >= 0; s -= 16)
    *بهر++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#تعریف_لغوه OPS

  *بهر++ = ffi_template_tramp_tile[2];

#وإلا
  /* TILEPro */
  UINT64 *بهر;
  intptr_t delta;

  إذا (cif->abi != FFI_UNIX)
    أرجع FFI_BAD_ABI;

  بهر = (UINT64 *)تړون->tramp;
  delta = (intptr_t)ffi_تړون_tile - (intptr_t)codeloc;

  *بهر++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#د_که_پای

  تړون->cif = cif;
  تړون->fun = fun;
  تړون->user_معلومات = user_معلومات;

  invalidate_icache(تړون->tramp, (حرف *)بهر - تړون->tramp,
                    getpagesize());

  أرجع FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
فراغ FFI_HIDDEN
ffi_تړون_tile_inner(ffi_تړون *تړون,
                       ffi_sarg reg_دلیلونه[2][عدد_ARG_REGS],
                       ffi_sarg *ډېری_دلیلونه)
{
  ffi_cif * ثابت cif = تړون->cif;
  فراغ ** ثابت avalue = alloca(cif->nargs * حجم(فراغ *));
  فراغ *rvalue;
  ffi_type ** ثابت arg_types = cif->arg_types;
  ffi_sarg * ثابت reg_دلیلونه_دننه = reg_دلیلونه[0];
  ffi_sarg * ثابت reg_دلیلونه_بهر = reg_دلیلونه[1];
  ffi_sarg * argp;
  اوږد i, arg_کلمه, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  اتحاد { ffi_sarg arg[عدد_ARG_REGS]; مزدوج d; UINT64 u64; } تړون_ret;

  /* Start out reading register arguments. */
  argp = reg_دلیلونه_دننه;

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
      rvalue = &تړون_ret;
      arg_کلمه = 0;
    }

  /* Grab the addresses of the arguments. */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type * ثابت type = arg_types[i];
      ثابت size_t arg_اندازه_دننه_ټکي =
        (type->اندازه + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#که_نه_تعریف_شوی __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      اوږد align = arg_کلمه & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_کلمه += align;
#د_که_پای

      إذا (arg_کلمه == عدد_ARG_REGS ||
          (arg_کلمه < عدد_ARG_REGS &&
           arg_کلمه + arg_اندازه_دننه_ټکي > عدد_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = ډېری_دلیلونه;
          arg_کلمه = عدد_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_اندازه_دننه_ټکي;
      arg_کلمه += arg_اندازه_دننه_ټکي;
    }

  /* Invoke the closure.  */
  تړون->fun(cif, rvalue, avalue, تړون->user_معلومات);

  إذا (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      ټاکل_منزل_ffi_arg(reg_دلیلونه_بهر, &تړون_ret, cif->rtype, 1);
    }
}
