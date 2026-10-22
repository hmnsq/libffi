#وصف_ڪريو UNIX64_RET_VOID		0
#وصف_ڪريو UNIX64_RET_UINT8	1
#وصف_ڪريو UNIX64_RET_UINT16	2
#وصف_ڪريو UNIX64_RET_UINT32	3
#وصف_ڪريو UNIX64_RET_SINT8	4
#وصف_ڪريو UNIX64_RET_SINT16	5
#وصف_ڪريو UNIX64_RET_SINT32	6
#وصف_ڪريو UNIX64_RET_INT64	7
#وصف_ڪريو UNIX64_RET_XMM32	8
#وصف_ڪريو UNIX64_RET_XMM64	9
#وصف_ڪريو UNIX64_RET_X87		10
#وصف_ڪريو UNIX64_RET_X87_2	11
#وصف_ڪريو UNIX64_RET_ST_XMM0_RAX	12
#وصف_ڪريو UNIX64_RET_ST_RAX_XMM0	13
#وصف_ڪريو UNIX64_RET_ST_XMM0_XMM1	14
#وصف_ڪريو UNIX64_RET_ST_RAX_RDX	15
#وصف_ڪريو UNIX64_RET_XMM128	16	/* binary128 long double in %xmm0 */

#وصف_ڪريو UNIX64_RET_آخري		16

#وصف_ڪريو UNIX64_FLAG_RET_اندريون_MEM	(1 << 10)
#وصف_ڪريو UNIX64_FLAG_XMM_دليلون	(1 << 11)
#وصف_ڪريو UNIX64_سائيز_سرڪايو	12

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#وصف_ڪريو UNIX64_TRAMP_MAP_سرڪايو	12
#وصف_ڪريو UNIX64_TRAMP_MAP_سائيز	(1 << UNIX64_TRAMP_MAP_سرڪايو)
#جيڪڏهن_وصف_ٿيل ENDBR_PRESENT
#وصف_ڪريو UNIX64_TRAMP_سائيز	40
#وإلا
#وصف_ڪريو UNIX64_TRAMP_سائيز	32
#جيڪڏهن_پورو
#جيڪڏهن_پورو
