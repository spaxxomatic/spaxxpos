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

#include "qs_servo.h"
#include "quicksilver_commands.h"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

byte tx_buff[128] = {0,};  
byte rx_buff[ARRAY_SIZE(tx_buff)] = {0, };
int fd = 0;

static char *device = "/dev/spidev0.0"; //TODO:  move to the ini file
static uint8_t mode;
static uint8_t bits = 8;
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
/*
2019-06-01 Lucian Nutiu lucian.nutiu@gmail.com

gcc qs_servo.c -lpigpio -lpthread

Interfaces via SPI to the RS485 arduino-based communcation adapter (QSCOMM) for the silvermax quicksilver servomotors

*/

static void pabort(const char *s)
{
	perror("QSSERVO: ");
    
    gpioTerminate();
    perror(s);
	abort();
}

uint8_t get_qs_status(){
    return qscomm_status;
}

void fetch_servo_stat()
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

	for (ret = 0; ret < sizeof(fetch_command); ret++) {
		printf(" R %.2X ", rx_buff[ret]);
	}
	//puts("");
    
    qscomm_status = rx_buff[sizeof(fetch_command)];
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
    printf("\r\nTX:");
    int i ;
	for ( i = 0; i < len; i++) {
		printf("%.2X:", tx_buff[i]);
	}
    printf("\r\nRX:");
	for (i = 0; i < len; i++) {
		printf("%.2X:", rx_buff[i]);
	}
	puts("");
    
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
  byte no_of_words  = 0x4;
 /* TBD
  byte msg[] = {addr, no_of_words, QS_CMD_MOVEREL_VELOCITYBASED,
            distance>>24, distance>>16, distance>>8, distance&0x000000ff, //distance
            0x00, 0x0f, 0xEA, 0x4B, //acceleration
            velocity>>24, velocity>>16, velocity>>8, velocity&0x000000ff, //velocity
            0x00, 0x00, 0x00, 0x00}; //stop enable / stop state
  send_servo_msg(msg, sizeof(msg));
  */
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
    uint32_t  dur = 0;
    printf("txreq %i\r\n", level); 
    if (level == 0){
        fetch_servo_stat();
    }else if (level == PI_TIMEOUT){
        printf("TIMEOUT\r\n"); 
    }        
    //printf("dur %u RPM %.1f\n",dur, rpm); 
};

int qsServoInitialize(int gpio_pin)
{
   //gpioCfgClock(10, 0, 0); 
   //int gpioCfgClock(unsigned cfgMicros, unsigned cfgPeripheral, unsigned cfgSource)
   //Configures pigpio to use a particular sample rate timed by a specified peripheral. 
   //This function is only effective if called before gpioInitialise. 
   //cfgMicros: 1, 2, 4, 5, 8, 10
   //cfgPeripheral: 0 (PWM), 1 (PCM)
   //cfgSource: deprecated, value is ignored

   printf("Initializing pigpio\r\n"); 
   if (gpioInitialise()<0) {
   gpioTerminate();
   return 1;
   }
   printf("Installing ISR on gpio %i \r\n",  gpio_pin);
   fflush(stdout); 
   
   gpioSetPullUpDown(gpio_pin, PI_PUD_UP);
   //gpioSetPullUpDown(18, PI_PUD_DOWN); // Sets a pull-down.
   //gpioSetPullUpDown(23, PI_PUD_OFF);  // Clear any pull-ups/downs.
   gpioSetMode(gpio_pin, PI_INPUT);
   gpioSetISRFunc(gpio_pin, FALLING_EDGE, 0, isrQscommTxReqCallback) ;

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

	printf("spi mode: %d\r\n", mode);
	printf("bits per word: %d\r\n", bits);
	printf("max speed: %d Hz (%d KHz)\r\n", speed, speed/1000);
    //if we reached this point we must have a valid fd
   
   return 0;
}


void qsServoShutdown(){
    printf("QS shutdown\r\n");
    gpioTerminate();
    printf("GPIO closed\r\n");
    if (fd != 0){
        printf("FD closed\r\n");
        close(fd);
    }
}

