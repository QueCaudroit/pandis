#include <ruby.h>
#include <stdio.h>
#include <unistd.h>
#include "matrix.h"
#include "matmul.h"

VALUE Matrix;
VALUE Pandis;
int core_number;

void Init_pandis();

void Init_pandis(){
  printf("hello world\n");
  core_number = sysconf(_SC_NPROCESSORS_ONLN);
  printf("%d cores available\n", core_number);
  Pandis = rb_define_module("Pandis");
  Matrix = rb_define_class_under(Pandis, "Matrix", rb_cObject);
  rb_define_alloc_func(Matrix, allocate_matrix); 
  rb_define_method(Matrix, "initialize", initialize_matrix, 1);
  rb_define_method(Matrix, "to_a", to_a_matrix, 0);
  rb_define_method(Matrix, "*", matmul_tiled, 1);
  rb_define_method(Matrix, "old_matmul", matmul_old, 1);

}




