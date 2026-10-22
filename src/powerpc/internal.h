#यदि_परिभाषितम् FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#परिभाषय PPC_TRAMP_MAP_सरणम्	16
#परिभाषय PPC_TRAMP_MAP_आकार	(1 << PPC_TRAMP_MAP_सरणम्)
# ifdef __PCREL__
#  define PPC_TRAMP_आकार	24
# अन्यथा
#  define PPC_TRAMP_आकार	40
# endif /* __PCREL__ */
#यदि_अन्तः /* FFI_EXEC_STATIC_TRAMP */
