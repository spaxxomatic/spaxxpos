
#include <stdio.h>
#include <unistd.h>
#include "pos_decode.h"

static int finished=0;

int main()
{
    if (init_comm("/dev/ttyAMA0", 115200)<0){
		return -1 ;
	}; 
    
    while (!finished) {
        sleep(1);
        axis_position_struct ret = get_axis_stat();
        printf("%i %i %i %i\n", ret.xerror, ret.yerror, ret.xposition, ret.yposition);

    }
    shutdown();
    return 0;
}