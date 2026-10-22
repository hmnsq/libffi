#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_EXEC_STATIC_TRAMP
/* For the trampoline code table mapping, a mapping size of 64K is chosen.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_TRAMP_MAP_ਖਿਸਕਾਓ	16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_TRAMP_MAP_ਮਾਪ	(1 << PPC_TRAMP_MAP_ਖਿਸਕਾਓ)
# ifdef __PCREL__
#  define PPC_TRAMP_ਮਾਪ	24
# ਨਹੀਂਤਾਂ
#  define PPC_TRAMP_ਮਾਪ	40
# endif /* __PCREL__ */
#ਜੇ_ਅੰਤ /* FFI_EXEC_STATIC_TRAMP */
