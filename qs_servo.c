#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <pigpio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <time.h>
#include <pigpio.h>
#include <signal.h>
#include <unistd.h>


#include "qs_servo.h"
#include "quicksilver_commands.h"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

byte tx_buff[128] = {0,};  
byte rx_buff[ARRAY_SIZE(tx_buff)] = {0, };
int fd = 0;

static char *device = "/dev/spidev0.0"; //TODO:  move to the ini file
static uint8_t mode;
static uint8_t bits = 8;
static int trace_level = 0;
static int qs_comm_error = 0;
static char* qs_comm_error_txt = 0;

static int step_x_pin;
static int dir_x_pin;
static int step_y_pin;
static int dir_y_pin; 

static uint8_t addr_x;
static uint8_t addr_y; 

int get_qs_comm_error(){
    return qs_comm_error ;
}

char* get_error_txt(){
    return qs_comm_error_txt;
}

void set_qs_comm_error(int error){
    qs_comm_error = error;
    qs_comm_error_txt = getSpiCommMsgText(error);
}

/*
  cdiv    speed
     2    125.0 MHz
     4     62.5 MHz
     8     31.2 MHz
    16     15.6 MHz
    32      7.8 MHz
    64      3.9 MHz
   128     1953 kHz
   256      976 kHz
   512      488 kHz
  1024      244 kHz
  2048      122 kHz
  4096       61 kHz
  8192     30.5 kHz
 16384     15.2 kHz
 32768     7629 Hz
*/ 
//static uint32_t speed = 0xff00; //TODO: move to the ini file
static uint32_t speed = 250000; //TODO: move to the ini file
static uint16_t delay = 0;

static uint8_t qscomm_status;

void set_trace_level(int t){
    trace_level = t;
}
/*
2019-06-01 Lucian Nutiu lucian.nutiu@gmail.com

gcc qs_servo.c -lpigpio -lpthread

Interfaces via SPI to the RS485 arduino-based communcation adapter (QSCOMM) for the silvermax quicksilver servomotors

*/

static void pabort(const char *s)
{
	perror("QSSERVO: ");
    
    gpioWaveClear();
    gpioTerminate();
    perror(s);
	abort();
}

uint8_t get_qs_status(){
    return qscomm_status;
}

static void trace_msg(int level, const char* fmt, ...){
    if (level <= trace_level){
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        printf("\r\n");
        va_end(args);        
    }
}

void fetch_comm_module_stat()
{
	int ret;
    uint8_t fetch_command[] = {0xFF, 0xFF};
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)fetch_command,
		.rx_buf = (unsigned long)rx_buff,
		.len = sizeof(fetch_command),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr, sizeof(fetch_command));
	if (ret < 1) pabort("can't send spi message");
    //the first byte is dirt because of the was SPI works, so the second one is our status
    qscomm_status = rx_buff[sizeof(fetch_command)-1]; 
    if (trace_level > 2){
    printf("RX: ");
    for ( int i = 0; i < sizeof(fetch_command); i++) {
		printf("%.2X:", rx_buff[i]);
	}
    printf("\r\n");
    }
    if (qscomm_status > ACK_OK){
        set_qs_comm_error(qscomm_status);
    }
    trace_msg(1, "QC stat %.2X %s", qscomm_status, getSpiCommMsgText(qscomm_status));
}

void send_servo_msg(byte* tx_buff, int len)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx_buff,
		.rx_buf = (unsigned long)rx_buff,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr, len);
	if (ret < 1) pabort("can't send spi message");
	if (trace_level > 2){
        printf("TX: ");
        for (int i = 0; i < len; i++) {
            printf("%.2X:", tx_buff[i]);
        }
        printf("\r\n");
    }
	//puts("");
    
}

void reset_servo_comm_interface(){
    byte msg[] = {0, 1, 'r'};
    send_servo_msg(msg, sizeof(msg));
}

