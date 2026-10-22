#جے_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#تعریف_کرو PPC_TRAMP_MAP_کھسکاؤ	16
#تعریف_کرو PPC_TRAMP_MAP_ناپ	(1 << PPC_TRAMP_MAP_کھسکاؤ)
# ifdef __PCREL__
#  define PPC_TRAMP_ناپ	24
# وإلا
#  define PPC_TRAMP_ناپ	40
# endif /* __PCREL__ */
#جے_انت /* FFI_EXEC_STATIC_TRAMP */
