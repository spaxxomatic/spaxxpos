#include <stdint.h>
#include "rpm_meter.h"
#include "qs_servo.h"


typedef struct {
    char axis;
    int um_distance_per_rev;
    int no_of_turns;
} m_axis_spindle_syncd_params_t;

