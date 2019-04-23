// DO NOT EDIT! THIS FILE WAS GENERATED FROM src/neuron/plasticity/stdp/weight_dependence/weight_additive_one_term_impl.c

#include "weight_additive_one_term_impl.h"
//---------------------------------------
// Globals
//---------------------------------------
// Global plasticity parameter data
plasticity_weight_region_data_t *plasticity_weight_region_data;
//---------------------------------------
// Functions
//---------------------------------------
address_t weight_initialise(address_t address, uint32_t n_synapse_types,
                            uint32_t *ring_buffer_to_input_buffer_left_shifts) {
    use(ring_buffer_to_input_buffer_left_shifts);

    log_mini_debug("%u", 7233);  /* "weight_initialise: starting"*/
    log_mini_debug("%u", 7234);  /* "\tSTDP additive one-term weight dependence"*/

    // Copy plasticity region data from address
    // **NOTE** this seems somewhat safer than relying on sizeof
    int32_t *plasticity_word = (int32_t*) address;
    plasticity_weight_region_data = (plasticity_weight_region_data_t *)
        spin1_malloc(sizeof(plasticity_weight_region_data_t) * n_synapse_types);
    if (plasticity_weight_region_data == NULL) {
        log_mini_error("%u", 7235);  /* "Could not initialise weight region data"*/
        return NULL;
    }
    for (uint32_t s = 0; s < n_synapse_types; s++) {
        plasticity_weight_region_data[s].min_weight = *plasticity_word++;
        plasticity_weight_region_data[s].max_weight = *plasticity_word++;
        plasticity_weight_region_data[s].a2_plus = *plasticity_word++;
        plasticity_weight_region_data[s].a2_minus = *plasticity_word++;

        log_mini_debug("%u%u%d%d%d%d", 7236,s, plasticity_weight_region_data[s].min_weight,plasticity_weight_region_data[s].max_weight,plasticity_weight_region_data[s].a2_plus,plasticity_weight_region_data[s].a2_minus);
        /* "\tSynapse type %u: Min weight:%d, Max weight:%d, A2+:%d, A2-:%d"*/




    }
    log_mini_debug("%u", 7237);  /* "weight_initialise: completed successfully"*/

    // Return end address of region
    return (address_t) plasticity_word;
}
