/* -----------------------------------------------------------------------
   sysv.S - Copyright (c) 2002, 2003, 2004, 2006, 2008 Kaz Kojima
   
   SuperH Foreign Function Interface 

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
   EXPRESS वा IMPLIED, INCLUDING BUT न LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE च
   NONINFRINGEMENT.  अन्तः NO EVENT SHALL THE AUTHORS वा COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES वा OTHER LIABILITY,
   WHETHER अन्तः AN ACTION OF CONTRACT, TORT वा OTHERWISE, ARISING FROM,
   OUT OF वा अन्तः CONNECTION WITH THE SOFTWARE वा THE USE वा OTHER
   DEALINGS अन्तः THE SOFTWARE.
   ----------------------------------------------------------------------- */

#define LIBFFI_ASM	
#include <fficonfig.h>
#include <ffi.h>
#ifdef HAVE_MACHINE_ASM_H
#include <machine/asm.h>
#else
/* XXX these lose for some platforms, I'm sure. */
#define C2(X, Y)	X ## Y
#define C1(X, Y)	C2(X, Y)
#define CNAME(x) C1(__USER_LABEL_PREFIX__, x)
#define ENTRY(x) .globl CNAME(x); .type CNAME(x),%function; CNAME(x):
#endif

#if defined(__HITACHI__)
#define STRUCT_VALUE_ADDRESS_WITH_ARG 1
#else
#define STRUCT_VALUE_ADDRESS_WITH_ARG 0
#endif

.text

	# r4:	ffi_prep_args
	# r5:	&ecif
	# r6:	bytes
	# r7:	flags
	# sp+0: rvalue
	# sp+4: fn

	# This assumes we are using gas.
ENTRY(ffi_call_SYSV)
	# Save registers
.LFB1:
	चालय.l	r8,@-r15
.LCFI0:
	चालय.l	r9,@-r15
.LCFI1:
	चालय.l	r10,@-r15
.LCFI2:
	चालय.l	r12,@-r15
.LCFI3:
	चालय.l	r14,@-r15
.LCFI4:
	sts.l	pr,@-r15
.LCFI5:
	चालय	r15,r14
.LCFI6:
#if defined(__SH4__)
	चालय	r6,r8
	चालय	r7,r9

	वियोजय	r6,r15
	योजय	#-16,r15
	चालय	#~7,r0
	and	r0,r15

	चालय	r4,r0
	jsr	@r0
	 चालय	r15,r4

	चालय	r9,r1
	shlr8	r9
	shlr8	r9
	shlr8	r9

	चालय	#FFI_TYPE_STRUCT,r2
	तुलय/eq	r2,r9
	bf	1f
#if STRUCT_VALUE_ADDRESS_WITH_ARG
 	चालय.l	@r15+,r4
	bra	2f
	 चालय	#5,r2
#else
 	चालय.l	@r15+,r10
#endif
1:
	चालय	#4,r2
2:
	चालय	#4,r3

L_pass:
	तुलय/pl	r8
	bf	L_call_it

	चालय	r1,r0
	and	#3,r0

L_pass_d:
	तुलय/eq	#FFI_TYPE_DOUBLE,r0
	bf	L_pass_f

	चालय	r3,r0
	and	#1,r0
	tst	r0,r0
	bt	1f
	योजय	#1,r3
1:
	चालय	#12,r0
	तुलय/hs	r0,r3
	bt/s	3f
	 shlr2	r1
	bsr	L_pop_d
	 nop
3:
	योजय	#2,r3
	bra	L_pass
	 योजय	#-8,r8

L_pop_d:
	चालय	r3,r0
	योजय	r0,r0
	योजय	r3,r0
	योजय	#-12,r0
	braf	r0
	 nop
#ifdef __LITTLE_ENDIAN__
	fmov.s	@r15+,fr5
	rts
	 fmov.s	@r15+,fr4
	fmov.s	@r15+,fr7
	rts
	 fmov.s	@r15+,fr6
	fmov.s	@r15+,fr9
	rts
	 fmov.s	@r15+,fr8
	fmov.s	@r15+,fr11
	rts
	 fmov.s	@r15+,fr10
