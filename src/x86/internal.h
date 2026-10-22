#تعریف_کن X86_RET_FLOAT		0
#تعریف_کن X86_RET_DOUBLE		1
#تعریف_کن X86_RET_LDOUBLE		2
#تعریف_کن X86_RET_SINT8		3
#تعریف_کن X86_RET_SINT16		4
#تعریف_کن X86_RET_UINT8		5
#تعریف_کن X86_RET_UINT16		6
#تعریف_کن X86_RET_INT64		7
#تعریف_کن X86_RET_INT32		8
#تعریف_کن X86_RET_VOID		9
#تعریف_کن X86_RET_STRUCTPOP	10
#تعریف_کن X86_RET_STRUCTARG       11
#تعریف_کن X86_RET_STRUCT_1B	12
#تعریف_کن X86_RET_STRUCT_2B	13
#تعریف_کن X86_RET_UNUSED14	14
#تعریف_کن X86_RET_UNUSED15	15

#تعریف_کن X86_RET_TYPE_ماسک	15
#تعریف_کن X86_RET_برداشت_لغزش	4

#تعریف_کن R_EAX	0
#تعریف_کن R_EDX	1
#تعریف_کن R_ECX	2

#اگر_تعریف_شده __PCC__
# define HAVE_FASTCALL 0
#وإلا
# define HAVE_FASTCALL 1
#پایان_اگر

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#تعریف_کن X86_TRAMP_MAP_لغزش	12
#تعریف_کن X86_TRAMP_MAP_اندازه	(1 << X86_TRAMP_MAP_لغزش)
#اگر_تعریف_شده ENDBR_PRESENT
#تعریف_کن X86_TRAMP_اندازه		44
#وإلا
#تعریف_کن X86_TRAMP_اندازه		40
#پایان_اگر
#پایان_اگر
