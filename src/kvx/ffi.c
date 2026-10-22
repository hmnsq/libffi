/* Copyright (c) 2020 Kalray

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
``Software''), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#अगर defined(__kvx__)
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <stdint.h>
#समावेश <fficonfig.h>
#समावेश <ffi.h>
#समावेश "ffi_common.h"
#समावेश "asm.h"

#घोषणा ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)
#घोषणा ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#घोषणा KVX_ABI_ढेर_ALIGNMENT (32)
#घोषणा KVX_ABI_ढेर_ARG_ALIGNMENT (8)
#घोषणा max(a,b) ((a) > (b) ? (a) : (b))

#अगर_घोषित FFI_DEBUG
#घोषणा DEBUG_PRINT(...) करो{ ख_लिखो( मानक_त्रुटि, __VA_ARGS__ ); } जबतक(0)
#अन्यथा
#घोषणा DEBUG_PRINT(...)
#पूर्ण_अगर

काष्ठा ret_मूल्य {
	अचिन्हित दीर्घ पूर्णांक r0;
	अचिन्हित दीर्घ पूर्णांक r1;
	अचिन्हित दीर्घ पूर्णांक r2;
	अचिन्हित दीर्घ पूर्णांक r3;
};

बाह्य काष्ठा ret_मूल्य ffi_call_SYSV(अचिन्हित योग_आकार,
                                      अचिन्हित आकार,
                                      extended_cif *ecif,
                                      अचिन्हित *rvalue_addr,
                                      व्योम *fn,
                                      अचिन्हित int_ext_method);

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->flags = cif->rtype->आकार;
  वापस FFI_OK;
}

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

व्योम *ffi_prep_args(अक्षर *ढेर, अचिन्हित पूर्णांक arg_slots_आकार, extended_cif *ecif)
{
  अक्षर *stacktemp = ढेर;
  अक्षर *current_arg_passed_by_मूल्य = ढेर + arg_slots_आकार;
  पूर्णांक i, s;
  ffi_type **arg;
  पूर्णांक गिनती = 0;
  ffi_cif *cif = ecif->cif;
  व्योम **argv = ecif->avalue;

  arg = cif->arg_types;

  DEBUG_PRINT("stack: %p\n", ढेर);
  DEBUG_PRINT("arg_slots_size: %u\n", arg_slots_आकार);
  DEBUG_PRINT("current_arg_passed_by_value: %p\n", current_arg_passed_by_मूल्य);
  DEBUG_PRINT("ecif: %p\n", ecif);
  DEBUG_PRINT("ecif->avalue: %p\n", ecif->avalue);

  क्रम (i = 0; i < cif->nargs; i++) {

    s = KVX_ABI_SLOT_आकार;
    चयन((*arg)->type) {
      हाल FFI_TYPE_SINT8:
      हाल FFI_TYPE_UINT8:
      हाल FFI_TYPE_SINT16:
      हाल FFI_TYPE_UINT16:
      हाल FFI_TYPE_SINT32:
      हाल FFI_TYPE_UINT32:
      हाल FFI_TYPE_FLOAT:
      हाल FFI_TYPE_DOUBLE:
      हाल FFI_TYPE_UINT64:
      हाल FFI_TYPE_SINT64:
      हाल FFI_TYPE_POINTER:
        DEBUG_PRINT("INT64/32/16/8/FLOAT/DOUBLE or POINTER @%p\n", ढेर);
        *(uint64_t *) ढेर = *(uint64_t *)(* argv);
        अवरोध;

      हाल FFI_TYPE_COMPLEX:
        अगर ((*arg)->आकार == 8)
          *(_Complex भग्न *) ढेर = *(_Complex भग्न *)(* argv);
        अन्यथा अगर ((*arg)->आकार == 16) {
          *(_Complex द्विगुन *) ढेर = *(_Complex द्विगुन *)(* argv);
          s = 16;
        } अन्यथा
          पात();
        अवरोध;
      हाल FFI_TYPE_STRUCT: {
        अक्षर *मूल्य;
        अचिन्हित पूर्णांक written_आकार = 0;
        DEBUG_PRINT("struct by value @%p\n", ढेर);
        अगर ((*arg)->आकार > KVX_ABI_MAX_AGGREGATE_IN_REG_आकार) {
          DEBUG_PRINT("big struct\n");
          *(uint64_t *) ढेर = (uintptr_t)current_arg_passed_by_मूल्य;
          मूल्य = current_arg_passed_by_मूल्य;
          current_arg_passed_by_मूल्य += (*arg)->आकार;
          written_आकार = KVX_ABI_SLOT_आकार;
        } अन्यथा {
          मूल्य = ढेर;
          written_आकार = (*arg)->आकार;
        }
        स_नकल(मूल्य, *argv, (*arg)->आकार);
        s = ALIGN(written_आकार, KVX_ABI_ढेर_ARG_ALIGNMENT);
        अवरोध;
      }
      शेष:
        म_लिखो("Error: unsupported arg type %d\n", (*arg)->type);
        पात();
        अवरोध;

    }
    ढेर += s;
    गिनती += s;
    argv++;
    arg++;
  }
#अगर_घोषित FFI_DEBUG
  FFI_ASSERT(((intptr_t)(stacktemp + REG_ARGS_आकार) & (KVX_ABI_ढेर_ALIGNMENT-1)) == 0);
#पूर्ण_अगर
  वापस stacktemp + REG_ARGS_आकार;
}

/* Perform machine dependent cif processing when we have a variadic function */

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif, अचिन्हित पूर्णांक nfixedargs,
                                    अचिन्हित पूर्णांक ntotalargs)
{
  cif->flags = cif->rtype->आकार;
  वापस FFI_OK;
}

