// DO NOT EDIT! THIS FILE WAS GENERATED FROM src/common/out_spikes.c

/*! \file
 *  \brief the implementation of the out_spikes.h interface.
 */
#include "out_spikes.h"

#include <recording.h>
#include <debug.h>

// Globals
typedef struct timed_out_spikes{
    uint32_t time;
    uint32_t out_spikes[];
} timed_out_spikes;

static timed_out_spikes *spikes;
bit_field_t out_spikes;
static size_t out_spikes_size;


//! \brief clears the currently recorded spikes
void out_spikes_reset() {
    clear_bit_field(out_spikes, out_spikes_size);
}

//! \brief initialise the recording of spikes
//! \param[in] max_spike_sources the number of spike sources to be recorded
//! \return True if the initialisation was successful, false otherwise
bool out_spikes_initialize(size_t max_spike_sources) {
    out_spikes_size = get_bit_field_size(max_spike_sources);
    log_mini_debug("%u%u%u", 7020,out_spikes_size, max_spike_sources);
    /* "Out spike size is %u words, allowing %u spike sources"*/
    spikes = (timed_out_spikes *) spin1_malloc(
        sizeof(timed_out_spikes) + (out_spikes_size * sizeof(uint32_t)));
    if (spikes == NULL) {
        log_mini_error("%u", 7021);  /* "Out of DTCM when allocating out_spikes"*/
        return false;
    }
    out_spikes = &(spikes->out_spikes[0]);
    out_spikes_reset();
    return true;
}

bool out_spikes_record(
        uint8_t channel, uint32_t time, uint32_t n_words,
        recording_complete_callback_t callback) {
    if (out_spikes_is_empty()) {
        return false;
    } else {
        spikes->time = time;
        recording_record_and_notify(
            channel, spikes, (n_words + 1) * sizeof(uint32_t),
            callback);
        return true;
    }
}

//! \brief Check if any spikes have been recorded
//! \return True if no spikes have been recorded, false otherwise
bool out_spikes_is_empty() {
    return (empty_bit_field(out_spikes, out_spikes_size));

}

//! \brief Check if a given neuron has been recorded to spike
//! \param[in] spike_source_index The index of the neuron.
//! \return true if the spike source has been recorded to spike
bool out_spikes_is_spike(index_t neuron_index) {
    return (bit_field_test(out_spikes, neuron_index));
}

//! \brief print out the contents of the output spikes (in DEBUG only)
void out_spikes_print() {
#if LOG_LEVEL >= LOG_DEBUG
    log_mini_debug("%u", 7022);  /* "out_spikes:\n"*/

    if (!out_spikes_is_empty()) {
        log_mini_debug("%u", 7023);  /* "-----------\n"*/
        print_bit_field(out_spikes, out_spikes_size);
        log_mini_debug("%u", 7024);  /* "-----------\n"*/
    }
#endif // LOG_LEVEL >= LOG_DEBUG
}

void out_spike_info_print(){
    log_mini_debug("%u", 7025);  /* "-----------\n"*/
    index_t i; //!< For indexing through the bit field

    for (i = 0; i < out_spikes_size; i++) {
        log_mini_debug("%u%08x", 7026, out_spikes[i]);  /* "%08x\n"*/
    }
    log_mini_debug("%u", 7027);  /* "-----------\n"*/
}
