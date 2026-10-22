#परिभाषय UNIX64_RET_VOID		0
#परिभाषय UNIX64_RET_UINT8	1
#परिभाषय UNIX64_RET_UINT16	2
#परिभाषय UNIX64_RET_UINT32	3
#परिभाषय UNIX64_RET_SINT8	4
#परिभाषय UNIX64_RET_SINT16	5
#परिभाषय UNIX64_RET_SINT32	6
#परिभाषय UNIX64_RET_INT64	7
#परिभाषय UNIX64_RET_XMM32	8
#परिभाषय UNIX64_RET_XMM64	9
#परिभाषय UNIX64_RET_X87		10
#परिभाषय UNIX64_RET_X87_2	11
#परिभाषय UNIX64_RET_ST_XMM0_RAX	12
#परिभाषय UNIX64_RET_ST_RAX_XMM0	13
#परिभाषय UNIX64_RET_ST_XMM0_XMM1	14
#परिभाषय UNIX64_RET_ST_RAX_RDX	15
#परिभाषय UNIX64_RET_XMM128	16	/* binary128 long double in %xmm0 */

#परिभाषय UNIX64_RET_चरमः		16

#परिभाषय UNIX64_FLAG_RET_अन्तर्_MEM	(1 << 10)
#परिभाषय UNIX64_FLAG_XMM_तर्काः	(1 << 11)
#परिभाषय UNIX64_आकार_सरणम्	12

#यदि defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#परिभाषय UNIX64_TRAMP_MAP_सरणम्	12
#परिभाषय UNIX64_TRAMP_MAP_आकार	(1 << UNIX64_TRAMP_MAP_सरणम्)
#यदि_परिभाषितम् ENDBR_PRESENT
#परिभाषय UNIX64_TRAMP_आकार	40
#अन्यथा
#परिभाषय UNIX64_TRAMP_आकार	32
#यदि_अन्तः
#यदि_अन्तः
