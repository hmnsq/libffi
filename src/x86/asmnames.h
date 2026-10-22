#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ASMNAMES_H
#ವ್ಯಾಖ್ಯಾನಿಸು ASMNAMES_H

#ವ್ಯಾಖ್ಯಾನಿಸು C2(X, Y)  X ## Y
#ವ್ಯಾಖ್ಯಾನಿಸು C1(X, Y)  C2(X, Y)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#ಇಲ್ಲದಿದ್ದರೆ
# define C(X)     X
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
# define L(X)     C1(L, X)
#ಇಲ್ಲದಿದ್ದರೆ
# define L(X)     C1(.L, X)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#ಇಲ್ಲದಿದ್ದರೆ
# define PLT(X)	  X
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __ELF__
# define ENDF(X)  .type	X,@function; .ಗಾತ್ರ_2 X, . - X
#ಇಲ್ಲದಿದ್ದರೆ
# define ENDF(X)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* ASMNAMES_H */
