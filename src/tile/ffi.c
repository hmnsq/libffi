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

#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश <stdlib.h>
#समावेश <stdint.h>
#समावेश <unistd.h>
#समावेश <arch/abi.h>
#समावेश <arch/icache.h>
#समावेश <arch/opcode.h>


/* The first 10 registers are used to pass arguments and return values. */
#घोषणा अंक_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
बाह्य व्योम ffi_call_tile(ffi_sarg reg_args[अंक_ARG_REGS],
                          स्थिर ffi_sarg *ढेर_args,
                          माप_प्रकार ढेर_args_bytes,
                          व्योम (*fnaddr)(व्योम))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
बाह्य व्योम ffi_closure_tile(व्योम) FFI_HIDDEN;


ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  अगर (cif->bytes < अंक_ARG_REGS * FFI_SIZEOF_ARG)
    cif->bytes = अंक_ARG_REGS * FFI_SIZEOF_ARG;

  अगर (cif->rtype->आकार > अंक_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  अन्यथा
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  वापस FFI_OK;
}


अटल दीर्घ
assign_to_ffi_arg(ffi_sarg *out, व्योम *in, स्थिर ffi_type *type,
                  पूर्णांक write_to_reg)
{
  चयन (type->type)
    {
    हाल FFI_TYPE_SINT8:
      *out = *(SINT8 *)in;
      वापस 1;

    हाल FFI_TYPE_UINT8:
      *out = *(UINT8 *)in;
      वापस 1;

    हाल FFI_TYPE_SINT16:
      *out = *(SINT16 *)in;
      वापस 1;

    हाल FFI_TYPE_UINT16:
      *out = *(UINT16 *)in;
      वापस 1;

    हाल FFI_TYPE_SINT32:
    हाल FFI_TYPE_UINT32:
#अगर_अघोषित __LP64__
    हाल FFI_TYPE_POINTER:
#पूर्ण_अगर
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *out = *(SINT32 *)in;
      वापस 1;

    हाल FFI_TYPE_FLOAT:
#अगर_घोषित __tilegx__
      अगर (write_to_reg)
        {
          /* Properly sign extend the value.  */
          जोड़ { भग्न f; SINT32 s32; } val;
          val.f = *(भग्न *)in;
          *out = val.s32;
        }
      अन्यथा
#पूर्ण_अगर
        {
          *(भग्न *)out = *(भग्न *)in;
        }
      वापस 1;

    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_DOUBLE:
#अगर_घोषित __LP64__
    हाल FFI_TYPE_POINTER:
#पूर्ण_अगर
      *(UINT64 *)out = *(UINT64 *)in;
      वापस माप(UINT64) / FFI_SIZEOF_ARG;

    हाल FFI_TYPE_STRUCT:
      स_नकल(out, in, type->आकार);
      वापस (type->आकार + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

    हाल FFI_TYPE_VOID:
      /* Must be a return type. Nothing to do. */
      वापस 0;

    शेष:
      FFI_ASSERT(0);
      वापस -1;
    }
}


व्योम
ffi_call(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  ffi_sarg * स्थिर arg_mem = alloca(cif->bytes);
  ffi_sarg * स्थिर reg_args = arg_mem;
  ffi_sarg * स्थिर ढेर_args = &reg_args[अंक_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** स्थिर arg_types = cif->arg_types;
  स्थिर दीर्घ अंक_args = cif->nargs;
  दीर्घ i;

  अगर (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->आकार));

      /* No more work needed to return anything. */
      rvalue = शून्य;
    }

  क्रम (i = 0; i < अंक_args; i++)
    {
      ffi_type *type = arg_types[i];
      व्योम * स्थिर arg_in = avalue[i];
      सूचक_भेद_प्रकार arg_word = argp - arg_mem;

#अगर_अघोषित __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      दीर्घ align = arg_word & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_word += align;
#पूर्ण_अगर

      अगर (type->type == FFI_TYPE_STRUCT)
        {
          स्थिर माप_प्रकार arg_आकार_in_शब्दगण =
            (type->आकार + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          अगर (arg_word < अंक_ARG_REGS &&
              arg_word + arg_आकार_in_शब्दगण > अंक_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = ढेर_args;
            }

          स_नकल(argp, arg_in, type->आकार);
          argp += arg_आकार_in_शब्दगण;
        }
      अन्यथा
        {
          argp += assign_to_ffi_arg(argp, arg_in, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_args, ढेर_args,
                cif->bytes - (अंक_ARG_REGS * FFI_SIZEOF_ARG), fn);

  अगर (rvalue != शून्य)
    assign_to_ffi_arg(rvalue, reg_args, cif->rtype, 0);
}


/* Template code for closure. */
बाह्य स्थिर UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif *cif,
                      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
                      व्योम *user_तथ्य,
                      व्योम *codeloc)
{
#अगर_घोषित __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  पूर्णांक s;
  UINT64 *out;

  अगर (cif->abi != FFI_UNIX)
    वापस FFI_BAD_ABI;

  out = (UINT64 *)closure->tramp;

  c = (intptr_t)closure;
  h = (intptr_t)ffi_closure_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  जबतक ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#घोषणा OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *out++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  क्रम (s -= 16; s >= 0; s -= 16)
    *out++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#अघोषित OPS

  *out++ = ffi_template_tramp_tile[2];

#अन्यथा
  /* TILEPro */
  UINT64 *out;
  intptr_t delta;

  अगर (cif->abi != FFI_UNIX)
    वापस FFI_BAD_ABI;

  out = (UINT64 *)closure->tramp;
  delta = (intptr_t)ffi_closure_tile - (intptr_t)codeloc;

  *out++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#पूर्ण_अगर

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  invalidate_icache(closure->tramp, (अक्षर *)out - closure->tramp,
                    getpagesize());

  वापस FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
व्योम FFI_HIDDEN
ffi_closure_tile_inner(ffi_closure *closure,
                       ffi_sarg reg_args[2][अंक_ARG_REGS],
                       ffi_sarg *ढेर_args)
{
  ffi_cif * स्थिर cif = closure->cif;
  व्योम ** स्थिर avalue = alloca(cif->nargs * माप(व्योम *));
  व्योम *rvalue;
  ffi_type ** स्थिर arg_types = cif->arg_types;
  ffi_sarg * स्थिर reg_args_in = reg_args[0];
  ffi_sarg * स्थिर reg_args_out = reg_args[1];
  ffi_sarg * argp;
  दीर्घ i, arg_word, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  जोड़ { ffi_sarg arg[अंक_ARG_REGS]; द्विगुन d; UINT64 u64; } closure_ret;

  /* Start out reading register arguments. */
  argp = reg_args_in;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  अगर (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Return by reference via hidden pointer. */
      rvalue = (व्योम *)(intptr_t)*argp++;
      arg_word = 1;
    }
  अन्यथा
    {
      /* Return the value in registers. */
      rvalue = &closure_ret;
      arg_word = 0;
    }

  /* Grab the addresses of the arguments. */
  क्रम (i = 0; i < nargs; i++)
    {
      ffi_type * स्थिर type = arg_types[i];
      स्थिर माप_प्रकार arg_आकार_in_शब्दगण =
        (type->आकार + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#अगर_अघोषित __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      दीर्घ align = arg_word & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_word += align;
#पूर्ण_अगर

      अगर (arg_word == अंक_ARG_REGS ||
          (arg_word < अंक_ARG_REGS &&
           arg_word + arg_आकार_in_शब्दगण > अंक_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = ढेर_args;
          arg_word = अंक_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_आकार_in_शब्दगण;
      arg_word += arg_आकार_in_शब्दगण;
    }

  /* Invoke the closure.  */
  closure->fun(cif, rvalue, avalue, closure->user_तथ्य);

  अगर (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      assign_to_ffi_arg(reg_args_out, &closure_ret, cif->rtype, 1);
    }
}
