// DO NOT EDIT! THIS FILE WAS GENERATED FROM src/neuron/models/neuron_model_izh_impl.c

#include "neuron_model_izh_impl.h"
#include <debug.h>
static global_neuron_params_pointer_t global_params;

/*! \brief For linear membrane voltages, 1.5 is the correct value. However
 * with actual membrane voltage behaviour and tested over an wide range of
 * use cases 1.85 gives slightly better spike timings.
 */
static const REAL SIMPLE_TQ_OFFSET = REAL_CONST(1.85);

/////////////////////////////////////////////////////////////
// definition for Izhikevich neuron
/*static inline void _neuron_ode(
        REAL t, REAL stateVar[], REAL dstateVar_dt[],
        neuron_pointer_t neuron, REAL input_this_timestep) {

    REAL V_now = stateVar[1];
    REAL U_now = stateVar[2];
    log_debug(" sv1 %9.4k  V %9.4k --- sv2 %9.4k  U %9.4k\n", stateVar[1],
              neuron->V, stateVar[2], neuron->U);

    // Update V
    dstateVar_dt[1] =
        REAL_CONST(140.0)
        + (REAL_CONST(5.0) + REAL_CONST(0.0400) * V_now) * V_now - U_now
        + input_this_timestep;

    // Update U
    dstateVar_dt[2] = neuron->A * (neuron->B * V_now - U_now);
} */

/*!
 * \brief Midpoint is best balance between speed and accuracy so far from
 * ODE solve comparison work paper shows that Trapezoid version gives better
 * accuracy at small speed cost
 * \param[in] h
 * \param[in] neuron
 * \param[in] input_this_timestep
 */
static inline void _rk2_kernel_midpoint(REAL h, neuron_pointer_t neuron,
                                        REAL input_this_timestep) {

    // to match Mathematica names
    REAL lastV1 = neuron->V;
    REAL lastU1 = neuron->U;
    REAL a = neuron->A;
    REAL b = neuron->B;

    REAL pre_alph = REAL_CONST(140.0) + input_this_timestep - lastU1;
    REAL alpha = pre_alph
                 + ( REAL_CONST(5.0) + REAL_CONST(0.040008544921875) * lastV1) * lastV1;
    REAL eta = lastV1 + REAL_HALF(h * alpha);

    // could be represented as a long fract?
    REAL beta = REAL_HALF(h * (b * lastV1 - lastU1) * a);

    neuron->V += h * (pre_alph - beta
                      + ( REAL_CONST(5.0) + REAL_CONST(0.040008544921875) * eta) * eta);

    neuron->U += a * h * (-lastU1 - beta + b * eta);
}

void neuron_model_set_global_neuron_params(
        global_neuron_params_pointer_t params) {
    global_params = params;
}

state_t neuron_model_state_update(
        uint16_t num_excitatory_inputs, input_t* exc_input,
		uint16_t num_inhibitory_inputs, input_t* inh_input,
		input_t external_bias, neuron_pointer_t neuron) {

		REAL total_exc = 0;
		REAL total_inh = 0;

		for (int i =0; i<num_excitatory_inputs; i++){
			total_exc += exc_input[i];
		}
		for (int i =0; i<num_inhibitory_inputs; i++){
			total_inh += inh_input[i];
		}

    input_t input_this_timestep = total_exc - total_inh
                                  + external_bias + neuron->I_offset;

    // the best AR update so far
    _rk2_kernel_midpoint(neuron->this_h, neuron, input_this_timestep);
    neuron->this_h = global_params->machine_timestep_ms;

    return neuron->V;
}

void neuron_model_has_spiked(neuron_pointer_t neuron) {

    // reset membrane voltage
    neuron->V = neuron->C;

    // offset 2nd state variable
    neuron->U += neuron->D;

    // simple threshold correction - next timestep (only) gets a bump
    neuron->this_h = global_params->machine_timestep_ms * SIMPLE_TQ_OFFSET;
}

state_t neuron_model_get_membrane_voltage(neuron_pointer_t neuron) {
    return neuron->V;
}

void neuron_model_print_state_variables(restrict neuron_pointer_t neuron) {
    log_mini_debug("%u%11.4k", 7256, neuron->V);  /* "V = %11.4k "*/
    log_mini_debug("%u%11.4k", 7257, neuron->U);  /* "U = %11.4k "*/
}

void neuron_model_print_parameters(restrict neuron_pointer_t neuron) {
    log_mini_debug("%u%11.4k", 7258, neuron->A);  /* "A = %11.4k "*/
    log_mini_debug("%u%11.4k", 7259, neuron->B);  /* "B = %11.4k "*/
    log_mini_debug("%u%11.4k", 7260, neuron->C);  /* "C = %11.4k "*/
    log_mini_debug("%u%11.4k", 7261, neuron->D);  /* "D = %11.4k "*/

    log_mini_debug("%u%11.4k", 7262, neuron->I_offset);  /* "I = %11.4k \n"*/
}
