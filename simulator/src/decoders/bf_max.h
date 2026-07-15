#ifndef BF_MAX_H
#define BF_MAX_H

#include "../include/simulation.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

// DEFINE THE MULTIPLY VALUE FOR THE MAXIMUM NUMBER OF ITERATION, SO WE CAN PASS ONLY THE VALUE OF T AS A PARAMETER

/**
 * @brief Implementation of the decoding algorithm BF_MAX for QC-MDPC codes.
 * 
 * @param[in] H 			Structure containing all information about the QC-MDPC parity matrix (specifically `h1` and `h2`).
 * @param[in,out] params 	All information about the data to use for the decoding phase.
 * @param[in] num_iter_max 	Maximum number of iterations allowed for decoding BF_MAX.
 */
static inline void bf_max(DecodingMatrix* H, DecodingParams* params, const int num_iter_max){

	int counters[CODE_LENGTH];
	int lookup_used = 0, num_iter = 0;
	size_t hw = hamming_weight(params->syndrome);

    memset(params->err_estimated, 0, sizeof(int)*CODE_LENGTH);
    memset(counters, 0, sizeof(int)*CODE_LENGTH);
	
	compute_counters(H->h1, counters, params->syndrome, 0);
	compute_counters(H->h2, counters, params->syndrome, CODE_REDUNDANCY);

	while((num_iter < num_iter_max) && (hw!=0)){
		num_iter++;
		
		int max_counter = 0;
		int pos_max[CODE_LENGTH]; 
		int num_max = 0; 
		for(int i = 0; i<CODE_LENGTH; i++){
			if(counters[i]>max_counter){
				max_counter = counters[i];
				pos_max[0] = i;
				num_max = 1;
			}
			if(counters[i] == max_counter){
				pos_max[num_max] = i;
				num_max ++;
			}
		}

		int pos = rand()%num_max;
		int pos_flip = pos_max[pos];
		params->err_estimated[pos_flip] ^= 1;

		for(int i = 0; i<COLUMN_WEIGHT; i++){
			unsigned long row_index = 0;
			if (pos_flip < CODE_REDUNDANCY){
				row_index = (H->h1[i]+pos_flip)%CODE_REDUNDANCY;
			}else{
				row_index = (H->h2[i]+pos_flip)%CODE_REDUNDANCY;
			}

			FLIP_BIT(params->syndrome, row_index);

			int d = 1;
			if(GET_BIT(params->syndrome, row_index) == 0){ 
                d = -1;
            }
			//update all counters in which bit in position row_index participates
			for(int j = 0; j < 2*COLUMN_WEIGHT; j++){
				//update counters
				unsigned long ell = (H->H_first_row[j]/CODE_REDUNDANCY)*CODE_REDUNDANCY+(H->H_first_row[j]+row_index)%CODE_REDUNDANCY;
				counters[ell] += d;
			}
		}

        //at the the end of the iteration update the hamming weight
        hw = hamming_weight(params->syndrome);
		decode_enhance_lookup(H, params, &lookup_used, &hw);

	}
	decoding_outcome(&hw, &lookup_used, params);
}

#endif