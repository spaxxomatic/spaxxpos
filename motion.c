#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "motion.h"

m_axis_spindle_syncd_params_t m_axis_spindle_syncd_params;

void start_synced_movement(uint32_t turn_duration){
    //compute distance in encoder increments qs_move_rel_velocitybased
    if (m_axis_spindle_syncd_params.no_of_turns == 0){
        //we're done, deregister callback and return 
        register_rpm_sensor_pulse_callback(NULL);
        return;
    }
    
    
    int counter_pulses_per_rev = m_axis_spindle_syncd_params.um_distance_per_rev*enc_x_axis_counts_per_mm;
    int cps_speed = counter_pulses_per_rev/turn_duration ; //the movement speed in counter pulses/sec
    
    //shall we use a time-based or a velocity-based move?
    //qs_move_rel_timebased();
    //start motion
    qs_move_rel_velocitybased(
        get_axis_motor_addr(m_axis_spindle_syncd_params.axis), 
        cps_speed, 
        counter_pulses_per_rev);
        
    m_axis_spindle_syncd_params.no_of_turns--;
}   

void move_axis_spindle_syncronized(char axis, int um_distance_per_rev, int no_of_turns){
    if (m_axis_spindle_syncd_params.axis != 'X'){
        fprintf(stderr, "Only X axis supported for move_axis_spindle_syncronized");
        return;
    }
    m_axis_spindle_syncd_params.axis = 'X';
    m_axis_spindle_syncd_params.um_distance_per_rev =  um_distance_per_rev;
    m_axis_spindle_syncd_params.no_of_turns = no_of_turns;
    register_rpm_sensor_pulse_callback(start_synced_movement);
}   
