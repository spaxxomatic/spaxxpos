#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <pigpio.h>
#include "qs_servo.h"
#include <curses.h> 
#include <signal.h>

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
      "Usage: sudo qs_servo_test gpio_pin servo_address\n" \
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

uint32_t distance = 4000; //distance   
uint32_t speed = 0xF0000; //distance   

void shutdown(int dummy) {
    qsServoShutdown();
    endwin();
    exit(EXIT_FAILURE);
}

#define GPIO_STEP_X 21
#define GPIO_DIR_X 20
#define GPIO_STEP_Y 16
#define GPIO_DIR_Y 19

#define QS_COMM_GPIO_PIN 4

int main(int argc, char *argv[])
{
   cbreak();
   
   uint8_t test_addr = 0;
   if (argc != 2){
        fatal(1, "Exactly one servo address must be specified");        
   }
   int g = 0;
   g = atoi(argv[1]);
   if ((g>=0) && (g<126)){
         test_addr = g;         
   }else fatal(1, "%d is not a valid servo address\n", g);
   
   if (qsServoInitialize(QS_COMM_GPIO_PIN, GPIO_STEP_X, GPIO_DIR_X, GPIO_STEP_Y, GPIO_DIR_Y)<0) return 1;
   set_trace_level(3);
    
    signal(SIGINT, shutdown);
    signal(SIGCONT, shutdown);
    signal(SIGTERM, shutdown);
    
    //qs_set_stepdir(test_addr);   
   initscr(); 
   char c;
   char motor_enable = 1;
   byte test_eroneous_msg[] = {1, 5, 0};  
   
   int wave_duration = 1000;
   
   //enable step/dir
  qs_set_stepdir(test_addr); 
   while(1) {
        usleep(10000);
        c = getchar();
        
        if (c == 'q') break;
        
        switch (c){
        case 'S':
            fetch_comm_module_stat();
            break;     
        case 's':
            qs_set_stepdir(test_addr); 
            break;     
        case '!':
            send_servo_msg(test_eroneous_msg, sizeof(test_eroneous_msg));
            break;               
        case 'm':
            qs_move_rel_velocitybased(test_addr, speed, distance); break;     
        case 'i':
            qs_mode_velocityimmediate(test_addr, speed); break;       
        case 'h':
            qs_stop_motion(test_addr); break;              
        case 'r':
            qs_restart_proc(test_addr); break;                  
        case 'o':
            reset_servo_comm_interface(test_addr); break;                 
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
          distance += 4000; 
          printf("Distance %i \r\n", distance);          
          break;
        case '2':
          distance -= 4000;
          printf("Distance %i \r\n", distance);
          break;     
        case 'w':
          if (wave_duration > 0) wave_duration -= 10;
          start_wave('X', wave_duration);
          printf("Freq %i \r\n", 1000000/wave_duration);
          break; 
        case 'W':
          wave_duration += 10;
          start_wave('X', wave_duration);
          printf("Freq %i \r\n", 1000000/wave_duration);
          break;
        }  
        //printf("%i\n", (int) round((int)(rpm/10))*10);
   }
   endwin();
   qsServoShutdown();
   
   return 0;
}

