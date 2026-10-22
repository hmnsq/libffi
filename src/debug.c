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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdio.h>

/* General debugging routines */

శూన్య ffi_stop_ఇక్కడ(శూన్య)
{
  /* This function is only useful for debugging purposes.
     Place a breakpoint on ffi_stop_here to be notified of
     significant events. */
}

/* This function should only be called via the FFI_ASSERT() macro */

NORETURN శూన్య ffi_assert(స్థిరరాశి అక్షరం *సమాసం, స్థిరరాశి అక్షరం *file, పూర్ణాంకం పంక్తి)
{
  fprintf(stderr, "ASSERTION FAILURE: %s at %s:%d\n", సమాసం, file, పంక్తి);
  ffi_stop_ఇక్కడ();
  abort();
}

/* Perform a sanity check on an ffi_type structure */

శూన్య ffi_type_test(ffi_type *a, స్థిరరాశి అక్షరం *file, పూర్ణాంకం పంక్తి)
{
  FFI_ASSERT_AT(a != ఖాళీ, file, పంక్తి);

  FFI_ASSERT_AT(a->type <= FFI_TYPE_చివరి, file, పంక్తి);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->కొలత > 0, file, పంక్తి);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->alignment > 0, file, పంక్తి);
  FFI_ASSERT_AT((a->type != FFI_TYPE_STRUCT && a->type != FFI_TYPE_COMPLEX
		 && a->type != FFI_TYPE_సదిశ)
		|| a->elements != ఖాళీ, file, పంక్తి);
  FFI_ASSERT_AT(a->type != FFI_TYPE_COMPLEX
		|| (a->elements != ఖాళీ
		    && a->elements[0] != ఖాళీ && a->elements[1] == ఖాళీ),
		file, పంక్తి);

}
