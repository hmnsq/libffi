#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_VOID		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_UINT8	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_UINT16	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_UINT32	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_SINT8	4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_SINT16	5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_SINT32	6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_INT64	7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_XMM32	8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_XMM64	9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_X87		10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_X87_2	11
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_ST_XMM0_RAX	12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_ST_RAX_XMM0	13
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_ST_XMM0_XMM1	14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_ST_RAX_RDX	15
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_XMM128	16	/* binary128 long double in %xmm0 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_RET_ਆਖਰੀ		16

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_FLAG_RET_ਵਿੱਚ_MEM	(1 << 10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_FLAG_XMM_ਦਲੀਲਾਂ	(1 << 11)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_ਮਾਪ_ਖਿਸਕਾਓ	12

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_TRAMP_MAP_ਖਿਸਕਾਓ	12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_TRAMP_MAP_ਮਾਪ	(1 << UNIX64_TRAMP_MAP_ਖਿਸਕਾਓ)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ENDBR_PRESENT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_TRAMP_ਮਾਪ	40
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX64_TRAMP_ਮਾਪ	32
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
