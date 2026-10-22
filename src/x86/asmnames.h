#جے_بنا_تعریف ASMNAMES_H
#تعریف_کرو ASMNAMES_H

#تعریف_کرو C2(X, Y)  X ## Y
#تعریف_کرو C1(X, Y)  C2(X, Y)
#جے_تعریف_شدہ __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#وإلا
# define C(X)     X
#جے_انت

#جے_تعریف_شدہ __APPLE__
# define L(X)     C1(L, X)
#وإلا
# define L(X)     C1(.L, X)
#جے_انت

#إذا defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#وإلا
# define PLT(X)	  X
#جے_انت

#جے_تعریف_شدہ __ELF__
# define ENDF(X)  .type	X,@function; .ناپ X, . - X
#وإلا
# define ENDF(X)
#جے_انت

#جے_انت /* ASMNAMES_H */
