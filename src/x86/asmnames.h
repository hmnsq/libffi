#جيڪڏهن_وصف_نه_ٿيل ASMNAMES_H
#وصف_ڪريو ASMNAMES_H

#وصف_ڪريو C2(X, Y)  X ## Y
#وصف_ڪريو C1(X, Y)  C2(X, Y)
#جيڪڏهن_وصف_ٿيل __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#وإلا
# define C(X)     X
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __APPLE__
# define L(X)     C1(L, X)
#وإلا
# define L(X)     C1(.L, X)
#جيڪڏهن_پورو

#إذا defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#وإلا
# define PLT(X)	  X
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __ELF__
# define ENDF(X)  .type	X,@function; .سائيز X, . - X
#وإلا
# define ENDF(X)
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* ASMNAMES_H */
