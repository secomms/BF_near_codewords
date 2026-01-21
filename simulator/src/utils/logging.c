#include "logging.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../../params.h"

#define SEPARATOR_WIDTH  80
#define SEPARATOR_COLOR  "\033[1;36m"



void print_styled_line(char style, const char *color, int width) {
    if (width <= 0) width = 80;
    if (color) printf("%s", color);
    for (int i = 0; i < width; i++)
        putchar(style);
    printf("\033[0m\n");
}

void print_centered_title(const char *title, const char *color, int width) {
    if (!title) return;
    if (width <= 0) width = 80;

    int len = (int)strlen(title);
    int padding = (width - len) / 2;

    if (padding < 0) padding = 0;

    if (color) printf("%s", color);
    for (int i = 0; i < padding; i++)
        putchar(' ');
    printf("%s", title);
    printf("\033[0m\n");
}



/** 
 * @brief Returns the string representation of the decoder type.
 * 
 * @return const char* Pointer to a string describing the decoder type.
 */
static inline char* get_decoder_type_str(void) {
    switch (DECODER_TYPE) {
        case 0: return "max";
        case 1: return "MLD";
        case 2: return "out";
        case 3: return "BGF";
        case 4: return "NEW BIKE";
    }
}


void print_process_info(pid_t pid) {
    print_styled_line('=', SEPARATOR_COLOR, SEPARATOR_WIDTH);
    print_centered_title("PROCESS INFORMATION", SEPARATOR_COLOR, 80);
    print_styled_line('=', SEPARATOR_COLOR, SEPARATOR_WIDTH);
    printf("%sProcess PID: %s%d \n", COLOR_GREEN, COLOR_RESET, pid);
    printf("%sDirectory for monitoring the process:%s /proc/%d\n",COLOR_GREEN, COLOR_RESET, pid);
    printf("%sLaunch this command in another terminal:\n%s",COLOR_GREEN, COLOR_RESET);
    printf("   watch -n 1 'grep -E \"VmPeak|VmSize|VmRSS|VmData|VmSwap\" /proc/%d/status'\n", pid);

}

/**
 * @brief Prints a compact, aligned, and color-coded summary of all simulation parameters.
 *
 * This function displays a structured overview of the parameters used for the
 * current DFR simulation run. It organizes the configuration into clearly labeled
 * sections — *General*, *Logging*, *Code*, *Simulation*, and *Decoder* — each showing
 * the most relevant constants defined in `params.h`.
 */
void print_simulation_params() {
    
    print_styled_line('=', SEPARATOR_COLOR, SEPARATOR_WIDTH);
    print_centered_title("SIMULATION PARAMETERS", SEPARATOR_COLOR, 80);
    print_styled_line('=', SEPARATOR_COLOR, SEPARATOR_WIDTH);

    // Header
    printf("\033[1;34m%-12s %-18s %-18s %-18s %-18s\033[0m\n", "Section", "Param 1", "Param 2", "Param 3", "Param 4");
    print_styled_line('-', "\033[1;34m", 80);

    // Buffers per formattare i campi
    char f1[32], f2[32], f3[32], f4[32];

    // [General]
    snprintf(f1, sizeof(f1), "Threads= %d", NUM_THREADS);
    snprintf(f2, sizeof(f2), "Lookup= %d", CHARS_FOR_LOOKUP);
    snprintf(f3, sizeof(f3), "Seed= %d", INITIAL_SEED);
    printf("\033[1;32m%-12s\033[0m %-18s %-18s %-18s %-18s\n", "[General]", f1, f2, f3, "");

    // [Logging]
    snprintf(f1, sizeof(f1), "Decodes= %d", NUM_DECODES_MAX);
    snprintf(f2, sizeof(f2), "Errors= %d", NUM_ERRORS_MAX);
    snprintf(f3, sizeof(f3), "Backup= %d", NUM_BACKUP);
    snprintf(f4, sizeof(f4), "Folder= %d", FOLDER_FOR_RESULTS);
    printf("\033[1;32m%-12s\033[0m %-18s %-18s %-18s %-18s\n", "[Logging]", f1, f2, f3, f4);

    // [Code]
    snprintf(f1, sizeof(f1), "n= %d", CODE_LENGTH);
    snprintf(f2, sizeof(f2), "r= %d", CODE_REDUNDANCY);
    snprintf(f3, sizeof(f3), "w_c= %d", COLUMN_WEIGHT);
    printf("\033[1;32m%-12s\033[0m %-18s %-18s %-18s %-18s\n", "[Code]", f1, f2, f3, "");

    // [Simulation]
    snprintf(f1, sizeof(f1), "Type= %d(%s)", SIMULATION_TYPE, (SIMULATION_TYPE == 0) ? "Random" : "Floor");
    snprintf(f2, sizeof(f2), "T= [%d:%d:%d]", T_MIN, T_MAX, T_STEP);
    snprintf(f3, sizeof(f3), "ErrW= %d", ERROR_WEIGHT);
    snprintf(f4, sizeof(f4), "U= [%d:%d:%d]", U_MIN, U_MAX, U_STEP);
    printf("\033[1;32m%-12s\033[0m %-18s %-18s %-18s %-18s\n", "[Sim]", f1, f2, f3, f4);


    snprintf(f1, sizeof(f1), "Type= %d(%s)", DECODER_TYPE, get_decoder_type_str());
    printf("\033[1;32m%-12s\033[0m %-18s %-18s %-18s %-18s\n", "[Decoder]", f1, "", "", "");
}


