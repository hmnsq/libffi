/* -----------------------------------------------------------------------
   sysv.S - Copyright (c) 1998, 2008, 2011 Red Hat, Inc.
	    Copyright (c) 2011 Plausible Labs Cooperative, Inc.

   ARM Foreign Function Interface

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

#ifdef __arm__
#define LIBFFI_ASM
#include <fficonfig.h>
#include <ffi.h>
#include <ffi_cfi.h>
#include "internal.h"

/* GCC 4.8 provides __ARM_ARCH; construct it otherwise.  */
#ifndef __ARM_ARCH
# if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) \
     || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) \
     || defined(__ARM_ARCH_7EM__)
#  define __ARM_ARCH 7
# elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) \
        || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) \
        || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) \
	|| defined(__ARM_ARCH_6M__)
#  define __ARM_ARCH 6
# elif defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5T__) \
	|| defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5TE__) \
	|| defined(__ARM_ARCH_5TEJ__)
#  define __ARM_ARCH 5
# else
#  define __ARM_ARCH 4
# endif
#endif

/* Conditionally compile unwinder directives.  */
#ifdef __ARM_EABI__
# define UNWIND(...)	__VA_ARGS__
#else
# define UNWIND(...)
#endif

#if defined(HAVE_AS_CFI_PSEUDO_OP) && defined(__ARM_EABI__)
	.cfi_sections	.debug_frame
#endif

#define CONCAT(a, b)	CONCAT2(a, b)
#define CONCAT2(a, b)	a ## b

#ifdef __USER_LABEL_PREFIX__
# define CNAME(X)	CONCAT (__USER_LABEL_PREFIX__, X)
#else
# define CNAME(X)	X
#endif
#ifdef __ELF__
# define SIZE(X)	.size CNAME(X), . - CNAME(X)
# define प्रकार(X, Y)	.type CNAME(X), Y
#else
# define SIZE(X)
# define प्रकार(X, Y)
#endif

#define ARM_FUNC_START_LOCAL(name)	\
	.align	3;			\
	प्रकार(CNAME(name), %function);	\
	CNAME(name):

#define ARM_FUNC_START(name)		\
	.globl CNAME(name);		\
	FFI_HIDDEN(CNAME(name));	\
	ARM_FUNC_START_LOCAL(name)

#define ARM_FUNC_END(name) \
	SIZE(name)

	.text
	.syntax unified
#if defined(_WIN32)
	/* Windows on ARM is thumb-only */
	.thumb
#else
	/* Keep the assembly in ARM mode in other cases, for simplicity
	 * (to avoid interworking issues). */
#undef __thumb__
	.arm
#endif

/* Aid in defining a jump table with 8 bytes between entries.  */
#ifdef __thumb__
/* In thumb mode, instructions can be shorter than expected in arm mode, so
 * we need to align the start of each case. */
# define E(index) .align 3
#elif defined(__clang__)
/* ??? The clang assembler doesn't handle .if with symbolic expressions.  */
# define E(index)
#else
# define E(index)				\
	.if . - 0b - 8*index;			\
	.error "type table out of sync";	\
	.endif
#endif


#ifdef __ELF__
	/* We require interworking on LDM, which implies ARMv5T,
	   which implies the existance of BLX.  */
 	.arch	armv5t
#endif

	/* Note that we use STC and LDC to encode VFP instructions,
	   so that we do not need ".fpu vfp", nor get that added to
	   the object file attributes.  These will not be executed
	   unless the FFI_VFP abi is used.  */

	@ r0:   stack
	@ r1:   frame
	@ r2:   fn
	@ r3:	vfp_used

ARM_FUNC_START(ffi_call_VFP)
	UNWIND(.fnstart)
	cfi_startproc

	तुलय	r3, #3			@ load only d0 if possible
	ite	le
	ldcle	p11, cr0, [r0]		@ vldrle d0, [r0]
	ldcgt	p11, cr0, [r0], {16}	@ vldmgt r0, {d0-d7}
	योजय	r0, r0, #64		@ discard the vfp register args
	/* FALLTHRU */
