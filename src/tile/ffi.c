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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <arch/abi.h>
#অন্তর্ভাব <arch/icache.h>
#অন্তর্ভাব <arch/opcode.h>


/* The first 10 registers are used to pass arguments and return values. */
#define অঙ্ক_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
বাহ্যিক শূন্য ffi_call_tile(ffi_sarg reg_args[অঙ্ক_ARG_REGS],
                          ধ্রুবক ffi_sarg *স্তূপ_args,
                          size_t স্তূপ_args_bytes,
                          শূন্য (*fnaddr)(শূন্য))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
বাহ্যিক শূন্য ffi_closure_tile(শূন্য) FFI_HIDDEN;


ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  যদি (cif->bytes < অঙ্ক_ARG_REGS * FFI_SIZEOF_ARG)
    cif->bytes = অঙ্ক_ARG_REGS * FFI_SIZEOF_ARG;

  যদি (cif->rtype->আকার_2 > অঙ্ক_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  অন্যথা
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  ফেরত FFI_OK;
}


স্থির long
assign_to_ffi_arg(ffi_sarg *out, শূন্য *in, ধ্রুবক ffi_type *type,
                  পূর্ণসংখ্যা write_to_reg)
{
  নির্বাচন (type->type)
    {
    ক্ষেত্র FFI_TYPE_SINT8:
      *out = *(SINT8 *)in;
      ফেরত 1;

    ক্ষেত্র FFI_TYPE_UINT8:
      *out = *(UINT8 *)in;
      ফেরত 1;

    ক্ষেত্র FFI_TYPE_SINT16:
      *out = *(SINT16 *)in;
      ফেরত 1;

    ক্ষেত্র FFI_TYPE_UINT16:
      *out = *(UINT16 *)in;
      ফেরত 1;

    ক্ষেত্র FFI_TYPE_SINT32:
    ক্ষেত্র FFI_TYPE_UINT32:
#ifndef __LP64__
    ক্ষেত্র FFI_TYPE_POINTER:
#endif
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *out = *(SINT32 *)in;
      ফেরত 1;

    ক্ষেত্র FFI_TYPE_FLOAT:
#ifdef __tilegx__
      যদি (write_to_reg)
        {
          /* Properly sign extend the value.  */
          সংঘ { দশমিক f; SINT32 s32; } val;
          val.f = *(দশমিক *)in;
          *out = val.s32;
        }
      অন্যথা
#endif
        {
          *(দশমিক *)out = *(দশমিক *)in;
        }
      ফেরত 1;

    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_DOUBLE:
#ifdef __LP64__
    ক্ষেত্র FFI_TYPE_POINTER:
#endif
      *(UINT64 *)out = *(UINT64 *)in;
      ফেরত আকার(UINT64) / FFI_SIZEOF_ARG;

    ক্ষেত্র FFI_TYPE_STRUCT:
      memcpy(out, in, type->আকার_2);
      ফেরত (type->আকার_2 + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

    ক্ষেত্র FFI_TYPE_VOID:
      /* Must be a return type. Nothing to do. */
      ফেরত 0;

    পূর্বনির্ধারিত:
      FFI_ASSERT(0);
      ফেরত -1;
    }
}


শূন্য
ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_sarg * ধ্রুবক arg_mem = alloca(cif->bytes);
  ffi_sarg * ধ্রুবক reg_args = arg_mem;
  ffi_sarg * ধ্রুবক স্তূপ_args = &reg_args[অঙ্ক_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** ধ্রুবক arg_types = cif->arg_types;
  ধ্রুবক long অঙ্ক_args = cif->nargs;
  long i;

  যদি (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->আকার_2));

      /* No more work needed to return anything. */
      rvalue = ফাঁকা;
    }

  জন্য (i = 0; i < অঙ্ক_args; i++)
    {
      ffi_type *type = arg_types[i];
      শূন্য * ধ্রুবক arg_in = avalue[i];
      ptrdiff_t arg_word = argp - arg_mem;

#ifndef __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      long align = arg_word & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_word += align;
#endif

      যদি (type->type == FFI_TYPE_STRUCT)
        {
          ধ্রুবক size_t arg_আকার_in_শব্দগুলি =
            (type->আকার_2 + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          যদি (arg_word < অঙ্ক_ARG_REGS &&
              arg_word + arg_আকার_in_শব্দগুলি > অঙ্ক_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = স্তূপ_args;
            }

          memcpy(argp, arg_in, type->আকার_2);
          argp += arg_আকার_in_শব্দগুলি;
        }
      অন্যথা
        {
          argp += assign_to_ffi_arg(argp, arg_in, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_args, স্তূপ_args,
                cif->bytes - (অঙ্ক_ARG_REGS * FFI_SIZEOF_ARG), fn);

  যদি (rvalue != ফাঁকা)
    assign_to_ffi_arg(rvalue, reg_args, cif->rtype, 0);
}


/* Template code for closure. */
বাহ্যিক ধ্রুবক UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif *cif,
                      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
                      শূন্য *user_তথ্য,
                      শূন্য *codeloc)
{
#ifdef __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  পূর্ণসংখ্যা s;
  UINT64 *out;

  যদি (cif->abi != FFI_UNIX)
    ফেরত FFI_BAD_ABI;

  out = (UINT64 *)closure->tramp;

  c = (intptr_t)closure;
  h = (intptr_t)ffi_closure_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  যখন ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#define OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *out++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  জন্য (s -= 16; s >= 0; s -= 16)
    *out++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#undef OPS

  *out++ = ffi_template_tramp_tile[2];

#অন্যথা
  /* TILEPro */
  UINT64 *out;
  intptr_t delta;

  যদি (cif->abi != FFI_UNIX)
    ফেরত FFI_BAD_ABI;

  out = (UINT64 *)closure->tramp;
  delta = (intptr_t)ffi_closure_tile - (intptr_t)codeloc;

  *out++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

  invalidate_icache(closure->tramp, (অক্ষর *)out - closure->tramp,
                    getpagesize());

  ফেরত FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
শূন্য FFI_HIDDEN
ffi_closure_tile_inner(ffi_closure *closure,
                       ffi_sarg reg_args[2][অঙ্ক_ARG_REGS],
                       ffi_sarg *স্তূপ_args)
{
  ffi_cif * ধ্রুবক cif = closure->cif;
  শূন্য ** ধ্রুবক avalue = alloca(cif->nargs * আকার(শূন্য *));
  শূন্য *rvalue;
  ffi_type ** ধ্রুবক arg_types = cif->arg_types;
  ffi_sarg * ধ্রুবক reg_args_in = reg_args[0];
  ffi_sarg * ধ্রুবক reg_args_out = reg_args[1];
  ffi_sarg * argp;
  long i, arg_word, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  সংঘ { ffi_sarg arg[অঙ্ক_ARG_REGS]; দ্বিগুণ d; UINT64 u64; } closure_ret;

  /* Start out reading register arguments. */
  argp = reg_args_in;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  যদি (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Return by reference via hidden pointer. */
      rvalue = (শূন্য *)(intptr_t)*argp++;
      arg_word = 1;
    }
  অন্যথা
    {
      /* Return the value in registers. */
      rvalue = &closure_ret;
      arg_word = 0;
    }

  /* Grab the addresses of the arguments. */
  জন্য (i = 0; i < nargs; i++)
    {
      ffi_type * ধ্রুবক type = arg_types[i];
      ধ্রুবক size_t arg_আকার_in_শব্দগুলি =
        (type->আকার_2 + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#ifndef __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      long align = arg_word & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_word += align;
#endif

      যদি (arg_word == অঙ্ক_ARG_REGS ||
          (arg_word < অঙ্ক_ARG_REGS &&
           arg_word + arg_আকার_in_শব্দগুলি > অঙ্ক_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = স্তূপ_args;
          arg_word = অঙ্ক_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_আকার_in_শব্দগুলি;
      arg_word += arg_আকার_in_শব্দগুলি;
    }

  /* Invoke the closure.  */
  closure->fun(cif, rvalue, avalue, closure->user_তথ্য);

  যদি (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      assign_to_ffi_arg(reg_args_out, &closure_ret, cif->rtype, 1);
    }
}
