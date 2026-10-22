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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdio.h>

/* General debugging routines */

ശൂന്യം ffi_stop_here(ശൂന്യം)
{
  /* This function is only useful for debugging purposes.
     Place a breakpoint on ffi_stop_here to be notified of
     significant events. */
}

/* This function should only be called via the FFI_ASSERT() macro */

NORETURN ശൂന്യം ffi_assert(സ്ഥിരം അക്ഷരം *വ്യഞ്ജകം, സ്ഥിരം അക്ഷരം *file, പൂർണ്ണസംഖ്യ വരി)
{
  fprintf(stderr, "ASSERTION FAILURE: %s at %s:%d\n", വ്യഞ്ജകം, file, വരി);
  ffi_stop_here();
  abort();
}

/* Perform a sanity check on an ffi_type structure */

ശൂന്യം ffi_type_test(ffi_type *a, സ്ഥിരം അക്ഷരം *file, പൂർണ്ണസംഖ്യ വരി)
{
  FFI_ASSERT_AT(a != ശൂന്യ, file, വരി);

  FFI_ASSERT_AT(a->type <= FFI_TYPE_LAST, file, വരി);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->size > 0, file, വരി);
  FFI_ASSERT_AT(a->type == FFI_TYPE_VOID || a->alignment > 0, file, വരി);
  FFI_ASSERT_AT((a->type != FFI_TYPE_STRUCT && a->type != FFI_TYPE_COMPLEX
		 && a->type != FFI_TYPE_VECTOR)
		|| a->elements != ശൂന്യ, file, വരി);
  FFI_ASSERT_AT(a->type != FFI_TYPE_COMPLEX
		|| (a->elements != ശൂന്യ
		    && a->elements[0] != ശൂന്യ && a->elements[1] == ശൂന്യ),
		file, വരി);

}
