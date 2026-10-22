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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <arch/abi.h>
#अन्तर्भाव <arch/icache.h>
#अन्तर्भाव <arch/opcode.h>


/* The first 10 registers are used to pass arguments and return values. */
#परिभाषय अङ्कः_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
बाह्य शून्य ffi_call_tile(ffi_sarg reg_तर्काः[अङ्कः_ARG_REGS],
                          ध्रुव ffi_sarg *राशिः_तर्काः,
                          size_t राशिः_तर्काः_अष्टकानि,
                          शून्य (*fnaddr)(शून्य))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
बाह्य शून्य ffi_संवृतिः_tile(शून्य) FFI_HIDDEN;


ffi_स्थितिः
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  यदि (cif->अष्टकानि < अङ्कः_ARG_REGS * FFI_SIZEOF_ARG)
    cif->अष्टकानि = अङ्कः_ARG_REGS * FFI_SIZEOF_ARG;

  यदि (cif->rtype->आकार > अङ्कः_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  अन्यथा
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  प्रत्यागम FFI_OK;
}


स्थिर दीर्घ
नियोजय_गन्तव्यम्_ffi_arg(ffi_sarg *बहिर्गमनम्, शून्य *अन्तर्, ध्रुव ffi_type *type,
                  पूर्णाङ्क write_गन्तव्यम्_reg)
{
  विकल्प (type->type)
    {
    पक्ष FFI_TYPE_SINT8:
      *बहिर्गमनम् = *(SINT8 *)अन्तर्;
      प्रत्यागम 1;

    पक्ष FFI_TYPE_UINT8:
      *बहिर्गमनम् = *(UINT8 *)अन्तर्;
      प्रत्यागम 1;

    पक्ष FFI_TYPE_SINT16:
      *बहिर्गमनम् = *(SINT16 *)अन्तर्;
      प्रत्यागम 1;

    पक्ष FFI_TYPE_UINT16:
      *बहिर्गमनम् = *(UINT16 *)अन्तर्;
      प्रत्यागम 1;

    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_UINT32:
#यदि_अपरिभाषितम् __LP64__
    पक्ष FFI_TYPE_POINTER:
#यदि_अन्तः
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *बहिर्गमनम् = *(SINT32 *)अन्तर्;
      प्रत्यागम 1;

    पक्ष FFI_TYPE_FLOAT:
#यदि_परिभाषितम् __tilegx__
      यदि (write_गन्तव्यम्_reg)
        {
          /* Properly sign extend the value.  */
          सङ्घ { भिन्न f; SINT32 s32; } val;
          val.f = *(भिन्न *)अन्तर्;
          *बहिर्गमनम् = val.s32;
        }
      अन्यथा
#यदि_अन्तः
        {
          *(भिन्न *)बहिर्गमनम् = *(भिन्न *)अन्तर्;
        }
      प्रत्यागम 1;

    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_DOUBLE:
#यदि_परिभाषितम् __LP64__
    पक्ष FFI_TYPE_POINTER:
#यदि_अन्तः
      *(UINT64 *)बहिर्गमनम् = *(UINT64 *)अन्तर्;
      प्रत्यागम परिमाण(UINT64) / FFI_SIZEOF_ARG;

    पक्ष FFI_TYPE_STRUCT:
      स्मृतिं_प्रतिलिख(बहिर्गमनम्, अन्तर्, type->आकार);
      प्रत्यागम (type->आकार + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

    पक्ष FFI_TYPE_VOID:
      /* Must be a return type. Nothing to do. */
      प्रत्यागम 0;

    स्वभाव:
      FFI_ASSERT(0);
      प्रत्यागम -1;
    }
}


शून्य
ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_sarg * ध्रुव arg_mem = alloca(cif->अष्टकानि);
  ffi_sarg * ध्रुव reg_तर्काः = arg_mem;
  ffi_sarg * ध्रुव राशिः_तर्काः = &reg_तर्काः[अङ्कः_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** ध्रुव arg_types = cif->arg_types;
  ध्रुव दीर्घ अङ्कः_तर्काः = cif->nargs;
  दीर्घ i;

  यदि (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->आकार));

      /* No more work needed to return anything. */
      rvalue = रिक्त;
    }

  प्रति (i = 0; i < अङ्कः_तर्काः; i++)
    {
      ffi_type *type = arg_types[i];
      शून्य * ध्रुव arg_अन्तर् = avalue[i];
      ptrdiff_t arg_word = argp - arg_mem;

#यदि_अपरिभाषितम् __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      दीर्घ align = arg_word & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_word += align;
#यदि_अन्तः

      यदि (type->type == FFI_TYPE_STRUCT)
        {
          ध्रुव size_t arg_आकार_अन्तर्_शब्दाः =
            (type->आकार + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          यदि (arg_word < अङ्कः_ARG_REGS &&
              arg_word + arg_आकार_अन्तर्_शब्दाः > अङ्कः_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = राशिः_तर्काः;
            }

          स्मृतिं_प्रतिलिख(argp, arg_अन्तर्, type->आकार);
          argp += arg_आकार_अन्तर्_शब्दाः;
        }
      अन्यथा
        {
          argp += नियोजय_गन्तव्यम्_ffi_arg(argp, arg_अन्तर्, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_तर्काः, राशिः_तर्काः,
                cif->अष्टकानि - (अङ्कः_ARG_REGS * FFI_SIZEOF_ARG), fn);

  यदि (rvalue != रिक्त)
    नियोजय_गन्तव्यम्_ffi_arg(rvalue, reg_तर्काः, cif->rtype, 0);
}


/* Template code for closure. */
बाह्य ध्रुव UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः *संवृतिः,
                      ffi_cif *cif,
                      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
                      शून्य *user_दत्त,
                      शून्य *codeloc)
{
#यदि_परिभाषितम् __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  पूर्णाङ्क s;
  UINT64 *बहिर्गमनम्;

  यदि (cif->abi != FFI_UNIX)
    प्रत्यागम FFI_BAD_ABI;

  बहिर्गमनम् = (UINT64 *)संवृतिः->tramp;

  c = (intptr_t)संवृतिः;
  h = (intptr_t)ffi_संवृतिः_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  यावत् ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#परिभाषय OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *बहिर्गमनम्++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  प्रति (s -= 16; s >= 0; s -= 16)
    *बहिर्गमनम्++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#अपरिभाषय OPS

  *बहिर्गमनम्++ = ffi_template_tramp_tile[2];

#अन्यथा
  /* TILEPro */
  UINT64 *बहिर्गमनम्;
  intptr_t delta;

  यदि (cif->abi != FFI_UNIX)
    प्रत्यागम FFI_BAD_ABI;

  बहिर्गमनम् = (UINT64 *)संवृतिः->tramp;
  delta = (intptr_t)ffi_संवृतिः_tile - (intptr_t)codeloc;

  *बहिर्गमनम्++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#यदि_अन्तः

  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;

  invalidate_icache(संवृतिः->tramp, (वर्ण *)बहिर्गमनम् - संवृतिः->tramp,
                    getpagesize());

  प्रत्यागम FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
शून्य FFI_HIDDEN
ffi_संवृतिः_tile_inner(ffi_संवृतिः *संवृतिः,
                       ffi_sarg reg_तर्काः[2][अङ्कः_ARG_REGS],
                       ffi_sarg *राशिः_तर्काः)
{
  ffi_cif * ध्रुव cif = संवृतिः->cif;
  शून्य ** ध्रुव avalue = alloca(cif->nargs * परिमाण(शून्य *));
  शून्य *rvalue;
  ffi_type ** ध्रुव arg_types = cif->arg_types;
  ffi_sarg * ध्रुव reg_तर्काः_अन्तर् = reg_तर्काः[0];
  ffi_sarg * ध्रुव reg_तर्काः_बहिर्गमनम् = reg_तर्काः[1];
  ffi_sarg * argp;
  दीर्घ i, arg_word, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  सङ्घ { ffi_sarg arg[अङ्कः_ARG_REGS]; द्विगुण d; UINT64 u64; } संवृतिः_ret;

  /* Start out reading register arguments. */
  argp = reg_तर्काः_अन्तर्;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  यदि (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Return by reference via hidden pointer. */
      rvalue = (शून्य *)(intptr_t)*argp++;
      arg_word = 1;
    }
  अन्यथा
    {
      /* Return the value in registers. */
      rvalue = &संवृतिः_ret;
      arg_word = 0;
    }

  /* Grab the addresses of the arguments. */
  प्रति (i = 0; i < nargs; i++)
    {
      ffi_type * ध्रुव type = arg_types[i];
      ध्रुव size_t arg_आकार_अन्तर्_शब्दाः =
        (type->आकार + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#यदि_अपरिभाषितम् __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      दीर्घ align = arg_word & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_word += align;
#यदि_अन्तः

      यदि (arg_word == अङ्कः_ARG_REGS ||
          (arg_word < अङ्कः_ARG_REGS &&
           arg_word + arg_आकार_अन्तर्_शब्दाः > अङ्कः_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = राशिः_तर्काः;
          arg_word = अङ्कः_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_आकार_अन्तर्_शब्दाः;
      arg_word += arg_आकार_अन्तर्_शब्दाः;
    }

  /* Invoke the closure.  */
  संवृतिः->fun(cif, rvalue, avalue, संवृतिः->user_दत्त);

  यदि (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      नियोजय_गन्तव्यम्_ffi_arg(reg_तर्काः_बहिर्गमनम्, &संवृतिः_ret, cif->rtype, 1);
    }
}
