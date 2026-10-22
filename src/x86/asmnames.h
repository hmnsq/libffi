#अगर_अघोषित ASMNAMES_H
#घोषणा ASMNAMES_H

#घोषणा C2(X, Y)  X ## Y
#घोषणा C1(X, Y)  C2(X, Y)
#अगर_घोषित __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#अन्यथा
# define C(X)     X
#पूर्ण_अगर

#अगर_घोषित __APPLE__
# define L(X)     C1(L, X)
#अन्यथा
# define L(X)     C1(.L, X)
#पूर्ण_अगर

#अगर defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#अन्यथा
# define PLT(X)	  X
#पूर्ण_अगर

#अगर_घोषित __ELF__
# define ENDF(X)  .type	X,@function; .आकार X, . - X
#अन्यथा
# define ENDF(X)
#पूर्ण_अगर

#पूर्ण_अगर /* ASMNAMES_H */
