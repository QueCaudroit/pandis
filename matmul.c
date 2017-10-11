#include <ruby.h>
#include "matmul.h"
#include "matrix.h"
#include "tiled_matmul.h"

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
  *m = (*c)->m_tiles = a->m_tiles;
  *n = (*c)->n_tiles = b->n_tiles;
  (*c)->m = a->m;
  (*c)->n = b->n;
  *k_max = a->n_tiles;
  *data_a = a->data;
  *data_b = b->data;
  *data_c = (*c)->data = (double*) malloc(sizeof(double)*(*m)*(*n)*TILE_LENGTH);
}

VALUE matmul_tiled(VALUE self, VALUE other){  
  matrix *c;
  long m,n,k_max;
  double *data_a, *data_b, *data_c, *temp_a, *temp_b, *temp_c;
  matmul_setup(self, other, &c, &m, &n, &k_max, &data_a, &data_b, &data_c);
  long i,j,k,ii,jj,kk;
  for(i=0; i<m*n*TILE_LENGTH; i++){
    data_c[i] = 0;
  }
  for(i=0;i<m;i++){
    for(j=0;j<n;j++){
      for(k=0;k<k_max;k++){
        temp_a = data_a + (i*k_max+k) * TILE_LENGTH;
        temp_b = data_b + (k*m+j) * TILE_LENGTH;
        temp_c = data_c + (i*m+j) * TILE_LENGTH;
        TILE_MATMUL(temp_a,temp_b,temp_c)
      }
    }
  }
  return Data_Wrap_Struct(Matrix, NULL, free_matrix, c);
}

VALUE matmul_tiled_new(VALUE self, VALUE other){  
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
