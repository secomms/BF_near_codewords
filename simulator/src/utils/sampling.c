#include "sampling.h"
#include "../include/debug.h"
#include "../../params.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



// Function to merge two halves of the array
void merge(unsigned long arr[], int l, int m, int r) {
    int n1 = m - l + 1; // Size of the left subarray
    int n2 = r - m;     // Size of the right subarray

    unsigned long L[n1], R[n2];   // Temporary arrays to hold the two halves

    // Copy data to temporary arrays L[] and R[]
    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l; // Initial indices of the subarrays and merged array

    // Merge the temporary arrays back into arr[l..r]
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[], if any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[], if any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

// Function to implement MergeSort
void mergeSort(unsigned long *arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2; // Find the middle point

        // Sort first and second halves
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);

        // Merge the sorted halves
        merge(arr, l, m, r);
    }
}

/**
 * @brief Extracts a subset of unique locations in the range [0, universe_size-1]
 *
 * @param[out] support       Output array where the extracted positions are stored.
 * @param[in]  universe_size Size of the universe to sample from.
 * @param[in]  k             Number of positions to extract. 
 */
void sample_unique(unsigned long support[], size_t universe_size, size_t k){

    unsigned long pos = rand()%universe_size;
    support[0] = pos;

    size_t num_found = 1;

    while(num_found < k){
        pos = rand()%universe_size;

        int flag_duplicate = 0;
        for(int i = 0;i<num_found;i++){
            if(support[i] == pos){ 
                flag_duplicate = 1;
                break;
            }
        }
        if (flag_duplicate == 0){
            support[num_found] = pos;
            num_found ++;
        }
    }
	
	//sort the vector
	mergeSort(support, 0, k-1);
}

/**
 * @brief Samples k unique positions from the universe [0, universe_size)
 *        using rejection sampling with bitmap-based duplicate detection.
 *
 * @param[out] support       Output array of size k. Filled with k distinct positions in [0, universe_size).
 * @param[in]  universe_size Size of the universe to sample from. Positions are drawn in [0, universe_size).
 * @param[in]  k             Number of unique positions to extract.
 */
void sample_unique_bitmap(unsigned long support[], size_t universe_size, size_t k){
	
    size_t bitmap_size = universe_size / 8 + 1;
	uint8_t *seen = calloc(bitmap_size, 1);
 
	size_t num_found = 0;
	while (num_found < k) {
		unsigned long pos = rand() % universe_size;
		if (!(seen[pos >> 3] & (1 << (pos & 7)))) {
			seen[pos >> 3] |= (1 << (pos & 7));
			support[num_found++] = pos;
		}
	}
	// free the bitmap
	free(seen);
	// sort the vector
	mergeSort(support, 0, k-1);
}

/**
 * @brief "Near codeword" refers to a situation in error-correcting codes where a received message is close to a valid codeword, but not exactly a codeword.
 * This function is specific for FLOOR simulation, sample error which has exactly u intersection with a random column of H
 *
 * @param[out] support       Output array where the position of the near codeword non empty element are stored.
 * @param[in] u              This is the weigth of the codeword.
 * @param[in] h1			Vector defining the first circulant of the parity check matrix
 * @param[in] h2			Vector defining the second circulant of the parity check matrix
 */