#else
	fmov.s	@r15+,fr4
	rts
	 fmov.s	@r15+,fr5
	fmov.s	@r15+,fr6
	rts
	 fmov.s	@r15+,fr7
	fmov.s	@r15+,fr8
	rts
	 fmov.s	@r15+,fr9
	fmov.s	@r15+,fr10
	rts
	 fmov.s	@r15+,fr11
#endif

L_pass_f:
	तुलय/eq	#FFI_TYPE_FLOAT,r0
	bf	L_pass_i

	चालय	#12,r0
	तुलय/hs	r0,r3
	bt/s	2f
	 shlr2	r1
	bsr	L_pop_f
	 nop
2:
	योजय	#1,r3
	bra	L_pass
	 योजय	#-4,r8

L_pop_f:
	चालय	r3,r0
	shll2	r0
	योजय	#-16,r0
	braf	r0
	 nop
#ifdef __LITTLE_ENDIAN__
	rts
	 fmov.s	@r15+,fr5
	rts
	 fmov.s	@r15+,fr4
	rts
	 fmov.s	@r15+,fr7
	rts
	 fmov.s	@r15+,fr6
	rts
	 fmov.s	@r15+,fr9
	rts
	 fmov.s	@r15+,fr8
	rts
	 fmov.s	@r15+,fr11
	rts
	 fmov.s	@r15+,fr10
#else
	rts
	 fmov.s	@r15+,fr4
	rts
	 fmov.s	@r15+,fr5
	rts
	 fmov.s	@r15+,fr6
	rts
	 fmov.s	@r15+,fr7
	rts
	 fmov.s	@r15+,fr8
	rts
	 fmov.s	@r15+,fr9
	rts
	 fmov.s	@r15+,fr10
	rts
	 fmov.s	@r15+,fr11
#endif

L_pass_i:
	तुलय/eq	#FFI_TYPE_INT,r0
	bf	L_call_it

	चालय	#8,r0
	तुलय/hs	r0,r2
	bt/s	2f
	 shlr2	r1
	bsr	L_pop_i
	 nop
2:
	योजय	#1,r2
	bra	L_pass
	 योजय	#-4,r8

L_pop_i:
	चालय	r2,r0
	shll2	r0
	योजय	#-16,r0
	braf	r0
	 nop
	rts
	 चालय.l	@r15+,r4
	rts
	 चालय.l	@r15+,r5
	rts
	 चालय.l	@r15+,r6
	rts
	 चालय.l	@r15+,r7

L_call_it:
	# आह्वान function
#if (! STRUCT_VALUE_ADDRESS_WITH_ARG)
	चालय	r10, r2
#endif
	चालय.l  @(28,r14),r1
	jsr    @r1
	 nop

L_ret_d:
	चालय	#FFI_TYPE_DOUBLE,r2
	तुलय/eq	r2,r9
	bf	L_ret_ll

	चालय.l	@(24,r14),r1
#ifdef __LITTLE_ENDIAN__
	fmov.s	fr1,@r1
	योजय	#4,r1
	bra	L_epilogue
	 fmov.s	fr0,@r1
#else
	fmov.s	fr0,@r1
	योजय	#4,r1
	bra	L_epilogue
	 fmov.s	fr1,@r1
#endif

L_ret_ll:
	चालय	#FFI_TYPE_SINT64,r2
	तुलय/eq	r2,r9
	bt/s	1f
	 चालय	#FFI_TYPE_UINT64,r2
	तुलय/eq	r2,r9
	bf	L_ret_f

1:
	चालय.l	@(24,r14),r2
	चालय.l	r0,@r2
	bra	L_epilogue
	 चालय.l	r1,@(4,r2)

L_ret_f:
	चालय	#FFI_TYPE_FLOAT,r2
	तुलय/eq	r2,r9
	bf	L_ret_i

	चालय.l	@(24,r14),r1
	bra	L_epilogue
	 fmov.s	fr0,@r1

