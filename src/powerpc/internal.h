#اگر_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#تعریف_کرو PPC_TRAMP_MAP_سرکاؤ	16
#تعریف_کرو PPC_TRAMP_MAP_سائز	(1 << PPC_TRAMP_MAP_سرکاؤ)
# ifdef __PCREL__
#  define PPC_TRAMP_سائز	24
# ورنہ
#  define PPC_TRAMP_سائز	40
# endif /* __PCREL__ */
#اگر_ختم /* FFI_EXEC_STATIC_TRAMP */
