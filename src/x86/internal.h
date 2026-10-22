#وصف_ڪريو X86_RET_FLOAT		0
#وصف_ڪريو X86_RET_DOUBLE		1
#وصف_ڪريو X86_RET_LDOUBLE		2
#وصف_ڪريو X86_RET_SINT8		3
#وصف_ڪريو X86_RET_SINT16		4
#وصف_ڪريو X86_RET_UINT8		5
#وصف_ڪريو X86_RET_UINT16		6
#وصف_ڪريو X86_RET_INT64		7
#وصف_ڪريو X86_RET_INT32		8
#وصف_ڪريو X86_RET_VOID		9
#وصف_ڪريو X86_RET_STRUCTPOP	10
#وصف_ڪريو X86_RET_STRUCTARG       11
#وصف_ڪريو X86_RET_STRUCT_1B	12
#وصف_ڪريو X86_RET_STRUCT_2B	13
#وصف_ڪريو X86_RET_UNUSED14	14
#وصف_ڪريو X86_RET_UNUSED15	15

#وصف_ڪريو X86_RET_TYPE_نقاب	15
#وصف_ڪريو X86_RET_ڪڍو_سرڪايو	4

#وصف_ڪريو R_EAX	0
#وصف_ڪريو R_EDX	1
#وصف_ڪريو R_ECX	2

#جيڪڏهن_وصف_ٿيل __PCC__
# define HAVE_FASTCALL 0
#وإلا
# define HAVE_FASTCALL 1
#جيڪڏهن_پورو

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#وصف_ڪريو X86_TRAMP_MAP_سرڪايو	12
#وصف_ڪريو X86_TRAMP_MAP_سائيز	(1 << X86_TRAMP_MAP_سرڪايو)
#جيڪڏهن_وصف_ٿيل ENDBR_PRESENT
#وصف_ڪريو X86_TRAMP_سائيز		44
#وإلا
#وصف_ڪريو X86_TRAMP_سائيز		40
#جيڪڏهن_پورو
#جيڪڏهن_پورو
