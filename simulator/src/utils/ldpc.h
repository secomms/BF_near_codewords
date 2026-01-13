#ifndef LDPC_H
#define LDPC_H

#include "../include/bitarray.h"

int compute_support(unsigned long* support, uint8_t* arr, size_t arr_len, int max_hw);

#endif