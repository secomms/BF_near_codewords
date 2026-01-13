#ifndef LOGGING_H
#define LOGGING_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>


#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_GRAY    "\033[1;90m"



/**
 * @brief Prints a compact, aligned, and color-coded summary of all simulation parameters.
 *
 * This function displays a structured overview of the parameters used for the
 * current DFR simulation run. It organizes the configuration into clearly labeled
 * sections — *General*, *Logging*, *Code*, *Simulation*, and *Decoder* — each showing
 * the most relevant constants defined in `params.h`.
 */
void print_simulation_params();


void print_process_info(pid_t pid);


void print_array(const unsigned long* arr, size_t len);

void print_array_int(const int* arr, size_t len);

void print_bitarray(const uint8_t *arr, size_t n_bits);

void create_log_file();

void create_out_file();

/** 
 * @brief  Updates the simulation results on file and prints progress to stdout.
 *
 * @param t                     Error weight considered in the current simulation round.
 * @param num_errors            Number of decoding failures observed.
 * @param num_errors_improved   Number of decoding failures observed with the improved decoder.
 * @param num_decodes           Total number of decoding attempts performed so far.
 * @param elapsed               Total elapsed time (in seconds) for executing `num_decodes` attempts.
 */
void update_out_file_mp(const int t, const int num_errors, const int num_errors_improved, const int num_decodes, double elapsed);

void update_log_file_mp(const int t, const int num_errors, const int num_errors_improved, const int num_decodes, int num_tx);

#endif