L_ret_i:
	चालय	#FFI_TYPE_INT,r2
	तुलय/eq	r2,r9
	bf	L_epilogue

	चालय.l	@(24,r14),r1
	bra	L_epilogue
	 चालय.l	r0,@r1

L_epilogue:
	# Remove the space we pushed for the args
	चालय   r14,r15

	lds.l  @r15+,pr
	चालय.l  @r15+,r14
	चालय.l  @r15+,r12
	चालय.l  @r15+,r10
	चालय.l  @r15+,r9
	rts
	 चालय.l  @r15+,r8
#else
	चालय	r6,r8
	चालय	r7,r9

	वियोजय	r6,r15
	योजय	#-16,r15
	चालय	#~7,r0
	and	r0,r15

	चालय	r4,r0
	jsr	@r0
	 चालय	r15,r4

	चालय	r9,r3
	shlr8	r9
	shlr8	r9
	shlr8	r9

	चालय	#FFI_TYPE_STRUCT,r2
	तुलय/eq	r2,r9
	bf	1f
#if STRUCT_VALUE_ADDRESS_WITH_ARG
	चालय.l	@r15+,r4
	bra	2f
	 चालय	#5,r2
#else
	चालय.l	@r15+,r10
#endif
1:
	चालय	#4,r2
2:

L_pass:
	तुलय/pl	r8
	bf	L_call_it

	चालय	r3,r0
	and	#3,r0

L_pass_d:
	तुलय/eq	#FFI_TYPE_DOUBLE,r0
	bf	L_pass_i

	चालय	r15,r0
	and	#7,r0
	tst	r0,r0
	bt	1f
	योजय	#4,r15
1:
	चालय	#8,r0
	तुलय/hs	r0,r2
	bt/s	2f
	 shlr2	r3
	bsr	L_pop_d
	 nop
2:
	योजय	#2,r2
	bra	L_pass
	 योजय	#-8,r8

L_pop_d:
	चालय	r2,r0
	योजय	r0,r0
	योजय	r2,r0
	योजय	#-12,r0
	योजय	r0,r0
	braf	r0
	 nop
	चालय.l	@r15+,r4
	rts
	 चालय.l	@r15+,r5
	चालय.l	@r15+,r5
	rts
	 चालय.l	@r15+,r6
	चालय.l	@r15+,r6
	rts
	 चालय.l	@r15+,r7
	rts
	 चालय.l	@r15+,r7

L_pass_i:
	तुलय/eq	#FFI_TYPE_INT,r0
	bf	L_call_it

	चालय	#8,r0
	तुलय/hs	r0,r2
	bt/s	2f
	 shlr2	r3
	bsr	L_pop_i
	 nop
2:
	योजय	#1,r2
	bra	L_pass
	 योजय	#-4,r8

L_pop_i:
	चालय	r2,r0
	shll2	r0
	योजय	#-16,r0
	braf	r0
	 nop
	rts
	 चालय.l	@r15+,r4
	rts
	 चालय.l	@r15+,r5
	rts
	 चालय.l	@r15+,r6
	rts
	 चालय.l	@r15+,r7

L_call_it:
	# आह्वान function
#if (! STRUCT_VALUE_ADDRESS_WITH_ARG)
	चालय	r10, r2
#endif
	चालय.l  @(28,r14),r1
	jsr    @r1
	 nop

L_ret_d:
	चालय	#FFI_TYPE_DOUBLE,r2
	तुलय/eq	r2,r9
	bf	L_ret_ll

	चालय.l	@(24,r14),r2
	चालय.l	r0,@r2
	bra	L_epilogue
	 चालय.l	r1,@(4,r2)

L_ret_ll:
	चालय	#FFI_TYPE_SINT64,r2
	तुलय/eq	r2,r9
	bt/s	1f
	 चालय	#FFI_TYPE_UINT64,r2
	तुलय/eq	r2,r9
	bf	L_ret_i

1:
	चालय.l	@(24,r14),r2
	चालय.l	r0,@r2
	bra	L_epilogue
	 चालय.l	r1,@(4,r2)

