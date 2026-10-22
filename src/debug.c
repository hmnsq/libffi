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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>

/* General debugging routines */

ଶୂନ୍ୟ ffi_stop_here(ଶୂନ୍ୟ)
{
  /* This function is only useful for debugging purposes.
     Place a breakpoint on ffi_stop_here to be notified of
     significant events. */
}

/* This function should only be called via the FFI_ASSERT() macro */

NORETURN ଶୂନ୍ୟ ffi_assert(ଅଚଳ ଅକ୍ଷର *ପଦସମଷ୍ଟି, ଅଚଳ ଅକ୍ଷର *file, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି)
{
  fprintf(stderr, "ASSERTION FAILURE: %s at %s:%d\n", ପଦସମଷ୍ଟି, file, ଧାଡ଼ି);
  ffi_stop_here();
  abort();
}

/* Perform a sanity check on an ffi_type structure */

ଶୂନ୍ୟ ffi_type_test(ffi_type *a, ଅଚଳ ଅକ୍ଷର *file, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି)
{
  FFI_ASSERT_AT(a != ଖାଲି, file, ଧାଡ଼ି);

  FFI_ASSERT_AT(a->type <= FFI_TYPE_LAST, file, ଧାଡ଼ି);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->size > 0, file, ଧାଡ଼ି);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->alignment > 0, file, ଧାଡ଼ି);
  FFI_ASSERT_AT((a->type != FFI_TYPE_STRUCT && a->type != FFI_TYPE_COMPLEX
		 && a->type != FFI_TYPE_VECTOR)
		|| a->elements != ଖାଲି, file, ଧାଡ଼ି);
  FFI_ASSERT_AT(a->type != FFI_TYPE_COMPLEX
		|| (a->elements != ଖାଲି
		    && a->elements[0] != ଖାଲି && a->elements[1] == ଖାଲି),
		file, ଧାଡ଼ି);

}