void qs_set_stepdir(uint8_t addr){
    byte msg[] = {addr, 1, QS_CMD_STEPDIR};    
    send_servo_msg(msg, sizeof(msg));
}

void qs_mode_velocityimmediate(uint8_t addr, uint32_t velocity){
  byte no_of_words  = 0xD;
 
  byte msg[] = {addr, no_of_words, QS_CMD_MODE_VELOCITYIMMEDIATE,
            0x00, 0x0f, 0xEA, 0x4B, //acceleration
            velocity>>24, velocity>>16, velocity>>8, velocity&0x000000ff, //velocity
            0x00, 0x00, 0x00, 0x00}; //stop enable / stop state
  send_servo_msg(msg, sizeof(msg));
 
}

void qs_mode_velocitycontrol(uint8_t addr, uint32_t velocity){
  /* TODO
 byte no_of_words  = 0x4;
  byte msg[] = {addr, no_of_words, QS_CMD_MOVEREL_VELOCITYBASED,
            distance>>24, distance>>16, distance>>8, distance&0x000000ff, //distance
            0x00, 0x0f, 0xEA, 0x4B, //acceleration
            velocity>>24, velocity>>16, velocity>>8, velocity&0x000000ff, //velocity
            0x00, 0x00, 0x00, 0x00}; //stop enable / stop state
  send_servo_msg(msg, sizeof(msg));
  */
}

void qs_move_abs_timebased(uint8_t addr, uint32_t accel_time, uint32_t total_time ){
  byte no_of_words  = 0x11;
 
  byte msg[] = {addr, no_of_words, QS_CMD_MOVEABS_TIMEBASED,
            accel_time>>24, accel_time>>16, accel_time>>8, accel_time&0x000000ff, //ramp_up time
            total_time>>24, total_time>>16, total_time>>8, total_time&0x000000ff, //velocity
            0x00, 0x00, 0x00, 0x00}; //stop enable / stop state
  send_servo_msg(msg, sizeof(msg));
}

void qs_move_rel_timebased(uint8_t addr, uint32_t ramp_time, uint32_t total_time ){
  byte no_of_words  = 0x11;
 
  byte msg[] = {addr, no_of_words, QS_CMD_MOVEREL_TIMEBASED,
            ramp_time>>24, ramp_time>>16, ramp_time>>8, ramp_time&0x000000ff, //ramp_up time
            total_time>>24, total_time>>16, total_time>>8, total_time&0x000000ff, //velocity
            0x00, 0x00, 0x00, 0x00}; //stop enable / stop state
  send_servo_msg(msg, sizeof(msg));
}


void qs_move_rel_velocitybased(uint8_t addr, uint32_t velocity, uint32_t distance){
  byte no_of_words  = 0x11;
 
  byte msg[] = {addr, no_of_words, QS_CMD_MOVEREL_VELOCITYBASED,
            distance>>24, distance>>16, distance>>8, distance&0x000000ff, //distance
            0x00, 0x0f, 0xEA, 0x4B, //acceleration
            velocity>>24, velocity>>16, velocity>>8, velocity&0x000000ff, //velocity
            0x00, 0x00, 0x00, 0x00}; //stop enable / stop state
  send_servo_msg(msg, sizeof(msg));
}

void qs_enable_motor_driver(uint8_t addr, char motor_enable){
    uint8_t cmd = QS_CMD_ENABLE_MOTOR_DRIVER;
    if (!motor_enable) cmd = QS_CMD_DISABLE_MOTOR_DRIVER;
    byte msg[] = {addr, 1, cmd};
    send_servo_msg(msg, sizeof(msg));
}

void qs_hard_stop(uint8_t addr){
    byte msg[] = {addr, 1, QS_CMD_HARD_STOP};
    send_servo_msg(msg, sizeof(msg));
}

void qs_stop_motion(uint8_t addr){
    byte msg[] = {addr, 0x5, QS_CMD_STOP_MOTION, 0x00, 0xcf, 0xEA, 0x4B};
    send_servo_msg(msg, sizeof(msg));
}

