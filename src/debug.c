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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>

/* General debugging routines */

शून्य ffi_stop_अत्र(शून्य)
{
  /* This function is only useful for debugging purposes.
     Place a breakpoint on ffi_stop_here to be notified of
     significant events. */
}

/* This function should only be called via the FFI_ASSERT() macro */

NORETURN शून्य ffi_assert(ध्रुव वर्ण *व्यञ्जकम्, ध्रुव वर्ण *file, पूर्णाङ्क पङ्क्तिः)
{
  fprintf(stderr, "ASSERTION FAILURE: %s at %s:%d\n", व्यञ्जकम्, file, पङ्क्तिः);
  ffi_stop_अत्र();
  abort();
}

/* Perform a sanity check on an ffi_type structure */

शून्य ffi_type_test(ffi_type *a, ध्रुव वर्ण *file, पूर्णाङ्क पङ्क्तिः)
{
  FFI_ASSERT_AT(a != रिक्त, file, पङ्क्तिः);

  FFI_ASSERT_AT(a->type <= FFI_TYPE_चरमः, file, पङ्क्तिः);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->आकार > 0, file, पङ्क्तिः);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->alignment > 0, file, पङ्क्तिः);
  FFI_ASSERT_AT((a->type != FFI_TYPE_STRUCT && a->type != FFI_TYPE_COMPLEX
		 && a->type != FFI_TYPE_सदिशम्)
		|| a->elements != रिक्त, file, पङ्क्तिः);
  FFI_ASSERT_AT(a->type != FFI_TYPE_COMPLEX
		|| (a->elements != रिक्त
		    && a->elements[0] != रिक्त && a->elements[1] == रिक्त),
		file, पङ्क्तिः);

}
