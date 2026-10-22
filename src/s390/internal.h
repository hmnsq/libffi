/* If these values change, sysv.S must be adapted!  */
#परिभाषय FFI390_RET_DOUBLE	0
#परिभाषय FFI390_RET_FLOAT	1
#परिभाषय FFI390_RET_INT64	2
#परिभाषय FFI390_RET_INT32	3
#परिभाषय FFI390_RET_VOID		4

#परिभाषय FFI360_RET_आवरणम्		7
#परिभाषय FFI390_RET_अन्तर्_MEM	8

#परिभाषय FFI390_RET_STRUCT	(FFI390_RET_VOID | FFI390_RET_अन्तर्_MEM)


#यदि defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K is chosen.
 */
#परिभाषय FFI390_TRAMP_MAP_सरणम्	12
#परिभाषय FFI390_TRAMP_MAP_आकार	(1 << FFI390_TRAMP_MAP_सरणम्)
#परिभाषय FFI390_TRAMP_आकार	16

#यदि_अन्तः
