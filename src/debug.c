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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdio.h>

/* General debugging routines */

শূন্য ffi_stop_here(শূন্য)
{
  /* This function is only useful for debugging purposes.
     Place a breakpoint on ffi_stop_here to be notified of
     significant events. */
}

/* This function should only be called via the FFI_ASSERT() macro */

NORETURN শূন্য ffi_assert(ধ্রুব বর্ণ *রাশিমালা, ধ্রুব বর্ণ *file, পূর্ণাঙ্ক সারি)
{
  fprintf(stderr, "ASSERTION FAILURE: %s at %s:%d\n", রাশিমালা, file, সারি);
  ffi_stop_here();
  abort();
}

/* Perform a sanity check on an ffi_type structure */

শূন্য ffi_type_test(ffi_type *a, ধ্রুব বর্ণ *file, পূর্ণাঙ্ক সারি)
{
  FFI_ASSERT_AT(a != রিক্ত, file, সারি);

  FFI_ASSERT_AT(a->type <= FFI_TYPE_LAST, file, সারি);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->size > 0, file, সারি);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->alignment > 0, file, সারি);
  FFI_ASSERT_AT((a->type != FFI_TYPE_STRUCT && a->type != FFI_TYPE_COMPLEX
		 && a->type != FFI_TYPE_VECTOR)
		|| a->elements != রিক্ত, file, সারি);
  FFI_ASSERT_AT(a->type != FFI_TYPE_COMPLEX
		|| (a->elements != রিক্ত
		    && a->elements[0] != রিক্ত && a->elements[1] == রিক্ত),
		file, সারি);

}