L_ret_i:
	चालय	#FFI_TYPE_FLOAT,r2
	तुलय/eq	r2,r9
	bt	1f
	चालय	#FFI_TYPE_INT,r2
	तुलय/eq	r2,r9
	bf	L_epilogue
1:
	चालय.l	@(24,r14),r1
	bra	L_epilogue
	 चालय.l	r0,@r1

L_epilogue:
	# Remove the space we pushed for the args
	चालय   r14,r15

	lds.l  @r15+,pr
	चालय.l  @r15+,r14
	चालय.l  @r15+,r12
	चालय.l  @r15+,r10
	चालय.l  @r15+,r9
	rts
	 चालय.l  @r15+,r8
#endif
.LFE1:
.ffi_call_SYSV_end:
        .size    CNAME(ffi_call_SYSV),.ffi_call_SYSV_end-CNAME(ffi_call_SYSV)

.globl	ffi_closure_helper_SYSV

ENTRY(ffi_closure_SYSV)
.LFB2:
	चालय.l	r7,@-r15
.LCFI7:
	चालय.l	r6,@-r15
.LCFI8:
	चालय.l	r5,@-r15
.LCFI9:
	चालय.l	r4,@-r15
.LCFIA:
	चालय.l	r14,@-r15
.LCFIB:
	sts.l	pr,@-r15

	/* Stack layout:	
	   xx bytes (on stack parameters)
	   16 bytes (register parameters)
	    4 bytes (saved frame pointer)
	    4 bytes (saved return address)
	   32 bytes (floating register parameters, SH-4 only)
	    8 bytes (result)
	    4 bytes (pad)
	    4 bytes (5th arg)
	   <- new stack pointer
	*/
.LCFIC:
#if defined(__SH4__)
	योजय	#-48,r15
#else
	योजय	#-16,r15
#endif
.LCFID:
	चालय	r15,r14
.LCFIE:

#if defined(__SH4__)
	चालय	r14,r1
	योजय	#48,r1
#ifdef __LITTLE_ENDIAN__
	fmov.s	fr10,@-r1
	fmov.s	fr11,@-r1
	fmov.s	fr8,@-r1
	fmov.s	fr9,@-r1
	fmov.s	fr6,@-r1
	fmov.s	fr7,@-r1
	fmov.s	fr4,@-r1
	fmov.s	fr5,@-r1
#else
	fmov.s	fr11,@-r1
	fmov.s	fr10,@-r1
	fmov.s	fr9,@-r1
	fmov.s	fr8,@-r1
	fmov.s	fr7,@-r1
	fmov.s	fr6,@-r1
	fmov.s	fr5,@-r1
	fmov.s	fr4,@-r1
#endif
	चालय	r1,r7
	चालय	r14,r6
	योजय	#56,r6
#else
	चालय	r14,r6
	योजय	#24,r6
#endif

	bt/s	10f
	 चालय	r2, r5
	चालय	r14,r1
	योजय	#8,r1
	चालय	r1,r5
10:

	चालय	r14,r1
#if defined(__SH4__)
	योजय	#72,r1
#else
	योजय	#40,r1
#endif
	चालय.l	r1,@r14

#ifdef PIC
	चालय.l	L_got,r1
	mova	L_got,r0
	योजय	r0,r1
	चालय.l	L_helper,r0
	योजय	r1,r0
#else
	चालय.l	L_helper,r0
#endif
	jsr	@r0
	 चालय	r3,r4

	shll	r0
	चालय	r0,r1
	mova	L_table,r0
	योजय	r1,r0
	चालय.w	@r0,r0
	चालय	r14,r2
	braf	r0
	 योजय	#8,r2
0:
	.align 2
#ifdef PIC
L_got:
	.long	_GLOBAL_OFFSET_TABLE_
L_helper:
	.long	ffi_closure_helper_SYSV@GOTOFF
#else
L_helper:
	.long	ffi_closure_helper_SYSV
#endif
L_table:
	.short L_case_v - 0b	/* FFI_TYPE_VOID */
	.short L_case_i - 0b	/* FFI_TYPE_INT */
