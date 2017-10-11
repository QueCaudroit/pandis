#include <ruby.h>
#include <stdio.h>
#include "matrix.h"
#include "tiled_matmul.h"

#define MAX_DISPLAY 10

extern VALUE Matrix;
extern VALUE Pandis;

void free_matrix(matrix* data){
  free(data->data);
  free(data);
}

VALUE allocate_matrix(VALUE klass){
  matrix* data = malloc(sizeof(matrix));
  if(data == NULL){
    rb_raise(rb_eSystemCallError, "Could not allocate memory for matrix");
  }
  return Data_Wrap_Struct(klass, NULL, free_matrix, data);
}

VALUE initialize_matrix(VALUE self, VALUE val){
  rb_check_type(val, T_ARRAY);
  long m = rb_array_len(val);
  if(m == 0){ rb_raise(rb_eArgError, "Matrix dimensions must be at least 1x1"); }
  const VALUE* ptr = rb_array_const_ptr(val);
  rb_check_type(ptr[0], T_ARRAY);
  long n = rb_array_len(ptr[0]);
  if(n == 0){ rb_raise(rb_eArgError, "Matrix dimensions must be at least 1x1"); }
  
  matrix* mat;
  Data_Get_Struct(self, matrix, mat);
  long m_tiles = (m + TILE_SIZE - 1)/TILE_SIZE;
  long n_tiles = (n + TILE_SIZE - 1)/TILE_SIZE;
  mat->data = malloc(sizeof(double) * m_tiles * n_tiles * TILE_LENGTH);
  if(mat->data == NULL){
    rb_raise(rb_eSystemCallError, "Could not allocate memory for matrix data");
  }

  long i, j, i_tile, j_tile, i_in_tile, j_in_tile;
  VALUE* ptr2;
  VALUE v;
  for(i=0; i<m_tiles*TILE_SIZE; i++){
    for(j=0; j<n_tiles*TILE_SIZE; j++){
      mat->data[i*m_tiles*TILE_SIZE+j] = 0;
    }
  }
  for(i=0; i<m; i++){
    rb_check_type(ptr[i], T_ARRAY);
    if(rb_array_len(ptr[i]) != n){ rb_raise(rb_eArgError, "Matrix rows length aren't consistent"); }
    ptr2 = rb_array_const_ptr(ptr[i]);
    for(j=0; j<n; j++){
      v = ptr2[j];
      switch(rb_type(v)){
        case T_FIXNUM:
        case T_FLOAT:
        case T_BIGNUM:
          i_tile = i/TILE_SIZE;
          i_in_tile = i%TILE_SIZE;
          j_tile = j/TILE_SIZE;
          j_in_tile = j%TILE_SIZE;
          mat->data[(n_tiles*i_tile+j_tile)*TILE_LENGTH +i_in_tile*TILE_SIZE+j_in_tile] = rb_num2dbl(v);
          break;
        default:
          rb_raise(rb_eArgError, "Matrix elements must all be numbers");
      }
    }
  }
  mat->m_tiles = m_tiles;
  mat->n_tiles = n_tiles;
  mat->m = m;
  mat->n = n;
  return self;
}

VALUE to_a_matrix(VALUE self){
  matrix* mat;
  Data_Get_Struct(self,matrix,mat);
  int size = 0;
  long m = mat->m;
  long n = mat->n;
  long n_tiles = mat->n_tiles;
  long m_tiles = mat->m_tiles;

  VALUE* rows = malloc(sizeof(VALUE)*m);
  VALUE* temp = malloc(sizeof(VALUE)*n);
  double* data = mat->data;
  long i,j,i_tile,j_tile,i_in_tile,j_in_tile;
  for(i=0; i<m && i<MAX_DISPLAY;i++){
    for(j=0; j<n && j<MAX_DISPLAY; j++){
      i_tile = i/TILE_SIZE;
      i_in_tile = i%TILE_SIZE;
      j_tile = j/TILE_SIZE;
      j_in_tile = j%TILE_SIZE;
      temp[j] = rb_float_new(data[(n_tiles*i_tile+j_tile)*TILE_LENGTH +i_in_tile*TILE_SIZE+j_in_tile]);
    }
    rows[i] = rb_ary_new_from_values(n, temp);
  }
  return rb_ary_new_from_values(m, rows);
}
