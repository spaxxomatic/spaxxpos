#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include <pigpio.h>
#include "rpm_meter.h"
/*
2019-06-01 Lucian Nutiu lucian.nutiu@gmail.com

gcc rpm_meter.c -lpigpio -lpthread

This program calculates the rpm based on the duration between pulses on a given GPIO pin

*/

static uint32_t lasttick = 0;
static uint32_t dur_accumulator = 0;
static uint16_t window_idx = 0;

#define MOVING_AVERAGE_WINDOW 0xF //!! must be a power of two - 1, so 0x3, 0x7, 0xF
#if (MOVING_AVERAGE_WINDOW & (MOVING_AVERAGE_WINDOW + 1)) != 0 
#error Invalid value for MOVING_AVERAGE_WINDOW. Must be x^2-1
#endif
static uint32_t ma_window_lastticks[MOVING_AVERAGE_WINDOW+1];

volatile int get_rpm(){
    //for (int i = 0; i < MOVING_AVERAGE_WINDOW; i++) printf ("%i ",ma_window_lastticks[i]) ;
    return (int) round(rpm*10)/10;;
}

 
void isrCallback(int gpio, int level, uint32_t tick){
    //tick : The number of microseconds since boot
    //this wraps around from 2^32 to 0 roughly every 72 minutes
    uint32_t  dur = 0;
    if (level == 0){
        if (tick < lasttick){ //taking care of tick wrapping
            lasttick = 0xFFFFFFFF - lasttick; //we keep the remaining ticks to overflow 
            dur = tick + lasttick;
        }else{
            dur = tick - lasttick ; //us duration
        }
        if (dur < 1000)  {//contact bounce or spurious irq if less than 10ms (a max rpm of 60000)
            return;
        }
        //shall we implement some jitter averaging? 
        //if (dur*(MOVING_AVERAGE_WINDOW+1) - dur_accumulator > JITTER_THRESHOLD)
        jitter =  abs(dur - ma_window_lastticks[(window_idx - 1)  & MOVING_AVERAGE_WINDOW]);
        dur&=0xFFFFFFFE; //reduce jitter by dropping last bit
        dur_accumulator -= ma_window_lastticks[window_idx & MOVING_AVERAGE_WINDOW]; //substract the oldest value
        dur_accumulator += dur; 
        ma_window_lastticks[window_idx & MOVING_AVERAGE_WINDOW] = dur ; //store the newest value
        window_idx++;
        //for (int i = 0; i < MOVING_AVERAGE_WINDOW; i++) printf ("%i ",ma_window_lastticks[i]) ;
        if (dur > 500000) //for less than 120 rpm do not use average, because response time will be to slow
            rpm = (float)60*1000000/dur;
        //we do a 'little bit of' exp moving average to improve response
        else 
            rpm = (60*1000000)*((float)MOVING_AVERAGE_WINDOW+1)*2/((float)(dur*(MOVING_AVERAGE_WINDOW+1) + dur_accumulator));
        lasttick = tick;
    }else if (level == PI_TIMEOUT){
        //stopped, empty the history
        for (int i = 0; i < MOVING_AVERAGE_WINDOW; i++) ma_window_lastticks[i] = 0;
        dur_accumulator = 0;
        rpm = 0;
        jitter = 0;
    }        
    //printf("dur %u RPM %.1f\n",dur, rpm); 
};
   
void setPwm(uint8_t pin, int freq, uint8_t fill_factor){
   gpioSetMode(pin, PI_OUTPUT);
   int iret = gpioSetPWMfrequency(pin, freq); //set pwm on GPIO 20 to a freq of 10 hz
   printf ("PWM set to %i ", iret);
   gpioPWM(pin, fill_factor); //send pulses of 1/255 fill factor 
}
    
int rpmmeterInitialize(int gpio_pin)
{
   //gpioCfgClock(10, 0, 0); 
   //int gpioCfgClock(unsigned cfgMicros, unsigned cfgPeripheral, unsigned cfgSource)
   //Configures pigpio to use a particular sample rate timed by a specified peripheral. 
   //This function is only effective if called before gpioInitialise. 
   //cfgMicros: 1, 2, 4, 5, 8, 10
   //cfgPeripheral: 0 (PWM), 1 (PCM)
   //cfgSource: deprecated, value is ignored

   if (gpioInitialise()<0) return 1;
   printf("Installing ISR on gpio %i \n",  gpio_pin);
   fflush(stdout); 
   
   gpioSetPullUpDown(gpio_pin, PI_PUD_UP);
   //gpioSetPullUpDown(18, PI_PUD_DOWN); // Sets a pull-down.
   //gpioSetPullUpDown(23, PI_PUD_OFF);  // Clear any pull-ups/downs.
   gpioSetMode(gpio_pin, PI_INPUT);
   gpioSetISRFunc(gpio_pin, FALLING_EDGE, 2000, isrCallback) ;
   return 0;
}

void rpmmeterShutdown(){
    gpioTerminate();
}

