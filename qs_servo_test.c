#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <pigpio.h>
#include "qs_servo.h"

/*
2019-06-01

gcc -o qs_servo_test qs_servo_test.c qs_servo.c -lpigpio -lpthread
$ sudo ./qs_servo_test 22

This program tests the qs_servo lib 

*/

void usage()
{
   fprintf
   (stderr,
      "\n" \
      "Usage: sudo ./qs_servo_test gpio_pin\n" \
      "The gpio pin is connected to the TX_REQ pin of the QSCOMM adapter"
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

   if (qsServoInitialize(g_gpio)<0) return 1;
    uint8_t test_addr = 0x03;
    qs_set_stepdir(test_addr);   
    qs_move_rel_velocitybased(test_addr);   
   while(1) {
        usleep(100000);
        
        //printf("%i\n", (int) round((int)(rpm/10))*10);
        printf("%i %i\n", get_qs_status());
   }
   qsServoShutdown();
   return 0;
}

