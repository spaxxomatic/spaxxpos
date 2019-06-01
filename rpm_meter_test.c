#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include <pigpio.h>
#include "rpm_meter.h"

/*
2019-06-01

gcc -o rpm_meter_test rpm_meter_test.c rpm_meter.c -lpigpio -lpthread
$ sudo ./rpm_meter_test 21

This program tests the rpm_meter lib 

*/

void usage()
{
   fprintf
   (stderr,
      "\n" \
      "Usage: sudo ./rpm_meter_test gpio_pin\n" \
      "\n"
   );
}


void fatal(int show_usage, char *fmt, ...)
{
   char buf[128];
   va_list ap;

   va_start(ap, fmt);
   vsnprintf(buf, sizeof(buf), fmt, ap);
   va_end(ap);

   fprintf(stderr, "%s\n", buf);

   if (show_usage) usage();

   fflush(stderr);

   exit(EXIT_FAILURE);
}

   
int main(int argc, char *argv[])
{
   
   /* get the gpios to monitor */
   int g_gpio;
   int g_num_gpios = 0;
   if (argc != 2){
        fatal(1, "Exactly one gpio must be specified");        
   }
   int g = 0;
   g = atoi(argv[1]);
   if ((g>=0) && (g<32)){
         g_gpio = g;
   }else fatal(1, "%d is not a valid g_gpio number\n", g);

   //gpioServo(20, width[g]);
   
   if (rpmmeterInitialize(g_gpio)<0) return 1;
   while(1) {
        usleep(100000);
        int rpm = (int) round(getRpm()*10)/10;
        //printf("%i\n", (int) round((int)(rpm/10))*10);
        printf("%i %i\n", rpm, jitter);
   }
   rpmmeterShutdown();
   return 0;
}

