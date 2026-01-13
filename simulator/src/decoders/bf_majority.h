#ifndef BF_MAJORITY_H
#define BF_MAJORITY_H

#include "common.h"
#include <stdint.h>
#include <stdlib.h>

#define NUM_ITERATIONS_MAJORITY 50

/** 
 * @brief Executes a step of the half-majority decoding algorithm by updating
 * the error estimate and the syndrome based on the majority voting threshold.
 * 
 * The use of `copy_syndrome` is necessary because the original syndrome is updated
 * during the algorithm; copying ensures that the bit count is correct
 * without having to maintain a separate counter array.
 *
 * @param[in] hi 					Pointer to the array of indices used to calculate positions in the syndrome.
 * @param[in,out] error_estimate 	Array of estimated errors; will be updated.
 * @param[in,out] syndrome 			Current syndrome; will be updated with any bit flips.
 * @param[in] copy_syndrome 		Copy of the original syndrome for bit counting.
 * @param[in] offset 				Offset to add to the index when writing to `error_estimate`.
 *
 */
static inline void half_majority(const unsigned long* hi, int* error_estimate, uint8_t* syndrome, uint8_t* copy_syndrome, int offset){

	// iterate on the half of the length of the vector
	for(int i=0; i<CODE_REDUNDANCY; i++){

		int counter = 0;
		// to calculate the counter we use a copy of the syndorme becuase because the syndrome is updated at each counter
		// this avoid using an array of counters
		for(int j = 0; j<COLUMN_WEIGHT; j++){
			unsigned long pos = (hi[j]+i)%CODE_REDUNDANCY;
			counter += GET_BIT(copy_syndrome, pos);
		}
		
		//flip if this above the threshold
		if (counter > (COLUMN_WEIGHT/2)){
			// flip error estimate	
			// TODO: change error estimate to bitarray
			error_estimate[offset + i] ^= 1;

			for(int j = 0; j<COLUMN_WEIGHT; j++){
				unsigned long row_index = (hi[j]+i)%CODE_REDUNDANCY;
                TOGGLE_BIT(syndrome, row_index);
			}
		}
	}

}

/**
 * @brief Majority-logic decoding algorithm for QC-MDPC codes.
 *
 * This function implements the main loop of the majority-logic decoding algorithm
 * for moderately dense parity matrix quasi-cyclic codes
 * (QC-MDPC).
 *
 * @param[in] H 			Structure containing all information about the QC-MDPC parity matrix (specifically `h1` and `h2`).
 * @param[in,out] params 	All information about the data to use for the decoding phase.
 *
 */
static inline void bf_majority(DecodingMatrix* H, DecodingParams* params){
    	
	size_t ws = 1, num_iter = 0; 
	int lookup_used = 0;

    memset(params->err_estimated, 0, sizeof(int)*CODE_LENGTH);
    uint8_t* c_syndrome = alloc_bitarray(CODE_REDUNDANCY);

    while((num_iter < NUM_ITERATIONS_MAJORITY)&(ws!=0)){
		
		num_iter ++;
		
		memcpy(c_syndrome, params->syndrome, BITS_TO_BYTES(CODE_REDUNDANCY));
		half_majority(H->h1, params->err_estimated, params->syndrome, c_syndrome, 0);
		half_majority(H->h2, params->err_estimated, params->syndrome, c_syndrome, CODE_REDUNDANCY);
		
        // always update the syndrome before the plugin
        ws = hamming_weight(params->syndrome);
        decode_enhance_lookup(H, params, &lookup_used, &ws);
	}
	free(c_syndrome);
	decoding_outcome(&ws, &lookup_used, params);
}

#endif