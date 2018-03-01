#include <stdio.h>
#include <stdlib.h> // for int = atoi(char), malloc
#include <pigpio.h> // for gpio access
#include <unistd.h> // for sleep(seconds)

// z is a counter for where we are in the transmit_data
int z = 0;
const int num_of_DAC = 1;	// CHANGE THIS TO 2 WHEN SECOND DAC IMPLEMENTED (QAM)
const uint clock_pin = 4;

// DAC_bits is a bit mask of the GPIO pins which that DAC uses, expressed here
// in the form b7,...,b0. This is XOR'd with the gpioWrite_Bits_0_31_Set bit mask
// (which is passed to this program to get the bit mask for gpioWrite_Bits_0_31_Clear 
const uint DAC_1_bits[8] = {5,6,13,19,26,21,20,16};
//const uint DAC_2_bits[8];
int DAC_1_mask = (1<<5)|(1<<6)|(1<<13)|(1<<19)|(1<<26)|(1<<21)|(1<<20)|(1<<16);
//int DAC_2_mask;


// Callback function is automatically passed gpio (will be clock_pin),
// level (1 or 0) and tick (time since program start, can use to compare times)
// gpioTick() (uint32_t) gets tick at any time in code, microseconds since boot
void newState(int gpio, int level, uint32_t tick) {
	//	TEST VERSION
	if(z++%2<1){
		gpioWrite_Bits_0_31_Clear(DAC_1_mask);
		gpioWrite_Bits_0_31_Set(0);
	} else {
		gpioWrite_Bits_0_31_Clear(0);
		gpioWrite_Bits_0_31_Set(DAC_1_mask);
	}

	/*	REAL VERSION PROTOTYPE
	gpioWrite_Bits_0_31_Clear(transmit_data_mask[z] ^ DAC_1_mask);
	gpioWrite_Bits_0_31_Set(transmit_data_mask[z++]);
	*/
}

int main(int argc, char *argv[]) {
	// REMEMBER TO CHANGE SAMPLE RATE OF GPIO's, STANDARD IS 5us (200kHz), FASTEST IS 1us (1MHz)
	// gpioCfgClock()
	if (gpioInitialise()<0) { printf("GPIO INIT FAIL\n"); return 1;}

	/*  argv should have values:
	    argv[1] = transmit_data - This is a sub-bit-mask of DAC pins only,
								  passed as a char*
		argv[2] = transmit_freq - Frequency of clock signal, or take default:
	*/	int transmit_freq = 5000;
	
	if(argc>1) {
		char* transmit_data = argv[1];
	} else {
		printf("PiTransmit_2\n\n");
		printf("Usage: ./PiTransmit_2 transmit_data transmit_freq\n");
		return 1;
	}
	if(argc>2) transmit_freq = atoi(argv[2]);
	

	const int sub_mask_size = 2 * num_of_DAC;
	const int mask_size = ( sizeof(transmit_data)-1 ) / sub_mask_size;
    char format[] = "%_x";			// %2x for one DAC, %4x for two DAC's
    format[1] = sub_mask_size + 48;	// +48 for ASCII value of number

	uint32_t transmit_data_mask[] = calloc(mask_size, sizeof(uint32_t));
	for(int i = 0; i<mask_size; i++) {
		// Move each sub-mask into an int in transmit_data_mask array
		sscanf((transmit_data + sub_mask_size*i),format,&transmit_data_mask[i]);
		// Expand sub-mask into 32-bit mask
		// TODO NEXT
		// transmit_data_mask[i] = F(transmit_data_mask[i]);
		// WHERE F() MAPS THE 8-BIT SUB-MASK TO THE 32-BIT MASK
	}
	
	for(int i=0;i<8;i++) {
		gpioSetMode(DAC_1_bits[i], PI_OUTPUT);
	}
	gpioHardwareClock(clock_pin, transmit_freq);
	gpioSetAlertFunc(clock_pin,newState);

	// When testing on scope, sleep for 60s to have enough time to check
	// sleep(60);
	gpioHardwareClock(clock_pin, 0);
	gpioTerminate();
}
