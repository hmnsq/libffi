#تعریف_کرو X86_RET_FLOAT		0
#تعریف_کرو X86_RET_DOUBLE		1
#تعریف_کرو X86_RET_LDOUBLE		2
#تعریف_کرو X86_RET_SINT8		3
#تعریف_کرو X86_RET_SINT16		4
#تعریف_کرو X86_RET_UINT8		5
#تعریف_کرو X86_RET_UINT16		6
#تعریف_کرو X86_RET_INT64		7
#تعریف_کرو X86_RET_INT32		8
#تعریف_کرو X86_RET_VOID		9
#تعریف_کرو X86_RET_STRUCTPOP	10
#تعریف_کرو X86_RET_STRUCTARG       11
#تعریف_کرو X86_RET_STRUCT_1B	12
#تعریف_کرو X86_RET_STRUCT_2B	13
#تعریف_کرو X86_RET_UNUSED14	14
#تعریف_کرو X86_RET_UNUSED15	15

#تعریف_کرو X86_RET_TYPE_نقاب	15
#تعریف_کرو X86_RET_نکالو_سرکاؤ	4

#تعریف_کرو R_EAX	0
#تعریف_کرو R_EDX	1
#تعریف_کرو R_ECX	2

#اگر_تعریف_شدہ __PCC__
# define HAVE_FASTCALL 0
#ورنہ
# define HAVE_FASTCALL 1
#اگر_ختم

#اگر defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#تعریف_کرو X86_TRAMP_MAP_سرکاؤ	12
#تعریف_کرو X86_TRAMP_MAP_سائز	(1 << X86_TRAMP_MAP_سرکاؤ)
#اگر_تعریف_شدہ ENDBR_PRESENT
#تعریف_کرو X86_TRAMP_سائز		44
#ورنہ
#تعریف_کرو X86_TRAMP_سائز		40
#اگر_ختم
#اگر_ختم