void qs_restart_proc(uint8_t addr){
    byte msg[] = {addr, 0x1, QS_CMD_RESTART};
    send_servo_msg(msg, sizeof(msg));
}

void isrQscommTxReqCallback(int gpio, int level, uint32_t tick){
    //this callback binds to the TX_REQ signal from the QSCOMM. It will initiate the SPI transfer of the status word from the QSCOMM
    //tick : The number of microseconds since boot
    //this wraps around from 2^32 to 0 roughly every 72 minutes
    //printf("txreq %i\r\n", level); 
    if (level == 0){
        fetch_comm_module_stat();
    }else if (level == PI_TIMEOUT){
        trace_msg(0,"TIMEOUT\r\n"); 
        set_qs_comm_error(ERR_QS_COMM_TIMEOUT);
    }        
    //printf("dur %u RPM %.1f\n",dur, rpm); 
};

int qsServoInitialize(int qs_comm_isr_gpio_pin,int p_step_x_pin, int p_dir_x_pin, int p_step_y_pin, int p_dir_y_pin, int mot_addr_x, int mot_addr_y)
{
   //gpioCfgClock(10, 0, 0); 
   //int gpioCfgClock(unsigned cfgMicros, unsigned cfgPeripheral, unsigned cfgSource)
   //Configures pigpio to use a particular sample rate timed by a specified peripheral. 
   //This function is only effective if called before gpioInitialise. 
   //cfgMicros: 1, 2, 4, 5, 8, 10
   //cfgPeripheral: 0 (PWM), 1 (PCM)
   //cfgSource: deprecated, value is ignored

   trace_msg(0, "Initializing pigpio\r\n"); 
   if (gpioInitialise()<0) {
   gpioWaveClear();
   gpioTerminate();
   return 1;
   }
   trace_msg(0, "Installing ISR on gpio %i \r\n",  qs_comm_isr_gpio_pin);
   fflush(stdout); 
   //setup pins 
   step_x_pin = p_step_x_pin;
   dir_x_pin = p_dir_x_pin;
   step_y_pin = p_step_y_pin;
   dir_y_pin = p_dir_y_pin;
   addr_x = mot_addr_x;
   addr_y = mot_addr_y;
   trace_msg(0, "Init stepdir: step_x_pin %i dir_x_pin %i step_y_pin %i  dir_y pin %i\n", step_x_pin, dir_x_pin, step_y_pin, dir_y_pin);
   
   gpioSetMode(step_x_pin, PI_OUTPUT);
   gpioSetMode(dir_x_pin, PI_OUTPUT);
   gpioSetMode(step_y_pin, PI_OUTPUT);
   gpioSetMode(dir_y_pin, PI_OUTPUT);
   
   gpioSetPullUpDown(qs_comm_isr_gpio_pin, PI_PUD_UP);
   //gpioSetPullUpDown(18, PI_PUD_DOWN); // Sets a pull-down.
   //gpioSetPullUpDown(23, PI_PUD_OFF);  // Clear any pull-ups/downs.
   gpioSetMode(qs_comm_isr_gpio_pin, PI_INPUT);
   gpioSetISRFunc(qs_comm_isr_gpio_pin, FALLING_EDGE, 0, isrQscommTxReqCallback) ;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	int ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	trace_msg(1, "spi mode: %d\r\n", mode);
	trace_msg(1, "bits per word: %d\r\n", bits);
	trace_msg(1, "max speed: %d Hz (%d KHz)\r\n", speed, speed/1000);
    //if we reached this point we must have a valid fd
   
   return 0;
}


void qsServoShutdown(){
    trace_msg(0, "QS shutdown\r\n");
    gpioWaveClear();
    gpioTerminate();
    trace_msg(0, "GPIO closed\r\n");
    if (fd != 0){
        close(fd);
    }
}

