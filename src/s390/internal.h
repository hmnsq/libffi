/* If these values change, sysv.S must be adapted!  */
#घोषणा FFI390_RET_DOUBLE	0
#घोषणा FFI390_RET_FLOAT	1
#घोषणा FFI390_RET_INT64	2
#घोषणा FFI390_RET_INT32	3
#घोषणा FFI390_RET_VOID		4

#घोषणा FFI360_RET_MASK		7
#घोषणा FFI390_RET_IN_MEM	8

#घोषणा FFI390_RET_STRUCT	(FFI390_RET_VOID | FFI390_RET_IN_MEM)


#अगर defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K is chosen.
 */
#घोषणा FFI390_TRAMP_MAP_SHIFT	12
#घोषणा FFI390_TRAMP_MAP_आकार	(1 << FFI390_TRAMP_MAP_SHIFT)
#घोषणा FFI390_TRAMP_आकार	16

#पूर्ण_अगर
