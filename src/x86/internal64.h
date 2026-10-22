#تعریف_کړه UNIX64_RET_VOID		0
#تعریف_کړه UNIX64_RET_UINT8	1
#تعریف_کړه UNIX64_RET_UINT16	2
#تعریف_کړه UNIX64_RET_UINT32	3
#تعریف_کړه UNIX64_RET_SINT8	4
#تعریف_کړه UNIX64_RET_SINT16	5
#تعریف_کړه UNIX64_RET_SINT32	6
#تعریف_کړه UNIX64_RET_INT64	7
#تعریف_کړه UNIX64_RET_XMM32	8
#تعریف_کړه UNIX64_RET_XMM64	9
#تعریف_کړه UNIX64_RET_X87		10
#تعریف_کړه UNIX64_RET_X87_2	11
#تعریف_کړه UNIX64_RET_ST_XMM0_RAX	12
#تعریف_کړه UNIX64_RET_ST_RAX_XMM0	13
#تعریف_کړه UNIX64_RET_ST_XMM0_XMM1	14
#تعریف_کړه UNIX64_RET_ST_RAX_RDX	15
#تعریف_کړه UNIX64_RET_XMM128	16	/* binary128 long double in %xmm0 */

#تعریف_کړه UNIX64_RET_وروستی		16

#تعریف_کړه UNIX64_FLAG_RET_دننه_MEM	(1 << 10)
#تعریف_کړه UNIX64_FLAG_XMM_دلیلونه	(1 << 11)
#تعریف_کړه UNIX64_اندازه_ښويدنه	12

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#تعریف_کړه UNIX64_TRAMP_MAP_ښويدنه	12
#تعریف_کړه UNIX64_TRAMP_MAP_اندازه	(1 << UNIX64_TRAMP_MAP_ښويدنه)
#که_تعریف_شوی ENDBR_PRESENT
#تعریف_کړه UNIX64_TRAMP_اندازه	40
#وإلا
#تعریف_کړه UNIX64_TRAMP_اندازه	32
#د_که_پای
#د_که_پای
