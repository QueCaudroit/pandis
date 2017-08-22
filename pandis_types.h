#ifndef PANDIS_TYPES
#define PANDIS_TYPES 1

typedef struct matrix matrix;
struct matrix{
  long m;
  long n;
  long r_n;
  double* data;
};

typedef struct mult_args mult_args;
struct mult_args{
  matrix a,b,c;
};

#endif
