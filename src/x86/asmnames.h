#ifndef ASMNAMES_H
#define ASMNAMES_H

#define C2(X, Y)  X ## Y
#define C1(X, Y)  C2(X, Y)
#ifdef __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#ᰀᰧᰀ
# define C(X)     X
#endif

#ifdef __APPLE__
# define L(X)     C1(L, X)
#ᰀᰧᰀ
# define L(X)     C1(.L, X)
#endif

#ᰀᰧ defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#ᰀᰧᰀ
# define PLT(X)	  X
#endif

#ifdef __ELF__
# define ENDF(X)  .type	X,@function; .size X, . - X
#ᰀᰧᰀ
# define ENDF(X)
#endif

#endif /* ASMNAMES_H */
