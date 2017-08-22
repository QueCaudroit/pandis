#include <ruby.h>
#include "matmul.h"
#include <pthread.h>
#include <semaphore.h>
#include "matrix.h"

#define TILE_SIZE 32

extern VALUE Matrix;
extern int core_number;
sem_t core_idle;
sem_t received_args;

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

VALUE matmul(VALUE self, VALUE other){
  matrix *c;
  long m,n,k_max;
  double *data_a, *data_b, *data_c;
  matmul_setup(self, other, &c, &m, &n, &k_max, &data_a, &data_b, &data_c);
  long n_tile = n/TILE_SIZE;
  long m_tile = m/TILE_SIZE;
  long i,j,k;
  matrix args;
  mult_args argss;
  sem_init(&core_idle, 0, core_number);
  sem_init(&received_args, 0, 1);
  pthread_t* threads = malloc(sizeof(pthread_t)*(n_tile+1)*(m_tile+1));
  for(i=0;i<=m_tile;i++){
    for(j=0;j<=n_tile;j++){
      sem_wait(&received_args);
      if(i!=m_tile){
        args.m = TILE_SIZE;
      }else{
        args.m = m%TILE_SIZE;
      }
      if(j!=n_tile){
        args.n = TILE_SIZE;
      }else{
        args.n = n%TILE_SIZE;
      }
      args.r_n = n;
      args.data = data_c + n*i*TILE_SIZE + j*TILE_SIZE;
      sem_wait(&core_idle);
      if(pthread_create(threads+i*(n_tile+1)+j, NULL, fill_null, &args)){
        rb_raise(rb_eSystemCallError, "Unable to create thread");
      }
    }
  }
  for(i=0;i<(m_tile+1)*(n_tile+1);i++){
    if(pthread_join(threads[i], NULL)){
      rb_raise(rb_eSystemCallError, "Thread ended badly");
    }
  }
  free(threads);
  long k_tile = k_max/TILE_SIZE;
  threads = malloc(sizeof(threads)*(m_tile+1)*(n_tile+1)*(k_tile+1));
  for(i=0;i<=m_tile;i++){
    for(j=0;j<=n_tile;j++){
      for(k=0;k<=k_tile;k++){
        sem_wait(&received_args);
        if(i!=m_tile){
          argss.a.m = argss.c.m = TILE_SIZE;
        }else{
          argss.a.m = argss.c.m = m%TILE_SIZE;
        }
        if(j!=n_tile){
          argss.b.n = argss.c.n = TILE_SIZE;
        }else{
          argss.b.n = argss.c.n = n%TILE_SIZE;
        }
        if(k!=k_tile){
          argss.a.n = TILE_SIZE;
        }else{
          argss.a.n = k_max%TILE_SIZE;
        }
        argss.a.r_n = k_max;
        argss.c.r_n = argss.b.r_n = n;
        argss.c.data = data_c + n*i*TILE_SIZE + j*TILE_SIZE;
        argss.a.data = data_a + k_max*i*TILE_SIZE + k*TILE_SIZE;
        argss.b.data = data_b + n*k*TILE_SIZE + j*TILE_SIZE;
        sem_wait(&core_idle);
        if(pthread_create(threads+k*(n_tile+1)*(m_tile+1)+i*(k_tile+1)+j, NULL, base_matmul, &args)){
          rb_raise(rb_eSystemCallError, "Unable to create thread");
        }
      }
    }
  }
  free(threads);
  for(i=0;i<(n_tile+1)*(m_tile+1)*(k_tile+1);i++){
    if(pthread_join(threads[i], NULL)){
      rb_raise(rb_eSystemCallError, "Thread ended badly");
    }
  }
  return Data_Wrap_Struct(Matrix, NULL, free_matrix, c);
}

void *base_matmul(void* argv){
  mult_args args = *((mult_args*) argv);
  sem_post(&received_args);
  long i,j,k;
  long m = args.a.m;
  long n = args.b.n;
  long k_max = args.a.n;
  double* data_a = args.a.data;
  double* data_b = args.b.data;
  double* data_c = args.c.data;
  long r_n_a = args.a.r_n;
  long r_n_b = args.b.r_n;
  long r_n_c = args.c.r_n;
  for(i=0; i<m; i++){
    for(j=0; j<n; j++){
      for(k=0; k<k_max; k++){
        data_c[r_n_c*i+j] += data_a[r_n_a*i+k] * data_b[r_n_b*k+j];
      }
    }
  }
  sem_post(&core_idle);
}

void *fill_null(void* argv){
  matrix mat = *((matrix*) argv);
  sem_post(&received_args);
  long i,j;
  for(i=0; i<mat.m; i++){
    for(j=0; j<mat.n; j++){
      mat.data[mat.r_n*i+j]=0;
    }
  }
  sem_post(&core_idle);
}

VALUE matmul_old(VALUE self, VALUE other){  
  matrix *c;
  long m,n,k_max;
  double *data_a, *data_b, *data_c;
  matmul_setup(self, other, &c, &m, &n, &k_max, &data_a, &data_b, &data_c);
  long i,j,k;
  for(i=0; i<m; i++){
    for(j=0; j<n; j++){
      data_c[n*i+j] = 0;
      for(k=0; k<k_max; k++){
        data_c[n*i+j] += data_a[k_max*i+k] * data_b[n*k+j];
      }
    }
  }
  return Data_Wrap_Struct(Matrix, NULL, free_matrix, c);
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