//########### wavegen section ##########//
#define DUR_OFF 10
#define RAMP_STEP 1 //each pulse during the ramp up period will be shorter by this value

int get_axis_motor_addr(char axis){
    if (axis == 'X'){
        return addr_x;
    }else if (axis == 'Y'){
        return addr_y;
    }else  {
        fprintf(stderr, "Invalid axis %c", axis);
        return 0; 
    }
}

int get_axis_pin(char axis){
    if (axis == 'X'){
        return step_x_pin;
    }else if (axis == 'Y'){
        return step_y_pin;
    }else  {
        fprintf(stderr, "Invalid axis %c", axis);
        return step_x_pin; 
    }
}

int rampup_wave(char axis, int period_dur_us){ 
   
   trace_msg(1, "Rampup step signal to %i", period_dur_us);
//   gpioWaveClear();
   
   int ramp_up = 1000;
   int gpio = get_axis_pin(axis);

   while (ramp_up > period_dur_us) {
       
       #define NO_OF_PULSES_PER_RAMPSTEP 4
       gpioPulse_t pulse[NO_OF_PULSES_PER_RAMPSTEP*2];
       for (int i= 0; i < 2*NO_OF_PULSES_PER_RAMPSTEP-1; i++){
       pulse[i].gpioOn = (1<<gpio);
       pulse[i].gpioOff = 0;
       pulse[i].usDelay = ramp_up;

       pulse[i+1].gpioOn = 0;
       pulse[i+1].gpioOff = (1<<gpio);
       pulse[i+1].usDelay = ramp_up;
       }
       gpioWaveAddGeneric(NO_OF_PULSES_PER_RAMPSTEP*2, pulse);
       int wave_id = gpioWaveCreate();
       gpioWaveTxSend(wave_id, PI_WAVE_MODE_REPEAT_SYNC);
       //gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);
       //while (gpioWaveTxBusy())  usleep(10);   
       //time_sleep(0.001);   
       time_sleep(0.0001);   
       gpioWaveDelete(wave_id);
       ramp_up -= RAMP_STEP;
    }
    //gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT_SYNC); 
    //gpioWaveChain(wid, wave_no);
    start_wave(axis, period_dur_us);
    //for (int i=0; i<wave_no; i++) gpioWaveDelete(wid[i]);
   return 0;
}

int start_wave(char axis, int period_dur_us){
    //gpioWaveClear();
   //fprintf(stderr, "start wave %i ", on);
   //fflush(stderr);
//   gpioWaveClear();
   
   gpioPulse_t pulse[2];
   uint8_t gpio_pin = get_axis_pin(axis);
   // trace_msg("start_wave %c %i on pin %i \n", axis, period_dur_us, gpio_pin);
   pulse[0].gpioOn = (1<<gpio_pin);
   pulse[0].gpioOff = 0;
   pulse[0].usDelay = period_dur_us/2;

   pulse[1].gpioOn = 0;
   pulse[1].gpioOff = (1<<gpio_pin);
   pulse[1].usDelay = period_dur_us/2;

   gpioWaveAddGeneric(2, pulse);
   int wave_id = gpioWaveCreate();
   if (wave_id < 0){
    fprintf(stderr, "Wave create error\n");
    if (wave_id == PI_EMPTY_WAVEFORM){
        fprintf(stderr, "PI_EMPTY_WAVEFORM\n");
    }else if (wave_id == PI_TOO_MANY_CBS){
        fprintf(stderr, "PI_TOO_MANY_CBS\n");
    }else if (wave_id == PI_TOO_MANY_OOL){
        fprintf(stderr, "PI_TOO_MANY_OOL\n");
    }else if (wave_id == PI_NO_WAVEFORM_ID){
        fprintf(stderr, "PI_NO_WAVEFORM_ID\n");
    };
    
    return 1;
   }
   
   gpioWaveTxSend(wave_id, PI_WAVE_MODE_REPEAT_SYNC); 
   return 0;
}
