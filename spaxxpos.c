
#include <stdio.h>
#include <unistd.h>
#include "pos_decode.h"

static int finished=0;

int main()
{
    init_comm("/dev/ttyAMA0", 115200);
    while (!finished) 
        pause();
    shutdown();
    return 0;
}