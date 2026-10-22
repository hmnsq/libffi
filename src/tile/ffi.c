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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <arch/abi.h>
#అన్తర్భావ <arch/icache.h>
#అన్తర్భావ <arch/opcode.h>


/* The first 10 registers are used to pass arguments and return values. */
#నిర్వచించు అంకె_ARG_REGS 10

/* Performs a raw function call with the given NUM_ARG_REGS register arguments
   and the specified additional stack arguments (if any). */
బాహ్య శూన్య ffi_call_tile(ffi_sarg reg_వాదనలు[అంకె_ARG_REGS],
                          స్థిరరాశి ffi_sarg *దొంతర_వాదనలు,
                          size_t దొంతర_వాదనలు_బైట్లు,
                          శూన్య (*fnaddr)(శూన్య))
  FFI_HIDDEN;

/* This handles the raw call from the closure stub, cleaning up the
   parameters and delegating to ffi_closure_tile_inner. */
బాహ్య శూన్య ffi_సంవృతం_tile(శూన్య) FFI_HIDDEN;


ffi_స్థితి
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* We always allocate room for all registers. Even if we don't
     use them as parameters, they get returned in the same array
     as struct return values so we need to make room. */
  ఒకవేళ (cif->బైట్లు < అంకె_ARG_REGS * FFI_SIZEOF_ARG)
    cif->బైట్లు = అంకె_ARG_REGS * FFI_SIZEOF_ARG;

  ఒకవేళ (cif->rtype->కొలత > అంకె_ARG_REGS * FFI_SIZEOF_ARG)
    cif->flags = FFI_TYPE_STRUCT;
  లేకపోతే
    cif->flags = FFI_TYPE_INT;

  /* Nothing to do. */
  తిరిగి FFI_OK;
}


