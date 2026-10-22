#که_تعریف_شوی FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#تعریف_کړه PPC_TRAMP_MAP_ښويدنه	16
#تعریف_کړه PPC_TRAMP_MAP_اندازه	(1 << PPC_TRAMP_MAP_ښويدنه)
# ifdef __PCREL__
#  define PPC_TRAMP_اندازه	24
# وإلا
#  define PPC_TRAMP_اندازه	40
# endif /* __PCREL__ */
#د_که_پای /* FFI_EXEC_STATIC_TRAMP */
