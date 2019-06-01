
/* Serial AS5311 position sensor
 * Copyright 2018 Lucian Nutiu lucian.nutiu@glmail.com
 * see COPYING file for details */

#include <stdio.h>
#include <unistd.h>
#include "cssl.h"
#include <sys/types.h>
#include <sys/syscall.h>
#include "pos_decode.h"
//Resolution/increment of the magnetic AS5311 sensor, in mm/step
#define AS5311_RESOLUTION 0.0004882
static float as5311resolution = AS5311_RESOLUTION;

typedef struct  {
      char name;
      int last_error;
      int error;
      uint16_t prev_enc_value;
      uint16_t enc_value;
      long abs_position;
      float position_mm;
} axis_struct;
      
static axis_struct X_axis = {.name = 'X', .enc_value = 0, .prev_enc_value = 0 , .error = 0, .abs_position = 0, .position_mm=0};

static axis_struct Y_axis = {.name = 'Y', .enc_value = 0, .prev_enc_value = 0 , .error = 0, .abs_position = 0, .position_mm=0};

char bla[200];

void setSensorResolution(float resolution){
    as5311resolution = resolution;
}

axis_position_struct get_axis_stat(){
    axis_position_struct ret = {.xerror = X_axis.last_error, .yerror = Y_axis.last_error, .xposition = X_axis.position_mm, .yposition = Y_axis.position_mm};
    return ret;
    //return X_axis.position_mm;
}

float get_x_pos(){
    return X_axis.position_mm;
}

float get_y_pos(){
    return Y_axis.position_mm;
}

//AS5311 delivers a 12 bit absolute position inside the 2 mm pole spacing
//in order to detect the movement over the index position, we need to check if the previous absolute value is hear index 
//and the actual position over it 

#define INDEX_POS 0xFFF
#define NEAR_INDEX 0xFFF - 0x1FF
#define OVER_INDEX 0x1FF

static void read_axis(axis_struct* ptr_axis, uint16_t rcvd_encoder_val){
    if (rcvd_encoder_val >= 0xFF00) {//some read error
        ptr_axis->error = rcvd_encoder_val & 0x00FF; //lower byte
        if (ptr_axis->error != 0){
            ptr_axis->last_error = ptr_axis->error;
        }        
        //printf("%c err %i %i\n", ptr_axis->name, highbyte, lowbyte);
    }else{
        ptr_axis->error = 0;
        ptr_axis->enc_value = rcvd_encoder_val;
        int delta = ptr_axis->enc_value - ptr_axis->prev_enc_value;
        if (ptr_axis->prev_enc_value > NEAR_INDEX && ptr_axis->enc_value < OVER_INDEX){ //moving from left to right over the index
            ptr_axis->abs_position += INDEX_POS + delta;
        }else if (ptr_axis->prev_enc_value < OVER_INDEX && ptr_axis->enc_value > NEAR_INDEX){ //moving from right to left over the index
            ptr_axis->abs_position -= INDEX_POS - delta;
        }else{        
            ptr_axis->abs_position += delta;
        }
        ptr_axis->prev_enc_value = ptr_axis->enc_value;
        ptr_axis->position_mm = (float)as5311resolution*(ptr_axis->abs_position);
        //printf("%c %i %i %i \n",ptr_axis->name, ptr_axis->enc_value,  ptr_axis->prev_enc_value, ptr_axis->position);
    }
}

uint16_t x_packet; 
uint16_t y_packet;
axis_struct* this_axis = 0;
uint8_t parser_state = 0;
uint16_t* pkt_ptr;

//parser states
#define AXIS_EXPECTED 1
#define PAYLOAD_FOLLOWS AXIS_EXPECTED+1
#define READING_PAYLOAD PAYLOAD_FOLLOWS+1
#define PAYLOAD_READ_DONE READING_PAYLOAD+1

/* callback function to be called by the cssl on data arrival */
static void callback(int id, uint8_t *buf, int length)
{
    //The data format consists of 4 bytes: axis name, upper byte, lower byte, CR
    //Upper byte = 0xFF signalises an error. Error code is then the lower byte	
    int i=0;
    for ( i=0; i<length; i++) {
        if (buf[i] == 'X') {
            if (parser_state == AXIS_EXPECTED){
                this_axis = &X_axis;
                pkt_ptr = &x_packet;
                parser_state = PAYLOAD_FOLLOWS;
            }
        }else if (buf[i] == 'Y') {      
            if (parser_state == AXIS_EXPECTED){        
                this_axis = &Y_axis;
                pkt_ptr = &y_packet;
                parser_state = PAYLOAD_FOLLOWS;
            }
        }else if (buf[i] == '\n') { //end of a packet
            if (parser_state == PAYLOAD_READ_DONE){
                read_axis(this_axis, *pkt_ptr);
            }
            parser_state = AXIS_EXPECTED;
        }else{ //copy byte by byte to the axis struct
            if (parser_state == PAYLOAD_FOLLOWS){
                *pkt_ptr = buf[i]<<8;
                parser_state = READING_PAYLOAD;
            }else if(parser_state == READING_PAYLOAD){
                *pkt_ptr += buf[i];
                parser_state = PAYLOAD_READ_DONE;
            }
        }

    }
    //putchar(buf[i]);    fflush(stdout);
}

cssl_t *serial;

int init_comm(char* serial_port, int baudrate){
    cssl_start();
    //serial=cssl_open("/dev/ttyAMA0",callback,0,115200,8,0,1);
    serial=cssl_open(serial_port,callback,0,baudrate,8,0,1);
    if (!serial) {
		printf("Cannot setup connection to %s %s\n",serial_port, cssl_geterrormsg());
		return -1;
    }
    return 0;
}

void shutdown(){
    cssl_close(serial);
    cssl_stop();
}

