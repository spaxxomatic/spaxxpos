
volatile int get_rpm();

int rpmmeterInitialize(int gpio_pin);
void rpmmeterShutdown();

void register_rpm_sensor_pulse_callback(void rsp_callback(uint32_t));

volatile static float rpm ;
uint32_t jitter;


