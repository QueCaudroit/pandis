#include <ruby.h>
#include <pthread.h>
#include "matmul.h"
#include "matrix.h"
#include "tiled_matmul.h"

extern VALUE Matrix;
extern int core_number;

struct matmul_line_arg{
  long i;
  long n;
  long k_max;
  double *data_a;
  double *data_b;
  double *data_c;
  pthread_t thread;
};
typedef struct matmul_line_arg matmul_line_arg;

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
  long i, b;
  b = 0;
  matmul_line_arg *args = (matmul_line_arg*) malloc(sizeof(matmul_line_arg)*m);
  for(i=0; i<m*n*TILE_LENGTH; i++){
    data_c[i] = 0;
  }
  for(i=0;i<m;i++){
    args[i].n = n;
    args[i].k_max = k_max;
    args[i].data_a = data_a;
    args[i].data_b = data_b;
    args[i].data_c = data_c;
    args[i].i = i;
    b = pthread_create(&(args[i].thread), NULL, matmul_line, &(args[i]));
    if(b){
      break;
    }
  }
  for(i--;i>=0;i--){
    pthread_join(args[i].thread, NULL);
  }
  free(args);
  if(b){
    rb_raise(rb_eThreadError, "Unable to create thread");
  }
  return Data_Wrap_Struct(Matrix, NULL, free_matrix, c);
}

void *matmul_line(void *arg){
  matmul_line_arg *args;
  args = (matmul_line_arg*) arg;
  long i,j,k,n,k_max;
  double *temp_a, *temp_b, *temp_c, *data_a, *data_b, *data_c;
  i = args->i;
  n = args->n;
  k_max = args->k_max;
  data_a = args->data_a;
  data_b = args->data_b;
  data_c = args->data_c;
  for(j=0;j<n;j++){
    for(k=0;k<k_max;k++){
      temp_a = data_a + i * k_max + k * TILE_LENGTH;
      temp_b = data_b + k * n + j * TILE_LENGTH;
      temp_c = data_c + i * n + j * TILE_LENGTH;
      TILE_MATMUL(temp_a,temp_b,temp_c)
    }
  }
  pthread_exit(NULL);
}

