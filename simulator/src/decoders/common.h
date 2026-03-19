#ifndef DECODER_COMMON_H
#define DECODER_COMMON_H

#include "../../params.h"
#include "../include/simulation.h"
#include "../include/bitarray.h"
#include "../utils/ldpc.h"
#include "../utils/lookup.h"

#include <stdlib.h>
#include <stdio.h>

// ADD A FUNCTION TO SET THE ERROR TO ZERO TO AVOID MISCONFIGURATION ON THE LENGTH


// GENERLIZE THIS FUNCTION WHEN THE ERROR VECTOR WILL BECOME A BITARRAY


static inline unsigned long cyclic_shift(unsigned long h, unsigned long i)
{
    // verificare per bene il tipo che si può utilizzare è uint32_t perchè unsigned long è un pò troppo
    unsigned long pos  = h + i;
    unsigned long mask = -(pos >= CODE_REDUNDANCY);
    return pos - (CODE_REDUNDANCY & mask);
}



/**
 * @brief Computes the Hamming weight of a bit array.
 *
 * Wrapper of popcount, the name `hamming_weight` is chosen for consistency with the terminology
 * used throughout the decoding algorithms.
 *
 * @param[in] arr Pointer to a byte array representing a bit vectorof length.
 * @return The number of bits set to 1 (Hamming weight) in the input bit array.
 */
static inline size_t hamming_weight(uint8_t* arr){
	return popcount_optimized(arr, BITS_TO_BYTES(CODE_REDUNDANCY));
}


// add documentation to this method
static inline void compute_counters(const unsigned long* hi, int* counters, uint8_t* syndrome, int offset){

	for(int i = 0; i<CODE_REDUNDANCY;i++){
		for(int j = 0; j<COLUMN_WEIGHT; j++){
			unsigned long pos = cyclic_shift(hi[j], i);
			counters[offset + i] += GET_BIT(syndrome, pos);
		}
	}
}

/**
 * @brief Update  the decoding structures with the residual weight of the syndrome
 * and prepare the error estimate for the solution verification phase.
 *
 * @param[in] hw 			Pointer to syndrome weight.
 * @param[in] flag 			Pointer to an integer indicating whether an improvement was found (1 = improved).
 * @param[in,out] params 	Pointer to the struct containing the info to update.
 */
static inline void decoding_outcome(size_t* hw, int* flag, DecodingParams* params){

    params->residual[0] = *hw;
	if (*flag == 0){
		memcpy(params->err_estimated_improved, params->err_estimated, CODE_LENGTH*sizeof(int));
		params->residual[1] = *hw;
	}			
}

/**
 * @brief Attempts to improve the decoding process by identifying and correcting known 
 * errors associated with trapping sets using a lookup table.
 *
 * This function is executed during the iterative decoding process to accelerate convergence or break deadlocks (loops) due to particular syndrome configurations
 * known as trapping sets. The operations are the following:
 * - Checks whether the current column weight (`*hw`) matches the column weight of the H matrix (`COLUMN_WEIGHT`).
 * - If the condition is met and the improvement flag is not already set, calculate the syndrome support.
 * - Generates an identifier string from the syndrome support and performs a lookup in a table containing the digests of known trapping sets.
 * - If a match is found: Sets the improvement flag (`*flag = 1`) 
 * - Copies the current error estimate to `err_estimated_improved`.
 * - Apply corrections to the estimated bits (bit-flips) using information from `H` and the found index.
 *  
 * @param[in]  H       	Pointer to the scruct containing all the Parity-Check Matrix info.
 * @param[in,out] data 	Pointer to the scruct with all the data required for the decoding process.
 * @param[in,out] flag 	Flag that indicates whether the lookup table has already been used
 * @param[in] hw   		Pointer to weigth of the syndrome
 */
static inline void decode_enhance_lookup(DecodingMatrix* H, DecodingParams* data, int* flag, size_t* hw){

	if((*hw == COLUMN_WEIGHT) && (*flag != 1)){
		
		unsigned long syndrome_support[COLUMN_WEIGHT];
	
		int num = 0;
		int w = compute_support(syndrome_support, data->syndrome, CODE_REDUNDANCY, COLUMN_WEIGHT);

		////////////////////////////////////////////////////////
		// Define a function to compute the index for the table
		////////////////////////////////////////////////////////
		char support_str[CHARS_FOR_LOOKUP];
		int n = 0;
		for (int u = 0; u < COLUMN_WEIGHT; u++) {
			n += sprintf (&support_str[n], "%lu", syndrome_support[u]);
			n += sprintf (&support_str[n], "-");
		}
		////////////////////////////////////////////////////////
		// Define a function to do the lookup in the table 
		// this function will return the index of the elemnet, if is -1 there is no element
		////////////////////////////////////////////////////////
		int found = 0; 
		int index = 0;
		while((found == 0)&&(index < CODE_LENGTH)){
			
			int cmp_res = strcmp(trapping_sets[index].digest, support_str);
			if(cmp_res == 0){
				found = 1;
			} else {
				index += 1;
			}
		}

		if (found == 1){
			data->residual[1] = 0;
			*flag = 1;
			memcpy(data->err_estimated_improved, data->err_estimated, CODE_LENGTH*sizeof(int));

			//flip bits
			if(index<CODE_REDUNDANCY){
				for(int j = 0; j < COLUMN_WEIGHT; j++){
					unsigned long pos_to_flip = (H->h1[j]+index)%CODE_REDUNDANCY;
					data->err_estimated_improved[pos_to_flip] ^= 1;
				}
			} else {
				for(int j = 0; j < COLUMN_WEIGHT; j++){
					unsigned long pos_to_flip = CODE_REDUNDANCY + (H->h2[j]+index)%CODE_REDUNDANCY;
					data->err_estimated_improved[pos_to_flip] ^= 1;
				}
			}
		}
		
	}

}


#endif