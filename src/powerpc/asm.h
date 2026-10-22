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

#ವ್ಯಾಖ್ಯಾನಿಸು ASM_GLOBAL_DIRECTIVE .globl


#ವ್ಯಾಖ್ಯಾನಿಸು C_ಚಿಹ್ನೆ_NAME(name) name
/* Macro for a label.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ	__STDC__
#ವ್ಯಾಖ್ಯಾನಿಸು C_LABEL(name)		name##:
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು C_LABEL(name)		name/**/:
#ಆದರೆ_ಅಂತ್ಯ

/* This seems to always be the case on PPC.  */
#ವ್ಯಾಖ್ಯಾನಿಸು ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#ವ್ಯಾಖ್ಯಾನಿಸು ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#ವ್ಯಾಖ್ಯಾನಿಸು ASM_ಗಾತ್ರ_DIRECTIVE(name) .ಗಾತ್ರ_2 name,.-name

/* If compiled for profiling, call `_mcount' at the start of each function.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ	PROF
/* The mcount code relies on the return address being on the stack
   to locate our caller and so it can restore it; so store one just
   for its benefit.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PIC
#ವ್ಯಾಖ್ಯಾನಿಸು CALL_MCOUNT							      \
  .pushsection;								      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.ದೀರ್ಘ 0;								      \
  .ಹಿಂದಿನ;								      \
  mflr  %r0;								      \
  stw   %r0,4(%r1);							      \
  bl    GLOBAL_ಸ್ಥಾನಾಂತರ_ಕೋಷ್ಟಕ@local-4;					      \
  mflr  %r11;								      \
  lwz   %r0,0b@got(%r11);						      \
  bl    JUMPTARGET(_mcount);
#ಇಲ್ಲದಿದ್ದರೆ  /* PIC */
#ವ್ಯಾಖ್ಯಾನಿಸು CALL_MCOUNT							      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.ದೀರ್ಘ 0;								      \
  .ಹಿಂದಿನ;								      \
  mflr  %r0;								      \
  lis   %r11,0b@ha;							      \
  stw   %r0,4(%r1);							      \
  addi  %r0,%r11,0b@l;							      \
  bl    JUMPTARGET(_mcount);
#ಆದರೆ_ಅಂತ್ಯ /* PIC */
#ಇಲ್ಲದಿದ್ದರೆ  /* PROF */
#ವ್ಯಾಖ್ಯಾನಿಸು CALL_MCOUNT		/* Do nothing.  */
#ಆದರೆ_ಅಂತ್ಯ /* PROF */

#ವ್ಯಾಖ್ಯಾನಿಸು	ನಮೂದು_2(name)							      \
  ASM_GLOBAL_DIRECTIVE C_ಚಿಹ್ನೆ_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_ಚಿಹ್ನೆ_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT

#ವ್ಯಾಖ್ಯಾನಿಸು EALIGN_W_0  /* No words to insert.  */
#ವ್ಯಾಖ್ಯಾನಿಸು EALIGN_W_1  nop
#ವ್ಯಾಖ್ಯಾನಿಸು EALIGN_W_2  nop;nop
#ವ್ಯಾಖ್ಯಾನಿಸು EALIGN_W_3  nop;nop;nop
#ವ್ಯಾಖ್ಯಾನಿಸು EALIGN_W_4  EALIGN_W_3;nop
#ವ್ಯಾಖ್ಯಾನಿಸು EALIGN_W_5  EALIGN_W_4;nop
#ವ್ಯಾಖ್ಯಾನಿಸು EALIGN_W_6  EALIGN_W_5;nop
#ವ್ಯಾಖ್ಯಾನಿಸು EALIGN_W_7  EALIGN_W_6;nop

/* EALIGN is like ENTRY, but does alignment to 'words'*4 bytes
   past a 2^align boundary.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PROF
#ವ್ಯಾಖ್ಯಾನಿಸು EFFI_ALIGN(name, alignt, ಪದಗಳು)					      \
  ASM_GLOBAL_DIRECTIVE C_ಚಿಹ್ನೆ_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_ಚಿಹ್ನೆ_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT								      \
  b 0f;									      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  0:
#ಇಲ್ಲದಿದ್ದರೆ /* PROF */
#ವ್ಯಾಖ್ಯಾನಿಸು EFFI_ALIGN(name, alignt, ಪದಗಳು)					      \
  ASM_GLOBAL_DIRECTIVE C_ಚಿಹ್ನೆ_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_ಚಿಹ್ನೆ_NAME(name),@function)			      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  C_LABEL(name)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು ಕೊನೆ_2(name)							      \
  ASM_ಗಾತ್ರ_DIRECTIVE(name)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PIC
#ವ್ಯಾಖ್ಯಾನಿಸು JUMPTARGET(name) name##@plt
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು JUMPTARGET(name) name
#ಆದರೆ_ಅಂತ್ಯ

/* Local labels stripped out by the linker.  */
#ವ್ಯಾಖ್ಯಾನಿಸು L(x) .L##x
