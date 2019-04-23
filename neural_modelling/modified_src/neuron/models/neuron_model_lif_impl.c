// DO NOT EDIT! THIS FILE WAS GENERATED FROM src/neuron/models/neuron_model_lif_impl.c

#include "neuron_model_lif_impl.h"
#include <debug.h>
// simple Leaky I&F ODE
static inline void _lif_neuron_closed_form(
        neuron_pointer_t neuron, REAL V_prev, input_t input_this_timestep) {

    REAL alpha = input_this_timestep * neuron->R_membrane + neuron->V_rest;

    // update membrane voltage
    neuron->V_membrane = alpha - (neuron->exp_TC * (alpha - V_prev));
}

void neuron_model_set_global_neuron_params(
        global_neuron_params_pointer_t params) {
    use(params);

    // Does Nothing - no params
}

state_t neuron_model_state_update(
		uint16_t num_excitatory_inputs, input_t* exc_input,
		uint16_t num_inhibitory_inputs, input_t* inh_input,
		input_t external_bias, neuron_pointer_t neuron) {

 log_mini_debug("%u%12.6k%12.6k", 7247, exc_input[0], exc_input[1]);  /* "Exc 1: %12.6k, Exc 2: %12.6k"*/
 log_mini_debug("%u%12.6k%12.6k", 7248, inh_input[0], inh_input[1]);  /* "Inh 1: %12.6k, Inh 2: %12.6k"*/


    // If outside of the refractory period
    if (neuron->refract_timer <= 0) {
		REAL total_exc = 0;
		REAL total_inh = 0;

		for (int i=0; i < num_excitatory_inputs; i++){
			total_exc += exc_input[i];
		}
		for (int i=0; i< num_inhibitory_inputs; i++){
			total_inh += inh_input[i];
		}
        // Get the input in nA
        input_t input_this_timestep =
            total_exc - total_inh + external_bias + neuron->I_offset;

        _lif_neuron_closed_form(
            neuron, neuron->V_membrane, input_this_timestep);
    } else {

        // countdown refractory timer
        neuron->refract_timer -= 1;
    }
    return neuron->V_membrane;
}

void neuron_model_has_spiked(neuron_pointer_t neuron) {

    // reset membrane voltage
    neuron->V_membrane = neuron->V_reset;

    // reset refractory timer
    neuron->refract_timer  = neuron->T_refract;
}

state_t neuron_model_get_membrane_voltage(neuron_pointer_t neuron) {
    return neuron->V_membrane;
}

void neuron_model_print_state_variables(restrict neuron_pointer_t neuron) {
    log_mini_debug("%u%11.4k", 7249, neuron->V_membrane);  /* "V membrane    = %11.4k mv"*/
}

void neuron_model_print_parameters(restrict neuron_pointer_t neuron) {
    log_mini_debug("%u%11.4k", 7250, neuron->V_reset);  /* "V reset       = %11.4k mv"*/
    log_mini_debug("%u%11.4k", 7251, neuron->V_rest);  /* "V rest        = %11.4k mv"*/

    log_mini_debug("%u%11.4k", 7252, neuron->I_offset);  /* "I offset      = %11.4k nA"*/
    log_mini_debug("%u%11.4k", 7253, neuron->R_membrane);  /* "R membrane    = %11.4k Mohm"*/

    log_mini_debug("%u%11.4k", 7254, neuron->exp_TC);  /* "exp(-ms/(RC)) = %11.4k [.]"*/

    log_mini_debug("%u%u", 7255, neuron->T_refract);  /* "T refract     = %u timesteps"*/
}