#if defined(__SH4__)
	.short L_case_f - 0b	/* FFI_TYPE_FLOAT */
	.short L_case_d - 0b	/* FFI_TYPE_DOUBLE */
	.short L_case_d - 0b	/* FFI_TYPE_LONGDOUBLE */
#else
	.short L_case_i - 0b	/* FFI_TYPE_FLOAT */
	.short L_case_ll - 0b	/* FFI_TYPE_DOUBLE */
	.short L_case_ll - 0b	/* FFI_TYPE_LONGDOUBLE */
#endif
	.short L_case_uq - 0b	/* FFI_TYPE_UINT8 */
	.short L_case_q - 0b	/* FFI_TYPE_SINT8 */
	.short L_case_uh - 0b	/* FFI_TYPE_UINT16 */
	.short L_case_h - 0b	/* FFI_TYPE_SINT16 */
	.short L_case_i - 0b	/* FFI_TYPE_UINT32 */
	.short L_case_i - 0b	/* FFI_TYPE_SINT32 */
	.short L_case_ll - 0b	/* FFI_TYPE_UINT64 */
	.short L_case_ll - 0b	/* FFI_TYPE_SINT64 */
	.short L_case_v - 0b	/* FFI_TYPE_STRUCT */
	.short L_case_i - 0b	/* FFI_TYPE_POINTER */

#if defined(__SH4__)
L_case_d:
#ifdef __LITTLE_ENDIAN__
	fmov.s	@r2+,fr1
	bra	L_case_v
	 fmov.s	@r2,fr0
#else
	fmov.s	@r2+,fr0
	bra	L_case_v
	 fmov.s	@r2,fr1
#endif

L_case_f:
	bra	L_case_v
	 fmov.s	@r2,fr0
#endif
	
L_case_ll:
	चालय.l	@r2+,r0
	bra	L_case_v
	 चालय.l	@r2,r1
	
L_case_i:
	bra	L_case_v
	 चालय.l	@r2,r0
	
L_case_q:
#ifdef __LITTLE_ENDIAN__
#else
	योजय	#3,r2
#endif
	bra	L_case_v
	 चालय.b	@r2,r0

L_case_uq:
#ifdef __LITTLE_ENDIAN__
#else
	योजय	#3,r2
#endif
	चालय.b	@r2,r0
	bra	L_case_v
	 extu.b r0,r0

L_case_h:
#ifdef __LITTLE_ENDIAN__
#else
	योजय	#2,r2
#endif
	bra	L_case_v
	 चालय.w	@r2,r0

L_case_uh:
#ifdef __LITTLE_ENDIAN__
#else
	योजय	#2,r2
#endif
	चालय.w	@r2,r0
	extu.w	r0,r0
	/* fall through */

L_case_v:
#if defined(__SH4__)
	योजय	#48,r15
#else
	योजय	#16,r15
#endif
	lds.l	@r15+,pr
	चालय.l	@r15+,r14
	rts
	 योजय	#16,r15
.LFE2:
.ffi_closure_SYSV_end:
        .size    CNAME(ffi_closure_SYSV),.ffi_closure_SYSV_end-CNAME(ffi_closure_SYSV)

#if defined __ELF__ && defined __linux__
	.खण्डः	.note.GNU-stack,"",@progbits
#endif

	.खण्डः	".eh_frame","aw",@progbits
__FRAME_BEGIN__:
	.4byte	.LECIE1-.LSCIE1	/* Length of Common Information Entry */
.LSCIE1:
	.4byte	0x0	/* CIE Identifier Tag */
	.byte	0x1	/* CIE Version */
#ifdef PIC
	.ascii "zR\0"	/* CIE Augmentation */
#else
	.byte	0x0	/* CIE Augmentation */
#endif
	.byte	0x1	/* uleb128 0x1; CIE Code Alignment Factor */
	.byte	0x7c	/* sleb128 -4; CIE Data Alignment Factor */
	.byte	0x11	/* CIE RA Column */
#ifdef PIC
	.uleb128 0x1	/* Augmentation size */
	.byte	0x10	/* FDE Encoding (pcrel) */
#endif
	.byte	0xc	/* DW_CFA_def_cfa */
	.byte	0xf	/* uleb128 0xf */
	.byte	0x0	/* uleb128 0x0 */
	.align	2
