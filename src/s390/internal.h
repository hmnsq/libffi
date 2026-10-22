/* If these values change, sysv.S must be adapted!  */
#تعریف_کن FFI390_RET_DOUBLE	0
#تعریف_کن FFI390_RET_FLOAT	1
#تعریف_کن FFI390_RET_INT64	2
#تعریف_کن FFI390_RET_INT32	3
#تعریف_کن FFI390_RET_VOID		4

#تعریف_کن FFI360_RET_ماسک		7
#تعریف_کن FFI390_RET_درون_MEM	8

#تعریف_کن FFI390_RET_STRUCT	(FFI390_RET_VOID | FFI390_RET_درون_MEM)


#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K is chosen.
 */
#تعریف_کن FFI390_TRAMP_MAP_لغزش	12
#تعریف_کن FFI390_TRAMP_MAP_اندازه	(1 << FFI390_TRAMP_MAP_لغزش)
#تعریف_کن FFI390_TRAMP_اندازه	16

#پایان_اگر
