#ifndef QUASI_CYCLIC_H
#define QUASI_CYCLIC_H

#include "../include/bitarray.h"
#include "../../params.h"

/**
 * @brief Populate the support, by randomly sampling, of the columns of the circulating stocks
 * 
 * @param[in] h1			Vector defining the first circulant of the parity check matrix
 * @param[in] h2			Vector defining the second circulant of the parity check matrix
 * @param[out] H_first_row  First row of the parity check matrix
 */
void QC_generate_parity_supports(unsigned long* h1, unsigned long* h2, unsigned long* H_first_row);

/**
 * @brief Given the first column of each circulant compute the first row of the parity check matrix
 *
 * @param[in] h1			Vector defining the first circulant of the parity check matrix
 * @param[in] h2			Vector defining the second circulant of the parity check matrix
 * @param[out] H_first_row  First row of the parity check matrix
 */
void QC_compute_firstrow(unsigned long* h1, unsigned long* h2, unsigned long* H_first_row);

/** 
 * @brief This function calculates the syndrome associated with a sparse error vector, represented by its support, exploiting the particular quasi-cyclic structure of the code parity matrix.
 * This function is the decoding process.
 *
 * @param[in] h1			Vector defining the first circulant of the parity check matrix
 * @param[in] h2			Vector defining the second circulant of the parity check matrix
 * @param[in] err_support	Array containing the positions of non-null elements in the error vector
 * @param[in] t             Error Weigth
 * @param[out] syndrome 	Is the result of the decoding process, the function populate this vector
 */
void QC_compute_syndrome(const unsigned long* h1, const unsigned long* h2, const unsigned long *err_support, const int err_weight, uint8_t* syndrome);

#endif