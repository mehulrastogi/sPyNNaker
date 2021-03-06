// DO NOT EDIT! THIS FILE WAS GENERATED FROM src/neuron/threshold_types/threshold_type_static.h

#ifndef _THRESHOLD_TYPE_STATIC_H_
#define _THRESHOLD_TYPE_STATIC_H_
#include "threshold_type.h"
typedef struct threshold_type_t {

    // The value of the static threshold
    REAL threshold_value;
} threshold_type_t;

static inline bool threshold_type_is_above_threshold(state_t value,
                        threshold_type_pointer_t threshold_type) {
    return REAL_COMPARE(value, >=, threshold_type->threshold_value);
}

#endif // _THRESHOLD_TYPE_STATIC_H_
