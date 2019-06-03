
volatile int get_rpm();
int rpmmeterInitialize(int gpio_pin);
void rpmmeterShutdown();

volatile static float rpm ;
uint32_t jitter;


