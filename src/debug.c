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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdio.h>

/* General debugging routines */

வெறுமை ffi_stop_here(வெறுமை)
{
  /* This function is only useful for debugging purposes.
     Place a breakpoint on ffi_stop_here to be notified of
     significant events. */
}

/* This function should only be called via the FFI_ASSERT() macro */

NORETURN வெறுமை ffi_assert(மாறிலி எழுத்து *கோவை, மாறிலி எழுத்து *file, முழு வரி)
{
  fprintf(stderr, "ASSERTION FAILURE: %s at %s:%d\n", கோவை, file, வரி);
  ffi_stop_here();
  abort();
}

/* Perform a sanity check on an ffi_type structure */

வெறுமை ffi_type_test(ffi_type *a, மாறிலி எழுத்து *file, முழு வரி)
{
  FFI_ASSERT_AT(a != வெற்று, file, வரி);

  FFI_ASSERT_AT(a->type <= FFI_TYPE_LAST, file, வரி);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->size > 0, file, வரி);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->alignment > 0, file, வரி);
  FFI_ASSERT_AT((a->type != FFI_TYPE_STRUCT && a->type != FFI_TYPE_COMPLEX
		 && a->type != FFI_TYPE_VECTOR)
		|| a->elements != வெற்று, file, வரி);
  FFI_ASSERT_AT(a->type != FFI_TYPE_COMPLEX
		|| (a->elements != வெற்று
		    && a->elements[0] != வெற்று && a->elements[1] == வெற்று),
		file, வரி);

}
