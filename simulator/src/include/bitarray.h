#ifndef BIT_H
#define BIT_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <x86intrin.h>

/*
**************************************************************************************************************
This module contains the definition of all the macros that will be used to perform operation on bit array
**************************************************************************************************************
*/

/**
 * @brief Converts a number of bits into the minimum number of bytes required.
 * Converts a length expressed in bits into the corresponding 
 * number of bytes needed to represent it, always rounding up 
 * if the length in bits is not a multiple of 8.
 *
 * @note The operation `(bits + 7) / 8` is a common trick to avoid
 *       using functions such as `ceil()`, ensuring rounding
 *       up when the bits are not multiples of 8.
 * 
 * @param x Lenght in bits.
 * @return Number of bytes needed.
 */
#define BITS_TO_BYTES(x) (((x)+7)/8)

/**
 * @brief Returns the number of bits added as padding in order to obtain a length that is a multiple of 8.
 *
 * @param x Lenght in bits.
 * @return Number of bits used as padding.
 */
#define PADDING_BITS(x) (BITS_TO_BYTES(x)*8 - (x))

/**
 * @brief Set the bit in position i inside the byte array to 1.
 *
 * This macro modifies a byte array by treating it as a bit vector, 
 * setting the bit corresponding to the specified position to 1.
 * 
 * @note The index of the bit is automatically converted to byte 
 *       position and bit offset using the operation `(1U << (i % 8))` 
 *       applied to the element `arr[i / 8]`.
 * 
 * @param arr   Pointer of the byte array.
 * @param i     Index of the bit to set (0 = first element).
 */
#define SET_BIT(arr, i) ((arr)[(i) / 8] |= (1U << ((i) % 8)))

/**
 * @brief Get the value of the bit in position i inside the byte array.
 * @note The operation identifies the byte corresponding to the requested
 *       bit, moves it so that the bit of interest is in the LSB position 
 *       (so it's value is 0 or 1) and applies a mask to extract its value.
 * 
 * @param arr   Pointer of the byte array.
 * @param i     Index of the bit which we want to know the value.
 * @return The velue of the bit, so 0 or 1.
 */
#define GET_BIT(array, i)  ( ((array[(i) / 8] >> ((i) % 8)) & 1U) )

/**
 * @brief Toggle the value of a bit in the specified position.
 * @note The index of the bit is automatically converted to byte 
 *       position and bit offset using the operation already defined. 
 *       But in this case the bitwise operator used is the XOR.
 * 
 * @param arr   Pointer of the byte array.
 * @param i     Index of the bit to toggle
 */
#define TOGGLE_BIT(arr, i) ((arr)[(i) / 8] ^= (1U << ((i) % 8)))
#define FLIP_BIT(arr, i) ((arr)[(i) / 8] ^= (1U << ((i) % 8)))

/** 
 * @brief Function to allocate a bit array 
 * @note To avoid unwanted behavior due to bit array padding, the memory is immediately set to 0.
 * The only parameter to pass is the length of the vector, the padding is handled automatically.
 * 
 * @param[in] nbits      Vector length
 * @param[out] bit_array    Pointer to the memory area of the array
 */
static inline uint8_t* alloc_bitarray(size_t nbits) {
    uint8_t *bit_array = (uint8_t*) malloc(BITS_TO_BYTES(nbits));
    if (bit_array != NULL) memset(bit_array, 0, BITS_TO_BYTES(nbits)); 
    return bit_array;
}

/**
 * @brief Sets all bits in the array to zero 
 * 
 * @param arr      Pointer to the bitarray.
 * @param nbits    Length of the array.
 */
static inline void zero_bitarray(uint8_t *arr, size_t nbits) {
    memset(arr, 0, BITS_TO_BYTES(nbits));
}

/**
 * @brief Starting from a bitarray generates another array of the same size and
 * return the pointer  
 * 
 * @param arr      Pointer to the bitarray.
 * @param nbits    Length of the array.
 */
static inline uint8_t* clone_bitarray(uint8_t* src, size_t nbits){
    uint8_t* dst = alloc_bitarray(nbits);
    memcpy(dst, src, BITS_TO_BYTES(nbits));
    return dst;
}

/** 
 * @brief Counts the total number of bits set to 1 in a byte array.

 * This function scans an array of bytes, interpreting it as a bit vector
 * and returns the total number of bits set to 1. It uses the built-in 
 * operation `__builtin_popcount` to optimize the calculation.
 * Instead of acting on one bit at a time, it acts on the entire byte.
 *
 * @note It is essential to ensure that any padding bits
 *       (added to round the length of the bit array to a multiple of 8)
 *       have been cleared before the call. Otherwise,
 *       the function will also include these bits in the count, returning
 *       an incorrect result.
 *
 * @param[in] arr       Pointer to the byte array to analyze.
 * @param[in] nbytes    Number of bytes of the array.
 * @return Total number of bit sets to 1.
 */
static inline size_t popcount(uint8_t* arr, size_t nbytes){
    int tot = 0;
    for (size_t i = 0; i < nbytes; i++){ 
        tot += __builtin_popcount(arr[i]);
    }
    return tot;
}

/** 
 * @brief Counts the total number of bits set to 1 in a byte array.
 *
 * For each 8-byte block of the array, a single popcount is performed using 
 * optimized hardware instructions,  thus reducing the number of calls and 
 * increasing performance compared to the version that use byte.
 * 
 * @note The popcountll approach works on 64-bit blocks simultaneously, 
 *       reducing cycle and function call overhead. Therefore, we need to 
 *       consider the case where the “tail” of the array is not a multiple of 8.
 *
 * @param[in] arr       Pointer to the byte array to analyze.
 * @param[in] nbytes    Number of bytes of the array.
 * @return Total number of bit sets to 1.
 */
static inline size_t popcount_optimized(const uint8_t *array, size_t nbytes) {
    int total = 0;
    size_t i = 0;
    // optimized calculation
    for (; i + 8 <= nbytes; i += 8) {
        uint64_t *ptr = (uint64_t*)(array + i);
        total += __builtin_popcountll(*ptr);
    }
    // tail of the byte array 
    for (; i < nbytes; i++) {
        total += __builtin_popcount(array[i]);
    }
    return total;
}

/*
**************************************************************************************************************
End of the module
**************************************************************************************************************
*/

#endif