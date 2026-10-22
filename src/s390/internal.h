/* If these values change, sysv.S must be adapted!  */
#تعریف_کرو FFI390_RET_DOUBLE	0
#تعریف_کرو FFI390_RET_FLOAT	1
#تعریف_کرو FFI390_RET_INT64	2
#تعریف_کرو FFI390_RET_INT32	3
#تعریف_کرو FFI390_RET_VOID		4

#تعریف_کرو FFI360_RET_نقاب		7
#تعریف_کرو FFI390_RET_اندر_MEM	8

#تعریف_کرو FFI390_RET_STRUCT	(FFI390_RET_VOID | FFI390_RET_اندر_MEM)


#اگر defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K is chosen.
 */
#تعریف_کرو FFI390_TRAMP_MAP_سرکاؤ	12
#تعریف_کرو FFI390_TRAMP_MAP_سائز	(1 << FFI390_TRAMP_MAP_سرکاؤ)
#تعریف_کرو FFI390_TRAMP_سائز	16

#اگر_ختم
