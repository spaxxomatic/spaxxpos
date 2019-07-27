
#define byte uint8_t
#include "spi_proto.h"

#ifndef _qs_servo_included
#define _qs_servo_included

uint8_t get_qs_status();
int qsServoInitialize(char* p_spi_device, int qs_comm_isr_gpio_pin,int p_step_x_pin, int p_dir_x_pin, 
        int p_step_y_pin, int p_dir_y_pin, int mot_addr_x, int mot_addr_y, int x_axis_counts_per_mm, int y_axis_counts_per_mm);
void qsServoShutdown();
void reset_servo_comm_interface();
void fetch_comm_module_stat();
void qs_set_stepdir(uint8_t addr);
void qs_move_rel_velocitybased(uint8_t addr, uint32_t velocity, uint32_t distance);
void qs_mode_velocityimmediate(uint8_t addr, uint32_t velocity); 
void qs_enable_motor_driver(uint8_t addr, char motor_enable);
void qs_hard_stop(uint8_t addr);
void qs_stop_motion(uint8_t addr);
void qs_restart_proc(uint8_t addr);
void send_servo_msg(byte* tx_buff, int len);
void set_trace_level(int trace_level);
int get_qs_comm_error();
char* get_error_txt();
int rampup_wave(char axis, int duration);
int start_wave(char axis, int on);
int get_axis_motor_addr(char axis);

int enc_x_axis_counts_per_mm;
int enc_y_axis_counts_per_mm;

#endif



