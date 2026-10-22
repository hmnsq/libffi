#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ASMNAMES_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASMNAMES_H

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C2(X, Y)  X ## Y
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C1(X, Y)  C2(X, Y)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#ਨਹੀਂਤਾਂ
# define C(X)     X
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
# define L(X)     C1(L, X)
#ਨਹੀਂਤਾਂ
# define L(X)     C1(.L, X)
#ਜੇ_ਅੰਤ

#ਜੇ defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#ਨਹੀਂਤਾਂ
# define PLT(X)	  X
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __ELF__
# define ENDF(X)  .type	X,@function; .ਮਾਪ X, . - X
#ਨਹੀਂਤਾਂ
# define ENDF(X)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* ASMNAMES_H */
