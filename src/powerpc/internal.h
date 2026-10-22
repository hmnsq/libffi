#నిర్వచితమైతే FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#నిర్వచించు PPC_TRAMP_MAP_జరుపు	16
#నిర్వచించు PPC_TRAMP_MAP_కొలత	(1 << PPC_TRAMP_MAP_జరుపు)
# ifdef __PCREL__
#  define PPC_TRAMP_కొలత	24
# లేకపోతే
#  define PPC_TRAMP_కొలత	40
# endif /* __PCREL__ */
#అయితే_ముగింపు /* FFI_EXEC_STATIC_TRAMP */
