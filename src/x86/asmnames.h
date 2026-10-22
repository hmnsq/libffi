#که_نه_تعریف_شوی ASMNAMES_H
#تعریف_کړه ASMNAMES_H

#تعریف_کړه C2(X, Y)  X ## Y
#تعریف_کړه C1(X, Y)  C2(X, Y)
#که_تعریف_شوی __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#وإلا
# define C(X)     X
#د_که_پای

#که_تعریف_شوی __APPLE__
# define L(X)     C1(L, X)
#وإلا
# define L(X)     C1(.L, X)
#د_که_پای

#إذا defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#وإلا
# define PLT(X)	  X
#د_که_پای

#که_تعریف_شوی __ELF__
# define ENDF(X)  .type	X,@function; .اندازه X, . - X
#وإلا
# define ENDF(X)
#د_که_پای

#د_که_پای /* ASMNAMES_H */
