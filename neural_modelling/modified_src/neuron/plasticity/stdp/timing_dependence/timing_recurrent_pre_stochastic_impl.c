// DO NOT EDIT! THIS FILE WAS GENERATED FROM src/neuron/plasticity/stdp/timing_dependence/timing_recurrent_pre_stochastic_impl.c

#include "timing_recurrent_pre_stochastic_impl.h"
//---------------------------------------
// Globals
//---------------------------------------
// Exponential lookup-tables
uint16_t pre_exp_dist_lookup[STDP_FIXED_POINT_ONE];
uint16_t post_exp_dist_lookup[STDP_FIXED_POINT_ONE];
// Global plasticity parameter data
plasticity_trace_region_data_t plasticity_trace_region_data;

//---------------------------------------
// Functions
//---------------------------------------
address_t timing_initialise(address_t address) {

    log_mini_debug("%u", 7156);  /* "timing_initialise: starting"*/
    log_mini_debug("%u", 7157);  /* "\tRecurrent pre-calculated stochastic STDP rule"*/

    // Copy plasticity region data from address
    // **NOTE** this seems somewhat safer than relying on sizeof
    plasticity_trace_region_data.accumulator_depression_plus_one =
        (int32_t) address[0];
    plasticity_trace_region_data.accumulator_potentiation_minus_one =
        (int32_t) address[1];

    log_mini_debug("%u%d%d", 7158,plasticity_trace_region_data.accumulator_depression_plus_one - 1,plasticity_trace_region_data.accumulator_potentiation_minus_one + 1);
    /* "\tAccumulator depression=%d, Accumulator potentiation=%d"*/



    // Copy LUTs from following memory
    // **HACK** these aren't actually int16_t-based but this function will still
    // work fine
    address_t lut_address = maths_copy_int16_lut(
        &address[2], STDP_FIXED_POINT_ONE, (int16_t*) &pre_exp_dist_lookup[0]);
    lut_address = maths_copy_int16_lut(
        lut_address, STDP_FIXED_POINT_ONE, (int16_t*) &post_exp_dist_lookup[0]);

    log_mini_debug("%u", 7159);  /* "timing_initialise: completed successfully"*/

    return lut_address;
}
