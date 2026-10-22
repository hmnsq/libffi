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

#تعریف_کرو UNIX64_FLAG_RET_اندر_MEM	(1 << 10)
#تعریف_کرو UNIX64_FLAG_XMM_دلائل	(1 << 11)
#تعریف_کرو UNIX64_سائز_سرکاؤ	12

#اگر defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#تعریف_کرو UNIX64_TRAMP_MAP_سرکاؤ	12
#تعریف_کرو UNIX64_TRAMP_MAP_سائز	(1 << UNIX64_TRAMP_MAP_سرکاؤ)
#اگر_تعریف_شدہ ENDBR_PRESENT
#تعریف_کرو UNIX64_TRAMP_سائز	40
#ورنہ
#تعریف_کرو UNIX64_TRAMP_سائز	32
#اگر_ختم
#اگر_ختم
