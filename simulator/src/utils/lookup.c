#include "lookup.h"
#include "ldpc.h"
#include "qc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/bitarray.h"

struct table_entry* trapping_sets = NULL;

void create_lookup(const unsigned long* h1, const unsigned long* h2, struct table_entry* trapping_sets){
    
	//Initialize syndrome
	uint8_t *syndrome = alloc_bitarray(CODE_REDUNDANCY);

	//Trapping sets for first block
    for(int i = 0;i < CODE_REDUNDANCY; i++){

		unsigned long* err_pos = malloc(COLUMN_WEIGHT*sizeof(unsigned long));
		
		for(int j = 0; j<COLUMN_WEIGHT; j++){
			unsigned long x = (h1[j]+i)%CODE_REDUNDANCY;
			err_pos[j] = x;
		}
		
		//the syndrome is set to 0 inside the function while there will be an errror
        QC_compute_syndrome(h1, h2, err_pos, COLUMN_WEIGHT, syndrome);

		unsigned long syndrome_support[COLUMN_WEIGHT];
        //int w = compute_support(syndrome_support, syndrome, CODE_REDUNDANCY, COLUMN_WEIGHT);

		int num = 0, w = 0;
		for(int j=0; j < CODE_REDUNDANCY; j++){
			if(GET_BIT(syndrome, j)){
				syndrome_support[num] = j;
				num++;
				w++;
			}
		}

		//save support
		int n = 0;
		for (int u = 0; u < COLUMN_WEIGHT; u++) {
			n += sprintf (&trapping_sets[i].digest[n], "%lu", syndrome_support[u]);
			n += sprintf (&trapping_sets[i].digest[n], "-");
		}

		//printf("\n--> TS Number %d, syndrome weight = %d, hash = %s\n",CODE_REDUNDANCY + i, w, trapping_sets[i].digest);
		
	//	trapping_sets[i].digest = support_str;
		trapping_sets[i].index = i;
    }
		
	//Trapping sets for second block
    for(int i = 0;i < CODE_REDUNDANCY; i++){
		
		unsigned long* err_pos = malloc(COLUMN_WEIGHT*sizeof(unsigned long));

		for(int j = 0; j<COLUMN_WEIGHT; j++){
			unsigned long x = (h2[j]+i)%CODE_REDUNDANCY;
			err_pos[j] = CODE_REDUNDANCY + x;
		}
        
        QC_compute_syndrome(h1, h2, err_pos, COLUMN_WEIGHT, syndrome);

		unsigned long syndrome_support[COLUMN_WEIGHT];
		
		int num = 0, w = 0;
		for(int j=0; j < CODE_REDUNDANCY; j++){
			if(GET_BIT(syndrome, j)){
				syndrome_support[num] = j;
				num++;
				w++;
			}
		}
		int n = 0;

		for (int u = 0; u < COLUMN_WEIGHT; u++) {
			n += sprintf (&trapping_sets[CODE_REDUNDANCY + i].digest[n], "%lu", syndrome_support[u]);
			n += sprintf (&trapping_sets[CODE_REDUNDANCY + i].digest[n], "-");
		}

		
		trapping_sets[CODE_REDUNDANCY + i].index = CODE_REDUNDANCY + i;
    }

}
