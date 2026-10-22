#నిర్వచితం_కాకపోతే ASMNAMES_H
#నిర్వచించు ASMNAMES_H

#నిర్వచించు C2(X, Y)  X ## Y
#నిర్వచించు C1(X, Y)  C2(X, Y)
#నిర్వచితమైతే __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#లేకపోతే
# define C(X)     X
#అయితే_ముగింపు

#నిర్వచితమైతే __APPLE__
# define L(X)     C1(L, X)
#లేకపోతే
# define L(X)     C1(.L, X)
#అయితే_ముగింపు

#ఒకవేళ defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#లేకపోతే
# define PLT(X)	  X
#అయితే_ముగింపు

#నిర్వచితమైతే __ELF__
# define ENDF(X)  .type	X,@function; .కొలత X, . - X
#లేకపోతే
# define ENDF(X)
#అయితే_ముగింపు

#అయితే_ముగింపు /* ASMNAMES_H */
