#घोषणा UNIX64_RET_VOID		0
#घोषणा UNIX64_RET_UINT8	1
#घोषणा UNIX64_RET_UINT16	2
#घोषणा UNIX64_RET_UINT32	3
#घोषणा UNIX64_RET_SINT8	4
#घोषणा UNIX64_RET_SINT16	5
#घोषणा UNIX64_RET_SINT32	6
#घोषणा UNIX64_RET_INT64	7
#घोषणा UNIX64_RET_XMM32	8
#घोषणा UNIX64_RET_XMM64	9
#घोषणा UNIX64_RET_X87		10
#घोषणा UNIX64_RET_X87_2	11
#घोषणा UNIX64_RET_ST_XMM0_RAX	12
#घोषणा UNIX64_RET_ST_RAX_XMM0	13
#घोषणा UNIX64_RET_ST_XMM0_XMM1	14
#घोषणा UNIX64_RET_ST_RAX_RDX	15
#घोषणा UNIX64_RET_XMM128	16	/* binary128 long double in %xmm0 */

#घोषणा UNIX64_RET_LAST		16

#घोषणा UNIX64_FLAG_RET_IN_MEM	(1 << 10)
#घोषणा UNIX64_FLAG_XMM_ARGS	(1 << 11)
#घोषणा UNIX64_आकार_SHIFT	12

#अगर defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#घोषणा UNIX64_TRAMP_MAP_SHIFT	12
#घोषणा UNIX64_TRAMP_MAP_आकार	(1 << UNIX64_TRAMP_MAP_SHIFT)
#अगर_घोषित ENDBR_PRESENT
#घोषणा UNIX64_TRAMP_आकार	40
#अन्यथा
#घोषणा UNIX64_TRAMP_आकार	32
#पूर्ण_अगर
#पूर्ण_अगर
