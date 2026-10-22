#جيڪڏهن_وصف_ٿيل FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#وصف_ڪريو PPC_TRAMP_MAP_سرڪايو	16
#وصف_ڪريو PPC_TRAMP_MAP_سائيز	(1 << PPC_TRAMP_MAP_سرڪايو)
# ifdef __PCREL__
#  define PPC_TRAMP_سائيز	24
# وإلا
#  define PPC_TRAMP_سائيز	40
# endif /* __PCREL__ */
#جيڪڏهن_پورو /* FFI_EXEC_STATIC_TRAMP */
