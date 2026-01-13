#ifndef PARAMS_H
#define PARAMS_H

/**
 * @file params.h
 * @brief Global configuration parameters for DFR simulations and decoding experiments.
 *
 * This header defines all compile-time parameters used across the project
 * 
 * @note Modify these parameters to customize the simulation without touching the core code.
 * The file is meant to be edited frequently to adjust experimental conditions.
*/

// ======================================================================
// Generic Options
// ======================================================================
#define NUM_THREADS        15      ///< Number of threads used for simulation.
#define CHARS_FOR_LOOKUP   500    ///< Characters used for look-up table id
#define INITIAL_SEED       0      ///< Seed for random number generator (0 = system time).

#define NUM_DECODES_MAX    100 ///< Maximum number of decoding attempts per simulation.
#define NUM_ERRORS_MAX     40     ///< Maximum number of error patterns tested.
#define NUM_BACKUP         1000   ///< Frequency (in iterations) of result backups.
#define FOLDER_FOR_RESULTS 1      ///< ID/index of the folder where results will be stored.

// AGGIUNGERE LA DEFINE PER IL DEBUG LEVEL
#define DEBUG_LEVEL DEBUG_LEVEL_INFO

// ======================================================================
// Code Parameters
// ======================================================================
#define CODE_LENGTH        4006   ///< Codeword length (n).
#define CODE_REDUNDANCY    2003   ///< Number of redundant bits (n - k).
#define COLUMN_WEIGHT      15     ///< Column weight (w_c) of the parity-check matrix.


// ======================================================================
// Simulation Options
// ======================================================================
/**
 * @brief Type of simulation to be performed during DFR calculation.
 * This parameter controls the generation mode (sampling) 
 * of errors that will then be subjected to the decoding procedure.
 *
 * Possible value:
 * - 0: Random Simulation -> Errors are generated completely randomly according to a uniform distribution. This mode is useful for a general estimate of the DFR under
 *   standard noise conditions or to validate the overall correctness of the decoding algorithm.
 * - 1: Near Codeword Simulation ->  This mode allows you to analyze and estimate the **floor** of the DFR, i.e., the region where decoding failures are not due to 
 *   random noise but to intrinsic code structures
 *
 * @verbatim
 * - 0 -> random errors
 * - 1 -> errors from nearcodeword
 * @endverbatim 
 */
#define SIMULATION_TYPE 1 

// ---- Error weight configuration ----/
#define T_MIN   3    ///< Minimum error weight to simulate.
#define T_MAX   250  ///< Maximum error weight to simulate.
#define T_STEP  1    ///< Step between consecutive simulated error weights.

// ---- Parameters for “near-codeword” exploration ----
#define ERROR_WEIGHT 50 ///< Specific error weight for single-weight simulations.

#define U_MIN   2    ///< Minimum value of parameter U (if applicable).
#define U_MAX   5   ///< Maximum value of parameter U.
#define U_STEP  1    ///< Step size for parameter U exploration.

// ======================================================================
// Decoder Configuration
// ======================================================================
/**
 * @brief Decoder to test.
 *
 * Possible value:
 * - 0: BF-Max  
 * - 1: Majority Logic  
 * - 2: BF out-of-place  
 * - 3: BGF  
 * - 4: New BIKE decoder  
 */
#define DECODER_TYPE 2


#endif
