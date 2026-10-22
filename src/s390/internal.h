/* If these values change, sysv.S must be adapted!  */
#నిర్వచించు FFI390_RET_DOUBLE	0
#నిర్వచించు FFI390_RET_FLOAT	1
#నిర్వచించు FFI390_RET_INT64	2
#నిర్వచించు FFI390_RET_INT32	3
#నిర్వచించు FFI390_RET_VOID		4

#నిర్వచించు FFI360_RET_ముసుగు		7
#నిర్వచించు FFI390_RET_లోపల_MEM	8

#నిర్వచించు FFI390_RET_STRUCT	(FFI390_RET_VOID | FFI390_RET_లోపల_MEM)


#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K is chosen.
 */
#నిర్వచించు FFI390_TRAMP_MAP_జరుపు	12
#నిర్వచించు FFI390_TRAMP_MAP_కొలత	(1 << FFI390_TRAMP_MAP_జరుపు)
#నిర్వచించు FFI390_TRAMP_కొలత	16

#అయితే_ముగింపు
