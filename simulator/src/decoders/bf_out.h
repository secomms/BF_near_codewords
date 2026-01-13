#ifndef BF_OUT_H
#define BF_OUT_H

#include "common.h"
#include <stdlib.h>
#include <string.h>

#define NUM_ITERATIONS_OUT 15

/**
 * @brief Threshold values used by the BF_OUT decoding algorithm.
 *
 * The `thresholds_out` array defines the decision thresholds for each decoding 
 * iteration in the `bf_out` algorithm. At each iteration, a bit is flipped if 
 * the number of unsatisfied parity checks it participates in (the counter) is 
 * greater than or equal to the corresponding threshold value.
 */
const int thresholds_out[] = {13, 12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 8, 7, 7,7};

/**
 * @brief Bit-Flipping decoding algorithm with iteration-specific thresholds (BF_OUT).
 *
 * The `bf_out` function implements a variation of the Bit-Flipping decoding 
 * algorithm for QC-MDPC codes. Instead of a single fixed threshold, this method 
 * applies a sequence of iteration-specific thresholds defined in `thresholds_out`
 *
 * @param[in] H             Pointer to a structure containing the QC-MDPC parity-check matrix .
 * @param[in,out] params    Pointer to a structure holding all the decoding parameters.
 */
static inline void bf_out(DecodingMatrix* H, DecodingParams* params){
    
	int lookup_used = 0, num_iter = 0;
    size_t ws = 1;

	// TODO: change error estimate to bitarray
    memset(params->err_estimated, 0, CODE_LENGTH*sizeof(int));
    uint8_t* c_syndrome = alloc_bitarray(CODE_REDUNDANCY);
    
    while((num_iter < NUM_ITERATIONS_OUT)&(ws!=0)){
        
        memcpy(c_syndrome, params->syndrome, BITS_TO_BYTES(CODE_REDUNDANCY));
        // write a function to compute this counters for the first and second half 
        for(int i = 0; i<CODE_REDUNDANCY;i++){
            
            int counter = 0;
            // compute counter
            for(int j = 0; j<COLUMN_WEIGHT; j++){
                unsigned long pos = (H->h1[j]+i)%CODE_REDUNDANCY;
                counter += GET_BIT(c_syndrome, pos);
            }
            //flip if this above the threshold
            if (counter >= (thresholds_out[num_iter])){
                
			    // TODO: change error estimate to bitarray
                params->err_estimated[i] ^= 1;
                //update syndrome
                for(int j = 0; j<COLUMN_WEIGHT; j++){
                    unsigned long row_index = (H->h1[j]+i)%CODE_REDUNDANCY;
                    TOGGLE_BIT(params->syndrome, row_index);
                }
            }
        }    

        for(int i = 0; i<CODE_REDUNDANCY;i++){
            
            int counter = 0;

            for(int j = 0; j<COLUMN_WEIGHT; j++){
                unsigned long pos = (H->h2[j]+i)%CODE_REDUNDANCY;
                counter += GET_BIT(c_syndrome, pos);
            }
            //flip if this above the threshold
            if (counter >= (thresholds_out[num_iter])){
			    // TODO: change error estimate to bitarray
                params->err_estimated[CODE_REDUNDANCY + i] ^= 1; //flip error estimate
                //update syndrome
                for(int j = 0; j<COLUMN_WEIGHT; j++){
                    unsigned long row_index = (H->h2[j]+i)%CODE_REDUNDANCY;
                    TOGGLE_BIT(params->syndrome, row_index);
                }
            }

        }
        
        ws = hamming_weight(params->syndrome);
        decode_enhance_lookup(H, params, &lookup_used, &ws);
        num_iter++;
    }
    free(c_syndrome);
    // writing the result of the decoding for classical and improved 
    decoding_outcome(&ws, &lookup_used, params);
}

#endif