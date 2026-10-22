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

#परिभाषय ASM_GLOBAL_DIRECTIVE .globl


#परिभाषय C_चिह्नम्_NAME(name) name
/* Macro for a label.  */
#यदि_परिभाषितम्	__STDC__
#परिभाषय C_LABEL(name)		name##:
#अन्यथा
#परिभाषय C_LABEL(name)		name/**/:
#यदि_अन्तः

/* This seems to always be the case on PPC.  */
#परिभाषय ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#परिभाषय ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#परिभाषय ASM_आकार_DIRECTIVE(name) .आकार name,.-name

/* If compiled for profiling, call `_mcount' at the start of each function.  */
#यदि_परिभाषितम्	PROF
/* The mcount code relies on the return address being on the stack
   to locate our caller and so it can restore it; so store one just
   for its benefit.  */
#यदि_परिभाषितम् PIC
#परिभाषय CALL_MCOUNT							      \
  .pushsection;								      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.दीर्घ 0;								      \
  .previous;								      \
  mflr  %r0;								      \
  stw   %r0,4(%r1);							      \
  bl    GLOBAL_विस्थापनम्_सारणी@local-4;					      \
  mflr  %r11;								      \
  lwz   %r0,0b@got(%r11);						      \
  bl    JUMPTARGET(_mcount);
#अन्यथा  /* PIC */
#परिभाषय CALL_MCOUNT							      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.दीर्घ 0;								      \
  .previous;								      \
  mflr  %r0;								      \
  lis   %r11,0b@ha;							      \
  stw   %r0,4(%r1);							      \
  addi  %r0,%r11,0b@l;							      \
  bl    JUMPTARGET(_mcount);
#यदि_अन्तः /* PIC */
#अन्यथा  /* PROF */
#परिभाषय CALL_MCOUNT		/* Do nothing.  */
#यदि_अन्तः /* PROF */

#परिभाषय	प्रविष्टिः_2(name)							      \
  ASM_GLOBAL_DIRECTIVE C_चिह्नम्_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_चिह्नम्_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT

#परिभाषय EALIGN_W_0  /* No words to insert.  */
#परिभाषय EALIGN_W_1  nop
#परिभाषय EALIGN_W_2  nop;nop
#परिभाषय EALIGN_W_3  nop;nop;nop
#परिभाषय EALIGN_W_4  EALIGN_W_3;nop
#परिभाषय EALIGN_W_5  EALIGN_W_4;nop
#परिभाषय EALIGN_W_6  EALIGN_W_5;nop
#परिभाषय EALIGN_W_7  EALIGN_W_6;nop

/* EALIGN is like ENTRY, but does alignment to 'words'*4 bytes
   past a 2^align boundary.  */
#यदि_परिभाषितम् PROF
#परिभाषय EFFI_ALIGN(name, alignt, शब्दाः)					      \
  ASM_GLOBAL_DIRECTIVE C_चिह्नम्_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_चिह्नम्_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT								      \
  b 0f;									      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  0:
#अन्यथा /* PROF */
#परिभाषय EFFI_ALIGN(name, alignt, शब्दाः)					      \
  ASM_GLOBAL_DIRECTIVE C_चिह्नम्_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_चिह्नम्_NAME(name),@function)			      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  C_LABEL(name)
#यदि_अन्तः

#परिभाषय अन्तः_2(name)							      \
  ASM_आकार_DIRECTIVE(name)

#यदि_परिभाषितम् PIC
#परिभाषय JUMPTARGET(name) name##@plt
#अन्यथा
#परिभाषय JUMPTARGET(name) name
#यदि_अन्तः

/* Local labels stripped out by the linker.  */
#परिभाषय L(x) .L##x
