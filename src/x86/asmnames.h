#اگر_غیر_تعریف_شدہ ASMNAMES_H
#تعریف_کرو ASMNAMES_H

#تعریف_کرو C2(X, Y)  X ## Y
#تعریف_کرو C1(X, Y)  C2(X, Y)
#اگر_تعریف_شدہ __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#ورنہ
# define C(X)     X
#اگر_ختم

#اگر_تعریف_شدہ __APPLE__
# define L(X)     C1(L, X)
#ورنہ
# define L(X)     C1(.L, X)
#اگر_ختم

#اگر defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#ورنہ
# define PLT(X)	  X
#اگر_ختم

#اگر_تعریف_شدہ __ELF__
# define ENDF(X)  .type	X,@function; .سائز X, . - X
#ورنہ
# define ENDF(X)
#اگر_ختم

#اگر_ختم /* ASMNAMES_H */
