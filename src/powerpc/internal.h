#ifdef FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#define PPC_TRAMP_MAP_SHIFT	16
#define PPC_TRAMP_MAP_আকার	(1 << PPC_TRAMP_MAP_SHIFT)
# ifdef __PCREL__
#  define PPC_TRAMP_আকার	24
# অন্যথা
#  define PPC_TRAMP_আকার	40
# endif /* __PCREL__ */
#endif /* FFI_EXEC_STATIC_TRAMP */
