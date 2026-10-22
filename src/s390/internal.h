/* If these values change, sysv.S must be adapted!  */
#تعریف_کرو FFI390_RET_DOUBLE	0
#تعریف_کرو FFI390_RET_FLOAT	1
#تعریف_کرو FFI390_RET_INT64	2
#تعریف_کرو FFI390_RET_INT32	3
#تعریف_کرو FFI390_RET_VOID		4

#تعریف_کرو FFI360_RET_نقاب		7
#تعریف_کرو FFI390_RET_وچ_MEM	8

#تعریف_کرو FFI390_RET_STRUCT	(FFI390_RET_VOID | FFI390_RET_وچ_MEM)


#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K is chosen.
 */
#تعریف_کرو FFI390_TRAMP_MAP_کھسکاؤ	12
#تعریف_کرو FFI390_TRAMP_MAP_ناپ	(1 << FFI390_TRAMP_MAP_کھسکاؤ)
#تعریف_کرو FFI390_TRAMP_ناپ	16

#جے_انت
