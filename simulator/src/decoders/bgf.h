#ifndef BGF_DECODER_H
#define BGF_DECODER_H

#include "common.h"
#include <math.h>

#define NUM_ITERATIONS_BGF 5 
#define TAU_BGF 3 

/**
 * @brief Compute the dynamic threshold for the BGF decoding algorithm.
 *
 * The threshold is computed as a linear function of the current syndrome weight
 * (`hw`) to adaptively determine which bits are flipped during decoding.
 * A minimum threshold of 36 is enforced to avoid overly aggressive flipping.
 *
 * @param[in] hw The current Hamming weight of the syndrome.
 * @return The computed threshold value (at least 36).
 */
 static inline int compute_threshold(int hw){
    // convert the value in constat with a define
    int T = floor(0.0069722*(float)hw + 13.530);
    if (T < 36){
        T = 36;
    }
    return T;
}

/**
 * @brief Perform a single colored iteration of the BGF decoder.
 *
 * This function performs a decoding pass over the codeword, flipping bits
 * selectively based on a mask of "colored" positions (e.g., black or gray bits).
 *  
 * @param[in] H                     Pointer to the structure containing the QC-MDPC parity-check matrix.
 * @param[in,out] syndrome          The current syndrome; updated after bit flips.
 * @param[in,out] error_estimate    The current error estimate vector; updated after bit flips.
 * @param[in] is_colored            An integer mask marking positions to evaluate
 */
static inline void colored_bgf_iteration(DecodingMatrix* H, uint8_t* syndrome, int* error_estimate, const int* is_colored){
    
    uint8_t* c_syndrome = clone_bitarray(syndrome, CODE_REDUNDANCY);
    
    //Compute counters using copy_sydnrome; counters for the first part
    for(int i = 0; i<CODE_REDUNDANCY;i++){
        if(is_colored[i] == 1){
            
            //compute i-th counter
            int counter = 0;
            
            for(int j = 0; j<COLUMN_WEIGHT; j++){
                unsigned long pos = (H->h1[j]+i)%CODE_REDUNDANCY;
                counter += GET_BIT(c_syndrome, pos);
            }
        
                //flip if this above the threshold
                if (counter >= ((COLUMN_WEIGHT+1)/2+1)){
                    
                    error_estimate[i] ^= 1; //flip error estimate

                    for(int j = 0; j<COLUMN_WEIGHT; j++){
                        unsigned long row_index = (H->h1[j]+i)%CODE_REDUNDANCY;
                        TOGGLE_BIT(syndrome, row_index);
                    }
                }
                
            }    
    }
            
    //Counters for the second part
    for(int i = 0; i<CODE_REDUNDANCY;i++){
        if (is_colored[CODE_REDUNDANCY+i] == 1){
            //compute i-th counter
            int counter = 0;
            
            for(int j = 0; j<COLUMN_WEIGHT; j++){
                unsigned long pos = (H->h2[j]+i)%CODE_REDUNDANCY;
                counter+= GET_BIT(c_syndrome, pos);
            }
            
            //flip if this above the threshold
            if (counter >= ((COLUMN_WEIGHT+1)/2+1)){
                                
                error_estimate[CODE_REDUNDANCY + i] ^= 1; //flip error estimate

                //update syndrome
                for(int j = 0; j<COLUMN_WEIGHT; j++){
                    unsigned long row_index = (H->h2[j]+i)%CODE_REDUNDANCY;
                    TOGGLE_BIT(syndrome, row_index);
                }
            }
                        
        }
    }
            
    free(c_syndrome);
        
}

/**
 * @brief Bit-Flipping decoder with dynamic thresholds (BGF) from BIKE.
 *
 * This function implements the BGF (Black-Gray Flip) decoding algorithm used in BIKE
 * (Bit Flipping Key Encapsulation) cryptosystems. The decoder iteratively identifies
 * bits with high likelihood of error based on dynamic thresholds derived from the
 * current syndrome weight, and applies targeted corrections using "black" and "gray"
 * classifications.
 *
 * @param[in] H             Pointer to a structure containing the QC-MDPC parity-check matrix .
 * @param[in,out] params    Pointer to a structure holding all the decoding parameters.
 */
