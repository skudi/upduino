#include <stdio.h>
#include <bcm2835.h>
#include "RaspberryPi_Prog.c" 

int main(int argc, char **argv) {

//bcm2835_set_debug(1);
if (!bcm2835_init()) return 1;
	
cfg();

return 0;
}