.LECIE1:
.LSFDE1:
	.4byte	.LEFDE1-.LASFDE1	/* FDE Length */
.LASFDE1:
	.4byte	.LASFDE1-__FRAME_BEGIN__	/* FDE CIE offset */
#ifdef PIC
	.4byte	.LFB1-.	/* FDE initial location */
#else
	.4byte	.LFB1	/* FDE initial location */
#endif
	.4byte	.LFE1-.LFB1	 /* FDE address range */
#ifdef PIC
	.uleb128 0x0	/* Augmentation size */
#endif
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFI0-.LFB1
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0x4	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFI1-.LCFI0
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0x8	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFI2-.LCFI1
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0xc	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFI3-.LCFI2
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0x10	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFI4-.LCFI3
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0x14	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFI5-.LCFI4
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0x18	/* uleb128 0x4 */
	.byte	0x91	/* DW_CFA_offset, column 0x11 */
	.byte	0x6	/* uleb128 0x6 */
	.byte	0x8e	/* DW_CFA_offset, column 0xe */
	.byte	0x5	/* uleb128 0x5 */
	.byte	0x8c	/* DW_CFA_offset, column 0xc */
	.byte	0x4	/* uleb128 0x4 */
	.byte	0x8a	/* DW_CFA_offset, column 0xa */
	.byte	0x3	/* uleb128 0x3 */
	.byte	0x89	/* DW_CFA_offset, column 0x9 */
	.byte	0x2	/* uleb128 0x2 */
	.byte	0x88	/* DW_CFA_offset, column 0x8 */
	.byte	0x1	/* uleb128 0x1 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFI6-.LCFI5
	.byte	0xd	/* DW_CFA_def_cfa_register */
	.byte	0xe	/* uleb128 0xe */
	.align	2
.LEFDE1:

.LSFDE3:
	.4byte	.LEFDE3-.LASFDE3	/* FDE Length */
.LASFDE3:
	.4byte	.LASFDE3-__FRAME_BEGIN__	/* FDE CIE offset */
#ifdef PIC
	.4byte	.LFB2-.	/* FDE initial location */
#else
	.4byte	.LFB2	/* FDE initial location */
#endif
	.4byte	.LFE2-.LFB2	 /* FDE address range */
#ifdef PIC
	.uleb128 0x0	/* Augmentation size */
#endif
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFI7-.LFB2
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0x4	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFI8-.LCFI7
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0x8	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFI9-.LCFI8
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0xc	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFIA-.LCFI9
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0x10	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFIB-.LCFIA
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0x14	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFIC-.LCFIB
	.byte	0xe	/* DW_CFA_def_cfa_offset */
	.byte	0x18	/* uleb128 0x4 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFID-.LCFIC
	.byte	0xe	/* DW_CFA_def_cfa_offset */
#if defined(__SH4__)
	.byte	24+48	/* uleb128 24+48 */
#else
	.byte	24+16	/* uleb128 24+16 */
#endif
	.byte	0x91	/* DW_CFA_offset, column 0x11 */
	.byte	0x6	/* uleb128 0x6 */
	.byte	0x8e	/* DW_CFA_offset, column 0xe */
	.byte	0x5	/* uleb128 0x5 */
	.byte	0x84	/* DW_CFA_offset, column 0x4 */
	.byte	0x4	/* uleb128 0x4 */
	.byte	0x85	/* DW_CFA_offset, column 0x5 */
	.byte	0x3	/* uleb128 0x3 */
	.byte	0x86	/* DW_CFA_offset, column 0x6 */
	.byte	0x2	/* uleb128 0x2 */
	.byte	0x87	/* DW_CFA_offset, column 0x7 */
	.byte	0x1	/* uleb128 0x1 */
	.byte	0x4	/* DW_CFA_advance_loc4 */
	.4byte	.LCFIE-.LCFID
	.byte	0xd	/* DW_CFA_def_cfa_register */
	.byte	0xe	/* uleb128 0xe */
	.align	2
.LEFDE3:
