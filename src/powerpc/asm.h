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

#وصف_ڪريو ASM_GLOBAL_DIRECTIVE .globl


#وصف_ڪريو C_نشان_NAME(name) name
/* Macro for a label.  */
#جيڪڏهن_وصف_ٿيل	__STDC__
#وصف_ڪريو C_LABEL(name)		name##:
#وإلا
#وصف_ڪريو C_LABEL(name)		name/**/:
#جيڪڏهن_پورو

/* This seems to always be the case on PPC.  */
#وصف_ڪريو ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#وصف_ڪريو ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#وصف_ڪريو ASM_سائيز_DIRECTIVE(name) .سائيز name,.-name

/* If compiled for profiling, call `_mcount' at the start of each function.  */
#جيڪڏهن_وصف_ٿيل	PROF
/* The mcount code relies on the return address being on the stack
   to locate our caller and so it can restore it; so store one just
   for its benefit.  */
#جيڪڏهن_وصف_ٿيل PIC
#وصف_ڪريو CALL_MCOUNT							      \
  .pushsection;								      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.ڊگهو 0;								      \
  .اڳيون;								      \
  mflr  %r0;								      \
  stw   %r0,4(%r1);							      \
  bl    GLOBAL_وٿي_جدول@local-4;					      \
  mflr  %r11;								      \
  lwz   %r0,0b@got(%r11);						      \
  bl    JUMPTARGET(_mcount);
#وإلا  /* PIC */
#وصف_ڪريو CALL_MCOUNT							      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.ڊگهو 0;								      \
  .اڳيون;								      \
  mflr  %r0;								      \
  lis   %r11,0b@ha;							      \
  stw   %r0,4(%r1);							      \
  addi  %r0,%r11,0b@l;							      \
  bl    JUMPTARGET(_mcount);
#جيڪڏهن_پورو /* PIC */
#وإلا  /* PROF */
#وصف_ڪريو CALL_MCOUNT		/* Do nothing.  */
#جيڪڏهن_پورو /* PROF */

#وصف_ڪريو	داخلا_2(name)							      \
  ASM_GLOBAL_DIRECTIVE C_نشان_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_نشان_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT

#وصف_ڪريو EALIGN_W_0  /* No words to insert.  */
#وصف_ڪريو EALIGN_W_1  nop
#وصف_ڪريو EALIGN_W_2  nop;nop
#وصف_ڪريو EALIGN_W_3  nop;nop;nop
#وصف_ڪريو EALIGN_W_4  EALIGN_W_3;nop
#وصف_ڪريو EALIGN_W_5  EALIGN_W_4;nop
#وصف_ڪريو EALIGN_W_6  EALIGN_W_5;nop
#وصف_ڪريو EALIGN_W_7  EALIGN_W_6;nop

/* EALIGN is like ENTRY, but does alignment to 'words'*4 bytes
   past a 2^align boundary.  */
#جيڪڏهن_وصف_ٿيل PROF
#وصف_ڪريو EFFI_ALIGN(name, alignt, لفظ)					      \
  ASM_GLOBAL_DIRECTIVE C_نشان_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_نشان_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT								      \
  b 0f;									      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  0:
#وإلا /* PROF */
#وصف_ڪريو EFFI_ALIGN(name, alignt, لفظ)					      \
  ASM_GLOBAL_DIRECTIVE C_نشان_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_نشان_NAME(name),@function)			      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  C_LABEL(name)
#جيڪڏهن_پورو

#وصف_ڪريو پڇاڙي_2(name)							      \
  ASM_سائيز_DIRECTIVE(name)

#جيڪڏهن_وصف_ٿيل PIC
#وصف_ڪريو JUMPTARGET(name) name##@plt
#وإلا
#وصف_ڪريو JUMPTARGET(name) name
#جيڪڏهن_پورو

/* Local labels stripped out by the linker.  */
#وصف_ڪريو L(x) .L##x