void sample_from_near_codeword(unsigned long *err_support, const int u, const unsigned long* h1, const unsigned long* h2){

	//sample error with large intersection from near codewords
	//assure that intersection with near codewords is exactly u

	// fare il check che i valori di column weigth e u_max sia uguale

	// rejection sampling
	unsigned long* pos = malloc(COLUMN_WEIGHT * sizeof(unsigned long));

	int iter = 0;

	int sample_ok = 0;
	while (sample_ok == 0){
		iter++;
		
		//Select column
		unsigned long col_index = rand()%CODE_LENGTH;

		// initial range of numbers
		for(int i=0; i<COLUMN_WEIGHT; ++i) pos[i]=i;

		//Generate random permutation of set [0 ; COLUMN_WEIGHT - 1]
		for (int i = COLUMN_WEIGHT -1; i >= 0; --i){
			
			//generate a random number [0, COLUMN_WEIGHT-1]
			int j = rand() % (i+1);
			//swap the last element with element at random index
			unsigned long temp = pos[i];
			pos[i] = pos[j];
			pos[j] = temp;
		}
		
		//Sample u from random column
		for(int i = 0; i<u;i++){
			unsigned long x;
			if (col_index < CODE_REDUNDANCY){
				x = (h1[pos[i]]+col_index)%CODE_REDUNDANCY;
			}else{
				x = CODE_REDUNDANCY + (h2[pos[i]]+col_index)%CODE_REDUNDANCY;
			}
			err_support[i] = x;
		}

		//sample remaining positions
		
		//assign first position
		int num_found = u;

		while(num_found < ERROR_WEIGHT){
			unsigned long new_pos = rand()%CODE_LENGTH;
			
			//First, check that new position has already been sampled
			int flag_ok = 0;

			for(int i = 0;i<num_found;i++){
				if(err_support[i] == new_pos){ 
					flag_ok = 1;
				}
			}
			
			//Now, check if it is not in the support of the column
			if (flag_ok == 0){
				int i = 0;
				while((i<COLUMN_WEIGHT)&&(flag_ok == 0)){
					unsigned long x;
					if (col_index < CODE_REDUNDANCY){
						x = (h1[i]+col_index)%CODE_REDUNDANCY;
					}else{
						x = CODE_REDUNDANCY + (h2[i]+col_index)%CODE_REDUNDANCY;
					}
					
					if (new_pos != x){
						i ++;
					}else{
						flag_ok = 1;
					}
				}
			}
			
			//append new position 
			if (flag_ok == 0){            
				err_support[num_found] = new_pos;
				num_found ++;
			}
		}
		
		//check if there is only one near codeword with intersection exactly u
		//if there is a near codeword intersecting in less than u positions, it is fine
		//if there is a near codeword intersecting in more than u positions, abort
		

		int num_intersections_u = 0; //number of near codewords with intersection exactly u
		int num_intersections_more_than_u = 0; //number of near codewords with intersection more than u
		int i = 0; //index for the near codeword to be analyzed
						
		//do the check for first block
		while((i<CODE_REDUNDANCY)&&(num_intersections_more_than_u == 0)){
			int num_inters = 0;
			for(int j = 0; j < COLUMN_WEIGHT; j++){
				unsigned long pos = (h1[j]+i)%CODE_REDUNDANCY; //shift position
				for(int ell = 0; ell < ERROR_WEIGHT; ell++){
					if(pos == err_support[ell]){ //check if this value is in the error support
						num_inters ++;
					}
				}
			}
			
			//see the value of num_inters
			if(num_inters > u){
				num_intersections_more_than_u ++;
			}
			
			i++;
		}

		
		
		//now, consider second block
		//do the check for first block
		i = 0;
		while((i<CODE_REDUNDANCY)&&(num_intersections_more_than_u == 0)){
			int num_inters = 0;
			for(int j = 0; j < COLUMN_WEIGHT; j++){
				unsigned long pos = CODE_REDUNDANCY+(h2[j]+i)%CODE_REDUNDANCY; //shift position
				for(int ell = 0; ell < ERROR_WEIGHT; ell++){
					if(pos == err_support[ell]){ //check if this value is in the error support
						num_inters ++;
					}
				}
			}
			
			//see the value of num_inters
			if(num_inters > u){
				num_intersections_more_than_u ++;
			}

			i++;
		}
		
		//set sample_ok to 1 if everything was successful
		if(num_intersections_more_than_u == 0){
			sample_ok = 1;
		}

		switch(iter){
			case 100: {
				fprintf(stderr, "When sampling nearcodeword with %d intersection, Rejection sampling exceeded 100 iterations.\n", u);
				break;
			};
			case 200: {
				fprintf(stderr, "When sampling nearcodeword with %d intersection, Rejection sampling exceeded 100 iterations.\n", u);
				break;
			};
			case 300: {
				fprintf(stderr, "When sampling nearcodeword with %d intersection, Rejection sampling exceeded 100 iterations.\n", u);
				break;
			};
			default:
				break;

		}

	}
	free(pos);

	//sort the vector
	mergeSort(err_support, 0, ERROR_WEIGHT-1);
}