void create_out_file(){
    
	//Format: n_r_v_SEED    
    //create file name
    char *file_out = (char*)malloc(50 * sizeof(char));
    char *bf_type = (char*)malloc(10*sizeof(char));
	char *sim_type = (char*)malloc(10*sizeof(char));
	
	#if SIMULATION_TYPE == 0
		sprintf(sim_type, "STD");
	#else
		sprintf(sim_type, "FLOOR_t_%d__",ERROR_WEIGHT);
	#endif

    bf_type = get_decoder_type_str();
	
    #if FOLDER_FOR_RESULTS > 0
        sprintf(file_out, "Results/%s_out_%d_%d_%d_%s.txt", sim_type, CODE_REDUNDANCY, COLUMN_WEIGHT, INITIAL_SEED, bf_type);
    #else
        sprintf(file_out, "%s_out_%d_%d_%d_%s.txt", sim_type, CODE_REDUNDANCY, COLUMN_WEIGHT, INITIAL_SEED, bf_type);
	#endif
    
    FILE *fptr_out;
    fptr_out = fopen(file_out, "w");
    fclose(fptr_out);    
}

void create_log_file(){

	//Format: n_r_v_SEED    
    //create file name
    char *file_log = (char*)malloc(50 * sizeof(char));
    char *bf_type = (char*)malloc(10*sizeof(char));
	char *sim_type = (char*)malloc(10*sizeof(char));
	
	#if SIMULATION_TYPE == 0
		sprintf(sim_type, "STD");
	#else
		sprintf(sim_type, "FLOOR_t_%d__",ERROR_WEIGHT);
	#endif

    bf_type = get_decoder_type_str();
	
    
    #if FOLDER_FOR_RESULTS > 0
        sprintf(file_log, "Results/%s_log_%d_%d_%d_%s.txt", sim_type, CODE_REDUNDANCY, COLUMN_WEIGHT, INITIAL_SEED, bf_type);
    #else
        sprintf(file_log, "%s_out_%d_%d_%d_%s.txt", sim_type, CODE_REDUNDANCY, COLUMN_WEIGHT, INITIAL_SEED, bf_type);
	#endif
    
    //Open file
    FILE *fptr_log;
    fptr_log = fopen(file_log, "w");
    fclose(fptr_log);    
}




void print_array(const unsigned long* arr, size_t len) {
    size_t preview = 10;  // quanti elementi stampare all'inizio e alla fine

    printf("[");
    if (len <= 2 * preview) {
        // Se l'array è piccolo lo stampo tutto
        for (size_t i = 0; i < len; i++) {
            printf("%4lu", arr[i]);
            if (i < len - 1) printf(", ");
        }
    } else {
        // Stampo i primi "preview"
        for (size_t i = 0; i < preview; i++) {
            printf("%4lu, ", arr[i]);
        }
        printf("..., "); // separatore

        // Stampo gli ultimi "preview"
        for (size_t i = len - preview; i < len; i++) {
            printf("%4lu", arr[i]);
            if (i < len - 1) printf(", ");
        }
    }
    printf("]\n");
}

void print_array_int(const int *arr, size_t len) {
    size_t preview = 10;  // quanti elementi stampare all'inizio e alla fine

    printf("[");
    if (len <= 2 * preview) {
        // Se l'array è piccolo lo stampo tutto
        for (size_t i = 0; i < len; i++) {
            printf("%4d", arr[i]);
            if (i < len - 1) printf(", ");
        }
    } else {
        // Stampo i primi "preview"
        for (size_t i = 0; i < preview; i++) {
            printf("%4d, ", arr[i]);
        }
        printf("..., "); // separatore

        // Stampo gli ultimi "preview"
        for (size_t i = len - preview; i < len; i++) {
            printf("%4d", arr[i]);
            if (i < len - 1) printf(", ");
        }
    }
    printf("]\n");
}

void print_bitarray(const uint8_t *arr, size_t n_bits) {
    for (size_t i = 0; i < n_bits; i++) {
        // Prendi il bit i-esimo
        uint8_t bit = (arr[i / 8] >> (i % 8)) & 1U;
        printf("%u", bit);

        // Inserisci uno spazio ogni 8 bit per leggibilità
        if ((i + 1) % 8 == 0) printf(" ");
    }
    printf("\n");
}


