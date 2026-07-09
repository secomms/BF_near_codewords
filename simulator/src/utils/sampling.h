#ifndef SAMPLING_UTILS_H
#define SAMPLING_UTILS_H

#include <stddef.h>

/**
 * @brief Extracts a subset of unique locations in the range [0, universe_size-1]
 * This is a general purpose function used to generate all vector support
 *
 * @param[out] support       Output array where the extracted positions are stored.
 * @param[in]  set           Size of the universe to sample from.
 * @param[in]  n             Number of element to extract from the set.
 */
void sample_unique(unsigned long support[], size_t set, size_t n);

void sample_unique_bitmap(unsigned long support[], size_t set, size_t n);

/**
 * @brief "Near codeword" refers to a situation in error-correcting codes where a received message is close to a valid codeword, but not exactly a codeword.
 * This function is specific for FLOOR simulation, sample error which has exactly u intersection with a random column of H.
 *
 * @param[out] support       Output array where the position of the near codeword non empty element are stored.
 * @param[in] u              This is the weigth of the codeword.
 * @param[in] h1			Vector defining the first circulant of the parity check matrix
 * @param[in] h2			Vector defining the second circulant of the parity check matrix
 */
void sample_from_near_codeword(unsigned long *err_support, const int u, const unsigned long* h1, const unsigned long* h2);

#endif