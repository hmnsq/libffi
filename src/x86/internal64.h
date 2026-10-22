#تعریف_کرو UNIX64_RET_VOID		0
#تعریف_کرو UNIX64_RET_UINT8	1
#تعریف_کرو UNIX64_RET_UINT16	2
#تعریف_کرو UNIX64_RET_UINT32	3
#تعریف_کرو UNIX64_RET_SINT8	4
#تعریف_کرو UNIX64_RET_SINT16	5
#تعریف_کرو UNIX64_RET_SINT32	6
#تعریف_کرو UNIX64_RET_INT64	7
#تعریف_کرو UNIX64_RET_XMM32	8
#تعریف_کرو UNIX64_RET_XMM64	9
#تعریف_کرو UNIX64_RET_X87		10
#تعریف_کرو UNIX64_RET_X87_2	11
#تعریف_کرو UNIX64_RET_ST_XMM0_RAX	12
#تعریف_کرو UNIX64_RET_ST_RAX_XMM0	13
#تعریف_کرو UNIX64_RET_ST_XMM0_XMM1	14
#تعریف_کرو UNIX64_RET_ST_RAX_RDX	15
#تعریف_کرو UNIX64_RET_XMM128	16	/* binary128 long double in %xmm0 */

#تعریف_کرو UNIX64_RET_آخری		16

#تعریف_کرو UNIX64_FLAG_RET_وچ_MEM	(1 << 10)
#تعریف_کرو UNIX64_FLAG_XMM_دلیلاں	(1 << 11)
#تعریف_کرو UNIX64_ناپ_کھسکاؤ	12

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#تعریف_کرو UNIX64_TRAMP_MAP_کھسکاؤ	12
#تعریف_کرو UNIX64_TRAMP_MAP_ناپ	(1 << UNIX64_TRAMP_MAP_کھسکاؤ)
#جے_تعریف_شدہ ENDBR_PRESENT
#تعریف_کرو UNIX64_TRAMP_ناپ	40
#وإلا
#تعریف_کرو UNIX64_TRAMP_ناپ	32
#جے_انت
#جے_انت
