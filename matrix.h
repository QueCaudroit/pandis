#include "pandis_types.h"

#ifndef PANDIS_MATRIX
#define PANDIS_MATRIX

void free_matrix(matrix* data);
VALUE allocate_matrix(VALUE self);
VALUE initialize_matrix(VALUE self, VALUE arr);

VALUE to_a_matrix(VALUE self);

#endif