స్థిర దీర్ఘ
కేటాయించు_గమ్యం_ffi_arg(ffi_sarg *బహిర్గతం, శూన్య *లోపల, స్థిరరాశి ffi_type *type,
                  పూర్ణాంకం write_గమ్యం_reg)
{
  ఎంపిక (type->type)
    {
    స్థితి FFI_TYPE_SINT8:
      *బహిర్గతం = *(SINT8 *)లోపల;
      తిరిగి 1;

    స్థితి FFI_TYPE_UINT8:
      *బహిర్గతం = *(UINT8 *)లోపల;
      తిరిగి 1;

    స్థితి FFI_TYPE_SINT16:
      *బహిర్గతం = *(SINT16 *)లోపల;
      తిరిగి 1;

    స్థితి FFI_TYPE_UINT16:
      *బహిర్గతం = *(UINT16 *)లోపల;
      తిరిగి 1;

    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_UINT32:
#నిర్వచితం_కాకపోతే __LP64__
    స్థితి FFI_TYPE_సూచిక:
#అయితే_ముగింపు
      /* Note that even unsigned 32-bit quantities are sign extended
         on tilegx when stored in a register.  */
      *బహిర్గతం = *(SINT32 *)లోపల;
      తిరిగి 1;

    స్థితి FFI_TYPE_FLOAT:
#నిర్వచితమైతే __tilegx__
      ఒకవేళ (write_గమ్యం_reg)
        {
          /* Properly sign extend the value.  */
          సంఘం { దశాంశం f; SINT32 s32; } val;
          val.f = *(దశాంశం *)లోపల;
          *బహిర్గతం = val.s32;
        }
      లేకపోతే
#అయితే_ముగింపు
        {
          *(దశాంశం *)బహిర్గతం = *(దశాంశం *)లోపల;
        }
      తిరిగి 1;

    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_DOUBLE:
#నిర్వచితమైతే __LP64__
    స్థితి FFI_TYPE_సూచిక:
#అయితే_ముగింపు
      *(UINT64 *)బహిర్గతం = *(UINT64 *)లోపల;
      తిరిగి పరిమాణం(UINT64) / FFI_SIZEOF_ARG;

    స్థితి FFI_TYPE_STRUCT:
      జ్ఞాపకం_నకలు(బహిర్గతం, లోపల, type->కొలత);
      తిరిగి (type->కొలత + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

    స్థితి FFI_TYPE_VOID:
      /* Must be a return type. Nothing to do. */
      తిరిగి 0;

    అప్రమేయ:
      FFI_ASSERT(0);
      తిరిగి -1;
    }
}


శూన్య
ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_sarg * స్థిరరాశి arg_mem = alloca(cif->బైట్లు);
  ffi_sarg * స్థిరరాశి reg_వాదనలు = arg_mem;
  ffi_sarg * స్థిరరాశి దొంతర_వాదనలు = &reg_వాదనలు[అంకె_ARG_REGS];
  ffi_sarg *argp = arg_mem;
  ffi_type ** స్థిరరాశి arg_types = cif->arg_types;
  స్థిరరాశి దీర్ఘ అంకె_వాదనలు = cif->nargs;
  దీర్ఘ i;

  ఒకవేళ (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Pass a hidden pointer to the return value. We make sure there
         is scratch space for the callee to store the return value even if
         our caller doesn't care about it. */
      *argp++ = (intptr_t)(rvalue ? rvalue : alloca(cif->rtype->కొలత));

      /* No more work needed to return anything. */
      rvalue = ఖాళీ;
    }

  కొరకు (i = 0; i < అంకె_వాదనలు; i++)
    {
      ffi_type *type = arg_types[i];
      శూన్య * స్థిరరాశి arg_లోపల = avalue[i];
      ptrdiff_t arg_మాట = argp - arg_mem;

#నిర్వచితం_కాకపోతే __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      దీర్ఘ align = arg_మాట & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_మాట += align;
#అయితే_ముగింపు

      ఒకవేళ (type->type == FFI_TYPE_STRUCT)
        {
          స్థిరరాశి size_t arg_కొలత_లోపల_పదాలు =
            (type->కొలత + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

          ఒకవేళ (arg_మాట < అంకె_ARG_REGS &&
              arg_మాట + arg_కొలత_లోపల_పదాలు > అంకె_ARG_REGS)
            {
              /* Args are not allowed to span registers and the stack. */
              argp = దొంతర_వాదనలు;
            }

          జ్ఞాపకం_నకలు(argp, arg_లోపల, type->కొలత);
          argp += arg_కొలత_లోపల_పదాలు;
        }
      లేకపోతే
        {
          argp += కేటాయించు_గమ్యం_ffi_arg(argp, arg_లోపల, arg_types[i], 1);
        }
    }

  /* Actually do the call. */
  ffi_call_tile(reg_వాదనలు, దొంతర_వాదనలు,
                cif->బైట్లు - (అంకె_ARG_REGS * FFI_SIZEOF_ARG), fn);

  ఒకవేళ (rvalue != ఖాళీ)
    కేటాయించు_గమ్యం_ffi_arg(rvalue, reg_వాదనలు, cif->rtype, 0);
}


/* Template code for closure. */
బాహ్య స్థిరరాశి UINT64 ffi_template_tramp_tile[] FFI_HIDDEN;


ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం *సంవృతం,
                      ffi_cif *cif,
                      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
                      శూన్య *user_దత్తాంశం,
                      శూన్య *codeloc)
{
#నిర్వచితమైతే __tilegx__
  /* TILE-Gx */
  SINT64 c;
  SINT64 h;
  పూర్ణాంకం s;
  UINT64 *బహిర్గతం;

  ఒకవేళ (cif->abi != FFI_UNIX)
    తిరిగి FFI_BAD_ABI;

  బహిర్గతం = (UINT64 *)సంవృతం->tramp;

  c = (intptr_t)సంవృతం;
  h = (intptr_t)ffi_సంవృతం_tile;
  s = 0;

  /* Find the smallest shift count that doesn't lose information
     (i.e. no need to explicitly insert high bits of the address that
     are just the sign extension of the low bits). */
  అయితే ((c >> s) != (SINT16)(c >> s) || (h >> s) != (SINT16)(h >> s))
    s += 16;

#నిర్వచించు OPS(a, b, shift) \
  (create_Imm16_X0((a) >> (shift)) | create_Imm16_X1((b) >> (shift)))

  /* Emit the moveli. */
  *బహిర్గతం++ = ffi_template_tramp_tile[0] | OPS(c, h, s);
  కొరకు (s -= 16; s >= 0; s -= 16)
    *బహిర్గతం++ = ffi_template_tramp_tile[1] | OPS(c, h, s);

#నిర్వచనం_రద్దు OPS

  *బహిర్గతం++ = ffi_template_tramp_tile[2];

#లేకపోతే
  /* TILEPro */
  UINT64 *బహిర్గతం;
  intptr_t delta;

  ఒకవేళ (cif->abi != FFI_UNIX)
    తిరిగి FFI_BAD_ABI;

  బహిర్గతం = (UINT64 *)సంవృతం->tramp;
  delta = (intptr_t)ffi_సంవృతం_tile - (intptr_t)codeloc;

  *బహిర్గతం++ = ffi_template_tramp_tile[0] | create_JOffLong_X1(delta >> 3);
#అయితే_ముగింపు

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  invalidate_icache(సంవృతం->tramp, (అక్షరం *)బహిర్గతం - సంవృతం->tramp,
                    getpagesize());

  తిరిగి FFI_OK;
}


