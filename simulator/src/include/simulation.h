#ifndef SIMULATION_H
#define SIMULATION_H

#include "bitarray.h"
#include "../../params.h"
#include <cstdint>
#include <stdint.h>
#include <stdlib.h>

/*
**************************************************************************************************************
This module contains the definition of all the macros that will be used to perform the simulation.
Those are defined in this modulo to make the code more readable.
**************************************************************************************************************
*/

#define POSITION_T  uint32_t
#define DIGIT       uint64_t

typedef struct {
    POSITION_T h_0[V];          
    POSITION_T h_1[V];           
    POSITION_T H_first_row[N0*V];
} sk;

// modificare aggiungendo padding di allineamento a 32

typedef struct {
    DIGIT       s[P];
    DIGIT       e_hat[N0*P];
    DIGIT       e_hat_impv[N0*P];
    POSITION_T  e_supp[T]; //questo ci sta da cambiarlo in base al tipo di simulazione
    uint8_t     upc[N0*P];
} dec;





#if SIMULATION_TYPE == 1
    #define SIM_LOOP int t = ERROR_WEIGHT; \
                     for (int u = U_MAX; u >= U_MIN; u -= U_STEP)
    #define SAMPLE_ERROR(data, H) sample_from_near_codeword((data)->err_support, u, (H).h1, (H).h2);
    #define UPDATE_OUTPUT(num_errors, num_errors_improved, num_decodes, elapsed) update_out_file_mp(u, num_errors, num_errors_improved, num_decodes, elapsed);
    #define UPDATE_LOG(num_errors, num_erros_impoved, num_decodes, my_decode_id) update_log_file_mp(u, num_errors, num_errors_improved, num_decodes, my_decode_id);
#elif SIMULATION_TYPE == 0
    #define SIM_LOOP for (int t = T_MAX; t >= T_MIN; t -= T_STEP)
    #define SAMPLE_ERROR(data, H) sample_unique((data)->err_support, CODE_LENGTH, t);
    #define UPDATE_OUTPUT(num_errors, num_errors_improved, num_decodes, elapsed) update_out_file_mp(t, num_errors, num_errors_improved, num_decodes, elapsed);
    #define UPDATE_LOG(num_errors, num_erros_impoved, num_decodes, my_decode_id) update_log_file_mp(t, num_errors, num_errors_improved, num_decodes, my_decode_id);
#endif


/**
 * @struct DecodingParams
 * @brief Contains all the information about the Parity-Check Matrix to use during the decoding phase.
 */
typedef struct {
    unsigned long* h1;              /**< First Circulant */
    unsigned long* h2;              /**< Second Circulant */
    unsigned long* H_first_row;     /**< First row of the matrix */
} DecodingMatrix;


static inline void reset_dec(dec* d, int t){
    /* e_supp: azzera solo le posizioni usate nell'iterazione precedente, non T_MAX intere */
    
    memset(d->e_hat,      0, sizeof(d->e_hat));
    memset(d->e_hat_impv, 0, sizeof(d->e_hat_impv));
    memset(d->s,          0, sizeof(d->s));
    memset(d->upc,        0, sizeof(d->upc));

}



/**
 * @struct DecodingParams
 * @brief Contains all the temporary buffers required for the simulation.
 */
typedef struct {
    uint8_t* syndrome;              /**< Syndrome Vectore is treated as bitarray (size CODE_REDUNDANCY). */
    int* err_estimated;             /**< Output of the classical decoding process (size CODE_LENGTH). */
    int* err_estimated_improved;    /**< Output of the decoding process that uses the lookup table when it detects the presence of a trapping set. */
    unsigned long* err_support;     /**< Support for the error generated which should be used to verify whether decoding was successful. */
    size_t* residual;               /**< Residual Hamming weight of the two-error estimate. */
} DecodingParams;

static inline DecodingParams* alloc_decoding_context(int error_wg) {

    DecodingParams* d = (DecodingParams*) malloc(sizeof(DecodingParams));
    d->syndrome               = alloc_bitarray(CODE_REDUNDANCY);
    d->err_estimated          = (int*) malloc(sizeof(int)*CODE_LENGTH); // convertire anche questo in bit array per il momento lasciare cosi
    d->err_estimated_improved = (int*) malloc(sizeof(int)*CODE_LENGTH); // convertire anche questo in futuro
    d->err_support            = (unsigned long*) malloc(error_wg*sizeof(unsigned long));
    d->residual               = (size_t*) malloc(2*sizeof(size_t));

    return d;
}


static inline void reset_decoding_context(DecodingParams* data, int t){
    memset(data->syndrome, 0, BITS_TO_BYTES(CODE_REDUNDANCY));
    memset(data->err_estimated, 0, sizeof(int)*CODE_LENGTH);
    memset(data->err_estimated_improved, 0, sizeof(int)*CODE_LENGTH);
    memset(data->residual, 0, sizeof(size_t)*2);
    memset(data->err_support, 0, sizeof(unsigned long)*t);
}

static inline void free_decoding_context(DecodingParams *params) {
    if (!params) return;
    free(params->syndrome);
    free(params->err_estimated);
    free(params->err_estimated_improved);
    free(params->err_support);
    free(params->residual);
    free(params);

}

#endif