/*
 * Copyright (c) 2013 Miodrag Vallat.  <miod@openbsd.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * m88k Foreign Function Interface
 */

#که_نه_تعریف_شوی LIBFFI_موخه_H
#تعریف_کړه LIBFFI_موخه_H

#که_نه_تعریف_شوی LIBFFI_ASM
تعريفنوع بینښه اوږد ffi_arg;
تعريفنوع نښهدار اوږد ffi_sarg;

تعريفنوع تعداد ffi_abi {
  FFI_لومړی_ABI = 0,
  FFI_OBSD,
  FFI_DEFAULT_ABI = FFI_OBSD,
  FFI_وروستی_ABI = FFI_DEFAULT_ABI + 1
} ffi_abi;
#د_که_پای

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کړه FFI_CLOSURES 1
#تعریف_کړه FFI_TRAMPOLINE_اندازه 0x14
#تعریف_کړه FFI_NATIVE_RAW_API 0

#د_که_پای
