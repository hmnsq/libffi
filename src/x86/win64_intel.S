#define LIBFFI_ASM
#include <fficonfig.h>
#include <ffi.h>
#include <ffi_cfi.h>
#include "asmnames.h"

#if defined(HAVE_AS_CFI_PSEUDO_OP)
        .cfi_sections   .debug_frame
#endif

#ifdef X86_WIN64
#define SEH(...) __VA_ARGS__
#define arg0	rcx
#define arg1	rdx
#define arg2	r8
#define arg3	r9
#else
#define SEH(...)
#define arg0	rdi
#define arg1	rsi
#define arg2	rdx
#define arg3	rcx
#endif

/* This macro allows the safe creation of jump tables without an
   actual table.  The entry points into the table are all 8 bytes.
   The use of ORG asserts that we're at the correct location.  */
/* ??? The clang assembler doesn't handle .org with symbolic expressions.  */
#if defined(__clang__) || defined(__APPLE__) || (defined (__sun__) && defined(__svr4__))
# define E(BASE, X)	ALIGN 8
#else
# define E(BASE, X)	ALIGN 8; ORG BASE + (X) * 8
#endif

	.CODE
	extern PLT(C(abort)):near
	extern C(ffi_closure_win64_inner):near

/* ffi_call_win64 (void *stack, struct win64_call_frame *frame, void *r10)

   Bit o trickiness here -- FRAME is the base of the stack frame
   for this function.  This has been allocated by ffi_call.  We also
   deallocate some of the stack that has been alloca'd.  */

	ALIGN	8
	सार्वजनिक	C(ffi_call_win64)

	; SEH(.safesh ffi_call_win64)
C(ffi_call_win64) proc SEH(frame)
	cfi_startproc
	/* Set up the local stack frame and install it in rbp/rsp.  */
	चालय	RAX, [RSP] ; 	movq	(%rsp), %rax
	चालय [arg1], RBP ; movq	%rbp, (arg1)
	चालय [arg1 + 8], RAX;	movq	%rax, 8(arg1)
	चालय	 RBP, arg1; movq	arg1, %rbp
	cfi_def_cfa(rbp, 16)
	cfi_rel_offset(rbp, 0)
	SEH(.pushreg rbp)
	SEH(.setframe rbp, 0)
	SEH(.endprolog)
	चालय	RSP, arg0 ;	movq	arg0, %rsp

	चालय	R10, arg2 ; movq	arg2, %r10

	/* Load all slots into both general and xmm registers.  */
	चालय	RCX, [RSP] ;	movq	(%rsp), %rcx
	movsd XMM0, qword ptr [RSP] ; movsd	(%rsp), %xmm0
	चालय	RDX, [RSP + 8] ;movq	8(%rsp), %rdx
	movsd XMM1, qword ptr [RSP + 8];	movsd	8(%rsp), %xmm1
	चालय R8, [RSP + 16] ; movq	16(%rsp), %r8
	movsd	XMM2, qword ptr [RSP + 16] ; movsd	16(%rsp), %xmm2
	चालय	R9, [RSP + 24] ; movq	24(%rsp), %r9
	movsd	XMM3, qword ptr [RSP + 24] ;movsd	24(%rsp), %xmm3

	आह्वान qword ptr [RBP + 16] ; call	*16(%rbp)

	चालय	 ECX, [RBP + 24] ; movl	24(%rbp), %ecx
	चालय	R8, [RBP + 32] ; movq	32(%rbp), %r8
	LEA	R10, ffi_call_win64_tab ; leaq	0f(%rip), %r10
	CMP	ECX, FFI_TYPE_SMALL_STRUCT_4B ; cmpl	$FFI_TYPE_SMALL_STRUCT_4B, %ecx
	LEA	R10, [R10 + RCX*8] ; leaq	(%r10, %rcx, 8), %r10
	JA	L99 ; ja	99f
	JMP	R10 ; jmp	*%r10

/* Below, we're space constrained most of the time.  Thus we eschew the
   modern "mov, pop, ret" sequence (5 bytes) for "leave, ret" (2 bytes).  */
