/* -----------------------------------------------------------------------
   debug.c - Copyright (c) 1996 Red Hat, Inc.

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
#ضمن <stdio.h>

/* General debugging routines */

فراغ ffi_stop_دلته(فراغ)
{
  /* This function is only useful for debugging purposes.
     Place a breakpoint on ffi_stop_here to be notified of
     significant events. */
}

/* This function should only be called via the FFI_ASSERT() macro */

NORETURN فراغ ffi_assert(ثابت حرف *بیان, ثابت حرف *file, صحيح کرښه)
{
  fprintf(stderr, "ASSERTION FAILURE: %s at %s:%d\n", بیان, file, کرښه);
  ffi_stop_دلته();
  abort();
}

/* Perform a sanity check on an ffi_type structure */

فراغ ffi_type_test(ffi_type *a, ثابت حرف *file, صحيح کرښه)
{
  FFI_ASSERT_AT(a != عدم, file, کرښه);

  FFI_ASSERT_AT(a->type <= FFI_TYPE_وروستی, file, کرښه);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->اندازه > 0, file, کرښه);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->alignment > 0, file, کرښه);
  FFI_ASSERT_AT((a->type != FFI_TYPE_STRUCT && a->type != FFI_TYPE_COMPLEX
		 && a->type != FFI_TYPE_ویکټور)
		|| a->elements != عدم, file, کرښه);
  FFI_ASSERT_AT(a->type != FFI_TYPE_COMPLEX
		|| (a->elements != عدم
		    && a->elements[0] != عدم && a->elements[1] == عدم),
		file, کرښه);

}
