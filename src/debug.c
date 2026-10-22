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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdio.h>

/* General debugging routines */

શૂન્ય ffi_stop_here(શૂન્ય)
{
  /* This function is only useful for debugging purposes.
     Place a breakpoint on ffi_stop_here to be notified of
     significant events. */
}

/* This function should only be called via the FFI_ASSERT() macro */

NORETURN શૂન્ય ffi_assert(અચલ અક્ષર *પદાવલિ, અચલ અક્ષર *file, પૂર્ણાંક લીટી)
{
  fprintf(stderr, "ASSERTION FAILURE: %s at %s:%d\n", પદાવલિ, file, લીટી);
  ffi_stop_here();
  abort();
}

/* Perform a sanity check on an ffi_type structure */

શૂન્ય ffi_type_test(ffi_type *a, અચલ અક્ષર *file, પૂર્ણાંક લીટી)
{
  FFI_ASSERT_AT(a != ખાલી, file, લીટી);

  FFI_ASSERT_AT(a->type <= FFI_TYPE_LAST, file, લીટી);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->size > 0, file, લીટી);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->alignment > 0, file, લીટી);
  FFI_ASSERT_AT((a->type != FFI_TYPE_STRUCT && a->type != FFI_TYPE_COMPLEX
		 && a->type != FFI_TYPE_VECTOR)
		|| a->elements != ખાલી, file, લીટી);
  FFI_ASSERT_AT(a->type != FFI_TYPE_COMPLEX
		|| (a->elements != ખાલી
		    && a->elements[0] != ખાલી && a->elements[1] == ખાલી),
		file, લીટી);

}
