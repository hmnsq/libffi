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

#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश <stdlib.h>
#समावेश <stdio.h>

/* General debugging routines */

व्योम ffi_stop_here(व्योम)
{
  /* This function is only useful for debugging purposes.
     Place a breakpoint on ffi_stop_here to be notified of
     significant events. */
}

/* This function should only be called via the FFI_ASSERT() macro */

NORETURN व्योम ffi_assert(स्थिर अक्षर *व्यंजक, स्थिर अक्षर *file, पूर्णांक पंक्ति_2)
{
  ख_लिखो(मानक_त्रुटि, "ASSERTION FAILURE: %s at %s:%d\n", व्यंजक, file, पंक्ति_2);
  ffi_stop_here();
  पात();
}

/* Perform a sanity check on an ffi_type structure */

व्योम ffi_type_test(ffi_type *a, स्थिर अक्षर *file, पूर्णांक पंक्ति_2)
{
  FFI_ASSERT_AT(a != शून्य, file, पंक्ति_2);

  FFI_ASSERT_AT(a->type <= FFI_TYPE_LAST, file, पंक्ति_2);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->आकार > 0, file, पंक्ति_2);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->alignment > 0, file, पंक्ति_2);
  FFI_ASSERT_AT((a->type != FFI_TYPE_STRUCT && a->type != FFI_TYPE_COMPLEX
		 && a->type != FFI_TYPE_VECTOR)
		|| a->elements != शून्य, file, पंक्ति_2);
  FFI_ASSERT_AT(a->type != FFI_TYPE_COMPLEX
		|| (a->elements != शून्य
		    && a->elements[0] != शून्य && a->elements[1] == शून्य),
		file, पंक्ति_2);

}
