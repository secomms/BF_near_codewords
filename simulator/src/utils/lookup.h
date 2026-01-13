#ifndef __LOOKUP_H__
#define __LOOKUP_H__

#include "../../params.h"

struct table_entry
{
    char digest[CHARS_FOR_LOOKUP]; 
    unsigned long index;
};

extern struct table_entry* trapping_sets;

//on input a syndrome of weight COLUMN_WEIGHT, computes a unique integer as the hash of the syndrome
int hash_syndrome(const unsigned long *syndrome_support);


void create_lookup(const unsigned long* h1, const unsigned long* h2, struct table_entry* trapping_sets);


#endif