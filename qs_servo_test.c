#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <pigpio.h>
#include "qs_servo.h"
#include <curses.h> 

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
      "Usage: sudo ./qs_servo_test gpio_pin servo_address\n" \
      "The gpio pin is connected to the TX_REQ pin of the QSCOMM adapter" \
      "The servo address is the address of one connected quicksilver servomotor " \
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

uint32_t distance = 0x00004000; //distance   
uint32_t speed = 0xF0000; //distance   

int main(int argc, char *argv[])
{
   cbreak();
   int g_gpio;
   int g_num_gpios = 0;
   uint8_t test_addr = 0;
   if (argc != 3){
        fatal(1, "Exactly one gpio and one servo address must be specified");        
   }
   int g = 0;
   g = atoi(argv[1]);
   if ((g>=0) && (g<32)){
         g_gpio = g;         
   }else fatal(1, "%d is not a valid g_gpio number\n", g);
   g = atoi(argv[2]);
   if ((g>=0) && (g<126)){
         test_addr = g;         
   }else fatal(1, "%d is not a valid servo address\n", g);
   
   if (qsServoInitialize(g_gpio)<0) return 1;
    
    //qs_set_stepdir(test_addr);   
   initscr(); 
   char c;
   char motor_enable = 1;
   while(1) {
        usleep(1000);
        c = getchar();
        if (c == 'q') break;
        switch (c){
        case 's':
            fetch_servo_stat(); break;     
        case 'm':
            qs_move_rel_velocitybased(test_addr, speed, distance); break;     
        case 'i':
            qs_mode_velocityimmediate(test_addr, speed); break;       
        case 'h':
            qs_stop_motion(test_addr); break;              
        case 'r':
            qs_restart_proc(test_addr); break;                          
        case 'e':
            motor_enable = !motor_enable;
            qs_enable_motor_driver(test_addr, motor_enable); break;  
        case '+':
          speed += 0xF0000; 
          printf("Speed %i \r\n", speed);
          qs_mode_velocityimmediate(test_addr, speed); break;                 
          break;     
        case '-':
          speed -= 0xF0000;
          printf("Speed %i \r\n", speed);          
          qs_mode_velocityimmediate(test_addr, speed); break;                 
          break;        
        case '8':
          distance += 0xFF; 
          printf("Distance %i \r\n", distance);          
          break;
        case '2':
          distance -= 0x0F;
          printf("Distance %i \r\n", distance);
          break;                   
        };
        //printf("%i\n", (int) round((int)(rpm/10))*10);
        printf("%i %i\r\n", get_qs_status());
   }
   endwin();
   qsServoShutdown();
   
   return 0;
}

