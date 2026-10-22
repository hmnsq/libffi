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

#నిర్వచించు ASM_GLOBAL_DIRECTIVE .globl


#నిర్వచించు C_చిహ్నం_NAME(name) name
/* Macro for a label.  */
#నిర్వచితమైతే	__STDC__
#నిర్వచించు C_LABEL(name)		name##:
#లేకపోతే
#నిర్వచించు C_LABEL(name)		name/**/:
#అయితే_ముగింపు

/* This seems to always be the case on PPC.  */
#నిర్వచించు ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#నిర్వచించు ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#నిర్వచించు ASM_కొలత_DIRECTIVE(name) .కొలత name,.-name

/* If compiled for profiling, call `_mcount' at the start of each function.  */
#నిర్వచితమైతే	PROF
/* The mcount code relies on the return address being on the stack
   to locate our caller and so it can restore it; so store one just
   for its benefit.  */
#నిర్వచితమైతే PIC
#నిర్వచించు CALL_MCOUNT							      \
  .pushsection;								      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.దీర్ఘ 0;								      \
  .మునుపటి;								      \
  mflr  %r0;								      \
  stw   %r0,4(%r1);							      \
  bl    GLOBAL_స్థానభ్రంశం_పట్టిక@local-4;					      \
  mflr  %r11;								      \
  lwz   %r0,0b@got(%r11);						      \
  bl    JUMPTARGET(_mcount);
#లేకపోతే  /* PIC */
#నిర్వచించు CALL_MCOUNT							      \
  .section ".data";							      \
  .align ALIGNARG(2);							      \
0:.దీర్ఘ 0;								      \
  .మునుపటి;								      \
  mflr  %r0;								      \
  lis   %r11,0b@ha;							      \
  stw   %r0,4(%r1);							      \
  addi  %r0,%r11,0b@l;							      \
  bl    JUMPTARGET(_mcount);
#అయితే_ముగింపు /* PIC */
#లేకపోతే  /* PROF */
#నిర్వచించు CALL_MCOUNT		/* Do nothing.  */
#అయితే_ముగింపు /* PROF */

#నిర్వచించు	నమోదు_3(name)							      \
  ASM_GLOBAL_DIRECTIVE C_చిహ్నం_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_చిహ్నం_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT

#నిర్వచించు EALIGN_W_0  /* No words to insert.  */
#నిర్వచించు EALIGN_W_1  nop
#నిర్వచించు EALIGN_W_2  nop;nop
#నిర్వచించు EALIGN_W_3  nop;nop;nop
#నిర్వచించు EALIGN_W_4  EALIGN_W_3;nop
#నిర్వచించు EALIGN_W_5  EALIGN_W_4;nop
#నిర్వచించు EALIGN_W_6  EALIGN_W_5;nop
#నిర్వచించు EALIGN_W_7  EALIGN_W_6;nop

/* EALIGN is like ENTRY, but does alignment to 'words'*4 bytes
   past a 2^align boundary.  */
#నిర్వచితమైతే PROF
#నిర్వచించు EFFI_ALIGN(name, alignt, పదాలు)					      \
  ASM_GLOBAL_DIRECTIVE C_చిహ్నం_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_చిహ్నం_NAME(name),@function)			      \
  .align ALIGNARG(2);							      \
  C_LABEL(name)								      \
  CALL_MCOUNT								      \
  b 0f;									      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  0:
#లేకపోతే /* PROF */
#నిర్వచించు EFFI_ALIGN(name, alignt, పదాలు)					      \
  ASM_GLOBAL_DIRECTIVE C_చిహ్నం_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_చిహ్నం_NAME(name),@function)			      \
  .align ALIGNARG(alignt);						      \
  EALIGN_W_##words;							      \
  C_LABEL(name)
#అయితే_ముగింపు

#నిర్వచించు చివర_2(name)							      \
  ASM_కొలత_DIRECTIVE(name)

#నిర్వచితమైతే PIC
#నిర్వచించు JUMPTARGET(name) name##@plt
#లేకపోతే
#నిర్వచించు JUMPTARGET(name) name
#అయితే_ముగింపు

/* Local labels stripped out by the linker.  */
#నిర్వచించు L(x) .L##x
