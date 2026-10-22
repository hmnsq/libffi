#اگر_تعریف_شده FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#تعریف_کن PPC_TRAMP_MAP_لغزش	16
#تعریف_کن PPC_TRAMP_MAP_اندازه	(1 << PPC_TRAMP_MAP_لغزش)
# ifdef __PCREL__
#  define PPC_TRAMP_اندازه	24
# وإلا
#  define PPC_TRAMP_اندازه	40
# endif /* __PCREL__ */
#پایان_اگر /* FFI_EXEC_STATIC_TRAMP */
