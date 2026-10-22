#घोषणा ARM_TYPE_VFP_S	0
#घोषणा ARM_TYPE_VFP_D	1
#घोषणा ARM_TYPE_VFP_N	2
#घोषणा ARM_TYPE_INT64	3
#घोषणा ARM_TYPE_INT	4
#घोषणा ARM_TYPE_VOID	5
#घोषणा ARM_TYPE_STRUCT	6

#अगर defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#घोषणा ARM_TRAMP_MAP_SHIFT	12
#घोषणा ARM_TRAMP_MAP_आकार	(1 << ARM_TRAMP_MAP_SHIFT)
#घोषणा ARM_TRAMP_आकार		20
#पूर्ण_अगर
