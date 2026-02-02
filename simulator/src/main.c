#include "../params.h"

#include "include/bitarray.h"
#include "include/debug.h"
#include "include/simulation.h"
#include "decoders/decoders.h"
#include "utils/qc.h"
#include "utils/logging.h"
#include "utils/sampling.h"
#include "utils/lookup.h"

#include <stddef.h>
#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>


#include <unistd.h>   // per getpid()
#include <stdlib.h>   // per system()



void signal_handler(int sig) {
    fprintf(stderr, "Process received signal %d\n", sig);
    fflush(stderr);
    // Potresti stampare stacktrace se vuoi (vedi sotto)
    exit(1);
}

// function that mask all the signal message received from the process
void setup_signal_handlers() {
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGFPE,  signal_handler);
    signal(SIGILL,  signal_handler);
    signal(SIGBUS,  signal_handler);
}

int check_new_decoder(const int err_weight, int* estimated_vector, const unsigned long* target_vector){

    size_t hw = 0;

    for(int x = 0; x<err_weight;x++){
        estimated_vector[target_vector[x]] ^= 1;
    }

    int ko_e1, ko_e2 = 0;
    
    for(int i=0; i < CODE_REDUNDANCY; i++){
        if(estimated_vector[i] == 1) ko_e1 = 1;
        if(estimated_vector[i + CODE_REDUNDANCY] == 1) ko_e2 = 1; 
    }

    if(ko_e1 && ko_e2) return 1;


    return 0;


}

int check_solution(const int err_weight, int* estimated_vector, const unsigned long* target_vector){

    int ok = 0;
    size_t hw = 0;
    
    //flip bits indexed by error estimate
    for(int x = 0; x<err_weight;x++){
        estimated_vector[target_vector[x]] ^= 1;
    }

    // **************************************************************************************************************
    // NOTE: if the error vector becomes an array vector we can reuse the population count so the function hamming_weigth
    // **************************************************************************************************************
    //compute hamming weight of vector
    for(int i = 0; i<CODE_LENGTH;i++){
        hw += estimated_vector[i];
    }
    
    if (hw > 0){
        ok = 1;
    }
    return ok;
}

// define the validation rule for the simulation parameters 
//#if SIMULATION_TYPE == 1 && COLUMN_WEIGHT != U_MAX
//#error Errore nei parametri per la simulatzione del FLOOR
//#endif

void setup_debug_log() {
    char filename[64];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    // Crea nome tipo "debug_2025-11-10_18-45-33.log"
    strftime(filename, sizeof(filename), "debug_%Y-%m-%d_%H-%M-%S.log", tm_info);

    FILE *fp = freopen(filename, "w", stderr);
    if (!fp) {
        perror("freopen");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, ">>> Logging started: %s\n", filename);
    fflush(stderr);
}



DecodingMatrix H;

int main(int argc, char* argv[]){

    setup_signal_handlers();
    setup_debug_log();


    print_process_info(getpid());
    printf("\n");

    //####################################################################################
    // initial setup
    //####################################################################################
    print_simulation_params();
    printf("\n");

    omp_set_num_threads(NUM_THREADS);
	srand(INITIAL_SEED);
    create_log_file();
    create_out_file();

    H.h1 = malloc(COLUMN_WEIGHT * sizeof(unsigned long));
    H.h2 = malloc(COLUMN_WEIGHT * sizeof(unsigned long));
    H.H_first_row = malloc(2 * COLUMN_WEIGHT * sizeof(unsigned long));

	trapping_sets = (struct table_entry*) malloc(CODE_LENGTH*sizeof(struct table_entry));

    QC_generate_parity_supports(H.h1, H.h2, H.H_first_row);
    create_lookup(H.h1, H.h2, trapping_sets);

    //####################################################################################
    // starting simulation 
    //####################################################################################
    printf("Starting simulation...\n");

    SIM_LOOP {
        // local variable for the simulation with a fixed t 
		int num_errors = 0, num_errors_improved = 0, num_decodes = 0;
        int num_errors_new = 0;
        // start timestamp
        double start_time = omp_get_wtime();  

        // start parallel region
        #pragma omp parallel shared(trapping_sets, H) 
		{
            // params are defined here to avoid the growth of the memory, this variable is threadprivate, this variable is threadprivate
            DecodingParams* data = alloc_decoding_context(t);

            while(1){

                reset_decoding_context(data, t);

				int stop = 0, my_decode_id = 0;

				#pragma omp critical
				{
					if (num_decodes >= NUM_DECODES_MAX || num_errors_improved >= NUM_ERRORS_MAX) stop = 1;
				}
		
				if (stop) break;

				#pragma omp atomic capture
				my_decode_id = num_decodes++;
		

                SAMPLE_ERROR(data, H);
                // memory safe
                QC_compute_syndrome(H.h1, H.h2, data->err_support, t, data->syndrome);
                
                switch (DECODER_TYPE) {
                    case 0: bf_max(&H, data, floor(ITER_MAX*t)); break;
                    case 1: bf_majority(&H, data);break;
                    case 2: bf_out(&H, data); break;
                    case 3: bgf(&H, data); break;
                    case 4: new_bike_decoder(&H, data); break;
                }



                int* new_err = malloc(CODE_LENGTH*sizeof(int));
                memcpy(new_err, data->err_estimated_improved, CODE_LENGTH*sizeof(int));

                int decode_ko          = ((data->residual[0] == 0) ? check_solution(t, data->err_estimated, data->err_support) : 1);
				int decode_ko_improved = ((data->residual[1] == 0) ? check_solution(t, data->err_estimated_improved, data->err_support) : 1);
                int decode_ko_new      = check_new_decoder(t, new_err, data->err_support);


				#pragma omp critical 
				{
					num_errors += decode_ko;
					num_errors_improved += decode_ko_improved;
                    num_errors_new += decode_ko_new;

					if((decode_ko_improved)||((num_decodes%NUM_BACKUP)==0)) 
                        UPDATE_LOG(num_errors, num_erros_impoved, num_errors_new, num_decodes, my_decode_id);
					
				}
                
            }
            
            free_decoding_context(data);
            
        }
        
		double elapsed = omp_get_wtime() - start_time;
		UPDATE_OUTPUT(num_errors, num_errors_improved, num_errors_new, num_decodes, elapsed);
    }

    printf("\nSimulation finished...\n");
    return 0;

}
