// DO NOT EDIT! THIS FILE WAS GENERATED FROM src/neuron/plasticity/stdp/timing_dependence/timing_pair_impl.c

#include "timing_pair_impl.h"
//---------------------------------------
// Globals
//---------------------------------------
// Exponential lookup-tables
int16_t tau_plus_lookup[TAU_PLUS_SIZE];
int16_t tau_minus_lookup[TAU_MINUS_SIZE];
//---------------------------------------
// Functions
//---------------------------------------
address_t timing_initialise(address_t address) {

    log_mini_debug("%u", 7153);  /* "timing_initialise: starting"*/
    log_mini_debug("%u", 7154);  /* "\tSTDP pair rule"*/
    // **TODO** assert number of neurons is less than max

    // Copy LUTs from following memory
    address_t lut_address = maths_copy_int16_lut(&address[0], TAU_PLUS_SIZE,
                                                 &tau_plus_lookup[0]);
    lut_address = maths_copy_int16_lut(lut_address, TAU_MINUS_SIZE,
                                       &tau_minus_lookup[0]);

    log_mini_debug("%u", 7155);  /* "timing_initialise: completed successfully"*/

    return lut_address;
}
