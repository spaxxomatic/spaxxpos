#define byte uint8_t

uint8_t get_qs_status();
int qsServoInitialize(int gpio_pin);
void qsServoShutdown();

void reset_servo_comm_interface();
void fetch_servo_stat();
void qs_set_stepdir(uint8_t addr);
void qs_move_rel_velocitybased(uint8_t addr, uint32_t velocity, uint32_t distance);
void qs_mode_velocityimmediate(uint8_t addr, uint32_t velocity);
void qs_enable_motor_driver(uint8_t addr, char motor_enable);
void qs_hard_stop(uint8_t addr);
void qs_stop_motion(uint8_t addr);
void qs_restart_proc(uint8_t addr);
void send_servo_msg(byte* tx_buff, int len);


