#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pigpio.h>
#include <time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
 
int gpio=21;

#define DUR_OFF 10
#define RAMP_STEP 1 //each pulse during the ramp up period will be shorter by this value

int start_wave(int on);

int rampup_wave(int duration){
   printf("Rampup to %i", duration);
//   gpioWaveClear();
   
   int ramp_up = 1000;
   
   int wave_no = 0; 
   while (ramp_up > duration) {
       
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
    //while (gpioWaveTxBusy())  time_sleep(0.0001);   
    start_wave(duration);
    //for (int i=0; i<wave_no; i++) gpioWaveDelete(wid[i]);
   return 0;
}

int start_wave(int on){
    //gpioWaveClear();
   //fprintf(stderr, "start wave %i ", on);
   //fflush(stderr);
//   gpioWaveClear();
   
   gpioPulse_t pulse[2];
   pulse[0].gpioOn = (1<<gpio);
   pulse[0].gpioOff = 0;
   pulse[0].usDelay = on;

   pulse[1].gpioOn = 0;
   pulse[1].gpioOff = (1<<gpio);
   pulse[1].usDelay = on;

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

int main(int argc, char *argv[])
{
   int on=1000;
   
   if (argc > 3)
   {
      gpioInitialise();
      gpioWaveClear();
      fprintf(stderr, "Too many params");
      return 1;
   }
   else if (argc > 2)
   {
      on   = atoi(argv[2]);
      gpio = atoi(argv[1]);
   }
   else if (argc > 1)
   {
      gpio = atoi(argv[1]);
   }

   printf("gpio=%d on=%d\n", gpio, on);

   if (gpioInitialise()<0) return -1;

   gpioSetMode(gpio, PI_OUTPUT);
   
       rampup_wave(on);
   
   //start_wave(on); 
   while (1) {
        int var = getchar();
        if(var == 's')   break;
        if(var == '+')   {
            on += 1;
            start_wave(on);
        }
        if(var == '-')   {
            on -= 1;
            start_wave(on);
        }        
        time_sleep(0.01);  
    }
   gpioWaveClear();
   gpioTerminate();
}