/** 
 * @brief  Updates the simulation results on file and prints progress to stdout.
 *
 * @param num_errors            Number of decoding failures observed.
 * @param num_errors_improved   Number of decoding failures observed with the improved decoder.
 * @param num_decodes           Total number of decoding attempts performed so far.
 * @param elapsed               Total elapsed time (in seconds) for executing `num_decodes` attempts.
 */
void update_out_file_mp(const int t, const int num_errors, const int num_errors_improved, const int num_errors_new, const int num_decodes, double elapsed){
	
    char file_out[100];
    char sim_type[20];
    const char *bf_type;

	#if SIMULATION_TYPE == 0
		sprintf(sim_type, "STD");
	#else
		sprintf(sim_type, "FLOOR_t_%d__",ERROR_WEIGHT);
	#endif

    bf_type = get_decoder_type_str();

    #if FOLDER_FOR_RESULTS > 0
        sprintf(file_out, "Results/%s_out_%d_%d_%d_%s.txt", sim_type, CODE_REDUNDANCY, COLUMN_WEIGHT, INITIAL_SEED, bf_type);
    #else
        sprintf(file_out, "%s_out_%d_%d_%d_%s.txt", sim_type, CODE_REDUNDANCY, COLUMN_WEIGHT, INITIAL_SEED, bf_type);
	#endif
    
    //Open file and append new result
    double time_taken = (elapsed)/(num_decodes); 
    float DFR = (float)num_errors/(float)num_decodes;
    float DFR_improved = (float)num_errors_improved/(float)num_decodes;		
    float DFR_new = (float)num_errors_new/(float)num_decodes;		

    #if SIMULATION_TYPE == 0
        printf("\n t = %d DFR = %e DFR(LU) = %e, DFR(Half) = %e, (time for one decode = %f s)", t, DFR, DFR_improved, DFR_new, time_taken);
    #else
        printf("\n u = %d DFR = %e DFR(LU) = %e, DFR(Half) = %e, (time for one decode = %f s)", t, DFR, DFR_improved, DFR_new, time_taken);
    #endif

    //Update results file
    FILE *fptr_out;
    fptr_out = fopen(file_out, "a");
    fprintf(fptr_out, "%d %e %e\n",t, (float)num_errors/(float)num_decodes, (float)num_errors_improved/(float)num_decodes);
    fclose(fptr_out);    
}

void update_log_file_mp(const int t, const int num_errors, const int num_errors_improved, const int num_errors_new, const int num_decodes, int num_tx){

	//Format: n_r_v_SEED    
    //create file name
    char file_log[100];
    char sim_type[20];
    const char *bf_type;
	
	#if SIMULATION_TYPE == 0
		sprintf(sim_type, "STD");
	#else
		sprintf(sim_type, "FLOOR_t_%d__",ERROR_WEIGHT);
	#endif

    bf_type = get_decoder_type_str();
	
    #if FOLDER_FOR_RESULTS > 0
        sprintf(file_log, "Results/%s_log_%d_%d_%d_%s.txt", sim_type, CODE_REDUNDANCY, COLUMN_WEIGHT, INITIAL_SEED, bf_type);
    #else
        sprintf(file_log, "%s_log_%d_%d_%d_%s.txt", sim_type, CODE_REDUNDANCY, COLUMN_WEIGHT, INITIAL_SEED, bf_type);
	#endif
    
    //measure time
    time_t time_now = time(NULL);
    struct tm tm = *localtime(&time_now);
//    printf("\n---> t = %d, Num Tx = %d, Num Errors = %d, Num Errors improved = %d, DFR =  %e, DFR_improved = %e (%d-%02d-%02d %02d:%02d:%02d)",t, num_decodes, num_errors, (float)num_errors/(float)num_decodes, (float)num_errors_improved/(float)num_decodes, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    //Open file and append new result
    FILE *fptr_log;
    fptr_log = fopen(file_log, "a");
	
	#if SIMULATION_TYPE == 0
		fprintf(fptr_log, "\nt = %d, Num Tx = %d, Num Errors = %d, Num Errors Improved = %d, DFR =  %e, DFR Improved = %e  (%d-%02d-%02d %02d:%02d:%02d)",t, num_tx, num_errors, num_errors_improved, (float)num_errors/(float)num_decodes, (float)num_errors_improved/(float)num_decodes, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	#else
		fprintf(fptr_log, "\nt = %d, u = %d, Num Tx = %d, Num Errors = %d, Num Errors Improved = %d, Num Errors Half = %d, DFR =  %e, DFR Improved = %e, DFR Half = %e (%d-%02d-%02d %02d:%02d:%02d)",ERROR_WEIGHT, t, num_decodes, num_errors, num_errors_improved, num_errors_new, (float)num_errors_new/(float)num_decodes, (float)num_errors/(float)num_decodes, (float)num_errors_improved/(float)num_decodes, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	#endif
	
    fclose(fptr_log);    


}
