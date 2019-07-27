#include "motion.h"

m_axis_spindle_syncd_params_t m_axis_spindle_syncd_params;

void move_axis_spindle_syncronized(char axis, int um_distance_per_rev, int no_of_turns){
    m_axis_spindle_syncd_params.axis = axis;
    m_axis_spindle_syncd_params.um_distance_per_rev =  m_axis_spindle_syncd_params;
    m_axis_spindle_syncd_params.no_of_turns = no_of_turns;
    register_rpm_sensor_pulse_callback(start_synced_movement);
}   

void start_synced_movement(uint32_t turn_duration){
    //compute distance in encoder increments qs_move_rel_velocitybased
    
    int counter_pulses_per_rev = m_axis_spindle_syncd_params.um_distance_per_rev*pulses_per_um;
    int cps_speed = counter_pulses_per_rev/turn_duration ; //the movement speed in counter pulses/sec
    
    //shall we use a time-based or a velocity-based move?
    //qs_move_rel_timebased();
    //start motion
    qs_move_rel_velocitybased(get_axis_addr(axis), cps_speed, counter_pulses_per_rev);
    m_axis_spindle_syncd_params.no_of_turns--;
}   