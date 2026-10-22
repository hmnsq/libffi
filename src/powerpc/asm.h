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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASM_GLOBAL_DIRECTIVE .globl


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C_ਚਿੰਨ੍ਹ_NAME(name) name
/* Macro for a label.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ	__STDC__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C_LABEL(name)		name##:
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C_LABEL(name)		name/**/:
#ਜੇ_ਅੰਤ

/* This seems to always be the case on PPC.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASM_ਮਾਪ_DIRECTIVE(name) .ਮਾਪ name,.-name

/* If compiled for profiling, call `_mcount' at the start of each function.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ	PROF
/* The mcount code relies on the return address being on the stack
   to locate our caller and so it can restore it; so store one just
   for its benefit.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PIC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MCOUNT							      \
  .pushsection;								      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.ਲੰਮਾ 0;								      \
  .ਪਿਛਲਾ;								      \
  mflr  %r0;								      \
  stw   %r0,4(%r1);							      \
  bl    GLOBAL_ਵਿੱਥ_ਸਾਰਣੀ@local-4;					      \
  mflr  %r11;								      \
  lwz   %r0,0b@got(%r11);						      \
  bl    JUMPTARGET(_mcount);
#ਨਹੀਂਤਾਂ  /* PIC */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MCOUNT							      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.ਲੰਮਾ 0;								      \
  .ਪਿਛਲਾ;								      \
  mflr  %r0;								      \
  lis   %r11,0b@ha;							      \
  stw   %r0,4(%r1);							      \
  addi  %r0,%r11,0b@l;							      \
  bl    JUMPTARGET(_mcount);
#ਜੇ_ਅੰਤ /* PIC */
#ਨਹੀਂਤਾਂ  /* PROF */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MCOUNT		/* Do nothing.  */
#ਜੇ_ਅੰਤ /* PROF */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ	ਇੰਦਰਾਜ_2(name)							      \
  ASM_GLOBAL_DIRECTIVE C_ਚਿੰਨ੍ਹ_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_ਚਿੰਨ੍ਹ_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EALIGN_W_0  /* No words to insert.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EALIGN_W_1  nop
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EALIGN_W_2  nop;nop
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EALIGN_W_3  nop;nop;nop
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EALIGN_W_4  EALIGN_W_3;nop
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EALIGN_W_5  EALIGN_W_4;nop
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EALIGN_W_6  EALIGN_W_5;nop
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EALIGN_W_7  EALIGN_W_6;nop

/* EALIGN is like ENTRY, but does alignment to 'words'*4 bytes
   past a 2^align boundary.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PROF
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EFFI_ALIGN(name, alignt, ਸ਼ਬਦ)					      \
  ASM_GLOBAL_DIRECTIVE C_ਚਿੰਨ੍ਹ_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_ਚਿੰਨ੍ਹ_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT								      \
  b 0f;									      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  0:
#ਨਹੀਂਤਾਂ /* PROF */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EFFI_ALIGN(name, alignt, ਸ਼ਬਦ)					      \
  ASM_GLOBAL_DIRECTIVE C_ਚਿੰਨ੍ਹ_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_ਚਿੰਨ੍ਹ_NAME(name),@function)			      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  C_LABEL(name)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿਰਾ_2(name)							      \
  ASM_ਮਾਪ_DIRECTIVE(name)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PIC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JUMPTARGET(name) name##@plt
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JUMPTARGET(name) name
#ਜੇ_ਅੰਤ

/* Local labels stripped out by the linker.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L(x) .L##x
