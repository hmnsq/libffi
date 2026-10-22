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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

/* General debugging routines */

ಶೂನ್ಯ ffi_stop_ಇಲ್ಲಿ(ಶೂನ್ಯ)
{
  /* This function is only useful for debugging purposes.
     Place a breakpoint on ffi_stop_here to be notified of
     significant events. */
}

/* This function should only be called via the FFI_ASSERT() macro */

NORETURN ಶೂನ್ಯ ffi_assert(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಅಭಿವ್ಯಕ್ತಿ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file, ಪೂರ್ಣಾಂಕ ಸಾಲು)
{
  fprintf(stderr, "ASSERTION FAILURE: %s at %s:%d\n", ಅಭಿವ್ಯಕ್ತಿ, file, ಸಾಲು);
  ffi_stop_ಇಲ್ಲಿ();
  abort();
}

/* Perform a sanity check on an ffi_type structure */

ಶೂನ್ಯ ffi_type_test(ffi_type *a, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file, ಪೂರ್ಣಾಂಕ ಸಾಲು)
{
  FFI_ASSERT_AT(a != ಖಾಲಿ, file, ಸಾಲು);

  FFI_ASSERT_AT(a->type <= FFI_TYPE_ಕೊನೆಯ, file, ಸಾಲು);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->ಗಾತ್ರ_2 > 0, file, ಸಾಲು);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->alignment > 0, file, ಸಾಲು);
  FFI_ASSERT_AT((a->type != FFI_TYPE_STRUCT && a->type != FFI_TYPE_COMPLEX
		 && a->type != FFI_TYPE_ಸದಿಶ)
		|| a->elements != ಖಾಲಿ, file, ಸಾಲು);
  FFI_ASSERT_AT(a->type != FFI_TYPE_COMPLEX
		|| (a->elements != ಖಾಲಿ
		    && a->elements[0] != ಖಾಲಿ && a->elements[1] == ಖಾಲಿ),
		file, ಸಾಲು);

}
