#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_TRAMP_MAP_ಸರಿಸು	16
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_TRAMP_MAP_ಗಾತ್ರ	(1 << PPC_TRAMP_MAP_ಸರಿಸು)
# ifdef __PCREL__
#  define PPC_TRAMP_ಗಾತ್ರ	24
# ಇಲ್ಲದಿದ್ದರೆ
#  define PPC_TRAMP_ಗಾತ್ರ	40
# endif /* __PCREL__ */
#ಆದರೆ_ಅಂತ್ಯ /* FFI_EXEC_STATIC_TRAMP */
