#परिभाषय X86_RET_FLOAT		0
#परिभाषय X86_RET_DOUBLE		1
#परिभाषय X86_RET_LDOUBLE		2
#परिभाषय X86_RET_SINT8		3
#परिभाषय X86_RET_SINT16		4
#परिभाषय X86_RET_UINT8		5
#परिभाषय X86_RET_UINT16		6
#परिभाषय X86_RET_INT64		7
#परिभाषय X86_RET_INT32		8
#परिभाषय X86_RET_VOID		9
#परिभाषय X86_RET_STRUCTPOP	10
#परिभाषय X86_RET_STRUCTARG       11
#परिभाषय X86_RET_STRUCT_1B	12
#परिभाषय X86_RET_STRUCT_2B	13
#परिभाषय X86_RET_UNUSED14	14
#परिभाषय X86_RET_UNUSED15	15

#परिभाषय X86_RET_TYPE_आवरणम्	15
#परिभाषय X86_RET_निष्कासय_सरणम्	4

#परिभाषय R_EAX	0
#परिभाषय R_EDX	1
#परिभाषय R_ECX	2

#यदि_परिभाषितम् __PCC__
# define HAVE_FASTCALL 0
#अन्यथा
# define HAVE_FASTCALL 1
#यदि_अन्तः

#यदि defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#परिभाषय X86_TRAMP_MAP_सरणम्	12
#परिभाषय X86_TRAMP_MAP_आकार	(1 << X86_TRAMP_MAP_सरणम्)
#यदि_परिभाषितम् ENDBR_PRESENT
#परिभाषय X86_TRAMP_आकार		44
#अन्यथा
#परिभाषय X86_TRAMP_आकार		40
#यदि_अन्तः
#यदि_अन्तः
