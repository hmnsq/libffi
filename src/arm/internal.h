#تعریف_کن ARM_TYPE_VFP_S	0
#تعریف_کن ARM_TYPE_VFP_D	1
#تعریف_کن ARM_TYPE_VFP_N	2
#تعریف_کن ARM_TYPE_INT64	3
#تعریف_کن ARM_TYPE_INT	4
#تعریف_کن ARM_TYPE_VOID	5
#تعریف_کن ARM_TYPE_STRUCT	6

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#تعریف_کن ARM_TRAMP_MAP_لغزش	12
#تعریف_کن ARM_TRAMP_MAP_اندازه	(1 << ARM_TRAMP_MAP_لغزش)
#تعریف_کن ARM_TRAMP_اندازه		20
#پایان_اگر
