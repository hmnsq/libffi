#تعریف_کړه X86_RET_FLOAT		0
#تعریف_کړه X86_RET_DOUBLE		1
#تعریف_کړه X86_RET_LDOUBLE		2
#تعریف_کړه X86_RET_SINT8		3
#تعریف_کړه X86_RET_SINT16		4
#تعریف_کړه X86_RET_UINT8		5
#تعریف_کړه X86_RET_UINT16		6
#تعریف_کړه X86_RET_INT64		7
#تعریف_کړه X86_RET_INT32		8
#تعریف_کړه X86_RET_VOID		9
#تعریف_کړه X86_RET_STRUCTPOP	10
#تعریف_کړه X86_RET_STRUCTARG       11
#تعریف_کړه X86_RET_STRUCT_1B	12
#تعریف_کړه X86_RET_STRUCT_2B	13
#تعریف_کړه X86_RET_UNUSED14	14
#تعریف_کړه X86_RET_UNUSED15	15

#تعریف_کړه X86_RET_TYPE_ماسک	15
#تعریف_کړه X86_RET_راوباسه_ښويدنه	4

#تعریف_کړه R_EAX	0
#تعریف_کړه R_EDX	1
#تعریف_کړه R_ECX	2

#که_تعریف_شوی __PCC__
# define HAVE_FASTCALL 0
#وإلا
# define HAVE_FASTCALL 1
#د_که_پای

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#تعریف_کړه X86_TRAMP_MAP_ښويدنه	12
#تعریف_کړه X86_TRAMP_MAP_اندازه	(1 << X86_TRAMP_MAP_ښويدنه)
#که_تعریف_شوی ENDBR_PRESENT
#تعریف_کړه X86_TRAMP_اندازه		44
#وإلا
#تعریف_کړه X86_TRAMP_اندازه		40
#د_که_پای
#د_که_پای
