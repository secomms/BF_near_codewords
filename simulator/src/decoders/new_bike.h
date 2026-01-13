#ifndef NEW_BIKE_DECODER_H
#define NEW_BIKE_DECODER_H

#include "common.h"
#include "math.h"

#define NUM_ITERATIONS_NEW_BIKE 7
#define DELTA_VAL 3
#define MUL_PARAMETER 0.006354868
#define SUM_PARAMETER 11.10143233

/**
 * @brief Auxiliary linear function used in threshold computation.
 *
 * This function defines a simple linear mapping used internally for 
 * threshold estimation in the new BIKE decoder.
 *
 * @param[in] x Input integer value, typically related to the weight (ws).
 * @return A floating-point value obtained from the linear mapping.
 */
static inline float f_t(int x){
    return MUL_PARAMETER*(float)x+SUM_PARAMETER;
}

/**
 * @brief Function for computing threshold for new bike decoder
 * This function calculates an adaptive threshold used in the decoding process.
 *
 * @param[in] initial_ws Initial weight (usually computed before decoding starts).
 * @param[in] ws Current weight (updated during decoding).
 * @param[in] num_iter Iteration index (starting from 0).
 * @return The computed threshold value as an integer. 
 */
static inline int new_bike_compute_threshold(int initial_ws, int ws, int num_iter){
    float T, T_prime = f_t(initial_ws);
    int M = (COLUMN_WEIGHT+1)/2;

    switch (num_iter) {
        case 0: T = T_prime + (float) DELTA_VAL; break;
        case 1: T = (2*T_prime + (float) M) / 3 + DELTA_VAL; break;
        case 2: T = (T_prime + 2*(float) M) / 3 + DELTA_VAL; break;
        default:T = (float)M + DELTA_VAL; break;
    }
    
    float new_T = f_t(ws);
    if (T<new_T) T = new_T;
    return ceil(T);
}

/**
 * @brief Bit-Flipping decoding algorithm.
 *
 * @param[in] H             Pointer to a structure containing the QC-MDPC parity-check matrix .
 * @param[in,out] params    Pointer to a structure holding all the decoding parameters.
 */
static inline void new_bike_decoder(DecodingMatrix* H, DecodingParams* data){
	
	int lookup_used = 0; 
    size_t ws = hamming_weight(data->syndrome);
    memset(data->err_estimated, 0, CODE_LENGTH*sizeof(int));

    int initial_ws = ws;
    
    //Start looping
	int num_iter = 0;
    while((num_iter < NUM_ITERATIONS_NEW_BIKE)&(ws!=0)){
        
        int T = new_bike_compute_threshold(initial_ws, ws, num_iter);
        uint8_t* c_syndrome = clone_bitarray(data->syndrome, CODE_REDUNDANCY);

        //Compute counters using copy_sydnrome
        for(int i = 0; i<CODE_REDUNDANCY;i++){
            
            //compute i-th counter
            int counter = 0;
            
            for(int j = 0; j<COLUMN_WEIGHT; j++){
                unsigned long pos = (H->h1[j]+i)%CODE_REDUNDANCY;
               counter+= GET_BIT(c_syndrome, pos);

            }
            //flip if this above the threshold
            if (counter >= T){
                data->err_estimated[i] ^= 1;
                //update syndrome
                for(int j = 0; j<COLUMN_WEIGHT; j++){
                    unsigned long row_index = (H->h1[j]+i)%CODE_REDUNDANCY;
                    FLIP_BIT(data->syndrome, row_index);
                }
            }

        }    
        
        //Counters for the second part
        for(int i = 0; i<CODE_REDUNDANCY;i++){
            
            //compute i-th counter
            int counter = 0;
            
            for(int j = 0; j<COLUMN_WEIGHT; j++){
                unsigned long pos = (H->h2[j]+i)%CODE_REDUNDANCY;
                counter += GET_BIT(c_syndrome, pos);
            }
            
            //flip if this above the threshold
            if (counter >= T){
               data->err_estimated[CODE_REDUNDANCY + i] ^= 1; 

                //update syndrome
                for(int j = 0; j<COLUMN_WEIGHT; j++){
                    unsigned long row_index = (H->h2[j]+i)%CODE_REDUNDANCY;
                    FLIP_BIT(data->syndrome, row_index);
                }
            }

        }
		
        free(c_syndrome);

        ws = hamming_weight(data->syndrome);
        decode_enhance_lookup(H, data, &lookup_used, &ws);
    	num_iter ++;
	}

    decoding_outcome(&ws, &lookup_used, data);

}

#endif
