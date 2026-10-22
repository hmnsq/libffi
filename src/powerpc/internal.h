#अगर_घोषित FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#घोषणा PPC_TRAMP_MAP_SHIFT	16
#घोषणा PPC_TRAMP_MAP_आकार	(1 << PPC_TRAMP_MAP_SHIFT)
# ifdef __PCREL__
#  define PPC_TRAMP_आकार	24
# अन्यथा
#  define PPC_TRAMP_आकार	40
# endif /* __PCREL__ */
#पूर्ण_अगर /* FFI_EXEC_STATIC_TRAMP */
