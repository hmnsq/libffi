#اگر_تعریف_نشده ASMNAMES_H
#تعریف_کن ASMNAMES_H

#تعریف_کن C2(X, Y)  X ## Y
#تعریف_کن C1(X, Y)  C2(X, Y)
#اگر_تعریف_شده __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#وإلا
# define C(X)     X
#پایان_اگر

#اگر_تعریف_شده __APPLE__
# define L(X)     C1(L, X)
#وإلا
# define L(X)     C1(.L, X)
#پایان_اگر

#إذا defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#وإلا
# define PLT(X)	  X
#پایان_اگر

#اگر_تعریف_شده __ELF__
# define ENDF(X)  .type	X,@function; .اندازه X, . - X
#وإلا
# define ENDF(X)
#پایان_اگر

#پایان_اگر /* ASMNAMES_H */