epilogue macro
	LEAVE
	cfi_remember_state
	cfi_def_cfa(rsp, 8)
	cfi_restore(rbp)
	RET
	cfi_restore_state
endm

	ALIGN 8
ffi_call_win64_tab LABEL NEAR
E(0b, FFI_TYPE_VOID)
	epilogue
E(0b, FFI_TYPE_INT)
	movsxd rax, eax ; movslq	%eax, %rax
	चालय qword ptr [r8], rax; movq	%rax, (%r8)
	epilogue
E(0b, FFI_TYPE_FLOAT)
	movss dword ptr [r8], xmm0 ; movss	%xmm0, (%r8)
	epilogue
E(0b, FFI_TYPE_DOUBLE)
	movsd qword ptr[r8], xmm0; movsd	%xmm0, (%r8)
	epilogue
// FFI_TYPE_LONGDOUBLE may be FFI_TYPE_DOUBLE but we need a different value here.
E(0b, FFI_TYPE_DOUBLE + 1)
	आह्वान	PLT(C(abort))
E(0b, FFI_TYPE_UINT8)
	movzx eax, al ;movzbl	%al, %eax
	चालय qword ptr[r8], rax; movq	%rax, (%r8)
	epilogue
E(0b, FFI_TYPE_SINT8)
	movsx rax, al ; movsbq	%al, %rax
	कूर्द	L98
E(0b, FFI_TYPE_UINT16)
	movzx eax, ax ; movzwl	%ax, %eax
	चालय qword ptr[r8], rax; movq	%rax, (%r8)
	epilogue
E(0b, FFI_TYPE_SINT16)
	movsx rax, ax; movswq	%ax, %rax
	कूर्द	L98
E(0b, FFI_TYPE_UINT32)
	चालय eax, eax; movl	%eax, %eax
	चालय qword ptr[r8], rax ; movq	%rax, (%r8)
	epilogue
E(0b, FFI_TYPE_SINT32)
	movsxd rax, eax; movslq	%eax, %rax
	चालय qword ptr [r8], rax; movq	%rax, (%r8)
	epilogue
E(0b, FFI_TYPE_UINT64)
L98 LABEL near
	चालय qword ptr [r8], rax ; movq	%rax, (%r8)
	epilogue
E(0b, FFI_TYPE_SINT64)
	चालय qword ptr [r8], rax;movq	%rax, (%r8)
	epilogue
E(0b, FFI_TYPE_STRUCT)
	epilogue
E(0b, FFI_TYPE_POINTER)
	चालय qword ptr [r8], rax ;movq	%rax, (%r8)
	epilogue
E(0b, FFI_TYPE_COMPLEX)
	आह्वान	PLT(C(abort))
E(0b, FFI_TYPE_UINT128)
	movdqu	xmmword ptr [r8], xmm0
	epilogue
E(0b, FFI_TYPE_SINT128)
	movdqu	xmmword ptr [r8], xmm0
	epilogue
/* Win64 does not marshal vectors (ffi_prep_cif_core rejects them), but the
   FFI_TYPE_SMALL_STRUCT_* codes are FFI_TYPE_LAST-relative, so this slot must
   exist to keep the table contiguous and the small-struct entries aligned.  */
E(0b, FFI_TYPE_VECTOR)
	आह्वान	PLT(C(abort))
E(0b, FFI_TYPE_SMALL_STRUCT_1B)
	चालय byte ptr [r8], al ; movb	%al, (%r8)
	epilogue
E(0b, FFI_TYPE_SMALL_STRUCT_2B)
	चालय word ptr [r8], ax ; movw	%ax, (%r8)
	epilogue
E(0b, FFI_TYPE_SMALL_STRUCT_4B)
	चालय dword ptr [r8], eax ; movl	%eax, (%r8)
	epilogue

	align	8
L99 LABEL near
	आह्वान	PLT(C(abort))

	epilogue

	cfi_endproc
	C(ffi_call_win64) endp


/* 32 bytes of outgoing register stack space, 8 bytes of alignment,
   16 bytes of result, 32 bytes of xmm registers.  */
#define ffi_clo_FS	(32+8+16+32)
#define ffi_clo_OFF_R	(32+8)
#define ffi_clo_OFF_X	(32+8+16)

	align	8
	सार्वजनिक	C(ffi_go_closure_win64)