अटल अचिन्हित दीर्घ handle_small_int_ext(kvx_intext_method *int_ext_method,
                                          स्थिर ffi_type *rtype)
{
  चयन (rtype->type) {
    हाल FFI_TYPE_SINT8:
      *int_ext_method = KVX_RET_SXBD;
      वापस KVX_REGISTER_आकार;

    हाल FFI_TYPE_SINT16:
      *int_ext_method = KVX_RET_SXHD;
      वापस KVX_REGISTER_आकार;

    हाल FFI_TYPE_SINT32:
      *int_ext_method = KVX_RET_SXWD;
      वापस KVX_REGISTER_आकार;

    हाल FFI_TYPE_UINT8:
      *int_ext_method = KVX_RET_ZXBD;
      वापस KVX_REGISTER_आकार;

    हाल FFI_TYPE_UINT16:
      *int_ext_method = KVX_RET_ZXHD;
      वापस KVX_REGISTER_आकार;

    हाल FFI_TYPE_UINT32:
      *int_ext_method = KVX_RET_ZXWD;
      वापस KVX_REGISTER_आकार;

    शेष:
      *int_ext_method = KVX_RET_NONE;
      वापस rtype->आकार;
  }
}

व्योम ffi_call(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  पूर्णांक i;
  अचिन्हित दीर्घ पूर्णांक slot_fitting_args_आकार = 0;
  अचिन्हित दीर्घ पूर्णांक योग_आकार = 0;
  अचिन्हित दीर्घ पूर्णांक big_struct_आकार = 0;
  kvx_intext_method int_extension_method;
  ffi_type **arg;
  काष्ठा ret_मूल्य local_rvalue = {0};
  माप_प्रकार wb_आकार;


  /* Calculate size to allocate on stack */
  क्रम (i = 0, arg = cif->arg_types; i < cif->nargs; i++, arg++) {
    DEBUG_PRINT("argument %d, type %d, size %lu\n", i, (*arg)->type, (*arg)->आकार);
    अगर (((*arg)->type == FFI_TYPE_STRUCT) || ((*arg)->type == FFI_TYPE_COMPLEX)) {
      अगर ((*arg)->आकार <= KVX_ABI_MAX_AGGREGATE_IN_REG_आकार) {
        slot_fitting_args_आकार += ALIGN((*arg)->आकार, KVX_ABI_SLOT_आकार);
      } अन्यथा {
        slot_fitting_args_आकार += KVX_ABI_SLOT_आकार; /* aggregate passed by reference */
        big_struct_आकार += ALIGN((*arg)->आकार, KVX_ABI_SLOT_आकार);
      }
    } अन्यथा अगर ((*arg)->आकार <= KVX_ABI_SLOT_आकार) {
      slot_fitting_args_आकार += KVX_ABI_SLOT_आकार;
    } अन्यथा {
      म_लिखो("Error: unsupported arg size %ld arg type %d\n", (*arg)->आकार, (*arg)->type);
      पात(); /* should never happen? */
    }
  }

  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  /* This implementation allocates anyway for all register based args */
  slot_fitting_args_आकार = max(slot_fitting_args_आकार, REG_ARGS_आकार);
  योग_आकार = slot_fitting_args_आकार + big_struct_आकार;
  योग_आकार = ALIGN(योग_आकार, KVX_ABI_ढेर_ALIGNMENT);

  /* wb_size: write back size, the size we will need to write back to user
   * provided buffer. In theory it should always be cif->flags which is
   * cif->rtype->size. But libffi API mandates that for integral types
   * of size <= system register size, then we *MUST* write back
   * the size of system register size.
   * in our case, if size <= 8 bytes we must write back 8 bytes.
   * floats, complex and structs are not affected, only integrals.
   */
  wb_आकार = handle_small_int_ext(&int_extension_method, cif->rtype);

  चयन (cif->abi) {
    हाल FFI_SYSV:
      DEBUG_PRINT("total_size: %lu\n", योग_आकार);
      DEBUG_PRINT("slot fitting args size: %lu\n", slot_fitting_args_आकार);
      DEBUG_PRINT("rvalue: %p\n", rvalue);
      DEBUG_PRINT("fn: %p\n", fn);
      DEBUG_PRINT("rsize: %u\n", cif->flags);
      DEBUG_PRINT("wb_size: %u\n", wb_आकार);
      DEBUG_PRINT("int_extension_method: %u\n", int_extension_method);
      local_rvalue = ffi_call_SYSV(योग_आकार, slot_fitting_args_आकार,
                                   &ecif, rvalue, fn, int_extension_method);
      अगर ((cif->flags <= KVX_ABI_MAX_AGGREGATE_IN_REG_आकार)
          && (cif->rtype->type != FFI_TYPE_VOID))
        स_नकल(rvalue, &local_rvalue, wb_आकार);
      अवरोध;
    शेष:
      पात();
      अवरोध;
  }
}

/* Closures not supported yet */
ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
                      व्योम *user_तथ्य,
                      व्योम *codeloc)
{
  वापस FFI_BAD_ABI;
}

#पूर्ण_अगर /* (__kvx__) */
