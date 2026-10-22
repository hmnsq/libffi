#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_VOID		0
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_UINT8	1
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_UINT16	2
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_UINT32	3
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_SINT8	4
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_SINT16	5
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_SINT32	6
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_INT64	7
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_XMM32	8
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_XMM64	9
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_X87		10
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_X87_2	11
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_ST_XMM0_RAX	12
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_ST_RAX_XMM0	13
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_ST_XMM0_XMM1	14
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_ST_RAX_RDX	15
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_XMM128	16	/* binary128 long double in %xmm0 */

#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_RET_ಕೊನೆಯ		16

#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_FLAG_RET_ಒಳ_MEM	(1 << 10)
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_FLAG_XMM_ವಾದಗಳು	(1 << 11)
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_ಗಾತ್ರ_ಸರಿಸು	12

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_TRAMP_MAP_ಸರಿಸು	12
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_TRAMP_MAP_ಗಾತ್ರ	(1 << UNIX64_TRAMP_MAP_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ENDBR_PRESENT
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_TRAMP_ಗಾತ್ರ	40
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು UNIX64_TRAMP_ಗಾತ್ರ	32
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
