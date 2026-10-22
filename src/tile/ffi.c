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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <arch/abi.h>
#அந்தர்பாவ <arch/icache.h>
#அந்தர்பாவ <arch/opcode.h>


/* The first 10 registers are used to pass arguments and return values. */
#define இலக்கம்_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
வெளி வெறுமை ffi_call_tile(ffi_sarg reg_args[இலக்கம்_ARG_REGS],
                          மாறிலி ffi_sarg *அடுக்கு_args,
                          size_t அடுக்கு_args_bytes,
                          வெறுமை (*fnaddr)(வெறுமை))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
வெளி வெறுமை ffi_closure_tile(வெறுமை) FFI_HIDDEN;


ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  எனில் (cif->bytes < இலக்கம்_ARG_REGS * FFI_SIZEOF_ARG)
    cif->bytes = இலக்கம்_ARG_REGS * FFI_SIZEOF_ARG;

  எனில் (cif->rtype->size > இலக்கம்_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  இல்லையெனில்
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  திரும்பு FFI_OK;
}


நிலையான long
assign_to_ffi_arg(ffi_sarg *out, வெறுமை *in, மாறிலி ffi_type *type,
                  முழு write_to_reg)
{
  தேர்வு (type->type)
    {
    நிலை FFI_TYPE_SINT8:
      *out = *(SINT8 *)in;
      திரும்பு 1;

    நிலை FFI_TYPE_UINT8:
      *out = *(UINT8 *)in;
      திரும்பு 1;

    நிலை FFI_TYPE_SINT16:
      *out = *(SINT16 *)in;
      திரும்பு 1;

    நிலை FFI_TYPE_UINT16:
      *out = *(UINT16 *)in;
      திரும்பு 1;

    நிலை FFI_TYPE_SINT32:
    நிலை FFI_TYPE_UINT32:
#ifndef __LP64__
    நிலை FFI_TYPE_POINTER:
#endif
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *out = *(SINT32 *)in;
      திரும்பு 1;

    நிலை FFI_TYPE_FLOAT:
#ifdef __tilegx__
      எனில் (write_to_reg)
        {
          /* Properly sign extend the value.  */
          ஒன்றிப்பு { தசம f; SINT32 s32; } val;
          val.f = *(தசம *)in;
          *out = val.s32;
        }
      இல்லையெனில்
#endif
        {
          *(தசம *)out = *(தசம *)in;
        }
      திரும்பு 1;

    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_DOUBLE:
#ifdef __LP64__
    நிலை FFI_TYPE_POINTER:
#endif
      *(UINT64 *)out = *(UINT64 *)in;
      திரும்பு அளவு(UINT64) / FFI_SIZEOF_ARG;

    நிலை FFI_TYPE_STRUCT:
      memcpy(out, in, type->size);
      திரும்பு (type->size + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

    நிலை FFI_TYPE_VOID:
      /* Must be a return type. Nothing to do. */
      திரும்பு 0;

    இயல்பு:
      FFI_ASSERT(0);
      திரும்பு -1;
    }
}


வெறுமை
ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_sarg * மாறிலி arg_mem = alloca(cif->bytes);
  ffi_sarg * மாறிலி reg_args = arg_mem;
  ffi_sarg * மாறிலி அடுக்கு_args = &reg_args[இலக்கம்_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** மாறிலி arg_types = cif->arg_types;
  மாறிலி long இலக்கம்_args = cif->nargs;
  long i;

  எனில் (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->size));

      /* No more work needed to return anything. */
      rvalue = வெற்று;
    }

  க்கு (i = 0; i < இலக்கம்_args; i++)
    {
      ffi_type *type = arg_types[i];
      வெறுமை * மாறிலி arg_in = avalue[i];
      ptrdiff_t arg_word = argp - arg_mem;

#ifndef __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      long align = arg_word & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_word += align;
#endif

      எனில் (type->type == FFI_TYPE_STRUCT)
        {
          மாறிலி size_t arg_size_in_சொற்கள் =
            (type->size + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          எனில் (arg_word < இலக்கம்_ARG_REGS &&
              arg_word + arg_size_in_சொற்கள் > இலக்கம்_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = அடுக்கு_args;
            }

          memcpy(argp, arg_in, type->size);
          argp += arg_size_in_சொற்கள்;
        }
      இல்லையெனில்
        {
          argp += assign_to_ffi_arg(argp, arg_in, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_args, அடுக்கு_args,
                cif->bytes - (இலக்கம்_ARG_REGS * FFI_SIZEOF_ARG), fn);

  எனில் (rvalue != வெற்று)
    assign_to_ffi_arg(rvalue, reg_args, cif->rtype, 0);
}


/* Template code for closure. */
வெளி மாறிலி UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif *cif,
                      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
                      வெறுமை *user_data,
                      வெறுமை *codeloc)
{
#ifdef __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  முழு s;
  UINT64 *out;

  எனில் (cif->abi != FFI_UNIX)
    திரும்பு FFI_BAD_ABI;

  out = (UINT64 *)closure->tramp;

  c = (intptr_t)closure;
  h = (intptr_t)ffi_closure_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  போது ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#define OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *out++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  க்கு (s -= 16; s >= 0; s -= 16)
    *out++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#undef OPS

  *out++ = ffi_template_tramp_tile[2];

#இல்லையெனில்
  /* TILEPro */
  UINT64 *out;
  intptr_t delta;

  எனில் (cif->abi != FFI_UNIX)
    திரும்பு FFI_BAD_ABI;

  out = (UINT64 *)closure->tramp;
  delta = (intptr_t)ffi_closure_tile - (intptr_t)codeloc;

  *out++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  invalidate_icache(closure->tramp, (எழுத்து *)out - closure->tramp,
                    getpagesize());

  திரும்பு FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
வெறுமை FFI_HIDDEN
ffi_closure_tile_inner(ffi_closure *closure,
                       ffi_sarg reg_args[2][இலக்கம்_ARG_REGS],
                       ffi_sarg *அடுக்கு_args)
{
  ffi_cif * மாறிலி cif = closure->cif;
  வெறுமை ** மாறிலி avalue = alloca(cif->nargs * அளவு(வெறுமை *));
  வெறுமை *rvalue;
  ffi_type ** மாறிலி arg_types = cif->arg_types;
  ffi_sarg * மாறிலி reg_args_in = reg_args[0];
  ffi_sarg * மாறிலி reg_args_out = reg_args[1];
  ffi_sarg * argp;
  long i, arg_word, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  ஒன்றிப்பு { ffi_sarg arg[இலக்கம்_ARG_REGS]; இரட்டை d; UINT64 u64; } closure_ret;

  /* Start out reading register arguments. */
  argp = reg_args_in;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  எனில் (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Return by reference via hidden pointer. */
      rvalue = (வெறுமை *)(intptr_t)*argp++;
      arg_word = 1;
    }
  இல்லையெனில்
    {
      /* Return the value in registers. */
      rvalue = &closure_ret;
      arg_word = 0;
    }

  /* Grab the addresses of the arguments. */
  க்கு (i = 0; i < nargs; i++)
    {
      ffi_type * மாறிலி type = arg_types[i];
      மாறிலி size_t arg_size_in_சொற்கள் =
        (type->size + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#ifndef __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      long align = arg_word & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_word += align;
#endif

      எனில் (arg_word == இலக்கம்_ARG_REGS ||
          (arg_word < இலக்கம்_ARG_REGS &&
           arg_word + arg_size_in_சொற்கள் > இலக்கம்_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = அடுக்கு_args;
          arg_word = இலக்கம்_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_size_in_சொற்கள்;
      arg_word += arg_size_in_சொற்கள்;
    }

  /* Invoke the closure.  */
  closure->fun(cif, rvalue, avalue, closure->user_data);

  எனில் (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      assign_to_ffi_arg(reg_args_out, &closure_ret, cif->rtype, 1);
    }
}
