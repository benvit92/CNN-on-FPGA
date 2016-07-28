#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <assert.h>

#include <math.h>
#include <stdlib.h>
#include "mycnn.h"

#define conv_channel 1
#define conv_in_x 16
#define conv_in_y 16

float * readInput(){
	double *w = malloc(conv_channel*conv_in_x*conv_in_y*sizeof(*w)); /* weights */

	FILE *f = fopen("input.bin", "rb");
	
	
	assert(fread(w, sizeof(*w), conv_channel*conv_in_x*conv_in_y, f) == conv_channel*conv_in_x*conv_in_y);
	fclose(f);
	
	//new stuff to convert it into a float array
	float *fw = malloc(conv_channel*conv_in_x*conv_in_y*sizeof(*fw));
	int i;
	for(i=0;i<conv_channel*conv_in_x*conv_in_y;i++){
		fw[i] = (float) w[i];
	}

	return fw;
	
}

int set_interface_attribs (int fd, int speed, int parity) {
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
            printf ("error %d from tcgetattr", errno);
            return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~ICRNL;
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
            printf ("error %d from tcsetattr", errno);
            return -1;
    }
    return 0;

}
void set_blocking (int fd, int should_block) {
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
            printf ("error %d from tggetattr", errno);
            return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
            printf ("error %d setting term attributes", errno);
}

void writeOutput(float * w, int N){


	FILE *f = fopen("output.txt", "w");
	if (f == NULL){
	
		printf("Error opening file!\n");
		exit(1);
	}
	int i;
	for(i=0; i<N; i++){
		fprintf(f, "%f\n",w[i]);
	}
	
	
	
	fclose(f);
	
}

#define CYCLES 4
void luaForward(){
	 char *portname = "/dev/ttyS0";

	
    int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
            printf ("error %d opening %s: %s", errno, portname, strerror (errno));
            return;
    }
    
    set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (fd, 0);                // set no blocking
    char daeliminare;
    int var2=1;

    while(var2>=1) {
		var2 =read(fd,&daeliminare,1);
		printf("read %d bytes\n",var2);
    }
    //float * in = readInput();
    
    char inchars[256*CYCLES];
    
	int i,j;
	for(i=0;i<CYCLES;i++){
		for(j=0;j<256;j++){
			inchars[i*256+j] = j;
		}
	}
	
	//try both write ways!
writeAll:    write (fd, inchars, CYCLES*256);  

//writeOne:    for(i=0;i<CYCLES*256;i++) write(fd,&inchars[i],1);    
    //free(in);
    
    usleep ((1 + 25) * 50000);             // sleep enough to transmit the 7 plus
       
    char outchars[256*CYCLES];

	int var=0;

    while(var<=0) {
		var =read(fd,outchars,256*CYCLES);
		printf("read %d bytes\n",var);
    }
    
    //checking if the transfer actually worked
    int flag = 1;
	for(i=0;i<CYCLES;i++){
		for(j=0;j<256;j++){
			if(inchars[i*256+j] != outchars[i*256+j]){
				printf("char at pos %d mismatch: %d vs %d\n",i*256+j,inchars[i*256+j],outchars[i*256+j]);
				flag = 0;
			}
		}
	}
	if(flag)
		printf("Success!\n");
	else
		printf("Failure\n");

	float buf[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	writeOutput(buf,10);

}

int main(){
	
	luaForward();

}
