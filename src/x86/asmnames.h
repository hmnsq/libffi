#यदि_अपरिभाषितम् ASMNAMES_H
#परिभाषय ASMNAMES_H

#परिभाषय C2(X, Y)  X ## Y
#परिभाषय C1(X, Y)  C2(X, Y)
#यदि_परिभाषितम् __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#अन्यथा
# define C(X)     X
#यदि_अन्तः

#यदि_परिभाषितम् __APPLE__
# define L(X)     C1(L, X)
#अन्यथा
# define L(X)     C1(.L, X)
#यदि_अन्तः

#यदि defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#अन्यथा
# define PLT(X)	  X
#यदि_अन्तः

#यदि_परिभाषितम् __ELF__
# define ENDF(X)  .type	X,@function; .आकार X, . - X
#अन्यथा
# define ENDF(X)
#यदि_अन्तः

#यदि_अन्तः /* ASMNAMES_H */
