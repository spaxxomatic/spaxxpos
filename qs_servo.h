
uint8_t get_qs_status();
int qsServoInitialize(int gpio_pin);
void qsServoShutdown();

void qs_set_stepdir(uint8_t addr);
void qs_move_rel_velocitybased(uint8_t addr);


