// DO NOT EDIT! THIS FILE WAS GENERATED FROM src/delay_extension/delay_extension.c

#include "delay_extension.h"
#include <common/neuron-typedefs.h>
#include <common/in_spikes.h>
#include <bit_field.h>
#include <data_specification.h>
#include <debug.h>
#include <simulation.h>
#include <spin1_api.h>
//! values for the priority for each callback
typedef enum callback_priorities {
    MC_PACKET = -1, SDP = 0, USER = 1, TIMER = 3, DMA = 2
} callback_priorities;

typedef enum extra_provenance_data_region_entries{
    N_PACKETS_RECEIVED = 0,
    N_PACKETS_PROCESSED = 1,
    N_PACKETS_ADDED = 2,
    N_PACKETS_SENT = 3,
    N_BUFFER_OVERFLOWS = 4,
    N_DELAYS = 5
} extra_provenance_data_region_entries;

// Globals
static uint32_t key = 0;
static uint32_t incoming_key = 0;
static uint32_t incoming_mask = 0;
static uint32_t incoming_neuron_mask = 0;
static uint32_t num_neurons = 0;
static uint32_t time = UINT32_MAX;
static uint32_t simulation_ticks = 0;
static uint32_t infinite_run;

static uint8_t **spike_counters = NULL;
static bit_field_t *neuron_delay_stage_config = NULL;
static uint32_t num_delay_stages = 0;
static uint32_t num_delay_slots_mask = 0;
static uint32_t neuron_bit_field_words = 0;

static uint32_t n_in_spikes = 0;
static uint32_t n_processed_spikes = 0;
static uint32_t n_spikes_sent = 0;
static uint32_t n_spikes_added = 0;

//! An amount of microseconds to back off before starting the timer, in an
//! attempt to avoid overloading the network
static uint32_t timer_offset;

//! The number of clock ticks between processing each neuron at each delay
//! stage
static uint32_t time_between_spikes;

//! The expected current clock tick of timer_1 to wait for
static uint32_t expected_time;

static uint32_t n_delays = 0;

// Spin1 API ticks - to know when the timer wraps
extern uint ticks;

// Initialise
static uint32_t timer_period = 0;

//---------------------------------------
// Because we don't want to include string.h or strings.h for memset
static inline void zero_spike_counters(void *location, uint32_t num_items)
{
    uint32_t i;

    for (i = 0 ; i < num_items ; i++) {
        ((uint8_t *) location)[i] = 0;
    }
}

