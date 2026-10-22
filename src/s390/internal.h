/* If these values change, sysv.S must be adapted!  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI390_RET_DOUBLE	0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI390_RET_FLOAT	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI390_RET_INT64	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI390_RET_INT32	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI390_RET_VOID		4

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI360_RET_ਨਕਾਬ		7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI390_RET_ਵਿੱਚ_MEM	8

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI390_RET_STRUCT	(FFI390_RET_VOID | FFI390_RET_ਵਿੱਚ_MEM)


#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K is chosen.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI390_TRAMP_MAP_ਖਿਸਕਾਓ	12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI390_TRAMP_MAP_ਮਾਪ	(1 << FFI390_TRAMP_MAP_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI390_TRAMP_ਮਾਪ	16

#ਜੇ_ਅੰਤ
