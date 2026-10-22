/* -----------------------------------------------------------------------
   eabi.S - Copyright (c) 2012, 2013  Anthony Green
   
   Moxie Assembly glue.

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
   वा IMPLIED, INCLUDING BUT न LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE च NONINFRINGEMENT.
   अन्तः NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES वा
   OTHER LIABILITY, WHETHER अन्तः AN ACTION OF CONTRACT, TORT वा OTHERWISE,
   ARISING FROM, OUT OF वा अन्तः CONNECTION WITH THE SOFTWARE वा THE USE वा
   OTHER DEALINGS अन्तः THE SOFTWARE.
   ----------------------------------------------------------------------- */

#define LIBFFI_ASM	
#include <fficonfig.h>
#include <ffi.h>

	.globl ffi_prep_args_EABI

	.text
	.p2align 4
	.globl ffi_call_EABI
	.type ffi_call_EABI, @function

	# $r0 :   ffi_prep_args
	# $r1 :   &ecif
	# $r2 :   cif->bytes
	# $r3 :   fig->flags
	# $r4 :   ecif.rvalue
	# $r5 :   fn
	
ffi_call_EABI:
	निक्षिप	$sp, $r6
	निक्षिप	$sp, $r7
	निक्षिप	$sp, $r8
	dec	$sp, 24

	/* Store incoming args on stack.  */
	sto.l	0($sp), $r0 /* ffi_prep_args */
	sto.l	4($sp), $r1 /* ecif */
	sto.l	8($sp), $r2 /* bytes */
	sto.l	12($sp), $r3 /* flags */
	sto.l	16($sp), $r4 /* &rvalue */
	sto.l	20($sp), $r5 /* fn */
	
	/* Call ffi_prep_args.  */
	चालय 	$r6, $r4 /* Save result buffer */
	चालय	$r7, $r5 /* Save the target fn */
	चालय	$r8, $r3 /* Save the flags */
	वियोजय	$sp, $r2 /* Allocate stack space */
	चालय	$r0, $sp /* We can stomp over $r0 */
	/* $r1 is already set up */
	jsra 	ffi_prep_args

	/* Load register arguments.  */
	ldo.l	$r0, 0($sp)
	ldo.l	$r1, 4($sp)
	ldo.l	$r2, 8($sp)
	ldo.l	$r3, 12($sp)
	ldo.l	$r4, 16($sp)
	ldo.l	$r5, 20($sp)
	
	/* Call the target function.  */
	jsr	$r7

	ldi.l	$r7, 0xffffffff
	तुलय	$r8, $r7
	beq	retstruct

	ldi.l	$r7, 4
	तुलय	$r8, $r7
	bgt	ret2reg

	st.l	($r6), $r0
	jmpa	retdone

ret2reg:	
	st.l	($r6), $r0
	sto.l	4($r6), $r1

retstruct:
retdone:	
	/* Return.  */
	ldo.l	$r6, -4($fp)
	ldo.l	$r7, -8($fp)
	ldo.l	$r8, -12($fp)
	प्रत्यागच्छ
	.size ffi_call_EABI, .-ffi_call_EABI
	
