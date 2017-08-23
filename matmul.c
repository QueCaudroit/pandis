#include <ruby.h>
#include "matmul.h"
#include "matrix.h"
#include "tiles.h"

#define TILE_SIZE 32

extern VALUE Matrix;
extern int core_number;

void matmul_setup(VALUE self, VALUE other, matrix** c, long* m, long* n, long* k_max, double** data_a, double** data_b, double** data_c){
  matrix *a, *b;
  if(rb_obj_is_instance_of(other, Matrix) != Qtrue){
    rb_raise(rb_eArgError, "Both must have type Matrix");
  }
  Data_Get_Struct(self, matrix, a);
  Data_Get_Struct(other, matrix, b);
  if(a->n != b->m){
    rb_raise(rb_eArgError, "Inconsistent dimensions");
  }
  *c = (matrix*) malloc(sizeof(matrix));
  *m = (*c)->m = a->m;
  *n = (*c)->n = b->n;
  *k_max = a->n;
  *data_a = a->data;
  *data_b = b->data;
  *data_c = (*c)->data = (double*) malloc(sizeof(double)*(*m)*(*n));
}

VALUE matmul_tiled(VALUE self, VALUE other){  
  matrix *c;
  long m,n,k_max;
  double *data_a, *data_b, *data_c;
  matmul_setup(self, other, &c, &m, &n, &k_max, &data_a, &data_b, &data_c);
  long i,j,k,ii,jj,kk;
  for(i=0; i<m; i++){
    for(j=0; j<n; j++){
      data_c[n*i+j] = 0;
    }
  }
  for(ii=0;ii<m;ii+=TILE_SIZE){
    for(jj=0;jj<n;jj+=TILE_SIZE){
      for(kk=0;kk<k_max;kk+=TILE_SIZE){
        for(i=ii; i-ii<TILE_SIZE && i<m; i++){
          for(j=jj; j-jj<TILE_SIZE && j<n; j++){
            for(k=kk; k-kk<TILE_SIZE && k<k_max; k++){
              data_c[n*i+j] += data_a[k_max*i+k] * data_b[n*k+j];
            }
          }
        }
      }
    }
  }
  return Data_Wrap_Struct(Matrix, NULL, free_matrix, c);
}


