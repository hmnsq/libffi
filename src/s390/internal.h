/* If these values change, sysv.S must be adapted!  */
#ವ್ಯಾಖ್ಯಾನಿಸು FFI390_RET_DOUBLE	0
#ವ್ಯಾಖ್ಯಾನಿಸು FFI390_RET_FLOAT	1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI390_RET_INT64	2
#ವ್ಯಾಖ್ಯಾನಿಸು FFI390_RET_INT32	3
#ವ್ಯಾಖ್ಯಾನಿಸು FFI390_RET_VOID		4

#ವ್ಯಾಖ್ಯಾನಿಸು FFI360_RET_ಮುಖವಾಡ		7
#ವ್ಯಾಖ್ಯಾನಿಸು FFI390_RET_ಒಳ_MEM	8

#ವ್ಯಾಖ್ಯಾನಿಸು FFI390_RET_STRUCT	(FFI390_RET_VOID | FFI390_RET_ಒಳ_MEM)


#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K is chosen.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು FFI390_TRAMP_MAP_ಸರಿಸು	12
#ವ್ಯಾಖ್ಯಾನಿಸು FFI390_TRAMP_MAP_ಗಾತ್ರ	(1 << FFI390_TRAMP_MAP_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI390_TRAMP_ಗಾತ್ರ	16

#ಆದರೆ_ಅಂತ್ಯ
