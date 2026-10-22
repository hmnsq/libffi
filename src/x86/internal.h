#నిర్వచించు X86_RET_FLOAT		0
#నిర్వచించు X86_RET_DOUBLE		1
#నిర్వచించు X86_RET_LDOUBLE		2
#నిర్వచించు X86_RET_SINT8		3
#నిర్వచించు X86_RET_SINT16		4
#నిర్వచించు X86_RET_UINT8		5
#నిర్వచించు X86_RET_UINT16		6
#నిర్వచించు X86_RET_INT64		7
#నిర్వచించు X86_RET_INT32		8
#నిర్వచించు X86_RET_VOID		9
#నిర్వచించు X86_RET_STRUCTPOP	10
#నిర్వచించు X86_RET_STRUCTARG       11
#నిర్వచించు X86_RET_STRUCT_1B	12
#నిర్వచించు X86_RET_STRUCT_2B	13
#నిర్వచించు X86_RET_UNUSED14	14
#నిర్వచించు X86_RET_UNUSED15	15

#నిర్వచించు X86_RET_TYPE_ముసుగు	15
#నిర్వచించు X86_RET_తీయి_జరుపు	4

#నిర్వచించు R_EAX	0
#నిర్వచించు R_EDX	1
#నిర్వచించు R_ECX	2

#నిర్వచితమైతే __PCC__
# define HAVE_FASTCALL 0
#లేకపోతే
# define HAVE_FASTCALL 1
#అయితే_ముగింపు

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#నిర్వచించు X86_TRAMP_MAP_జరుపు	12
#నిర్వచించు X86_TRAMP_MAP_కొలత	(1 << X86_TRAMP_MAP_జరుపు)
#నిర్వచితమైతే ENDBR_PRESENT
#నిర్వచించు X86_TRAMP_కొలత		44
#లేకపోతే
#నిర్వచించు X86_TRAMP_కొలత		40
#అయితే_ముగింపు
#అయితే_ముగింపు
