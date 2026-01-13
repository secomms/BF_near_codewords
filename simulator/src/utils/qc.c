#include "qc.h"
#include "sampling.h"

/**
 * @brief Populate the support, by randomly sampling, of the columns of the circulating stocks
 * 
 * @param[in] h1			Vector defining the first circulant of the parity check matrix
 * @param[in] h2			Vector defining the second circulant of the parity check matrix
 * @param[out] H_first_row  First row of the parity check matrix
 */
void QC_generate_parity_supports(unsigned long* h1, unsigned long* h2, unsigned long* H_first_row){

    sample_unique(h1, CODE_REDUNDANCY, COLUMN_WEIGHT);
    sample_unique(h2, CODE_REDUNDANCY, COLUMN_WEIGHT);
    QC_compute_firstrow(h1, h2, H_first_row);

}

/** 
 * @brief Given the first column of each circulant compute the first row of the parity check matrix
 *
 * @param[in] h1			Vector defining the first circulant of the parity check matrix
 * @param[in] h2			Vector defining the second circulant of the parity check matrix
 * @param[out] H_first_row  First row of the parity check matrix
 */
void QC_compute_firstrow(unsigned long* h1, unsigned long* h2, unsigned long* H_first_row){
	
	int num_ones = 0;
	
	if(h1[0]==0){
		H_first_row[num_ones] = 0;
		num_ones ++;
	}
	for(int i = num_ones; i< COLUMN_WEIGHT; i++){
		H_first_row[num_ones] = (CODE_REDUNDANCY-h1[COLUMN_WEIGHT-1-i]);
		num_ones ++;
	}
	
	if(h2[0]==0){
		H_first_row[num_ones] = 0;
		num_ones ++;
	}
	for(int i = 0; i< COLUMN_WEIGHT; i++){
		H_first_row[num_ones] = CODE_REDUNDANCY+(CODE_REDUNDANCY-h2[COLUMN_WEIGHT-1-i]);
		num_ones ++;
	}
}

/** 
 * @brief This function calculates the syndrome associated with a sparse error vector, represented by its support, exploiting the particular quasi-cyclic structure of the code parity matrix.
 * This function is the decoding process.
 *
 * @param[in] h1			Vector defining the first circulant of the parity check matrix
 * @param[in] h2			Vector defining the second circulant of the parity check matrix
 * @param[in] err_support	Array containing the positions of non-null elements in the error vector
 * @param[out] syndrome 	Is the result of the decoding process, the function populate this vector
 */
void QC_compute_syndrome(const unsigned long* h1, const unsigned long* h2, const unsigned long *err_support, const int t, uint8_t* syndrome){

    zero_bitarray(syndrome, CODE_REDUNDANCY);

	//start modifying syndrome
	for(int i = 0; i < t; i++){
		int pos = err_support[i];
		if (pos < CODE_REDUNDANCY){
			for(int j = 0; j<COLUMN_WEIGHT; j++){
				unsigned long pos_to_flip = (h1[j]+pos)%CODE_REDUNDANCY;
				TOGGLE_BIT(syndrome, pos_to_flip);
			}
		} else {
			for(int j = 0; j<COLUMN_WEIGHT; j++){
				unsigned long pos_to_flip = (h2[j]+pos)%CODE_REDUNDANCY;
				TOGGLE_BIT(syndrome, pos_to_flip);
			}
		}
	}
}