ARM_FUNC_END(ffi_call_VFP)

ARM_FUNC_START(ffi_call_SYSV)
	stm	r1, {fp, lr}
	चालय	fp, r1

	@ This is a bit of a lie wrt the origin of the unwind info, but
	@ now we've got the usual frame pointer and two saved registers.
	UNWIND(.save {fp,lr})
	UNWIND(.setfp fp, sp)
	cfi_def_cfa(fp, 8)
	cfi_rel_offset(fp, 0)
	cfi_rel_offset(lr, 4)

	चालय	sp, r0		@ install the stack pointer
	चालय	lr, r2		@ move the fn pointer out of the way
	ldr	ip, [fp, #16]	@ install the static chain
	ldmia	sp!, {r0-r3}	@ move first 4 parameters in registers.
	blx	lr		@ आह्वान fn

	@ Load r2 with the pointer to storage for the return value
	@ Load r3 with the return type code
	ldr	r2, [fp, #8]
	ldr	r3, [fp, #12]

	@ Deallocate the stack with the arguments.
	चालय	sp, fp
	cfi_def_cfa_register(sp)

	@ Store values stored in registers.
#ifndef __thumb__
	.align	3
	योजय	pc, pc, r3, lsl #3
	nop
#else
	adr	ip, 0f
	योजय	ip, ip, r3, lsl #3
	चालय	pc, ip
	.align	3
#endif
0:
E(ARM_TYPE_VFP_S)
	stc	p10, cr0, [r2]		@ vstr s0, [r2]
	उद्धर	{fp,pc}
E(ARM_TYPE_VFP_D)
	stc	p11, cr0, [r2]		@ vstr d0, [r2]
	उद्धर	{fp,pc}
E(ARM_TYPE_VFP_N)
	stc	p11, cr0, [r2], {8}	@ vstm r2, {d0-d3}
	उद्धर	{fp,pc}
E(ARM_TYPE_INT64)
	str	r1, [r2, #4]
	nop
E(ARM_TYPE_INT)
	str	r0, [r2]
	उद्धर	{fp,pc}
E(ARM_TYPE_VOID)
	उद्धर	{fp,pc}
	nop
E(ARM_TYPE_STRUCT)
	उद्धर	{fp,pc}

	cfi_endproc
	UNWIND(.fnend)
ARM_FUNC_END(ffi_call_SYSV)

#if FFI_CLOSURES

/*
	int ffi_closure_inner_* (cif, fun, user_data, frame)
*/

ARM_FUNC_START(ffi_go_closure_SYSV)
	cfi_startproc
	stmdb	sp!, {r0-r3}			@ save argument regs
	cfi_adjust_cfa_offset(16)
	ldr	r0, [ip, #4]			@ load cif
	ldr	r1, [ip, #8]			@ load fun
	चालय	r2, ip				@ load user_data
	b	0f
	cfi_endproc
ARM_FUNC_END(ffi_go_closure_SYSV)

ARM_FUNC_START(ffi_closure_SYSV)
	UNWIND(.fnstart)
	cfi_startproc
#ifdef _WIN32
	ldmfd	sp!, {r0, ip}			@ restore fp (r0 is used for stack alignment)
#endif
	stmdb	sp!, {r0-r3}			@ save argument regs
	cfi_adjust_cfa_offset(16)

#if FFI_EXEC_TRAMPOLINE_TABLE
	ldr ip, [ip]				@ ip points to the config page, dereference to get the ffi_closure*
#endif
	ldr	r0, [ip, #FFI_TRAMPOLINE_CLOSURE_OFFSET]	@ load cif
	ldr	r1, [ip, #FFI_TRAMPOLINE_CLOSURE_OFFSET+4]  @ load fun
	ldr	r2, [ip, #FFI_TRAMPOLINE_CLOSURE_OFFSET+8]  @ load user_data
0:
	योजय	ip, sp, #16			@ compute entry sp
	वियोजय	sp, sp, #64+32			@ allocate frame
	cfi_adjust_cfa_offset(64+32)
	stmdb	sp!, {ip,lr}

	/* Remember that EABI unwind info only applies at आह्वान sites.
	   We need do nothing except note the save of the stack pointer
	   and the link registers.  */
	UNWIND(.save {sp,lr})
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(lr, 4)

	योजय	r3, sp, #8			@ load frame
	bl	CNAME(ffi_closure_inner_SYSV)

	@ Load values returned in registers.
	योजय	r2, sp, #8+64			@ load result
	adr	r3, CNAME(ffi_closure_ret)
#ifndef __thumb__
	योजय	pc, r3, r0, lsl #3
#else
	योजय	r3, r3, r0, lsl #3
	चालय	pc, r3
#endif
	cfi_endproc
	UNWIND(.fnend)
ARM_FUNC_END(ffi_closure_SYSV)

ARM_FUNC_START(ffi_go_closure_VFP)
	cfi_startproc
	stmdb	sp!, {r0-r3}			@ save argument regs
	cfi_adjust_cfa_offset(16)
	ldr	r0, [ip, #4]			@ load cif
	ldr	r1, [ip, #8]			@ load fun
	चालय	r2, ip				@ load user_data
	b	0f
	cfi_endproc
ARM_FUNC_END(ffi_go_closure_VFP)

ARM_FUNC_START(ffi_closure_VFP)
	UNWIND(.fnstart)
	cfi_startproc
#ifdef _WIN32
	ldmfd	sp!, {r0, ip}			@ restore fp (r0 is used for stack alignment)
#endif
	stmdb	sp!, {r0-r3}			@ save argument regs
	cfi_adjust_cfa_offset(16)

#if FFI_EXEC_TRAMPOLINE_TABLE
	ldr ip, [ip]				@ ip points to the config page, dereference to get the ffi_closure*
#endif
	ldr	r0, [ip, #FFI_TRAMPOLINE_CLOSURE_OFFSET]	@ load cif
	ldr	r1, [ip, #FFI_TRAMPOLINE_CLOSURE_OFFSET+4]  @ load fun
	ldr	r2, [ip, #FFI_TRAMPOLINE_CLOSURE_OFFSET+8]  @ load user_data
0:
	योजय	ip, sp, #16
	वियोजय	sp, sp, #64+32			@ allocate frame
	cfi_adjust_cfa_offset(64+32)
	stc	p11, cr0, [sp], {16}		@ vstm sp, {d0-d7}
	stmdb	sp!, {ip,lr}

	/* See above.  */
	UNWIND(.save {sp,lr})
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(lr, 4)

	योजय	r3, sp, #8			@ load frame
	bl	CNAME(ffi_closure_inner_VFP)

	@ Load values returned in registers.
	योजय	r2, sp, #8+64			@ load result
	adr	r3, CNAME(ffi_closure_ret)
#ifndef __thumb__
	योजय	pc, r3, r0, lsl #3
#else
	योजय	r3, r3, r0, lsl #3
	चालय	pc, r3
#endif
	cfi_endproc
	UNWIND(.fnend)
ARM_FUNC_END(ffi_closure_VFP)

/* Load values returned in registers for both closure entry points.
   Note that we use LDM with SP in the register set.  This is deprecated
   by ARM, but not yet unpredictable.  */

ARM_FUNC_START_LOCAL(ffi_closure_ret)
	cfi_startproc
	cfi_rel_offset(sp, 0)
	cfi_rel_offset(lr, 4)
0:
E(ARM_TYPE_VFP_S)
	ldc	p10, cr0, [r2]			@ vldr s0, [r2]
	b	call_epilogue
E(ARM_TYPE_VFP_D)
	ldc	p11, cr0, [r2]			@ vldr d0, [r2]
	b	call_epilogue
E(ARM_TYPE_VFP_N)
	ldc	p11, cr0, [r2], {8}		@ vldm r2, {d0-d3}
	b	call_epilogue
E(ARM_TYPE_INT64)
	ldr	r1, [r2, #4]
	nop
E(ARM_TYPE_INT)
	ldr	r0, [r2]
	b	call_epilogue
E(ARM_TYPE_VOID)
	b	call_epilogue
	nop
E(ARM_TYPE_STRUCT)
	b	call_epilogue
call_epilogue:
#ifndef __thumb__
	ldm	sp, {sp,pc}
#else
	ldm	sp, {ip,lr}
	चालय	sp, ip
	bx	lr
#endif
	cfi_endproc
ARM_FUNC_END(ffi_closure_ret)

#if defined(FFI_EXEC_STATIC_TRAMP)
ARM_FUNC_START(ffi_closure_SYSV_alt)
	/* See the comments above trampoline_code_table. */
	ldr	ip, [sp, #4]			/* Load closure in ip */
	योजय	sp, sp, 8			/* Restore the stack */
	b	CNAME(ffi_closure_SYSV)
ARM_FUNC_END(ffi_closure_SYSV_alt)

ARM_FUNC_START(ffi_closure_VFP_alt)
	/* See the comments above trampoline_code_table. */
	ldr	ip, [sp, #4]			/* Load closure in ip */
	योजय	sp, sp, 8			/* Restore the stack */
	b	CNAME(ffi_closure_VFP)
ARM_FUNC_END(ffi_closure_VFP_alt)

/*
 * Below is the definition of the trampoline code table. Each element in
 * the code table is a trampoline.
 */
/*
 * The trampoline uses register ip (r12). It saves the original value of ip
 * on the stack.
 *
 * The trampoline has two parameters - target code to jump to and data for
 * the target code. The trampoline extracts the parameters from its parameter
 * block (see tramp_table_map()). The trampoline saves the data address on
 * the stack. Finally, it jumps to the target code.
 *
 * The target code can choose to:
 *
 * - restore the value of ip
 * - load the data address in a register
 * - restore the stack pointer to what it was when the trampoline was invoked.
 */
	.align	ARM_TRAMP_MAP_SHIFT
ARM_FUNC_START(trampoline_code_table)
	.rept	ARM_TRAMP_MAP_SIZE / ARM_TRAMP_SIZE
	वियोजय	sp, sp, #8		/* Make space on the stack */
	str	ip, [sp]		/* Save ip on stack */
	ldr	ip, [pc, #4080]		/* Copy data into ip */
	str	ip, [sp, #4]		/* Save data on stack */
	ldr	pc, [pc, #4076]		/* Copy code into PC */
	.endr
ARM_FUNC_END(trampoline_code_table)
	.align	ARM_TRAMP_MAP_SHIFT
#endif /* FFI_EXEC_STATIC_TRAMP */

#endif /* FFI_CLOSURES */

#if FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#include <mach/machine/vm_param.h>

.align	PAGE_MAX_SHIFT
ARM_FUNC_START(ffi_closure_trampoline_table_page)
.rept	PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE
	adr ip, #-PAGE_MAX_SIZE   @ the config page is PAGE_MAX_SIZE behind the trampoline page
	वियोजय ip, #8				  @ account for pc bias
	ldr	pc, [ip, #4]		  @ jump to ffi_closure_SYSV or ffi_closure_VFP
.endr
ARM_FUNC_END(ffi_closure_trampoline_table_page)
#endif

#elif defined(_WIN32)

ARM_FUNC_START(ffi_arm_trampoline)
0:	adr	ip, 0b
	stmdb	sp!, {r0, ip}
	ldr	pc, 1f
1:	.long	0
ARM_FUNC_END(ffi_arm_trampoline)

#else

ARM_FUNC_START(ffi_arm_trampoline)
0:	adr	ip, 0b
	ldr	pc, 1f
1:	.long	0
ARM_FUNC_END(ffi_arm_trampoline)

#endif /* FFI_EXEC_TRAMPOLINE_TABLE */
#endif /* __arm__ */

#if defined __ELF__ && defined __linux__
	.खण्डः	.note.GNU-stack,"",%progbits
#endif