static inline void bgf(DecodingMatrix* H, DecodingParams* params){
	
	int lookup_used = 0; //(set to 1 if lookup is used)
    memset(params->err_estimated,0 , sizeof(int)*CODE_LENGTH);

///////////////////////////////////////////////////////////////////////////////////////////////
    //First iteration
    //gets syndrome weight
    size_t ws = hamming_weight(params->syndrome);	    
/////////////////////////////////////////////////////////////////////////////////////////////    
    //define mask for black bits
    int* is_black;
    is_black = (int*) malloc(CODE_LENGTH*sizeof(int));
    memset(is_black, 0, CODE_LENGTH*sizeof(int));
    
    //define mask for gray bits
    int* is_gray;
    is_gray = (int*) malloc(CODE_LENGTH*sizeof(int));
    memset(is_gray, 0, CODE_LENGTH*sizeof(int));
    
    //Start looping
	int num_iter = 0;
    while((num_iter < NUM_ITERATIONS_BGF)&(ws!=0)){
        
        //Get threshold
        int T = compute_threshold(ws);

        // use clone_bitarray 
        uint8_t* c_syndrome = alloc_bitarray(CODE_REDUNDANCY);
        memcpy(c_syndrome, params->syndrome, BITS_TO_BYTES(CODE_REDUNDANCY));
        //

        //Compute counters using copy_sydnrome
        //Counters for the first part
        for(int i = 0; i<CODE_REDUNDANCY;i++){
            
            //compute i-th counter
            int counter = 0;
            
            for(int j = 0; j<COLUMN_WEIGHT; j++){
                unsigned long pos = (H->h1[j]+i)%CODE_REDUNDANCY;
                counter+= GET_BIT(c_syndrome, pos);
            }
            
            //flip if this above the threshold
            if (counter >= T){
                
                //set the bit to black
                is_black[i] = 1;
                
                params->err_estimated[i] ^= 1; //flip error estimate

                //update syndrome
                for(int j = 0; j<COLUMN_WEIGHT; j++){
                    unsigned long row_index = (H->h1[j]+i)%CODE_REDUNDANCY;
                    TOGGLE_BIT(params->syndrome, row_index);
                }
            }
            
            //if coutner > T - TAU, set is_gray to 1
            if(counter > (T - TAU_BGF)){
                is_gray[i] = 1;
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
                
                //set the bit to black
                is_black[CODE_REDUNDANCY + i] = 1;
                
                params->err_estimated[CODE_REDUNDANCY + i] ^= 1; //flip error estimate

                //update syndrome
                for(int j = 0; j<COLUMN_WEIGHT; j++){
                    unsigned long row_index = (H->h2[j]+i)%CODE_REDUNDANCY;
                    TOGGLE_BIT(params->syndrome, row_index);
                }
            }
            
            //if coutner > T - TAU, set is_gray to 1
            if(counter > (T - TAU_BGF)){
                is_gray[CODE_REDUNDANCY + i] = 1;
            }
            
        }
		
        free(c_syndrome);
		
        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        //if first iteration, do additional stuff with black and gray
        if(num_iter == 0){
            colored_bgf_iteration(H, params->syndrome, params->err_estimated, is_black);
            colored_bgf_iteration(H, params->syndrome, params->err_estimated, is_gray);
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////

		//Recompute syndrome weight
    
		ws = 0; 
        ws = hamming_weight(params->syndrome);
		
        decode_enhance_lookup(H, params, &lookup_used, &ws);
		
    	num_iter ++;
	}

    free(is_black);
    free(is_gray);
    decoding_outcome(&ws, &lookup_used, params);
}

#endif