/* This is called by the assembly wrapper for closures. This does
   all of the work. On entry reg_args[0] holds the values the registers
   had when the closure was invoked. On return reg_args[1] holds the register
   values to be returned to the caller (many of which may be garbage). */
శూన్య FFI_HIDDEN
ffi_సంవృతం_tile_inner(ffi_సంవృతం *సంవృతం,
                       ffi_sarg reg_వాదనలు[2][అంకె_ARG_REGS],
                       ffi_sarg *దొంతర_వాదనలు)
{
  ffi_cif * స్థిరరాశి cif = సంవృతం->cif;
  శూన్య ** స్థిరరాశి avalue = alloca(cif->nargs * పరిమాణం(శూన్య *));
  శూన్య *rvalue;
  ffi_type ** స్థిరరాశి arg_types = cif->arg_types;
  ffi_sarg * స్థిరరాశి reg_వాదనలు_లోపల = reg_వాదనలు[0];
  ffi_sarg * స్థిరరాశి reg_వాదనలు_బహిర్గతం = reg_వాదనలు[1];
  ffi_sarg * argp;
  దీర్ఘ i, arg_మాట, nargs = cif->nargs;
  /* Use a union to guarantee proper alignment for double. */
  సంఘం { ffi_sarg arg[అంకె_ARG_REGS]; ద్విగుణ d; UINT64 u64; } సంవృతం_ret;

  /* Start out reading register arguments. */
  argp = reg_వాదనలు_లోపల;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  ఒకవేళ (cif->flags == FFI_TYPE_STRUCT)
    {
      /* Return by reference via hidden pointer. */
      rvalue = (శూన్య *)(intptr_t)*argp++;
      arg_మాట = 1;
    }
  లేకపోతే
    {
      /* Return the value in registers. */
      rvalue = &సంవృతం_ret;
      arg_మాట = 0;
    }

  /* Grab the addresses of the arguments. */
  కొరకు (i = 0; i < nargs; i++)
    {
      ffi_type * స్థిరరాశి type = arg_types[i];
      స్థిరరాశి size_t arg_కొలత_లోపల_పదాలు =
        (type->కొలత + FFI_SIZEOF_ARG - 1) / FFI_SIZEOF_ARG;

#నిర్వచితం_కాకపోతే __tilegx__
      /* Doubleword-aligned values are always in an even-number register
         pair, or doubleword-aligned stack slot if out of registers. */
      దీర్ఘ align = arg_మాట & (type->alignment > FFI_SIZEOF_ARG);
      argp += align;
      arg_మాట += align;
#అయితే_ముగింపు

      ఒకవేళ (arg_మాట == అంకె_ARG_REGS ||
          (arg_మాట < అంకె_ARG_REGS &&
           arg_మాట + arg_కొలత_లోపల_పదాలు > అంకె_ARG_REGS))
        {
          /* Switch to reading arguments from the stack. */
          argp = దొంతర_వాదనలు;
          arg_మాట = అంకె_ARG_REGS;
        }

      avalue[i] = argp;
      argp += arg_కొలత_లోపల_పదాలు;
      arg_మాట += arg_కొలత_లోపల_పదాలు;
    }

  /* Invoke the closure.  */
  సంవృతం->fun(cif, rvalue, avalue, సంవృతం->user_దత్తాంశం);

  ఒకవేళ (cif->flags != FFI_TYPE_STRUCT)
    {
      /* Canonicalize for register representation. */
      కేటాయించు_గమ్యం_ffi_arg(reg_వాదనలు_బహిర్గతం, &సంవృతం_ret, cif->rtype, 1);
    }
}
