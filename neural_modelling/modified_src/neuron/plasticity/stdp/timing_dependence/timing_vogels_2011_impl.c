// DO NOT EDIT! THIS FILE WAS GENERATED FROM src/neuron/plasticity/stdp/timing_dependence/timing_vogels_2011_impl.c

#include "timing_vogels_2011_impl.h"
//---------------------------------------
// Globals
//---------------------------------------
// Exponential lookup-tables
int16_t tau_lookup[TAU_SIZE];
// Global plasticity parameter data
plasticity_trace_region_data_t plasticity_trace_region_data;

//---------------------------------------
// Functions
//---------------------------------------
uint32_t *timing_initialise(uint32_t* address)
{
    log_mini_info("%u", 7175);  /* "timing_initialise: starting"*/
    log_mini_info("%u", 7176);  /* "\tVogels 2011 timing rule"*/

    // Copy parameters
    plasticity_trace_region_data.alpha = (int32_t)address[0];

    // Copy LUTs from following memory
    address_t lut_address = maths_copy_int16_lut(&address[1], TAU_SIZE, &tau_lookup[0]);

    log_mini_info("%u", 7177);  /* "timing_initialise: completed successfully"*/

    return lut_address;
}
