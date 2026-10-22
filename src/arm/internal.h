#تعریف_کرو ARM_TYPE_VFP_S	0
#تعریف_کرو ARM_TYPE_VFP_D	1
#تعریف_کرو ARM_TYPE_VFP_N	2
#تعریف_کرو ARM_TYPE_INT64	3
#تعریف_کرو ARM_TYPE_INT	4
#تعریف_کرو ARM_TYPE_VOID	5
#تعریف_کرو ARM_TYPE_STRUCT	6

#اگر defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#تعریف_کرو ARM_TRAMP_MAP_سرکاؤ	12
#تعریف_کرو ARM_TRAMP_MAP_سائز	(1 << ARM_TRAMP_MAP_سرکاؤ)
#تعریف_کرو ARM_TRAMP_سائز		20
#اگر_ختم
