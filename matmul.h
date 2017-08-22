#include "pandis_types.h"

#ifndef PANDIS_MATMUL
#define PANDIS_MATMUL 1

VALUE matmul(VALUE self, VALUE b);
VALUE matmul_old(VALUE self, VALUE b);
VALUE matmul_tiled(VALUE self, VALUE b);
void matmul_setup(VALUE self, VALUE other, matrix** c, long* m, long* n, long* k_max, double** data_a, double** data_b, double** data_c);
void *base_matmul(void* argv);
void *fill_null(void* argv);

#endif
