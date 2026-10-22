#परिभाषय ARM_TYPE_VFP_S	0
#परिभाषय ARM_TYPE_VFP_D	1
#परिभाषय ARM_TYPE_VFP_N	2
#परिभाषय ARM_TYPE_INT64	3
#परिभाषय ARM_TYPE_INT	4
#परिभाषय ARM_TYPE_VOID	5
#परिभाषय ARM_TYPE_STRUCT	6

#यदि defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#परिभाषय ARM_TRAMP_MAP_सरणम्	12
#परिभाषय ARM_TRAMP_MAP_आकार	(1 << ARM_TRAMP_MAP_सरणम्)
#परिभाषय ARM_TRAMP_आकार		20
#यदि_अन्तः
