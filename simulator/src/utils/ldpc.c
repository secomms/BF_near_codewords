#include "ldpc.h"

int compute_support(unsigned long* support, uint8_t* arr, size_t arr_len, int max_hw){
	int count = 0;
    for (int i = 0; i < arr_len; i++) {
        if (GET_BIT(arr, i) == 1) {
            support[count++] = i;
        }
    }
    return count;
}
