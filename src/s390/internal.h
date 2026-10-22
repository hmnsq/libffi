/* If these values change, sysv.S must be adapted!  */
#تعریف_کړه FFI390_RET_DOUBLE	0
#تعریف_کړه FFI390_RET_FLOAT	1
#تعریف_کړه FFI390_RET_INT64	2
#تعریف_کړه FFI390_RET_INT32	3
#تعریف_کړه FFI390_RET_VOID		4

#تعریف_کړه FFI360_RET_ماسک		7
#تعریف_کړه FFI390_RET_دننه_MEM	8

#تعریف_کړه FFI390_RET_STRUCT	(FFI390_RET_VOID | FFI390_RET_دننه_MEM)


#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K is chosen.
 */
#تعریف_کړه FFI390_TRAMP_MAP_ښويدنه	12
#تعریف_کړه FFI390_TRAMP_MAP_اندازه	(1 << FFI390_TRAMP_MAP_ښويدنه)
#تعریف_کړه FFI390_TRAMP_اندازه	16

#د_که_پای
