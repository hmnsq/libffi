#تعریف_کن UNIX64_RET_VOID		0
#تعریف_کن UNIX64_RET_UINT8	1
#تعریف_کن UNIX64_RET_UINT16	2
#تعریف_کن UNIX64_RET_UINT32	3
#تعریف_کن UNIX64_RET_SINT8	4
#تعریف_کن UNIX64_RET_SINT16	5
#تعریف_کن UNIX64_RET_SINT32	6
#تعریف_کن UNIX64_RET_INT64	7
#تعریف_کن UNIX64_RET_XMM32	8
#تعریف_کن UNIX64_RET_XMM64	9
#تعریف_کن UNIX64_RET_X87		10
#تعریف_کن UNIX64_RET_X87_2	11
#تعریف_کن UNIX64_RET_ST_XMM0_RAX	12
#تعریف_کن UNIX64_RET_ST_RAX_XMM0	13
#تعریف_کن UNIX64_RET_ST_XMM0_XMM1	14
#تعریف_کن UNIX64_RET_ST_RAX_RDX	15
#تعریف_کن UNIX64_RET_XMM128	16	/* binary128 long double in %xmm0 */

#تعریف_کن UNIX64_RET_آخرین		16

#تعریف_کن UNIX64_FLAG_RET_درون_MEM	(1 << 10)
#تعریف_کن UNIX64_FLAG_XMM_آرگومانها	(1 << 11)
#تعریف_کن UNIX64_اندازه_لغزش	12

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#تعریف_کن UNIX64_TRAMP_MAP_لغزش	12
#تعریف_کن UNIX64_TRAMP_MAP_اندازه	(1 << UNIX64_TRAMP_MAP_لغزش)
#اگر_تعریف_شده ENDBR_PRESENT
#تعریف_کن UNIX64_TRAMP_اندازه	40
#وإلا
#تعریف_کن UNIX64_TRAMP_اندازه	32
#پایان_اگر
#پایان_اگر
