#घोषणा X86_RET_FLOAT		0
#घोषणा X86_RET_DOUBLE		1
#घोषणा X86_RET_LDOUBLE		2
#घोषणा X86_RET_SINT8		3
#घोषणा X86_RET_SINT16		4
#घोषणा X86_RET_UINT8		5
#घोषणा X86_RET_UINT16		6
#घोषणा X86_RET_INT64		7
#घोषणा X86_RET_INT32		8
#घोषणा X86_RET_VOID		9
#घोषणा X86_RET_STRUCTPOP	10
#घोषणा X86_RET_STRUCTARG       11
#घोषणा X86_RET_STRUCT_1B	12
#घोषणा X86_RET_STRUCT_2B	13
#घोषणा X86_RET_UNUSED14	14
#घोषणा X86_RET_UNUSED15	15

#घोषणा X86_RET_TYPE_MASK	15
#घोषणा X86_RET_निकालो_SHIFT	4

#घोषणा R_EAX	0
#घोषणा R_EDX	1
#घोषणा R_ECX	2

#अगर_घोषित __PCC__
# define HAVE_FASTCALL 0
#अन्यथा
# define HAVE_FASTCALL 1
#पूर्ण_अगर

#अगर defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#घोषणा X86_TRAMP_MAP_SHIFT	12
#घोषणा X86_TRAMP_MAP_आकार	(1 << X86_TRAMP_MAP_SHIFT)
#अगर_घोषित ENDBR_PRESENT
#घोषणा X86_TRAMP_आकार		44
#अन्यथा
#घोषणा X86_TRAMP_आकार		40
#पूर्ण_अगर
#पूर्ण_अगर
