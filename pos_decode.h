
typedef struct {
    int xerror;
    int yerror;
    float xposition;
    float yposition;
} axis_position_struct;
axis_position_struct get_axis_stat();
float get_x_pos();
float get_y_pos();
int init_comm(char* serial_port, int baudrate);
void pos_decoder_shutdown();