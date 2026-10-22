/* -----------------------------------------------------------------------
   asm.h - Copyright (c) 1998 Geoffrey Keating

   PowerPC Assembly glue.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#تعریف_کرو ASM_GLOBAL_DIRECTIVE .globl


#تعریف_کرو C_علامت_NAME(name) name
/* Macro for a label.  */
#جے_تعریف_شدہ	__STDC__
#تعریف_کرو C_LABEL(name)		name##:
#وإلا
#تعریف_کرو C_LABEL(name)		name/**/:
#جے_انت

/* This seems to always be the case on PPC.  */
#تعریف_کرو ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#تعریف_کرو ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#تعریف_کرو ASM_ناپ_DIRECTIVE(name) .ناپ name,.-name

/* If compiled for profiling, call `_mcount' at the start of each function.  */
#جے_تعریف_شدہ	PROF
/* The mcount code relies on the return address being on the stack
   to locate our caller and so it can restore it; so store one just
   for its benefit.  */
#جے_تعریف_شدہ PIC
#تعریف_کرو CALL_MCOUNT							      \
  .pushsection;								      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.لما 0;								      \
  .پچھلا;								      \
  mflr  %r0;								      \
  stw   %r0,4(%r1);							      \
  bl    GLOBAL_وتھ_جدول@local-4;					      \
  mflr  %r11;								      \
  lwz   %r0,0b@got(%r11);						      \
  bl    JUMPTARGET(_mcount);
#وإلا  /* PIC */
#تعریف_کرو CALL_MCOUNT							      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.لما 0;								      \
  .پچھلا;								      \
  mflr  %r0;								      \
  lis   %r11,0b@ha;							      \
  stw   %r0,4(%r1);							      \
  addi  %r0,%r11,0b@l;							      \
  bl    JUMPTARGET(_mcount);
#جے_انت /* PIC */
#وإلا  /* PROF */
#تعریف_کرو CALL_MCOUNT		/* Do nothing.  */
#جے_انت /* PROF */

#تعریف_کرو	اندراج_2(name)							      \
  ASM_GLOBAL_DIRECTIVE C_علامت_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_علامت_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT

#تعریف_کرو EALIGN_W_0  /* No words to insert.  */
#تعریف_کرو EALIGN_W_1  nop
#تعریف_کرو EALIGN_W_2  nop;nop
#تعریف_کرو EALIGN_W_3  nop;nop;nop
#تعریف_کرو EALIGN_W_4  EALIGN_W_3;nop
#تعریف_کرو EALIGN_W_5  EALIGN_W_4;nop
#تعریف_کرو EALIGN_W_6  EALIGN_W_5;nop
#تعریف_کرو EALIGN_W_7  EALIGN_W_6;nop

/* EALIGN is like ENTRY, but does alignment to 'words'*4 bytes
   past a 2^align boundary.  */
#جے_تعریف_شدہ PROF
#تعریف_کرو EFFI_ALIGN(name, alignt, الفاظ)					      \
  ASM_GLOBAL_DIRECTIVE C_علامت_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_علامت_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT								      \
  b 0f;									      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  0:
#وإلا /* PROF */
#تعریف_کرو EFFI_ALIGN(name, alignt, الفاظ)					      \
  ASM_GLOBAL_DIRECTIVE C_علامت_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_علامت_NAME(name),@function)			      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  C_LABEL(name)
#جے_انت

#تعریف_کرو سرا_2(name)							      \
  ASM_ناپ_DIRECTIVE(name)

#جے_تعریف_شدہ PIC
#تعریف_کرو JUMPTARGET(name) name##@plt
#وإلا
#تعریف_کرو JUMPTARGET(name) name
#جے_انت

/* Local labels stripped out by the linker.  */
#تعریف_کرو L(x) .L##x
