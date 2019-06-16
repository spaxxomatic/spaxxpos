rm qs_servo.o qs_servo_test.o qs_servo_test
gcc -o qs_servo_test qs_servo_test.c qs_servo.c -lpigpio -lpthread