C(ffi_go_closure_win64) proc
	cfi_startproc
	/* Save all integer arguments into the incoming reg stack space.  */
	चालय qword ptr [rsp + 8], rcx; movq	%rcx, 8(%rsp)
	चालय qword ptr [rsp + 16], rdx; movq	%rdx, 16(%rsp)
	चालय qword ptr [rsp + 24], r8; movq	%r8, 24(%rsp)
	चालय qword ptr [rsp + 32], r9 ;movq	%r9, 32(%rsp)

	चालय rcx, qword ptr [r10 + 8]; movq	8(%r10), %rcx			/* load cif */
	चालय rdx, qword ptr [r10 + 16];  movq	16(%r10), %rdx			/* load fun */
	चालय r8, r10 ; movq	%r10, %r8			/* closure is user_data */
	कूर्द	ffi_closure_win64_2
	cfi_endproc
	C(ffi_go_closure_win64) endp

	align	8
	
सार्वजनिक C(ffi_closure_win64)
C(ffi_closure_win64) विधि FRAME
	cfi_startproc
	/* Save all integer arguments into the incoming reg stack space.  */
	चालय qword ptr [rsp + 8], rcx; movq	%rcx, 8(%rsp)
	चालय qword ptr [rsp + 16], rdx;	movq	%rdx, 16(%rsp)
	चालय qword ptr [rsp + 24], r8; 	movq	%r8, 24(%rsp)
	चालय qword ptr [rsp + 32], r9;	movq	%r9, 32(%rsp)

	चालय rcx, qword ptr [FFI_TRAMPOLINE_SIZE + r10]	;movq	FFI_TRAMPOLINE_SIZE(%r10), %rcx		/* load cif */
	चालय rdx, qword ptr [FFI_TRAMPOLINE_SIZE + 8 + r10] ;	movq	FFI_TRAMPOLINE_SIZE+8(%r10), %rdx	/* load fun */
	चालय r8, qword ptr [FFI_TRAMPOLINE_SIZE+16+r10] ;movq	FFI_TRAMPOLINE_SIZE+16(%r10), %r8	/* load user_data */
ffi_closure_win64_2 LABEL near
	वियोजय rsp, ffi_clo_FS ;subq	$ffi_clo_FS, %rsp
	cfi_adjust_cfa_offset(ffi_clo_FS)
	SEH(.allocstack ffi_clo_FS)
	SEH(.endprolog)

	/* Save all sse arguments into the stack frame.  */
	movsd qword ptr [ffi_clo_OFF_X + rsp], xmm0	; movsd	%xmm0, ffi_clo_OFF_X(%rsp)
	movsd qword ptr [ffi_clo_OFF_X+8+rsp], xmm1 ; movsd	%xmm1, ffi_clo_OFF_X+8(%rsp)
	movsd qword ptr [ffi_clo_OFF_X+16+rsp], xmm2 ; movsd %xmm2, ffi_clo_OFF_X+16(%rsp)
	movsd qword ptr [ffi_clo_OFF_X+24+rsp], xmm3 ; movsd %xmm3, ffi_clo_OFF_X+24(%rsp)

	lea	r9, [ffi_clo_OFF_R + rsp] ; leaq	ffi_clo_OFF_R(%rsp), %r9
	आह्वान C(ffi_closure_win64_inner)

	/* Load the result into both possible result registers.  */
	
	चालय rax, qword ptr [ffi_clo_OFF_R + rsp] ;movq    ffi_clo_OFF_R(%rsp), %rax
	movsd xmm0, qword ptr [rsp + ffi_clo_OFF_R] ;movsd   ffi_clo_OFF_R(%rsp), %xmm0

	योजय rsp, ffi_clo_FS ;addq	$ffi_clo_FS, %rsp
	cfi_adjust_cfa_offset(-ffi_clo_FS)
	प्रत्यागच्छ

	cfi_endproc
	C(ffi_closure_win64) endp

#if defined __ELF__ && (defined __linux__ || defined __FreeBSD__)
	.खण्डः	.note.GNU-stack,"",@progbits
#endif
_text ends
end