static inline uint32_t round_to_next_pot(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

static bool read_parameters(address_t address) {

    log_mini_debug("%u", 7379);  /* "read_parameters: starting"*/

    key = address[KEY];
    incoming_key = address[INCOMING_KEY];
    incoming_mask = address[INCOMING_MASK];
    incoming_neuron_mask = ~incoming_mask;
    log_mini_debug("%u%08x%08x%08x%08x", 7380,key, incoming_key, incoming_mask, incoming_neuron_mask);
    /* "\t key = 0x%08x, incoming key = 0x%08x, incoming mask = 0x%08x,incoming key mask = 0x%08x"*/



    num_neurons = address[N_ATOMS];
    neuron_bit_field_words = get_bit_field_size(num_neurons);

    num_delay_stages = address[N_DELAY_STAGES];
    timer_offset = address[RANDOM_BACKOFF];
    time_between_spikes = address[TIME_BETWEEN_SPIKES] * sv->cpu_clk;

    uint32_t num_delay_slots = num_delay_stages * DELAY_STAGE_LENGTH;
    uint32_t num_delay_slots_pot = round_to_next_pot(num_delay_slots);
    num_delay_slots_mask = (num_delay_slots_pot - 1);

    log_mini_debug("%u%u%u%u%u%u%08x", 7381,num_neurons, neuron_bit_field_words,num_delay_stages, num_delay_slots, num_delay_slots_pot,num_delay_slots_mask);
    /* "\t parrot neurons = %u, neuron bit field words = %u, num delay stages = %u, num delay slots = %u (pot = %u), num delay slots mask = %08x"*/





    log_mini_debug("%u%u%u", 7382,timer_offset, time_between_spikes);
    /* "\t random back off = %u, time_between_spikes = %u"*/


    // Create array containing a bitfield specifying whether each neuron should
    // emit spikes after each delay stage
    neuron_delay_stage_config = (bit_field_t*) spin1_malloc(
        num_delay_stages * sizeof(bit_field_t));

    // Loop through delay stages
    for (uint32_t d = 0; d < num_delay_stages; d++) {
        log_mini_debug("%u%u", 7383, d);  /* "\t delay stage %u"*/

        // Allocate bit-field
        neuron_delay_stage_config[d] = (bit_field_t) spin1_malloc(
            neuron_bit_field_words * sizeof(uint32_t));

        // Copy delay stage configuration bits into delay stage configuration bit-field
        address_t neuron_delay_stage_config_data_address =
            &address[DELAY_BLOCKS] + (d * neuron_bit_field_words);
        spin1_memcpy(neuron_delay_stage_config[d],
               neuron_delay_stage_config_data_address,
               neuron_bit_field_words * sizeof(uint32_t));

        for (uint32_t w = 0; w < neuron_bit_field_words; w++) {
            log_mini_debug("%u%u%08x", 7384, w,neuron_delay_stage_config[d][w]);
            /* "\t\t delay stage config word %u = %08x"*/
        }
    }

    // Allocate array of counters for each delay slot
    spike_counters = (uint8_t**) spin1_malloc(
        num_delay_slots_pot * sizeof(uint8_t*));

    for (uint32_t s = 0; s < num_delay_slots_pot; s++) {

        // Allocate an array of counters for each neuron and zero
        spike_counters[s] = (uint8_t*) spin1_malloc(
            num_neurons * sizeof(uint8_t));
        zero_spike_counters(spike_counters[s], num_neurons);
    }

    log_mini_debug("%u", 7385);  /* "read_parameters: completed successfully"*/
    return true;
}

void _store_provenance_data(address_t provenance_region){
    log_mini_debug("%u", 7386);  /* "writing other provenance data"*/

    // store the data into the provenance data region
    provenance_region[N_PACKETS_RECEIVED] = n_in_spikes;
    provenance_region[N_PACKETS_PROCESSED] = n_processed_spikes;
    provenance_region[N_PACKETS_ADDED] = n_spikes_added;
    provenance_region[N_PACKETS_SENT] = n_spikes_sent;
    provenance_region[N_BUFFER_OVERFLOWS] = in_spikes_get_n_buffer_overflows();
    provenance_region[N_DELAYS] = n_delays;
    log_mini_debug("%u", 7387);  /* "finished other provenance data"*/
}

static bool initialize() {
    log_mini_info("%u", 7388);  /* "initialise: started"*/

    // Get the address this core's DTCM data starts at from SRAM
    address_t address = data_specification_get_data_address();

    // Read the header
    if (!data_specification_read_header(address)) {
        return false;
    }

    // Get the timing details and set up the simulation interface
    if (!simulation_initialise(
            data_specification_get_region(SYSTEM, address),
            APPLICATION_NAME_HASH, &timer_period, &simulation_ticks,
            &infinite_run, SDP, DMA)) {
        return false;
    }
    simulation_set_provenance_function(
        _store_provenance_data,
        data_specification_get_region(PROVENANCE_REGION, address));

    // Get the parameters
    if (!read_parameters(data_specification_get_region(
            DELAY_PARAMS, address))) {
        return false;
    }

    log_mini_info("%u", 7389);  /* "initialise: completed successfully"*/

    return true;
}

// Callbacks
void incoming_spike_callback(uint key, uint payload) {
    use(payload);

    log_mini_debug("%u%x", 7390, key);  /* "Received spike %x"*/
    n_in_spikes += 1;

    // If there was space to add spike to incoming spike queue
    in_spikes_add_spike(key);
}

// Gets the neuron ID of the incoming spike
static inline key_t _key_n(key_t k) {
    return k & incoming_neuron_mask;
}

static void spike_process() {

    // turn off interrupts as this function is critical for
    // keeping time in sync.
    uint state = spin1_int_disable();

    // Get current time slot of incoming spike counters
    uint32_t current_time_slot = time & num_delay_slots_mask;
    uint8_t *current_time_slot_spike_counters =
        spike_counters[current_time_slot];

    log_mini_debug("%u%u", 7391, current_time_slot);  /* "Current time slot %u"*/

    // While there are any incoming spikes
    spike_t s;
    while (in_spikes_get_next_spike(&s)) {
        n_processed_spikes += 1;

        if ((s & incoming_mask) == incoming_key) {

            // Mask out neuron ID
            uint32_t neuron_id = _key_n(s);
            if (neuron_id < num_neurons) {

                // Increment counter
                current_time_slot_spike_counters[neuron_id]++;
                log_mini_debug("%u%u%u", 7392, neuron_id,current_time_slot_spike_counters[neuron_id]);
                /* "Incrementing counter %u = %u\n"*/
                n_spikes_added += 1;
            } else {
                log_mini_debug("%u%u", 7393, neuron_id);  /* "Invalid neuron ID %u"*/
            }
        } else {
            log_mini_debug("%u%08x", 7394, s);  /* "Invalid spike key 0x%08x"*/
        }
    }

    // reactivate interrupts as critical section complete
    spin1_mode_restore(state);
}

void timer_callback(uint timer_count, uint unused1) {
    use(unused1);

    // Process all the spikes from the last timestep
    spike_process();

    time++;

    log_mini_debug("%u%u", 7395, time);  /* "Timer tick %u"*/

    // If a fixed number of simulation ticks are specified and these have passed
    if (infinite_run != TRUE && time >= simulation_ticks) {

        // handle the pause and resume functionality
        simulation_handle_pause_resume(NULL);

        log_mini_debug("%u%u%u%u%u%u", 7396,time, n_in_spikes, n_processed_spikes, n_spikes_sent,n_spikes_added);
        /* "Delay extension finished at time %u, %u received spikes, %u processed spikes, %u sent spikes, %u added spikes"*/




        log_mini_debug("%u%u", 7397, n_delays);  /* "Delayed %u times"*/

        // Subtract 1 from the time so this tick gets done again on the next
        // run
        time -= 1;

        simulation_ready_to_read();
        return;
    }

    // Set the next expected time to wait for between spike sending
    expected_time = sv->cpu_clk * timer_period;

    // Loop through delay stages
    for (uint32_t d = 0; d < num_delay_stages; d++) {

        // If any neurons emit spikes after this delay stage
        bit_field_t delay_stage_config = neuron_delay_stage_config[d];
        if (nonempty_bit_field(delay_stage_config, neuron_bit_field_words)) {

            // Get key mask for this delay stage and it's time slot
            uint32_t delay_stage_delay = (d + 1) * DELAY_STAGE_LENGTH;
            uint32_t delay_stage_time_slot =
                ((time - delay_stage_delay) & num_delay_slots_mask);
            uint8_t *delay_stage_spike_counters =
                spike_counters[delay_stage_time_slot];

            log_mini_debug("%u%u%u%u", 7398,time, delay_stage_time_slot, d);
            /* "%u: Checking time slot %u for delay stage %u"*/

            // Loop through neurons
            for (uint32_t n = 0; n < num_neurons; n++) {

                // If this neuron emits a spike after this stage
                if (bit_field_test(delay_stage_config, n)) {

                    // Calculate key all spikes coming from this neuron will be
                    // sent with
                    uint32_t spike_key = ((d * num_neurons) + n) + key;

                    if (delay_stage_spike_counters[n] > 0) {
                        log_mini_debug("%u%u%u%u%x", 7399,n, delay_stage_spike_counters[n], d,spike_key);
                        /* "Neuron %u sending %u spikes after delaystage %u with key %x"*/


                    }

                    // Loop through counted spikes and send
                    for (uint32_t s = 0; s < delay_stage_spike_counters[n];
                            s++) {
                        while (!spin1_send_mc_packet(spike_key, 0,
                                                     NO_PAYLOAD)) {
                            spin1_delay_us(1);
                        }
                        n_spikes_sent += 1;

                    }
                }

                // Wait until the expected time to send
                while ((ticks == timer_count) && tc[T1_COUNT] > expected_time) {

                    // Do Nothing
                    n_delays += 1;
                }
                expected_time -= time_between_spikes;
            }
        }
    }

    // Zero all counters in current time slot
    uint32_t current_time_slot = time & num_delay_slots_mask;
    zero_spike_counters(spike_counters[current_time_slot], num_neurons);
}

// Entry point
void c_main(void) {

    if (!initialize(&timer_period)) {
        log_mini_error("%u", 7400);  /* "Error in initialisation - exiting!"*/
        rt_error(RTE_SWERR);
    }

    // Start the time at "-1" so that the first tick will be 0
    time = UINT32_MAX;

    // Initialise the incoming spike buffer
    if (!in_spikes_initialize_spike_buffer(256)) {
         rt_error(RTE_SWERR);
    }

    // Set timer tick (in microseconds)
    log_mini_debug("%u%u", 7401, timer_period);  /* "Timer period %u"*/
    spin1_set_timer_tick_and_phase(timer_period, timer_offset);

    // Register callbacks
    spin1_callback_on(MC_PACKET_RECEIVED, incoming_spike_callback, MC_PACKET);
    spin1_callback_on(TIMER_TICK, timer_callback, TIMER);

    simulation_run();
}